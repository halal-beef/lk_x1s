/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 *
 *
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
#include "phy-exynos-usbdp-gen2.h"
#include "usb.h"

#define DEBUG			0
#define DISABLE			0

#define CONFIG_USB_SSPHY_SAMSUNG_V2
#define USB_PHY_CONTROL_BASE	USBDEVICE3_PHYCTRL_CH0_BASE
#if defined(CONFIG_USB_SSPHY_SAMSUNG_V2)
#define USBDP_PHY_BASE		USBDEVICE3_PHYCTRL_DP_BASE
#define USBPCS_BASE		USBDEVICE3_PHYCTRL_PCS_BASE
#endif

/*
 * Phy tuning parameter - define here till final tune param is fixed
 * 2.0 USB Only PHY Info*/
static struct exynos_usb_tune_param usbcal_20phy_tune[] = {
	{ .name = "tx_pre_emp", .value = 0x3, },
	{ .name = "tx_vref", .value = 0x7, },
	{ .name = "rx_sqrx", .value = 0x5, },
	{ .name = "utim_clk", .value = USBPHY_UTMI_PHYCLOCK, },
	{ .value = EXYNOS_USB_TUNE_LAST, },
};

static struct exynos_usbphy_info usbphy_cal_info = {
	.version = EXYNOS_USBCON_VER_03_0_1,
	.refclk = USBPHY_REFCLK_DIFF_26MHZ,
	.refsel = USBPHY_REFSEL_CLKCORE,
	.not_used_vbus_pad = true,
	.use_io_for_ovc = DISABLE,
	.regs_base = (void *)USB_PHY_CONTROL_BASE,
	.tune_param = usbcal_20phy_tune,
	.used_phy_port = 0,
	.hs_rewa = 1,
};

/* 3.0 USB/DP PHY */
#if defined(CONFIG_USB_SSPHY_SAMSUNG) || defined(CONFIG_USB_SSPHY_SAMSUNG_V2)
static struct exynos_usb_tune_param usbcal_ssphyy_tune[] = {
#if defined(CONFIG_USB_SSPHY_SAMSUNG)
	{ .name = "sstx_deemph", .value = 0x6, },
	{ .name = "sstx_amp",  .value = 0x0, },
	{ .name = "ssrx_los",  .value = 0x4, },
	{ .name = "ssrx_ctle_peak", .value = 0x2, },
	{ .name = "ssrx_eq_code", .value = 0x6, },
	{ .name = "ssrx_cur_ctrl", .value = 0x0, },
	{ .name = "ssrx_eqen",  .value = 0x3, },
	{ .name = "ssrx_sqth",  .value = 0x5, },
#elif defined(CONFIG_USB_SSPHY_SAMSUNG_V2)
	{ .name = "ssrx_sqhs_th",		.value = 0x4, },
	{ .name = "ssrx_lfps_th",		.value = 0x2, },
	{ .name = "ssrx_mf_eq_en",		.value = 0x1, },
	{ .name = "ssrx_mf_eq_ctrl_ss",		.value = 0x6, },
	{ .name = "ssrx_hf_eq_ctrl_ss",		.value = 0xe, },
	{ .name = "ssrx_mf_eq_ctrl_ssp",	.value = 0xc, },
	{ .name = "ssrx_hf_eq_ctrl_ssp",	.value = 0xc, },
	{ .name = "ssrx_dfe1_tap_ctrl",		.value = 0x4, },
	{ .name = "ssrx_dfe2_tap_ctrl",		.value = 0x0, },
	{ .name = "ssrx_dfe3_tap_ctrl",		.value = 0x0, },
	{ .name = "ssrx_dfe4_tap_ctrl",		.value = 0x0, },
	{ .name = "ssrx_dfe5_tap_ctrl",		.value = 0x0, },
	{ .name = "ssrx_term_cal",		.value = 0x5, },
	{ .name = "sstx_amp",			.value = 0xb, },
	{ .name = "sstx_deemp",			.value = 0x5, },
	{ .name = "sstx_pre_shoot",		.value = 0x3, },
	{ .name = "sstx_idrv_up",		.value = 0x7, },
	{ .name = "sstx_idrv_dn",		.value = 0x0, },
	{ .name = "sstx_up_term",		.value = 0x3, },
	{ .name = "sstx_dn_term",		.value = 0x3, },
#endif /* if defined(CONFIG_USB_SSPHY_SAMSUNG) */
	{ .value = EXYNOS_USB_TUNE_LAST, },
};

static struct exynos_usbphy_info usbphy_cal_ssphy_info = {
#if defined(CONFIG_USB_SSPHY_SAMSUNG)
	.version = EXYNOS_USBCON_VER_04_0_0,
	.refsel = USBPHY_REFSEL_CLKCORE,
	.regs_base = (void *)USBDP_PHY_BASE,
	.regs_base_2nd = (void *)USBPCS_BASE,
#else
	.version = EXYNOS_USBCON_VER_04_1_0,
	.refsel = USBPHY_REFSEL_CLKCORE,
	.regs_base = (void *)USBDEVICE3_PHYCTRL_DP_BASE,
	.regs_base_2nd = (void *)USBDEVICE3_PHYCTRL_PCS_BASE,
#endif
	.used_phy_port = 0,
	.tune_param = usbcal_ssphyy_tune,
};
#endif /* if defined(CONFIG_USB_SSPHY_SAMSUNG) || defined(CONFIG_USB_SSPHY_SAMSUNG_V2) */

enum samsung_phy_set_option {
	SET_DPPULLUP_ENABLE,
	SET_DPPULLUP_DISABLE,
	SET_DPDM_PULLDOWN,
};

void exynos_usb_phy_tune(void)
{
	phy_exynos_usb_v3p1_tune(&usbphy_cal_info);
}

void exynos_usb_sub_phy_init(void)
{
#if defined(CONFIG_USB_SSPHY_SAMSUNG)
	phy_exynos_usb_v3p1_pma_ready(&usbphy_cal_info);

	phy_exynos_usbdp_enable(&usbphy_cal_ssphy_info);

	phy_exynos_usb_v3p1_pma_sw_rst_release(&usbphy_cal_info);
#elif defined(CONFIG_USB_SSPHY_SAMSUNG_V2)
	int cnt;

	phy_exynos_usb_v3p1_g2_pma_ready(&usbphy_cal_info);
	phy_exynos_usbdp_g2_enable(&usbphy_cal_ssphy_info);
	phy_exynos_usb_v3p1_g2_pma_sw_rst_release(&usbphy_cal_info);
	for (cnt = 100; cnt != 0; cnt--) {
		int pll_lock;

		pll_lock = phy_exynos_usbdp_g2_check_pll_lock(&usbphy_cal_ssphy_info);
		if (!pll_lock)
			break;
	}
	if (!cnt) {
		printf("pll lock is fail!!\n");
		return;
	}
	/* [step5] run RX / TX lane */
	phy_exynos_usbdp_g2_run_lane(&usbphy_cal_ssphy_info);
	printf("wait cdr lock donw\n");
	/* [step6] wait for RX CDR lock done */
	for (cnt = 100; cnt != 0; cnt--) {
		int pll_lock;

		pll_lock = phy_exynos_usbdp_g2_rx_cdr_pll_lock(
			&usbphy_cal_ssphy_info);
		if (!pll_lock)
			break;
	}
	if (!cnt) {
		printf("pll lock is fail!!\n");
		return;
	}
	/* [step7] link_pclk_sel change */
	phy_exynos_usb_v3p1_g2_link_pclk_sel(&usbphy_cal_info);
#endif /* if defined(CONFIG_USB_SSPHY_SAMSUNG) */
}

void exynos_usb_phy_init(void)
{
	phy_exynos_usb_v3p1_enable(&usbphy_cal_info);

	/* phy_exynos_usb_v3p1_pipe_ovrd(&usbphy_cal_info); */

	exynos_usb_phy_tune();
}

static void exynos_usbdrd_phy_set(int option)
{
	switch (option) {
	case SET_DPPULLUP_ENABLE:
		phy_exynos_usb_v3p1_enable_dp_pullup(&usbphy_cal_info);
		break;
	case SET_DPPULLUP_DISABLE:
		phy_exynos_usb_v3p1_disable_dp_pullup(&usbphy_cal_info);
		break;
	case SET_DPDM_PULLDOWN:
		phy_exynos_usb_v3p1_config_host_mode(&usbphy_cal_info);
		break;
	default:
		break;
	}
}

void exynos_usb_phy_exit(void)
{
	phy_exynos_usb_v3p1_disable(&usbphy_cal_info);

	//exynos_usb_phy_isol(0);
}

void exynos_usb_set_ss_port(int port_num)
{
	usbphy_cal_info.used_phy_port = port_num;
#if defined(CONFIG_USB_SSPHY_SAMSUNG) || defined(CONFIG_USB_SSPHY_SAMSUNG_V2)
	usbphy_cal_ssphy_info.used_phy_port = port_num;
#endif
}
