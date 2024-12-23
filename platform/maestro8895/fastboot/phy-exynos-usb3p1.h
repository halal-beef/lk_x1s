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

#ifndef DRIVER_USB_USBPHY_CAL_PHY_EXYNOS_USB3P1_H_
#define DRIVER_USB_USBPHY_CAL_PHY_EXYNOS_USB3P1_H_

#define HS_REWA_INTR_SRC_RET_EN		(1 << 0)
#define HS_REWA_INTR_SRC_RET_DIS	(1 << 1)
#define HS_REWA_INTR_SRC_BYPASS_DIS	(1 << 2)
#define HS_REWA_INTR_SRC_DISCON		(1 << 16)
#define HS_REWA_INTR_SRC_ERR_DEV_K	(1 << 17)
#define HS_REWA_INTR_SRC_ERR_SUS	(1 << 18)

#define HS_REWA_EN_STS_ENALBED		0
#define HS_REWA_EN_STS_DISABLED		1
#define HS_REWA_EN_STS_DISCONNECT	2
#define HS_REWA_EN_STS_NOT_SUSPEND	-1


/* initialted */
extern void phy_exynos_usb_v3p1_enable(struct exynos_usbphy_info *info);
extern void phy_exynos_usb_v3p1_disable(struct exynos_usbphy_info *info);
extern void phy_exynos_usb3p1_sw_rst(struct exynos_usbphy_info *info);
/* USB/DP PHY control */
extern void phy_exynos_usb_v3p1_pma_ready(struct exynos_usbphy_info *info);
extern void phy_exynos_usb_v3p1_pma_sw_rst_release(struct exynos_usbphy_info *info);
extern void phy_exynos_usb_v3p1_pipe_ovrd(struct exynos_usbphy_info *info);
extern void phy_exynos_usb_v3p1_pipe_ready(struct exynos_usbphy_info *info);
/* Tune */
extern void phy_exynos_usb_v3p1_config_host_mode(struct exynos_usbphy_info *info);
extern void phy_exynos_usb_v3p1_enable_dp_pullup(struct exynos_usbphy_info *info);
extern void phy_exynos_usb_v3p1_disable_dp_pullup(struct exynos_usbphy_info *info);
extern void phy_exynos_usb_v3p1_tune_host(struct exynos_usbphy_info *info);
extern void phy_exynos_usb_v3p1_tune_dev(struct exynos_usbphy_info *info);
extern void phy_exynos_usb_v3p1_tune(struct exynos_usbphy_info *info);
extern void phy_exynos_usb_v3p1_tune_each(struct exynos_usbphy_info *info, char *para_name, int val);
extern void phy_exynos_usb_v3p1_wr_tune_reg(struct exynos_usbphy_info *info, u32 val);
extern void phy_exynos_usb_v3p1_rd_tune_reg(struct exynos_usbphy_info *info, u32 *val);
/* High Speed Remote Wake-up Advisor(HS ReWA) */
extern int phy_exynos_usb3p1_rewa_enable(struct exynos_usbphy_info *info);
extern int phy_exynos_usb3p1_rewa_disable(struct exynos_usbphy_info *info);
extern int phy_exynos_usb3p1_rewa_req_sys_valid(struct exynos_usbphy_info *info);
extern int phy_exynos_usb3p1_rewa_cancel(struct exynos_usbphy_info *info);

#endif /* DRIVER_USB_USBPHY_CAL_PHY_EXYNOS_USB3P1_H_ */
