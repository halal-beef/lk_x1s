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

#define be16_to_cpu(x) \
	((((x) & 0xff00) >> 8) | \
	 (((x) & 0x00ff) << 8))
#define cpu_to_be16(x) be16_to_cpu(x)

#define be32_to_cpu(x) \
	((((x) & 0xff000000) >> 24) | \
	 (((x) & 0x00ff0000) >>  8) | \
	 (((x) & 0x0000ff00) <<  8) | \
	 (((x) & 0x000000ff) << 24))
#define cpu_to_be32(x) be32_to_cpu(x)

#ifdef	SCSI_UFS_DEBUG
#define	ufs_debug(fmt,args...)	dprintf (INFO, fmt ,##args)
#else
#define ufs_debug(INFO, fmt,args...)
#endif

#define	SCSI_MAX_INITIATOR	1
#define	SCSI_MAX_DEVICE		8

static int send_uic_cmd(struct ufs_host *ufs);
static int setufs_boot(u32 enable);

/*
	Multiple UFS host : cmd_scsi should be changed
	Now only one host supported
*/
static struct ufs_host *_ufs[SCSI_MAX_INITIATOR] = { NULL, };
static int _ufs_curr_host = 0;

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

	return setufs_boot((u32) argv[2].u);
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


static int ufs_send_upiu(ufs_upiu_cmd cmd, int enable);
static int ufs_link_startup(struct ufs_host *ufs);

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
	struct ufs_cal_param *p = NULL;

	ufs->cal_param = p = malloc(sizeof(struct ufs_cal_param));
	ufs->cal_param->host = (void *)ufs;

	#if defined(CONFIG_MACH_UNIVERSAL9820)
	ufs->cal_param->board = BRD_UNIV;
	#else
	ufs->cal_param->board = BRD_SMDK;
	#endif

	ufs->cal_param->evt_ver = (readl(0x10000010) >> 20) & 0xf;
	printf("ufs->cal_param->evt_ver is EVT%d!!!\n", ufs->cal_param->evt_ver);

	ret = ufs_cal_init(p, idx);
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

static status_t ufs_map_sg(struct ufs_host *ufs)
{
	int i, len, sg_segments;

	if (!ufs->scsi_cmd)
		goto no_data;

	len = ufs->scsi_cmd->datalen;
	if (len < 0)
		goto no_data;

	if (len) {
		sg_segments = (len + UFS_SG_BLOCK_SIZE - 1) / UFS_SG_BLOCK_SIZE;
		ufs->utrd_addr->prdt_len = (sg_segments) * sizeof(struct ufs_prdt);
		ufs->utrd_addr->prdt_off = (u16) (ALIGNED_UPIU_SIZE * 2);
		for (i = 0; i < sg_segments; i++) {
			ufs->cmd_desc_addr->prd_table[i].size =
			    (u32) UFS_SG_BLOCK_SIZE - 1;
			ufs->cmd_desc_addr->prd_table[i].base_addr =
			    (u32)(((u64) (ufs->scsi_cmd->buf) + i * UFS_SG_BLOCK_SIZE) & (((u64)1 << UFS_BIT_LEN_OF_DWORD) - 1));
			ufs->cmd_desc_addr->prd_table[i].upper_addr =
			    (u32)(((u64) (ufs->scsi_cmd->buf) + i * UFS_SG_BLOCK_SIZE) >> UFS_BIT_LEN_OF_DWORD);
			len -= UFS_SG_BLOCK_SIZE;
		}
		return 0;
	}

 no_data:
	ufs->utrd_addr->prdt_len = 0;
	ufs->utrd_addr->prdt_off = 0;
	return 0;
}

static void ufs_set_dir_flag_in_cmd_upiu(scm *pscm, u32 *data_direction,
							u32 *upiu_flags)
{
	if (pscm->datalen) {
		switch (pscm->cdb[0]) {
		case SCSI_OP_UNMAP:
		case SCSI_OP_FORMAT_UNIT:
		case SCSI_OP_WRITE_10:
		case SCSI_OP_WRITE_BUFFER:
		case SCSI_OP_SECU_PROT_OUT:
			*data_direction = UTP_HOST_TO_DEVICE;
			*upiu_flags = UPIU_CMD_FLAGS_WRITE;
			break;
		case SCSI_OP_START_STOP_UNIT:
			*data_direction = UTP_HOST_TO_DEVICE;
			*upiu_flags = UPIU_CMD_FLAGS_NONE;
			break;
		default:
			*data_direction = UTP_DEVICE_TO_HOST;
			*upiu_flags = UPIU_CMD_FLAGS_READ;
			break;
		}
	} else {
		*data_direction = UTP_NO_DATA_TRANSFER;
		*upiu_flags = UPIU_CMD_FLAGS_NONE;
	}
}

static void ufs_set_specific_fields_in_query_upiu(struct ufs_host *ufs)
{
	u32 function = ((ufs->command_type) >> 8) & 0xff;
	u32 opcode = ((ufs->command_type) >> 16) & 0xff;
	u32 descid = ((ufs->command_type) >> 24) & 0xff;
	u8 *p = (u8 *)ufs->cmd_desc_addr->command_upiu.header.tsf;

	struct ufs_upiu *cmd_upiu = &ufs->cmd_desc_addr->command_upiu;
	u16 min_len = (u16) MIN(ufs->data_seg_len, UPIU_DATA_SIZE);

	switch (function) {
	case UFS_STD_READ_REQ:
		*(u16 *) (&p[6]) = cpu_to_be16(UPIU_DATA_SIZE);
		break;
	case UFS_STD_WRITE_REQ:
		if (ufs->data_seg_len) {
			*(u16 *) (&p[6]) = cpu_to_be16(ufs->data_seg_len);
			cmd_upiu->header.datalength =
				cpu_to_be16(ufs->data_seg_len);
			memcpy(cmd_upiu->data, ufs->upiu_data, min_len);
		}

		if (opcode == UPIU_QUERY_OPCODE_WRITE_ATTR)
			*(u32 *) (&p[8]) = cpu_to_be32(ufs->attributes.arry[descid]);
		break;
	case UFS_VENDOR_FORMAT_REQ:
		if (ufs->data_seg_len) {
			cmd_upiu->header.datalength =
				cpu_to_be16(ufs->data_seg_len);
			memcpy(cmd_upiu->data, ufs->upiu_data, min_len);
		}
		break;
	}
}

static void ufs_compose_upiu(struct ufs_host *ufs)
{
	u32 data_direction;
	u32 upiu_flags;
	u32 type;
	u32 function;
	u32 opcode;
	u32 descid;
	u8 *p = (u8 *)ufs->cmd_desc_addr->command_upiu.header.tsf;
	//scm *pscm = (scm *) & tempscm;
/*
	command_type :
	| descriptor ID | opcode | function | UPIU type |
	|       LUN        |Task Tag| function | UPIU type |

	UPIU type[7:0] : NOP_OUT, COMMAND, DATA_OUT, TASK_REQ, QUERY_REQ
	function[15:8] : Used by QUERY_REQ / TASK_REQ
	opcode[23:16] : Opcode in query / input parameter2=Task Tag (TASK_REQ)
	descriptor ID[31:24] : descriptor ID in query / input parameter1=LUN (TASK_REQ)
*/
	type = (ufs->command_type) & 0xff;
	function = ((ufs->command_type) >> 8) & 0xff;
	opcode = ((ufs->command_type) >> 16) & 0xff;
	descid = ((ufs->command_type) >> 24) & 0xff;
#ifdef	SCSI_UFS_DEBUG
	dprintf(INFO, "ufs_compose_upiu : type(%02x) function(%02x) opcode(%02x) descID(%02x)\n", type,
	       function, opcode, descid);
#endif
	switch (type) {
	case UPIU_TRANSACTION_COMMAND:
		ufs_set_dir_flag_in_cmd_upiu(ufs->scsi_cmd, &data_direction,
								&upiu_flags);
		ufs->utrd_addr->dw[0] =
		    (u32)(data_direction | UTP_SCSI_COMMAND | UTP_REQ_DESC_INT_CMD);
		ufs->utrd_addr->dw[2] = (u32)(OCS_INVALID_COMMAND_STATUS);

		ufs->cmd_desc_addr->command_upiu.header.type = type;
		ufs->cmd_desc_addr->command_upiu.header.flags = upiu_flags;
		ufs->cmd_desc_addr->command_upiu.header.lun = ufs->lun;
		ufs->cmd_desc_addr->command_upiu.header.tag = 0;	/* queuing is not used */

		ufs->cmd_desc_addr->command_upiu.header.tsf[0] =
		    cpu_to_be32(ufs->scsi_cmd->datalen);

		if (ufs->scsi_cmd->cdb[0] == SCSI_OP_START_STOP_UNIT)
			ufs->cmd_desc_addr->command_upiu.header.tsf[0] = 0;

		memcpy(&ufs->cmd_desc_addr->command_upiu.header.tsf[1],
				ufs->scsi_cmd->cdb, MAX_CDB_SIZE);
		break;
	case UPIU_TRANSACTION_TASK_REQ:
		ufs->utmrd_addr->dw[2] = (u32)(OCS_INVALID_COMMAND_STATUS);
		ufs->utmrd_addr->task_req_upiu.type = type;
		ufs->utmrd_addr->task_req_upiu.flags = UPIU_CMD_FLAGS_NONE;
		ufs->utmrd_addr->task_req_upiu.lun = ufs->lun;
		ufs->utmrd_addr->task_req_upiu.tag = 0;	/* queuing is not used */
		ufs->utmrd_addr->task_req_upiu.function = function;
		ufs->utmrd_addr->task_req_upiu.tsf[0] = cpu_to_be32(ufs->lun);
		ufs->utmrd_addr->task_req_upiu.tsf[1] = 0;	/* queuing is not used */
		break;
	case UPIU_TRANSACTION_QUERY_REQ:
		/* For query function implementation */
		ufs->utrd_addr->dw[0] = UTP_REQ_DESC_INT_CMD;
		ufs->utrd_addr->dw[2] = (u32)(OCS_INVALID_COMMAND_STATUS);

		ufs->cmd_desc_addr->command_upiu.header.type = type;
		ufs->cmd_desc_addr->command_upiu.header.flags = UPIU_CMD_FLAGS_NONE;
		ufs->cmd_desc_addr->command_upiu.header.tag = 0;		/* queuing is not used */
		ufs->cmd_desc_addr->command_upiu.header.function = function;
		p[0] = opcode;	/* opcode */
		p[1] = descid;	/* descripot id */
		p[2] = ufs->lun;

		ufs_set_specific_fields_in_query_upiu(ufs);
		break;
	case UPIU_TRANSACTION_NOP_OUT:
	default:
		break;
	}
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
	enum ufs_transaction_code code = ufs->command_type & 0xff;

	/*
	 * Check completion
	 */
	switch (code) {
	case UPIU_TRANSACTION_NOP_OUT:
	case UPIU_TRANSACTION_COMMAND:
	case UPIU_TRANSACTION_QUERY_REQ:
		if (intr_stat & UTP_TRANSFER_REQ_COMPL) {
			if (!(readl(ufs->ioaddr +
				REG_UTP_TRANSFER_REQ_DOOR_BELL) & 1))
				ret = UFS_NO_ERROR;
		}
		break;
	case UPIU_TRANSACTION_TASK_REQ:
		if (intr_stat & UTP_TASK_REQ_COMPL) {
			if (!(readl(ufs->ioaddr +
				REG_UTP_TASK_REQ_DOOR_BELL) & 1))
				ret = UFS_NO_ERROR;
		}
		break;
	default:
		printf("UFS: UNEXPECTED TRANSACTION CODE 0x%08x\n", code);
		ret = UFS_ERROR;
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

static void print_scsi_cmd(scm * pscm)
{
	int i, len;
	if (!pscm)
		return;

	dprintf(INFO, "LUN %d\n\tCMD(%u):", pscm->sdev->lun, pscm->cdb[0]);
	for (i = 0; i < MAX_CDB_SIZE; i++) {
		dprintf(INFO, " %02x", pscm->cdb[i]);
	}
	dprintf(INFO, "\n");
	if (pscm->datalen) {
		len = pscm->datalen;
		if (len > 16)
			len = 16;
		dprintf(INFO, "\tData(%d):", pscm->datalen);
		for (i = 0; i < len; i++) {
			dprintf(INFO, " %02x", pscm->buf[i]);
		}
		dprintf(INFO, "\n");
	}
}

static void print_ufs_hci_sfr(struct ufs_host *ufs, int level)
{
	int i, len;

	printf("UFS registers\n");
	if (level > 1)
		len = 0x200;
	else
		len = 0x100;
	for (i = 0; i < len / 4; i++) {
		if ((i & 3) == 0)
			printf("%08llx :", ((u64) (ufs->ioaddr) + 4 * i));
		printf(" %08x", *((u32 *) (ufs->ioaddr) + i));
		if ((i & 3) == 3)
			printf("\n");
	}
}

static void print_ufs_hci_utrd(struct ufs_host *ufs, int level)
{
	int i;

	printf("UTP Transfer Request Descriptor (0x%08lx)\n", (ulong)ufs->utrd_addr);
	for (i = 0; i < 8; i++) {
		printf(" %08x", *((u32 *) (ufs->utrd_addr) + i));
		if ((i & 3) == 3)
			printf("\n");
	}
}

static void print_ufs_hci_cmd(struct ufs_host *ufs, int level)
{
	int i, len;

	len = level * 4;
	printf("UTP Command Descriptor\nCommand UPIU (0x%08lx)\n",
			(ulong)&(ufs->cmd_desc_addr->command_upiu));
	for (i = 0; i < len; i++) {
		printf(" %08x", *((u32 *) (&ufs->cmd_desc_addr->command_upiu) + i));
		if ((i & 3) == 3)
			printf("\n");
	}
}

static void print_ufs_hci_resp(struct ufs_host *ufs, int level)
{
	int i, len;

	len = level * 4;
	printf("Response UPIU (0x%08lx)\n", (ulong)&(ufs->cmd_desc_addr->response_upiu));
	for (i = 0; i < len; i++) {
		printf(" %08x", *((u32 *) (&ufs->cmd_desc_addr->response_upiu) + i));
		if ((i & 3) == 3)
			printf("\n");
	}
}

static void print_ufs_hci_prdt(struct ufs_host *ufs, int level)
{
	int i, len;

	len = ufs->utrd_addr->prdt_len / 4;
	if (len > 4 * level)
		len = 4 * level;
	printf("PRDT (0x%08lx) length %d\n", (ulong)ufs->cmd_desc_addr->prd_table,
			ufs->utrd_addr->prdt_len);
	for (i = 0; i < len; i++) {
		printf(" %08x", *((u32 *) (ufs->cmd_desc_addr->prd_table) + i));
		if ((i & 3) == 3)
			printf("\n");
	}
}

static void print_ufs_hci_data(struct ufs_host *ufs, int level)
{
	u64 prdt_addr;
	int i, len;

	prdt_addr = ((u64) ufs->cmd_desc_addr->prd_table[0].base_addr) |
		((u64) ufs->cmd_desc_addr->prd_table[0].upper_addr << UFS_BIT_LEN_OF_DWORD);

	len = ufs->cmd_desc_addr->prd_table[0].size + 1;
	printf("Data (addr=0x%016llx, total length=%d, first prd length=%d)\n", prdt_addr,
			(u32)(ufs->cmd_desc_addr->command_upiu.header.tsf[0]), len);
	if (len > 16 * level)
		len = 16 * level;
	for (i = 0; i < len; i++) {
		printf(" %02x",
				*((u8 *) (prdt_addr + i)));
		if ((i & 15) == 15)
			printf("\n");
	}
}
static void print_ufs_upiu(int print_level)
{
	u32 level;
	struct ufs_host *ufs = get_cur_ufs_host();

	if (!ufs)
		return;

	dprintf(INFO, "\n");

	level = print_level & 0x7;
	if (level)
		print_ufs_hci_sfr(ufs, level);

	level = (print_level >> 4) & 0x7;
	if (level)
		print_ufs_hci_utrd(ufs, level);

	level = (print_level >> 8) & 0x7;
	if (level++)
		print_ufs_hci_cmd(ufs, level);

	level = (print_level >> 12) & 0x7;
	if (level++)
		print_ufs_hci_resp(ufs, level);

	if (ufs->utrd_addr->prdt_len) {
		level = (print_level >> 16) & 0x7;
		if (level)
			print_ufs_hci_prdt(ufs, level);

		level = (print_level >> 20) & 0x7;
		if (level++)
			print_ufs_hci_data(ufs, level);
	}

	level = (print_level >> 24) & 0x7;
	if (level) {
		print_scsi_cmd(ufs->scsi_cmd);
	}
	dprintf(INFO, "\n");
}

static void print_ufs_desc(u8 * desc)
{
	int i, len, type;
	if (!desc)
		return;

	len = desc[0];
	type = desc[1];

	dprintf(INFO, "Descriptor length 0x%02x, type 0x%02x\n", len, type);

	for (i = 0; i < len; i++) {
		if ((i & 0xf) == 0)
			dprintf(INFO, "%02x :", i);
		dprintf(INFO, " %02x", desc[i]);
		if ((i & 0xf) == 0xf)
			dprintf(INFO, "\n");
	}
}

static void print_ufs_device_desc(u8 * desc)
{
	struct ufs_device_desc *dp = (struct ufs_device_desc *)desc;
	if (!desc)
		return;

	printf("Device Descriptor\n");
	printf("------------------------------------------------------\n");
	printf("bLength = 0x%02x (1Fh)\n", dp->bLength);
	printf("bDescriptorType = 0x%02x (00h)\n", dp->bDescriptorType);
	printf("bDevice = 0x%02x (00h device, others reserved)\n", dp->bDevice);
	printf("bDeviceClass = 0x%02x (00h Mass storage)\n", dp->bDeviceClass);
	printf
	    ("bDeviceSubClass = 0x%02x (00h embedded bootable, 01h embedded non-bootable, 02h removable non-bootable)\n",
	     dp->bDeviceSubClass);
	printf("bProtocol = 0x%02x (00h scsi)\n", dp->bProtocol);
	printf("bNumberLU = 0x%02x (01h~08h, default 01h)\n", dp->bNumberLU);
	printf("iNumberWLU = 0x%02x (04h)\n", dp->iNumberWLU);
	printf("bBootEnable = 0x%02x (00h boot disabled, 01h boot enabled)\n", dp->bBootEnable);
	printf("bDescrAccessEn = 0x%02x (00h device desc access disabled, 01h access enabled)\n",
	       dp->bDescrAccessEn);

	printf("bInitPowerMode = 0x%02x (00h UFS sleep mode, 01h active mode)\n",
	       dp->bInitPowerMode);
	printf("bHighPriorityLUN = 0x%02x (00h~07h, 7Fh same priority)\n", dp->bHighPriorityLUN);
	printf("bSecureRemovalType = 0x%02x (00h~03h)\n", dp->bSecureRemovalType);
	printf("bSecurityLU = 0x%02x (00h not support, 01h RPMB)\n", dp->bSecurityLU);
	printf("bInitActiveICCLevel = 0x%02x (00h~0Fh, default 00h)\n", dp->bInitActiveICCLevel);
	printf("wSpecVersion = 0x%04x\n", be16_to_cpu(dp->wSpecVersion));
	printf("wManufactureData = 0x%04x (MM/YY)\n", be16_to_cpu(dp->wManufactureData));
	printf("iManufacturerName = 0x%02x\n", dp->iManufacturerName);
	printf("iProductName = 0x%02x\n", dp->iProductName);
	printf("iSerialNumber = 0x%02x\n", dp->iSerialNumber);

	printf("iOemID = 0x%02x\n", dp->iOemID);
	printf("wManufacturerID = 0x%04x\n", be16_to_cpu(dp->wManufacturerID));
	printf("bUD0BaseOffset = 0x%02x (10h)\n", dp->bUD0BaseOffset);
	printf("bUDConfigPlength = 0x%02x (10h)\n", dp->bUDConfigPlength);
	printf("bDeviceRTTCap = 0x%02x (minimum value 02h)\n", dp->bDeviceRTTCap);
	printf("wPeriodicRTCUpdate = 0x%04x\n", be16_to_cpu(dp->wPeriodicRTCUpdate));
	printf("------------------------------------------------------\n");
}

static void print_ufs_configuration_desc(u8 * desc)
{
	int i;
	struct ufs_config_desc *dp = (struct ufs_config_desc *)desc;
	if (!desc)
		return;

	printf("Configuration Descriptor\n");
	printf("------------------------------------------------------\n");
	printf("Configuration Descriptor Header\n");
	printf("bLength = 0x%02x (90h)\n", dp->header.bLength);
	printf("bDescriptorType = 0x%02x (01h)\n", dp->header.bDescriptorType);
	printf("bConfDescContinue = 0x%02x\n", dp->header.bConfDescContinue);
	printf("bBootEnable = 0x%02x\n", dp->header.bBootEnable);
	printf("bDescrAccessEn = 0x%02x\n", dp->header.bDescrAccessEn);
	printf("bInitPowerMode = 0x%02x\n", dp->header.bInitPowerMode);
	printf("bHighPriorityLUN = 0x%02x\n", dp->header.bHighPriorityLUN);
	printf("bSecureRemovalType = 0x%02x\n", dp->header.bSecureRemovalType);
	printf("bInitActiveICCLevel = 0x%02x\n", dp->header.bInitActiveICCLevel);
	printf("wPeriodicRTCUpdate = 0x%04x\n", be16_to_cpu(dp->header.wPeriodicRTCUpdate));

	for (i = 0; i < 8; i++) {
		printf("Unit Descriptor configurable parameters (%02xh)\n", i);
		printf("\tbLUEnable = 0x%02x\n", dp->unit[i].bLUEnable);
		printf("\tbBootLunID = 0x%02x\n", dp->unit[i].bBootLunID);
		printf("\tbLUWriteProtect = 0x%02x\n", dp->unit[i].bLUWriteProtect);
		printf("\tbMemoryType = 0x%02x\n", dp->unit[i].bMemoryType);
		printf("\tdNumAllocUnits = 0x%08x\n", be32_to_cpu(dp->unit[i].dNumAllocUnits));
		printf("\tbDataReliability = 0x%02x\n", dp->unit[i].bDataReliability);
		printf("\tbLogicalBlockSize = 0x%02x\n", dp->unit[i].bLogicalBlockSize);
		printf("\tbProvisioningType = 0x%02x\n", dp->unit[i].bProvisioningType);
		printf("\twContextCapabilities = 0x%04x\n",
		       be16_to_cpu(dp->unit[i].wContextCapabilities));
	}
	dprintf(INFO, "------------------------------------------------------\n");
}

static void print_ufs_geometry_desc(u8 * desc)
{
	struct ufs_geometry_desc *dp = (struct ufs_geometry_desc *)desc;
	if (!desc)
		return;

	printf("Geometry Descriptor\n");
	printf("------------------------------------------------------\n");
	printf("bLength = 0x%02x (44h)\n", dp->bLength);
	printf("bDescriptorType = 0x%02x (07h)\n", dp->bDescriptorType);
	printf("bMediaTechnology = 0x%02x (00h reserved)\n", dp->bMediaTechnology);
	printf("qTotalRawDeviceCapacity = 0x%08x%08x (512bytes)\n",
	       be32_to_cpu(dp->qTotalRawDeviceCapacity_h),
	       be32_to_cpu(dp->qTotalRawDeviceCapacity_l));
	printf("dSegmentSize = 0x%08x (512bytes)\n", be32_to_cpu(dp->dSegmentSize));
	printf("bAllocationUnitSize = 0x%02x (segments)\n", dp->bAllocationUnitSize);
	printf("bMinAddrBlockSize = 0x%02x (512bytes, minimum value 08h=4KB)\n",
	       dp->bMinAddrBlockSize);
	printf("bOptimalReadBlockSize = 0x%02x (512bytes, 0=not available)\n",
	       dp->bOptimalReadBlockSize);
	printf("bOptimalWriteBlockSize = 0x%02x (512bytes)\n", dp->bOptimalWriteBlockSize);
	printf("bMaxInBufferSize = 0x%02x (512bytes, minimum value 08h=4KB)\n",
	       dp->bMaxInBufferSize);
	printf("bMaxOutBufferSize = 0x%02x (512bytes, minimum value 08h=4KB)\n",
	       dp->bMaxOutBufferSize);
	printf("bRPMB_ReadWriteSize = 0x%02x\n", dp->bRPMB_ReadWriteSize);
	printf("bDataOrdering = 0x%02x (00h not support, 01h support)\n", dp->bDataOrdering);
	printf("bMaxContexIDNumber = 0x%02x (minimum value 05h)\n", dp->bMaxContexIDNumber);
	printf("bSysDataTagUnitSize = 0x%02x\n", dp->bSysDataTagUnitSize);
	printf("bSysDataTagResSize = 0x%02x\n", dp->bSysDataTagResSize);
	printf("bSupportedSecRTypes = 0x%02x\n", dp->bSupportedSecRTypes);
	printf("wSupportedMemoryTypes = 0x%04x\n", be16_to_cpu(dp->wSupportedMemoryTypes));
	printf("dSystemCodeMaxNAllocU = 0x%08x\n", be32_to_cpu(dp->dSystemCodeMaxNAllocU));
	printf("wSystemCodeCapAdjFac = 0x%04x\n", be16_to_cpu(dp->wSystemCodeCapAdjFac));
	printf("dNonPersistMaxNAllocU = 0x%08x\n", be32_to_cpu(dp->dNonPersistMaxNAllocU));
	printf("wNonPersistCapAdjFac = 0x%04x\n", be16_to_cpu(dp->wNonPersistCapAdjFac));
	printf("dEnhanced1MaxNAllocU = 0x%08x\n", be32_to_cpu(dp->dEnhanced1MaxNAllocU));
	printf("wEnhanced1CapAdjFac = 0x%0x\n", be16_to_cpu(dp->wEnhanced1CapAdjFac));
	printf("dEnhanced2MaxNAllocU = 0x%08x\n", be32_to_cpu(dp->dEnhanced2MaxNAllocU));
	printf("wEnhanced2CapAdjFac = 0x%04x\n", be16_to_cpu(dp->wEnhanced2CapAdjFac));
	printf("dEnhanced3MaxNAllocU = 0x%08x\n", be32_to_cpu(dp->dEnhanced3MaxNAllocU));
	printf("wEnhanced3CapAdjFac = 0x%04x\n", be16_to_cpu(dp->wEnhanced3CapAdjFac));
	printf("dEnhanced4MaxNAllocU = 0x%08x\n", be32_to_cpu(dp->dEnhanced4MaxNAllocU));
	printf("wEnhanced4CapAdjFac = 0x%04x\n", be16_to_cpu(dp->wEnhanced4CapAdjFac));
	printf("------------------------------------------------------\n");
}

static void print_ufs_config_desc_units(struct ufs_config_desc *desc, u32 alloc_unit)
{
	int i;
	u32 value;

	printf("----------------------------------------------------------------------\n");
	printf("\t\t\tLUN0\tLUN1\tLUN2\tLUN3\tLUN4\tLUN5\tLUN6\tLUN7\n");
	printf("LU en");
	for (i = 0; i < 8; i++) {
		printf("\t\t");
		if (desc->unit[i].bLUEnable == 1)
			printf("en");
		else
			printf("dis");
	}
	printf("\n");

	printf("Boot\t");
	for (i = 0; i < 8; i++) {
		printf("\t");
		switch (desc->unit[i].bBootLunID) {
		case 1:
			printf("BootA");
			break;
		case 2:
			printf("BootB");
			break;
		default:
			printf("\t");
			break;
		}
	}
	printf("\n");

	printf("WP\t");
	for (i = 0; i < 8; i++) {
		printf("\t\t");
		switch (desc->unit[i].bLUWriteProtect) {
		case 1:
			printf("WP");
			break;
		case 2:
			printf("perWP");
			break;
		}
	}
	printf("\n");

	printf("Type\t");
	for (i = 0; i < 8; i++) {
		printf("\t");
		switch (desc->unit[i].bMemoryType) {
		case 0:
			printf("Normal");
			break;
		case 1:
			printf("SysCode");
			break;
		case 2:
			printf("NonPer");
			break;
		default:
			printf("Type%d", desc->unit[i].bMemoryType - 2);
			break;
		}
	}
	printf("\n");

	printf("Capa");
	for (i = 0; i < 8; i++) {
		printf("\t\t");
		value = be32_to_cpu(desc->unit[i].dNumAllocUnits) * alloc_unit;
		if (value < 1024)
			printf("%dK", value);
		else if (value < 1024 * 1024)
			printf("%dM", value / 1024);
		else
			printf("%dG", value / (1024 * 1024));
	}
	printf("\n");

	printf("BlSize");
	for (i = 0; i < 8; i++) {
		printf("\t\t");
		value = 1 << desc->unit[i].bLogicalBlockSize;
		if (value < 1024)
			printf("%dK", value);
		else if (value < 1024 * 1024)
			printf("%dM", value / 1024);
		else
			printf("%dG", value / (1024 * 1024));
	}
	printf("\n");
}

static void print_ufs_config_desc_summary(struct ufs_config_desc *desc, u32 alloc_unit)
{
	printf("bConfDescContinue \t\t\t%d ea\n", desc->header.bConfDescContinue);
	if (desc->header.bBootEnable == 1)
		printf("Boot feature\t\t\tenabled\n");
	else
		printf("Boot feature\t\t\tdisabled\n");
	if (desc->header.bDescrAccessEn == 1)
		printf("Descriptor access\t\tenabled\n");
	else
		printf("Descriptor access\t\tdisabled\n");
	if (desc->header.bInitPowerMode == 1)
		printf("Initial Power Mode\t\tActive Mode\n");
	else
		printf("Initial Power Mode\t\tUFS-Sleep Mode\n");
	if (desc->header.bHighPriorityLUN == 0x7f)
		printf("All logical unit have the same priority\n");
	else
		printf("High priority logical unit\t%d\n",
		       desc->header.bHighPriorityLUN);

	print_ufs_config_desc_units(desc, alloc_unit);
}

#if 0
static void print_ufs_information(void)
{
	int i;
	u32 capacity, alloc_unit, value;
	struct ufs_host *ufs = get_cur_ufs_host();
	if (!ufs)
		return;

	dprintf(INFO, "----------------------------------------------------------------------\n");
	dprintf(INFO, "UFS device information\n");
	dprintf(INFO, "----------------------------------------------------------------------\n");
	capacity = (2048 * 1024) * be32_to_cpu(ufs->geometry_desc.qTotalRawDeviceCapacity_h)
	    + be32_to_cpu(ufs->geometry_desc.qTotalRawDeviceCapacity_l) / (2 * 1024);
	dprintf(INFO, "Capacity\t\t\t%d Gbytes (%dMbytes)\n", capacity / 1024, capacity);
	dprintf(INFO, "Erase block size\t\t%d Kbytes\n", be32_to_cpu(ufs->geometry_desc.dSegmentSize) / 2);
	alloc_unit =
	    ufs->geometry_desc.bAllocationUnitSize * be32_to_cpu(ufs->geometry_desc.dSegmentSize) /
	    2;
	dprintf(INFO, "Allocation unit size\t\t%d Kbytes\n", alloc_unit);
	dprintf(INFO, "Address block size\t\t%d Kbytes\n", ufs->geometry_desc.bMinAddrBlockSize / 2);
	dprintf(INFO, "Optimal read block size\t\t%d Kbytes\n",
	       ufs->geometry_desc.bOptimalReadBlockSize / 2);
	dprintf(INFO, "Optimal write block size\t%d Kbytes\n",
	       ufs->geometry_desc.bOptimalReadBlockSize / 2);
	dprintf(INFO, "Supported memory type\n");
	value = be16_to_cpu(ufs->geometry_desc.wSupportedMemoryTypes);
	if (value & (1 << 0))
		dprintf(INFO, "\tNormal memory\n");
	if (value & (1 << 1))
		dprintf(INFO, "\tSystem code memory\n");
	if (value & (1 << 2))
		dprintf(INFO, "\tNon-Persistent memory\n");
	if (value & (1 << 3))
		dprintf(INFO, "\tEnhanced memory memory type 1\n");
	if (value & (1 << 4))
		dprintf(INFO, "\tEnhanced memory memory type 2\n");
	if (value & (1 << 5))
		dprintf(INFO, "\tEnhanced memory memory type 3\n");
	if (value & (1 << 6))
		dprintf(INFO, "\tEnhanced memory memory type 4\n");
	if (value & (1 << 15))
		dprintf("\tRPMB memory\n");

	print_ufs_config_desc_summary(&ufs->config_desc, alloc_unit);

	printf("----------------------------------------------------------------------\n");
}
#endif

static void print_ufs_flags(void)
{
	struct ufs_host *ufs = get_cur_ufs_host();
	if (!ufs)
		return;

	dprintf(INFO, "----------------------------------------------------------------------\n");
	dprintf(INFO, "UFS device flags\n");
	dprintf(INFO, "----------------------------------------------------------------------\n");
	dprintf(INFO, "UFS flag : fDeviceInit: %d\n", ufs->flags.flag.fDeviceInit);
	dprintf(INFO, "UFS flag : fPermanentWPEn: %d\n", ufs->flags.flag.fPermanentWPEn);
	dprintf(INFO, "UFS flag : fPowerOnWPEn: %d\n", ufs->flags.flag.fPowerOnWPEn);
	dprintf(INFO, "UFS flag : fBackgroundOpsEn: %d\n", ufs->flags.flag.fBackgroundOpsEn);
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

static int send_cmd(struct ufs_host *ufs)
{
	int err = UFS_IN_PROGRESS;

	switch (ufs->command_type & 0xff) {
	case UPIU_TRANSACTION_NOP_OUT:
	case UPIU_TRANSACTION_QUERY_REQ:
		writel(0x0, (ufs->vs_addr + VS_UTRL_NEXUS_TYPE));
		break;
	case UPIU_TRANSACTION_COMMAND:
		writel(0xFFFFFFFF, (ufs->vs_addr + VS_UTRL_NEXUS_TYPE));
	case UPIU_TRANSACTION_TASK_REQ:
	default:
		break;
	}

	switch (ufs->command_type & 0xff) {
	case UPIU_TRANSACTION_NOP_OUT:
	case UPIU_TRANSACTION_COMMAND:
	case UPIU_TRANSACTION_QUERY_REQ:
		writel(1, (ufs->ioaddr + REG_UTP_TRANSFER_REQ_DOOR_BELL));
		break;
	case UPIU_TRANSACTION_TASK_REQ:
		writel(1, (ufs->ioaddr + REG_UTP_TASK_REQ_DOOR_BELL));
		break;
	}

	ufs->timeout = ufs->ufs_cmd_timeout;

	/* FORMAT_UNIT should have longer timeout, 10 min */
	if (ufs->scsi_cmd && ufs->scsi_cmd->cdb[0] == SCSI_OP_FORMAT_UNIT)
		ufs->timeout = 10 * 60 * 1000 * 1000;
	while (UFS_IN_PROGRESS == (err = handle_ufs_int(ufs, 0)))
		;
	writel(readl(ufs->ioaddr + REG_INTERRUPT_STATUS),
			ufs->ioaddr + REG_INTERRUPT_STATUS);

	switch (ufs->command_type & 0xff) {
	case UPIU_TRANSACTION_NOP_OUT:
	case UPIU_TRANSACTION_QUERY_REQ:
		writel((readl(ufs->ioaddr + 0x140) | 0x01), (ufs->ioaddr + 0x140));
		break;
	case UPIU_TRANSACTION_COMMAND:
		memcpy(ufs->scsi_cmd->sense_buf,
		       (u8 *) (ufs->cmd_desc_addr->response_upiu.data) + 2, 18);
		break;
	case UPIU_TRANSACTION_TASK_REQ:
	default:
		break;
	}

	return err;
}

static status_t ufs_parse_respnse(struct ufs_host *ufs)
{
	scm *pscm = NULL;

	pscm = ufs->scsi_cmd;
	if (!ufs || !pscm)
		return ERR_GENERIC;

	if (ufs->utrd_addr->dw[2]) {
		dprintf(INFO, "SCSI cdb : %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
		       pscm->cdb[0], pscm->cdb[1], pscm->cdb[2], pscm->cdb[3],
		       pscm->cdb[4], pscm->cdb[5], pscm->cdb[6], pscm->cdb[7],
		       pscm->cdb[8], pscm->cdb[9]);
		dprintf(INFO, "SCSI Response(%02x) : ", ufs->cmd_desc_addr->response_upiu.header.response);
		dprintf(INFO, "SCSI sense : %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
				pscm->sense_buf[0], pscm->sense_buf[1], pscm->sense_buf[2], pscm->sense_buf[3], pscm->sense_buf[4],
				pscm->sense_buf[5], pscm->sense_buf[6], pscm->sense_buf[7], pscm->sense_buf[8], pscm->sense_buf[9],
				pscm->sense_buf[10], pscm->sense_buf[11], pscm->sense_buf[12], pscm->sense_buf[13], pscm->sense_buf[14],
				pscm->sense_buf[15], pscm->sense_buf[16], pscm->sense_buf[17]);

		if (ufs->cmd_desc_addr->response_upiu.header.response) {
			dprintf(INFO, "Target Failure\n");
		} else {
			dprintf(INFO, "Target success\n");
		}
	}
	return NO_ERROR;
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
	int retry = 0;
	status_t err;

	do {
		ufs = get_cur_ufs_host();
		ufs->scsi_cmd = pscm;
		ufs->sense_buffer = pscm->sense_buf;
		ufs->sense_buflen = 64;	/* defined in include/scsi.h */
		if (pscm->sdev->lun == 0x44)
			ufs->lun = 0xC4;
		else if (pscm->sdev->lun == 0x50)
			ufs->lun = 0xD0;
		else
			ufs->lun = pscm->sdev->lun;
		ufs->command_type = UFS_SEND_CMD;

		memset(ufs->cmd_desc_addr, 0x00, sizeof(struct ufs_cmd_desc));

		ufs_compose_upiu(ufs);
		err = ufs_map_sg(ufs);
		if (err)
			return err;

		err = send_cmd(ufs);

#ifdef	SCSI_UFS_DEBUG
		print_ufs_upiu(UFS_DEBUG_UPIU);
#endif

		if (err) {
			/* Re-init UFS & retry transection */
			ufs_link_startup(ufs);
			dprintf(INFO, "%s: error %d, retrying %d ... \n", __func__, err, ++retry);
		} else {
			retry = 0;
		}
	} while (retry);

	/* Update SCSI status. SCSI would handle it.. */
	pscm->status = ufs->cmd_desc_addr->response_upiu.header.status;

	if (!err)
		err = ufs_parse_respnse(ufs);

	return err;
}

/*
 * CALLBACK FUNCTION: scsi_exec
 *
 * This is called for SCSI stack to get a Logical Unit for Start Stop Unit
 * command execution. This is registered in SCSI stack
 * when executing scsi_scan().
 */

static scsi_device_t *scsi_get_ssu_sdev(void)
{
	return (struct scsi_device_s *)&ufs_dev_ssu;
}

static void ufs_bootlun_enable(int enable)
{
	if (enable)
		ufs_send_upiu(UFS_SEND_WRITE_BOOTLUNEN_ATTR, 1);
	else
		ufs_send_upiu(UFS_SEND_WRITE_BOOTLUNEN_ATTR, 0);
}

static int setufs_boot(u32 enable)
{
	if (enable == 1) {
		ufs_bootlun_enable(1);
		return NO_ERROR;
	} else if (enable == 0) {
		ufs_bootlun_enable(0);
		return NO_ERROR;
	}
	return ERR_GENERIC;
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
	ufs_send_upiu(UFS_SEND_READ_GEOMETRY_DESC, 1);

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

#if 0
static void ufs_setup_cmd(scm * pscm, u8 scsi_cmd)
{
	pscm->cmd[0] = scsi_cmd;
	if (scsi_cmd < 0x20)
		pscm->cmdlen = 6;
	if (scsi_cmd >= 0x20 && scsi_cmd < 0x60)
		pscm->cmdlen = 10;
	if (scsi_cmd >= 0x80 && scsi_cmd < 0xa0)
		pscm->cmdlen = 16;
	if (scsi_cmd >= 0xa0 && scsi_cmd < 0xc0)
		pscm->cmdlen = 12;
}
#endif

static void ufs_pre_send_query(struct ufs_host *ufs,
					ufs_upiu_cmd cmd_type, int enable)
{
	switch ((cmd_type >> 16) & 0xff) {
	case UPIU_QUERY_OPCODE_WRITE_DESC:
		switch ((cmd_type >> 24) & 0xff) {
		case UPIU_DESC_ID_CONFIGURATION:
			ufs->data_seg_len = 0x90;
			memcpy(ufs->upiu_data, &ufs->config_desc, ufs->data_seg_len);
			printf("UPIU_QUERY_OPCODE_WRITE_DESC : len%02x\n",
				ufs->data_seg_len);
			break;
		}
		break;
	case UPIU_QUERY_OPCODE_WRITE_ATTR:
		if (((cmd_type >> 24) & 0xff) == UPIU_ATTR_ID_BOOTLUNEN) {
			if (enable) {
				ufs->attributes.arry[UPIU_ATTR_ID_BOOTLUNEN] = 0x01;
				printf("UPIU_ATTR_ID_BOOTLUNEN : %08x\n",
						ufs->attributes.arry[UPIU_ATTR_ID_BOOTLUNEN]);
			} else {
				ufs->attributes.arry[UPIU_ATTR_ID_BOOTLUNEN] = 0x00;
				printf("UPIU_ATTR_ID_BOOTLUNEN : %08x\n",
						ufs->attributes.arry[UPIU_ATTR_ID_BOOTLUNEN]);
			}
		} else if (((cmd_type >> 24) & 0xff) == UPIU_ATTR_ID_CONFIGDESCLOCK) {
			ufs->attributes.arry[UPIU_ATTR_ID_CONFIGDESCLOCK] = 0x01;
			printf("UPIU_ATTR_ID_CONFIGDESCLOCK : %08x\n",
					ufs->attributes.arry[UPIU_ATTR_ID_CONFIGDESCLOCK]);

		/* Exynos always use reference clock attr as 0x01 */
		} else if (((cmd_type >> 24) & 0xff) == UPIU_ATTR_ID_REFCLKFREQ) {
			ufs->attributes.arry[UPIU_ATTR_ID_REFCLKFREQ] = 0x01;
			printf("UPIU_ATTR_ID_REFCLKFREQ : %08x\n",
					ufs->attributes.arry[UPIU_ATTR_ID_REFCLKFREQ]);
		}
		break;
	}
}

static void ufs_post_send_query(struct ufs_host *ufs,
						ufs_upiu_cmd cmd_type, u8 *p)
{
	u8 index;
	u32 value;

	switch ((cmd_type >> 16) & 0xff) {
	case UPIU_QUERY_OPCODE_READ_DESC:
		switch ((cmd_type >> 24) & 0xff) {
		case UPIU_DESC_ID_UNIT:
			index = *((u8 *) (ufs->cmd_desc_addr->response_upiu.data) + 2);
			if (index > 7) {
				printf("bUnitIndex : 0x%02x\n", index);
				break;
			}
			memcpy(&ufs->unit_desc[index], ufs->cmd_desc_addr->response_upiu.data,
					MIN(*(u8 *)(ufs->cmd_desc_addr->response_upiu.data),
						sizeof(struct ufs_unit_desc)));
			break;
		case UPIU_DESC_ID_DEVICE:
			memcpy(&ufs->device_desc, ufs->cmd_desc_addr->response_upiu.data,
					MIN(*(u8 *) (ufs->cmd_desc_addr->response_upiu.data),
						sizeof(struct ufs_device_desc)));
			//print_ufs_device_desc(&ufs->device_desc);
			break;
		case UPIU_DESC_ID_CONFIGURATION:
			memcpy(&ufs->config_desc, ufs->cmd_desc_addr->response_upiu.data, 0x90);
			//print_ufs_configuration_desc(&ufs->config_desc);
			break;
		case UPIU_DESC_ID_GEOMETRY:
			memcpy(&ufs->geometry_desc, ufs->cmd_desc_addr->response_upiu.data,
					MIN(*(u8 *) (ufs->cmd_desc_addr->response_upiu.data),
						sizeof(struct ufs_geometry_desc)));
			//print_ufs_geometry_desc(&ufs->geometry_desc);
			break;
		}
		//print_ufs_desc(ufs->cmd_desc_addr->response_upiu.data);
		break;
	case UPIU_QUERY_OPCODE_READ_ATTR:
		value = UPIU_HEADER_DWORD((u32) p[8], (u32) p[9], (u32) p[10], (u32) p[11]);
		index = p[1];
#ifdef SCSI_UFS_DEBUG
		printf("ATTRIBUTE IDN(0x%02x) = 0x%08x\n", index, value);
#endif
		ufs->attributes.arry[index] = value;
		break;
	case UPIU_QUERY_OPCODE_READ_FLAG:
		value = p[11];
		index = p[1];
#ifdef SCSI_UFS_DEBUG
		printf("FLAG IDN(0x%02x) = %x\n", index, value);
#endif
		ufs->flags.arry[index] = value;
		break;
	}
}

static int ufs_send_upiu(ufs_upiu_cmd cmd, int enable)
{
	int res = 0;
	struct ufs_host *ufs = get_cur_ufs_host();
	u8 *p = NULL;
	//scm tccb;

	if (!ufs)
		return res;

	ufs->command_type = cmd;

	switch (cmd & 0xff) {
	case UPIU_TRANSACTION_QUERY_REQ:
		/*
		 * For Device management command, LUN is meaningless.
		 */
		p = (u8 *)ufs->cmd_desc_addr->response_upiu.header.tsf;
		memset(ufs->cmd_desc_addr, 0x00, sizeof(struct ufs_cmd_desc));
		ufs->lun = 0;
		ufs->scsi_cmd = 0;
		ufs_pre_send_query(ufs, cmd, enable);
		break;
	default:
		break;
	}

	ufs_compose_upiu(ufs);
	ufs_map_sg(ufs);

#ifdef	SCSI_UFS_DEBUG
	print_ufs_upiu(UFS_DEBUG_UPIU_ALL2);
#endif

	res = send_cmd(ufs);

#ifdef	SCSI_UFS_DEBUG
	print_ufs_upiu(UFS_DEBUG_UPIU_ALL2);
#endif

/*
	command_type :
	| descriptor ID | opcode | function | UPIU type |

	UPIU type[7:0] : NOP_OUT, COMMAND, DATA_OUT, TASK_REQ, QUERY_REQ
	function[15:8] : Used by QUERY_REQ / TASK_REQ
	opcode[23:16] : Opcode in query / input parameter1 (TASK_REQ)
	descriptor ID[31:24] : descriptor ID in query / input parameter2 (TASK_REQ)

*/
	switch (cmd & 0xff) {
	case UPIU_TRANSACTION_QUERY_REQ:
		if (ufs->cmd_desc_addr->response_upiu.header.response) {
			printf("lun:%d Query Response : 0x%02x\n", ufs->lun,
			       ufs->cmd_desc_addr->response_upiu.header.response);
			res = ERR_GENERIC;
			break;
		}
		ufs_post_send_query(ufs, cmd, p);
		break;
	case UPIU_TRANSACTION_TASK_REQ:
		if (ufs->utmrd_addr->task_rsp_upiu.response) {
			printf("Task Management Request Fail : 0x%02x 0x%02x\n",
			       ufs->utmrd_addr->task_rsp_upiu.response, p[3]);
			break;
		}
		break;
	case UPIU_TRANSACTION_COMMAND:
		break;
	default:
		break;
	}

	return res;
}

static int ufs_send_upiu_with_lun(ufs_upiu_cmd cmd, u8 lun)
{
	struct ufs_host *ufs = get_cur_ufs_host();
	ufs->lun = lun;

	return ufs_send_upiu(cmd, 1);
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

	res = ufs_send_upiu(UFS_SEND_NOP, 1);
	if (res) {
		dprintf(INFO, "UFS: NOP OUT failed\n");
		return res;
	}

	ufs->flags.arry[UPIU_FLAG_ID_DEVICEINIT] = flag;
	res = ufs_send_upiu_with_lun(UFS_SEND_SET_DEVICEINIT_FLAG, 0);
	if (res)
		return res;

	do {
		ufs_send_upiu_with_lun(UFS_SEND_READ_DEVICEINIT_FLAG, 0);
		flag = ufs->flags.arry[UPIU_FLAG_ID_DEVICEINIT];
		retry--;
	} while (flag && retry > 0);

	if (flag) {
		dprintf(INFO, "UFS: fdeviceinit faild\n");
		return ERR_GENERIC;
	}

	return NO_ERROR;
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
	memset(ufs->utmrd_addr, 0x00, UFS_NUTMRS*sizeof(struct ufs_utmrd));
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

static int ufs_vendor_setup(struct ufs_host *ufs)
{
	/* DMA little endian, order change */
	writel(0xa, (ufs->vs_addr + VS_DATA_REORDER));

	writel(1, (ufs->ioaddr + REG_UTP_TASK_REQ_LIST_RUN_STOP));

	writel(1, (ufs->ioaddr + REG_UTP_TRANSFER_REQ_LIST_RUN_STOP));

	writel(UFS_SG_BLOCK_SIZE_BIT, (ufs->vs_addr + VS_TXPRDT_ENTRY_SIZE));
	writel(UFS_SG_BLOCK_SIZE_BIT, (ufs->vs_addr + VS_RXPRDT_ENTRY_SIZE));

	writel(0xFFFFFFFF, (ufs->vs_addr + VS_UTRL_NEXUS_TYPE));
	writel(0xFFFFFFFF, (ufs->vs_addr + VS_UMTRL_NEXUS_TYPE));

	return NO_ERROR;
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

	res = ufs_send_upiu(UFS_SEND_READ_REFCLK_ATTR, 1);
	if (res) {
		printf("[UFS] read ref clk failed\n");
		return res;
	} else {
		printf("[UFS] ref clk setting is %x\n", ufs->attributes.arry[UPIU_ATTR_ID_REFCLKFREQ]);
	}

	if (ufs->attributes.arry[UPIU_ATTR_ID_REFCLKFREQ] != 0x1)
		ufs_send_upiu(UFS_SEND_WRITE_REFCLK_ATTR, 1);

	return 0;
}

static int ufs_link_startup(struct ufs_host *ufs)
{
	struct ufs_uic_cmd uic_cmd = { UIC_CMD_DME_LINK_STARTUP, 0, 0, 0};
	struct ufs_uic_cmd get_a_lane_cmd = { UIC_CMD_DME_GET, (0x1540 << 16), 0, 0 };
	struct uic_pwr_mode *pmd = &ufs->pmd_cxt;

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
	if (ufs_vendor_setup(ufs))
		goto out;


	/* 7. NOP and fDeviceinit */
	if (ufs_end_boot_mode(ufs))
		goto out;

	/* 8. Check a number of connected lanes */
	if (ufs_check_2lane(ufs))
		goto out;

	if(ufs_ref_clk_setup(ufs))
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

out:
	return NO_ERROR;
}

static int ufs_mem_init(struct ufs_host *ufs)
{
	if (!ufs->cmd_desc_addr) {
		ufs->cmd_desc_addr = memalign(0x1000, UFS_NUTRS * sizeof(struct ufs_cmd_desc));
		if (!ufs->cmd_desc_addr) {
			dprintf(INFO, "cmd_desc_addr memory alloc error!!!\n");
			goto out;
		}
		if ((u64) (ufs->cmd_desc_addr) & 0xfff) {
			dprintf(INFO, "allocated cmd_desc_addr memory align error!!!\n");
			goto free_ucd_out;
		}
	}

	ufs_debug("cmd_desc_addr : %p\n", ufs->cmd_desc_addr);
	ufs_debug("\tresponse_upiu : %p\n", &ufs->cmd_desc_addr->response_upiu);
	ufs_debug("\tprd_table : %p (size=%lx)\n", ufs->cmd_desc_addr->prd_table,
		  sizeof(ufs->cmd_desc_addr->prd_table));
	ufs_debug("\tsizeof upiu : %lx\n", sizeof(struct ufs_upiu));

	memset(ufs->cmd_desc_addr, 0x00, UFS_NUTRS * sizeof(struct ufs_cmd_desc));

	if (!ufs->utrd_addr) {
		ufs->utrd_addr = memalign(0x1000, UFS_NUTRS * sizeof(struct ufs_utrd));
		if (!ufs->utrd_addr) {
			dprintf(INFO, "utrd_addr memory alloc error!!!\n");
			goto free_ucd_out;
		}
		if ((u64) (ufs->utrd_addr) & 0xfff) {
			dprintf(INFO, "allocated utrd_addr memory align error!!!\n");
			goto free_utrd_out;
		}
	}
	ufs_debug("utrd_addr : %p\n", ufs->utrd_addr);
	memset(ufs->utrd_addr, 0x00, UFS_NUTRS * sizeof(struct ufs_utrd));

	if (!ufs->utmrd_addr) {
		ufs->utmrd_addr = memalign(0x1000, UFS_NUTMRS * sizeof(struct ufs_utmrd));
		if (!ufs->utmrd_addr) {
			dprintf(INFO, "utmrd_addr memory alloc error!!!\n");
			goto free_utrd_out;
		}
		if ((u64) (ufs->utmrd_addr) & 0xfff) {
			dprintf(INFO, "allocated utmrd_addr memory align error!!!\n");
			goto free_all_out;
		}
	}
	ufs_debug("utmrd_addr : %p\n", ufs->utmrd_addr);
	memset(ufs->utmrd_addr, 0x00, UFS_NUTMRS * sizeof(struct ufs_utmrd));

	ufs->utrd_addr->cmd_desc_addr_l = (u64)(ufs->cmd_desc_addr);
	ufs->utrd_addr->rsp_upiu_off = (u16)(offsetof(struct ufs_cmd_desc, response_upiu));
	ufs->utrd_addr->rsp_upiu_len = (u16)(ALIGNED_UPIU_SIZE);

	writel((u64)ufs->utmrd_addr, (ufs->ioaddr + REG_UTP_TASK_REQ_LIST_BASE_L));
	writel(0, (ufs->ioaddr + REG_UTP_TASK_REQ_LIST_BASE_H));

	writel((u64)ufs->utrd_addr, (ufs->ioaddr + REG_UTP_TRANSFER_REQ_LIST_BASE_L));
	writel(0, (ufs->ioaddr + REG_UTP_TRANSFER_REQ_LIST_BASE_H));

	ufs->utmrd_addr[0].dw[2] = (u32)(OCS_INVALID_COMMAND_STATUS);
	ufs->utmrd_addr[0].task_req_upiu.type = UPIU_TRANSACTION_TASK_REQ;
	ufs->utmrd_addr[0].task_req_upiu.flags = UPIU_CMD_FLAGS_NONE;
	ufs->utmrd_addr[0].task_req_upiu.lun = 0;
	ufs->utmrd_addr[0].task_req_upiu.tag = 0;
	ufs->utmrd_addr[0].task_req_upiu.function = UFS_QUERY_TASK;
	ufs->utmrd_addr[0].task_req_upiu.tsf[0] = 0;
	ufs->utmrd_addr[0].task_req_upiu.tsf[1] = 0;

	ufs->utmrd_addr[1].dw[2] = (u32)(OCS_INVALID_COMMAND_STATUS);
	ufs->utmrd_addr[1].task_req_upiu.type = UPIU_TRANSACTION_TASK_REQ;
	ufs->utmrd_addr[1].task_req_upiu.flags = UPIU_CMD_FLAGS_NONE;
	ufs->utmrd_addr[1].task_req_upiu.lun = 0;
	ufs->utmrd_addr[1].task_req_upiu.tag = 1;
	ufs->utmrd_addr[1].task_req_upiu.function = UFS_QUERY_TASK;
	ufs->utmrd_addr[1].task_req_upiu.tsf[0] = 0;
	ufs->utmrd_addr[1].task_req_upiu.tsf[1] = 1;

	return NO_ERROR;

 free_all_out:
	free(ufs->utmrd_addr);

 free_utrd_out:
	free(ufs->utrd_addr);

 free_ucd_out:
	free(ufs->cmd_desc_addr);

 out:
	return ERR_NO_MEMORY;

}

static void ufs_mem_clear(struct ufs_host *ufs)
{
	free(ufs->utmrd_addr);
	free(ufs->utrd_addr);
	free(ufs->cmd_desc_addr);
}

static int ufs_identify_bootlun(struct ufs_host *ufs)
{
	int boot_lun_en;
	int i;

	ufs_send_upiu(UFS_SEND_READ_BOOTLUNEN_ATTR, 1);
	boot_lun_en = ufs->attributes.arry[UPIU_ATTR_ID_BOOTLUNEN];
	if (boot_lun_en == 0) {
		printf("Boot LUN is disabled\n");
		return 1;
	}

	for (i = 0; i < 8; i++) {
		ufs_send_upiu_with_lun(UFS_SEND_READ_UNIT_DESC, i);
		if (boot_lun_en == ufs->unit_desc[i].bBootLunID) {
			// TODO:
			//set_bootLUN(i);
			dprintf(INFO, "Boot LUN is #%d, bBootLunID:%d\n", i, ufs->unit_desc[i].bBootLunID);
		}
	}

	return 0;
}

static void ufs_disable_ufsp(struct ufs_host *ufs)
{
	writel(0x0, ufs->fmp_addr + UFSP_UPSBEGIN0);
	writel(0xffffffff, ufs->fmp_addr + UFSP_UPSEND0);
	writel(0xff, ufs->fmp_addr + UFSP_UPLUN0);
	writel(0xf1, ufs->fmp_addr + UFSP_UPSCTRL0);
}

/*
	2. host & memory init
*/
static int ufs_host_init(int host_index, struct ufs_host *ufs)
{
	int rst_cnt = 0;
	int res;
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

	if (ufs_mem_init(ufs))
		goto out;

	/* Encryption by-passed */
	ufs_disable_ufsp(ufs);

	if (ufs_init_cal(ufs, host_index))
		goto out;

	do {
		res = ufs_link_startup(ufs);
		if (!res)
			break;
		rst_cnt++;
		dprintf(INFO, " Retry Link Startup CNT : %d\n", rst_cnt);
	} while (rst_cnt < 3);

	if (res)
		goto freeout;
	writel(0xFFFFFFFF, (ufs->vs_addr + VS_UTRL_NEXUS_TYPE));

	return NO_ERROR;
 freeout:
	ufs_mem_clear(ufs);

 out:
	return ERR_GENERIC;
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

	if (!ufs)
		return 1;

	ufs->lun = 0;

	ufs_send_upiu(UFS_SEND_READ_DEVICE_DESC, 1);
	ufs_send_upiu(UFS_SEND_READ_GEOMETRY_DESC, 1);

	ufs_send_upiu(UFS_SEND_READ_UNIT_DESC, 1);

	ufs_send_upiu(UFS_SEND_READ_CONFIG_DESC, 1);
	ufs_send_upiu(UFS_SEND_READ_BOOTLUNEN_ATTR, 1);

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

	if (ufs_identify_bootlun(ufs) == 1)
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

		ret = ufs_send_upiu(UFS_SEND_WRITE_CONFIG_DESC, 1);
		if (ret) {
			printf("[UFS] LU config: Descriptor write query fail with %d!!!\n", ret);
			continue;
		}

		ret = ufs_send_upiu(UFS_SEND_WRITE_BOOTLUNEN_ATTR, 1);
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

	int res = 0, i;

	// TODO:
#if 0
	if ((mode == 2) && (exynos_boot_mode() != BOOT_UFS)) {
		dprintf(INFO, " Not UFS boot mode. Init UFS manually.\n");
		return res;
	}
#endif

	ufs_lu_list.prev = 0;
	ufs_lu_list.next = 0;

	for (i = 0; i < SCSI_MAX_INITIATOR; i++) {
		_ufs_curr_host = i;
		if (!_ufs[i]) {
			_ufs[i] = malloc(sizeof(struct ufs_host));
			if (_ufs[i])
				memset(_ufs[i], 0x00, sizeof(struct ufs_host));
			else
				goto out;
		}
		res = ufs_host_init(i, _ufs[i]);
		if (res) {
			free(_ufs[i]);
			goto out;
		}

		ufs_dev[i] = (scsi_device_t *)
			malloc(sizeof(scsi_device_t) * SCSI_MAX_DEVICE);
		if (!ufs_dev[i]) {
			printf("ufs_dev memory allocation failed\n");
			return ERR_NO_MEMORY;
		}

		ufs_identify_bootlun(_ufs[i]);

		scsi_scan(ufs_dev[i], 0, SCSI_MAX_DEVICE, scsi_exec, NULL, 128, &ufs_lu_list);
		scsi_scan(&ufs_dev_rpmb, 0x44, 0, scsi_exec, "rpmb", 128, &ufs_lu_list);
		scsi_scan_ssu(&ufs_dev_ssu, 0x50, scsi_exec, (get_sdev_t *)scsi_get_ssu_sdev, &ufs_lu_list);
	}

out:
	/*
	 * Current host is zero by default after preparing to read and write
	 * because we assume that system boot requires host #0
	 */
	_ufs_curr_host = 0;

	return res;
}

