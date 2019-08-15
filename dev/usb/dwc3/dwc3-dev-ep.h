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

#define CMDCOMPLETEWAIT_UNIT		50000 // 1000000

typedef enum {
	DEPCFG_EVT_XFER_CMPL = (1 << 1),		// Transfer Complete
	DEPCFG_EVT_XFER_IN_PROG = (1 << 2),		// Trasfer on going
	DEPCFG_EVT_XFER_NRDY = (1 << 3),		// Transfer Start
	DEPCFG_EVT_FIFOXRUN = (1 << 4),			// FIFO Mul-Function :: Under-Run, Over-Run
	DEPCFG_EVT_STRM_EVT = (1 << 6),			// Stream EVENT(?)
} USB3_DEV_DEPCFG_EVT_e;

typedef enum {
	DEPEVT_CALLBACK_XFERCMPL = 0,
	DEPEVT_CALLBACK_XFERINPROG = 1,
	DEPEVT_CALLBACK_XFERNRDY = 2,
	DEPEVT_CALLBACK_FIFOXRUN = 3,
	DEPEVT_CALLBACK_STRM = 4,
	DEPEVT_CALLBACK_UNKNOWN = 0xff,
} USB3_DEV_DEPEVT_CALLBACK;

typedef enum {
	DEPEVT_EVT_XFER_CMPL = 1,
	DEPEVT_EVT_XFER_IN_PROG = 2,
	DEPEVT_EVT_XFER_NRDY = 3,
	DEPEVT_EVT_FIFOXRUN = 4,
	DEPEVT_EVT_STRM_EVT = 6,
	DEPEVT_EVT_EPCMD_CMPL = 7,
} USB3_DEV_EPEVT_TYPE_e;

typedef enum {
	// Xfer Not Ready
	DEPEVT_STS_NRDY_DATA = 1,
	DEPEVT_STS_NRDY_STATUS = 2,
	DEPEVT_STS_NRDY_NoValidTRB = 8
} USB3_DEV_EPEVT_STATUS_NRDY_e;

typedef enum {
	// Xfer Complete
	DEPEVT_STS_CMPL_LastIsocPacket_ShortPacket = 2,
	DEPEVT_STS_CMPL_IOC = 4,
	DEPEVT_STS_CMPL_LST = 8
} USB3_DEV_EPEVT_STATUS_CMPL_e;

typedef enum {
	// Xfer In Progress
	DEPEVT_STS_PROG_MissedIsoc = 4
} USB3_DEV_EPEVT_STATUS_INPROGRESS_e;

typedef enum {
	// Stream Event
	DEPEVT_STS_STREAM_NotFound = 2,
	DEPEVT_STS_STREAM_Found = 1,
} USB3_DEV_EPEVT_STATUS_STREAM_e;

typedef struct {
	unsigned non_ep_evnt : 1;	// bit[0] : 0-> ep-specific event
	unsigned ep_num : 5;	// bit[5:1] : ep number
	unsigned evt_type : 4;	// bit[9:6] : event type
	unsigned rsvd11_10 : 2;	// bit[11:10]
	unsigned evnt_sts : 4;	// bit[15:12] : event status
	unsigned evnt_param : 16;	// bit[31:16] : event parameters
} USB3_DEV_DEPEVT_BitField;

typedef union {
	u32 data;
	USB3_DEV_DEPEVT_BitField b;
} DWC3_DEV_DEPEVT_o;

typedef enum {
	DEPCMD_RSVD = 0x0,
	DEPCMD_SetEpCfg = 0x1,
	DEPCMD_SetEpXferRsrcCfg = 0x2,
	DEPCMD_GetEpState = 0x3,
	DEPCMD_SetStall = 0x4,
	DEPCMD_ClrStall = 0x5,
	DEPCMD_StartXfer = 0x6,
	DEPCMD_UpdateXfer = 0x7,
	DEPCMD_EndXfer = 0x8,
	DEPCMD_StartNewCfg = 0x9
} USB3_DEV_DEPCMD_TYPE_e;
typedef enum {
	DEPCFG_INIT = 0, DEPCFG_RESTORE = 1, DEPCFG_MODIFY = 2,
} USB3_DEV_EPCFG_TYPE_e;

typedef struct {
	unsigned cmd_type : 4;	// bit[3:0] : Command Type
	unsigned rsvd7_4 : 4;	// bit[7:4]
	unsigned ioc : 1;	// bit[8] : command interrupt on complete
	unsigned rsvd9 : 1;	// bit9]
	unsigned cmd_active : 1;	// bit[10] : command active
	unsigned hipri_forcerm : 1;
	unsigned cmd_sts : 4;	// bit[15:12] : command completion status
	unsigned param : 16;	// bit[31:16] : command parameters(written case), event parameters(read case)
} DWC3_DEV_DEPCMD_BitField;

typedef union {
	u32 data;	// reset value : 0x0
	DWC3_DEV_DEPCMD_BitField b;
} DWC3_DEV_DEPCMD_o;

// DEPCMDPAR0
// This structure represents the bit fields in the Device Endpoint Command
// Parameter 0 Register (DEPCMDPAR0n) for the Set Endpoint Configuration
// (DEPCMD_SetEpCfg) command.
typedef struct {
	unsigned rsvd0 : 1;	// bit[0]
	unsigned ep_type : 2;	// bit[2:1] : ep type
	unsigned mps : 11;	// bit[13:3] : maximum packet size
	unsigned rsvd16_14 : 3;	// bit[16:14]
	unsigned fifo_num : 5;	// bit[21:17] : fifo number
	unsigned brst_siz : 4;	// bit[25:22] : burst size
	unsigned rsvd29_26 : 4;	// bit[29:26]
	unsigned ConfigAction : 2;	// bit[31:30] : Config Action
} DWC3_DEV_DEPCMD_PARA0_BitField;
typedef union {
	u32 data;	// reset value : 0x0
	DWC3_DEV_DEPCMD_PARA0_BitField b;
} DWC3_DEV_DEPCMD_PARA0_o;

// DEPCMDPAR1
// This structure represents the bit fields in the Device Endpoint Command
// Parameter 1 Register (DEPCMDPAR1n) for the Set Endpoint Configuration
// (DEPCMD_SetEpCfg) command.
typedef struct {
	unsigned intr_num : 5;		// bit[4:0] : interrupt number
	unsigned rsvd7_5 : 3;		// bit[7:5]
	unsigned xfer_cmpl_en : 1;	// bit[8] : transfer complete true
	unsigned xfer_in_prog_en : 1;	// bit[9] : xfer in progress true
	unsigned xfer_nrdy_en : 1;	// bit[10] : xfer not ready true
	unsigned fifo_xrun_en : 1;	// bit[11] : fifo under/over-run true
	unsigned rsvd12 : 1;		// bit[12]
	unsigned str_evnt_en : 1;	// bit[13] : stream event true
	unsigned rsvd15_14 : 1;		// bit[14]
	unsigned ebc : 1;		// bit[15] : external buffer control
	unsigned binterval_m1 : 8;	// bit[23:16] : b interval -1
	unsigned strm_cap : 1;		// bit[24] : stream capable
	unsigned ep_dir : 1;		// bit[25] : ep direction(0:out, 1:in)
	unsigned ep_num : 4;		// bit[29:26] : ep number
	unsigned bulk_based : 1;	// bit[30] : bulk-base
	unsigned fifo_based : 1;	// bit[31] : fifo-based
} USB3_DEV_DEPCMD_PARA1_BitField;

typedef union {
	u32 data;	// reset value : 0x0
	USB3_DEV_DEPCMD_PARA1_BitField b;
} DWC3_DEV_DEPCMD_PARA1_o;

typedef enum {
	DEV30_EP_STATE_INIT = 0x00,
	DEV30_EP_STATE_CONFIGURED = 0x01,
	DEV30_EP_STATE_READY = 0x02,
	DEV30_EP_STATE_REQUESTED_HOST = 0x10,
	DEV30_EP_STATE_SW_RUN_REQ = 0x20,
	DEV30_EP_STATE_RUN_XFER = 0x30
} DWC3_DEV_EP_STATE_e;

typedef struct _dwc3_dev_ep_handle {
	// Basic : Nubmer, Direction, Type and Max Packet Size
	u8 m_ucEpNum;
	USB_EP m_eType;
	u32 m_uMaxPktSize;
	u8 m_ucMaxBurst;
	u8 m_ucInterval;
	USB_DIR m_eDir;

	// NEW TRB Handler
	USB3_DEV_TRB_HANDLER trb_h;
	USB3_DEV_TRB_p last_trb_ptr;

	// Transfer Resource Index
	u32 m_uXferRscIdx;

	// CallBack Function
	void (*m_fnEpCallBack[5])(void *);
	void *m_uEpCallBackVar0[5];

	u32 m_uTotalXferSize;
	u32 m_uRealXferSize;
	u32 m_uRealXferSize2;

	// Stall Status
	u8 m_bStalled;
	u8 m_bIsFirstTRB;

	// for ISOC
	DWC3_DEV_DEPEVT_o m_oLastEPEvtValue;
	u8 m_bCircularLink;
	u32 m_uBufferPushCnt;

	/* Root USB Device */
	DWC3_DEV_HANDLER dwc3_dev_h;

	DWC3_DEV_EP_STATE_e eState;

	u32 low_trb_addr;
	u32 high_trb_addr;
} DWC3_DEV_EP, *DWC3_DEV_EP_HANDLER;

#include "sys/types.h"
#include <stddef.h>

static DWC3_DEV_EP_HANDLER g_hUSB30EP[32];

void __dwc3_dev_ep_CalculXferSize(DWC3_DEV_EP_HANDLER hEP, u8 bInit,
	u32 uInitXferSize, u32 uTrbBufSize)
{
	if (bInit) {
		if (!hEP->m_uRealXferSize && hEP->m_eType != USBEP_ISOC)
			hEP->m_uRealXferSize = hEP->m_uTotalXferSize;
		else if (hEP->m_eType == USBEP_ISOC)
			hEP->m_uRealXferSize = hEP->m_uTotalXferSize;
		hEP->m_uRealXferSize2 = 0;
	} else {
		hEP->m_uRealXferSize -= uTrbBufSize;
		hEP->m_uRealXferSize2 += (uInitXferSize - uTrbBufSize);
	}
}

void __dwc3_dev_ep_CompleteOP(DWC3_DEV_EP_HANDLER hEP, u8 bEnClear)
{
	__dwc3_dev_ep_CalculXferSize(hEP, true, 0, 0);
	hEP->m_uRealXferSize = __usb3_dev_trb_CompleteOP(hEP->trb_h, bEnClear, hEP->m_eDir);
}

void __usb_dev_ep_Release(DWC3_DEV_EP_HANDLER hEP)
{
	__dwc3_dev_ep_CompleteOP(hEP, true);
	dwc3_dev_trb_release(hEP->trb_h, hEP->m_eDir);
	//hEP->trb_h->m_bCachedXfer = 0;
	//hEP->trb_h->m_uCurrentBlock = 1;
	hEP->m_uTotalXferSize = 0;
	hEP->m_bCircularLink = 0;
	hEP->m_uBufferPushCnt = 0;
}

u8 dwc3_dev_ep_cmd(DWC3_DEV_EP_HANDLER hEP, USB3_DEV_DEPCMD_TYPE_e eEPCMD,
		u32 uPara0, u32 uPara1)
{
	DWC3_DEV_DEPCMD_o oDEPCMD = {
		0,
	};
	s32 sSec;
	u32 uEPCMD;
	u8 ucEpNum = hEP->m_ucEpNum;
	DWC3_DEV_HANDLER dwc3_dev_h = hEP->dwc3_dev_h;
	u8 mask_suspend;

	sSec = CMDCOMPLETEWAIT_UNIT;

	/* Mask USB2.0 PHY Suspend Suspend */
	if (dwc3_dev_h->link_version < 0x0290a)
		mask_suspend = 1;
	else if (dwc3_dev_h->m_eEnumeratedSpeed != USBSPEED_SUPER)
		mask_suspend = 1;
	else
		mask_suspend = 0;

	if (mask_suspend)
		dwc3_glb_unmask_suspend(dwc3_dev_h->glb_dev_h, false);

	// Get Register Address
	if (eEPCMD != DEPCMD_StartNewCfg) {
		if (ucEpNum == 0 && hEP->m_eDir == USBDIR_IN)
			ucEpNum++;
		uEPCMD = rDEPCMD(ucEpNum);
		DWC3_REG_WR32(rDEPCMDPAR0(ucEpNum), uPara0);
		DWC3_REG_WR32(rDEPCMDPAR1(ucEpNum), uPara1);
		DWC3_REG_WR32(rDEPCMDPAR2(ucEpNum), 0);
	} else {
		uEPCMD = rDEPCMD(0);
		DWC3_REG_WR32(rDEPCMDPAR0(0), 0);
		DWC3_REG_WR32(rDEPCMDPAR1(0), 0);
		DWC3_REG_WR32(rDEPCMDPAR2(0), 0);
	}
	// Make EP Command
	oDEPCMD.b.cmd_type = eEPCMD;
	oDEPCMD.b.cmd_active = 1;
	if (eEPCMD == DEPCMD_StartXfer) {
		sSec *= 10;
		oDEPCMD.b.hipri_forcerm = 1;
		hEP->m_uRealXferSize = 0;
		hEP->m_bIsFirstTRB = 1;
		if (hEP->m_eType == USBEP_ISOC) {
			u8 ucInterVal = 1 << (hEP->m_ucInterval - 1);

			// Set Next Future Frame
			oDEPCMD.b.param = hEP->m_oLastEPEvtValue.b.evnt_param
							  / ucInterVal;
			oDEPCMD.b.param += 1;
			oDEPCMD.b.param *= ucInterVal;
			oDEPCMD.b.param &= 0x7fff;
		}
		U3DBG_EPCMD("EP%d : Start transfer\n", hEP->m_ucEpNum);
	} else if (eEPCMD == DEPCMD_EndXfer) {
		oDEPCMD.b.hipri_forcerm = 1;
		oDEPCMD.b.ioc = 1;
		sSec *= 10;
		oDEPCMD.b.param = hEP->m_uXferRscIdx;
		U3DBG_EPCMD("EP%d : End transfer\n", hEP->m_ucEpNum);
		U3DBG_EPCMD("Resource Index : %d\n", hEP->m_uXferRscIdx);
	} else if (eEPCMD == DEPCMD_StartNewCfg) {
		if (uPara0)
			oDEPCMD.b.param = 2;
		else
			oDEPCMD.b.param = 0;
		U3DBG_EPCMD("Start New Config : %d\n", oDEPCMD.b.param);
	} else if (eEPCMD == DEPCMD_UpdateXfer) {
		oDEPCMD.b.param = hEP->m_uXferRscIdx;
		U3DBG_EPCMD("EP%d : Update transfer\n", hEP->m_ucEpNum);
		U3DBG_EPCMD("Resource Index : %d\n", hEP->m_uXferRscIdx);
	} else if (eEPCMD == DEPCMD_SetStall) {
		U3DBG_EPCMD("EP%d : Set STALL\n", hEP->m_ucEpNum);
	} else if (eEPCMD == DEPCMD_ClrStall) {
		U3DBG_EPCMD("EP%d : Clear STALL\n", hEP->m_ucEpNum);
	}

	U3DBG_EPCMD("EP Command 0x%x, Para0 : 0x%x, Para1 : 0x%x\n",
		    oDEPCMD.data, uPara0, uPara1);
	if (eEPCMD == DEPCMD_StartXfer) {
		DWC3_DEV_LNKSTS eU2Sts;

		eU2Sts = dwc3_dev_get_lnksts(hEP->dwc3_dev_h);
		/*
		 * For L1 Sleep Feature
		 */
		if (dwc3_dev_h->m_eEnumeratedSpeed < USBSPEED_SUPER &&
				((eU2Sts == LNKSTS_SLEEP) ||
				 (eU2Sts == LNKSTS_SUSPEND)) &&
				 !dwc3_dev_h->hw_l1_wakeup) {
			dwc3_dev_req_lnksts(hEP->dwc3_dev_h,
						   LNKSTS_REMOTEWKUP);
		}
	}
	wmb();
	// Set EP Command to EP Command Register
	DWC3_REG_WR32(uEPCMD, oDEPCMD.data);
	// Wait Command Completion
	do {
		oDEPCMD.data = DWC3_REG_RD32(uEPCMD);
		if (!(oDEPCMD.b.cmd_active))
			break;
#if !defined(CONFIG_BOARD_ZEBU)
		udelay(1);
#endif
	} while (sSec-- != 0);
	// Release Signal Mask USB2.0 PHY Suspend
	if (mask_suspend)
		dwc3_glb_unmask_suspend(dwc3_dev_h->glb_dev_h, true);

	// Check Command Activation
	if (sSec) {
		if (eEPCMD == DEPCMD_StartXfer) {
			oDEPCMD.data = DWC3_REG_RD32(uEPCMD);
			hEP->m_uXferRscIdx = oDEPCMD.b.param;
			if ((hEP->m_eType == USBEP_ISOC) &&
					(oDEPCMD.b.cmd_sts & 0x2)) {
				dwc3_dev_ep_cmd(hEP, DEPCMD_EndXfer, 0, 0);
				return false;
			}
		}
		if (eEPCMD == DEPCMD_EndXfer) {
			dwc3_dev_trb_release(hEP->trb_h, hEP->m_eDir);
			hEP->m_uXferRscIdx = 0;
		}
		U3DBG_EPCMD("Complete: DEPCMD(%d)=0x%08x\n", ucEpNum, oDEPCMD.data);
		return true;
	} else {
		U3DBG_EPCMD("TimeOver: DEPCMD(%d)=0x%08x\n", ucEpNum, oDEPCMD.data);
		return false;
	}
}

u8 _dwc3_dev_ep_Activate(DWC3_DEV_EP_HANDLER hEP,
	USB3_DEV_EPCFG_TYPE_e eConfigAction,
	USB_DIR eEpDir,
	USB3_DEV_DEPCFG_EVT_e eEnEpEvt)
{
	DWC3_DEV_DEPCMD_PARA0_o oDEPCMD_PARA0 = {
		0,
	};
	DWC3_DEV_DEPCMD_PARA1_o oDEPCMD_PARA1 = {
		0,
	};
	DWC3_DEV_HANDLER dwc3_dev_h = hEP->dwc3_dev_h;
	// . Issue Set Ep Configuraton
	//------------------------------------
	// Parameter 0
	oDEPCMD_PARA0.data = 0;
	oDEPCMD_PARA0.b.ep_type = hEP->m_eType;
	oDEPCMD_PARA0.b.brst_siz = hEP->m_ucMaxBurst;
	oDEPCMD_PARA0.b.mps = hEP->m_uMaxPktSize;

	if (eEpDir == USBDIR_IN)
		oDEPCMD_PARA0.b.fifo_num = (int) (hEP->m_ucEpNum >> 1);

	oDEPCMD_PARA1.data = 0;
	if (eEnEpEvt & DEPCFG_EVT_XFER_CMPL)
		oDEPCMD_PARA1.b.xfer_cmpl_en = 1;
	if (eEnEpEvt & DEPCFG_EVT_XFER_IN_PROG)
		oDEPCMD_PARA1.b.xfer_in_prog_en = 1;
	if (eEnEpEvt & DEPCFG_EVT_XFER_NRDY)
		oDEPCMD_PARA1.b.xfer_nrdy_en = 1;
	if (eEnEpEvt & DEPCFG_EVT_FIFOXRUN)
		oDEPCMD_PARA1.b.fifo_xrun_en = 1;
	if (eEnEpEvt & DEPCFG_EVT_STRM_EVT)
		oDEPCMD_PARA1.b.str_evnt_en = 1;
	oDEPCMD_PARA1.b.ep_dir = (u32) eEpDir;
	oDEPCMD_PARA1.b.ep_num = (int) (hEP->m_ucEpNum >> 1);
	oDEPCMD_PARA1.b.intr_num = dwc3_dev_h->p_oDevConfig->used_intr_num;
	if (hEP->m_ucInterval)
		oDEPCMD_PARA1.b.binterval_m1 = hEP->m_ucInterval - 1;
	//if ( hEP->m_eType == USBEP_ISOC )
	//	oDEPCMD_PARA1.b.fifo_based = 1;
	if (!dwc3_dev_ep_cmd(hEP, DEPCMD_SetEpCfg, oDEPCMD_PARA0.data,
						  oDEPCMD_PARA1.data))
		return false;
	if (!dwc3_dev_ep_cmd(hEP, DEPCMD_SetEpXferRsrcCfg, 1, 0))
		return false;
	return true;
}

/* for debug */
static int inep_not_valid_cnt;
static DWC3_DEV_DEPEVT_o *last_evnt_ptr;

void dwc3_dev_ep_isr(DWC3_DEV_EP_HANDLER hEP, u8 ucEpNum, DWC3_DEV_DEPEVT_o *eEpEvent)
{
	USB3_DEV_DEPEVT_CALLBACK eEvtCallBackNum = DEPEVT_CALLBACK_UNKNOWN;
	int on_demand;

	if (!hEP)
		return;
	last_evnt_ptr = eEpEvent;
	hEP->m_oLastEPEvtValue = *eEpEvent;
	on_demand = hEP->dwc3_dev_h->p_oDevConfig->on_demand;
	switch (eEpEvent->b.evt_type) {
	case DEPEVT_EVT_XFER_CMPL:
		U3DBG_ISR_EP("EP : %d XFER Complete", ucEpNum);
		hEP->eState &= ~DEV30_EP_STATE_RUN_XFER;
		hEP->eState &= ~DEV30_EP_STATE_READY;
		eEvtCallBackNum = DEPEVT_CALLBACK_XFERCMPL;
		/* If Circular Ring TRB, update All TRB and start again */
		if (hEP->m_bCircularLink) {
			usb3_dev_trb_reset_hwo(hEP->trb_h);
			dwc3_dev_ep_cmd(hEP, DEPCMD_UpdateXfer, 0, 0);
		} else {
			/* Remove Rescource Index */
			if (hEP->m_uXferRscIdx)
				hEP->m_uXferRscIdx = 0;
			__usb_dev_ep_Release(hEP);
		}
		break;
	case DEPEVT_EVT_XFER_NRDY:
		if (eEpEvent->b.evnt_sts & 0x8) {
			U3DBG_ISR_EP("EP : %d XFER Not Ready but not valid TRB\n", ucEpNum);
			inep_not_valid_cnt++;
		} else
			U3DBG_ISR_EP("EP : %d XFER Not Ready\n", ucEpNum);

		eEvtCallBackNum = DEPEVT_CALLBACK_XFERNRDY;
		if (hEP->m_bCircularLink && 0) {
			usb3_dev_trb_reset_hwo(hEP->trb_h);
			dwc3_dev_ep_cmd(hEP, DEPCMD_UpdateXfer, 0, 0);
			eEvtCallBackNum = DEPEVT_CALLBACK_XFERCMPL;
		}

		if (((hEP->eState & DEV30_EP_STATE_RUN_XFER) != DEV30_EP_STATE_RUN_XFER) &&
				(hEP->eState & DEV30_EP_STATE_SW_RUN_REQ) &&
				(on_demand == 1)) {
			dwc3_dev_ep_cmd(hEP, DEPCMD_StartXfer,
					 hEP->high_trb_addr,
					 hEP->low_trb_addr);
		}
		hEP->eState |= DEV30_EP_STATE_REQUESTED_HOST;
		break;
	case DEPEVT_EVT_XFER_IN_PROG:
		hEP->eState &= ~DEV30_EP_STATE_RUN_XFER;
		// Reset HWO
		usb3_dev_trb_reset_hwo(hEP->trb_h);
		//_usb3_dev_TRB_Prepare(ucEpNum, false);
		if (!dwc3_dev_ep_cmd(hEP, DEPCMD_UpdateXfer, 0, 0))
			dwc3_dev_ep_cmd(hEP, DEPCMD_EndXfer, 0, 0);
		U3DBG_ISR_EP("EP : %d XFER In Progress", ucEpNum);
		eEvtCallBackNum = DEPEVT_CALLBACK_XFERINPROG;
		break;
	case DEPEVT_EVT_FIFOXRUN:
		U3DBG_ISR_EP("EP : %d FIFO Mul function(Under/Over Run)",
			     ucEpNum);
		eEvtCallBackNum = DEPEVT_CALLBACK_FIFOXRUN;
		break;
	case DEPEVT_EVT_STRM_EVT:
		U3DBG_ISR_EP("EP : %d Stream Event\n", ucEpNum);
		eEvtCallBackNum = DEPEVT_CALLBACK_STRM;
		break;
	case DEPEVT_EVT_EPCMD_CMPL:
		U3DBG_ISR_EP("EP : %d Command Complete", ucEpNum);
		break;
	default:
		U3DBG_ISR_EP("Unknown event!");
		break;
	}
	if ((eEvtCallBackNum != DEPEVT_CALLBACK_UNKNOWN)
			&& (hEP->m_fnEpCallBack[eEvtCallBackNum])) {
		hEP->m_fnEpCallBack[eEvtCallBackNum](hEP->m_uEpCallBackVar0[eEvtCallBackNum]);
	}
}
// APIs for IOCTL Request
void dwc3_dev_ep_SetCallBack_StartXfer(DWC3_DEV_EP_HANDLER hEP,
	void fnCallBack(void *), void *pArg)
{
	hEP->m_fnEpCallBack[DEPEVT_CALLBACK_XFERNRDY] = fnCallBack;
	hEP->m_uEpCallBackVar0[DEPEVT_CALLBACK_XFERNRDY] = pArg;
}

void dwc3_dev_ep_set_cb_xfer_done(DWC3_DEV_EP_HANDLER hEP,
	void fnCallBack(void *), void *pArg)
{
	hEP->m_fnEpCallBack[DEPEVT_CALLBACK_XFERCMPL] = fnCallBack;
	hEP->m_uEpCallBackVar0[DEPEVT_CALLBACK_XFERCMPL] = pArg;
}

u32 dwc3_dev_ep_GetMaxPacketSize(DWC3_DEV_EP_HANDLER hEP)
{
	return hEP->m_uMaxPktSize;
}

u32 dwc3_dev_ep_get_current_pkt(DWC3_DEV_EP_HANDLER hEP)
{
	return hEP->m_uRealXferSize;
}

void dwc3_dev_ep_set_buffer(DWC3_DEV_EP_HANDLER hEP, void *pAddr, u32 uSize, unsigned int uOption)
{
	void *p_TempAddr = pAddr;
	USB3_DEV_TRB_OPTION uTempTRBOption = TRB_NO_OPTION;
	USB3_DEV_TRB_TYPE_e eTRBType;

	// Check Non Cached Option
	if (uOption & GADGET_BUF_NON_CACHED)
		uTempTRBOption |= TRB_NONCACHED;

	if (hEP->m_eType == USBEP_ISOC) {
		if (hEP->m_bIsFirstTRB) {
			hEP->m_bIsFirstTRB = 0;
			eTRBType = TRB_CTRL_ISOC_1ST;
		} else
			eTRBType = TRB_CTRL_ISOC;
	} else
		eTRBType = TRB_CTRL_NORMAL;

	if ((hEP->m_eDir == USBDIR_OUT) && (uSize % hEP->m_uMaxPktSize)) {
		// When Direction is Out, Device is ready to receive packet
		uSize = ((uSize / hEP->m_uMaxPktSize) + 1) * hEP->m_uMaxPktSize;
	}

	if (uSize > (TRB_BUF_SIZ_LIMIT + 1 - hEP->m_uMaxPktSize)) {
		u32 uChunkSize = TRB_BUF_SIZ_LIMIT + 1 - hEP->m_uMaxPktSize;
		u32 uCnt = 1;
		for (; uCnt * uChunkSize < uSize; uCnt++) {
			dwc3_dev_trb_set(hEP->trb_h, p_TempAddr, uChunkSize,
							  eTRBType, uTempTRBOption);
			p_TempAddr += uChunkSize;

			if (eTRBType == TRB_CTRL_ISOC_1ST)
				eTRBType = TRB_CTRL_ISOC;
		}
		uSize = uSize - ((uCnt - 1) * uChunkSize);
	}

	// Check Last Option
	if (uOption & GADGET_BUF_LAST)
		uTempTRBOption |= TRB_LAST_INTR_EN;

	/* Check Circular TRB */
	if (uOption & GADGET_BUF_RING_BACK) {
		hEP->m_bCircularLink = true;
		uTempTRBOption &= ~TRB_LAST;
	}

	hEP->last_trb_ptr = dwc3_dev_trb_set(hEP->trb_h, p_TempAddr, uSize, eTRBType, uTempTRBOption);
}

u8 dwc3_dev_ep_start_xfer(DWC3_DEV_EP_HANDLER hEP, u32 uCacheOpEnable)
{
	u32 low_trb_addr, high_trb_addr;
	void *trb;
	bool ret;

	// Start Transfer
	hEP->eState |= DEV30_EP_STATE_SW_RUN_REQ;

#if 0
	if (hEP->m_bCircularLink == false)
		dwc3_dev_trb_check_last_trb(hEP->last_trb_ptr);
#endif
	trb = usb3_dev_trb_prepare(hEP->trb_h, uCacheOpEnable & 0x1,
			hEP->m_eDir,
			hEP->m_bCircularLink);

	if (sizeof(void *) > 4) {
		low_trb_addr =
			(unsigned long) ((u64) trb & 0xffffffff);
		high_trb_addr =
			(unsigned long) (((u64) trb >> 32) & 0xffffffff);
	} else {
		low_trb_addr = (unsigned long) trb;
		high_trb_addr = 0x0;
	}

	/* check host requested this ep transfer */
	if ((hEP->eState & DEV30_EP_STATE_REQUESTED_HOST) ||
			(hEP->dwc3_dev_h->p_oDevConfig->on_demand == 0)) {
		ret = dwc3_dev_ep_cmd(hEP, DEPCMD_StartXfer, high_trb_addr,
				       low_trb_addr);
		if (ret)
			ret = 0;
		else
			ret = -1;
	} else {
		hEP->low_trb_addr = low_trb_addr;
		hEP->high_trb_addr = high_trb_addr;
		ret = 0;
	}

	return ret;
}

u8 dwc3_dev_ep_is_xfer_done(DWC3_DEV_EP_HANDLER hEP)
{
	u8 bRet = true;
	if (hEP->eState & DEV30_EP_STATE_RUN_XFER)
		bRet = false;
	return bRet;
}

u8 dwc3_dev_ep_WaitTransferDone(DWC3_DEV_EP_HANDLER hEP, s32 sTimeOutValue)
{
	u8 bRet = false;
	if (sTimeOutValue == -1) {
		do {
			if (dwc3_dev_ep_is_xfer_done(hEP)) {
				bRet = true;
				break;
			}
			udelay(10);
		} while (1);
	} else {
		s32 sTimeOutCnt = 0;
		for (; sTimeOutCnt < sTimeOutValue; sTimeOutCnt++) {
			if (dwc3_dev_ep_is_xfer_done(hEP)) {
				bRet = true;
				break;
			}
			mdelay(1);
		}
	}
	return bRet;
}

bool dwc3_dev_ep_stop_xfer(DWC3_DEV_EP_HANDLER hEP)
{
	bool ret = false;

	if (hEP->m_uXferRscIdx)
		ret = dwc3_dev_ep_cmd(hEP, DEPCMD_EndXfer, 0, 0);
	else {
		U3DBG_ISR_EP("EP%d has not been started transfer\n",
			     hEP->m_ucEpNum);
		ret = true;
	}
	return ret;
}

void dwc3_dev_ep_init(DWC3_DEV_HANDLER dwc3_dev_h, u8 ucEpNum, USB_DIR eEpDir,
					  USB_EP eType, u8 ucMaxBurstSize, u8 bInterval)
{
	DWC3_DEV_EP_HANDLER *hUSB30_EP = (DWC3_DEV_EP_HANDLER *)dwc3_dev_h->p_EPArrayAddress;
	DWC3_DEV_EP_HANDLER hEP = NULL;
	USB3_DEV_EPCFG_TYPE_e eEpCfgType = DEPCFG_INIT;

	hEP = hUSB30_EP[ucEpNum];
	if (hEP->eState == DEV30_EP_STATE_INIT) {
		hEP->eState = DEV30_EP_STATE_CONFIGURED;
		eEpCfgType = DEPCFG_INIT;
	} else if (hEP->eState == DEV30_EP_STATE_CONFIGURED) {
		eEpCfgType = DEPCFG_MODIFY;
	}

	hEP->m_eDir = eEpDir;
	hEP->m_eType = eType;
	hEP->m_uMaxPktSize = dwc3_get_mx_pkt_sz(dwc3_dev_get_conn_speed(dwc3_dev_h), eType);
	hEP->m_ucMaxBurst = ucMaxBurstSize;
	hEP->m_ucInterval = bInterval;
	hEP->m_bIsFirstTRB = 1;
	if (ucEpNum == 0 && eType == USBEP_CTRL) {
		s8 scCnt;
		dwc3_dev_ep_cmd(hEP, DEPCMD_StartNewCfg, 0, 0);
		/* After 2.60a, Init EP sequence is start from EP0 Out. */
		for (scCnt = 0; scCnt < 2; scCnt++) {
			hEP->m_eDir = (USB_DIR) scCnt;
			_dwc3_dev_ep_Activate(hEP, eEpCfgType, (USB_DIR) scCnt,
					DEPCFG_EVT_XFER_CMPL | DEPCFG_EVT_XFER_NRDY);
			dwc3_dev_EnableEp(dwc3_dev_h, scCnt);
		}
		// for Control Transfer State Machine Init State
		hEP->m_eDir = USBDIR_OUT;

	} else {
		_dwc3_dev_ep_Activate(hEP, eEpCfgType, eEpDir,
				      DEPCFG_EVT_XFER_CMPL | DEPCFG_EVT_XFER_NRDY);
		dwc3_dev_EnableEp(dwc3_dev_h, ucEpNum);
	}

}

void dwc3_dev_ep_deinit(DWC3_DEV_EP_HANDLER hEP)
{
	u8 ucEpNum = hEP->m_ucEpNum;
	u32 uTempCnt;

	// Stop Transfer if already transfed
	if (hEP->eState & DEV30_EP_STATE_RUN_XFER)
		dwc3_dev_ep_cmd(hEP, DEPCMD_EndXfer, 0, 0);

	if (hEP->eState != DEV30_EP_STATE_INIT) {
		//_dwc3_dev_ep_Activate(hEP, DEPCFG_MODIFY, hEP->m_eDir, 0);
		dwc3_dev_DisableEp(hEP->dwc3_dev_h, ucEpNum);
	}

	// Clear Member Variable for next usage
	for (uTempCnt = 0; uTempCnt < 5; uTempCnt++) {
		hEP->m_fnEpCallBack[uTempCnt] = NULL;
		hEP->m_uEpCallBackVar0[uTempCnt] = NULL;
	}
	hEP->m_bIsFirstTRB = 1;
	hEP->m_uXferRscIdx = 0;
	hEP->m_bStalled = 0;
	hEP->m_bCircularLink = 0;
	hEP->eState = DEV30_EP_STATE_INIT;

}

static int gadget_ops_cfg_ep(void *dev_handle,
	USB_ENDPOINT_DESCRIPTOR *p_oEPDesc, USB_EPCOMP_DESCRIPTOR *p_oEpComp)
{
	DWC3_DEV_HANDLER dwc3_dev_h;
	DWC3_DEV_EP_HANDLER *hUSB30_EP;
	DWC3_DEV_EP_HANDLER hEP;
	u8 ucEpNum;
	USB3_DEV_EPCFG_TYPE_e eEpCfgType;

	if (!p_oEPDesc)
		return -1;

	dwc3_dev_h = dev_handle;
	hUSB30_EP = (DWC3_DEV_EP_HANDLER *)dwc3_dev_h->p_EPArrayAddress;
	ucEpNum = p_oEPDesc->bEndpointAddress & 0xf;
	ucEpNum *= 2;
	if (p_oEPDesc->bEndpointAddress & 0x80) {
		ucEpNum++;
		hEP = hUSB30_EP[ucEpNum];
		hEP->m_eDir = USBDIR_IN;
	} else {
		hEP = hUSB30_EP[ucEpNum];
		hEP->m_eDir = USBDIR_OUT;
	}
	hEP->m_ucEpNum = ucEpNum;
	eEpCfgType = DEPCFG_INIT;
	if (hEP->eState == DEV30_EP_STATE_INIT) {
		hEP->eState = DEV30_EP_STATE_CONFIGURED;
		eEpCfgType = DEPCFG_INIT;
	} else if (hEP->eState == DEV30_EP_STATE_CONFIGURED) {
		eEpCfgType = DEPCFG_MODIFY;
	}
	hEP->m_eType = p_oEPDesc->bmAttributes & 0x3;
	hEP->m_uMaxPktSize = p_oEPDesc->wMaxPacketSize;
	hEP->m_ucInterval = p_oEPDesc->bInterval;
	hEP->m_bIsFirstTRB = 1;

	if (p_oEpComp)
		hEP->m_ucMaxBurst = p_oEpComp->bMaxBurst;

	if (hEP->m_eType != USBEP_ISOC) {
		unsigned int intr_opt = DEPCFG_EVT_XFER_CMPL | DEPCFG_EVT_XFER_NRDY;

		if (dwc3_dev_h->p_oDevConfig->on_demand == 1)
			intr_opt |= DEPCFG_EVT_XFER_NRDY | DEPCFG_EVT_XFER_IN_PROG;
		_dwc3_dev_ep_Activate(hEP, eEpCfgType, hEP->m_eDir, intr_opt);
	} else {
		_dwc3_dev_ep_Activate(hEP, eEpCfgType, hEP->m_eDir,
							  DEPCFG_EVT_XFER_NRDY
							  | DEPCFG_EVT_XFER_IN_PROG);
	}
	dwc3_dev_EnableEp(dwc3_dev_h, ucEpNum);

	U3DBG_ISR_EP("EP:%d(0x%x) configured\n", ucEpNum, p_oEPDesc->bEndpointAddress);

	return ucEpNum;
}

static DWC3_DEV_EP_HANDLER get_ep_handle_from_gadget_param(void *dev_handle, unsigned char ep_id)
{
	DWC3_DEV_HANDLER dwc3_dev_h;
	DWC3_DEV_EP_HANDLER *ep_handle_array;
	unsigned char ep_array_num;

	dwc3_dev_h = dev_handle;
	ep_handle_array = (DWC3_DEV_EP_HANDLER *) dwc3_dev_h->p_EPArrayAddress;
	ep_array_num = (ep_id & 0xf) * 2;
	ep_array_num = (ep_id & 0x80) ? ep_array_num + 1 : ep_array_num;

	return ep_handle_array[ep_array_num];
}

static void gadget_ops_set_buffer(void *dev_handle, unsigned char ep_id, void *buf_addr,
		unsigned int xfer_sz, unsigned int option)
{
	DWC3_DEV_EP_HANDLER ep_handle;

	ep_handle = get_ep_handle_from_gadget_param(dev_handle, ep_id);
	dwc3_dev_ep_set_buffer(ep_handle, buf_addr, xfer_sz, option);
}

static void gadget_ops_set_xfer_done(void *dev_handle, unsigned char ep_id,
		void (*cb)(void *), void *cb_arg)
{
	DWC3_DEV_EP_HANDLER ep_handle;

	ep_handle = get_ep_handle_from_gadget_param(dev_handle, ep_id);
	dwc3_dev_ep_set_cb_xfer_done(ep_handle, cb, cb_arg);
}

static bool gadget_ops_ep_start(void *dev_handle, unsigned char ep_id)
{
	DWC3_DEV_EP_HANDLER ep_handle;

	ep_handle = get_ep_handle_from_gadget_param(dev_handle, ep_id);
	return dwc3_dev_ep_start_xfer(ep_handle, 0);
}

static void gadget_ops_ep_stop(void *dev_handle, unsigned char ep_id)
{
	DWC3_DEV_EP_HANDLER ep_handle;

	ep_handle = get_ep_handle_from_gadget_param(dev_handle, ep_id);
	dwc3_dev_ep_stop_xfer(ep_handle);
}

static bool gadget_ops_is_done(void *dev_handle, unsigned char ep_id)
{
	DWC3_DEV_EP_HANDLER ep_handle;

	ep_handle = get_ep_handle_from_gadget_param(dev_handle, ep_id);
	return dwc3_dev_ep_is_xfer_done(ep_handle);
}

unsigned int dwc3_dev_gadget_get_rx_sz(void *dev_handle, unsigned char ep_id)
{
	DWC3_DEV_EP_HANDLER ep_handle;

	ep_handle = get_ep_handle_from_gadget_param(dev_handle, ep_id);
	return dwc3_dev_ep_get_current_pkt(ep_handle);
}
