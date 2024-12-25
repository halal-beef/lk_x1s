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
	.version = EXYNOS_USBCON_VER_01_1_1,
	.refclk = USBPHY_REFCLK_DIFF_26MHZ,
	.refsel = USBPHY_REFSEL_CLKCORE,
	.not_used_vbus_pad = true,
	.use_io_for_ovc = DISABLE,
	.regs_base = (void *) USB_PHY_CONTROL_BASE,
	.tune_param = mach7885_20phy_tune,
	.used_phy_port = 0,
	.hs_rewa = 1,
};

void exynos_usb_phy_init(void)
{
	phy_exynos_usb_v3p1_enable(&phyinfo_9810);
}

void exynos_usb_phy_exit(void)
{
	phy_exynos_usb_v3p1_disable(&phyinfo_9810);
}
