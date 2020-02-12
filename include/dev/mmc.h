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

#ifndef __MMC__

#define __MMC__

#include <lib/bio.h>
#include <err.h>

#define MMC_MAX_CHANNEL 3
#define MMC_MAX_DEVICE 16

/* Card status */
#define MMC_BOOT_CARD_STATUS(x)          ((x>>9) & 0x0F)
#define MMC_BOOT_TRAN_STATE              4
#define MMC_BOOT_PROG_STATE              7
#define MMC_BOOT_SWITCH_FUNC_ERR_FLAG    (1 << 7)

/* SD Memory Card bus commands */
#define CMD0_GO_IDLE_STATE               0
#define CMD1_SEND_OP_COND                1
#define CMD2_ALL_SEND_CID                2
#define CMD3_SEND_RELATIVE_ADDR          3
#define CMD4_SET_DSR                     4
#define CMD6_SWITCH_FUNC                 6
#define ACMD6_SET_BUS_WIDTH              6	/* SD card */
#define CMD7_SELECT_DESELECT_CARD        7
#define CMD8_SEND_EXT_CSD                8
#define CMD8_SEND_IF_COND                8	/* SD card */
#define CMD9_SEND_CSD                    9
#define CMD10_SEND_CID                   10
#define CMD11_SWITCH_VOLTAGE             11	/* SD card */
#define CMD12_STOP_TRANSMISSION          12
#define CMD13_SEND_STATUS                13
#define CMD15_GO_INACTIVE_STATUS         15
#define CMD16_SET_BLOCKLEN               16
#define CMD17_READ_SINGLE_BLOCK		 17
#define CMD18_READ_MULTIPLE_BLOCK	 18
#define CMD19_SEND_TUNING_BLOCK		 19
#define CMD23_SET_BLOCK_COUNT            23
#define CMD24_WRITE_SINGLE_BLOCK         24
#define CMD25_WRITE_MULTIPLE_BLOCK       25
#define CMD28_SET_WRITE_PROTECT          28
#define CMD29_CLEAR_WRITE_PROTECT        29
#define CMD31_SEND_WRITE_PROT_TYPE       31
#define CMD32_ERASE_WR_BLK_START         32
#define CMD33_ERASE_WR_BLK_END           33
#define CMD35_ERASE_GROUP_START          35
#define CMD36_ERASE_GROUP_END            36
#define CMD38_ERASE                      38
#define ACMD41_SEND_OP_COND              41	/* SD card */
#define ACMD51_SEND_SCR                  51	/* SD card */
#define CMD55_APP_CMD                    55	/* SD card */

/*
 * Swtich command mode
 */
#define MMC_SWITCH_MODE_CMD_SET		0x00 /* Change the command set */
#define MMC_SWITCH_MODE_SET_BITS	0x01 /* Set bits in EXT_CSD byte
					      * addressed by index which are
					      * 1 in value field
					      */
#define MMC_SWITCH_MODE_CLEAR_BITS	0x02 /* Clear bits in EXT_CSD byte
					      *	addressed by index, which are
					      *	1 in value field
					      */
#define MMC_SWITCH_MODE_WRITE_BYTE	0x03 /* Set target byte to value */
#define SD_SWITCH_CHECK		0
#define SD_SWITCH_SWITCH	1

/*
 * EXT_CSD field definitions
 */
#define EXT_CSD_SEC_ER_EN		(1 << 0)
#define EXT_CSD_SEC_BD_BLK_EN		(1 << 2)
#define EXT_CSD_SEC_GB_CL_EN		(1 << 4)
#define EXT_CSD_SEC_SANITIZE		(1 << 6)  /* v4.5 only */

#define EXT_CSD_CMD_SET_NORMAL		(1 << 0)
#define EXT_CSD_CMD_SET_SECURE		(1 << 1)
#define EXT_CSD_CMD_SET_CPSECURE	(1 << 2)

#define EXT_CSD_CARD_TYPE_26		(1 << 0)/* Card can run at 26MHz */
#define EXT_CSD_CARD_TYPE_52		(1 << 1)/* Card can run at 52MHz */
#define EXT_CSD_CARD_TYPE_52_DDR_18_30	(1 << 2)/* Card can run at 52MHz DDR 1.8V or 3V */
#define EXT_CSD_CARD_TYPE_52_DDR_12	(1 << 3)/* Card can run at 52MHz DDR 1.2V */

#define EXT_CSD_NO_POWER_NOTIFICATION	0
#define EXT_CSD_POWER_ON		1
#define EXT_CSD_POWER_OFF_SHORT		2
#define EXT_CSD_POWER_OFF_LONG		3

/*
 * EXT_CSD fields
 */
#define EXT_CSD_POWER_OFF_NOTIFICATION	34	/* R/W */
#define EXT_CSD_PARTITION_SETTING	155	/* R/W */
#define EXT_CSD_PARTITIONS_ATTRIBUTE	156	/* R/W */
#define EXT_CSD_PARTITIONING_SUPPORT	160	/* RO */
#define EXT_CSD_RST_N_FUNCTION		162	/* R/W */
#define EXT_CSD_BKOPS_EN		163	/* R/W & R/W/E */
#define EXT_CSD_WR_REL_PARAM		166	/* R */
#define EXT_CSD_WR_REL_SET		167	/* R/W */
#define EXT_CSD_RPMB_MULT		168	/* RO */
#define EXT_CSD_USER_WP			171	/* RO */
#define EXT_CSD_BOOT_WP			173	/* RO */
#define EXT_CSD_BOOT_WP_STATUS		174	/* RO */
#define EXT_CSD_ERASE_GROUP_DEF		175	/* R/W */
#define EXT_CSD_BOOT_BUS_WIDTH		177
#define EXT_CSD_PART_CONF		179	/* R/W */
#define EXT_CSD_BUS_WIDTH		183	/* R/W */
#define EXT_CSD_HS_TIMING		185	/* R/W */
#define EXT_CSD_REV			192	/* RO */
#define EXT_CSD_CARD_TYPE		196	/* RO */
#define EXT_CSD_SEC_CNT			212	/* RO, 4 bytes */
#define EXT_CSD_HC_WP_GRP_SIZE		221	/* RO */
#define EXT_CSD_HC_ERASE_GRP_SIZE	224	/* RO */
#define EXT_CSD_BOOT_MULT		226	/* RO */
#define EXT_CSD_SEC_FEATURE_SUPPORT	231	/* RO */
#define EXT_CSD_BKOPS_SUPPORT		502	/* RO */

/*
 * EXT_CSD[173] BOOT_WP
 */
#define EXT_CSD_USER_WP_B_PWR_WP_DIS    (0x40)
#define EXT_CSD_USER_WP_B_PERM_WP_DIS   (0x10)
#define EXT_CSD_USER_WP_B_PERM_WP_EN    (0x04)
#define EXT_CSD_USER_WP_B_PWR_WP_EN     (0x01)

/*
 * EXT_CSD[173] BOOT_WP
 */
#define EXT_CSD_BOOT_WP_B_PWR_WP_DIS    (0x40)
#define EXT_CSD_BOOT_WP_B_PERM_WP_DIS   (0x10)
#define EXT_CSD_BOOT_WP_B_PERM_WP_EN    (0x04)
#define EXT_CSD_BOOT_WP_B_PWR_WP_EN     (0x01)

/*
 * EXT_CSD[196] DEVICE_TYPE
 */
#define MMC_HS_52MHZ		(1 << 1)
#define MMC_HS_52MHZ_1_8V_3V_IO	(1 << 2)
#define MMC_HS_52MHZ_1_2V_IO	(1 << 3)
#define SD_DEFAULT_SDR12	(1 << 0)
#define SD_HS_SDR25		(1 << 1)
#define SD_UHS_SDR50		(1 << 2)
#define SD_UHS_SDR104		(1 << 3)
/* Block size maximum */
#define MMC_MAX_BLOCK_LEN	512

#define NORMAL_ERASE		0x00000000
#define SECURE_ERASE		0x80000000
/* Response types */
#define MMC_RSP_PRESENT (1 << 0)
#define MMC_RSP_136	(1 << 1)		/* 136 bit response */
#define MMC_RSP_CRC	(1 << 2)		/* expect valid crc */
#define MMC_RSP_BUSY	(1 << 3)		/* card may send busy */
#define MMC_RSP_OPCODE	(1 << 4)		/* response contains opcode */

#define MMC_BOOT_RESP_NONE                0
#define MMC_BOOT_RESP_R1		(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_BOOT_RESP_R1B		(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE| \
					MMC_RSP_BUSY)
#define MMC_BOOT_RESP_R2		(MMC_RSP_PRESENT|MMC_RSP_136|MMC_RSP_CRC)
#define MMC_BOOT_RESP_R3		(MMC_RSP_PRESENT)
#define MMC_BOOT_RESP_R4		(MMC_RSP_PRESENT)
#define MMC_BOOT_RESP_R5		(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_BOOT_RESP_R6		(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_BOOT_RESP_R7		(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)

#define IS_RESP_136_BITS(x)              (x & MMC_BOOT_RESP_R2)
#define CHECK_FOR_BUSY_AT_RESP(x)

/* OCR Register */
#define MMC_BOOT_OCR_17_19                (1 << 7)
#define MMC_BOOT_OCR_27_36                (0x1FF << 15)
#define MMC_BOOT_OCR_SEC_MODE             (2 << 29)
#define MMC_BOOT_OCR_BUSY                 (1 << 31)

/* Card Status bits (R1 register) */
#define MMC_BOOT_R1_STATUS_MASK           (~0x0206BF7F)
#define MMC_BOOT_R1_AKE_SEQ_ERROR         (1 << 3)
#define MMC_BOOT_R1_APP_CMD               (1 << 5)
#define MMC_BOOT_R1_RDY_FOR_DATA          (1 << 8)
#define MMC_BOOT_R1_CURR_STATE		  (0xf << 9)
#define MMC_BOOT_R1_CURR_STATE_IDLE       (0 << 9)
#define MMC_BOOT_R1_CURR_STATE_RDY        (1 << 9)
#define MMC_BOOT_R1_CURR_STATE_IDENT      (2 << 9)
#define MMC_BOOT_R1_CURR_STATE_STBY       (3 << 9)
#define MMC_BOOT_R1_CURR_STATE_TRAN       (4 << 9)
#define MMC_BOOT_R1_CURR_STATE_DATA       (5 << 9)
#define MMC_BOOT_R1_CURR_STATE_RCV        (6 << 9)
#define MMC_BOOT_R1_CURR_STATE_PRG        (7 << 9)
#define MMC_BOOT_R1_CURR_STATE_DIS        (8 << 9)
#define MMC_BOOT_R1_ERASE_RESET           (1 << 13)
#define MMC_BOOT_R1_CARD_ECC_DISABLED     (1 << 14)
#define MMC_BOOT_R1_WP_ERASE_SKIP         (1 << 15)
#define MMC_BOOT_R1_ERROR                 (1 << 19)
#define MMC_BOOT_R1_CC_ERROR              (1 << 20)
#define MMC_BOOT_R1_CARD_ECC_FAILED       (1 << 21)
#define MMC_BOOT_R1_ILLEGAL_CMD           (1 << 22)
#define MMC_BOOT_R1_COM_CRC_ERR           (1 << 23)
#define MMC_BOOT_R1_LOCK_UNLOCK_FAIL      (1 << 24)
#define MMC_BOOT_R1_CARD_IS_LOCKED        (1 << 25)
#define MMC_BOOT_R1_WP_VIOLATION          (1 << 26)
#define MMC_BOOT_R1_ERASE_PARAM           (1 << 27)
#define MMC_BOOT_R1_ERASE_SEQ_ERR         (1 << 28)
#define MMC_BOOT_R1_BLOCK_LEN_ERR         (1 << 29)
#define MMC_BOOT_R1_ADDR_ERR              (1 << 30)
#define MMC_BOOT_R1_OUT_OF_RANGE          (1 << 31)


#define MMC_BOOT_MAX_COMMAND_RETRY    5
#define MMC_BOOT_RD_BLOCK_LEN         512
#define MMC_BOOT_WR_BLOCK_LEN         512


/* For SD */
#define MMC_BOOT_SD_HC_VOLT_SUPPLIED      0x000001AA
#define MMC_BOOT_SD_NEG_OCR               0x00FF8000
#define MMC_BOOT_SD_HC_HCS                0x40000000
#define MMC_BOOT_SD_DEV_READY             0x80000000
#define MMC_BOOT_SD_SWITCH_HS             0x80FFFFF1
#define MMC_BOOT_SD_OCR_XPC		  0x10000000
#define MMC_BOOT_SD_OCR_S18R		  0x01000000


#define MMC_SD_SWITCH_FUNCTION_GROUP1	  376

struct mmc_data {
	unsigned int flags;
#define MMC_DATA_READ	0
#define MMC_DATA_WRITE	1
	unsigned char *dest;
	const unsigned char *src;
	unsigned int block_cnt;
	unsigned int block_size;

};
struct mmc_cmd {
	unsigned int cmdidx;
	unsigned int argument;
	unsigned int resp_type;
	unsigned int retries;

	unsigned int response[4];
	struct mmc_data *data;
};
struct mmc {
	void *host;
	unsigned int channel;
	unsigned int ocr;
	unsigned int type;
#define MMC_BOOT_TYPE_STD_SD            0
#define MMC_BOOT_TYPE_SDHC              1
#define MMC_BOOT_TYPE_SDIO              2
#define MMC_BOOT_TYPE_MMCHC             3
#define MMC_BOOT_TYPE_STD_MMC           4
	unsigned int cid;
	unsigned int version;
	unsigned int csd[4];
	unsigned int scr[2];
	unsigned int rca;
	unsigned int cmd_retry;
	unsigned int status;
#define MMC_BOOT_STATUS_INACTIVE	0
#define MMC_BOOT_STATUS_ACTIVE		1
	unsigned int clock;
#define MMC_CLK_26MHZ			26000000
#define MMC_CLK_52MHZ			52000000
#define SD_CLK_25MHZ			25000000
#define SD_CLK_50MHZ			50000000
#define SD_CLK_100MHZ			100000000
#define SD_CLK_208MHZ			208000000
	unsigned int bus_width;
#define MMC_BOOT_BUS_1BIT		0
#define MMC_BOOT_BUS_4BIT		1
#define MMC_BOOT_BUS_8BIT		2
	unsigned int bus_mode;
#define MMC_BOOT_BUS_SDR		0
#define MMC_BOOT_BUS_DDR		1
	u64 capacity;
	u64 user_size;
	u64 boot_size;
	u64 rpmb_size;
	u32 erase_grp_size;
	u32 rd_block_len;
	u32 wr_block_len;
	u32 transpeed;
	u32 maxseg_size;
	u32 card_caps;
	u32 speed_mode;
#define MMC_TRY_UHS			0
#define MMC_SET_UHS			1
#define MMC_DENY_UHS			2
	u32 sec_feature_support;
	u32 csd_version;
	u32 exist;
	struct mmc_cmd abort_cmd;

	int (*send_command)(struct mmc *mmc, struct mmc_cmd *cmd);
	int (*set_ios)(struct mmc *mmc);
	void (*reset)(struct mmc *mmc);
	void (*host_init)(struct mmc *mmc);
	int (*change_clock)(struct mmc *mmc, unsigned int target_clock);
	void (*change_bus_width)(struct mmc *mmc);
	void (*change_mode)(struct mmc *mmc);
	int (*voltage_switch)(struct mmc *mmc);
	int (*change_clksel)(struct mmc *mmc, unsigned int pass_index);
};
#define MMC_CHANNEL_EMMC		0
#define MMC_CHANNEL_SDIO		1
#define MMC_CHANNEL_SD			2

typedef struct mmc_device_s mmc_device_t;

#define MMC_PARTITION_SD_USER		0
#define MMC_PARTITION_MMC_USER		0
#define MMC_PARTITION_MMC_BOOT1		1
#define MMC_PARTITION_MMC_BOOT2		2
#define MMC_PARTITION_MMC_RPMB		3
struct mmc_device_s {
	bdev_t dev;
	struct mmc *mmc;
	unsigned int partition;
	u32 block_size;
	u32 block_cnt;
};
void mmc_init(unsigned int channel);
int mmc_reinit(struct bdev *);
int mmc_board_init(struct mmc *mmc, unsigned int channel);
int mmc_board_reinit(struct mmc *mmc);
void mmc_power_set(unsigned int channel, unsigned int enable);

void mmc_test(void);
#endif
