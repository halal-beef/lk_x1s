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

#ifndef PHY_EXYNOS_USBDP_H_
#define PHY_EXYNOS_USBDP_H_


extern void phy_exynos_usbdp_enable(struct exynos_usbphy_info *);
extern void phy_exynos_usbdp_ilbk(struct exynos_usbphy_info *info);
extern void phy_exynos_usbdp_pcs_reset(struct exynos_usbphy_info *info);
extern int phy_exynos_usbdp_check_pll_lock(struct exynos_usbphy_info *info);
extern void phy_exynos_usbdp_disable(struct exynos_usbphy_info *);
extern void phy_exynos_usbdp_tune_each(struct exynos_usbphy_info *, char *, int);
extern void phy_exynos_usbdp_tune(struct exynos_usbphy_info *info);
extern void exynos_usbdrd_request_phy_isol(void);
extern int exynos_usbdrd_inform_dp_use(int use, int lane_cnt);

#endif /* PHY_EXYNOS_USBDP_H_ */
