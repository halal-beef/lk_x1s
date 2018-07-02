/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.


 * Alternatively, this program is free software in case of open source project
 * you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

//#include <common.h>
//#include <malloc.h>
//#include <watchdog.h>
//#include <asm/arch/cpu.h>
//#include <asm/arch/power.h>
//#include <asm/arch/gpio.h>
//#include <linux/compat.h>
//#include <linux/usb/ch9.h>
#include "phy-samsung-usb-cal.h"
#include "phy-exynos-usb3p1.h"
#include "usb.h"

#define DEBUG 0
#define DISABLE	0

#define USB_PHY_CONTROL_BASE	USBDEVICE3_PHYCTRL_CH0_BASE

/* Phy tuning parameter - define here till final tune param is fixed */
/* 2.0 USB Only PHY Info */
static struct exynos_usb_tune_param mach7885_20phy_tune[] = {
	{ .name = "tx_pre_emp", .value = 0x3, },
	{ .name = "tx_pre_emp_plus", .value = 0x0, },
	{ .name = "tx_vref", .value = 0xf, },
	{ .name = "rx_sqrx", .value = 0x7, },
	{ .name = "tx_rise", .value = 0x3, },
	{ .name = "compdis", .value = 0x7, },
	{ .name = "tx_hsxv", .value = 0x3, },
	{ .name = "tx_fsls", .value = 0x3, },
	{ .name = "tx_res", .value = 0x3, },
	{ .name = "utim_clk", .value = USBPHY_UTMI_PHYCLOCK, },
	{ .value = EXYNOS_USB_TUNE_LAST, },
};

static struct exynos_usbphy_info phyinfo_9810 = {
	.version = EXYNOS_USBCON_VER_03_0_0,
	.refclk = USBPHY_REFCLK_DIFF_26MHZ,
	.refsel = USBPHY_REFSEL_CLKCORE,
	.not_used_vbus_pad = true,
	.use_io_for_ovc = DISABLE,
	.regs_base = (void *) USB_PHY_CONTROL_BASE,
	.tune_param = mach7885_20phy_tune,
	.used_phy_port = 0,
	.hs_rewa = 1,
};

enum samsung_phy_set_option {
	SET_DPPULLUP_ENABLE,
	SET_DPPULLUP_DISABLE,
	SET_DPDM_PULLDOWN,
};

void exynos_usb_phy_tune(void)
{
	phy_exynos_usb_v3p1_tune(&phyinfo_9810);
}
#if 0
void exynos_usb_sub_phy_init(void)
{
	phy_exynos_usb_v3p1_pma_ready(&phyinfo_9810);

	phy_exynos_usbdp_enable(&g_oMach9810_combo_info);

	phy_exynos_usb_v3p1_pma_sw_rst_release(&phyinfo_9810);
}
#endif
void exynos_usb_phy_init(void)
{
	phy_exynos_usb_v3p1_enable(&phyinfo_9810);

	phy_exynos_usb_v3p1_pipe_ovrd(&phyinfo_9810);

	exynos_usb_phy_tune();
}

static void exynos_usbdrd_phy_set(int option)
{
	switch (option) {
	case SET_DPPULLUP_ENABLE:
		phy_exynos_usb_v3p1_enable_dp_pullup(&phyinfo_9810);
		break;
	case SET_DPPULLUP_DISABLE:
		phy_exynos_usb_v3p1_disable_dp_pullup(&phyinfo_9810);
		break;
	case SET_DPDM_PULLDOWN:
		phy_exynos_usb_v3p1_config_host_mode(&phyinfo_9810);
	default:
		break;
	}
}

void exynos_usb_phy_exit(void)
{
	phy_exynos_usb_v3p1_disable(&phyinfo_9810);

	//exynos_usb_phy_isol(0);
}

static void exynos_usbdrd_fill_hstune(struct exynos_usbphy_info *info)
{
	struct exynos_usbphy_hs_tune *hs_tune = info->hs_tune;

	/* for HS */
	hs_tune->tx_pre_emp_puls = 0x0;
	hs_tune->tx_res		 = 0x3;
	hs_tune->tx_rise	 = 0x3;
	hs_tune->tx_hsxv	 = 0x3;
	hs_tune->tx_fsls	 = 0x3;
	hs_tune->rx_sqrx	 = 0x7;
	hs_tune->otg		 = 0x4;
	hs_tune->enable_user_imp = false;
	hs_tune->user_imp_value = 0;
	hs_tune->utmi_clk	 = USBPHY_UTMI_PHYCLOCK;

	/* additional tuning for host */
	hs_tune->tx_vref	 = 0x7;
	hs_tune->tx_pre_emp	 = 0x3;
	hs_tune->compdis	 = 0x7;
}

static void exynos_usbdrd_fill_sstune(struct exynos_usbphy_info *info)
{
	struct exynos_usbphy_ss_tune *ss_tune = info->ss_tune;

	/* for SS */
	ss_tune->tx_boost_level	= 0x2;
	ss_tune->tx_swing_level	= 0x1;
	ss_tune->tx_swing_full	= 0x7F;
	ss_tune->tx_swing_low	= 0x7F;
	ss_tune->tx_deemphasis_mode	= 0x1;
	ss_tune->tx_deemphasis_3p5db	= 0x20;
	ss_tune->tx_deemphasis_6db	= 0x20;
	ss_tune->enable_ssc	= 0x1;
	ss_tune->ssc_range	= 0x0;
	ss_tune->los_bias	= 0x5;
	ss_tune->los_mask_val	= 0x104;
	ss_tune->enable_fixed_rxeq_mode	= 0x1;
	ss_tune->fix_rxeq_value	= 0x3;
}

static void exynos_usb_get_phy_info(struct exynos_usbphy_info *info)
{
	struct exynos_usbphy_info *phyinfo = info;

	phyinfo->regs_base = USB_PHY_CONTROL_BASE;

	phyinfo->version = EXYNOS_USBCON_VER_01_0_1;
	phyinfo->use_io_for_ovc = true;
	phyinfo->refclk = USBPHY_REFCLK_DIFF_26MHZ;
	phyinfo->not_used_vbus_pad = true;
	phyinfo->refsel = USBPHY_REFSEL_DIFF_INTERNAL;

	exynos_usbdrd_fill_hstune(&phyinfo_9810);
	exynos_usbdrd_fill_sstune(&phyinfo_9810);
}

