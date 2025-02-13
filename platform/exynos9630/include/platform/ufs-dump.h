#ifndef __UFS_DUMP_H_
#define __UFS_DUMP_H_

#define SCSI_UFS_DEBUG_FULL	0
/*
 * Exynos's Vendor specific registers for UFSHCI
 */
#define HCI_TXPRDT_ENTRY_SIZE		0x00
#define HCI_RXPRDT_ENTRY_SIZE		0x04
#define HCI_TO_CNT_DIV_VAL              0x08
#define HCI_1US_TO_CNT_VAL		0x0C
#define HCI_INVALID_UPIU_CTRL		0x10
#define HCI_INVALID_UPIU_BADDR		0x14
#define HCI_INVALID_UPIU_UBADDR		0x18
#define HCI_INVALID_UTMR_OFFSET_ADDR	0x1C
#define HCI_INVALID_UTR_OFFSET_ADDR	0x20
#define HCI_INVALID_DIN_OFFSET_ADDR	0x24
#define HCI_VENDOR_SPECIFIC_IS		0x38
#define HCI_VENDOR_SPECIFIC_IE		0x3C
#define HCI_UTRL_NEXUS_TYPE		0x40
#define HCI_UTMRL_NEXUS_TYPE		0x44
#define HCI_E2EFC_CTRL			0x48
#define HCI_SW_RST			0x50
#define HCI_LINK_VERSION		0x54
#define HCI_IDLE_TIMER_CONFIG		0x58
#define HCI_RX_UPIU_MATCH_ERROR_CODE	0x5C
#define HCI_DATA_REORDER		0x60
#define HCI_MAX_DOUT_DATA_SIZE		0x64
#define HCI_UNIPRO_APB_CLK_CTRL		0x68
#define HCI_AXIDMA_RWDATA_BURST_LEN	0x6C
#define HCI_GPIO_OUT			0x70
#define HCI_WRITE_DMA_CTRL		0x74
#define HCI_ERROR_EN_PA_LAYER		0x78
#define HCI_ERROR_EN_DL_LAYER		0x7C
#define HCI_ERROR_EN_N_LAYER		0x80
#define HCI_ERROR_EN_T_LAYER		0x84
#define HCI_ERROR_EN_DME_LAYER		0x88
#define HCI_UFSHCI_V2P1_CTRL			0X8C
#define HCI_REQ_HOLD_EN			0xAC
#define HCI_CLKSTOP_CTRL		0xB0
#define HCI_FORCE_HCS			0xB4
#define HCI_FSM_MONITOR			0xC0
#define HCI_PRDT_HIT_RATIO		0xC4
#define HCI_DMA0_MONITOR_STATE		0xC8
#define HCI_DMA0_MONITOR_CNT		0xCC
#define HCI_DMA1_MONITOR_STATE		0xD0
#define HCI_DMA1_MONITOR_CNT		0xD4
#define HCI_UFS_AXI_DMA_IF_CTRL		0xF8
#define HCI_UFS_ACG_DISABLE		0xFC
#define HCI_IOP_ACG_DISABLE		0x100
#define HCI_MPHY_REFCLK_SEL		0x108
#define HCI_SMU_ABORT_MATCH_INFO		0x10C
#define HCI_DBR_DUPLICATION_INFO		0x120
#define HCI_DBR_TIMER_CONFIG		0x140
#define HCI_DBR_TIMER_ENABLE		0x144
#define HCI_DBR_TIMER_STATUS		0x148
#define HCI_UTRL_DBR_3_0_TIMER_EXPIRED_VALUE		0x150
#define HCI_UTRL_DBR_7_4_TIMER_EXPIRED_VALUE		0x154
#define HCI_UTRL_DBR_11_8_TIMER_EXPIRED_VALUE		0x158
#define HCI_UTRL_DBR_15_12_TIMER_EXPIRED_VALUE		0x15C
#define HCI_UTMRL_DBR_3_0_TIMER_EXPIRED_VALUE		0x160
/*
 * UNIPRO registers
 */
#define UNIP_DME_LINKSTARTUP_CNF_RESULT		0x7854
#define UNIP_DME_HIBERN8_ENTER_CNF_RESULT	0x7864
#define UNIP_DME_HIBERN8_ENTER_IND_RESULT	0x7868
#define UNIP_DME_HIBERN8_EXIT_CNF_RESULT	0x7874
#define UNIP_DME_HIBERN8_EXIT_IND_RESULT	0x7878
#define UNIP_DME_PWR_IND_RESULT			0x78EC
#define UNIP_DME_INTR_STATUS_LSB			   0x7B00
#define UNIP_DME_INTR_STATUS_MSB	           0x7B04
#define UNIP_DME_INTR_ERROR_CODE			   0x7B20
#define UNIP_DME_DISCARD_PORT_ID	           0x7B24
#define UNIP_DME_DBG_OPTION_SUITE			   0x7C00
#define UNIP_DME_DBG_CTRL_FSM		           0x7D00
#define UNIP_DME_DBG_FLAG_STATUS			   0x7D14
#define UNIP_DME_DBG_LINKCFG_FSM	           0x7D18

#ifdef SCSI_UFS_DEBUG_FULL
struct exynos_ufs_sfr_log ufs_log_sfr[] = {
	{"STD HCI SFR"			,	LOG_STD_HCI_SFR,		0},

	{"INTERRUPT STATUS"		,	REG_INTERRUPT_STATUS,		0},
	{"INTERRUPT ENABLE"		,	REG_INTERRUPT_ENABLE,		0},
	{"CONTROLLER STATUS"		,	REG_CONTROLLER_STATUS,		0},
	{"CONTROLLER ENABLE"		,	REG_CONTROLLER_ENABLE,		0},
	{"UTP TRANSF REQ INT AGG CNTRL"	,	REG_UTP_TRANSFER_REQ_INT_AGG_CONTROL,		0},
	{"UTP TRANSF REQ LIST BASE L"	,	REG_UTP_TRANSFER_REQ_LIST_BASE_L,		0},
	{"UTP TRANSF REQ LIST BASE H"	,	REG_UTP_TRANSFER_REQ_LIST_BASE_H,		0},
	{"UTP TRANSF REQ DOOR BELL"	,	REG_UTP_TRANSFER_REQ_DOOR_BELL,		0},
	{"UTP TRANSF REQ LIST CLEAR"	,	REG_UTP_TRANSFER_REQ_LIST_CLEAR,		0},
	{"UTP TRANSF REQ LIST RUN STOP"	,	REG_UTP_TRANSFER_REQ_LIST_RUN_STOP,		0},
	{"UTP TRANSF REQ LIST CNR"	,	REG_UTP_TRANSFER_REQ_LIST_CNR,		0},
	{"UTP TASK REQ LIST BASE L"	,	REG_UTP_TASK_REQ_LIST_BASE_L,		0},
	{"UTP TASK REQ LIST BASE H"	,	REG_UTP_TASK_REQ_LIST_BASE_H,		0},
	{"UTP TASK REQ DOOR BELL"	,	REG_UTP_TASK_REQ_DOOR_BELL,		0},
	{"UTP TASK REQ LIST CLEAR"	,	REG_UTP_TASK_REQ_LIST_CLEAR,		0},
	{"UTP TASK REQ LIST RUN STOP"	,	REG_UTP_TASK_REQ_LIST_RUN_STOP,		0},
	{"UIC COMMAND"			,	REG_UIC_COMMAND,		0},
	{"UIC COMMAND ARG1"		,	REG_UIC_COMMAND_ARG_1,		0},
	{"UIC COMMAND ARG2"		,	REG_UIC_COMMAND_ARG_2,		0},
	{"UIC COMMAND ARG3"		,	REG_UIC_COMMAND_ARG_3,		0},
	{"CCAP"				,	REG_CRYPTO_CAPABILITY,		0},


	{"VS HCI SFR"			,	LOG_VS_HCI_SFR,			0},

	{"TXPRDT ENTRY SIZE"		,	HCI_TXPRDT_ENTRY_SIZE,		0},
	{"RXPRDT ENTRY SIZE"		,	HCI_RXPRDT_ENTRY_SIZE,		0},
	{"TO CNT DIV VAL"		,	HCI_TO_CNT_DIV_VAL,		0},
	{"1US TO CNT VAL"		,	HCI_1US_TO_CNT_VAL,		0},
	{"INVALID UPIU CTRL"		,	HCI_INVALID_UPIU_CTRL,		0},
	{"INVALID UPIU BADDR"		,	HCI_INVALID_UPIU_BADDR,		0},
	{"INVALID UPIU UBADDR"		,	HCI_INVALID_UPIU_UBADDR,		0},
	{"INVALID UTMR OFFSET ADDR"	,	HCI_INVALID_UTMR_OFFSET_ADDR,		0},
	{"INVALID UTR OFFSET ADDR"	,	HCI_INVALID_UTR_OFFSET_ADDR,		0},
	{"INVALID DIN OFFSET ADDR"	,	HCI_INVALID_DIN_OFFSET_ADDR,		0},
	{"VENDOR SPECIFIC IS"		,	HCI_VENDOR_SPECIFIC_IS,		0},
	{"VENDOR SPECIFIC IE"		,	HCI_VENDOR_SPECIFIC_IE,		0},
	{"UTRL NEXUS TYPE"		,	HCI_UTRL_NEXUS_TYPE,		0},
	{"UTMRL NEXUS TYPE"		,	HCI_UTMRL_NEXUS_TYPE,		0},
	{"SW RST"			,	HCI_SW_RST,		0},
	{"RX UPIU MATCH ERROR CODE"	,	HCI_RX_UPIU_MATCH_ERROR_CODE,		0},
	{"DATA REORDER"			,	HCI_DATA_REORDER,		0},
	{"AXIDMA RWDATA BURST LEN"	,	HCI_AXIDMA_RWDATA_BURST_LEN,		0},
	{"WRITE DMA CTRL"		,	HCI_WRITE_DMA_CTRL,		0},
	{"V2P1 CTRL"			,	HCI_UFSHCI_V2P1_CTRL,	 	0},
	{"CLKSTOP CTRL"			,	HCI_CLKSTOP_CTRL,		0},
	{"FORCE HCS"			,	HCI_FORCE_HCS,		0},
	{"FSM MONITOR"			,	HCI_FSM_MONITOR,		0},
	{"DMA0 MONITOR STATE"		,	HCI_DMA0_MONITOR_STATE,		0},
	{"DMA0 MONITOR CNT"		,	HCI_DMA0_MONITOR_CNT,		0},
	{"DMA1 MONITOR STATE"		,	HCI_DMA1_MONITOR_STATE,		0},
	{"DMA1 MONITOR CNT"		,	HCI_DMA1_MONITOR_CNT,		0},
	{"AXI DMA IF CTRL"		,	HCI_UFS_AXI_DMA_IF_CTRL,	0},
	{"UFS ACG DISABLE"	 	,	HCI_UFS_ACG_DISABLE,		0},
	{"MPHY REFCLK SEL"		,	HCI_MPHY_REFCLK_SEL,		0},
	{"SMU ABORT MATCH INFO"		,	HCI_SMU_ABORT_MATCH_INFO,	0},
	{"DBR DUPLICATION INFO"		,	HCI_DBR_DUPLICATION_INFO,	0},
	{"DBR TIMER CONFIG"		,	HCI_DBR_TIMER_CONFIG,		0},
	{"DBR TIMER ENABLE"		,	HCI_DBR_TIMER_ENABLE,		0},
	{"DBR TIMER STATUS"		,	HCI_DBR_TIMER_STATUS,		0},
	{"UTRL DBR 3 0 TIMER EXPIRED VALUE"		,	HCI_UTRL_DBR_3_0_TIMER_EXPIRED_VALUE,		0},
	{"UTRL DBR 7 4 TIMER EXPIRED VALUE"		,	HCI_UTRL_DBR_7_4_TIMER_EXPIRED_VALUE,	0},
	{"UTRL DBR 11 8 TIMER EXPIRED VALUE"		,	HCI_UTRL_DBR_11_8_TIMER_EXPIRED_VALUE,	0},
	{"UTRL DBR 15 12 TIMER EXPIRED VALUE"		,	HCI_UTRL_DBR_15_12_TIMER_EXPIRED_VALUE,		0},
	{"UTMRL DBR 3 0 TIMER EXPIRED VALUE"		,	HCI_UTMRL_DBR_3_0_TIMER_EXPIRED_VALUE,		0},

	{"UNIPRO SFR"			,	LOG_UNIPRO_SFR,			0},

	{"DME_LINKSTARTUP_CNF_RESULT"	,	UNIP_DME_LINKSTARTUP_CNF_RESULT	,	0},
	{"DME_HIBERN8_ENTER_CNF_RESULT"	,	UNIP_DME_HIBERN8_ENTER_CNF_RESULT,	0},
	{"DME_HIBERN8_ENTER_IND_RESULT"	,	UNIP_DME_HIBERN8_ENTER_IND_RESULT,	0},
	{"DME_HIBERN8_EXIT_CNF_RESULT"	,	UNIP_DME_HIBERN8_EXIT_CNF_RESULT,	0},
	{"DME_HIBERN8_EXIT_IND_RESULT"	,	UNIP_DME_HIBERN8_EXIT_IND_RESULT,	0},
	{"DME_PWR_IND_RESULT"		,	UNIP_DME_PWR_IND_RESULT	,	0},
	{"DME_INTR_STATUS_LSB"		,	UNIP_DME_INTR_STATUS_LSB,	0},
	{"DME_INTR_STATUS_MSB"		,	UNIP_DME_INTR_STATUS_MSB,	0},
	{"DME_INTR_ERROR_CODE"		,	UNIP_DME_INTR_ERROR_CODE,	0},
	{"DME_DISCARD_PORT_ID"		,	UNIP_DME_DISCARD_PORT_ID,	0},
	{"DME_DBG_OPTION_SUITE"		,	UNIP_DME_DBG_OPTION_SUITE,	0},
	{"DME_DBG_CTRL_FSM"		,	UNIP_DME_DBG_CTRL_FSM,	0},
	{"DME_DBG_FLAG_STATUS"		,	UNIP_DME_DBG_FLAG_STATUS,	0},
	{"DME_DBG_LINKCFG_FSM"		,	UNIP_DME_DBG_LINKCFG_FSM,	0},

	{"PMA SFR"			,	LOG_PMA_SFR,			0},

	{"COMN 0x2f"			,	(0x00BC),			0},
	{"TRSV_L0 0x4b"			,	(0x01EC),			0},
	{"TRSV_L0 0x4f"			,	(0x01FC),			0},
	{"TRSV_L1 0x4b"			,	(0x032C),			0},
	{"TRSV_L1 0x4f"			,	(0x033C),			0},

	{},
};

struct exynos_ufs_attr_log ufs_log_attr[] = {
	/* PA Standard */
	{UIC_ARG_MIB(0x1520),	0, 0},
	{UIC_ARG_MIB(0x1540),	0, 0},
	{UIC_ARG_MIB(0x1543),	0, 0},
	{UIC_ARG_MIB(0x155C),	0, 0},
	{UIC_ARG_MIB(0x155D),	0, 0},
	{UIC_ARG_MIB(0x155E),	0, 0},
	{UIC_ARG_MIB(0x155F),	0, 0},
	{UIC_ARG_MIB(0x1560),	0, 0},
	{UIC_ARG_MIB(0x1561),	0, 0},
	{UIC_ARG_MIB(0x1564),	0, 0},
	{UIC_ARG_MIB(0x1567),	0, 0},
	{UIC_ARG_MIB(0x1568),	0, 0},
	{UIC_ARG_MIB(0x1569),	0, 0},
	{UIC_ARG_MIB(0x156A),	0, 0},
	{UIC_ARG_MIB(0x1571),	0, 0},
	{UIC_ARG_MIB(0x1580),	0, 0},
	{UIC_ARG_MIB(0x1581),	0, 0},
	{UIC_ARG_MIB(0x1582),	0, 0},
	{UIC_ARG_MIB(0x1583),	0, 0},
	{UIC_ARG_MIB(0x1584),	0, 0},
	{UIC_ARG_MIB(0x1585),	0, 0},
	{UIC_ARG_MIB(0x1590),	0, 0},
	{UIC_ARG_MIB(0x1591),	0, 0},
	{UIC_ARG_MIB(0x15A1),	0, 0},
	{UIC_ARG_MIB(0x15A2),	0, 0},
	{UIC_ARG_MIB(0x15A3),	0, 0},
	{UIC_ARG_MIB(0x15A4),	0, 0},
	{UIC_ARG_MIB(0x15A7),	0, 0},
	{UIC_ARG_MIB(0x15A8),	0, 0},
	{UIC_ARG_MIB(0x15C0),	0, 0},
	{UIC_ARG_MIB(0x15C1),	0, 0},
	/* PA Debug */
	{UIC_ARG_MIB(0x9514),	0, 0},
	{UIC_ARG_MIB(0x9536),	0, 0},
	{UIC_ARG_MIB(0x9556),	0, 0},
	{UIC_ARG_MIB(0x9564),	0, 0},
	{UIC_ARG_MIB(0x9566),	0, 0},
	{UIC_ARG_MIB(0x9567),	0, 0},
	{UIC_ARG_MIB(0x9568),	0, 0},
	{UIC_ARG_MIB(0x956A),	0, 0},
	{UIC_ARG_MIB(0x9595),	0, 0},
	{UIC_ARG_MIB(0x9596),	0, 0},
	{UIC_ARG_MIB(0x9597),	0, 0},
	/* DL Standard */
	{UIC_ARG_MIB(0x2046),	0, 0},
	{UIC_ARG_MIB(0x2047),	0, 0},
	{UIC_ARG_MIB(0x2066),	0, 0},
	{UIC_ARG_MIB(0x2067),	0, 0},
	/* DL Debug */
	{UIC_ARG_MIB(0xA000),	0, 0},
	{UIC_ARG_MIB(0xA005),	0, 0},
	{UIC_ARG_MIB(0xA007),	0, 0},
	{UIC_ARG_MIB(0xA010),	0, 0},
	{UIC_ARG_MIB(0xA011),	0, 0},
	{UIC_ARG_MIB(0xA020),	0, 0},
	{UIC_ARG_MIB(0xA021),	0, 0},
	{UIC_ARG_MIB(0xA022),	0, 0},
	{UIC_ARG_MIB(0xA023),	0, 0},
	{UIC_ARG_MIB(0xA024),	0, 0},
	{UIC_ARG_MIB(0xA025),	0, 0},
	{UIC_ARG_MIB(0xA026),	0, 0},
	{UIC_ARG_MIB(0xA027),	0, 0},
	{UIC_ARG_MIB(0xA028),	0, 0},
	{UIC_ARG_MIB(0xA029),	0, 0},
	{UIC_ARG_MIB(0xA02A),	0, 0},
	{UIC_ARG_MIB(0xA02B),	0, 0},
	{UIC_ARG_MIB(0xA100),	0, 0},
	{UIC_ARG_MIB(0xA101),	0, 0},
	{UIC_ARG_MIB(0xA102),	0, 0},
	{UIC_ARG_MIB(0xA103),	0, 0},
	{UIC_ARG_MIB(0xA114),	0, 0},
	{UIC_ARG_MIB(0xA115),	0, 0},
	{UIC_ARG_MIB(0xA116),	0, 0},
	{UIC_ARG_MIB(0xA120),	0, 0},
	{UIC_ARG_MIB(0xA121),	0, 0},
	{UIC_ARG_MIB(0xA122),	0, 0},
	/* NL Standard */
	{UIC_ARG_MIB(0x3000),	0, 0},
	{UIC_ARG_MIB(0x3001),	0, 0},
	/* NL Debug */
	{UIC_ARG_MIB(0xB010),	0, 0},
	{UIC_ARG_MIB(0xB011),	0, 0},
	/* TL Standard */
	{UIC_ARG_MIB(0x4020),	0, 0},
	{UIC_ARG_MIB(0x4021),	0, 0},
	{UIC_ARG_MIB(0x4022),	0, 0},
	{UIC_ARG_MIB(0x4023),	0, 0},
	{UIC_ARG_MIB(0x4025),	0, 0},
	{UIC_ARG_MIB(0x402B),	0, 0},
	/* TL Debug */
	{UIC_ARG_MIB(0xC001),	0, 0},
	{UIC_ARG_MIB(0xC024),	0, 0},
	{UIC_ARG_MIB(0xC025),	0, 0},
	{UIC_ARG_MIB(0xC026),	0, 0},
	/* MPHY PCS Lane 0*/
	{UIC_ARG_MIB_SEL(0x0021, TX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x0022, TX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x0023, TX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x0024, TX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x0028, TX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x0029, TX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x002A, TX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x002B, TX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x002C, TX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x002D, TX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x0033, TX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x0035, TX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x0036, TX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x0041, TX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x00A1, RX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x00A2, RX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x00A3, RX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x00A4, RX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x00A7, RX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x00C1, RX_LANE_0+0),	0, 0},
	/* MPHY PCS Lane 1*/
	{UIC_ARG_MIB_SEL(0x0021, TX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x0022, TX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x0023, TX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x0024, TX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x0028, TX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x0029, TX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x002A, TX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x002B, TX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x002C, TX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x002D, TX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x0033, TX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x0035, TX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x0036, TX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x0041, TX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x00A1, RX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x00A2, RX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x00A3, RX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x00A4, RX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x00A7, RX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x00C1, RX_LANE_0+1),	0, 0},
	{},
};
#else
struct exynos_ufs_sfr_log ufs_log_sfr[] = {
	{"STD HCI SFR"			,	LOG_STD_HCI_SFR,		0},

	{"INTERRUPT STATUS"		,	REG_INTERRUPT_STATUS,		0},
	{"CONTROLLER STATUS"		,	REG_CONTROLLER_STATUS,		0},
	{"UTP TRANSF REQ DOOR BELL"	,	REG_UTP_TRANSFER_REQ_DOOR_BELL,		0},
	{"UTP TASK REQ DOOR BELL"	,	REG_UTP_TASK_REQ_DOOR_BELL,		0},

	{"VS HCI SFR"			,	LOG_VS_HCI_SFR,			0},

	{"VENDOR SPECIFIC IS"		,	HCI_VENDOR_SPECIFIC_IS,		0},
	{"RX UPIU MATCH ERROR CODE"	,	HCI_RX_UPIU_MATCH_ERROR_CODE,		0},
	{"CLKSTOP CTRL",			HCI_CLKSTOP_CTRL,		0},
	{"FORCE HCS",				HCI_FORCE_HCS,		0},
	{"DMA0 MONITOR STATE"		,	HCI_DMA0_MONITOR_STATE,		0},
	{"DMA1 MONITOR STATE"		,	HCI_DMA1_MONITOR_STATE,		0},
	{"SMU ABORT MATCH INFO"		,	HCI_SMU_ABORT_MATCH_INFO,	0},

	{"FMP SFR"			,	LOG_FMP_SFR,			0},

	{"UFSPRSECURITY"		,	UFSPRSECURITY,			0},


	{"UNIPRO SFR"			,	LOG_UNIPRO_SFR,			0},

	{"DME_HIBERN8_ENTER_IND_RESULT"	,	UNIP_DME_HIBERN8_ENTER_IND_RESULT	,	0},
	{"DME_HIBERN8_EXIT_IND_RESULT"	,	UNIP_DME_HIBERN8_EXIT_IND_RESULT	,	0},
	{"DME_PWR_IND_RESULT"		,	UNIP_DME_PWR_IND_RESULT			,	0},
	{"DME_DBG_CTRL_FSM"		,	UNIP_DME_DBG_CTRL_FSM			,	0},

	{"PMA SFR"			,	LOG_PMA_SFR,			0},

	{"COMN 0x2f"			,	(0x00BC),			0},
	{"TRSV_L0 0x4b"			,	(0x01EC),			0},
	{"TRSV_L0 0x4f"			,	(0x01FC),			0},
	{"TRSV_L1 0x4b"			,	(0x032C),			0},
	{"TRSV_L1 0x4f"			,	(0x033C),			0},
	{},
};

struct exynos_ufs_attr_log ufs_log_attr[] = {
	/* PA Standard */
	{UIC_ARG_MIB(0x1560),	0, 0},
	{UIC_ARG_MIB(0x1571),	0, 0},
	{UIC_ARG_MIB(0x1580),	0, 0},
	/* PA Debug */
	{UIC_ARG_MIB(0x9595),	0, 0},
	{UIC_ARG_MIB(0x9597),	0, 0},
	/* DL Debug */
	{UIC_ARG_MIB(0xA000),	0, 0},
	{UIC_ARG_MIB(0xA005),	0, 0},
	{UIC_ARG_MIB(0xA010),	0, 0},
	{UIC_ARG_MIB(0xA114),	0, 0},
	{UIC_ARG_MIB(0xA116),	0, 0},
	/* MPHY PCS Lane 0*/
	{UIC_ARG_MIB_SEL(0x0021, TX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x0022, TX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x0023, TX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x0024, TX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x0028, TX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x0029, TX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x002A, TX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x002B, TX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x002C, TX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x002D, TX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x0033, TX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x0035, TX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x0036, TX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x0041, TX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x00A1, RX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x00A2, RX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x00A3, RX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x00A4, RX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x00A7, RX_LANE_0+0),	0, 0},
	{UIC_ARG_MIB_SEL(0x00C1, RX_LANE_0+0),	0, 0},
	/* MPHY PCS Lane 1*/
	{UIC_ARG_MIB_SEL(0x0021, TX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x0022, TX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x0023, TX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x0024, TX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x0028, TX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x0029, TX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x002A, TX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x002B, TX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x002C, TX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x002D, TX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x0033, TX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x0035, TX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x0036, TX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x0041, TX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x00A1, RX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x00A2, RX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x00A3, RX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x00A4, RX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x00A7, RX_LANE_0+1),	0, 0},
	{UIC_ARG_MIB_SEL(0x00C1, RX_LANE_0+1),	0, 0},
	{},
};
#endif
#endif	/* __UFS_DUMP_H__ */
