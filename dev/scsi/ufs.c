/*
 * (C) Copyright 2017 SAMSUNG Electronics
 * Kiwoong Kim <kwmad.kim@samsung.com>
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 *
 */


#include <reg.h>
#include <stdlib.h>
#include <dev/ufs.h>
#include <dev/ufs_provision.h>
#include <platform/delay.h>
#include <platform/exynos9830.h>

#define	SCSI_MAX_INITIATOR	1
#define	SCSI_MAX_DEVICE		8

static int send_uic_cmd(struct ufs_host *ufs);
static int ufs_bootlun_enable(int enable);

/*
	Multiple UFS host : cmd_scsi should be changed
	Now only one host supported
*/
static struct ufs_host *_ufs[SCSI_MAX_INITIATOR] = { NULL, };
static int _ufs_curr_host = 0;


/* Array index of ufs_query_params */
typedef enum {
	FLAG_W_FDEVICEINIT = 1,
	FLAG_R_FDEVICEINIT,

	DESC_R_DEVICE_DESC,
	DESC_W_CONFIG_DESC,
	DESC_R_CONFIG_DESC,
	DESC_R_UNIT_DESC,
	DESC_R_GEOMETRY_DESC,

	ATTR_W_BOOTLUNEN,
	ATTR_R_BOOTLUNEN,
	ATTR_W_REFCLKFREQ,
	ATTR_R_REFCLKFREQ,
} query_index;

/*	Query Function		OPCODE				IDN				INDEX	SELECTOR	*/
static u8 ufs_query_params[][5] = {
	/* The first index is not used for query operation */
	{0			,0				,0				,0	,0},

	{UFS_STD_WRITE_REQ	,UPIU_QUERY_OPCODE_SET_FLAG	,UPIU_FLAG_ID_DEVICEINIT	,0	,0},
	{UFS_STD_READ_REQ	,UPIU_QUERY_OPCODE_READ_FLAG	,UPIU_FLAG_ID_DEVICEINIT	,0	,0},

	/*
	 * INDEX of Configuration Descriptor means target LUN ranges to be configured,
	 * but this driver always set this to zero because it is supposed to configure less or equal than eight LUs.
	 */
	{UFS_STD_READ_REQ	,UPIU_QUERY_OPCODE_READ_DESC	,UPIU_DESC_ID_DEVICE		,0	,0},
	{UFS_STD_WRITE_REQ	,UPIU_QUERY_OPCODE_WRITE_DESC	,UPIU_DESC_ID_CONFIGURATION	,0	,0},
	{UFS_STD_READ_REQ	,UPIU_QUERY_OPCODE_READ_DESC	,UPIU_DESC_ID_CONFIGURATION	,0	,0},
	{UFS_STD_READ_REQ	,UPIU_QUERY_OPCODE_READ_DESC	,UPIU_DESC_ID_UNIT		,0	,0},
	/*
	 * INDEX of Unit Descriptor means target LUN, so this driver will override the value.
	 */
	{UFS_STD_READ_REQ	,UPIU_QUERY_OPCODE_READ_DESC	,UPIU_DESC_ID_GEOMETRY		,0	,0},

	{UFS_STD_WRITE_REQ	,UPIU_QUERY_OPCODE_WRITE_ATTR	,UPIU_ATTR_ID_BOOTLUNEN		,0	,0},
	{UFS_STD_READ_REQ	,UPIU_QUERY_OPCODE_READ_ATTR	,UPIU_ATTR_ID_BOOTLUNEN		,0	,0},
	{UFS_STD_WRITE_REQ	,UPIU_QUERY_OPCODE_WRITE_ATTR	,UPIU_ATTR_ID_REFCLKFREQ	,0	,0},
	{UFS_STD_READ_REQ	,UPIU_QUERY_OPCODE_READ_ATTR	,UPIU_ATTR_ID_REFCLKFREQ	,0	,0},
	{},
};


/* UFS user command definition */
#if defined(WITH_LIB_CONSOLE)

#include <lib/console.h>

static int cmd_ufs(int argc, const cmd_args *argv)
{
    int rc = 0;

    if (argc < 2) {
notenoughargs:
        printf("not enough arguments:\n");
usage:
        return -1;
    }

    if (!strcmp(argv[1].str, "boot")) {
        if (argc < 3) goto notenoughargs;

	return ufs_bootlun_enable((int) argv[2].u);
    } else {
        printf("unrecognized subcommand\n");
        goto usage;
    }

    return rc;
}

STATIC_COMMAND_START
STATIC_COMMAND("ufs", "UFS commands", &cmd_ufs)
STATIC_COMMAND_END(ufs);

#endif

/* Containers that have a block device */
static scsi_device_t *ufs_dev[SCSI_MAX_INITIATOR];
static scsi_device_t ufs_dev_ssu;
static scsi_device_t ufs_dev_rpmb;
static struct list_node ufs_lu_list = LIST_INITIAL_VALUE(ufs_lu_list);



/*********************************************************************************
 * forward declerations
 */


/* Helper for UFS CAL */

/*
 * UFS CAL HELPER
 *
 * This driver configures all the UIC by using these functions.
 * The functions exists in UFS CAL.
 */
static inline int ufs_init_cal(struct ufs_host *ufs, int idx)
{
	int ret = 0;

	ufs->cal_param->host = (void *)ufs;

	if (CONFIG_UFS_BOARD_TYPE)
		ufs->cal_param->board = BRD_UNIV;
	else
		ufs->cal_param->board = BRD_SMDK;

	ufs->cal_param->evt_ver = (readl(0x10000010) >> 20) & 0xf;
	printf("ufs->cal_param->evt_ver is EVT%d!!!\n", ufs->cal_param->evt_ver);

	ret = ufs_cal_init(ufs->cal_param, idx);
	if (ret != UFS_CAL_NO_ERROR) {
		printf("ufs_init_cal failed with %d!!!\n", ret);
		return ERR_GENERIC;
	}

	return NO_ERROR;
}

static inline int ufs_pre_link(struct ufs_host *ufs, u8 lane)
{
	int ret = 0;
	struct ufs_cal_param *p = ufs->cal_param;

	p->mclk_rate = ufs->mclk_rate;
	p->available_lane = lane;
	p->tbl = HOST_EMBD;

	ret = ufs_cal_pre_link(p);
	if (ret != UFS_CAL_NO_ERROR) {
		printf("ufs_pre_link failed with %d!!!\n", ret);
		return ERR_GENERIC;
	}

	return NO_ERROR;
}

static inline int ufs_post_link(struct ufs_host *ufs)
{
	int ret = 0;

	ret = ufs_cal_post_link(ufs->cal_param);
	if (ret != UFS_CAL_NO_ERROR) {
		printf("ufs_post_link failed with %d!!!\n", ret);
		return ERR_GENERIC;
	}

	return NO_ERROR;
}

static inline int ufs_pre_gear_change(struct ufs_host *ufs,
				struct uic_pwr_mode *pmd)
{
	struct ufs_cal_param *p = ufs->cal_param;
	int ret = 0;

	p->pmd = pmd;
	ret = ufs_cal_pre_pmc(p);
	if (ret != UFS_CAL_NO_ERROR) {
		printf("ufs_pre_gear_change failed with %d!!!\n", ret);
		return ERR_GENERIC;
	}

	return NO_ERROR;
}

static inline int ufs_post_gear_change(struct ufs_host *ufs)
{
	int ret = 0;

	ret = ufs_cal_post_pmc(ufs->cal_param);
	if (ret != UFS_CAL_NO_ERROR) {
		printf("ufs_post_gear_change failed with %d!!!\n", ret);
		return ERR_GENERIC;
	}

	return NO_ERROR;
}

/*
 * UFS CAL ADAPTOR
 *
 * This is called by UFS CAL in each project directory,
 * thus these are declared as global.
 */
void ufs_lld_dme_set(void *h, u32 addr, u32 val)
{
	struct ufs_host *ufs = (struct ufs_host *) h;
	struct ufs_uic_cmd cmd = { UIC_CMD_DME_SET, 0, 0, 0};

	cmd.uiccmdarg1 = addr;
	cmd.uiccmdarg3 = val;
	ufs->uic_cmd = &cmd;
	send_uic_cmd(ufs);
}

void ufs_lld_dme_get(void *h, u32 addr, u32 *val)
{
	struct ufs_host *ufs = (struct ufs_host *) h;
	struct ufs_uic_cmd cmd = { UIC_CMD_DME_GET, 0, 0, 0};

	cmd.uiccmdarg1 = addr;
	ufs->uic_cmd = &cmd;
	send_uic_cmd(ufs);
	*val = cmd.uiccmdarg3;
}

void ufs_lld_dme_peer_set(void *h, u32 addr, u32 val)
{
	struct ufs_host *ufs = (struct ufs_host *) h;
	struct ufs_uic_cmd cmd = { UIC_CMD_DME_PEER_SET, 0, 0, 0};

	cmd.uiccmdarg1 = addr;
	cmd.uiccmdarg3 = val;
	ufs->uic_cmd = &cmd;
	send_uic_cmd(ufs);
}

void ufs_lld_pma_write(void *h, u32 val, u32 addr)
{
	struct ufs_host *ufs = (struct ufs_host *) h;

	writel(val, ufs->phy_pma + addr);
}

u32 ufs_lld_pma_read(void *h, u32 addr)
{
	struct ufs_host *ufs = (struct ufs_host *) h;

	return readl(ufs->phy_pma + addr);
}

void ufs_lld_unipro_write(void *h, u32 val, u32 addr)
{
	struct ufs_host *ufs = (struct ufs_host *) h;

	writel(val, ufs->unipro_addr + addr);
}

void ufs_lld_udelay(u32 val)
{
	u_delay(val);
}

void ufs_lld_usleep_delay(u32 min, u32 max)
{
	u_delay(max);
}

unsigned long ufs_lld_get_time_count(unsigned long offset)
{
	return offset;
}

unsigned long ufs_lld_calc_timeout(const unsigned int ms)
{
	return 1000 * ms;
}


/*
 * INTERNAL CORE FUNCTIONS
 */
struct ufs_host *get_cur_ufs_host(void)
{
	return _ufs[_ufs_curr_host];
}

static void __utp_map_sg(struct ufs_host *ufs)
{
	u32 i, len, sg_segments;

	len = ufs->scsi_cmd->datalen;

	if (len) {
		sg_segments = (len + UFS_SG_BLOCK_SIZE - 1) / UFS_SG_BLOCK_SIZE;
		for (i = 0; i < sg_segments; i++) {
			ufs->cmd_desc_addr->prd_table[i].size =
			    (u32) UFS_SG_BLOCK_SIZE - 1;
			ufs->cmd_desc_addr->prd_table[i].base_addr =
			    (u32)(((u64) (ufs->scsi_cmd->buf) + i * UFS_SG_BLOCK_SIZE) & (((u64)1 << UFS_BIT_LEN_OF_DWORD) - 1));
			ufs->cmd_desc_addr->prd_table[i].upper_addr =
			    (u32)(((u64) (ufs->scsi_cmd->buf) + i * UFS_SG_BLOCK_SIZE) >> UFS_BIT_LEN_OF_DWORD);
		}
	}
}

static u32 __utp_cmd_get_dir(scm *pscm)
{
	u32 data_direction;

	if (pscm->datalen) {
		switch (pscm->cdb[0]) {
		case SCSI_OP_UNMAP:
		case SCSI_OP_FORMAT_UNIT:
		case SCSI_OP_WRITE_10:
		case SCSI_OP_WRITE_BUFFER:
		case SCSI_OP_SECU_PROT_OUT:
		case SCSI_OP_START_STOP_UNIT:
			data_direction = UTP_HOST_TO_DEVICE;
			break;
		default:
			data_direction = UTP_DEVICE_TO_HOST;
			break;
		}
	} else
		data_direction = UTP_NO_DATA_TRANSFER;

	return data_direction;
}

static u32 __utp_cmd_get_flags(scm *pscm)
{
	u32 upiu_flags;

	if (pscm->datalen) {
		switch (pscm->cdb[0]) {
		case SCSI_OP_UNMAP:
		case SCSI_OP_FORMAT_UNIT:
		case SCSI_OP_WRITE_10:
		case SCSI_OP_WRITE_BUFFER:
		case SCSI_OP_SECU_PROT_OUT:
			upiu_flags = UPIU_CMD_FLAGS_WRITE;
			break;
		case SCSI_OP_START_STOP_UNIT:
			upiu_flags = UPIU_CMD_FLAGS_NONE;
			break;
		default:
			upiu_flags = UPIU_CMD_FLAGS_READ;
			break;
		}
	} else
		upiu_flags = UPIU_CMD_FLAGS_NONE;

	return upiu_flags;
}

static void __utp_write_cmd_ucd(struct ufs_host *ufs)
{
	u32 datalen;

	struct ufs_upiu *cmd_ptr = &ufs->cmd_desc_addr->command_upiu;
	struct ufs_upiu_header *hdr = &cmd_ptr->header;
	u8 *tsf = cmd_ptr->tsf;

	u32 upiu_flags;

	upiu_flags = __utp_cmd_get_flags(ufs->scsi_cmd);

	/* header */
	hdr->type = UPIU_TRANSACTION_COMMAND;
	hdr->flags = upiu_flags;
	hdr->lun = ufs->lun;
	hdr->tag = 0;					/* Only tag #0 is used */

	/* Transaction Specific Fields */
	datalen = cpu_to_be32(ufs->scsi_cmd->datalen);
	memcpy(&tsf[0], &datalen, sizeof(u32));
	memcpy(&tsf[4], ufs->scsi_cmd->cdb, MAX_CDB_SIZE);
}

static int __utp_write_query_ucd(struct ufs_host *ufs, query_index qry)
{
	int r = 0;

	struct ufs_upiu *cmd_ptr = &ufs->cmd_desc_addr->command_upiu;
	struct ufs_upiu_header *hdr = &cmd_ptr->header;
	u8 *tsf = cmd_ptr->tsf;
	u16 data_len = MIN(0x90, UPIU_DATA_SIZE);		//
	u32 info;

	/* header */
	hdr->type = UPIU_TRANSACTION_QUERY_REQ;
	hdr->flags = UPIU_CMD_FLAGS_NONE;
	hdr->tag = 0;					/* Only tag #0 is used */
	hdr->function = ufs_query_params[qry][0];	/* Query Function */
	if (hdr->function == UPIU_QUERY_OPCODE_WRITE_DESC)
		hdr->datalength = cpu_to_be16(data_len);

	/* Transaction Specific Fields */
	tsf[0] = ufs_query_params[qry][1];		/* OPCODE */
	tsf[1] = ufs_query_params[qry][2];		/* IDN */
	tsf[2] = ufs_query_params[qry][3];		/* INDEX */
	tsf[3] = ufs_query_params[qry][4];		/* SELECTOR */
	if (hdr->function == UPIU_QUERY_OPCODE_WRITE_DESC) {
		info = cpu_to_be16(data_len);
		memcpy(&tsf[6], &info, sizeof(u16));
	} else if (hdr->function == UPIU_QUERY_OPCODE_READ_DESC) {
		info = cpu_to_be16(UPIU_DATA_SIZE);
		memcpy(&tsf[6], &info, sizeof(u16));
	}
	if (tsf[0] == UPIU_QUERY_OPCODE_WRITE_ATTR) {
		info = cpu_to_be32(ufs->attributes.arry[tsf[1]]);
		memcpy(&tsf[8], &info, sizeof(u32));
	} else if (tsf[0] == UPIU_QUERY_OPCODE_SET_FLAG)
		tsf[11] = (u8)ufs->flags.arry[tsf[1]];

	/* Data */
	if (tsf[0] == UPIU_QUERY_OPCODE_WRITE_DESC)
		memcpy(cmd_ptr->data, &ufs->config_desc, data_len);

	return r;
}

static int __utp_write_utrd(struct ufs_host *ufs, u32 type)
{
	int r = 0;

	struct ufs_utrd *utrd_ptr = ufs->utrd_addr;
	u32 len = ufs->scsi_cmd->datalen;
	u16 sg_segments = (u16)((len + UFS_SG_BLOCK_SIZE - 1) / UFS_SG_BLOCK_SIZE);

	u32 data_direction;

	switch (type) {
	case UPIU_TRANSACTION_COMMAND:
		data_direction = __utp_cmd_get_flags(ufs->scsi_cmd);

		utrd_ptr->dw[0] = (u32)(data_direction | UTP_SCSI_COMMAND | UTP_REQ_DESC_INT_CMD);
		utrd_ptr->dw[2] = (u32)(OCS_INVALID_COMMAND_STATUS);
		if (len) {
			utrd_ptr->prdt_len = sg_segments * sizeof(struct ufs_prdt);
			utrd_ptr->prdt_off = ALIGNED_UPIU_SIZE * 2;
		} else {
			utrd_ptr->prdt_len = 0;
			utrd_ptr->prdt_off = 0;
		}

		break;
	case UPIU_TRANSACTION_QUERY_REQ:
		utrd_ptr->dw[0] = UTP_REQ_DESC_INT_CMD;
		utrd_ptr->dw[2] = (u32)(OCS_INVALID_COMMAND_STATUS);

		break;
	case UPIU_TRANSACTION_NOP_OUT:
		break;
	default:
		r = -1;
		printf("UFS: type %02x is not supported\n", type);
		break;
	}

	return r;
}

static int __utp_write_cmd_all_descs(struct ufs_host *ufs)
{
	/* ucd */
	__utp_write_cmd_ucd(ufs);

	/* prdt */
	__utp_map_sg(ufs);

	/* utrd*/
	return __utp_write_utrd(ufs, UPIU_TRANSACTION_COMMAND);
}

static int __utp_write_query_all_descs(struct ufs_host *ufs, query_index qry)
{
	/* ucd */
	__utp_write_query_ucd(ufs, qry);

	/* utrd*/
	return __utp_write_utrd(ufs, UPIU_TRANSACTION_QUERY_REQ);
}

/********************************************************************************
 * ufs minimal interrupt handler
 */
static int handle_ufs_uic_int(struct ufs_host *ufs, u32 intr_stat)
{
	int ret = UFS_IN_PROGRESS;
	struct ufs_uic_cmd *uic_cmd = ufs->uic_cmd;

	/*
	 * Check completion
	 *
	 * In some cases, you need to check more.
	 */
	if (intr_stat & UIC_COMMAND_COMPL) {
		/* Link startup */
		if (uic_cmd->uiccmdr == UIC_CMD_DME_LINK_STARTUP) {
			ret = UFS_NO_ERROR;
		/* Gear change */
		} else if (uic_cmd->uiccmdr == UIC_CMD_DME_SET &&
				ufs->uic_cmd->uiccmdarg1 == (0x1571 << 16)) {
			if (intr_stat & UIC_POWER_MODE)
				ret = UFS_NO_ERROR;
		} else {
			ret = UFS_NO_ERROR;
		}
	}

	/*
	 * Expected error case
	 *
	 * In case of link startup, bogus uic error is raised for
	 * LINE-RESET.
	 */
	if (intr_stat & UIC_ERROR &&
			uic_cmd->uiccmdr != UIC_CMD_DME_LINK_STARTUP) {
		printf("UFS: UIC ERROR 0x%08x\n", intr_stat);
		ret = UFS_ERROR;
	}

	return ret;
}

static int handle_ufs_utp_int(struct ufs_host *ufs, u32 intr_stat)
{
	int ret = UFS_IN_PROGRESS;

	if (intr_stat & UTP_TRANSFER_REQ_COMPL) {
		if (!(readl(ufs->ioaddr + REG_UTP_TRANSFER_REQ_DOOR_BELL) & 1))
			ret = UFS_NO_ERROR;
	}

	return ret;
}

static int handle_ufs_int(struct ufs_host *ufs, int is_uic)
{
	u32 intr_stat;
	int ret;

	intr_stat = readl(ufs->ioaddr + REG_INTERRUPT_STATUS);

	if (is_uic)
		ret = handle_ufs_uic_int(ufs, intr_stat);
	else
		ret = handle_ufs_utp_int(ufs, intr_stat);

	/* Fatal error case */
	if (intr_stat & INT_FATAL_ERRORS) {
		printf("UFS: FATAL ERROR 0x%08x\n", intr_stat);
		ret = UFS_ERROR;
	}

	/* Terminate if success, error or progress, try again elsewhere */
	if (ret == UFS_IN_PROGRESS) {
		if (ufs->timeout--)
			u_delay(1);
		else {
			ret = UFS_TIMEOUT;
			printf("UFS: TIMEOUT\n");
		}
	}

#ifdef	SCSI_UFS_DEBUG
	if (ret != UFS_IN_PROGRESS)
		printf("UFS: INTERRUPT STATUS 0x%08x\n", intr_stat);
#endif

	return ret;
}

static int send_uic_cmd(struct ufs_host *ufs)
{
	int err = 0, error_code;

	writel(ufs->uic_cmd->uiccmdarg1, (ufs->ioaddr + REG_UIC_COMMAND_ARG_1));
	writel(ufs->uic_cmd->uiccmdarg2, (ufs->ioaddr + REG_UIC_COMMAND_ARG_2));
	writel(ufs->uic_cmd->uiccmdarg3, (ufs->ioaddr + REG_UIC_COMMAND_ARG_3));
	writel(ufs->uic_cmd->uiccmdr, (ufs->ioaddr + REG_UIC_COMMAND));

	ufs->timeout = ufs->uic_cmd_timeout;	/* usec unit */
	while (UFS_IN_PROGRESS == (err = handle_ufs_int(ufs, 1)))
		;
	writel(readl(ufs->ioaddr + REG_INTERRUPT_STATUS),
			ufs->ioaddr + REG_INTERRUPT_STATUS);

	error_code = readl(ufs->ioaddr + REG_UIC_COMMAND_ARG_2);
	if (err || error_code) {
		printf("UFS(%d) UIC command error!\n\t%08x %08x %08x %08x\n\t%08x %08x %08x %08x\n",
		       ufs->host_index, ufs->uic_cmd->uiccmdr, ufs->uic_cmd->uiccmdarg1,
		       ufs->uic_cmd->uiccmdarg2, ufs->uic_cmd->uiccmdarg3,
		       readl(ufs->ioaddr + REG_UIC_COMMAND),
		       readl(ufs->ioaddr + REG_UIC_COMMAND_ARG_1)
		       , readl(ufs->ioaddr + REG_UIC_COMMAND_ARG_2),
		       readl(ufs->ioaddr + REG_UIC_COMMAND_ARG_3));
	}
	if (ufs->uic_cmd->uiccmdr == UIC_CMD_DME_GET
	    || ufs->uic_cmd->uiccmdr == UIC_CMD_DME_PEER_GET) {
		ufs->uic_cmd->uiccmdarg3 = readl(ufs->ioaddr + REG_UIC_COMMAND_ARG_3);
#ifdef SCSI_UFS_DEBUG
		printf
		    ("UFS(%d) UIC_CMD_DME_(PEER)_GET command\n\t%08x %08x %08x %08x\n\t%08x %08x %08x %08x\n",
		     ufs->host_index, ufs->uic_cmd->uiccmdr, ufs->uic_cmd->uiccmdarg1,
		     ufs->uic_cmd->uiccmdarg2, ufs->uic_cmd->uiccmdarg3,
		     readl(ufs->ioaddr + REG_UIC_COMMAND),
		     readl(ufs->ioaddr + REG_UIC_COMMAND_ARG_1)
		     , readl(ufs->ioaddr + REG_UIC_COMMAND_ARG_2),
		     readl(ufs->ioaddr + REG_UIC_COMMAND_ARG_3));
#endif
	}

	return error_code | err;
}

static void __utp_send(struct ufs_host *ufs, u32 type)
{

	switch (type) {
	case UPIU_TRANSACTION_NOP_OUT:
	case UPIU_TRANSACTION_QUERY_REQ:
		writel(0x0, (ufs->vs_addr + VS_UTRL_NEXUS_TYPE));
		break;
	case UPIU_TRANSACTION_COMMAND:
		writel(0xFFFFFFFF, (ufs->vs_addr + VS_UTRL_NEXUS_TYPE));
	default:
		break;
	}

	switch (type & 0xff) {
	case UPIU_TRANSACTION_NOP_OUT:
	case UPIU_TRANSACTION_COMMAND:
	case UPIU_TRANSACTION_QUERY_REQ:
		writel(1, (ufs->ioaddr + REG_UTP_TRANSFER_REQ_DOOR_BELL));
		break;
	default:
		break;
	}
}

static int __utp_wait_for_response(struct ufs_host *ufs, u32 type)
{
	int err = UFS_IN_PROGRESS;

	ufs->timeout = ufs->ufs_cmd_timeout;

	/* FORMAT_UNIT should have longer timeout, 10 min */
	if (type == UPIU_TRANSACTION_COMMAND && ufs->scsi_cmd->cdb[0] == SCSI_OP_FORMAT_UNIT)
		ufs->timeout = 10 * 60 * 1000 * 1000;
	while (UFS_IN_PROGRESS == (err = handle_ufs_int(ufs, 0)))
		;
	writel(readl(ufs->ioaddr + REG_INTERRUPT_STATUS),
			ufs->ioaddr + REG_INTERRUPT_STATUS);

	/* Nexus configuration */
	if (type == UPIU_TRANSACTION_NOP_OUT || type == UPIU_TRANSACTION_QUERY_REQ)
		writel((readl(ufs->ioaddr + 0x140) | 0x01), (ufs->ioaddr + 0x140));

	return err;
}

static void __utp_init(struct ufs_host *ufs, u32 lun)
{
	ufs->lun = lun;
	ufs->scsi_cmd = NULL;
	memset(ufs->cmd_desc_addr, 0x00, sizeof(struct ufs_cmd_desc));
}

static void __utp_query_read_info(struct ufs_host *ufs, u8 idn)
{
	struct ufs_upiu *resp_ptr = &ufs->cmd_desc_addr->response_upiu;
	u8 *data = resp_ptr->data;
	void *dst = NULL;
	size_t len = 0;


	switch (idn) {
	case UPIU_DESC_ID_UNIT:
		dst = &ufs->unit_desc[data[2]];
		len = MIN(data[0], sizeof(struct ufs_unit_desc));
		break;
	case UPIU_DESC_ID_DEVICE:
		dst = &ufs->device_desc;
		len = MIN(data[0], sizeof(struct ufs_device_desc));
		break;
	case UPIU_DESC_ID_CONFIGURATION:
		dst = &ufs->config_desc;
		len = MIN(data[0], 0x90);	// TODO: 90??
		break;
	case UPIU_DESC_ID_GEOMETRY:
		dst = &ufs->geometry_desc;
		len = MIN(data[0], sizeof(struct ufs_geometry_desc));
		break;
	default:
		//
		break;
	}
	if (dst)
		memcpy(dst, resp_ptr->data, len);
}

static void __utp_query_get_data(struct ufs_host *ufs, query_index qry)
{
	struct ufs_upiu *resp_ptr = &ufs->cmd_desc_addr->response_upiu;
	u8 opcode = ufs_query_params[qry][1];

	u8 *tsf;
	u32 val;

	switch (opcode) {
	case UPIU_QUERY_OPCODE_READ_DESC:
		__utp_query_read_info(ufs, ufs_query_params[qry][2]);
		break;
	case UPIU_QUERY_OPCODE_READ_ATTR:
		tsf = resp_ptr->tsf;
		val = UPIU_HEADER_DWORD((u32) tsf[8], (u32) tsf[9], (u32) tsf[10], (u32) tsf[11]);
		ufs->attributes.arry[ufs_query_params[qry][2]] = val;
		break;
	case UPIU_QUERY_OPCODE_READ_FLAG:
		tsf = resp_ptr->tsf;
		val = (u32)tsf[11];
		ufs->flags.arry[ufs_query_params[qry][2]] = val;
		break;
	case UPIU_QUERY_OPCODE_SET_FLAG:
	case UPIU_QUERY_OPCODE_WRITE_DESC:
	case UPIU_QUERY_OPCODE_WRITE_ATTR:
		break;
	default:
		printf("UFS: %s: query with opcode 0x%02x is not supported\n", __func__, opcode);
		break;
	}
}

static int __utp_check_result(struct ufs_host *ufs)
{
	const char resp_msg[2][20] = { "Target Success", "Target Failure" };
	int r = 0;
	struct ufs_utrd *utrd_ptr = ufs->utrd_addr;
	struct ufs_upiu *resp_ptr = &ufs->cmd_desc_addr->response_upiu;
	struct ufs_upiu_header *hdr = &resp_ptr->header;
	scm * pscm = ufs->scsi_cmd;

	/* Update SCSI status. SCSI would handle it.. */
	if (pscm)
		pscm->status = hdr->status;

	/* OCS - Overall Command Status */
	if (utrd_ptr->dw[2] == OCS_SUCCESS)
		goto end;

	/* Show information */
	if (hdr->type == UPIU_TRANSACTION_RESPONSE) {

		/* Copy sense data */
		memcpy(ufs->scsi_cmd->sense_buf,
				&resp_ptr->data[2], 18);

		printf("SCSI cdb : %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
		       pscm->cdb[0], pscm->cdb[1], pscm->cdb[2], pscm->cdb[3],
		       pscm->cdb[4], pscm->cdb[5], pscm->cdb[6], pscm->cdb[7],
		       pscm->cdb[8], pscm->cdb[9]);
		printf("SCSI Response(%02x) \n", hdr->response);
		printf("SCSI Sense - RESPONSE CODE (%02x), SENSE KEY (%02x), ASC/ASCQ (%02x/%02x) \n",
						pscm->sense_buf[0],
						pscm->sense_buf[2],
						pscm->sense_buf[12],
						pscm->sense_buf[13]);
		printf("SCSI Sense raw data : %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
				pscm->sense_buf[0], pscm->sense_buf[1], pscm->sense_buf[2], pscm->sense_buf[3], pscm->sense_buf[4],
				pscm->sense_buf[5], pscm->sense_buf[6], pscm->sense_buf[7], pscm->sense_buf[8], pscm->sense_buf[9],
				pscm->sense_buf[10], pscm->sense_buf[11], pscm->sense_buf[12], pscm->sense_buf[13], pscm->sense_buf[14],
				pscm->sense_buf[15], pscm->sense_buf[16], pscm->sense_buf[17]);
	} else if (hdr->type == UPIU_TRANSACTION_QUERY_RSP)
		printf("QUERY Response(%02x) : ", hdr->response);

	/* Show Reponse */
	printf("UFS: %s for type 0x%02x\n", resp_msg[hdr->response], hdr->type);

	/* Return non-zero if target failure */
	if (hdr->response != 0)
		r = -1;
end:
	return r;
}

static void __utp_get_scsi_cxt(struct ufs_host *ufs, scm * pscm) {
	u32 lun;

	ufs->scsi_cmd = pscm;
	ufs->sense_buffer = pscm->sense_buf;
	ufs->sense_buflen = 64;	/* defined in include/scsi.h */
	if (pscm->sdev->lun == 0x44)
		lun = 0xC4;
	else if (pscm->sdev->lun == 0x50)
		lun = 0xD0;
	else
		lun = pscm->sdev->lun;

	ufs->lun = lun;
}

/*
 * This function does things on submitting COMMAND UPIU,
 * wait for and check RESPONSE UPIU. And then it reports
 * the result to the upper layer.
 */
static int ufs_utp_cmd_process(struct ufs_host *ufs, scm * pscm)
{
	int r;
	u32 type = UPIU_TRANSACTION_COMMAND;

	/* Init context */
	__utp_init(ufs, 0);

	/* Get context from SCSI */
	__utp_get_scsi_cxt(ufs, pscm);

	/* Describe all descriptors */
	r = __utp_write_cmd_all_descs(ufs);
	if (r != 0)
		goto end;

	/* Submit a command */
	__utp_send(ufs, type);

	/* Wait for response */
	r = __utp_wait_for_response(ufs, type);
	if (r != 0)
		goto end;

	/* Get and check result */
	r = __utp_check_result(ufs);
	if (r != 0)
		goto end;
end:
	return r;
}

/*
 * This function does things on submitting NOP OUT UPIU,
 * wait for and check NOP IN UPIU.
 */
static int ufs_utp_nop_process(struct ufs_host *ufs)
{
	int r;
	u32 type = UPIU_TRANSACTION_NOP_OUT;

	/*
	 * Init context. NOP OUT should be filled with zero
	 * except for Task Tag, but we only use tag #0.
	 * Therefore, therer is not necessary to write descriptors in here.
	 */
	__utp_init(ufs, 0);

	/* Submit a command */
	__utp_send(ufs, type);

	/* Wait for response */
	r = __utp_wait_for_response(ufs, type);
	if (r != 0)
		goto end;

	/* Get and check result */
	r = __utp_check_result(ufs);
	if (r != 0)
		goto end;

end:
	return r;
}

/*
 * This function does things on submitting QUERY REQUEST UPIU,
 * wait for and check QUERY RESPONSE UPIU.
 */
static int ufs_utp_query_process(struct ufs_host *ufs, query_index qry, u32 lun)
{
	int r;
	u32 type = UPIU_TRANSACTION_QUERY_REQ;

	/* Init context */
	__utp_init(ufs, lun);

	/* Describe all descriptors */
	r = __utp_write_query_all_descs(ufs, qry);
	if (r != 0)
		goto end;

	/* Submit a command */
	__utp_send(ufs, type);

	/* Wait for response */
	__utp_wait_for_response(ufs, type);
	if (r != 0)
		goto end;

	/* Get and check result */
	r = __utp_check_result(ufs);
	if (r != 0)
		goto end;

	__utp_query_get_data(ufs, qry);

end:
	return r;
}

/*
 * CALLBACK FUNCTION: scsi_exec
 *
 * This is called for SCSI stack to process some SCSI commands.
 * This is registered in SCSI stack when executing scsi_scan().
 */
static status_t scsi_exec(scm * pscm)
{
	struct ufs_host *ufs;

	if (!pscm)
		return ERR_NOT_VALID;

	ufs = get_cur_ufs_host();
#ifdef	SCSI_UFS_DEBUG
	print_ufs_upiu(ufs, UFS_DEBUG_UPIU);
#endif

	return ufs_utp_cmd_process(ufs, pscm);
}

static scsi_device_t *scsi_get_ssu_sdev(void)
{
	return (struct scsi_device_s *)&ufs_dev_ssu;
}

static int ufs_bootlun_enable(int enable)
{
	struct ufs_host *ufs = get_cur_ufs_host();
	query_index qry = ATTR_W_BOOTLUNEN;

	ufs->attributes.arry[ufs_query_params[qry][2]] = enable;
	return ufs_utp_query_process(ufs, qry, 0);
}

static int ufs_edit_config_desc(u32 lun, u32 enable, u32 bootlun, u32 writeprotect, u32 type, u32 capacity_in_mb, int *flag_p)
{
	int i, luns = 0, boot = 0, flag = *flag_p;
	u32 sum = 0;
	struct ufs_host *ufs = get_cur_ufs_host();

	unsigned long num_alloc_units;
	unsigned long target_in_byte;
	unsigned long alloc_unit_in_byte;
	unsigned int enhanced_type_one_ratio;

	if (!ufs || (lun > 7))
		return 1;

	ufs->lun = 0;
	if (ufs_utp_query_process(ufs, DESC_R_GEOMETRY_DESC, 0))
		return 1;

	target_in_byte = capacity_in_mb * 1024 * 1024;
	alloc_unit_in_byte = ufs->geometry_desc.bAllocationUnitSize *
			be32_to_cpu(ufs->geometry_desc.dSegmentSize) * 512;
	enhanced_type_one_ratio = be16_to_cpu(ufs->geometry_desc.wEnhanced1CapAdjFac) / 256;

	if (enable) {
		/*
		 * We only assume non-zero input capacity
		 * which is alinged to the size of Allocation size in bytes of
		 * the attached UFS device.
		 */
		if (target_in_byte % alloc_unit_in_byte) {
			printf("UFS: input capacity is not aligned to the size of Allocation size: %ld != %ld\n",
					target_in_byte, alloc_unit_in_byte);
			return 2;
		}

		/*
		 * The current way assumes zero input capacity case
		 * should happen once.
		 */
		if (flag) {
			printf("UFS: zero input capacity cases occurs twice.\n");
			return 2;
		}

		ufs->config_desc.unit[lun].bLUEnable = 0x1;
		ufs->config_desc.unit[lun].bBootLunID = bootlun;
		ufs->config_desc.unit[lun].bLUWriteProtect = writeprotect;
		ufs->config_desc.unit[lun].bMemoryType = type;
		ufs->config_desc.unit[lun].bLogicalBlockSize = 0xc;
		ufs->config_desc.unit[lun].bProvisioningType = 0x2;

		ufs->config_desc.unit[lun].bDataReliability =
				stUnitDescrConfigParam[lun].bDataReliability;

		if (bootlun)
			num_alloc_units = target_in_byte *
				enhanced_type_one_ratio / alloc_unit_in_byte;
		else
			num_alloc_units = target_in_byte / alloc_unit_in_byte;

		ufs->config_desc.unit[lun].dNumAllocUnits = cpu_to_be32(num_alloc_units);

		/*
		 * In this case, a size of this LU will be calculated
		 * at the end of this function.
		 */
		if (!capacity_in_mb)
			flag = 1;
	} else {
		ufs->config_desc.unit[lun].bLUEnable = 0x0;
		ufs->config_desc.unit[lun].bBootLunID = 0x0;
		ufs->config_desc.unit[lun].bLUWriteProtect = 0x0;
		ufs->config_desc.unit[lun].bMemoryType = 0x0;
		ufs->config_desc.unit[lun].dNumAllocUnits = 0x0;

		ufs->config_desc.unit[lun].bDataReliability =
				stUnitDescrConfigParam[lun].bDataReliability;
		ufs->config_desc.unit[lun].bLogicalBlockSize =
				stUnitDescrConfigParam[lun].bLogicalBlockSize;
		ufs->config_desc.unit[lun].bProvisioningType =
				stUnitDescrConfigParam[lun].bProvisioningType;
		ufs->config_desc.unit[lun].wContextCapabilities =
				stUnitDescrConfigParam[lun].wContextCapabilities;
	}

	for (i = 0; i < 8; i++) {
		if (ufs->config_desc.unit[i].bLUEnable)
			luns++;
		if (ufs->config_desc.unit[i].bBootLunID)
			boot = 1;
		sum += be32_to_cpu(ufs->config_desc.unit[i].dNumAllocUnits);
	}

	ufs->config_desc.header.bLength = 0x90;
	ufs->config_desc.header.bDescriptorType = 0x1;
	ufs->config_desc.header.bConfDescContinue = 0x0;
	ufs->config_desc.header.bBootEnable = boot;
	ufs->config_desc.header.bDescrAccessEn = 0x1;
	ufs->config_desc.header.bInitPowerMode = 0x1;
	ufs->config_desc.header.bHighPriorityLUN = 0x7F;
	ufs->config_desc.header.bSecureRemovalType = 0x0;
	ufs->config_desc.header.bInitActiveICCLevel = 0x0;
	ufs->config_desc.header.wPeriodicRTCUpdate = 0x0;

	if (flag) {
		num_alloc_units =
			be32_to_cpu(ufs->geometry_desc.qTotalRawDeviceCapacity_l) /
				be32_to_cpu(ufs->geometry_desc.dSegmentSize) /
				ufs->geometry_desc.bAllocationUnitSize;
		num_alloc_units -= sum;

		ufs->config_desc.unit[lun].dNumAllocUnits = cpu_to_be32(num_alloc_units);
	}

	*flag_p = flag;
	return 0;
}

static int ufs_mphy_unipro_setting(struct ufs_host *ufs, struct ufs_uic_cmd *uic_cmd_list)
{
	int res = 0;
	u32 timeout;

	if (!uic_cmd_list) {
		dprintf(INFO, "%s: cmd list is empty\n", __func__);
		return res;
	}

	while (uic_cmd_list->uiccmdr) {
		ufs->uic_cmd = uic_cmd_list++;
		switch (ufs->uic_cmd->uiccmdr) {
		case UIC_CMD_WAIT:
			u_delay(ufs->uic_cmd->uiccmdarg2);
			break;
		case UIC_CMD_WAIT_ISR:
			timeout = ufs->uic_cmd_timeout;
			while ((readl(ufs->ioaddr + ufs->uic_cmd->uiccmdarg1) &
				ufs->uic_cmd->uiccmdarg2) != ufs->uic_cmd->uiccmdarg2) {
				if (!timeout--) {
					res = 0;
					goto out;
				}
				u_delay(1);
			}
			writel(ufs->uic_cmd->uiccmdarg2, (ufs->ioaddr + ufs->uic_cmd->uiccmdarg1));
			break;
		case UIC_CMD_REGISTER_SET:
			writel(ufs->uic_cmd->uiccmdarg2, (ufs->unipro_addr + ufs->uic_cmd->uiccmdarg1));
			break;
		case PHY_PMA_COMN_SET:
		case PHY_PMA_TRSV_SET:
			writel(ufs->uic_cmd->uiccmdarg2, (ufs->phy_pma + ufs->uic_cmd->uiccmdarg1));
			break;
		case PHY_PMA_COMN_WAIT:
		case PHY_PMA_TRSV_WAIT:
			timeout = ufs->uic_cmd_timeout;
			while ((readl(ufs->phy_pma + ufs->uic_cmd->uiccmdarg1) &
				ufs->uic_cmd->uiccmdarg2) != ufs->uic_cmd->uiccmdarg2) {
				if (!timeout--) {
					res = 0;
					goto out;
				}
				u_delay(1);
			}
			break;
		default:
			res = send_uic_cmd(ufs);
			break;
		}
	}

out:
	if (res)
		dprintf(INFO, "%s: failed cmd:0x%x arg1:0x%x arg2:0x%x with err %d\n",
			__func__, ufs->uic_cmd->uiccmdr,
			ufs->uic_cmd->uiccmdarg1,
			ufs->uic_cmd->uiccmdarg2, res);
	return res;
}

static int ufs_update_max_gear(struct ufs_host *ufs)
{
	struct ufs_uic_cmd rx_cmd = { UIC_CMD_DME_GET, (0x1587 << 16), 0, 0 };	/* PA_MAXRXHSGEAR */
	struct ufs_cal_param *p;
	int ret = 0;
	u32 max_rx_hs_gear = 0;

	if (!ufs)
		return ret;

	p = ufs->cal_param;

	ufs->uic_cmd = &rx_cmd;
	ret = send_uic_cmd(ufs);
	if (ret)
		goto out;

	max_rx_hs_gear = ufs->uic_cmd->uiccmdarg3;
	p->max_gear = MIN(max_rx_hs_gear, UFS_GEAR);

	printf("ufs max_gear(%d)\n", p->max_gear);

out:
	return ret;
}

static int ufs_update_active_lane(struct ufs_host *ufs)
{
	int res = 0;
	struct ufs_uic_cmd tx_cmd = { UIC_CMD_DME_GET, (0x1560 << 16), 0, 0 };	/* PA_ACTIVETXDATALANES */
	struct ufs_uic_cmd rx_cmd = { UIC_CMD_DME_GET, (0x1580 << 16), 0, 0 };	/* PA_ACTIVERXDATALANES */
	struct ufs_cal_param *p;

	if (!ufs)
		return res;

	p = ufs->cal_param;

	ufs->uic_cmd = &tx_cmd;
	res = send_uic_cmd(ufs);
	if (res)
		goto out;
	p->active_tx_lane = ufs->uic_cmd->uiccmdarg3;

	ufs->uic_cmd = &rx_cmd;
	res = send_uic_cmd(ufs);
	if (res)
		goto out;

	p->active_rx_lane = ufs->uic_cmd->uiccmdarg3;
	printf("active_tx_lane(%d), active_rx_lane(%d)\n", p->active_tx_lane, p->active_rx_lane);

out:
	return res;
}

static int ufs_check_2lane(struct ufs_host *ufs)
{
	struct ufs_cal_param *p;
	int res = 0;
	int tx, rx;
	struct ufs_uic_cmd tx_cmd = { UIC_CMD_DME_GET, (0x1561 << 16), 0, 0 };	/* PA_ConnectedTxDataLane */
	struct ufs_uic_cmd rx_cmd = { UIC_CMD_DME_GET, (0x1581 << 16), 0, 0 };	/* PA_ConnectedRxDataLane */
	struct ufs_uic_cmd ufs_2lane_cmd[] = {
		{UIC_CMD_DME_SET, (0x1560 << 16), 0, 2}	/* PA_ActiveTxDataLanes */
		, {UIC_CMD_DME_SET, (0x1580 << 16), 0, 2}	/* PA_ActiveRxDataLanes */
		/* NULL TERMINATION */
		, {0, 0, 0, 0}
	};
	struct ufs_uic_cmd ufs_1lane_cmd[] = {
		 {UIC_CMD_DME_SET, (0x1560 << 16), 0, 1}	/* PA_ActiveTxDataLanes */
		, {UIC_CMD_DME_SET, (0x1580 << 16), 0, 1}	/* PA_ActiveRxDataLanes */
		/* NULL TERMINATION */
		, {0, 0, 0, 0}
	};

	if (!ufs)
		return res;

	p = ufs->cal_param;

#if defined(CONFIG_UFS_1LANE_ONLY)
	tx = 1;
	rx = 1;
#else
	ufs->uic_cmd = &tx_cmd;
	res = send_uic_cmd(ufs);
	if (res)
		goto out;
	tx = ufs->uic_cmd->uiccmdarg3;

	ufs->uic_cmd = &rx_cmd;
	res = send_uic_cmd(ufs);
	if (res)
		goto out;
	rx = ufs->uic_cmd->uiccmdarg3;
#endif
	p->connected_tx_lane = tx;
	p->connected_rx_lane = rx;

	if (tx == 2 && rx == 2) {
		res = ufs_mphy_unipro_setting(ufs, ufs_2lane_cmd);
		if (res) {
			dprintf(INFO, "trying to use 2 lane connection, but fail...\n");
			goto out;
		}
		dprintf(INFO, "Use 2 lane connection\n");
	} else if (tx == 1 && rx == 1) {
		res = ufs_mphy_unipro_setting(ufs, ufs_1lane_cmd);
		if (res) {
			dprintf(INFO, "trying to use 1 lane connection, but fail...\n");
			goto out;
		}
		ufs->quirks = UFS_QUIRK_USE_1LANE;
		dprintf(INFO, "Use 1 lane connection\n");
	} else
		dprintf(INFO, "trying to use the abnormal lane connection tx=%d rx=%d, so it has failed...\n", tx, rx);

	/*
	 * I don't think it's necessary to check a number of Tx against
	 * a number of Rx.
	 */
	ufs->pmd_cxt.lane = tx;
 out:
	return res;
}


static int ufs_end_boot_mode(struct ufs_host *ufs)
{
	int flag = 1, retry = 1000;
	int res;

	res = ufs_utp_nop_process(ufs);
	if (res) {
		printf("UFS: NOP OUT failed\n");
		return res;
	}

	ufs->flags.arry[UPIU_FLAG_ID_DEVICEINIT] = flag;
	res = ufs_utp_query_process(ufs, FLAG_W_FDEVICEINIT , 0);
	if (res)
		return res;

	do {
		res = ufs_utp_query_process(ufs, FLAG_R_FDEVICEINIT, 0);
		if (res)
			goto end;
		flag = ufs->flags.arry[UPIU_FLAG_ID_DEVICEINIT];
		retry--;
	} while (flag && retry > 0);

	if (flag) {
		printf("UFS: fdeviceinit faild\n");
		res = -1;
	}

end:
	return res;
}

int ufs_device_reset()
{
	struct ufs_host *ufs = get_cur_ufs_host();

	writel(0, ufs->vs_addr + VS_GPIO_OUT);
	u_delay(5);
	writel(1, ufs->vs_addr + VS_GPIO_OUT);

	return 0;
}

static int ufs_device_power(struct ufs_host *ufs, int onoff)
{
	/* GPD0_DAT[2] : GPG0_2=1 */
	if (ufs->dev_pwr_addr)
		UFS_SET_SFR(ufs->dev_pwr_addr, onoff, 0x1, ufs->dev_pwr_shift);

	return 0;
}

static int ufs_pre_setup(struct ufs_host *ufs)
{
	u32 reg;
	u32 val;
	int res = 0;

	struct ufs_uic_cmd reset_cmd = {UIC_CMD_DME_RESET, 0, 0, 0};
	struct ufs_uic_cmd enable_cmd = {UIC_CMD_DME_ENABLE, 0, 0, 0};

	/* UFS_PHY_CONTROL : 1 = Isolation bypassed, PMU MPHY ON */
	val = readl(ufs->phy_iso_addr);
	if ((val & 0x1) == 0) {
		val |= (1 << 0);
		writel(val, ufs->phy_iso_addr);
	}

	/* VS_SW_RST */
	if ((readl(ufs->vs_addr + VS_FORCE_HCS) >> 4) & 0xf)
		writel(0x0, (ufs->vs_addr +  VS_FORCE_HCS));

	writel(3, (ufs->vs_addr + VS_SW_RST));
	while (readl(ufs->vs_addr + VS_SW_RST))
		;

	/* VENDOR_SPECIFIC_IS[20] : clear UFS_IDLE_Indicator bit (if UFS_LINK is reset, this bit is asserted) */
	reg = readl(ufs->vs_addr + VS_IS);
	if ((reg >> 20) & 0x1)
		writel(reg, (ufs->vs_addr + VS_IS));


	ufs_device_power(ufs, 1);
	u_delay(1000);
	ufs_device_reset();

	if (ufs->quirks & UFS_QUIRK_BROKEN_HCE) {
		ufs->uic_cmd = &reset_cmd;
		res = send_uic_cmd(ufs);
		if (res) {
			printf("UFS%d reset error!\n", ufs->host_index);
			return res;
		}
		ufs->uic_cmd = &enable_cmd;
		res = send_uic_cmd(ufs);
		if (res) {
			printf("UFS%d reset error!\n", ufs->host_index);
			return res;
		}
	} else {
		writel(1, (ufs->ioaddr + REG_CONTROLLER_ENABLE));

		while (!(readl(ufs->ioaddr + REG_CONTROLLER_ENABLE) & 0x1))
			u_delay(1);
	}

	/*ctrl refclkout*/
	writel((readl(ufs->vs_addr + VS_CLKSTOP_CTRL) & ~(1 << 4)), ufs->vs_addr + VS_CLKSTOP_CTRL);

	/*CLOCK GATING SET*/
	writel(0xde0, ufs->vs_addr + VS_FORCE_HCS);

	writel(readl(ufs->vs_addr + VS_UFS_ACG_DISABLE)|1, ufs->vs_addr + VS_UFS_ACG_DISABLE);
	memset(ufs->cmd_desc_addr, 0x00, UFS_NUTRS*sizeof(struct ufs_cmd_desc));
	memset(ufs->utrd_addr, 0x00, UFS_NUTRS*sizeof(struct ufs_utrd));
	//memset(ufs->utmrd_addr, 0x00, UFS_NUTMRS*sizeof(struct ufs_utmrd));
	ufs->utrd_addr->cmd_desc_addr_l = (u64)(ufs->cmd_desc_addr);
	ufs->utrd_addr->rsp_upiu_off = (u16)(offsetof(struct ufs_cmd_desc, response_upiu));
	ufs->utrd_addr->rsp_upiu_len = (u16)(ALIGNED_UPIU_SIZE);

	writel((u64)ufs->utmrd_addr, (ufs->ioaddr + REG_UTP_TASK_REQ_LIST_BASE_L));
	writel(0, (ufs->ioaddr + REG_UTP_TASK_REQ_LIST_BASE_H));

	writel((u64)ufs->utrd_addr, (ufs->ioaddr + REG_UTP_TRANSFER_REQ_LIST_BASE_L));
	writel(0, (ufs->ioaddr + REG_UTP_TRANSFER_REQ_LIST_BASE_H));

	// TODO: cport
	writel(0x22, ufs->vs_addr + 0x114);
	writel(1, ufs->vs_addr + 0x110);

	return NO_ERROR;
}

static void ufs_vendor_setup(struct ufs_host *ufs)
{
	/* DMA little endian, order change */
	writel(0xa, (ufs->vs_addr + VS_DATA_REORDER));

	writel(1, (ufs->ioaddr + REG_UTP_TASK_REQ_LIST_RUN_STOP));

	writel(1, (ufs->ioaddr + REG_UTP_TRANSFER_REQ_LIST_RUN_STOP));

	writel(UFS_SG_BLOCK_SIZE_BIT, (ufs->vs_addr + VS_TXPRDT_ENTRY_SIZE));
	writel(UFS_SG_BLOCK_SIZE_BIT, (ufs->vs_addr + VS_RXPRDT_ENTRY_SIZE));

	writel(0xFFFFFFFF, (ufs->vs_addr + VS_UTRL_NEXUS_TYPE));
	writel(0xFFFFFFFF, (ufs->vs_addr + VS_UMTRL_NEXUS_TYPE));
}

static int ufs_pmc_common(struct ufs_host *ufs, struct uic_pwr_mode *pmd)
{
	u32 reg;
	int res = NO_ERROR;
	struct ufs_uic_cmd cmd[] = {
		{UIC_CMD_DME_SET, (0x1583 << 16), 0, UFS_GEAR}, /* PA_RxGear */
		{UIC_CMD_DME_SET, (0x1568 << 16), 0, UFS_GEAR}, /* PA_TxGear */
		{UIC_CMD_DME_SET, (0x1580 << 16), 0, 0}, /* PA_ActiveRxDataLanes */
		{UIC_CMD_DME_SET, (0x1560 << 16), 0, 0}, /* PA_ActiveTxDataLanes */
		{UIC_CMD_DME_SET, (0x1584 << 16), 0, 1}, /* PA_RxTermination */
		{UIC_CMD_DME_SET, (0x1569 << 16), 0, 1}, /* PA_TxTermination */
		{UIC_CMD_DME_SET, (0x156a << 16), 0, UFS_RATE}, /* PA_HSSeries */
		{0, 0, 0, 0}
	};

	struct ufs_uic_cmd pmc_cmd = {
		UIC_CMD_DME_SET, (0x1571 << 16), 0, UFS_RXTX_POWER_MODE
	};

	/* Modity a value to be set PA_ActiveXxDataLanes */
	cmd[2].uiccmdarg3 = pmd->lane;
	cmd[3].uiccmdarg3 = pmd->lane;

	res = ufs_mphy_unipro_setting(ufs, cmd);
	if (res)
		goto out;

	/* Actual power mode change */
	ufs->uic_cmd = &pmc_cmd;
	res = send_uic_cmd(ufs);
	if (res)
		goto out;
	//printf("Use 2 lane connection\n");

	reg = readl(ufs->ioaddr + REG_CONTROLLER_STATUS);
	if (UPMCRS(reg) != PWR_LOCAL) {
		printf("UFS: gear change failed, UPMCRS = %x !!\n",
							UPMCRS(reg));
		res = ERR_GENERIC;
	}
out:
	return res;
}

/*
 * In this function, read device's bRefClkFreq attribute
 * and if attr is not 1h, change it to 1h which means 26MHz.
 * It's because Exynos always use 26MHz reference clock
 * and device should know about soc's ref clk value.
 */
static int ufs_ref_clk_setup(struct ufs_host *ufs)
{
	int res;

	res = ufs_utp_query_process(ufs, ATTR_R_REFCLKFREQ, 0);
	if (res) {
		printf("[UFS] read ref clk failed\n");
		return res;
	} else {
		printf("[UFS] ref clk setting is %x\n", ufs->attributes.arry[UPIU_ATTR_ID_REFCLKFREQ]);
	}

	if (ufs->attributes.arry[UPIU_ATTR_ID_REFCLKFREQ] != 0x1) {
		ufs->attributes.arry[UPIU_ATTR_ID_REFCLKFREQ] = 0x01;
		res = ufs_utp_query_process(ufs, ATTR_W_REFCLKFREQ, 0);
	}

	return res;
}

static int ufs_init_interface(struct ufs_host *ufs)
{
	struct ufs_uic_cmd uic_cmd = { UIC_CMD_DME_LINK_STARTUP, 0, 0, 0};
	struct ufs_uic_cmd get_a_lane_cmd = { UIC_CMD_DME_GET, (0x1540 << 16), 0, 0 };
	struct uic_pwr_mode *pmd = &ufs->pmd_cxt;
	int res = -1;

	if (ufs_pre_setup(ufs))
		goto out;

	ufs_pre_vendor_setup(ufs);

	/* 1. pre link */
	ufs->uic_cmd = &get_a_lane_cmd;
	if (send_uic_cmd(ufs)) {
		printf("UFS%d getting a number of lanes error!\n", ufs->host_index);
		goto out;
	}

	if (ufs_pre_link(ufs, ufs->uic_cmd->uiccmdarg3))
		goto out;

	/* 2. link startup */
	ufs->uic_cmd = &uic_cmd;
	if (send_uic_cmd(ufs)) {
		printf("UFS%d linkstartup error!\n", ufs->host_index);
		goto out;
	}

	/* 3. update max gear */
	if (ufs_update_max_gear(ufs))
		goto out;

	/* 4. post link */
	if (ufs_post_link(ufs))
		goto out;

	/* 5. update active lanes */
	if (ufs_update_active_lane(ufs))
		goto out;

	/* 6. misc hci setup for NOP and fDeviceinit */
	ufs_vendor_setup(ufs);

	/* 7. NOP and fDeviceinit */
	if (ufs_end_boot_mode(ufs))
		goto out;

	/* 8. Check a number of connected lanes */
	if (ufs_check_2lane(ufs))
		goto out;

	/* 9. pre pmc */
	pmd->gear = UFS_GEAR;
	pmd->mode = UFS_POWER_MODE;
	pmd->hs_series = UFS_RATE;

	if (ufs_pre_gear_change(ufs, pmd))
		goto out;

	/* 10. pmc (power mode change) */
	if (ufs_pmc_common(ufs, pmd))
		goto out;

	/* 11. update active lanes */
	if (ufs_update_active_lane(ufs))
		goto out;

	/* 12. post pmc */
	if (ufs_post_gear_change(ufs))
		goto out;

	printf("Power mode change: M(%d)G(%d)L(%d)HS-series(%d)\n",
			(pmd->mode & 0xF), pmd->gear, pmd->lane, pmd->hs_series);
	res = 0;

out:
	return res;
}

static void ufs_init_mem(struct ufs_host *ufs)
{
	ufs_debug("cmd_desc_addr : %p\n", ufs->cmd_desc_addr);
	ufs_debug("\tresponse_upiu : %p\n", &ufs->cmd_desc_addr->response_upiu);
	ufs_debug("\tprd_table : %p (size=%lx)\n", ufs->cmd_desc_addr->prd_table,
		  sizeof(ufs->cmd_desc_addr->prd_table));
	ufs_debug("\tsizeof upiu : %lx\n", sizeof(struct ufs_upiu));

	memset(ufs->cmd_desc_addr, 0x00, UFS_NUTRS * sizeof(struct ufs_cmd_desc));

	ufs_debug("utrd_addr : %p\n", ufs->utrd_addr);
	memset(ufs->utrd_addr, 0x00, UFS_NUTRS * sizeof(struct ufs_utrd));

	ufs->utrd_addr->cmd_desc_addr_l = (u64)(ufs->cmd_desc_addr);
	ufs->utrd_addr->rsp_upiu_off = (u16)(offsetof(struct ufs_cmd_desc, response_upiu));
	ufs->utrd_addr->rsp_upiu_len = (u16)(ALIGNED_UPIU_SIZE);

	writel((u64)ufs->utmrd_addr, (ufs->ioaddr + REG_UTP_TASK_REQ_LIST_BASE_L));
	writel(0, (ufs->ioaddr + REG_UTP_TASK_REQ_LIST_BASE_H));

	writel((u64)ufs->utrd_addr, (ufs->ioaddr + REG_UTP_TRANSFER_REQ_LIST_BASE_L));
	writel(0, (ufs->ioaddr + REG_UTP_TRANSFER_REQ_LIST_BASE_H));
}

static int ufs_identify_bootlun(struct ufs_host *ufs)
{
	int boot_lun_en;
	int i;
	int res;

	res = ufs_utp_query_process(ufs, ATTR_R_BOOTLUNEN, 0);
	if (res)
		goto end;
	boot_lun_en = ufs->attributes.arry[UPIU_ATTR_ID_BOOTLUNEN];
	if (boot_lun_en == 0) {
		printf("Boot LUN is disabled\n");
		res = -1;
		goto end;
	}

	for (i = 0; i < 8; i++) {
		res = ufs_utp_query_process(ufs, DESC_R_UNIT_DESC, i);
		if (res)
			goto end;
		if (boot_lun_en == ufs->unit_desc[i].bBootLunID) {
			printf("Boot LUN is #%d, bBootLunID:%d\n", i, ufs->unit_desc[i].bBootLunID);
		}
	}

end:
	return res;
}

static void ufs_disable_ufsp(struct ufs_host *ufs)
{
	writel(0x0, ufs->fmp_addr + UFSP_UPSBEGIN0);
	writel(0xffffffff, ufs->fmp_addr + UFSP_UPSEND0);
	writel(0xff, ufs->fmp_addr + UFSP_UPLUN0);
	writel(0xf1, ufs->fmp_addr + UFSP_UPSCTRL0);
}

/*
 * This function shall be called at least once.
 * Expept for the case that LUs are not configured properly,
 * this function execution makes devices to prepare IO process
 */
static int ufs_init_host(int host_index, struct ufs_host *ufs)
{
	/* command timeout may be redefined in ufs_board_init()  */
	ufs->ufs_cmd_timeout = 1000000;	/* 100msec */
	ufs->uic_cmd_timeout = 1000000;	/* 100msec */
	ufs->ufs_query_req_timeout = 15000000;	/* 1500msec */

	/* AP specific UFS host init */
	if (ufs_board_init(host_index, ufs))
		goto out;

	/* Read capabilities registers */
	ufs->capabilities = readl(ufs->ioaddr + REG_CONTROLLER_CAPABILITIES);
	ufs->ufs_version = readl(ufs->ioaddr + REG_UFS_VERSION);
	ufs_debug
	    ("%s\n\tcaps(0x%p) 0x%08x\n\tver(0x%p)  0x%08x\n\tPID(0x%p)  0x%08x\n\tMID(0x%p)  0x%08x\n",
	     ufs->host_name, ufs->ioaddr + REG_CONTROLLER_CAPABILITIES, ufs->capabilities,
	     ufs->ioaddr + REG_UFS_VERSION, ufs->ufs_version, ufs->ioaddr + REG_CONTROLLER_PID,
	     readl(ufs->ioaddr + REG_CONTROLLER_PID)
	     , ufs->ioaddr + REG_CONTROLLER_MID, readl(ufs->ioaddr + REG_CONTROLLER_MID));

	ufs_init_mem(ufs);

	if (*(unsigned int *)DRAM_BASE != 0xabcdef) {
		/* It boots by T32. set FMP as by-passed */
		ufs_disable_ufsp(ufs);
	}

	if (ufs_init_cal(ufs, host_index))
		goto out;

	return 0;

 out:
	return -1;
}

static int ufs_check_available_blocks(struct ufs_host *ufs,
				struct ufs_unit_desc_param *src,
				struct ufs_unit_desc_param *tgt,
				int lun,
				u32 sum_in_alloc_unit)
{
	int ret = 0;

	unsigned long src_alloc_units = be32_to_cpu(src->dNumAllocUnits);
	unsigned long tgt_alloc_units;
	unsigned long target_in_byte = tgt->dNumAllocUnits * 1024 * 1024;
	unsigned int enhanced_type_one_ratio = be16_to_cpu(ufs->geometry_desc.wEnhanced1CapAdjFac) / 256;
	unsigned long alloc_unit_in_byte = ufs->geometry_desc.bAllocationUnitSize *
			be32_to_cpu(ufs->geometry_desc.dSegmentSize) * 512;

	if (tgt->dNumAllocUnits == 0 && tgt->bLUEnable)
		/*
		 * Zero input capacity case such as LU #0 currently
		 * To calculate any amount in allocation unit
		 * with qTotalRawDeviceCapacity_l, we need to divide it
		 * with dSegmentSize once and bAllocationUnitSize once
		 * because a unit of qTotalRawDeviceCapacity_l is 512 bytes
		 * specified in UFS specification.
		 */
		tgt_alloc_units = be32_to_cpu(ufs->geometry_desc.qTotalRawDeviceCapacity_l) /
					be32_to_cpu(ufs->geometry_desc.dSegmentSize) /
					ufs->geometry_desc.bAllocationUnitSize - sum_in_alloc_unit;

	else if (tgt->bMemoryType > 0)
		/* Boot LU */
		tgt_alloc_units = target_in_byte * enhanced_type_one_ratio /
							alloc_unit_in_byte;

	else if (tgt->bMemoryType == 0)
		/* Normal LU */
		tgt_alloc_units = target_in_byte / alloc_unit_in_byte;

	if (src_alloc_units != tgt_alloc_units) {
		printf("UFS: dNumAllocUnits error at LU%d: (src= %ld, tgt= %ld)\n",
				lun, src_alloc_units, tgt_alloc_units);
		ret = 1;
	}

	return ret;
}

static int ufs_check_config_desc(void)
{
	int lun = 0;
	u32 sum = 0;
	struct ufs_host *ufs = get_cur_ufs_host();
	struct ufs_unit_desc_param *src;
	struct ufs_unit_desc_param *tgt;
	int res = -1;

	if (!ufs)
		return res;

	ufs->lun = 0;

	res = ufs_utp_query_process(ufs, DESC_R_DEVICE_DESC, 0);
	if (res)
		goto error;
	res = ufs_utp_query_process(ufs, DESC_R_GEOMETRY_DESC, 0);
	if (res)
		goto error;

	res = ufs_utp_query_process(ufs, DESC_R_CONFIG_DESC, 0);
	if (res)
		goto error;
	res = ufs_utp_query_process(ufs, ATTR_R_BOOTLUNEN, 0);
	if (res)
		goto error;

	for (lun = 7; lun >= 0; lun--) {
		src = &ufs->config_desc.unit[lun];
		tgt = &stUnitDescrConfigParam[lun];

		if (tgt->bLUEnable != src->bLUEnable) {
			printf("bLUEnable error at LU%d  %d, %d\n",
				lun, tgt->bLUEnable, src->bLUEnable);
			goto error;
		}

		if (tgt->bBootLunID != src->bBootLunID) {
			printf("bBootLunID error at LU%d  %d, %d\n",
				lun, tgt->bBootLunID, src->bBootLunID);
			goto error;
		}

		if (tgt->bLUWriteProtect != src->bLUWriteProtect) {
			printf("bLUWriteProtect error at LU%d  %d, %d\n",
				lun, tgt->bLUWriteProtect,
					 src->bLUWriteProtect);
			goto error;
		}

		if (tgt->bMemoryType != src->bMemoryType) {
			printf("bMemoryType error at LU%d  %d, %d\n",
				lun, tgt->bMemoryType, src->bMemoryType);
			goto error;
		}

		if (tgt->bDataReliability != src->bDataReliability) {
			printf("bDataReliability error at LU%d  %d, %d\n",
				lun, tgt->bDataReliability,
					 src->bDataReliability);
			goto error;
		}

		if (tgt->bLogicalBlockSize != src->bLogicalBlockSize) {
			printf("bLogicalBlockSize error at LU%d  %d, %d\n",
				lun, tgt->bLogicalBlockSize,
					 src->bLogicalBlockSize);
			goto error;
		}

		if (tgt->bProvisioningType != src->bProvisioningType) {
			printf("bProvisioningType error at LU%d  %d, %d\n",
				lun, tgt->bProvisioningType,
					 src->bProvisioningType);
			goto error;
		}

		if (tgt->wContextCapabilities != src->wContextCapabilities) {
			printf("wContextCapabilities error at LU%d  %d, %d\n",
				lun, tgt->wContextCapabilities,
					 src->wContextCapabilities);
			goto error;
		}

		if (ufs_check_available_blocks(ufs, src, tgt, lun, sum))
			goto error;
		 sum += be32_to_cpu(src->dNumAllocUnits);
	}

	/* When you type 'ufs boot 0', you will face this condition */
	if (ufs->attributes.arry[UPIU_ATTR_ID_BOOTLUNEN] == 0)
		goto error;

	printf("PROVISION CHECK: PASS\n");
	return 1;
error:
	printf("PROVISION CHECCK: FAIL\n");
	return 0;
}

/*
 * EXTERNAL FUNCTION: ufs_set_configuration_descriptor
 *
 * This is called at boot time to check that LU configuration is done
 * with expected values and, if not, do it.
 *
 * Return values
 * 0: no error
 * 1: LU configuration done
 * others: uncorrectable error
 */
int ufs_set_configuration_descriptor(void)
{
	int lun = 0;
	int ret = 0;

	int retry_count = 3;
	int retry = 0;

	/* The pointer to represent whether capacity 0 LU is assigned which is LUN0 in general */
	int found_no_capacity = 0;
	struct ufs_host *ufs = get_cur_ufs_host();

	while (ufs_check_config_desc() == 0) {
		if (retry_count == retry) {
			printf("[UFS] LU config: FAIL !!!\n");
			break;
		}
		ret = 0;
		printf("[UFS] LU config: trying %d...\n", retry++);

		for (lun = 7; lun >= 0; lun--) {
			ret = ufs_edit_config_desc(lun,
				stUnitDescrConfigParam[lun].bLUEnable,
				stUnitDescrConfigParam[lun].bBootLunID,
				stUnitDescrConfigParam[lun].bLUWriteProtect,
				stUnitDescrConfigParam[lun].bMemoryType,
				stUnitDescrConfigParam[lun].dNumAllocUnits,
				&found_no_capacity);
			if (ret == 2) {
				printf("[UFS] LU config: can't edit config descriptor !!!\n");
				goto fail;
			}
		}

		ret = ufs_utp_query_process(ufs, DESC_W_CONFIG_DESC, 0);
		if (ret) {
			printf("[UFS] LU config: Descriptor write query fail with %d!!!\n", ret);
			continue;
		}

		ufs->attributes.arry[UPIU_ATTR_ID_BOOTLUNEN] = 0x01;
		ret = ufs_utp_query_process(ufs, ATTR_W_BOOTLUNEN, 0);
		if (ret) {
			printf("[UFS] LU config: BootLUNEN setting fail with %d!!!\n", ret);
			continue;
		}

		u_delay(1000*1000);
		break;
	}

	/* in case of UFS provisioning execution */
	if (retry) {
		puts("[UFS] LU config: PASS !!!\n");

		/* remove enumerated bdevs*/
		scsi_exit(&ufs_lu_list, "scsi");
		ret = 1;
	}

fail:
	return ret;
}


/*
 * EXTERNAL FUNCTION: ufs_init
 *
 * This is called at boot time to initialize UFS driver and
 * enumerate all the attached Logical Units.
 */
status_t ufs_init(int mode)
{

	int r = 0, i;
	int rst_cnt = 0;

	printf("\nUFS: %s: START TO INIT --------------------------------------------- \n", __func__);

	// TODO:
#if 0
	if ((mode == 2) && (exynos_boot_mode() != BOOT_UFS)) {
		dprintf(INFO, " Not UFS boot mode. Init UFS manually.\n");
		return r;
	}
#endif

	ufs_lu_list.prev = 0;
	ufs_lu_list.next = 0;

	for (i = 0; i < SCSI_MAX_INITIATOR; i++) {
		/* Initialize host */
		r = ufs_init_host(i, _ufs[i]);
		if (r)
			goto out;

		/* Establish interface */
		do {
			r = ufs_init_interface(_ufs[i]);
			if (!r)
				break;
			rst_cnt++;
			printf("UFS: Retry Link Startup CNT : %d\n", rst_cnt);
		} while (rst_cnt < 3);
		if (r)
			goto out;

		/* Check if boot LUs exist */
		r = ufs_identify_bootlun(_ufs[i]);
		if (r)
			goto out;

		/* SCSI device enumeration */
		scsi_scan(ufs_dev[i], 0, SCSI_MAX_DEVICE, scsi_exec, NULL, 128, &ufs_lu_list);
		if (r)
			goto out;
		scsi_scan(&ufs_dev_rpmb, 0x44, 0, scsi_exec, "rpmb", 128, &ufs_lu_list);
		if (r)
			goto out;
		scsi_scan_ssu(&ufs_dev_ssu, 0x50, scsi_exec, (get_sdev_t *)scsi_get_ssu_sdev, &ufs_lu_list);
		if (r)
			goto out;
	}

out:
	/*
	 * Current host is zero by default after preparing to read and write
	 * because we assume that system boot requires host #0
	 */
	_ufs_curr_host = 0;

	return r;
}

/*
 * This function shall be called only once. These memory would be used
 * permantely in bootloader lifcycle, so we don't need to free memory
 */
int ufs_alloc_memory()
{
	struct ufs_host *ufs;
	int r = -1, i;
	size_t len;

	for (i = 0; i < SCSI_MAX_INITIATOR; i++) {
		_ufs_curr_host = i;

		/* Allocation for host */
		len = sizeof(struct ufs_host);
		if (!(_ufs[i] = malloc(len)))
			goto end;
		ufs = _ufs[i];
		memset(_ufs[i], 0x00, sizeof(struct ufs_host));

		/* Allocation for CAL */
		len = sizeof(struct ufs_cal_param);
		if (!(ufs->cal_param = malloc(len)))
			goto end;

		/* Allocation for descriptor */
		len = UFS_NUTRS * sizeof(struct ufs_cmd_desc);
		if (!(ufs->cmd_desc_addr = memalign(0x1000, len))) {
			printf("UFS: %s: cmd_desc_addr memory alloc error!!!\n", __func__);
			goto end;
		}
		if ((u64)(ufs->cmd_desc_addr) & 0xfff) {
			printf("UFS: %s: allocated cmd_desc_addr memory align error!!!\n", __func__);
			goto end;
		}

		len = UFS_NUTRS * sizeof(struct ufs_utrd);
		if (!(ufs->utrd_addr = memalign(0x1000, len))) {
			printf("UFS: %s: utrd_addr memory alloc error!!!\n", __func__);
			goto end;
		}
		if ((u64)(ufs->utrd_addr) & 0xfff) {
			printf("UFS: %s: allocated utrd_addr memory align error!!!\n", __func__);
			goto end;
		}

		/* Allocation for device enumeration */
		len = sizeof(scsi_device_t) * SCSI_MAX_DEVICE;
		if (!(ufs_dev[i] = (scsi_device_t *)malloc(len))) {
			printf("UFS: %s: ufs_dev memory allocation failed\n", __func__);
			goto end;
		}
	}

	r = 0;
end:
	if (r != 0)
		printf("##### LK memory allocation fails !!! #####\n");

	return r;
}
