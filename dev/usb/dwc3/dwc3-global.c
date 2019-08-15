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
#include <debug.h>
#include <reg.h>
#include <malloc.h>
#include <platform/delay.h>
#include <platform/interrupts.h>

#include <usb-def.h>
#include "dev/usb/dwc3-config.h"
#include "dev/phy-usb.h"
#include "dwc3-global.h"
#include "dwc3-reg.h"

#define DWC3_INP32(_REG)		readl(glb_dev->plat.base + _REG)
#define DWC3_OUTP32(_REG, _DATA)	writel(_DATA, glb_dev->plat.base + _REG)

static enum handler_return isr_dwc3(void *argv)
{
	struct dwc3_glb_dev *glb_dev = (struct dwc3_glb_dev *) argv;
	USB3_REG_GSTS_o GSTS;

	GSTS.data = DWC3_INP32(rGSTS);
	if (GSTS.b.DevIP && glb_dev->FnDRDCallBack[0] && glb_dev->ArgDRDCallBack[0])
		glb_dev->FnDRDCallBack[0](glb_dev->ArgDRDCallBack[0]);
	if (GSTS.b.HostIP && glb_dev->FnDRDCallBack[1] && glb_dev->ArgDRDCallBack[1])
		glb_dev->FnDRDCallBack[1](glb_dev->ArgDRDCallBack[1]);

	return INT_RESCHEDULE;
}

void dwc3_glb_en_intr(struct dwc3_glb_dev *glb_dev, int intr_num, bool en)
{
	if ((en == true) && (glb_dev->init_intr == 0)) {
		register_int_handler(glb_dev->plat.array_intr[intr_num], &isr_dwc3,
				glb_dev);
		unmask_interrupt(glb_dev->plat.array_intr[intr_num]);
		glb_dev->init_intr = 1;

	} else if ((en == false) && (glb_dev->init_intr == 1)) {
		mask_interrupt(glb_dev->plat.array_intr[intr_num]);
		glb_dev->init_intr = 0;
	}
}

void dwc3_glb_register_isr(struct dwc3_glb_dev *glb_dev,
	USB_OPMODE eOpMode,
	int intr_num, void (fnCallBack(void *)), void *pd_upper)
{
	switch (eOpMode) {
	case USB_DEV:
		glb_dev->FnDRDCallBack[0] = (void *) fnCallBack;
		glb_dev->ArgDRDCallBack[0] = pd_upper;
		break;
	case USB_HOST:
		glb_dev->FnDRDCallBack[1] = (void *) fnCallBack;
		glb_dev->ArgDRDCallBack[1] = pd_upper;
		break;
	default:
		dprintf(INFO, "[dwc3_glb]Ohter USB Mode is not supported yet!!\n");
		break;
	}

	/* true IRQ */
	dwc3_glb_en_intr(glb_dev, 0, true);
}

static u8 dwc3_glb_is_init(struct dwc3_glb_dev *glb_dev)
{
	if ((glb_dev->FnDRDCallBack[0] || glb_dev->FnDRDCallBack[1]
		|| glb_dev->FnDRDCallBack[2]) || glb_dev->FnDRDCallBack[3])
		return false;
	else
		return true;
}

void dwc3_glb_release_isr(struct dwc3_glb_dev *glb_dev, USB_OPMODE eOpMode, int intr_num)
{
	/* false */
	dwc3_glb_en_intr(glb_dev, 0, false);

	switch (eOpMode) {
	case USB_DEV:
		glb_dev->FnDRDCallBack[0] = NULL;
		break;
	case USB_HOST:
		glb_dev->FnDRDCallBack[1] = NULL;
		break;
	case USB_OPMODE_NONE:
		break;
	default:
		dprintf(INFO, "[dwc3_usb]Ohter USB Mode is not supported yet!!\n");
		break;
	}
}

u32 dwc3_glb_get_ipid(struct dwc3_glb_dev *glb_dev)
{
	return DWC3_INP32(rGSNPSID) & 0xffff;
}

void dwc3_glb_unmask_suspend(struct dwc3_glb_dev *glb_dev, u8 bSetValue)
{
	USB3_REG_GUSB2PHYCFG_o regGUSB2PHYCFG;
	u8 clear_mark = 0;

	if (glb_dev->susp_sema_flag) {
		if (!bSetValue) {
			glb_dev->susp_sema_flag++;
			return;
		} else {
			glb_dev->susp_sema_flag--;
			if (glb_dev->susp_sema_flag)
				return;
			else
				clear_mark = 1;
		}
	} else
		glb_dev->susp_sema_flag++;

	regGUSB2PHYCFG.data = DWC3_INP32(rGUSB2PHYCFG);

	if (regGUSB2PHYCFG.b.SusPHY) {
		regGUSB2PHYCFG.b.SusPHY = bSetValue & 0x1;
		glb_dev->susp_set_flag = 1;
	} else if (clear_mark && glb_dev->susp_set_flag) {
		regGUSB2PHYCFG.b.SusPHY = 1;
		glb_dev->susp_set_flag = 0;
	}

	if (glb_dev->link_verion > 0x0280a) {
		if (regGUSB2PHYCFG.b.EnblSlpM) {
			regGUSB2PHYCFG.b.EnblSlpM = bSetValue & 0x1;
			glb_dev->sleep_set_flag = 1;
		} else if (clear_mark && glb_dev->sleep_set_flag)
			regGUSB2PHYCFG.b.EnblSlpM = 1;
	}

	DWC3_OUTP32(rGUSB2PHYCFG, regGUSB2PHYCFG.data);
}

void dwc3_glb_set_ramclk(struct dwc3_glb_dev *glb_dev, const char *ram_clk)
{
	USB3_REG_GCTL_o oRegGCTL;

	oRegGCTL.data = DWC3_INP32(rGCTL);

	if (!strcmp(ram_clk, "pipe"))
		oRegGCTL.b.ram_clk_sel = USB3_RAMCLK_PIPE;
	else if (!strcmp(ram_clk, "pipe/2"))
		oRegGCTL.b.ram_clk_sel = USB3_RAMCLK_PIPE_DIV2;
	else if (!strcmp(ram_clk, "mac2"))
		oRegGCTL.b.ram_clk_sel = USB3_RAMCLK_MAC2;
	else
		oRegGCTL.b.ram_clk_sel = USB3_RAMCLK_BUSCLK;

	DWC3_OUTP32(rGCTL, oRegGCTL.data);
}

void dwc3_glb_set_refclk(struct dwc3_glb_dev *glb_dev, int sclk_rate)
{
	double dPeriodOfRefClk = 1000000000 / (double) sclk_rate;
	u32 uPeriodOfRefClk = (u32) dPeriodOfRefClk;
	u32 reg;
	u32 reg_fladj;
	double temp1, temp2, temp3;

	reg = DWC3_INP32(rGCTL);
	reg_fladj = DWC3_INP32(rGFLADJ);

#if 1
	temp1 = 125000 / (double) uPeriodOfRefClk;
	temp2 = 125000 / dPeriodOfRefClk;
	temp3 = (temp1 - temp2) * dPeriodOfRefClk;
	((USB3_REG_FLADJ_p) (&reg_fladj))->b.GFLADJ_REFCLK_FLADJ =(u32) temp3;
	dprintf(INFO, "[dwc3_usb]125000/uPeriodOfRefClk: %2.3lf\n",
		 temp1);
	dprintf(INFO, "[dwc3_usb]125000/dPeriodOfRefClk: %2.3lf\n",
		 temp2);
	dprintf(INFO, "[dwc3_usb]GFLADJ_REFCLK_FLADJ(double): %2.3lf\n",
		 temp3);
#else
	((USB3_REG_FLADJ_p) (&reg_fladj))->b.GFLADJ_REFCLK_FLADJ = 0;
#endif
	if (glb_dev->plat.config.refclk_lpm) {
		int first_digit;
		int sclk_mhz = (int)(sclk_rate / 1000000);
		double refclk_240mhz = 240 / (double) sclk_mhz;

		/* SOFITPSYNC = 0 and REFCLK_LPM_SEL = 1*/
		((USB3_REG_GCTL_p) (&reg))->b.SOFITPSYNC = 0;
		((USB3_REG_FLADJ_p) (&reg_fladj))->b.GFLADJ_REFCLK_LPM_SEL = 1;

		/* Set Refclk 240MHz Decrement */
		first_digit = (int) (240 / (double) sclk_mhz);
		((USB3_REG_FLADJ_p) (&reg_fladj))->b.GFLADJ_REFCLK_240MHZ_DECR =
				first_digit;
		first_digit = (refclk_240mhz - first_digit) * 1000;
		if (first_digit > 498) {
			((USB3_REG_FLADJ_p) (&reg_fladj))->b.GFLADJ_REFCLK_240MHZ_DECR =
				((USB3_REG_FLADJ_p) (&reg_fladj))->b.GFLADJ_REFCLK_240MHZ_DECR + 1;
			((USB3_REG_FLADJ_p) (&reg_fladj))->b.GFLADJ_REFCLK_240MHZDECR_PLS1 = 1;
		} else
			((USB3_REG_FLADJ_p) (&reg_fladj))->b.GFLADJ_REFCLK_240MHZDECR_PLS1 = 0;
	} else {
		/* SOFITPSYNC = 1 and REFCLK_LPM_SEL = 0 */
		((USB3_REG_GCTL_p) (&reg))->b.SOFITPSYNC = 1;
		((USB3_REG_FLADJ_p) (&reg_fladj))->b.GFLADJ_REFCLK_LPM_SEL = 0;
	}
	DWC3_OUTP32(rGCTL, reg);
	DWC3_OUTP32(rGFLADJ, reg_fladj);

	dprintf(INFO, "[dwc3_usb]period of ref_clk : %2.3lf nsec\n",
		 dPeriodOfRefClk);
	dprintf(INFO, "[dwc3_usb]integer value : %d nsec\n",
		 uPeriodOfRefClk);
	dprintf(INFO, "[dwc3_usb]refclk_fladj : %d\n",
		((USB3_REG_FLADJ_p) (&reg_fladj))->b.GFLADJ_REFCLK_FLADJ);

	reg = DWC3_INP32(rGUCTL);
	((USB3_REG_GUCTL_p) (&reg))->b.REFCLKPER = uPeriodOfRefClk;
	DWC3_OUTP32(rGUCTL, reg);
}

static struct dwc3_glb_dev glb_handle;
__attribute__((weak)) int dwc3_plat_init(struct dwc3_plat_config *plat_config){return -1;}

struct dwc3_glb_dev *usb3_glb_init(void)
{
	struct dwc3_glb_dev *glb_dev;
	USB3_REG_GUSB2PHYCFG_o oRegUSB2PHYCFG;
	USB3_REG_GUSB3PIPECTL_o oRegUSB3PIPECTL;
	u32 reg;
	int ret;

	glb_dev = &glb_handle;

	/* Check already initated */
	if (glb_dev->eUSBMode != USB_OPMODE_NONE)
		return glb_dev;

	if (!dwc3_glb_is_init(glb_dev))
		return glb_dev;

	ret = dwc3_plat_init(&glb_dev->plat);
	if (ret != 0)
		return NULL;

	oRegUSB3PIPECTL.data = DWC3_INP32(rGUSB3PIPECTL);
	oRegUSB3PIPECTL.b.PHYSoftRst = 1;
	DWC3_OUTP32(rGUSB3PIPECTL, oRegUSB3PIPECTL.data);

	oRegUSB2PHYCFG.data = DWC3_INP32(rGUSB2PHYCFG);
	oRegUSB2PHYCFG.b.PHYSoftRst = 1;
	DWC3_OUTP32(rGUSB2PHYCFG, oRegUSB2PHYCFG.data);

	udelay(50);

	oRegUSB2PHYCFG.b.PHYSoftRst = 0;
	oRegUSB2PHYCFG.b.SusPHY = 0;
	oRegUSB2PHYCFG.b.EnblSlpM = 0;
	DWC3_OUTP32(rGUSB2PHYCFG, oRegUSB2PHYCFG.data);

	oRegUSB3PIPECTL.b.suspend_usb3_ss_phy = 0;
	oRegUSB3PIPECTL.b.PHYSoftRst = 0;
	DWC3_OUTP32(rGUSB3PIPECTL, oRegUSB3PIPECTL.data);

	/* TODO:true PHY and System material for global block used */
	ret = phy_usb_init();
	if (ret != 0)
		return NULL;

	/* Return Link version ID */
	glb_dev->link_verion = DWC3_INP32(rGSNPSID) & 0xffff;
	glb_dev->link_id = (DWC3_INP32(rGSNPSID) & 0xffff0000) >> 16;

	dprintf(INFO, "[dwc3_glb]IP Code : %c%c => %s\n",
		(glb_dev->link_id & 0xff00) >> 8,
		glb_dev->link_id & 0xff,
		(glb_dev->link_id == 0x5533) ? "Super Speed" :
				"Super Speed Plus");

	dprintf(INFO, "[dwc3_glb]IP version is %1x.%03x\n",
		(glb_dev->link_verion & 0xf000) >> 12,
		(glb_dev->link_verion & 0x0fff));

	glb_dev->susp_sema_flag = 0;
	glb_dev->susp_set_flag = 0;
	glb_dev->sleep_set_flag = 0;

	reg = DWC3_INP32(rGFLADJ);
	((USB3_REG_FLADJ_p) & reg)->b.GFLADJ_30MHZ_REG_SEL = 0x1;
	((USB3_REG_FLADJ_p) & reg)->b.GFLADJ_30MHz = 0x20;
	DWC3_OUTP32(rGFLADJ, reg);

	return glb_dev;
}

static void usb3_glb_late_init_utmi(struct dwc3_glb_dev *glb_dev, int num_phy, int ref_clk)
{
	USB3_REG_GUSB2PHYCFG_o oRegUSB2PHYCFG;

	oRegUSB2PHYCFG.data = DWC3_INP32(rGUSB2PHYCFG);
	/* UTMI IF Selection
	 * 0 : 8Bit 60MHz, 1 : 16bit 30MHz */
	if (glb_dev->plat.config.m_bUTMIIFSel == 0) {
		oRegUSB2PHYCFG.b.PHYIf = 0;
		oRegUSB2PHYCFG.b.USBTrdTim = 9;
	} else {
		oRegUSB2PHYCFG.b.PHYIf = 1;
		oRegUSB2PHYCFG.b.USBTrdTim = 5;
	}
	// PHY UTMI Power Mode(L1 Sleep/L2 Suspend)
	if (glb_dev->plat.config.m_bEnableL2SusCtrl == true) {
		// Controlled by DRD Link HW
		oRegUSB2PHYCFG.b.SusPHY = 1;
		oRegUSB2PHYCFG.b.EnblSlpM = 1;
	} else {
		// Controlled by DRD Link HW
		oRegUSB2PHYCFG.b.SusPHY = 0;
		oRegUSB2PHYCFG.b.EnblSlpM = 0;
	}
	oRegUSB2PHYCFG.b.U2_FREECLK_EXISTS = 0;
	DWC3_OUTP32(rGUSB2PHYCFG, oRegUSB2PHYCFG.data);

	dwc3_glb_set_refclk(glb_dev, ref_clk);
}

static void usb3_glb_late_init_pipe3(struct dwc3_glb_dev *glb_dev, int num_phy)
{
	struct dwc3_global_config *config = &glb_dev->plat.config;
	USB3_REG_GUSB3PIPECTL_o pipectl;
	u32 reg;

	/* PIPE Interface control */
	pipectl.data = DWC3_INP32(rGUSB3PIPECTL_NUM(num_phy));
	/* for Host Operation */
	if (config->m_bEnableU3Ctrl == true)
		pipectl.b.suspend_usb3_ss_phy = 1;
	else
		pipectl.b.suspend_usb3_ss_phy = 0;

	if (config->bForceU3Ctrl == true)
		pipectl.b.suspend_usb3_ss_phy = 1;

	/* These Options are possible if Link Version is upper than
	 * 2.50a */
	if (glb_dev->link_verion >= 0x0250a) {
		/* Only Downstream Port PHY Control Option
		 * RX Detection When U3 State :: Default false */
		if (config->m_bEnableRxDetectP3 == true)
			pipectl.b.DisRxDetP3 = 1;
		else
			pipectl.b.DisRxDetP3 = 0;
		// for Power Consumption
		if (config->m_bP3WhenU2 == true)
			pipectl.b.U2SSInactP3ok = 1;
		else
			pipectl.b.U2SSInactP3ok = 0;
	}
	if (glb_dev->link_verion < 0x0220a)
		pipectl.b.DelayP1P2P3 = 4;
	else
		pipectl.b.DelayP1P2P3 = 1;
	/* TODO: Get SS PHY Tune value */
#if 0
	/* Set PHY Tune Value */
	pipectl.b.TxSwing = tune->tx_swing_level;
	pipectl.b.TxDeemphasis = tune->tx_deemphasis_mode;
	pipectl.b.DelayPhyPwrChng = 0;
#endif
	if (!strcmp(glb_dev->plat.ssphy_type, "samsung_ssphy")) {
		/* Lhotse configuration */
		pipectl.b.u1u2exitfail_to_recov = 1;
		pipectl.b.P3ExSigP2 = 1;
		pipectl.b.UxExitInPx = 1;
	} else if (!strcmp(glb_dev->plat.ssphy_type, "samsung_ssphy_v2")) {
		/* Makalu configuration */
		pipectl.b.ElasticBufferMode = 1;
		pipectl.b.UxExitInPx = 1;
	}
	DWC3_OUTP32(rGUSB3PIPECTL_NUM(num_phy), pipectl.data);

	if (!strcmp(glb_dev->plat.ssphy_type, "samsung_ss_v2")) {
		void *pReg = &reg;

		reg = DWC3_INP32(rLLUCTL_NUM(num_phy));
		((USB3_REG_LLUCTL_p) pReg)->b.pending_hp_timer_us = 0xb;
		((USB3_REG_LLUCTL_p) pReg)->b.en_us_hp_timer = 0x1;

		// force gen1 only option
		//((USB3_REG_LLUCTL_p) pReg)->b.force_gen1 = 0x1;
		// Gen2 compatibility enhanced - 2018.08.21
		((USB3_REG_LLUCTL_p) pReg)->b.tx_ts1_cnt = 0xb;

		DWC3_OUTP32(rLLUCTL_NUM(num_phy), reg);

		reg = DWC3_INP32(rLSKIPFREQ_NUM(num_phy));
		((USB3_REG_LSKIPFREQ_p) pReg)->b.pm_entry_timer_us = 0x9;
		((USB3_REG_LSKIPFREQ_p) pReg)->b.pm_lc_timer_us = 0x5;
		((USB3_REG_LSKIPFREQ_p) pReg)->b.en_pm_timer_us = 0x1;
		DWC3_OUTP32(rLSKIPFREQ_NUM(num_phy), reg);

		/* 9001276244 B2  LFPS Handshake Interoperability Issues */
		reg = DWC3_INP32(rLU3LFPSRXTIM_NUM(num_phy));
		((USB3_REG_LU3LFPSRXTIM_p) pReg)->b.gen1_u3_exit_rsp_rx_clk = 0x05;
		((USB3_REG_LU3LFPSRXTIM_p) pReg)->b.gen2_u2_exit_rsp_rx_clk = 0x06;
		DWC3_OUTP32(rLU3LFPSRXTIM_NUM(num_phy), reg);

	}
}

int usb3_glb_init_early(struct dwc3_glb_dev *glb_dev, USB_OPMODE mode)
{
	unsigned long uSCLK;
	USB3_REG_GCTL_o oRegGCTL;

	/* get sclk for ref_clk and suspend_clk */
	uSCLK = glb_dev->plat.ref_clk;
	dprintf(INFO, "[dwc3_usb]SCLK for USBDRD : %3.3lfMHz(%luHz)\n",
		uSCLK / (double) 1000000, uSCLK);
	// TODO: PHY Initiated
#if 0
	if (glb_dev->num_of_phy == -1) {
		dprintf(INFO,
			 "[dwc3_usb]Single PHY Architecture\n");
		/* Single PHY Architecture */
		if (!subdev_call_ioctl(glb_dev->phydev, common,
				       USBPHY_CONFIG_IS_SUPPORT,
				       USBSPEED_HIGH)) {
			usb3_glb_late_init_utmi(glb_dev, mode, 0,
						uSCLK, glb_dev->phydev);
		} else
			dwc3_glb_set_refclk(glb_dev, uSCLK);

		if (!subdev_call_ioctl(glb_dev->phydev, common,
				       USBPHY_CONFIG_IS_SUPPORT,
				       USBSPEED_SUPER)) {
			usb3_glb_late_init_pipe3(glb_dev, mode, 0,
						 glb_dev->phydev);
		}
	} else if (glb_dev->num_of_phy > 0) {
		int phy_cnt, utmi_phy_cnt = 0, pipe_phy_cnt = 0;

		dprintf(INFO,
			 "[dwc3_usb]Multiple PHY Architecture\n");
		for (phy_cnt = 0; phy_cnt < glb_dev->num_of_phy; phy_cnt++) {
			void *tune = NULL;
			int ret_hs;
			struct subdev_desc *phydev;

			phydev = glb_dev->desc_phy_list[phy_cnt];
			ret_hs = subdev_call_ioctl(phydev, common,
						   USBPHY_CONFIG_IS_SUPPORT,
						   USBSPEED_HIGH);
			if (!ret_hs) {
				dprintf(INFO,
					"phy %d is %dth UTMI PHY\n",
					phy_cnt + 1,
					utmi_phy_cnt + 1);
				usb3_glb_late_init_utmi(glb_dev, mode,
							utmi_phy_cnt,
							uSCLK, phydev);
				utmi_phy_cnt++;
			} else {
				/* Get SS Tune parameter structure */
				subdev_call_ioctl(phydev, common,
						  USBPHY_CONFIG_GET_TUNE,
						  USBSPEED_SUPER,
						  &tune);

				dprintf(INFO,
					"phy %d is %dth PIPE PHY\n",
					phy_cnt + 1,
					pipe_phy_cnt + 1);

				usb3_glb_late_init_pipe3(glb_dev, mode,
							 pipe_phy_cnt, phydev);
				pipe_phy_cnt++;
			}
		}
		if (utmi_phy_cnt == 0)
			dwc3_glb_set_refclk(glb_dev, uSCLK);
	} else {
		dprintf(CRITICAL,
			"[dwc3_usb]No PHY Mode : Only for test\n");
	}
#else

#endif
	/* GCTL : Global Control Register */
	oRegGCTL.data = DWC3_INP32(rGCTL);
	if (mode == USB_DEV) {
		oRegGCTL.b.port_capdir = 0x2;
		oRegGCTL.b.u2rst_ecn = 1;
		/* Most Case Bus Clock is faster than PIPE3 Clock */
		oRegGCTL.b.ram_clk_sel = USB3_RAMCLK_BUSCLK;
	} else if (mode == USB_HOST) {
		oRegGCTL.b.port_capdir = 0x1;
		if (glb_dev->link_verion < 0x0290a) {
			/* In host mode, this bit must be set to 2'b00, that is,
			 * the ram_clk must be set to bus_clk only. */
			oRegGCTL.b.ram_clk_sel = USB3_RAMCLK_BUSCLK;
		} else {
			/* In host mode, if a value of 2/3 is chosen,
			 * then controller switches ram_clk between bus_clk,
			 * mac2_clk and pipe_clk, pipe_clk/2, based on the
			 * state of the U2/U3 ports. For example if only the
			 * U2 port is active and the U3 ports are suspended,
			 * then the ram_clk is swicthed to mac2_clk. When only
			 * the U3 ports are active and the U2 ports are
			 * suspended, then the core internally swicth the
			 * ram_clk to pipe3 clock and when all U2 and U3 ports
			 * are suspended, it switch the ram_clk to bus_clk.
			 * This allows de coupling the ram_clk from the bus_clk
			 * and depending on the bandwidth requiredmnet allows
			 * the bus_clk to be run at a lower frequency than
			 * the ram_clk requirements. bus_clk frequency still
			 * cannot go below 60Mhz in host mode, and
			 * this is not verified.*/
			oRegGCTL.b.ram_clk_sel = USB3_RAMCLK_MAC2;
		}
		oRegGCTL.b.ram_clk_sel = USB3_RAMCLK_BUSCLK;
	} else {
		/* OTG Mode, any exynos is not supproted */
		oRegGCTL.b.port_capdir = 0x3;
	}
	/* Data Scrambling : 0[D] : true, 1 : false */
	oRegGCTL.b.DisScramble = 0x0;
	/* VBUS Valid Filter Bypass : Not used VBUS Pin */
	oRegGCTL.b.MasterFiltBypass = 0x1;
	/* Scale down divider for suspend clock */
	uSCLK = glb_dev->plat.suspend_clk;
	oRegGCTL.b.pwr_down_scale = (u32) (uSCLK / 16000);
	dprintf(INFO, "[dwc3_usb]Power Down Scaler : %d(0x%x)\n", oRegGCTL.b.pwr_down_scale, oRegGCTL.b.pwr_down_scale);
	dprintf(INFO, "[dwc3_usb]Suspend CLK : %2.3lfKHz\n", (double) uSCLK / oRegGCTL.b.pwr_down_scale / 1000);
	// Auto Clock Gating
	if (glb_dev->plat.config.m_bDisableAutoClkGating)
		oRegGCTL.b.dis_clk_gating = 0x1;
	else
		oRegGCTL.b.dis_clk_gating = 0x0;
#if defined(CONFIG_BOARD_ZEBU)
	/* Scale Down Bit 0 :
	 * HS : true Scale-down of all timing value except Device mode
	 *      suspend/resume. These include speed enum, HNP/SRP and Host mode
	 *      suspend/resume.
	 * SS : true scale-down SS Timing and repeat values include
	 *      (1) Number of TxEq Training sequence reduce to 8;
	 *      (2) LFPS polling burst time reduce to 256nS;
	 *      (3) LFPS Warm reset receive reduce to 30uS.
	 * Scale Down Bit 1 :
	 * HS : true Scale-down of Device mode suspend/resume mode only.
	 * SS : No TxEq Training sequences ar sent. Overrides bit 0 of scaledown
	 */
	if (glb_dev->config->scale_down_time)
		oRegGCTL.b.ScaleDown = 0x3;	// 0x1 for FS mode.
	else
		oRegGCTL.b.ScaleDown = 0x0;
#else
	oRegGCTL.b.ScaleDown = 0;
#endif
	oRegGCTL.b.DisScramble = 0;
	DWC3_OUTP32(rGCTL, oRegGCTL.data);

	return 0;
}

int dwc3_glb_init_late(struct dwc3_glb_dev *glb_dev, USB_OPMODE mode)
{
	int ret = 0;
	u32 reg;
	void *pReg = &reg;
	struct dwc3_global_config *cfg = &glb_dev->plat.config;
	u32 version = glb_dev->link_verion;

	/* TODO:Init PCS Block in the PHY */
	//usb30_plat_subdev_phy_init_late(glb_dev, mode);

	/* Global Configuration */
	// . to configure GSBUSCFG0
	//----------------------------------------------
	reg = DWC3_INP32(rGSBUSCFG0);
	if (glb_dev->plat.config.m_eBurstLength & (u32) USB3_INTCEN)
		((USB3_REG_GSBUSCFG0_p) pReg)->b.incr_xbrst_ena = 1;
	else
		((USB3_REG_GSBUSCFG0_p) pReg)->b.incr_xbrst_ena = 0;

	if (glb_dev->plat.config.m_eBurstLength & (u32) USB3_INCR4)
		((USB3_REG_GSBUSCFG0_p) pReg)->b.incr_4brst_ena = 1;
	else
		((USB3_REG_GSBUSCFG0_p) pReg)->b.incr_4brst_ena = 0;

	if (glb_dev->plat.config.m_eBurstLength & (u32) USB3_INCR8)
		((USB3_REG_GSBUSCFG0_p) pReg)->b.incr_8brst_ena = 1;
	else
		((USB3_REG_GSBUSCFG0_p) pReg)->b.incr_8brst_ena = 0;

	if (glb_dev->plat.config.m_eBurstLength & (u32) USB3_INCR16)
		((USB3_REG_GSBUSCFG0_p) pReg)->b.incr_16brst_ena = 1;
	else
		((USB3_REG_GSBUSCFG0_p) pReg)->b.incr_16brst_ena = 0;
	if (glb_dev->link_verion >= 0x0210a) {
		// Set AWCACHE/ARCACHE Attributte for DMA Master Request
		((USB3_REG_GSBUSCFG0_p) pReg)->b.des_wr_req_infor = 0x2;
		((USB3_REG_GSBUSCFG0_p) pReg)->b.data_wr_req_infor = 0x2;
		((USB3_REG_GSBUSCFG0_p) pReg)->b.des_rd_req_infor = 0x2;
		((USB3_REG_GSBUSCFG0_p) pReg)->b.data_rd_req_infor = 0x2;
	}
	DWC3_OUTP32(rGSBUSCFG0, reg);

	/* Avoid Babble Error for SS Host */
	if (mode == USB_HOST && version <= 0x0210a) {
		// for Host WA Test SS Bulk EP : STAR
		reg = DWC3_INP32(rGTXTHRCFG);
		((USB3_REG_GTXTHRCFG_p) pReg)->b.ss.USBTxPktCntSel = 1;
		((USB3_REG_GTXTHRCFG_p) pReg)->b.ss.USBTxPktCnt = 3;
		((USB3_REG_GTXTHRCFG_p) pReg)->b.ss.USBMaxTxBurstSize = 3;
		DWC3_OUTP32(rGTXTHRCFG, reg);
		// for Host WA Test SS Bulk In EP :
		reg = DWC3_INP32(rGRXTHRCFG);
		if (glb_dev->link_id == 0x5533) {
			((USB3_REG_GRXTHRCFG_p) pReg)->b.ss.USBRxPktCntSel = 1;
			((USB3_REG_GRXTHRCFG_p) pReg)->b.ss.USBRxPktCnt = 3;
			((USB3_REG_GRXTHRCFG_p) pReg)->b.ss.USBMaxRxBurstSize = 3;
		}
		DWC3_OUTP32(rGRXTHRCFG, reg);
	} else {
		reg = DWC3_INP32(rGTXTHRCFG);
		((USB3_REG_GTXTHRCFG_p) pReg)->b.ss.USBTxPktCntSel = 1;
		((USB3_REG_GTXTHRCFG_p) pReg)->b.ss.USBTxPktCnt = 3;
		((USB3_REG_GTXTHRCFG_p) pReg)->b.ss.USBMaxTxBurstSize = 3;
		DWC3_OUTP32(rGTXTHRCFG, reg);
		// for Host WA Test SS Bulk In EP :
		reg = DWC3_INP32(rGRXTHRCFG);
		if (glb_dev->link_id == 0x5533) {
			((USB3_REG_GRXTHRCFG_p) pReg)->b.ss.USBRxPktCntSel = 1;
			((USB3_REG_GRXTHRCFG_p) pReg)->b.ss.USBRxPktCnt = 3;
			((USB3_REG_GRXTHRCFG_p) pReg)->b.ss.USBMaxRxBurstSize = 3;
		}
		DWC3_OUTP32(rGRXTHRCFG, reg);
	}
	/* link 1.70a global IP Name(0xC120) = 0x3331_3130 */
	if (glb_dev->link_id == 0x3331) {
		reg = DWC3_INP32(rGTXTHRCFG);
		((USB3_REG_GTXTHRCFG_p) pReg)->b.ssp.UsbMaxTxBurstSize = 0x1f;
		((USB3_REG_GTXTHRCFG_p) pReg)->b.ssp.UsbTxPktCnt = 0x7;
		((USB3_REG_GTXTHRCFG_p) pReg)->b.ssp.UsbTxPktCntSel = 1;
		DWC3_OUTP32(rGTXTHRCFG, reg);

		reg = DWC3_INP32(rGRXTHRCFG);
		((USB3_REG_GRXTHRCFG_p) pReg)->b.ssp.UsbMaxRxBurstSize = 0x1f;
		((USB3_REG_GRXTHRCFG_p) pReg)->b.ssp.UsbRxPktCnt = 0x7;
		((USB3_REG_GRXTHRCFG_p) pReg)->b.ssp.UsbRxPktCntSel = 1;
		DWC3_OUTP32(rGRXTHRCFG, reg);
	}

	reg = DWC3_INP32(rGUCTL);
	if (glb_dev->link_id == 0x5533) {
		((USB3_REG_GUCTL_p) pReg)->b.DTCT = 0x3;
		((USB3_REG_GUCTL_p) pReg)->b.DTFT = 0x1ff;
	} else {
		((USB3_REG_GUCTL_p) pReg)->b.DTCT = 0x3;
		((USB3_REG_GUCTL_p) pReg)->b.DTFT = 0x1ff;
	}
	((USB3_REG_GUCTL_p) pReg)->b.USBHstInAutoRetryEn = 1;
	/* Enhanced HS Host Performance :
	 *	remove  2usec delay after SOF Packet */
	((USB3_REG_GUCTL_p) pReg)->b.NoExtrDl = 1;
	/* Enhanced HS Device  :
	 *	increase enumeration success rate
	 *	but current F/W does not effect */
	if (cfg && cfg->sparse_en)
		((USB3_REG_GUCTL_p) pReg)->b.SprsCtrlTransEn = 1;
	else
		((USB3_REG_GUCTL_p) pReg)->b.SprsCtrlTransEn = 0;
	DWC3_OUTP32(rGUCTL, reg);

	reg = DWC3_INP32(rGUCTL1);
	((USB3_REG_GUCTL1_p) pReg)->b.LOA_FILTER_EN = 0x1;
	if (cfg && cfg->dev_l1_evt_split && version >= 0x0300a)
		((USB3_REG_GUCTL1_p) pReg)->b.DEV_DECOUPLE_L1L2_EVT = 0x1;
	else
		((USB3_REG_GUCTL1_p) pReg)->b.DEV_DECOUPLE_L1L2_EVT = 0x0;
	if (cfg && cfg->dev_hw_l1_wakeup && version >= 0x0290a)
		((USB3_REG_GUCTL1_p) pReg)->b.DEV_L1_EXIT_BY_HW = 0x1;
	else
		((USB3_REG_GUCTL1_p) pReg)->b.DEV_L1_EXIT_BY_HW = 0x0;
	DWC3_OUTP32(rGUCTL1, reg);

	if (glb_dev->link_id == 0x5533 && version >= 0x0310a) {
		reg = DWC3_INP32(rGUCTL2);
		((USB3_REG_GUCTL2_p) pReg)->b.DisableCFC = 0x1;
		DWC3_OUTP32(rGUCTL2, reg);
	}

	return ret;
}

int usb3_glb_deinit(struct dwc3_glb_dev *glb_dev)
{
	int ret = 0;
	u32 reg;

	// PHY Power state controlled only SW through PHYUTMI
	reg = DWC3_INP32(rGUSB2PHYCFG);
	((USB3_REG_GUSB2PHYCFG_o *) (&reg))->b.SusPHY = 1;
	((USB3_REG_GUSB2PHYCFG_o *) (&reg))->b.EnblSlpM = 1;
	DWC3_OUTP32(rGUSB2PHYCFG, reg);

	// PIPE3 Control : SS PHY Operation Configuration
	reg = DWC3_INP32(rGUSB3PIPECTL);
	((USB3_REG_GUSB3PIPECTL_o *) (&reg))->b.suspend_usb3_ss_phy = 1;
	DWC3_OUTP32(rGUSB3PIPECTL, reg);

	/* Release IRQ */
	dwc3_glb_en_intr(glb_dev, 0, false);

	/* true PHY and System material for global block used */
	ret = phy_usb_terminate();

	glb_dev->eUSBMode = USB_OPMODE_NONE;

	return ret;

}

int usb3_glb_deinit_early(struct dwc3_glb_dev *glb_dev, USB_OPMODE mode)
{
	int ret = 0;

	return ret;

}

void dwc3_glb_phy_dp_pullup_en(struct dwc3_glb_dev *glb_dev, bool en_pullup)
{

}
