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

#include <stdio.h>
#include <string.h>
#include <lk/debug.h>
#include <lk/trace.h>
#include <lk/reg.h>
#include <malloc.h>
#include <lk/init.h>
#include <platform/delay.h>
#include <platform/mmu/barrier.h>
#include <lib/font_display.h>
#include <kernel/timer.h>
#include <platform/mmu/mmu.h>

#include "dev/usb/gadget.h"
#include "dwc3-reg.h"
#include "dwc3-global.h"
#include "dwc3-dev.h"

#define dwc3_DEV_GET_HANDLER(_PDEV) (DWC3_DEV_HANDLER)(dev_get_drvdata(&_PDEV->dev))
#define LOCAL_TRACE 0

int usb_config_state;
struct timer config_timer;
int retry_cnt;

void muic_sw_usb(void);
static enum handler_return dwc3_dev_config_check(struct timer *timer, unsigned int now, void *arg);

enum dwc3_dev_dbg_bit {
	UDEV3DBG_E = (1 << 0),

	UDEV3DBG_ISR_E = (1 << 1),		// Evnet Buffer control
	UDEV3DBG_ISR_DEV_E = (1 << 2),		// Device Event
	UDEV3DBG_ISR_DEV_SUS_E = (1 << 3),	// Device Event for suspend mode only
	U3DEVDBG_ISR_STS_E = (1 << 4),		// Device Event - Status change

	U3DEVDBG_ISR_EP0_E = (1 << 5),		// EP0 Event
	U3DEVDBG_ISR_EP0_S_E = (1 << 6),	// EP0 Event Short Message
	U3DEVDBG_STDREQ_E = (1 << 7),		// Control Transfer
	U3DEVDBG_ISR_EP_E = (1 << 8),		// Other EP Event

	U3DEVDBG_EPCMD_E = (1 << 9),		// EP Command
	U3DEVDBG_TRB_E = (1 << 10),		// TRB
	U3DEVDBG_TIME_E = (1 << 11),		// Transfer Execrution Time
	U3DEVDBG_CACHE_e = (1 << 11),		// Cache operation

	/* All of function call */
	U3DEV_FUNCCALL_E = (1 << 31),
} ;

static volatile u32 g_uUSB3DbgEn = UDEV3DBG_E|U3DEVDBG_STDREQ_E|U3DEVDBG_ISR_EP_E;

#define U3DBG(x...)		LTRACEF_LEVEL((g_uUSB3DbgEn&UDEV3DBG_E)?INFO:(LK_DEBUGLEVEL+1),x)
#define U3DBG_ISR(x...)		LTRACEF_LEVEL((g_uUSB3DbgEn&UDEV3DBG_ISR_E)?INFO:(LK_DEBUGLEVEL+1),x)
#define U3DBG_ISR_DEV(x...)	LTRACEF_LEVEL((g_uUSB3DbgEn&UDEV3DBG_ISR_DEV_E)?INFO:(LK_DEBUGLEVEL+1),x)
#define U3DBG_ISR_DEV_SUS(x...)	LTRACEF_LEVEL((g_uUSB3DbgEn&UDEV3DBG_ISR_DEV_SUS_E)?INFO:(LK_DEBUGLEVEL+1),x)
#define U3DBG_ISR_EP(x...)	LTRACEF_LEVEL((g_uUSB3DbgEn&U3DEVDBG_ISR_EP_E)?INFO:(LK_DEBUGLEVEL+1),x)
#define U3DBG_ISR_EP0(x...)	LTRACEF_LEVEL((g_uUSB3DbgEn&U3DEVDBG_ISR_EP0_E)?INFO:(LK_DEBUGLEVEL+1),x)
#define U3DBG_ISR_EP0_S(x...)	LTRACEF_LEVEL((g_uUSB3DbgEn&U3DEVDBG_ISR_EP0_S_E)?INFO:(LK_DEBUGLEVEL+1),x)
#define U3DBG_ISR_STS(x...)	LTRACEF_LEVEL((g_uUSB3DbgEn&U3DEVDBG_ISR_STS_E)?INFO:(LK_DEBUGLEVEL+1),x)
#define U3DBG_STDREQ(x...)	LTRACEF_LEVEL((g_uUSB3DbgEn&U3DEVDBG_STDREQ_E)?INFO:(LK_DEBUGLEVEL+1),x)
#define U3DBG_EPCMD(x...)	LTRACEF_LEVEL((g_uUSB3DbgEn&U3DEVDBG_EPCMD_E)?INFO:(LK_DEBUGLEVEL+1),x)
#define U3DBG_TRB(x...)		LTRACEF_LEVEL((g_uUSB3DbgEn&U3DEVDBG_TRB_E)?INFO:(LK_DEBUGLEVEL+1),x)
#define U3DBG_TIME(x...)	LTRACEF_LEVEL((g_uUSB3DbgEn&U3DEVDBG_TIME_E)?INFO:(LK_DEBUGLEVEL+1),x)
/*
#define U3DBG_CACHE(x...)	LTRACEF_LEVEL((g_uUSB3DbgEn&U3DEVDBG_CACHE_e)?INFO:(LK_DEBUGLEVEL+1),x)
*/
#define U3DBG_CACHE(x...)

//static DWC3_DEV_HANDLER g_hUSB30;

#define DWC3_REG_RD32(_REG)		readl(dwc3_dev_h->base+_REG)
#define DWC3_REG_WR32(_REG, _DATA)	writel(_DATA, dwc3_dev_h->base+_REG)

extern void dwc3_dev_glue_exit_start_debug_trace(void *handle);
extern void dwc3_dev_glue_ss_phy_rest(void *handle);
extern int dwc3_dev_glue_get_logic_trace_val(void *, int, u64 *);
extern int dwc3_dev_glue_ask_wa(void *, char *);

static void *dwc3_calloc_align(u32 size, u32 align)
{
	u64 uAddr, uTemp1, uTemp2, uTemp3;
	u64 uAddr_aligned;
	u64 uAllocHwMem_AlignConstraint = 64;

	/* get the GCD(Great Common Divider) */
	uTemp2 = align;
	uTemp3 = uAllocHwMem_AlignConstraint;
	while(uTemp3)
	{
		uTemp1 = uTemp2%uTemp3;
		uTemp2 = uTemp3;
		uTemp3 = uTemp1;
	}

	/* get the LCM(Least Common Multiple) */
	align = align*uAllocHwMem_AlignConstraint/uTemp2;
	/* TODO : check align */
	uAddr = (u64) calloc(1, size + align);
	uTemp2 = uAddr % align;

	uAddr_aligned = uAddr + (align - uTemp2);
	*((void **)(uAddr_aligned - 8)) = (void *) uAddr;

	return (void *) uAddr_aligned;
}

void dwc3_free(void *ptr)
{
	void *org_alloc_addr;
	u64 uAddr_aligned;

	uAddr_aligned = (u64) ptr;
	org_alloc_addr = *((void **)(uAddr_aligned - 8));
	free(org_alloc_addr);
}

u32 dwc3_get_mx_pkt_sz(USB_SPEED eSpeed, USB_EP eEpXfer)
{
	u32 uRet = 0;
	switch (eEpXfer) {
	case USBEP_CTRL:
		switch (eSpeed) {
		case USBSPEED_FULL:
			uRet = 64;
			break;
		default:
		case USBSPEED_HIGH:
			uRet = 64;
			break;
		case USBSPEED_SUPER:
			uRet = 512;
			break;
		case USBSPEED_SUPERPLUS:
			uRet = 512;
			break;
		}
		break;
	case USBEP_BULK:
		switch (eSpeed) {
		case USBSPEED_FULL:
			uRet = 64;
			break;
		default:
		case USBSPEED_HIGH:
			uRet = 512;
			break;
		case USBSPEED_SUPER:
			uRet = 1024;
			break;
		case USBSPEED_SUPERPLUS:
			uRet = 1024;
			break;
		}
		break;
	case USBEP_INTR:
		switch (eSpeed) {
		case USBSPEED_SUPERPLUS:
		case USBSPEED_SUPER:
		case USBSPEED_FULL:
		case USBSPEED_HIGH:
		default:
			uRet = 64;
			break;
		}
		break;
	case USBEP_ISOC:
	default:
		U3DBG("Ohter Transfer Type is not Realized!!\n");
		break;
	}
	return uRet;
}

DWC3_DEV_LNKSTS dwc3_dev_get_lnksts(DWC3_DEV_HANDLER dwc3_dev_h);
USB_SPEED dwc3_dev_get_conn_speed(DWC3_DEV_HANDLER dwc3_dev_h);

/* DCFG APIs*/
USB_SPEED dwc3_dev_GetInitSpeed(DWC3_DEV_HANDLER dwc3_dev_h)
{
	USB3_REG_DCFG_o oDCFG = {
		DWC3_REG_RD32(rDCFG),
	};

	return (USB_SPEED) oDCFG.b.dev_speed;
}

static void _dwc3_dev_setusbaddr(DWC3_DEV_HANDLER dwc3_dev_h, u32 uUSBAddr)
{
	USB3_REG_DCFG_o oDCFG;

	oDCFG.data = DWC3_REG_RD32(rDCFG);
	oDCFG.b.dev_addr = uUSBAddr & 0x7f;
	DWC3_REG_WR32(rDCFG, oDCFG.data);

	/* Set USB Dev State to Addressed */
	if (uUSBAddr)
		gadget_chg_state(USB_DEV_STATE_ADDRESSED);
	else
		gadget_chg_state(USB_DEV_STATE_CONNECT);
}

/* DCTL APIs*/
void dwc3_dev_set_rs(DWC3_DEV_HANDLER dwc3_dev_h, u8 ucSet)
{
	USB3_REG_DCTL_o oDCTL;

	oDCTL.data = DWC3_REG_RD32(rDCTL);
	oDCTL.b.run_stop = ucSet;
	DWC3_REG_WR32(rDCTL, oDCTL.data);
}

u8 dwc3_dev_is_running(DWC3_DEV_HANDLER dwc3_dev_h)
{
	USB3_REG_DCTL_o oDCTL;

	oDCTL.data = DWC3_REG_RD32(rDCTL);

	if (oDCTL.b.run_stop)
		return true;
	else
		return false;
}

void dwc3_dev_softreset(DWC3_DEV_HANDLER dwc3_dev_h)
{
	USB3_REG_DCTL_o oDCTL;

	oDCTL.data = DWC3_REG_RD32(rDCTL);
	oDCTL.b.core_soft_reset = 1;
	//oDCTL.b.run_stop = 0;
	DWC3_REG_WR32(rDCTL, oDCTL.data);

	do {
#if !defined(CONFIG_BOARD_ZEBU)
		udelay(10);
#endif
		oDCTL.data = DWC3_REG_RD32(rDCTL);
	} while (oDCTL.b.core_soft_reset == 1);

#if !defined(CONFIG_BOARD_ZEBU)
	/* s/w must wait at least 3 phy clocks(1/60Mz * 3 = 48ns)
	 * before accessing the phy domain
	 */
	udelay(1);
#endif
	dwc3_glb_init_late(dwc3_dev_h->glb_dev_h, USB_DEV);
}

void dwc3_dev_lightrst(DWC3_DEV_HANDLER dwc3_dev_h)
{
	USB3_REG_DCTL_o oDCTL = {
		DWC3_REG_RD32(rDCTL),
	};

	oDCTL.b.LSftRst = 1;
	DWC3_REG_WR32(rDCTL, oDCTL.data);

	udelay(10);

	oDCTL.b.LSftRst = 0;
	DWC3_REG_WR32(rDCTL, oDCTL.data);
}

void dwc3_dev_SetExitLinkStatus(DWC3_DEV_HANDLER dwc3_dev_h,
	DWC3_DEV_LNKSTS eTargetSts)
{
	USB3_REG_DCTL_o oDCTL = {
		DWC3_REG_RD32(rDCTL),
	};

	switch (eTargetSts) {
	case LNKSTS_U2:
	case LNKSTS_U3:
	case LNKSTS_SS_DIS:
	case LNKSTS_RX_DET:
	case LNKSTS_SS_INACT:
		oDCTL.b.TrgtULSt = (u8)(eTargetSts);
		break;
	default:
		break;
	}
	DWC3_REG_WR32(rDCTL, oDCTL.data);
}

void dwc3_dev_req_lnksts(DWC3_DEV_HANDLER dwc3_dev_h,
	DWC3_DEV_LNKSTS eTargetSts)
{
	USB3_REG_DCTL_o oDCTL = {
		DWC3_REG_RD32(rDCTL),
	};

	/* Write 0 that field before requesting */
	oDCTL.b.ULStChngReq = 0;
	DWC3_REG_WR32(rDCTL, oDCTL.data);

	if (dwc3_dev_get_conn_speed(dwc3_dev_h) == USBSPEED_SUPERPLUS) {
		switch (eTargetSts) {
		case LNKSTS_ESS_DIS:
		case LNKSTS_RX_DET:
		case LNKSTS_ESS_INACT:
		case LNKSTS_RECOV:
		case LNKSTS_CMPLY:
		case LNKSTS_LPBK:
		case LNKSTS_U0:
			oDCTL.b.ULStChngReq = (u8)(eTargetSts);
			DWC3_REG_WR32(rDCTL, oDCTL.data);
			break;
		default:
			break;
		}

	} else if (dwc3_dev_get_conn_speed(dwc3_dev_h) == USBSPEED_SUPER) {
		switch (eTargetSts) {
		case LNKSTS_SS_DIS:
		case LNKSTS_RX_DET:
		case LNKSTS_SS_INACT:
		case LNKSTS_RECOV:
		case LNKSTS_CMPLY:
		case LNKSTS_LPBK:
		case LNKSTS_U0:
			oDCTL.b.ULStChngReq = (u8)(eTargetSts);
			DWC3_REG_WR32(rDCTL, oDCTL.data);
			break;
		default:
			break;
		}
	} else {
		DWC3_DEV_LNKSTS eU2Sts;

		eU2Sts = dwc3_dev_get_lnksts(dwc3_dev_h);
		if ((eTargetSts == LNKSTS_REMOTEWKUP)
				&& ((eU2Sts == LNKSTS_SUSPEND)
					|| (eU2Sts == LNKSTS_SLEEP)
					|| (eU2Sts == LNKSTS_ERLYSUS)
					|| (eU2Sts == LNKSTS_RESET)
					|| (eU2Sts == LNKSTS_RESUME))) {
			s32 sTimeOutCnt;
			bool ealry_suspend_mark;

			/* Driver Remote wake-up signal */
			oDCTL.b.ULStChngReq = (u8)(LNKSTS_REMOTEWKUP);
			DWC3_REG_WR32(rDCTL, oDCTL.data);
			sTimeOutCnt = 10000;
			ealry_suspend_mark = false;
			for (; sTimeOutCnt != 0;
					sTimeOutCnt--) {
				eU2Sts = dwc3_dev_get_lnksts(dwc3_dev_h);
				if (eU2Sts == LNKSTS_ON ||
						eU2Sts == LNKSTS_SLEEP)
					break;
				if (eU2Sts == LNKSTS_ERLYSUS)
					ealry_suspend_mark = true;
				if (ealry_suspend_mark &&
						(eU2Sts == LNKSTS_SUSPEND)) {
					sTimeOutCnt = 0;
					break;
				}
				udelay(1);

			}
			if (!sTimeOutCnt)
				U3DBG("Remote Wake-up Fail\n");
			oDCTL.b.ULStChngReq = 0x0;
			DWC3_REG_WR32(rDCTL, oDCTL.data);
		}
	}
}

u8 dwc3_dev_RemoteWakeUp(DWC3_DEV_HANDLER dwc3_dev_h)
{
	dwc3_dev_req_lnksts(dwc3_dev_h, LNKSTS_REMOTEWKUP);
	return true;
}

void dwc3_dev_SetTestMode(DWC3_DEV_HANDLER dwc3_dev_h, USB_TEST eTestMode)
{
	USB3_REG_DCTL_o oDCTL = {
		DWC3_REG_RD32(rDCTL),
	};
	U3DBG("\nTest Mode :");
	switch (eTestMode) {
	case USBTEST_J:
		U3DBG("J\n");
		oDCTL.b.test_ctrl = (u32) DCTL_TEST_J_MODE;
		break;
	case USBTEST_K:
		U3DBG("K\n");
		oDCTL.b.test_ctrl = (u32) DCTL_TEST_K_MODE;
		break;
	case USBTEST_SE0_NAK:
		U3DBG("SE0 NAK\n");
		oDCTL.b.test_ctrl = (u32) DCTL_TEST_SE0_NAK_MODE;
		break;
	case USBTEST_PACKET:
		U3DBG("Test Packet\n");
		oDCTL.b.test_ctrl = (u32) DCTL_TEST_PACKET_MODE;
		break;
	case USBTEST_FORCE_ENABLE:
		U3DBG("Force true\n");
		oDCTL.b.test_ctrl = (u32) DCTL_TEST_FORCE_ENABLE;
		break;
	default:
	case USBTEST_DISABLED:
		U3DBG("Off\n");
		oDCTL.b.test_ctrl = (u32) DCTL_TEST_MODE_DISABLED;
		break;
	}
	DWC3_REG_WR32(rDCTL, oDCTL.data);
}

void dwc3_dev_AcceptUx(DWC3_DEV_HANDLER dwc3_dev_h, u8 bU1Accept, u8 bU2Accept)
{
	USB3_REG_DCTL_o DCTL;

	DCTL.data = DWC3_REG_RD32(rDCTL);
	DCTL.b.AcceptU1Ena = bU1Accept & 0x1;
	DCTL.b.AcceptU2Ena = bU2Accept & 0x1;
	DWC3_REG_WR32(rDCTL, DCTL.data);
}

/* DSTS APIs*/
USB_SPEED dwc3_dev_get_conn_speed(DWC3_DEV_HANDLER dwc3_dev_h)
{
	USB3_REG_DSTS_o oDSTS = {
		DWC3_REG_RD32(rDSTS),
	};
	return (USB_SPEED)(oDSTS.b.connect_speed);
}

DWC3_DEV_LNKSTS dwc3_dev_get_lnksts(DWC3_DEV_HANDLER dwc3_dev_h)
{
	USB3_REG_DSTS_o oDSTS = {
		DWC3_REG_RD32(rDSTS),
	};
	return (DWC3_DEV_LNKSTS)(oDSTS.b.usb_link_sts);
}

u8 dwc3_dev_WaitDevControlHalted(DWC3_DEV_HANDLER dwc3_dev_h)
{
	u8 bRet = false;
	s32 sTimeOut = 1000; //10000
	USB3_REG_DSTS_o oDSTS;
	// Clear Event Counter if task is in Interrupt Routine
	// Wait Halted bit
	do {
		oDSTS.data = DWC3_REG_RD32(rDSTS);
		if (oDSTS.b.dev_ctrl_halted == 1) {
			bRet = true;
			break;
		}
		mdelay(1);
	} while (sTimeOut-- > 0);
	return bRet;
}

u8 US3_DEV_IsDevCtrlHalted(DWC3_DEV_HANDLER dwc3_dev_h)
{
	USB3_REG_DSTS_o oDSTS = {
		DWC3_REG_RD32(rDSTS),
	};
	return (u8)(oDSTS.b.dev_ctrl_halted);
}

u16 dwc3_dev_GetFrameNum(DWC3_DEV_HANDLER dwc3_dev_h)
{
	USB3_REG_DSTS_o oDSTS = {
		DWC3_REG_RD32(rDSTS),
	};
	u16 uRetSofNum = oDSTS.b.soffn;

	if (dwc3_dev_get_conn_speed(dwc3_dev_h) == USBSPEED_FULL)
		return (uRetSofNum & 0x7ff);
	else
		return uRetSofNum;
}

void dwc3_dev_EnableEp(DWC3_DEV_HANDLER dwc3_dev_h, u8 ucEpNum)
{
	DWC3_REG_WR32(rDALEPENA, DWC3_REG_RD32(rDALEPENA) | (1 << ucEpNum));
}

void dwc3_dev_DisableEp(DWC3_DEV_HANDLER dwc3_dev_h, u8 ucEpNum)
{
	u32 uRegValue = DWC3_REG_RD32(rDALEPENA);

	uRegValue &= ~(1 << ucEpNum);
	DWC3_REG_WR32(rDALEPENA, uRegValue);
}

/* DGCMD APIs */
void dwc3_dev_EnLinkFuctionLMP(DWC3_DEV_HANDLER dwc3_dev_h, u8 bEn)
{
	USB3_REG_DGCMD_o DGCMD;
	DGCMD.b.cmd_type = DGCMD_CMD_XMIT_SET_LINK_FUNC_LMP;
	DGCMD.b.cmd_active = 1;
	DWC3_REG_WR32(rDGCMDPAR, bEn & 1);
	DWC3_REG_WR32(rDGCMD, DGCMD.data);
}

void dwc3_dev_SetForceNak(DWC3_DEV_HANDLER dwc3_dev_h, u8 ucEpNum,
	USB_DIR eEpDir)
{
	USB3_REG_DGCMD_o DGCMD;
	u32 uEpEnaIdx = ucEpNum * 2;
	if (eEpDir == USBDIR_IN)
		uEpEnaIdx++;
	DGCMD.b.cmd_type = DGCMD_CMD_SET_EP_NRDY;
	DGCMD.b.cmd_active = 1;
	DWC3_REG_WR32(rDGCMDPAR, uEpEnaIdx);
	DWC3_REG_WR32(rDGCMD, DGCMD.data);
}

void dwc3_dev_SendDevNoti_FuncWake(DWC3_DEV_HANDLER dwc3_dev_h,
	u8 ucInterfaceNum)
{
	USB3_REG_DGCMD_o DGCMD = {
		0,
	};
	u32 uParameter = 0;
	// Set Parameter
	uParameter |= (u8) DEV_NOTI_FUNCTION_WAKE & 0xf;
	uParameter |= ucInterfaceNum << 4;
	// Make Command
	DGCMD.b.cmd_type = DGCMD_CMD_SEND_DEV_NOTI;
	DGCMD.b.cmd_active = 1;
	DWC3_REG_WR32(rDGCMDPAR, uParameter);
	DWC3_REG_WR32(rDGCMD, DGCMD.data);
}

void dwc3_dev_SendDevNoti_LTM(DWC3_DEV_HANDLER dwc3_dev_h, u32 uBELT)
{
	USB3_REG_DGCMD_o DGCMD = {
		0,
	};
	u32 uParameter = 0;
	// Set Parameter
	uParameter |= (u8) DEV_NOTI_LTM & 0xf;
	if (uBELT < 1024)
		uParameter |= uBELT << 4;
	else if (uBELT < 32768)
		uParameter |= ((0x1 << 10) | (u32)(uBELT / 1024)) << 4;
	else if (uBELT < 1048576)
		uParameter |= ((0x2 << 10) | (u32)(uBELT / 32768)) << 4;
	else
		uParameter |= ((0x3 << 10) | (u32)(uBELT / 1048576)) << 4;

	// Make Command
	DGCMD.b.cmd_type = DGCMD_CMD_SEND_DEV_NOTI;
	DGCMD.b.cmd_active = 1;
	DWC3_REG_WR32(rDGCMDPAR, uParameter);
	DWC3_REG_WR32(rDGCMD, DGCMD.data);
}

void dwc3_dev_SendDevNoti_BusInterval(DWC3_DEV_HANDLER dwc3_dev_h,
	s16 ssBusInterval)
{
	USB3_REG_DGCMD_o DGCMD = {
		0,
	};
	u32 uParameter = 0;
	// Set Parameter
	uParameter |= (u8) DEV_NOTI_BUS_INTERVAL_ADJUST & 0xf;
	uParameter |= (u16) ssBusInterval << 4;
	// Make Command
	DGCMD.b.cmd_type = DGCMD_CMD_SEND_DEV_NOTI;
	DGCMD.b.cmd_active = 1;
	DWC3_REG_WR32(rDGCMDPAR, uParameter);
	DWC3_REG_WR32(rDGCMD, DGCMD.data);
}

void dwc3_dev_SetSystemExitLatency(DWC3_DEV_HANDLER dwc3_dev_h, u32 uVar0);

/* usb30 dev private reference */
#include "dwc3-dev-trb.h"
#include "dwc3-dev-ep.h"
#include "dwc3-dev-ep0.h"

void dwc3_dev_SetSystemExitLatency(DWC3_DEV_HANDLER dwc3_dev_h, u32 uVar0)
{
	USB3_REG_DGCMD_o DGCMD = {
		0,
	};
	DWC3_DEV_EP0_HANDLER hdwc3_EP0;
	volatile u8 *pAddr;
	struct dwc3_dev_config *p_oDevConfig;
	u8 bRunCmd = false;

	hdwc3_EP0 = (DWC3_DEV_EP0_HANDLER) dwc3_dev_h->p_EP0Address;
	pAddr = (volatile u8 *) (hdwc3_EP0->m_uSELBuffer);
	p_oDevConfig = dwc3_dev_h->p_oDevConfig;

	if (p_oDevConfig->m_bU2Support == true) {
		u16 usU2PSEL = *((u16 *)(&pAddr[4]));
		U3DBG("System Latency U2PSEL : %d\n", usU2PSEL);
		if (usU2PSEL > 125)
			DWC3_REG_WR32(rDGCMDPAR, 0);
		else
			DWC3_REG_WR32(rDGCMDPAR, usU2PSEL);
		bRunCmd = 1;
	} else if (p_oDevConfig->m_bU1Support == true) {
		U3DBG("System Latency U1PSEL : %d\n", pAddr[1]);
		DWC3_REG_WR32(rDGCMDPAR, pAddr[1]);
		bRunCmd = 1;
	}
	if (bRunCmd) {
		s32 sCnt = 1000;
		DGCMD.b.cmd_type = DGCMD_CMD_SET_PERIODIC_PARAMS;
		DGCMD.b.cmd_active = 1;
		DWC3_REG_WR32(rDGCMD, DGCMD.data);

		for (; sCnt != 0; sCnt--) {
			DGCMD.data = DWC3_REG_RD32(rDGCMD);
			if (DGCMD.b.cmd_active == 0) {
				if (DGCMD.b.cmd_sts)
					U3DBG("Device Generic CMD Error\n");
				break;
			}
		}
		if (sCnt == 0)
			U3DBG("Device Generic CMD Error\n");
	}
}

u8 dwc3_dev_sw_reconn(DWC3_DEV_HANDLER dwc3_dev_h)
{
	USB3_REG_DSTS_o oDSTS;
	u32 ucEpNum = 0;
	DWC3_DEV_EP_HANDLER *hdwc3_EP;

	hdwc3_EP = (DWC3_DEV_EP_HANDLER *)dwc3_dev_h->p_EPArrayAddress;

	for (ucEpNum = 1; ucEpNum < DWC3_DEV_TOTAL_EP_CNT; ucEpNum++) {
		if (hdwc3_EP[ucEpNum]->eState & DEV30_EP_STATE_RUN_XFER) {
			dwc3_dev_ep_cmd(hdwc3_EP[ucEpNum], DEPCMD_EndXfer,
							 0, 0);
			hdwc3_EP[ucEpNum]->eState &= ~DEV30_EP_STATE_RUN_XFER;
		}
	}
	/* DP Pull-up false if vbus pad is not used */
	dwc3_glb_phy_dp_pullup_en(dwc3_dev_h->glb_dev_h, false);
	/* Stop Link : Disconnect terminaion */
	dwc3_dev_set_rs(dwc3_dev_h, 0);
	mdelay(30);
	/* Wait control halted */
	do {
		oDSTS.data = DWC3_REG_RD32(rDSTS);
	} while (!oDSTS.b.dev_ctrl_halted);
	/* DP Pull-up true if vbus pad is not used */
	dwc3_glb_phy_dp_pullup_en(dwc3_dev_h->glb_dev_h, true);
	/* If the software attempts a connect after the soft disconnect or
	 * detects a disconnect event, it must set DCTL[8:5] to 5 before
	 * reasserting the Run/Stop bit.
	 */
	dwc3_dev_SetExitLinkStatus(dwc3_dev_h, LNKSTS_RX_DET);
	/* Start Link : true terminaion */
	dwc3_dev_set_rs(dwc3_dev_h, 1);
	/* Clear ULSTCHGREQ register */
	dwc3_dev_SetExitLinkStatus(dwc3_dev_h, 0x0);

	return true;
}

void dwc3_dev_HandleDisconnectInt(DWC3_DEV_HANDLER dwc3_dev_h)
{
	/* Notify to device platfor layer */
	gadget_notify_disconnect();

	/* DP Pull-up false if vbus pad is not used */
	dwc3_glb_phy_dp_pullup_en(dwc3_dev_h->glb_dev_h, false);
}

void dwc3_dev_HandleConnectDoneInt(DWC3_DEV_HANDLER dwc3_dev_h)
{
	USB_SPEED speed = dwc3_dev_get_conn_speed(dwc3_dev_h);
	struct dwc3_dev_config *p_oDevConfig = dwc3_dev_h->p_oDevConfig;
	int ret;

	/* Reconfig EP0 if connected speed is not matched with initial speed
	 * on the DCFG */
	switch (speed) {
	case USBSPEED_SUPERPLUS:
		U3DBG("(Enumerated Speed:SuperPlus\n");
		break;
	case USBSPEED_SUPER:
		U3DBG("(Enumerated Speed:Super\n");
		break;
	case USBSPEED_HIGH:
	case USBSPEED_FULL:
		if (speed == USBSPEED_FULL) {
			dwc3_glb_phy_dp_pullup_en(dwc3_dev_h->glb_dev_h, true);
			U3DBG("(Enumerated Speed:Full)\n");

		} else {
			/* DP Pull-up false if vbus pad is not used */
			dwc3_glb_phy_dp_pullup_en(dwc3_dev_h->glb_dev_h, false);
			U3DBG("(Enumerated Speed:High)\n");
		}
		break;
	default:
		U3DBG("(Enumerated Speed:Not Supported!!)\n");
		break;
	}
	if (speed != dwc3_dev_h->m_eEnumeratedSpeed) {
		DWC3_DEV_EP_HANDLER *hdwc3_EP;
		DWC3_DEV_EP_HANDLER hEP;
		DWC3_DEV_DEPCMD_PARA0_o oDEPCMD_PARA0;
		DWC3_DEV_DEPCMD_PARA1_o oDEPCMD_PARA1;
		s8 scCnt;

		hdwc3_EP = (DWC3_DEV_EP_HANDLER *)dwc3_dev_h->p_EPArrayAddress;
		hEP = hdwc3_EP[0];

		hEP->m_uMaxPktSize = dwc3_get_mx_pkt_sz(speed,
							 USBEP_CTRL);
		for (scCnt = 0; scCnt < 2; scCnt++) {
			hEP->m_eDir = (USB_DIR) scCnt;
			/* Issue Set Ep Configuraton */
			/* Parameter 0 */
			oDEPCMD_PARA0.data = 0;
			/* Modify Max Packet Size */
			oDEPCMD_PARA0.b.ConfigAction = (u8) DEPCFG_MODIFY;
			oDEPCMD_PARA0.b.ep_type = (u8) USBEP_CTRL;
			oDEPCMD_PARA0.b.mps = hEP->m_uMaxPktSize;
			oDEPCMD_PARA0.b.fifo_num = 0;
			/* Control Transfer is not supported Burst */
			oDEPCMD_PARA0.b.brst_siz = 0;
			/* Parameter 1 */
			oDEPCMD_PARA1.data = 0;
			oDEPCMD_PARA1.b.xfer_cmpl_en = 1;
			oDEPCMD_PARA1.b.xfer_nrdy_en = 1;
			/* oDEPCMD_PARA1.b.xfer_in_prog_en = 1;
			* OUT EP(RX) : Over-Run, In EP(TX) : Under-Run
			* oDEPCMD_PARA1.b.fifo_xrun_en = 1;
			* oDEPCMD_PARA1.b.str_evnt_en = 1; */
			oDEPCMD_PARA1.b.ep_dir = (u32) scCnt;
			oDEPCMD_PARA1.b.ep_num = 0;
			if (!dwc3_dev_ep_cmd(hEP, DEPCMD_SetEpCfg,
					      oDEPCMD_PARA0.data,
					      oDEPCMD_PARA1.data)) {
				U3DBG("Endpoint 0 Set Config Command Fail\n");
			}
		}
		/* for Control Transfer State Machine Init State */
		hEP->m_eDir = USBDIR_OUT;
		dwc3_dev_h->m_eEnumeratedSpeed = speed;
		/*Set Device Attribute Descitor (Device and BOS Desctiptor) */
		make_dev_desc(speed, &dwc3_dev_h->dec_desc);
		do {
			ret = make_bos_desc(speed, dwc3_dev_h->bos_desc_buf, dwc3_dev_h->bos_desc_alloc_sz);
			if (ret < 0) {
				dwc3_free(dwc3_dev_h->bos_desc_buf);
				dwc3_dev_h->bos_desc_alloc_sz *= 2;
				dwc3_calloc_align(dwc3_dev_h->bos_desc_alloc_sz, 64);
			} else
				dwc3_dev_h->bos_desc_total_sz = ret;
		} while (ret < 0);
		/* Make Descriptor for connected speed */
		do {
			ret = make_configdesc(speed, dwc3_dev_h->config_desc_buf, dwc3_dev_h->config_desc_alloc_sz);
			if (ret < 0) {
				dwc3_free(dwc3_dev_h->config_desc_buf);
				dwc3_dev_h->config_desc_alloc_sz *= 2;
				dwc3_calloc_align(dwc3_dev_h->config_desc_alloc_sz, 64);
			} else
				dwc3_dev_h->config_desc_total_sz = ret;
		} while (ret < 0);
	}
	/* true U1, U2 Feature */
	dwc3_dev_AcceptUx(dwc3_dev_h, p_oDevConfig->m_bU1Support, p_oDevConfig->m_bU2Support);
	/* Notify to usb dev layer to connected state */
	gadget_chg_state(USB_DEV_STATE_CONNECT);
}

void dwc3_dev_HandleUsbResetInt(DWC3_DEV_HANDLER dwc3_dev_h)
{
	struct dwc3_dev_config *p_oDevConfig = dwc3_dev_h->p_oDevConfig;
	DWC3_DEV_EP0_HANDLER hdwc3_EP0;

	hdwc3_EP0 = (DWC3_DEV_EP0_HANDLER)dwc3_dev_h->p_EP0Address;

	/* true DP Pull-up if vbus pad is not used */
	dwc3_glb_phy_dp_pullup_en(dwc3_dev_h->glb_dev_h, true);

	// Notify to device platfor layer
	//UDEV_NotifyDisconnect(dwc3_dev_h->udev_p);

	/* Clear last usb address (set to 0) */
	_dwc3_dev_setusbaddr(dwc3_dev_h, 0);

	// Init Device Selector
	if (p_oDevConfig->m_bRemoteWake)
		hdwc3_EP0->m_ucDeviceSelector = (1 << 1);
	else
		hdwc3_EP0->m_ucDeviceSelector = 0x0;

	// Change USB State to connected state
	//UDEV_ChgDevState(dwc3_dev_h->udev_p, UDEV_STATE_DEFAULT);
}

void dwc3_dev_handle_enter_suspend(DWC3_DEV_HANDLER dwc3_dev_h)
{
	U3DBG_ISR_DEV("Suspend(L2/U3)\n");
}

void dwc3_dev_handle_resume_wakeup(DWC3_DEV_HANDLER dwc3_dev_h)
{
	U3DBG_ISR_DEV("Resume/Wake-up(L0/U0)\n");
}

extern void dwc3_dev_glue_set_power_state(void *, int, u32);
extern void dwc3_dev_glue_req_pwr_sts(void *, int, u32);
extern void dwc3_dev_glue_get_u1_ipc_ack(void *, int, u32 *);

static void dwc3_dev_ss_gen1_phy_wa_on_u0(DWC3_DEV_HANDLER dwc3_dev_h)
{
	u32 u1_handshake = 0;

	if (dwc3_dev_h->force_wake_from_u1) {
		u32 reg;

		dwc3_dev_h->force_wake_from_u1 = 0;
		reg = DWC3_REG_RD32(rDCTL);
		((USB3_REG_DCTL_o *) (&reg))->b.InitU1Ena = true;
		((USB3_REG_DCTL_o *) (&reg))->b.AcceptU1Ena = true;
		((USB3_REG_DCTL_o *) (&reg))->b.InitU2Ena = true;
		((USB3_REG_DCTL_o *) (&reg))->b.AcceptU2Ena = true;
		DWC3_REG_WR32(rDCTL, reg);
	}

	dwc3_dev_glue_get_u1_ipc_ack(dwc3_dev_h, LNKSTS_U1, &u1_handshake);
	if (u1_handshake == 1)
		dwc3_dev_glue_set_power_state(dwc3_dev_h, LNKSTS_U1, 0);
}

void dwc3_dev_HandleLinkStatusChange(DWC3_DEV_HANDLER dwc3_dev_h, USB3_DEV_DEVT_o*uDevEvent)
{
	if (uDevEvent->b.evt_info & 0x10) {
		U3DBG_ISR_DEV("SS Event occured");
		switch ((DWC3_DEV_LNKSTS)(uDevEvent->b.evt_info & 0xf)) {
		case LNKSTS_U0:
			U3DBG_ISR_STS("U0");
			print_lcd_update(FONT_GREEN, FONT_BLACK,
						"USB cable connected...");
			break;
		case LNKSTS_U1:
			U3DBG_ISR_STS("U0");
			break;
		case LNKSTS_U2:
			U3DBG_ISR_STS("U2");
			break;
		case LNKSTS_U3:
			U3DBG_ISR_STS("U3");
			print_lcd_update(FONT_YELLOW, FONT_RED,
						"USB cable disconnected...");
			break;
		case LNKSTS_SS_DIS:
			U3DBG_ISR_STS("SS Disabled");
			break;
		case LNKSTS_RX_DET:
			U3DBG_ISR_STS("RX Detection");
			break;
		case LNKSTS_SS_INACT:
			U3DBG_ISR_STS("SS Inactive");
			break;
		case LNKSTS_POLL:
			U3DBG_ISR_STS("POLL");
			break;
		case LNKSTS_RECOV:
			U3DBG_ISR_STS("Recover");
			break;
		case LNKSTS_HRESET:
			U3DBG_ISR_STS("Hot Reset");
			break;
		case LNKSTS_CMPLY:
			U3DBG_ISR_STS( "Compliance Mode");
			break;
		case LNKSTS_LPBK:
			U3DBG_ISR_STS("Loop Back");
			break;
		case LNKSTS_RESUME:
			U3DBG_ISR_STS("Resume");
			break;
		default:
			U3DBG_ISR_STS("Unkwon :: 0x%x",
				      (uDevEvent->b.evt_info & 0xf));
			break;
		}
	} else {
		U3DBG_ISR_DEV("FS/HS Event Occured\n");
		switch ((DWC3_DEV_LNKSTS)(uDevEvent->b.evt_info & 0xf)) {
		case LNKSTS_ON:
			dwc3_glb_phy_dp_pullup_en(dwc3_dev_h->glb_dev_h, true);
			U3DBG_ISR_STS( "On State\n");
			break;
		case LNKSTS_SLEEP:
			U3DBG_ISR_STS("Sleep(L1) State\n");
			break;
		case LNKSTS_SUSPEND:
			U3DBG_ISR_STS("Suspend(L2) State\n");
			break;
		case LNKSTS_SS_DIS:
			U3DBG_ISR_STS("Disconnected State\n");
			break;
		case LNKSTS_ERLYSUS:
			U3DBG_ISR_STS("Early Suspend\n");
			/* true DP Pull-up if vbus pad is not used */
			dwc3_glb_phy_dp_pullup_en(dwc3_dev_h->glb_dev_h, true);
			break;
		case LNKSTS_RESET:
			U3DBG_ISR_STS("Reset\n");
			break;
		case LNKSTS_RESUME:
			U3DBG_ISR_STS("Resume\n");
			break;
		default:
			U3DBG_ISR_STS("Unkwon :: 0x%x\n", (uDevEvent->b.evt_info & 0xf));
			break;
		}
	}
}

void dwc3_dev_HandleDevEvent(DWC3_DEV_HANDLER dwc3_dev_h, USB3_DEV_DEVT_o *uDevEvent)
{
	switch (uDevEvent->b.evt_type) {
	case DEVT_USBRESET:
		usb_config_state = 0;
		timer_cancel(&config_timer);
		timer_set_periodic(&config_timer, 2000, dwc3_dev_config_check, dwc3_dev_h);
		U3DBG_ISR_DEV("USB Reset\n");
		dwc3_dev_HandleUsbResetInt(dwc3_dev_h);
		break;

	case DEVT_CONNDONE:
		U3DBG_ISR_DEV("Connect Done\n");
		dwc3_dev_HandleConnectDoneInt(dwc3_dev_h);
		break;
	case DEVT_WKUP:
		U3DBG_ISR_DEV_SUS("Wakeup\n");
		/* true DP Pull-up if vbus pad is not used */
		dwc3_glb_phy_dp_pullup_en(dwc3_dev_h->glb_dev_h, true);
		dwc3_dev_handle_resume_wakeup(dwc3_dev_h);
		break;
	case DEVT_SUSPEND_ENTRY:
		U3DBG_ISR_DEV_SUS("Enter Suspend(L2/U3)\n");
		dwc3_dev_handle_enter_suspend(dwc3_dev_h);
		break;
	case DEVT_L1_SUSPEND:
		U3DBG_ISR_DEV_SUS("Enter L1 Suspend\n");
		break;
	case DEVT_L1_RESUME_WAKE_UP:
		U3DBG_ISR_DEV_SUS("Exit L1 by Resume or Remote Wake-up\n");
		break;
	case DEVT_ULST_CHNG:
		U3DBG_ISR_DEV("Link Status Change\n");
		dwc3_dev_HandleLinkStatusChange(dwc3_dev_h, uDevEvent);
		break;
	case DEVT_DISCONN:
		U3DBG_ISR_DEV("Disconnect\n");
		dwc3_dev_HandleDisconnectInt(dwc3_dev_h);
		break;
	case DEVT_SOF:
		U3DBG_ISR_DEV("Start of Frame\n");
		break;
	case DEVT_ERRATICERR:
		U3DBG_ISR_DEV("Erratic Error\n");
		break;
	case DEVT_CMD_CMPL:
		U3DBG_ISR_DEV("Command Complete\n");
		break;
	case DEVT_OVERFLOW:
		U3DBG_ISR_DEV("Overflow\n");
		break;
	case DEVT_HIBERNATION:
		U3DBG_ISR_DEV("Hibernation Request\n");
		break;
	case DEVT_VNDR_DEV_TST_RCVD:
		U3DBG_ISR_DEV("Vendor Device Test LMP Received");
		break;
	default:
		U3DBG_ISR_DEV("Unknown event!");
	}
}

static void dwc3_dev_invaild_evntbuf(DWC3_DEV_HANDLER dwc3_dev_h)
{
	if (dwc3_dev_h->non_cachable)
		return;
	struct dwc3_dev_config *p_oDevConfig;
	p_oDevConfig = dwc3_dev_h->p_oDevConfig;

	U3DBG_CACHE("EVENT BUFFER INAVLID");
	U3DBG_CACHE("ADDR : 0x%lx Size : %d",
		    (addr_t) dwc3_dev_h->m_pEventBuffer,
		    4 * p_oDevConfig->m_uEventBufDepth);
	InvalidateDCache((u64) dwc3_dev_h->m_pEventBuffer,
			 4 * p_oDevConfig->m_uEventBufDepth);
}

void dwc3_dev_en_event(DWC3_DEV_HANDLER dwc3_dev_h, u8 intr_num, u8 bEn)
{
	USB3_REG_GEVNTSIZ_o usb3_gevntsiz = {
		DWC3_REG_RD32(rGEVNTSIZ(intr_num)),

	};
	if (bEn)
		usb3_gevntsiz.b.event_int_mask = 0;
	else
		usb3_gevntsiz.b.event_int_mask = 1;

	DWC3_REG_WR32(rGEVNTSIZ(intr_num), usb3_gevntsiz.data);
}

void dwc3_dev_set_event_buf(DWC3_DEV_HANDLER dwc3_dev_h, u8 intr_num,
	u32 *uAddr, u32 uDepth)
{
	USB3_REG_GEVNTSIZ_o usb3_gevntsiz;

	if (sizeof(u32 *) <= 4) {
		DWC3_REG_WR32(rGEVNTADR_LO(intr_num), (unsigned long) uAddr);
		DWC3_REG_WR32(rGEVNTADR_HI(intr_num), 0);
	} else {
		DWC3_REG_WR32(rGEVNTADR_LO(intr_num), (u64)uAddr & 0xffffffff);
		DWC3_REG_WR32(rGEVNTADR_HI(intr_num),
			      ((u64)uAddr >> 32) & 0xffffffff);
	}

	usb3_gevntsiz.data = DWC3_REG_RD32(rGEVNTSIZ(intr_num));
	usb3_gevntsiz.b.event_siz = uDepth * 4;
	DWC3_REG_WR32(rGEVNTSIZ(intr_num), usb3_gevntsiz.data);
}

void dwc3_dev_set_event_cnt(DWC3_DEV_HANDLER dwc3_dev_h, u8 intr_num,
							u32 uValue)
{
	DWC3_REG_WR32(rGEVNTCOUNT(intr_num), uValue * 4);
}

void dwc3_dev_flush_event_cnt(DWC3_DEV_HANDLER dwc3_dev_h, u8 intr_num)
{
	u32 uEventCnt = DWC3_REG_RD32(rGEVNTCOUNT(intr_num));

	if (uEventCnt)
		DWC3_REG_WR32(rGEVNTCOUNT(intr_num), uEventCnt);
}

u16 dwc3_dev_get_event_cnt(DWC3_DEV_HANDLER dwc3_dev_h, u8 intr_num)
{
	u32 data;

	data = DWC3_REG_RD32(rGEVNTCOUNT(intr_num));
	data /= 4;
	return (u16)data;
}

static void isr_dwc3_dev(void *arg)
{
	DWC3_DEV_HANDLER dwc3_dev_h = arg;
	DWC3_DEV_EP_HANDLER *hEP;
	struct dwc3_dev_config *dev_cfg;
	s16 sEventCnt = 0;
	u32 *evnt_buf_val;

	hEP = (DWC3_DEV_EP_HANDLER *) dwc3_dev_h->p_EPArrayAddress;
	dev_cfg = dwc3_dev_h->p_oDevConfig;

	/* Get Event Buffer Count */
	dwc3_dev_invaild_evntbuf(dwc3_dev_h);
	sEventCnt = dwc3_dev_get_event_cnt(dwc3_dev_h, dev_cfg->used_intr_num);
	if (sEventCnt) {
		int evnt_pos;

		U3DBG_ISR("## Event Count is %d ##\n", sEventCnt);
		evnt_pos = dwc3_dev_h->m_uCurrentEventPosition;
		while (sEventCnt-- > 0) {
			/* to avoid that event buffer is overwritten. */
			evnt_buf_val = dwc3_dev_h->m_pEventBuffer + evnt_pos;
			if (*evnt_buf_val == 0) {
				/* core update event buffer whenever event
				 * occurs */
				U3DBG_ISR("Null Event!");
			} else {
				// event buffer contains event information
				U3DBG_ISR("Event Position %d : Content of Event"
					  " Buffer is 0x%08x at %p\n",
					  dwc3_dev_h->m_uCurrentEventPosition,
					  *evnt_buf_val, evnt_buf_val);
				if (*evnt_buf_val & 0x1) {
					// Device-Specific Event
					USB3_DEV_DEVT_o *dwc3_dev_devt;
					dwc3_dev_devt = (USB3_DEV_DEVT_o *) evnt_buf_val;

					U3DBG_ISR("Device-Specific Event Occurred\n");
					if (dwc3_dev_devt->b.dev_specific != 0)
						U3DBG_ISR("Other Core Event");
					else
						dwc3_dev_HandleDevEvent(dwc3_dev_h, dwc3_dev_devt);
				} else {
					// Device Endpoint-Specific Event
					DWC3_DEV_DEPEVT_o *depevt;
					u32 uEpNum = 0;

					depevt = (DWC3_DEV_DEPEVT_o *) evnt_buf_val;
					uEpNum = depevt->b.ep_num;
					U3DBG_ISR("EP %d Event\n", uEpNum);
					if (uEpNum < 2) {
						dwc3_dev_ep0_isr(dwc3_dev_h, depevt);
					} else {
						int ep_id;

						ep_id = uEpNum;
						dwc3_dev_ep_isr(hEP[ep_id], ep_id, depevt);
					}
				}
			}
			/* Move Event Buffer Position; */
			evnt_pos++;
			evnt_pos %= dev_cfg->m_uEventBufDepth;
			/* Decrease Event Count Value */
			dwc3_dev_set_event_cnt(dwc3_dev_h,
						dev_cfg->used_intr_num, 1);
		}
		dwc3_dev_h->m_uCurrentEventPosition = evnt_pos;
	} else
		U3DBG_ISR("## NULL Event ##");

	if (dwc3_dev_h->m_bDisconnectNoti) {
		dwc3_dev_h->m_bDisconnectNoti = 0;
		gadget_notify_disconnect();
	} else
		dwc3_dev_invaild_evntbuf(dwc3_dev_h);
}

__attribute__((weak)) int dwc3_dev_plat_init(void **base_addr, struct dwc3_dev_config **plat_config){return -1;}

void *dwc3_dev_init_once(void)
{
	DWC3_DEV_HANDLER dwc3_dev_h;
	DWC3_DEV_EP0_HANDLER hdwc3_EP0 = NULL;
	u32 uEPCnt = 0;
	int ret;
	struct dwc3_dev_config *p_oDevConfig;

	dwc3_dev_h = calloc(1, sizeof(struct _dwc3_dev_handler));

	/* get base address and config from platform layer */
	ret = dwc3_dev_plat_init(&dwc3_dev_h->base, &dwc3_dev_h->p_oDevConfig);
	if (ret != 0) {
		LTRACEF("[dwc3_dev]not found platform inforamtion\n");
		return NULL;
	}
	p_oDevConfig = dwc3_dev_h->p_oDevConfig;

	/* Make Private data for USB30 DEV Driver */
	/* Event Buffer */
	dwc3_dev_h->m_pEventBuffer = dwc3_calloc_align(4 * p_oDevConfig->m_uEventBufDepth, 4 * p_oDevConfig->m_uEventBufDepth);
	/* BOS Descriptor */
	dwc3_dev_h->bos_desc_buf = dwc3_calloc_align(1024, 64);
	dwc3_dev_h->bos_desc_alloc_sz = 1024;
	/* configuration Descriptor */
	dwc3_dev_h->config_desc_buf = dwc3_calloc_align(1024, 64);
	dwc3_dev_h->config_desc_alloc_sz = 1024;

	if (p_oDevConfig->need_cache_ops)
		dwc3_dev_h->non_cachable = false;
	else
		dwc3_dev_h->non_cachable = true;

	// Initiate All EP Handler
	for (; uEPCnt < 32; uEPCnt++) {
		DWC3_DEV_EP_HANDLER hEP;

		hEP = calloc(1, sizeof(DWC3_DEV_EP));
		dwc3_dev_h->p_EPArrayAddress[uEPCnt] = hEP;
		g_hUSB30EP[uEPCnt] = hEP;

		// Set Initial value for EP Handler
		hEP->dwc3_dev_h = dwc3_dev_h;
		hEP->eState = DEV30_EP_STATE_INIT;
		hEP->m_ucEpNum = uEPCnt;
		hEP->trb_h = dwc3_trb_create(dwc3_dev_h->non_cachable,
					     (uEPCnt == 0) ? 0 : p_oDevConfig->on_demand);
		// EP Deinit for initialization of member variable
		dwc3_dev_ep_deinit(hEP);
	}
	/* Init buffers for EP0 */
	hdwc3_EP0 = calloc(1, sizeof(DWC3_DEV_EP0));
	// Will be
	dwc3_dev_h->p_EP0Address = hdwc3_EP0;

	// Status Buffer
	hdwc3_EP0->m_pStatusBuf = calloc(1, p_oDevConfig->m_uCtrlBufSize);
	// Device Request Buffer for control transfer
	hdwc3_EP0->m_pDeviceRequest = calloc(1, sizeof(USB_STD_REQUEST));

#if 0
	/* Save Private USB3.0 Device Handler */
	g_hUSB30 = dwc3_dev_h;
#endif

	return dwc3_dev_h;
}

static enum handler_return
dwc3_dev_config_check(struct timer *timer, unsigned int now, void *arg) {
	if (!usb_config_state) {
		muic_sw_usb();
		mdelay(50);
		printf("%s: DCTL runstop cnt %d!\n", __func__, retry_cnt);
		dwc3_dev_set_rs(arg, false);
		mdelay(50);
		dwc3_dev_set_rs(arg, true);
	} else {
		printf("%s: configuration done. delete timer!!\n", __func__);
		timer_cancel(&config_timer);
		retry_cnt = 0;
	}

	if (retry_cnt >= 3) {
		printf("%s: retry all fail!\n", __func__);
		timer_cancel(&config_timer);
		retry_cnt = 0;
	}
	retry_cnt++;

	return INT_NO_RESCHEDULE;
}

int dwc3_dev_init(void *dev_handle)
{
	u32 uRegData = 0;
	USB_SPEED default_speed;
	USB3_REG_DCTL_o DCTL;
	DWC3_DEV_HANDLER dwc3_dev_h;
	int ret;

	dwc3_dev_h = dev_handle;
	dwc3_dev_h->glb_dev_h = usb3_glb_init();
	if (!dwc3_dev_h->glb_dev_h) {
		LTRACEF("Can't not found global handle\n");
		return -1;
	}
	dwc3_dev_h->link_version = dwc3_dev_h->glb_dev_h->link_verion;
	/* check link version */
	if (!dwc3_dev_h->link_version) {
		LTRACEF("Fail: Get Link version ID\n");
		return -1;
	}
	/* Check Device is already connected, and must be disconnnect */
	if (dwc3_dev_is_running(dwc3_dev_h)) {
		/* Stop Link : Disconnect terminaion */
		dwc3_dev_set_rs(dwc3_dev_h, 0);
		mdelay(30);
		dwc3_dev_SetExitLinkStatus(dwc3_dev_h, LNKSTS_RX_DET);
	}
	/* Device Soft Reset */
	dwc3_dev_softreset(dwc3_dev_h);
	dwc3_glb_register_isr(dwc3_dev_h->glb_dev_h, USB_DEV,
			      dwc3_dev_h->p_oDevConfig->used_intr_num,
			      isr_dwc3_dev,
			      dwc3_dev_h);

	if (!strcmp(dwc3_dev_h->p_oDevConfig->speed, "full"))
		default_speed = USBSPEED_FULL;
	else if (!strcmp(dwc3_dev_h->p_oDevConfig->speed, "super"))
		default_speed = USBSPEED_SUPER;
	else if (!strcmp(dwc3_dev_h->p_oDevConfig->speed, "superplus"))
		default_speed = USBSPEED_SUPERPLUS;
	else
		default_speed = USBSPEED_HIGH;

	dwc3_dev_h->m_eEnumeratedSpeed = default_speed;
	/* Set Device Config Register */
	uRegData = 0;
	if (default_speed == USBSPEED_SUPERPLUS)
		((USB3_REG_DCFG_p)(&uRegData))->b.dev_speed = 0x5;
	else if (default_speed == USBSPEED_SUPER)
		((USB3_REG_DCFG_p)(&uRegData))->b.dev_speed = 0x4;
	else if (default_speed == USBSPEED_FULL)
		((USB3_REG_DCFG_p)(&uRegData))->b.dev_speed = 0x1;
	else
		((USB3_REG_DCFG_p)(&uRegData))->b.dev_speed = 0x0;

	((USB3_REG_DCFG_p)(&uRegData))->b.dev_addr = 0;
	((USB3_REG_DCFG_p)(&uRegData))->b.per_fr_int = 2;	// 90%
	((USB3_REG_DCFG_p)(&uRegData))->b.intr_num =
		dwc3_dev_h->p_oDevConfig->used_intr_num;
	((USB3_REG_DCFG_p)(&uRegData))->b.num_rx_buf = 4;
	((USB3_REG_DCFG_p)(&uRegData))->b.lpm_cap = 1;
	DWC3_REG_WR32(rDCFG, uRegData);

	// Set Device Interrupt Register
	uRegData = 0;
	((USB3_REG_DEVTEN_p)(&uRegData))->b.disconn_evt_en = 1;
	((USB3_REG_DEVTEN_p)(&uRegData))->b.usb_reset_en = 1;
	((USB3_REG_DEVTEN_p)(&uRegData))->b.conn_done_en = 1;
	((USB3_REG_DEVTEN_p)(&uRegData))->b.u3l2l1_susp_en = 1;
	((USB3_REG_DEVTEN_p)(&uRegData))->b.wake_up_en = 1;
	if (dwc3_dev_h->l1_evt_split)
		((USB3_REG_DEVTEN_p)(&uRegData))->b.l1_wkup_evt_en = 1;
	((USB3_REG_DEVTEN_p)(&uRegData))->b.usb_lnk_sts_chng_en = 0;
	((USB3_REG_DEVTEN_p)(&uRegData))->b.evnt_overflow_en = 1;
	DWC3_REG_WR32(rDEVTEN, uRegData);

	// Initialize Event buffer
	dwc3_dev_h->m_uCurrentEventPosition = 0;
	/* false Event buffer */
	dwc3_dev_en_event(dwc3_dev_h,
			  dwc3_dev_h->p_oDevConfig->used_intr_num,
			  false);
	/* Flush Current All Event counter */
	dwc3_dev_flush_event_cnt(dwc3_dev_h,
			dwc3_dev_h->p_oDevConfig->used_intr_num);
	/* event buffer */
	dwc3_dev_set_event_buf(dwc3_dev_h,
			dwc3_dev_h->p_oDevConfig->used_intr_num,
			dwc3_dev_h->m_pEventBuffer,
			dwc3_dev_h->p_oDevConfig->m_uEventBufDepth);
	/* true Global Interrupt for Device */
	dwc3_dev_invaild_evntbuf(dwc3_dev_h);
	/* true Event buffer */
	dwc3_dev_en_event(dwc3_dev_h,
			  dwc3_dev_h->p_oDevConfig->used_intr_num,
			  true);
	/* Init Control Transfer Sequence */
	dwc3_dev_ep0_init(dwc3_dev_h);
	/* Make Descriptor */
	/* Set Device Attribute Descitor (Device and BOS Desctiptor) */
	make_dev_desc(default_speed, &dwc3_dev_h->dec_desc);
	do {
		ret = make_bos_desc(default_speed, dwc3_dev_h->bos_desc_buf, dwc3_dev_h->bos_desc_alloc_sz);
		if (ret < 0) {
			dwc3_free(dwc3_dev_h->bos_desc_buf);
			dwc3_dev_h->bos_desc_alloc_sz *= 2;
			dwc3_calloc_align(dwc3_dev_h->bos_desc_alloc_sz, 64);
		} else
			dwc3_dev_h->bos_desc_total_sz = ret;
	} while (ret < 0);
	/* Make Descriptor for connected speed */
	do {
		ret = make_configdesc(default_speed, dwc3_dev_h->config_desc_buf, dwc3_dev_h->config_desc_alloc_sz);
		if (ret < 0) {
			dwc3_free(dwc3_dev_h->config_desc_buf);
			dwc3_dev_h->config_desc_alloc_sz *= 2;
			dwc3_calloc_align(dwc3_dev_h->config_desc_alloc_sz, 64);
		} else
			dwc3_dev_h->config_desc_total_sz = ret;
	} while (ret < 0);
	DCTL.data = DWC3_REG_RD32(rDCTL);
	DCTL.b.InitU1Ena = false;
	DCTL.b.InitU2Ena = false;
	DCTL.b.AcceptU1Ena = false;
	DCTL.b.AcceptU2Ena = false;
	DWC3_REG_WR32(rDCTL, DCTL.data);

	/* true Deivce */
	dwc3_dev_set_rs(dwc3_dev_h, true);

	dwc3_dev_h->fastboot_mode = true;

	muic_sw_usb();
	timer_initialize(&config_timer);
	timer_set_periodic(&config_timer, 2000, dwc3_dev_config_check, dwc3_dev_h);

	return 0;
}

void dwc3_dev_deinit(void *dev_handle)
{
	u32 uCnt;
	DWC3_DEV_EP0_HANDLER hdwc3_EP0;
	DWC3_DEV_EP_HANDLER *hdwc3_EP;
	DWC3_DEV_HANDLER dwc3_dev_h;

	dwc3_dev_h = dev_handle;
	hdwc3_EP0 = (DWC3_DEV_EP0_HANDLER)dwc3_dev_h->p_EP0Address;
	hdwc3_EP = (DWC3_DEV_EP_HANDLER *)dwc3_dev_h->p_EPArrayAddress;

	// false All Device Interrupts
	DWC3_REG_WR32(rDEVTEN, 0x0);
	dwc3_dev_set_rs(dwc3_dev_h, false);
	dwc3_dev_en_event(dwc3_dev_h,
			  dwc3_dev_h->p_oDevConfig->used_intr_num,
			  false);
	dwc3_dev_flush_event_cnt(dwc3_dev_h,
		dwc3_dev_h->p_oDevConfig->used_intr_num);
	for (uCnt = 0; uCnt < DWC3_DEV_TOTAL_EP_CNT; uCnt++)
		dwc3_dev_ep_deinit(hdwc3_EP[uCnt]);
	dwc3_dev_WaitDevControlHalted(dwc3_dev_h);
	/* Release ISR */
	dwc3_glb_release_isr(dwc3_dev_h->glb_dev_h, USB_DEV, dwc3_dev_h->p_oDevConfig->used_intr_num);
	// Clear member variable
	hdwc3_EP0->dwc3_dev_h = NULL;
	dwc3_dev_h->fastboot_mode = false;

	return;
}

static struct gadget_dev_ops dwc3_gadget_ops = {
	.get_dev_handle = dwc3_dev_init_once,
	.dev_start = dwc3_dev_init,
	.dev_stop = dwc3_dev_deinit,
	.dev_handle_isr = isr_dwc3_dev,

	.ep_req_cfg = gadget_ops_cfg_ep,
	.ep_xfer_set_buf = gadget_ops_set_buffer,
	.ep_set_cb_xferdone = gadget_ops_set_xfer_done,
	.ep_xfer_start = gadget_ops_ep_start,
	.ep_xfer_stop = gadget_ops_ep_stop,
	.ep_xfer_is_done = gadget_ops_is_done,
	.ep_xfer_wait_done = NULL,

	.ep_get_rx_sz = dwc3_dev_gadget_get_rx_sz,
};

/* Static Functions */
static void dwc3_gadget_probe(uint level)
{
	probe_dev_api_ops(&dwc3_gadget_ops);
}
LK_INIT_HOOK(dwc3_gadget_probe, &dwc3_gadget_probe, LK_INIT_LEVEL_PLATFORM_EARLY);
