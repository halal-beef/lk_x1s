/*
 * (C) Copyright 2019 SAMSUNG Electronics
 * Author: sunghyun.na@samsung.com (Sung-Hyun Na)
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted
 * transcribed, stored in a retrieval system or translated into any human or computer language in an
 * form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 *
 */

#ifndef DEV_USB_DWC3_DWC3_DEV_H_
#define DEV_USB_DWC3_DWC3_DEV_H_

#define DWC3_DEV_TOTAL_EP_CNT		32

#include <usb-def.h>
#include "dev/usb/dwc3-config.h"

typedef enum {
	DCTL_TEST_MODE_DISABLED = 0,
	DCTL_TEST_J_MODE = 1,
	DCTL_TEST_K_MODE = 2,
	DCTL_TEST_SE0_NAK_MODE = 3,
	DCTL_TEST_PACKET_MODE = 4,
	DCTL_TEST_FORCE_ENABLE = 5,
	DCTL_TEST_CTRL_FIELD = 7
} USB3_DEV_DCTL_TEST_CTRL_e;

typedef enum {
	// In eSS Mode LTS State Machine
	LNKSTS_ESS_DIS = 0x4,
	LNKSTS_ESS_INACT = 0x6,

	// In SS Mode LTS State Machine
	LNKSTS_U0 = 0x0,
	LNKSTS_NOACTION = 0x0,
	LNKSTS_U1 = 0x1,
	LNKSTS_U2 = 0x2,
	LNKSTS_U3 = 0x3,
	LNKSTS_SS_DIS = 0x4,
	LNKSTS_RX_DET = 0x5,
	LNKSTS_SS_INACT = 0x6,
	LNKSTS_POLL = 0x7,
	LNKSTS_RECOV = 0x8,
	LNKSTS_HRESET = 0x9,
	LNKSTS_CMPLY = 0xa,
	LNKSTS_LPBK = 0xb,

	// In HS/FS Mode
	LNKSTS_ON = 0x0,
	LNKSTS_SLEEP = 0x2,
	LNKSTS_SUSPEND = 0x3,
	LNKSTS_ERLYSUS = 0x5,
	LNKSTS_REMOTEWKUP = 0x8,

	// Add 2.00a
	LNKSTS_RESET = 0xe,
	LNKSTS_RESUME = 0xf,
} DWC3_DEV_LNKSTS;

typedef enum {
	DGCMD_CMD_XMIT_SET_LINK_FUNC_LMP = 0x1,		// Supported Lower than 2.20a
	DGCMD_CMD_SET_PERIODIC_PARAMS = 0x2,
	DGCMD_CMD_SEND_DEV_NOTF_FUNC_WAKE = 0x3,	// Supported Lower than 2.20a
	DGCMD_CMD_SET_SCRATCHPAD_BUF_LO = 0x4,
	DGCMD_CMD_SET_SCRATCHPAD_BUF_HI = 0x5,
	DGCMD_CMD_SEND_DEV_NOTF_HOST_ROLE_REQ = 0x6,	// Lower than 2.20a
	DGCMD_CMD_SEND_DEV_NOTI = 0x7,			// Supported from 2.20a
	DGCMD_CMD_SELECTED_FIFO_FLUSH = 0x9,
	DGCMD_CMD_ALL_FIFO_FLUSH = 0xa,
	DGCMD_CMD_SET_EP_NRDY = 0xc,
	DGCMD_CMD_RUN_SOC_BUS_LOOPBACK_TEST = 0x10
} USB3_DEV_DGCMD_CMD_e;

typedef enum {
	DEVT_DISCONN = 0,
	DEVT_USBRESET = 1,
	DEVT_CONNDONE = 2,
	DEVT_ULST_CHNG = 3,
	DEVT_WKUP = 4,
	DEVT_HIBERNATION = 5,
	DEVT_SUSPEND_ENTRY = 6,
	DEVT_SOF = 7,
	/* Add on the 3.10a */
	DEVT_L1_SUSPEND = 8,
	DEVT_ERRATICERR = 9,
	DEVT_CMD_CMPL = 10,
	DEVT_OVERFLOW = 11,
	DEVT_VNDR_DEV_TST_RCVD = 12,
	DEVT_INACT_TIMEOUT_RCVD = 13,
	/* Add on the 3.10a */
	DEVT_L1_RESUME_WAKE_UP = 14,
} DWC3_DEV_DEVT_e;

typedef struct {
	unsigned dev_speed : 3;	// bit[2:0] : device speed
	unsigned dev_addr : 7;	// bit[9:3] : device address
	unsigned per_fr_int : 2;	// bit[11:10] : periodic frame interval
	unsigned intr_num : 5;	// bit[16:12] : interrupt number
	unsigned num_rx_buf : 5;	// bit[21:17] : # of rx buffers
	unsigned lpm_cap : 1;	// bit[22] : lpm capable
	unsigned ignore_stream_pp : 1;	// bit[23] : ignore stream pp ???
	unsigned rsvd31_24 : 8;	// bit[31:24] :
} USB3_REG_DCFG_BitField;

typedef union {
	unsigned int data;	// reset value : 0x00080804
	USB3_REG_DCFG_BitField b;
} USB3_REG_DCFG_o, *USB3_REG_DCFG_p;

// DCTL
typedef struct {
	unsigned rsvd0 : 1;	// bit[0] :
	unsigned test_ctrl : 4;	// bit[4:1] : Test Control
	unsigned ULStChngReq : 4;	// bit[8:5] : USB/LINK State Change Request
	unsigned AcceptU1Ena : 1;	// bit[9] : Accept U1 Enable
	unsigned InitU1Ena : 1;	// bit[10] : Initiate U1 Enable
	unsigned AcceptU2Ena : 1;	// bit[11] : Accept U2 Enable
	unsigned InitU2Ena : 1;	// bit[12] : Initiate U2 Enable
	unsigned rsvd16_13 : 4;	// bit[16:13] : RSVD
	unsigned TrgtULSt : 4;	// bit[20:17] : Target USB/Link State
	unsigned rsvd22_21 : 2;	// bit[22:21] : RSVD
	unsigned AppL1Res : 1;	// bit[23] : LPM Response Programmed by Application
	unsigned HIRD_Thres : 5;	// bit[28:24] : HIRD Threshold
	unsigned LSftRst : 1;	// bit[29] : Light Soft Reset
	unsigned core_soft_reset : 1;	// bit[30] : core soft reset
	unsigned run_stop : 1;	// bit[31] : run/stop
} USB3_REG_DCTL_BitField;

typedef union {
	unsigned int data;	// reset value : 0x0
	USB3_REG_DCTL_BitField b;
} USB3_REG_DCTL_o;

// DEVTEN
typedef struct {
	unsigned disconn_evt_en : 1;		// bit[0] : disconnect detected event enable
	unsigned usb_reset_en : 1;		// bit[1] : usb reset  enable
	unsigned conn_done_en : 1;		// bit[2] : connection done enable
	unsigned usb_lnk_sts_chng_en : 1;	// bit[3] : usb/link state change event enable
	unsigned wake_up_en : 1;		// bit[4] : resume/remote wakeup detected event enable
	unsigned HibernationReq : 1;		// bit[5] :
	unsigned u3l2l1_susp_en : 1;		// bit[6] : U3/L2 or U3/L2/L1 Suspend event
	unsigned sof_en : 1;			// bit[7] : start of (micro)frame enable
	unsigned l1_suspend_en : 1;		// bit[8] : L1 Suspend Event
	unsigned errtic_err_en : 1;		// bit[9] : erratic error event enable
	unsigned cmd_cmplt_en : 1;		// bit[10] : generic command compete event enable
	unsigned evnt_overflow_en : 1;		// bit[11] : event buffer overflow event enable
	unsigned vndr_dev_tst_rcved_en : 1;	// bit[12] : vendor device test LMP received event enable
	unsigned stop_on_disconn_evt_en : 1;	// bit[13] : Stop On Disconnect Enable
	unsigned l1_wkup_evt_en : 1;		// bit[14] : L1 Resume Detect Event Enable
	unsigned rsvd31_15 : 17;		// bit[31:15] :
} USB3_REG_DEVTEN_BitField;

typedef union {
	unsigned int data;	// reset value : 0x0
	USB3_REG_DEVTEN_BitField b;
} USB3_REG_DEVTEN_o, *USB3_REG_DEVTEN_p;

// DSTS
typedef struct {
	unsigned connect_speed : 3;	// bit[2:0] : connected speed(0:hs, 1:fs, 4:ss)
	unsigned soffn : 14; // bit[16:3] : (u)frame # of the received SOF
	unsigned rx_fifo_empty : 1;	// bit[17] : RxFIFO Empty
	unsigned usb_link_sts : 4;	// bit[21:18] : USB/Link State
	unsigned dev_ctrl_halted : 1;	// bit[22] : device controller halted
	unsigned core_idle : 1;	// bit[23] : core idle
	unsigned pwr_up_req : 1;	// bit[24] : power up request
	unsigned rsvd31_25 : 7;	// bit[31:25]
} USB3_REG_DSTS_BitField;

typedef union {
	unsigned int data;	// reset value : 0x00020004
	USB3_REG_DSTS_BitField b;
} USB3_REG_DSTS_o;

typedef struct {
	unsigned cmd_type : 8;	// bit[7:0] : command type
	unsigned ioc : 1;	// bit[8] : command interrupt on complete
	unsigned rsvd9 : 1;	// bit[9]
	unsigned cmd_active : 1;	// bit[10] : command active
	unsigned rsvd14_11 : 4;	// bit[14:11]
	unsigned cmd_sts : 1;	// bit[15] : command completion status (0:error, 1:success)
	unsigned rsvd31_16 : 16;	// bit[31:16] :
} USB3_REG_DGCMD_BitField;

// Device Generic Command Register(DGCMD)
typedef union {
	unsigned int data;	// reset value : 0x0
	USB3_REG_DGCMD_BitField b;
} USB3_REG_DGCMD_o;

typedef struct {
	unsigned IMODI : 16;	// bit[15:0] : Moderation Interval
	unsigned IMODC : 16;	// bit[31:16] : Interrupt Moderation Donw Counter
} USB3_REG_DIMOD_BitField;

// Device Interrupt Moderation Register(DIMOD)
typedef union {
	unsigned int data;	// reset value : 0x0
	USB3_REG_DIMOD_BitField b;
} USB3_REG_DIMOD_o;

typedef union {
	unsigned int data;
	struct {
		unsigned non_ep_evnt : 1;	// bit[0] : 1-> device-specific event
		unsigned dev_specific : 7;	// bit[7:1] : 0-> device specific, 1-> OTG, 3-> ULPI Carkit, 4-> I2C
		unsigned evt_type : 4;		// bit[11:8] : event type
		unsigned rsvd15_12 : 4;		// bit[15:12]
		unsigned evt_info : 16;		// bit[31:16] : event information bits
		//unsigned rsvd31_24 : 8;		// bit[31:24]
	} b;
} USB3_DEV_DEVT_o;

typedef struct _dwc3_dev_handler {
	/* Base Address */
	void *base;
	/* Configuration data */
	struct dwc3_dev_config *p_oDevConfig;

	USB_SPEED m_eEnumeratedSpeed;

	/* Event Buffer */
	u32 *m_pEventBuffer;
	u32 m_uCurrentEventPosition;

	/* descriptor buffer */
	USB_DEVICE_DESCRIPTOR dec_desc __attribute__((aligned(64)));
	void *bos_desc_buf;
	unsigned int bos_desc_alloc_sz;
	unsigned int bos_desc_total_sz;
	void *config_desc_buf;
	unsigned int config_desc_alloc_sz;
	unsigned int config_desc_total_sz;

	// EP Handler
	void *p_EPArrayAddress[32];
	u8 ucAllocatedEpCnt;
	u32 m_bEPHalted;

	// EP0 Handler
	void *p_EP0Address;

	// IPID
	u32 link_version;

	// Work Around for PHY BUG
	u8 m_bExitAltMode;

	// for Disconnection
	u8 m_bDisconnectNoti;

	// Cache Coherency enable
	bool non_cachable;


	/* Global Logic Capabilites */
	bool hw_l1_wakeup;
	bool l1_evt_split;

	/* Debugging parameter */
	int force_wake_from_u1;
	u64 abnormal_wake_cnt;
	u64 u1_enter_cnt;

	u64 u3_enter_cnt;
	u64 u3_abnormal_wake_cnt;

	struct dwc3_glb_dev *glb_dev_h;

	/* fastboot mode flag */
	bool fastboot_mode;

} *DWC3_DEV_HANDLER;


#endif /* DEV_USB_DWC3_DWC3_DEV_H_ */
