/*
 *
 * (C) Copyright 2018 SAMSUNG Electronics
 * Sanghyun Lee <sh425.lee@samsung.com>
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 *
 */

#include <dev/mmc.h>
#include <reg.h>
#include <stdlib.h>
#include <string.h>
#include <platform/delay.h>
#include <err.h>
#include <part.h>

#ifndef CONFIG_SYS_MMC_MAX_BLK_COUNT
//#define CONFIG_SYS_MMC_MAX_BLK_COUNT 32767
/* HACK MAX BLK */
#define CONFIG_SYS_MMC_MAX_BLK_COUNT 4096
#endif

/* #define MMC_TEST */

static struct mmc mmc_channel[MMC_MAX_CHANNEL];
static mmc_device_t _mmc_dev[MMC_MAX_DEVICE];
static unsigned int _mmc_dev_cnt;
static unsigned char ext_csd_buf[512];
const u8 tuning_blk_pattern_4bit[] = {
		0xff, 0x0f, 0xff, 0x00, 0xff, 0xcc, 0xc3, 0xcc,
		0xc3, 0x3c, 0xcc, 0xff, 0xfe, 0xff, 0xfe, 0xef,
		0xff, 0xdf, 0xff, 0xdd, 0xff, 0xfb, 0xff, 0xfb,
		0xbf, 0xff, 0x7f, 0xff, 0x77, 0xf7, 0xbd, 0xef,
		0xff, 0xf0, 0xff, 0xf0, 0x0f, 0xfc, 0xcc, 0x3c,
		0xcc, 0x33, 0xcc, 0xcf, 0xff, 0xef, 0xff, 0xee,
		0xff, 0xfd, 0xff, 0xfd, 0xdf, 0xff, 0xbf, 0xff,
		0xbb, 0xff, 0xf7, 0xff, 0xf7, 0x7f, 0x7b, 0xde,
};

/* frequency bases */
/* divided by 10 to be nice to platforms without floating point */
static const int fbase[] = {
	10000,
	100000,
	1000000,
	10000000,
};

/* Multiplier values for TRAN_SPEED.  Multiplied by 10 to be nice
 * to platforms without floating point.
 */
static const int multipliers[] = {
	0,			/* reserved */
	10,
	12,
	13,
	15,
	20,
	25,
	30,
	35,
	40,
	45,
	50,
	55,
	60,
	70,
	80,
};

static unsigned int change_endian(unsigned int value)
{
	unsigned int ret;

	ret = ((value & (0xFF << 0)) << 24) |
		((value & (0xFF << 8)) << 8) |
		((value & (0xFF << 16)) >> 8) |
		((value & (0xFF << 24)) >> 24);
	return ret;
}

/*
 * Check cards is SD
 */
static bool mmc_is_sd(struct mmc *mmc)
{
	if (mmc->type == MMC_BOOT_TYPE_SDHC
		|| mmc->type == MMC_BOOT_TYPE_STD_SD)
		return 1;
	return 0;
}

/*
 * Check cards supports high capacity
 */
static bool mmc_is_hc(struct mmc *mmc)
{
	if (mmc->type == MMC_BOOT_TYPE_SDHC
		|| mmc->type == MMC_BOOT_TYPE_MMCHC)
		return 1;
	return 0;
}

/*
 * Get data from specific bits area to Big Endian
 */
static unsigned int mmc_extract_bits_reverse(unsigned int start, unsigned int end,
				unsigned int *value, unsigned int size)
{
	unsigned int idx1 = (size - 1) - (start / 32);
	unsigned int offset = start % 32;
	unsigned int ret = change_endian(value[idx1]) >> offset;
	unsigned int idx2 = (size - 1) - (end / 32);
	unsigned int mask = (1 << (end - start + 1)) - 1;

	if (idx2 > idx1)
		ret |= (change_endian(value[idx2]) << (32 - offset));
	ret &= mask;
	return ret;
}

/*
 * Get data from specific bits area.
 */
static unsigned int mmc_extract_bits(unsigned int start, unsigned int end,
				unsigned int *value)
{
	unsigned int idx1 = start / 32;
	unsigned int offset = start % 32;
	unsigned int ret = value[idx1] >> offset;
	unsigned int idx2 = end / 32;
	unsigned int mask = (1 << (end - start + 1)) - 1;

	if (idx2 > idx1)
		ret |= (value[idx2] << (32 - offset));
	ret &= mask;
	return ret;
}

/*
 * Get data from ext_csd
 */
static unsigned int mmc_extract_ext_csd(unsigned int start, unsigned int end,
				unsigned char *ext_csd)
{
	return mmc_extract_bits(start * 8, end * 8 + 7, (unsigned int *)ext_csd);
}

/*
 * Send command include retry function
 */
static int mmc_send_command(struct mmc *mmc, struct mmc_cmd *cmd)
{
	unsigned int i;
	int ret = NO_ERROR;

	if (!cmd->retries || cmd->retries < 1)
		cmd->retries = 1;
	if (cmd->retries > mmc->cmd_retry)
		cmd->retries = mmc->cmd_retry;

	for (i = 0; i < cmd->retries ; i++) {
		ret = mmc->send_command(mmc, cmd);
		if (ret == NO_ERROR)
			return ret;
	}
	printf("send command failed\n");
	return ret;
}

/*
 * Send app command for changing next command to SD app command
 */
static int mmc_boot_send_app_cmd(struct mmc *mmc, unsigned int rca)
{
	struct mmc_cmd cmd;
	int mmc_ret = NO_ERROR;

	memset((struct mmc_cmd *)&cmd, 0,
	       sizeof(struct mmc_cmd));

	cmd.cmdidx = CMD55_APP_CMD;
	cmd.argument = (rca << 16);
	cmd.resp_type = MMC_BOOT_RESP_R1;
	cmd.data = NULL;

	mmc_ret = mmc_send_command(mmc, &cmd);

	if (mmc_ret != NO_ERROR) {
		printf("app command failed\n");
		return mmc_ret;
	}

	return mmc_ret;
}

/*
 * Send status command for checking card status
 */
int mmc_boot_get_card_status(struct mmc *mmc, int timeout, unsigned int *status)
{
	struct mmc_cmd cmd;
	int err;

	memset((struct mmc_cmd *)&cmd, 0,
	       sizeof(struct mmc_cmd));

	cmd.cmdidx = CMD13_SEND_STATUS;
	cmd.argument = mmc->rca << 16;
	cmd.resp_type = MMC_BOOT_RESP_R1;
	cmd.data = NULL;

	do {
		err = mmc_send_command(mmc, &cmd);
		if (err == NO_ERROR) {
			if ((cmd.response[0] & MMC_BOOT_R1_RDY_FOR_DATA) &&
			    (cmd.response[0] & MMC_BOOT_R1_CURR_STATE)
			    != MMC_BOOT_R1_CURR_STATE_PRG)
				break;
			else if (cmd.response[0] & MMC_BOOT_R1_STATUS_MASK) {
				printf("Status Error: 0x%08x\n", cmd.response[0]);
				return ERR_GENERIC;
			}
		}
		udelay(1000);

	} while (timeout--);

	*status = (cmd.response[0] & MMC_BOOT_R1_CURR_STATE) >> 9;
	printf("CURR STATE:%d\n", *status);

	if (timeout == 0) {
		printf("Timeout waiting card ready\n");
		return ERR_TIMED_OUT;
	}

	return NO_ERROR;
}

/*
 * Send switch command for changing SD
 */
int mmc_boot_sd_switch_cmd(struct mmc *mmc, int mode, int group, u8 value, u8 *resp)
{
	struct mmc_cmd cmd;
	int ret = 0;
	struct mmc_data data;

	memset((struct mmc_cmd *)&cmd, 0,
	       sizeof(struct mmc_cmd));
	memset((struct mmc_data *)&data, 0,
	       sizeof(struct mmc_data));


	/* Switch the frequency */
	cmd.cmdidx = CMD6_SWITCH_FUNC;
	cmd.resp_type = MMC_BOOT_RESP_R1B;
	cmd.argument = ((mode << 31) | 0xffffff);
	cmd.argument &= ~(0xf << (group * 4));
	cmd.argument |= value << (group * 4);

	data.flags = MMC_DATA_READ;
	data.dest = resp;
	data.block_cnt = 1;
	data.block_size = 64;
	cmd.data = &data;
	ret = mmc_send_command(mmc, &cmd);

	return ret;
}


/*
 * Send set_bus_width command for changing SD bus width
 */
int mmc_boot_sd_set_buswidth(struct mmc *mmc)
{
	struct mmc_cmd cmd;
	int ret = 0;

	memset((struct mmc_cmd *)&cmd, 0,
	       sizeof(struct mmc_cmd));

	cmd.cmdidx = ACMD6_SET_BUS_WIDTH;
	cmd.resp_type = MMC_BOOT_RESP_R1;
	cmd.argument = 1 << mmc->bus_width;
	cmd.data = NULL;
	ret = mmc_send_command(mmc, &cmd);

	return ret;
}

/*
* Send switch command for changing EXT_CSD register
*/
int mmc_boot_switch_cmd(struct mmc *mmc, u8 set, u8 index, u8 value)
{
	struct mmc_cmd cmd;
	unsigned int status;
	int timeout = 1000;
	int ret;

	if (!mmc_is_sd(mmc) && index > 191) {
		printf("EXT_CSD can modified under 192\n");
		return ERR_GENERIC;
	}

	memset((struct mmc_cmd *)&cmd, 0,
	       sizeof(struct mmc_cmd));

	cmd.cmdidx = CMD6_SWITCH_FUNC;
	cmd.resp_type = MMC_BOOT_RESP_R1B;
	cmd.argument = (MMC_SWITCH_MODE_WRITE_BYTE << 24) | (index << 16) | (value << 8);
	cmd.data = NULL;

	ret = mmc_send_command(mmc, &cmd);

	/* Waiting for the ready status */
	if (ret == NO_ERROR)
		ret = mmc_boot_get_card_status(mmc, timeout, &status);

	return ret;
}

/*
 * Set cards init state
 */
static int mmc_set_init_state(struct mmc *mmc)
{
	int ret;

	mmc->status = MMC_BOOT_STATUS_INACTIVE;
	mmc->rd_block_len = MMC_BOOT_RD_BLOCK_LEN;
	mmc->wr_block_len = MMC_BOOT_WR_BLOCK_LEN;
	mmc->cmd_retry = MMC_BOOT_MAX_COMMAND_RETRY;
	mmc->clock = 400000;
	mmc->bus_width = MMC_BOOT_BUS_1BIT;
	mmc->bus_mode = MMC_BOOT_BUS_SDR;

	mmc->abort_cmd.cmdidx = CMD12_STOP_TRANSMISSION;
	mmc->abort_cmd.argument = 0;
	mmc->abort_cmd.resp_type = MMC_BOOT_RESP_R1B;
	mmc->abort_cmd.data = NULL;
	mmc->host_init(mmc);
	ret = mmc->set_ios(mmc);
	if (ret != NO_ERROR) {
		printf("Initializing start failed\n");
		return ret;
	}
	udelay(500);
	return NO_ERROR;
}

/*
 * Reset all the cards to idle condition (CMD 0)
 */
static int mmc_boot_reset_cards(struct mmc *mmc)
{
	struct mmc_cmd cmd;

	memset((struct mmc_cmd *)&cmd, 0,
	       sizeof(struct mmc_cmd));

	cmd.cmdidx = CMD0_GO_IDLE_STATE;
	cmd.argument = 0;
	cmd.resp_type = MMC_BOOT_RESP_NONE;
	cmd.data = NULL;

	/* send command */

	return mmc_send_command(mmc, &cmd);
}



/*
 * Send CMD1 to know whether the card supports host VDD profile or not.
 */
static int mmc_boot_send_op_cond(struct mmc *mmc)
{
	struct mmc_cmd cmd;
	unsigned int mmc_resp = 0;
	int mmc_ret = NO_ERROR;

	/* basic check */
	if (mmc == NULL)
		return ERR_GENERIC;

	memset((struct mmc_cmd *)&cmd, 0,
	       sizeof(struct mmc_cmd));

	/* CMD1 format:
	 * [31] Busy bit
	 * [30:29] Access mode
	 * [28:24] reserved
	 * [23:15] 2.7-3.6
	 * [14:8]  2.0-2.6
	 * [7]     1.7-1.95
	 * [6:0]   reserved
	 */

	cmd.cmdidx = CMD1_SEND_OP_COND;
	cmd.argument = mmc->ocr;
	cmd.resp_type = MMC_BOOT_RESP_R3;

	/* HACK */
	cmd.argument = 0x40300000;

	mmc_ret = mmc_send_command(mmc, &cmd);
	if (mmc_ret != NO_ERROR)
		return mmc_ret;

	/* Now it's time to examine response */
	mmc_resp = cmd.response[0];

	/* Check the response for busy status */
	if (!(mmc_resp & MMC_BOOT_OCR_BUSY))
		return ERR_BUSY;

	if (mmc_resp & MMC_BOOT_OCR_SEC_MODE)
		mmc->type = MMC_BOOT_TYPE_MMCHC;
	else
		mmc->type = MMC_BOOT_TYPE_STD_MMC;

	return mmc_ret;
}

static int mmc_boot_sd_init_card(struct mmc *mmc)
{
	int i, mmc_ret;
	unsigned int ocr_cmd_arg;
	struct mmc_cmd cmd;

	memset((struct mmc_cmd *)&cmd, 0,
	       sizeof(struct mmc_cmd));

	mmc_ret = mmc_boot_reset_cards(mmc);
	if (mmc_ret != NO_ERROR) {
		printf("Error No.:%d: Failure resetting MMC cards!\n", mmc_ret);
		return mmc_ret;
	}
	udelay(2000);

	/* Send CMD8 to set interface condition */
	for (i = 0; i < 3; i++) {
		cmd.cmdidx = CMD8_SEND_IF_COND;
		cmd.argument = MMC_BOOT_SD_HC_VOLT_SUPPLIED;
		cmd.resp_type = MMC_BOOT_RESP_R7;
		cmd.data = NULL;

		mmc_ret = mmc_send_command(mmc, &cmd);
		if (mmc_ret == NO_ERROR) {
			if ((cmd.response[0] & MMC_BOOT_SD_HC_VOLT_SUPPLIED)
					!= MMC_BOOT_SD_HC_VOLT_SUPPLIED)
				return ERR_GENERIC;
			/* Set argument for ACMD41 */
			ocr_cmd_arg = MMC_BOOT_SD_NEG_OCR | MMC_BOOT_SD_HC_HCS;
			ocr_cmd_arg |= MMC_BOOT_SD_OCR_XPC | MMC_BOOT_SD_OCR_S18R;
			break;
		}
		mdelay(1);
	}

	if (mmc_ret != NO_ERROR) {
		mmc->exist = 0;
		return mmc_ret;
	}

	/* Send ACMD41 to set operating condition */
	/* Try for a max of 1 sec as per spec */
	for (i = 0; i < 20; i++) {
		mmc_ret = mmc_boot_send_app_cmd(mmc, 0);
		if (mmc_ret != NO_ERROR)
			return mmc_ret;

		cmd.cmdidx = ACMD41_SEND_OP_COND;
		cmd.argument = ocr_cmd_arg;
		cmd.resp_type = MMC_BOOT_RESP_R3;
		cmd.data = NULL;

		mmc_ret = mmc_send_command(mmc, &cmd);
		if (mmc_ret != NO_ERROR) {
			return mmc_ret;
		} else if (cmd.response[0] & MMC_BOOT_SD_DEV_READY) {
			mmc->ocr = cmd.response[0];
			/* Check for HC */
			if (cmd.response[0] & MMC_BOOT_SD_HC_HCS)
				mmc->type = MMC_BOOT_TYPE_SDHC;
			else
				mmc->type = MMC_BOOT_TYPE_STD_SD;
			break;
		}
		mdelay(1);
	}

	/* 
	 * If SD support 1.8V mode and not failed mode changed,
	 * change UHS mode with voltage switch.
	 */
	if (((mmc->ocr & (MMC_BOOT_SD_HC_HCS | MMC_BOOT_SD_OCR_S18R)) ==
			(MMC_BOOT_SD_HC_HCS | MMC_BOOT_SD_OCR_S18R))
			&& mmc->speed_mode != MMC_DENY_UHS) {
		mmc->set_ios(mmc);
		cmd.cmdidx = CMD11_SWITCH_VOLTAGE;
		cmd.argument = 0;
		cmd.resp_type = MMC_BOOT_RESP_R1;
		cmd.data = NULL;
		mmc_ret = mmc_send_command(mmc, &cmd);
		if (mmc_ret)
			return mmc_ret;
		if (cmd.response[0] & (1<<19))
			return ERR_GENERIC;
		mmc->voltage_switch(mmc);
		mmc->speed_mode = MMC_SET_UHS;
	}
	return NO_ERROR;
}

/*
 * Initialize Card before knowing card type.
 */
static int mmc_boot_init_card(struct mmc *mmc)
{
	unsigned int mmc_retry = 0;
	int mmc_return = NO_ERROR;

	/* basic check */
	if (mmc == NULL)
		return ERR_GENERIC;

	/* 1. Card Reset - CMD0 */
	mmc_return = mmc_boot_reset_cards(mmc);
	if (mmc_return != NO_ERROR) {
		printf("Error No.:%d: Failure resetting MMC cards!\n", mmc_return);
		return mmc_return;
	}
	udelay(2000);

	/* 2. Card Initialization process */

	/* Send CMD1 to identify and reject cards that do not match host's VDD range
	 * profile. Cards sends its OCR register in response.
	 */
	mmc_retry = 0;
	do {
		mmc_return = mmc_boot_send_op_cond(mmc);
		/* Card returns busy status. We'll retry again! */
		if (mmc_return == ERR_BUSY) {
			mmc_retry++;
			mdelay(1);
			continue;
		} else if (mmc_return == NO_ERROR) {
			mmc->rca = 0;
			break;
		} else {
			return mmc_return;
		}
	} while (mmc_retry < mmc->cmd_retry);

	/* If card still returned busy status we are out of luck.
	 * Card cannot be initialized
	 */
	if (mmc_return == ERR_BUSY) {
		printf("Error No. %d: Card has busy status set.\n", mmc_return);
		return mmc_return;
	}
	return mmc_return;
}


/*
 * Request any card to send its uniquie card identification (CID) number (CMD2).
 */
static int mmc_boot_all_send_cid(struct mmc *mmc)
{
	struct mmc_cmd cmd;
	int mmc_ret = NO_ERROR;

	/* basic check */
	if (mmc == NULL)
		return ERR_GENERIC;

	memset((struct mmc_cmd *)&cmd, 0,
	       sizeof(struct mmc_cmd));

	/* CMD2 Format:
	 * [31:0] stuff bits
	 */
	cmd.cmdidx = CMD2_ALL_SEND_CID;
	cmd.argument = 0;
	cmd.resp_type = MMC_BOOT_RESP_R2;
	cmd.data = NULL;

	/* send command */
	mmc_ret = mmc_send_command(mmc, &cmd);
	if (mmc_ret != NO_ERROR) {
		printf("Error No. %d: Failure Send CID!\n", mmc_ret);
		return mmc_ret;
	}

	memcpy(&mmc->cid, cmd.response, 16);
	return NO_ERROR;
	}

/*
 * Ask any card to send it's relative card address (RCA).This RCA number is
 * shorter than CID and is used by the host to address the card in future (CMD3)
 */
static int mmc_boot_send_relative_address(struct mmc *mmc)
{
	struct mmc_cmd cmd;
	int mmc_ret = NO_ERROR;

	/* basic check */
	if (mmc == NULL)
		return ERR_GENERIC;

	memset((struct mmc_cmd *)&cmd, 0,
	       sizeof(struct mmc_cmd));

	/* CMD3 Format:
	 * [31:0] stuff bits
	 */
	cmd.cmdidx = CMD3_SEND_RELATIVE_ADDR;
	if (mmc_is_sd(mmc))
		cmd.argument = 0;
	else
		cmd.argument = mmc->rca << 16;
	cmd.resp_type = MMC_BOOT_RESP_R6;
	cmd.data = NULL;

	/* send command */
	mmc_ret = mmc_send_command(mmc, &cmd);
	if (mmc_ret != NO_ERROR) {
		printf("Error No. %d: Failure Send RCA!\n", mmc_ret);
		return mmc_ret;
	}

	/* For sD, card will send RCA. Store it */
	if (mmc_is_sd(mmc))
		mmc->rca = (cmd.response[0] >> 16) & 0xffff;

	return NO_ERROR;
}

/*
 * Requests card to send it's CSD register's contents. (CMD9)
 */
static int mmc_boot_send_csd(struct mmc *mmc, unsigned int *raw_csd)
{
	struct mmc_cmd cmd;
	unsigned int mmc_arg = 0;
	int mmc_ret = NO_ERROR;

	/* basic check */
	if (mmc == NULL)
		return ERR_GENERIC;

	memset((struct mmc_cmd *)&cmd, 0,
	       sizeof(struct mmc_cmd));

	/* CMD9 Format:
	 * [31:16] RCA
	 * [15:0] stuff bits
	 */
	mmc_arg |= mmc->rca << 16;

	cmd.cmdidx = CMD9_SEND_CSD;
	cmd.argument = mmc_arg;
	cmd.resp_type = MMC_BOOT_RESP_R2;
	cmd.data = NULL;

	/* send command */
	mmc_ret = mmc_send_command(mmc, &cmd);
	if (mmc_ret != NO_ERROR) {
		printf("Error No. %d: Failure Send CSD!\n", mmc_ret);
		return mmc_ret;
	}

	/* response contains the card csd */
	memcpy(raw_csd, cmd.response, sizeof(cmd.response));

	return NO_ERROR;
}

/*
 * Selects a card by sending CMD7 to the card with its RCA.
 * If RCA field is set as 0 ( or any other address ),
 * the card will be de-selected. (CMD7)
 */
static int mmc_boot_select_card(struct mmc *mmc, unsigned int rca)
{
	struct mmc_cmd cmd;
	unsigned int mmc_arg = 0;
	int mmc_ret = NO_ERROR;

	/* basic check */
	if (mmc == NULL)
		return ERR_GENERIC;

	memset((struct mmc_cmd *)&cmd, 0,
	       sizeof(struct mmc_cmd));

	/* CMD7 Format:
	 * [31:16] RCA
	 * [15:0] stuff bits
	 */
	mmc_arg |= mmc->rca << 16;

	cmd.cmdidx = CMD7_SELECT_DESELECT_CARD;
	cmd.argument = mmc_arg;
	/* If we are deselecting card, we do not get response */
	if (rca == mmc->rca && rca) {
		if (mmc_is_sd(mmc))
			cmd.resp_type = MMC_BOOT_RESP_R1B;
		else
			cmd.resp_type = MMC_BOOT_RESP_R1;
	} else {
		cmd.resp_type = MMC_BOOT_RESP_NONE;
	}

	/* send command */
	mmc_ret = mmc_send_command(mmc, &cmd);
	if (mmc_ret != NO_ERROR) {
		printf("Error No. %d: Failure Select card\n", mmc_ret);
		return mmc_ret;
	}

	return NO_ERROR;
}

/*
 * Send command to set block length.
 */
static int mmc_boot_set_block_len(struct mmc *mmc, unsigned int block_len)
{
	struct mmc_cmd cmd;
	int mmc_ret = NO_ERROR;

	/* basic check */
	if (mmc == NULL)
		return ERR_GENERIC;

	memset((struct mmc_cmd *)&cmd, 0,
	       sizeof(struct mmc_cmd));

	/* CMD16 Format:
	 * [31:0] block length
	 */

	cmd.cmdidx = CMD16_SET_BLOCKLEN;
	cmd.argument = block_len;
	cmd.resp_type = MMC_BOOT_RESP_R1;

	/* send command */
	mmc_ret = mmc_send_command(mmc, &cmd);
	if (mmc_ret != NO_ERROR) {
		printf("Error No. %d: Send block length failed\n", mmc_ret);
		return mmc_ret;
	}

	/* If blocklength is larger than 512 bytes,
	 * the card sets BLOCK_LEN_ERROR bit.
	 */
	if (cmd.response[0] & MMC_BOOT_R1_BLOCK_LEN_ERR) {
		printf("Error No. %d: Block length error\n", mmc_ret);
		return ERR_BAD_LEN;
	}

	return NO_ERROR;
}

/*
 * Send ext csd command.
 */
static int mmc_boot_send_scr(struct mmc *mmc, unsigned char *buf)
{
	struct mmc_cmd cmd;
	struct mmc_data data;
	int mmc_ret = NO_ERROR;

	/* basic check */
	if (mmc == NULL)
		return ERR_GENERIC;

	memset(buf, 0, 64);

	memset((struct mmc_cmd *)&cmd, 0,
	       sizeof(struct mmc_cmd));
	memset((struct mmc_data *)&data, 0,
	       sizeof(struct mmc_data));

	/* CMD51 */
	cmd.cmdidx = ACMD51_SEND_SCR;
	cmd.argument = 0;
	cmd.resp_type = MMC_BOOT_RESP_R1;
	data.flags = MMC_DATA_READ;
	data.dest = buf;
	data.block_cnt = 1;
	data.block_size = 8;
	cmd.data = &data;

	/* send command */
	mmc_ret = mmc_send_command(mmc, &cmd);
	if (mmc_ret != NO_ERROR) {
		printf("Error No.%d: Send SCR Failed\n", mmc_ret);
		return mmc_ret;
	}

	return NO_ERROR;
}

/*
 * Send ext csd command.
 */
static int mmc_boot_send_ext_cmd(struct mmc *mmc, unsigned char *buf)
{
	struct mmc_cmd cmd;
	struct mmc_data data;
	int mmc_ret = NO_ERROR;
	unsigned int status;

	/* basic check */
	if (mmc == NULL)
		return ERR_GENERIC;

	memset(buf, 0, 512);

#if 0
	/* set block len */
	if (mmc_is_hc(mmc)) {
		mmc_ret = mmc_boot_set_block_len(mmc, 512);
		if (mmc_ret != NO_ERROR) {
			printf("Error No.%d: Failure setting block length for Card (RCA:%08x)\n", mmc_ret, mmc->rca);
			return mmc_ret;
		}
	}
#endif

	memset((struct mmc_cmd *)&cmd, 0,
	       sizeof(struct mmc_cmd));
	memset((struct mmc_data *)&data, 0,
	       sizeof(struct mmc_data));

	/* HACK Check eMMC device */
	mmc_boot_get_card_status(mmc, 1000, &status);

	/* CMD8 */
	cmd.cmdidx = CMD8_SEND_EXT_CSD;
	cmd.argument = 0;
	cmd.resp_type = MMC_BOOT_RESP_R1;
	data.flags = MMC_DATA_READ;
	data.dest = buf;
	data.block_cnt = 1;
	data.block_size = 512;
	cmd.data = &data;

	/* send command */
	mmc_ret = mmc_send_command(mmc, &cmd);
	if (mmc_ret != NO_ERROR) {
		printf("Error No.%d: Send EXT_CSD Failed\n", mmc_ret);
		return mmc_ret;
	}

	return NO_ERROR;
}

/*
 * Decodes CSD response received from the card. Note that we have defined only
 * few of the CSD elements in csd structure. We'll only decode those values.
 */
static int
mmc_boot_decode_mmc_info(struct mmc *mmc, unsigned int *raw_csd)
{
	unsigned int freq, mult;
	u64 capacity, csize, cmult;
	int mmc_return = NO_ERROR;

	mmc->clock = 400000;
	mmc->user_size = 0;
	mmc->boot_size = 0;
	mmc->rpmb_size = 0;
	mmc->capacity = 0;
	mmc->erase_grp_size = 0;
	mmc->rd_block_len = 0;
	mmc->wr_block_len = 0;
	mmc->maxseg_size = 0;

	mmc->csd[0] = raw_csd[3];
	mmc->csd[1] = raw_csd[2];
	mmc->csd[2] = raw_csd[1];
	mmc->csd[3] = raw_csd[0];

	/* Decode version data from csd */
	mmc->version = mmc_extract_bits(122, 125, mmc->csd);

	/* Decode frequency data from csd */
	freq = fbase[mmc_extract_bits(96, 98, mmc->csd)];
	mult = multipliers[mmc_extract_bits(99, 102, mmc->csd)];
	mmc->clock = freq * mult;

	/* Decode block length data from csd */
	mmc->rd_block_len = mmc_extract_bits(80, 83, mmc->csd);
	mmc->wr_block_len = mmc_extract_bits(22, 25, mmc->csd);

	/* Calculate capacity from csd */
	if (mmc_is_hc(mmc)) {
		csize = mmc_extract_bits(48, 69, mmc->csd);
		cmult = 8;
	} else {
		csize = mmc_extract_bits(62, 73, mmc->csd);
		cmult = mmc_extract_bits(47, 49, mmc->csd);
	}
	mmc->user_size = (csize + 1) << (cmult + 2);
	mmc->user_size *= mmc->rd_block_len;

	if (mmc->rd_block_len > MMC_MAX_BLOCK_LEN)
		mmc->rd_block_len = MMC_MAX_BLOCK_LEN;

	if (mmc->wr_block_len > MMC_MAX_BLOCK_LEN)
		mmc->wr_block_len = MMC_MAX_BLOCK_LEN;

	/* HACK read/write block size */
	mmc->rd_block_len = MMC_MAX_BLOCK_LEN;
	mmc->wr_block_len = MMC_MAX_BLOCK_LEN;

	/* If mmc version higher than 4, support EXT_CSD register */
	if (mmc->version >= 4) {

		/* Get EXT_CSD data */
		mmc_return = mmc_boot_send_ext_cmd(mmc, ext_csd_buf);
		if (mmc_return != NO_ERROR) {
			printf("Error No.%d: Failure getting card's ExtCSD information!\n", mmc_return);
			return mmc_return;
		}

		/* Get capacity data */
		capacity = (u64)mmc_extract_ext_csd(EXT_CSD_SEC_CNT, EXT_CSD_SEC_CNT+3, ext_csd_buf);
		capacity *= MMC_MAX_BLOCK_LEN;
		if (capacity >> 31)
			mmc->user_size = capacity;
		if (ext_csd_buf[EXT_CSD_REV])
			mmc->version = ext_csd_buf[EXT_CSD_REV] + 4;

		if (ext_csd_buf[EXT_CSD_BOOT_MULT])
			mmc->boot_size = ext_csd_buf[EXT_CSD_BOOT_MULT];
		if (ext_csd_buf[EXT_CSD_RPMB_MULT])
			mmc->rpmb_size = ext_csd_buf[EXT_CSD_RPMB_MULT];

		/* Get erase group data */
		if (ext_csd_buf[EXT_CSD_ERASE_GROUP_DEF] & 0x01)
			mmc->erase_grp_size = ext_csd_buf[EXT_CSD_HC_ERASE_GRP_SIZE] * 512 * 1024;
		else
			mmc->erase_grp_size = (mmc_extract_bits(42, 46, mmc->csd) + 1) *
						(mmc_extract_bits(37, 41, mmc->csd) + 1);

		/* Check secure feature support */
		mmc->sec_feature_support = ext_csd_buf[EXT_CSD_SEC_FEATURE_SUPPORT];

		/* Change high-speed mode */
		mmc_return = mmc_boot_switch_cmd(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_HS_TIMING, 1);
		if (mmc_return != NO_ERROR) {
			printf("Error No.%d: Failure sending switch command\n", mmc_return);
			return mmc_return;
		}

		/* Check mmc changing high-speed mode */
		mmc_return = mmc_boot_send_ext_cmd(mmc, ext_csd_buf);
		if (mmc_return != NO_ERROR) {
			printf("Error No.%d: Failure getting card's ExtCSD information!\n", mmc_return);
			return mmc_return;
		}
		if (ext_csd_buf[EXT_CSD_HS_TIMING])
			mmc->card_caps = ext_csd_buf[EXT_CSD_CARD_TYPE];
		else
			mmc->card_caps = 0;
	}

	mmc->capacity = mmc->user_size;
	return NO_ERROR;
}


/*
 * Decode SD Device information include CSD, SCR, Function group data.
 */
static unsigned int
mmc_boot_decode_sd_info(struct mmc *mmc, unsigned int *raw_csd)
{
	unsigned int freq, mult;
	u64 csize, cmult;
	int mmc_return = NO_ERROR;
	unsigned int scr_buf[2];
	unsigned int switch_status[16];
	unsigned int retries;

	mmc->clock = 400000;
	mmc->user_size = 0;
	mmc->boot_size = 0;
	mmc->rpmb_size = 0;
	mmc->capacity = 0;
	mmc->erase_grp_size = 0;
	mmc->rd_block_len = 0;
	mmc->wr_block_len = 0;
	mmc->maxseg_size = 0;
	mmc->csd_version = 0;

	mmc->csd[0] = raw_csd[3];
	mmc->csd[1] = raw_csd[2];
	mmc->csd[2] = raw_csd[1];
	mmc->csd[3] = raw_csd[0];

	/* Decode version data from csd */
	mmc->csd_version = mmc_extract_bits(126, 127, mmc->csd);

	/* Calculate capacity from csd */
	freq = fbase[mmc_extract_bits(96, 98, mmc->csd)];
	mult = multipliers[mmc_extract_bits(99, 102, mmc->csd)];
	mmc->clock = freq * mult;

	/* SD card ERASE_GROUP_SIZE is only 1 */
	mmc->erase_grp_size = 1;

	/* CSD is different about SD card version */
	if (mmc->csd_version == 0) {
		mmc->rd_block_len = mmc_extract_bits(80, 83, mmc->csd);
		mmc->wr_block_len = mmc->rd_block_len;

		if (mmc_is_hc(mmc)) {
			csize = mmc_extract_bits(48, 69, mmc->csd);
			cmult = 8;
		} else {
			csize = mmc_extract_bits(62, 73, mmc->csd);
			cmult = mmc_extract_bits(47, 49, mmc->csd);
		}
		mmc->user_size = (csize + 1) << (cmult + 2);
		mmc->user_size *= mmc->rd_block_len;

		if (mmc->rd_block_len > MMC_MAX_BLOCK_LEN)
			mmc->rd_block_len = MMC_MAX_BLOCK_LEN;
		if (mmc->wr_block_len > MMC_MAX_BLOCK_LEN)
			mmc->wr_block_len = MMC_MAX_BLOCK_LEN;

	} else {
		mmc->rd_block_len = MMC_MAX_BLOCK_LEN;
		mmc->wr_block_len = MMC_MAX_BLOCK_LEN;

		csize = mmc_extract_bits(48, 69, mmc->csd);
		mmc->user_size = (1 + csize) * 512 * 1024;
	}

	/* SD get SCR register */
	mmc_return = mmc_boot_send_app_cmd(mmc, mmc->rca);
	if (mmc_return != NO_ERROR) {
		printf("Error No. %d: Failure send app command before sending SCR\n", mmc_return);
		return mmc_return;
	}

	mmc_return = mmc_boot_send_scr(mmc, (unsigned char *)scr_buf);
	if (mmc_return != NO_ERROR) {
		printf("Error No. %d: Failure sending SCR\n", mmc_return);
		return mmc_return;
	}

	mmc->scr[0] = scr_buf[0];
	mmc->scr[1] = scr_buf[1];

	/* Check SD Bus width data */
	mmc->version = mmc_extract_bits_reverse(56, 59, mmc->scr, 2);
	if (mmc_extract_bits_reverse(48, 51, mmc->scr, 2) & (1<<2))
		mmc->bus_width = MMC_BOOT_BUS_4BIT;
	else
		mmc->bus_width = MMC_BOOT_BUS_1BIT;

	if (mmc->version == 0)
		return NO_ERROR;

	/* Get function group and check SD supporting speed mode */
	retries = 4;
	while (retries--) {
		mmc_return = mmc_boot_sd_switch_cmd(mmc, 0, 0, 1, (u8 *) switch_status);
		if (mmc_return != NO_ERROR) {
			printf("Error No. %d: Failure switch\n", mmc_return);
			return mmc_return;
		}
		if (!(mmc_extract_bits_reverse(272, 287, switch_status, (sizeof(switch_status)/4)) & (1 << 1)))
			break;
	}

	if (retries == 0) {
		printf("High speed is busy\n");
		return NO_ERROR;
	}
	mmc->card_caps = mmc_extract_bits_reverse(400, 415, switch_status, (sizeof(switch_status)/4));

	return NO_ERROR;
}


/*
 * Performs card identification process:
 * - get card's unique identification number (CID)
 * - get(for sd)/set (for mmc) relative card address (RCA)
 * - get CSD
 * - select the card, thus transitioning it to Transfer State
 * - get Extended CSD (for mmc)
 */
static int mmc_boot_identify_card(struct mmc *mmc)
{
	int mmc_return = NO_ERROR;
	unsigned int raw_csd[4];

	/* basic check */
	if (mmc == NULL)
		return ERR_GENERIC;

	/* Ask card to send its unique card identification (CID) number (CMD2) */
	mmc_return = mmc_boot_all_send_cid(mmc);
	if (mmc_return != NO_ERROR) {
		printf("Error No. %d: Failure getting card's CID number!\n", mmc_return);
		return mmc_return;
	}

	/* Ask card to send a relative card address (RCA) (CMD3) */
	mmc_return = mmc_boot_send_relative_address(mmc);
	if (mmc_return != NO_ERROR) {
		printf("Error No. %d: Failure getting card's RCA!\n",
			mmc_return);
		return mmc_return;
	}

	/* Get card's CSD register (CMD9) */
	mmc_return = mmc_boot_send_csd(mmc, raw_csd);
	if (mmc_return != NO_ERROR) {
		printf("Error No.%d: Failure getting card's CSD information!\n", mmc_return);
		return mmc_return;
	}

	/* Select the card (CMD7) */
	mmc_return = mmc_boot_select_card(mmc, mmc->rca);
	if (mmc_return != NO_ERROR) {
		printf("Error No.%d: Failure selecting the Card with RCA: %x\n", mmc_return, mmc->rca);
		return mmc_return;
	}

	/* Set the card status as active */
	mmc->status = MMC_BOOT_STATUS_ACTIVE;

	/* Decode and save the CSD register */
	if (mmc_is_sd(mmc))
		mmc_return = mmc_boot_decode_sd_info(mmc, raw_csd);
	else
		mmc_return = mmc_boot_decode_mmc_info(mmc, raw_csd);

	if (mmc_return != NO_ERROR) {
		printf("Error No.%d: Failure decoding card's CSD information!\n", mmc_return);
		return mmc_return;
	}

	return NO_ERROR;
}

/*
 * Do SD card tuning process
 */
int mmc_boot_sd_tuning(struct mmc *mmc)
{
	int err;
	unsigned int i;
	unsigned int original;
	unsigned int sample_good = 0;
	unsigned int median;
	unsigned int tuning_blk_size = sizeof(tuning_blk_pattern_4bit);
	u8 tuning_blk[64];
	int max = 0, cur_cnt = 0;
	int max_index = 0, cur_index = -1;
	int mask_num[8] = { 13, 11, 9, 7, 5, 4, 3, 0};
	int mask_bit[8] = { 9, 7, 6, 5, 3, 2, 1, 0};
	struct mmc_cmd cmd;
	struct mmc_data data;

	/* basic check */
	if (mmc == NULL)
		return ERR_GENERIC;

	memset((struct mmc_cmd *)&cmd, 0,
	       sizeof(struct mmc_cmd));
	memset((struct mmc_data *)&data, 0,
	       sizeof(struct mmc_data));

	/* Setting tuning command */
	cmd.cmdidx = CMD19_SEND_TUNING_BLOCK;
	cmd.resp_type = MMC_BOOT_RESP_R1;
	data.dest = tuning_blk;
	data.block_size = tuning_blk_size;
	data.block_cnt = 1;
	data.flags = MMC_DATA_READ;
	cmd.data = &data;

	original = mmc->change_clksel(mmc, 0);
	original = (original & (0x7)) * 2 + ((original & (1<<6))>>6);

	/* Check tuning command about each clksel */
	for (i = 0; i < 16; i++) {
		mmc->change_clksel(mmc, i%16);
		err = mmc_send_command(mmc, &cmd);
		if (!memcmp(tuning_blk_pattern_4bit, tuning_blk, tuning_blk_size) && !err)
			sample_good |= (1<<i);
	}

	/* In all pass situaton after tuning process, get entire median value */
	if (sample_good == 0xFFFF) {
		max_index = 0;
		max = 16;
		printf("tuning pattern all pass\n");
		goto out;
	}

	sample_good |= (sample_good<<16);
	printf("tuning pattern : %08x\n", sample_good);

	/* Find the longest interval that has passed the tuning process. */
	for (i = 0; i < 32; i++) {
		if (sample_good & (1<<i)) {
			if (cur_index == -1)
				cur_index = i;
			cur_cnt++;
		} else {
			if (max < cur_cnt) {
				max_index = cur_index;
				max = cur_cnt;
			}
			cur_index = -1;
			cur_cnt = 0;
		}
	}
out:
	/* Find the medium value pass interval */
	for (i = 0; i < 8; i++) {
		if (max >= mask_num[i]) {
			max = mask_bit[i];
			break;
		}
	}
	median = (max_index + max) % 16;

	/* Ignore sample tuning bit */
	if (median % 2)
		median = (median + 1) % 16;
	mmc->change_clksel(mmc, median);
	return NO_ERROR;
}

/*
 * Adjust the interface speed to optimal speed
 */
static int mmc_boot_sd_adjust_interface_speed(struct mmc *mmc)
{
	int mmc_ret = NO_ERROR;
	int i;
	unsigned int switch_status[16];
	unsigned int sel;
	unsigned int start, end;
	unsigned int switch_order[3] = {2, 0, 3};
	unsigned int function_group[4][4] = {{0x0, 0x0, 0x0, SD_CLK_25MHZ},
					{0x0, 0x0, 0x1, SD_CLK_50MHZ},
					{0x0, 0x1, 0x2, SD_CLK_100MHZ},
					{0x0, 0x3, 0x3, SD_CLK_208MHZ} };

	mmc->bus_mode = MMC_BOOT_BUS_SDR;
	if (mmc->bus_width == MMC_BOOT_BUS_4BIT) {
		mmc_ret = mmc_boot_send_app_cmd(mmc, mmc->rca);
		if (mmc_ret != NO_ERROR) {
			printf("Error No. %d: Failure send app command before changing bus_width\n", mmc_ret);
			return mmc_ret;
		}
		mmc_ret = mmc_boot_sd_set_buswidth(mmc);
		if (mmc_ret != NO_ERROR) {
			printf("Error No. %d: Failure send set_bus_width\n", mmc_ret);
			return mmc_ret;
		}
	}
	mmc->set_ios(mmc);

	if (mmc->card_caps & SD_UHS_SDR104)
		sel = 3;
	else if (mmc->card_caps & SD_UHS_SDR50)
		sel = 2;
	else if (mmc->card_caps & SD_HS_SDR25)
		sel = 1;
	else
		sel = 0;

	/*
	 * switch for changing function group in SD card.
	 * the order is driver strength, current limit, speed mode.
	 */
	if (mmc->speed_mode == MMC_SET_UHS) {
		for (i = 0; i < 3 ; i++) {
			mmc_ret = mmc_boot_sd_switch_cmd(mmc, 1, switch_order[i], function_group[sel][i], (u8 *)switch_status);
			if (mmc_ret) {
				printf("Switch cmd returned failure %d\n", __LINE__);
				return mmc_ret;
			}
			start = MMC_SD_SWITCH_FUNCTION_GROUP1 + switch_order[i] * 4;
			end = start + 4;
			if (mmc_extract_bits_reverse(start, end, switch_status, 16) != function_group[sel][i])
				printf("Send switch cmd but mode not changed %d\n", __LINE__);
		}
	}

	mmc->clock = function_group[sel][3];
	mmc->set_ios(mmc);
	if (sel == 3 || sel == 2)
		mmc_boot_sd_tuning(mmc);

	return NO_ERROR;
}

/*
 * Adjust the interface speed to optimal speed
 */
static int mmc_boot_adjust_interface_speed(struct mmc *mmc)
{
	int mmc_ret = NO_ERROR;

	if (mmc->card_caps & (MMC_HS_52MHZ_1_8V_3V_IO | MMC_HS_52MHZ_1_2V_IO)) {
#if 0
		mmc->clock = MMC_CLK_52MHZ;
		mmc->bus_width = MMC_BOOT_BUS_8BIT;
		mmc->bus_mode = MMC_BOOT_BUS_DDR;
#else
		/* HACK 8bit SDR mode set */
		mmc->bus_width = MMC_BOOT_BUS_8BIT;
		mmc->bus_mode = MMC_BOOT_BUS_SDR;
#endif
	} else if (mmc->card_caps & MMC_HS_52MHZ) {
		mmc->clock = MMC_CLK_52MHZ;
		mmc->bus_width = MMC_BOOT_BUS_8BIT;
		mmc->bus_mode = MMC_BOOT_BUS_DDR;
	} else {
		mmc->clock = MMC_CLK_26MHZ;
		mmc->bus_width = MMC_BOOT_BUS_8BIT;
		mmc->bus_mode = MMC_BOOT_BUS_SDR;
	}
	/* Setting HS_TIMING in EXT_CSD (CMD6) */
	mmc_ret = mmc_boot_switch_cmd(mmc, EXT_CSD_CMD_SET_NORMAL,
				      EXT_CSD_BUS_WIDTH,
				      mmc->bus_mode * 4 + mmc->bus_width);

	if (mmc_ret != NO_ERROR) {
		printf("Switch cmd returned failure %d\n", __LINE__);
		return mmc_ret;
	}
	mmc->set_ios(mmc);

	return NO_ERROR;
}

/*
 * Initialize Device and get device data.
 */
static int mmc_boot_init_and_identify_card(struct mmc *mmc)
{
	int mmc_return = NO_ERROR;

	/* Basic check */
	if (mmc == NULL)
		return ERR_GENERIC;

	/* Initialize MMC card structure */
	mmc_return = mmc_set_init_state(mmc);
	if (mmc_return != NO_ERROR)
		return mmc_return;

	/* Check for sD card */
	mmc_return = mmc_boot_sd_init_card(mmc);
	if (mmc_return != NO_ERROR) {
		/* Start initialization process (CMD0 & CMD1) */
		mmc_return = mmc_boot_init_card(mmc);
		if (mmc_return != NO_ERROR) {
			return mmc_return;
		}
	}

	/* Identify (CMD2, CMD3 & CMD9) and select the card (CMD7) */
	mmc_return = mmc_boot_identify_card(mmc);
	if (mmc_return != NO_ERROR)
		return mmc_return;

	if (mmc_is_sd(mmc)) {
		/* set interface speed */
		mmc_return = mmc_boot_sd_adjust_interface_speed(mmc);
		if (mmc_return != NO_ERROR)
			return mmc_return;
	} else {
		/* set interface speed */
		mmc_return = mmc_boot_adjust_interface_speed(mmc);
		if (mmc_return != NO_ERROR)
			return mmc_return;
	}

	if (!mmc_is_sd(mmc)) {
		/* eMMC_n_RST Eanble */
		mmc_return = mmc_boot_switch_cmd(mmc, EXT_CSD_CMD_SET_NORMAL,
				EXT_CSD_RST_N_FUNCTION, 1);

		if (mmc_return != NO_ERROR) {
			printf("eMMC n RST Enable Fail!\n");
			return mmc_return;

		}
	}

	return NO_ERROR;
}

/*
 * Select physical partition in mmc.
 */
static int mmc_select_partition(mmc_device_t *mdev, struct mmc *mmc)
{
	int err = NO_ERROR;
	struct mmc_cmd cmd;

	if (mmc_is_sd(mmc))
		return NO_ERROR;

	cmd.cmdidx = CMD6_SWITCH_FUNC;
	cmd.resp_type = MMC_BOOT_RESP_R1B;
	cmd.argument = (MMC_SWITCH_MODE_WRITE_BYTE<<24) |
			(EXT_CSD_PART_CONF << 16) |
			(((1 << 6) | (1 << 3)) << 8);
	cmd.argument |= mdev->partition << 8;
	cmd.data = NULL;

	err = mmc_send_command(mmc, &cmd);

	cmd.cmdidx = CMD6_SWITCH_FUNC;
	cmd.resp_type = MMC_BOOT_RESP_R1B;
	cmd.argument = ((3 << 24) | (177 << 16) | ((1 << 0) << 8));
	cmd.data = NULL;

	err = mmc_send_command(mmc, &cmd);

	return err;
}

/*
 * Process write request 
 */
static status_t mmc_bwrite(struct bdev *dev, const void *buf, bnum_t block, uint count)
{
	mmc_device_t *mdev = (mmc_device_t *)dev->private;
	struct mmc *mmc = (struct mmc *)mdev->mmc;
	struct mmc_cmd cmd;
	struct mmc_data data;
	int mmc_return = NO_ERROR;
	u32 backup;

	if (mdev->partition != 0) {
		mmc_return = mmc_select_partition(mdev, mmc);
		if (mmc_return != NO_ERROR) {
			printf("Select partition failed\n");
			return mmc_return;
		}
	}

	memset((struct mmc_cmd *)&cmd, 0,
	       sizeof(struct mmc_cmd));

	if (count > 1)
		cmd.cmdidx = CMD25_WRITE_MULTIPLE_BLOCK;
	else
		cmd.cmdidx = CMD24_WRITE_SINGLE_BLOCK;

	if (mmc_is_hc(mmc))
		cmd.argument = block;
	else
		cmd.argument = block * mmc->wr_block_len;

	cmd.resp_type = MMC_BOOT_RESP_R1;
	data.flags = MMC_DATA_WRITE;
	data.src = buf;
	data.block_cnt = count;
	data.block_size = mmc->wr_block_len;
	cmd.data = &data;

	mmc_return = mmc_send_command(mmc, &cmd);
	if (mmc_return != NO_ERROR) {
		printf("mmc fail to send write cmd\n");
		return mmc_return;
	}

	if (count > 1) {
		cmd.cmdidx = CMD12_STOP_TRANSMISSION;
		cmd.argument = 0;
		cmd.resp_type = MMC_BOOT_RESP_R1B;
		cmd.data = NULL;
		mmc_return = mmc_send_command(mmc, &cmd);
		if (mmc_return != NO_ERROR) {
			printf("mmc fail to send stop cmd\n");
			return mmc_return;
		}
	}

	if (mdev->partition != 0) {
		backup = mdev->partition;
		mdev->partition = 0;
		mmc_return = mmc_select_partition(mdev, mmc);
		mdev->partition = backup;
		if (mmc_return != NO_ERROR) {
			printf("Select partition failed\n");
			goto err;
		}
	}

	return NO_ERROR;
err:
	return -1;
}

/*
 * Write 0 buffer, It is same result that erase sequence
 */
static status_t mmc_berase_write(struct bdev *dev, bnum_t block, uint count)
{
	STACKBUF_DMA_ALIGN(temp, dev->block_size * count);

	memset(temp, 0, dev->block_size * count);
	return mmc_bwrite(dev, (void *)temp, block, count);
}

/*
 * Process read request 
 */
static status_t mmc_bread(struct bdev *dev, void *buf, bnum_t block, uint count)
{
	mmc_device_t *mdev = (mmc_device_t *)dev->private;
	struct mmc *mmc = (struct mmc *)mdev->mmc;
	struct mmc_cmd cmd;
	struct mmc_data data;
	int mmc_return = NO_ERROR;
	u32 backup;

	if (mdev->partition != 0) {
		mmc_return = mmc_select_partition(mdev, mmc);
		if (mmc_return != NO_ERROR) {
			printf("Select partition failed\n");
			return mmc_return;
		}
	}

	memset((struct mmc_cmd *)&cmd, 0,
	       sizeof(struct mmc_cmd));

	if (count > 1)
		cmd.cmdidx = CMD18_READ_MULTIPLE_BLOCK;
	else
		cmd.cmdidx = CMD17_READ_SINGLE_BLOCK;

	if (mmc_is_hc(mmc))
		cmd.argument = block;
	else
		cmd.argument = block * mmc->rd_block_len;

	cmd.resp_type = MMC_BOOT_RESP_R1;
	data.flags = MMC_DATA_READ;
	data.dest = buf;
	data.block_cnt = count;
	data.block_size = mmc->rd_block_len;
	cmd.data = &data;
	mmc_return = mmc_send_command(mmc, &cmd);
	if (mmc_return != NO_ERROR) {
		printf("mmc fail to send read cmd\n");
		return mmc_return;
	}
	if (count > 1) {
		cmd.cmdidx = CMD12_STOP_TRANSMISSION;
		cmd.argument = 0;
		cmd.resp_type = MMC_BOOT_RESP_R1B;
		cmd.data = NULL;
		mmc_return = mmc_send_command(mmc, &cmd);
		if (mmc_return != NO_ERROR) {
			printf("mmc fail to send stop cmd\n");
			return mmc_return;
		}
	}

	if (mdev->partition != 0) {
		backup = mdev->partition;
		mdev->partition = 0;
		mmc_return = mmc_select_partition(mdev, mmc);
		mdev->partition = backup;
		if (mmc_return != NO_ERROR) {
			printf("Select partition failed\n");
			goto err;
		}
	}

	return NO_ERROR;
err:
	return -1;
}

/*
 * Process erase request 
 */
static status_t mmc_berase(struct bdev *dev, bnum_t block, uint count)
{
	mmc_device_t *mdev = (mmc_device_t *)dev->private;
	struct mmc *mmc = (struct mmc *)mdev->mmc;
	struct mmc_cmd cmd;
	int mmc_return = NO_ERROR;
	ulong end;
	u32 start;
	int start_cmd, end_cmd;

	memset((struct mmc_cmd *)&cmd, 0,
	       sizeof(struct mmc_cmd));

	start = block;
	end = start + count - 1;
	if (!mmc_is_hc(mmc)) {
		end *= mmc->wr_block_len;
		start *= mmc->wr_block_len;
	}

	if (mmc_is_sd(mmc)) {
		start_cmd = CMD32_ERASE_WR_BLK_START;
		end_cmd = CMD33_ERASE_WR_BLK_END;
	} else {
		start_cmd = CMD35_ERASE_GROUP_START;
		end_cmd = CMD36_ERASE_GROUP_END;
	}

	cmd.cmdidx = start_cmd;
	cmd.argument = start;
	cmd.resp_type = MMC_BOOT_RESP_R1;
	cmd.data = NULL;

	mmc_return = mmc_send_command(mmc, &cmd);
	if (mmc_return != NO_ERROR)
		goto err_out;

	cmd.cmdidx = end_cmd;
	cmd.argument = end;

	mmc_return = mmc_send_command(mmc, &cmd);
	if (mmc_return != NO_ERROR)
		goto err_out;

	cmd.argument = NORMAL_ERASE;

	cmd.cmdidx = CMD38_ERASE;
	cmd.resp_type = MMC_BOOT_RESP_R1B;

	mmc_return = mmc_send_command(mmc, &cmd);
	if (mmc_return != NO_ERROR)
		goto err_out;

	return NO_ERROR;
err_out:
	puts("mmc erase failed\n");

	return mmc_return;
}

/*
 * Get alloc mmc_device_t
 */
mmc_device_t *mmc_get_new_dev(void)
{
	if (_mmc_dev_cnt >= MMC_MAX_DEVICE)
		return NULL;
	return &_mmc_dev[_mmc_dev_cnt++];
}

/*
 * Register device struct to block layer
 */
static int mmc_mmc_register(mmc_device_t *mdev, struct mmc *mmc, unsigned int partition)
{
	unsigned int block_size;
	unsigned int block_count;
	char name[16];

	if (mdev == NULL)
		return ERR_GENERIC;

	mdev->dev.private = mdev;
	mdev->mmc = mmc;
	mdev->block_size = mmc->rd_block_len;
	if (partition == MMC_PARTITION_MMC_USER) {
		strcpy(name, "mmc0");
		mdev->partition = MMC_PARTITION_MMC_USER;
		mdev->block_cnt = mmc->user_size / mdev->block_size;
	} else if (partition == MMC_PARTITION_SD_USER) {
		strcpy(name, "sd0");
		mdev->partition = MMC_PARTITION_SD_USER;
		mdev->block_cnt = mmc->capacity / mdev->block_size;
	} else if (partition == MMC_PARTITION_MMC_BOOT1) {
		strcpy(name, "mmc1");
		mdev->partition = MMC_PARTITION_MMC_BOOT1;
		mdev->block_cnt = mmc->boot_size * 128 * 1024 / mdev->block_size;
	} else if (partition == MMC_PARTITION_MMC_BOOT2) {
		strcpy(name, "mmc2");
		mdev->partition = MMC_PARTITION_MMC_BOOT2;
		mdev->block_cnt = mmc->boot_size * 128 * 1024 / mdev->block_size;
	} else if (partition == MMC_PARTITION_MMC_RPMB) {
		strcpy(name, "mmcrpmb");
		mdev->partition = MMC_PARTITION_MMC_RPMB;
		mdev->block_cnt = mmc->rpmb_size * 128 * 1024 / mdev->block_size;
	}

	block_size = mdev->block_size;
	block_count = mdev->block_cnt;
	bio_initialize_bdev(&mdev->dev,
			name,
			block_size,
			block_count,
			0,
			NULL,
			BIO_FLAGS_NONE);
	mdev->dev.new_read_native = mmc_bread;
	mdev->dev.new_write_native = mmc_bwrite;
	if (partition == MMC_PARTITION_MMC_BOOT1 ||
		partition == MMC_PARTITION_MMC_BOOT2 ||
		partition == MMC_PARTITION_MMC_RPMB) {
		mdev->dev.new_erase_native = mmc_berase_write;
		mdev->dev.erase_byte = mmc->rd_block_len;
	} else {
		mdev->dev.new_erase_native = mmc_berase;
		mdev->dev.erase_size = mmc->erase_grp_size;
	}

	mdev->dev.max_blkcnt_per_cmd = CONFIG_SYS_MMC_MAX_BLK_COUNT * block_size / USER_BLOCK_SIZE;

	bio_register_device(&mdev->dev);
	return NO_ERROR;
}

/*
 * Register device struct to block layer
 */
static void mmc_register_bdev(struct mmc *mmc)
{
	mmc_device_t *mdev;

	if (mmc_is_sd(mmc)) {
		mdev = mmc_get_new_dev();
		mmc_mmc_register(mdev, mmc, MMC_PARTITION_SD_USER);
	} else {
		if (mmc->boot_size) {
			mdev = mmc_get_new_dev();
			mmc_mmc_register(mdev, mmc, MMC_PARTITION_MMC_BOOT1);
			mdev = mmc_get_new_dev();
			mmc_mmc_register(mdev, mmc, MMC_PARTITION_MMC_BOOT2);
		}
		if (mmc->rpmb_size) {
			mdev = mmc_get_new_dev();
			mmc_mmc_register(mdev, mmc, MMC_PARTITION_MMC_RPMB);
		}
		if (mmc->user_size) {
			mdev = mmc_get_new_dev();
			mmc_mmc_register(mdev, mmc, MMC_PARTITION_MMC_USER);
		}
	}
}

/*
 * Initialize all mmc devices.
 */
void mmc_init(void)
{
	int i, err;
	struct mmc *mmc;

	for (i = 0; i < MMC_MAX_CHANNEL; i++) {
		mmc = &mmc_channel[i];
		err = mmc_board_init(mmc, i);
		if (err) {
			printf("MMC channel %d don't existed or not used\n", i);
			continue;
		}
		err = mmc_boot_init_and_identify_card(mmc);
		if (err) {
			if (mmc->exist == 0) {
				printf("MMC channel %d no card or error card\n", i);
				continue;
			}
			printf("MMC channel %d initialize failed\n", i);
			mmc->speed_mode = MMC_DENY_UHS;
			mmc_board_reinit(mmc);
			err = mmc_boot_init_and_identify_card(mmc);
			if (err != NO_ERROR) {
				printf("MMC channel %d reinitializing failed\n", mmc->channel);
				continue;
			} else 
				printf("MMC channel %d reinitializing success without SDR104\n", mmc->channel);
		}
		mmc_register_bdev(mmc);
		printf("MMC channel %d initialize success\n", i);
	}

	part_set_def_dev(DEV_MMC);
	mmc_test();
}

int mmc_reinit(struct bdev *dev)
{
	mmc_device_t *mdev = (mmc_device_t *)dev->private;
	struct mmc *mmc = (struct mmc *)mdev->mmc;
	int mmc_return = NO_ERROR;

	mmc_return = mmc_board_reinit(mmc);
	if (mmc_return != NO_ERROR) {
		printf("MMC channel %d don't existed\n", mmc->channel);
		return mmc_return;
	}

	mmc_return = mmc_boot_init_and_identify_card(mmc);
	if (mmc_return != NO_ERROR) {
		printf("MMC channel %d reinitializing failed\n", mmc->channel);
		return mmc_return;
	}

	return mmc_return;
}

/*
 * read/write/erase test function
 */
void mmc_test(void)
{
#ifdef MMC_TEST
	bdev_t *dev;
	u32 buffer[200000];
	u32 buffer2[1024];
	int i;
	u32 timeru, timerl;

	dev = bio_open("sd0");
	dev->new_read(dev, buffer, 0, 3);
	dev->new_read(dev, buffer2, 0, 3);
	for (i = 0; i < 128 * 3; i++) {
		if (i%4 == 0)
			printf("\n");
		printf("%08x ", buffer[i]);
		buffer[i] = i;
	}
	dev->new_write(dev, buffer, 1, 1);
	dev->new_read(dev, buffer, 0, 3);
	for (i = 0; i < 128 * 3; i++) {
		if (i%4 == 0)
			printf("\n");
		printf("%08x ", buffer[i]);
		buffer[i] = 0x0;
	}
	dev->new_erase(dev, 1, 1);
	dev->new_read(dev, buffer, 0, 3);
	for (i = 0; i < 128*3; i++) {
		if (i%4 == 0)
			printf("\n");
		printf("%08x ", buffer[i]);
	}
	printf("-------------------------------------------------------------------\n");
	dev->new_write(dev, buffer2, 0, 3);
	timeru = (*(volatile u32 *)(0x10040000 + 0x104));
	timerl = (*(volatile u32 *)(0x10040000 + 0x100));
	dev->new_read(dev, buffer, 0, 1000);
	timeru = (*(volatile u32 *)(0x10040000 + 0x104)) - timeru;
	timerl = (*(volatile u32 *)(0x10040000 + 0x100)) - timerl;
	printf("read diff time : %08x %08x\n", timeru, timerl);

	timeru = (*(volatile u32 *)(0x10040000 + 0x104));
	timerl = (*(volatile u32 *)(0x10040000 + 0x100));
	dev->new_write(dev, buffer, 0, 1000);
	timeru = (*(volatile u32 *)(0x10040000 + 0x104)) - timeru;
	timerl = (*(volatile u32 *)(0x10040000 + 0x100)) - timerl;
	printf("read diff time : %08x %08x\n", timeru, timerl);
#endif
}
