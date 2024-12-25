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

#include <debug.h>
#include <string.h>
#include <reg.h>
#include <malloc.h>
#include <platform/delay.h>
#include "phy-samsung-usb-cal.h"
#include "phy-exynos-usb3p1.h"

#define BIT(x) (1 << (x))

/* Exynos5: USB 3.0 DRD PHY registers */
#define EXYNOS5_DRD_LINKSYSTEM			0x04
#define EXYNOS5_DRD_PHYUTMI			0x08
#define EXYNOS5_DRD_PHYCLKRST			0x10
#define EXYNOS5_DRD_PHYTEST			0x28
#define EXYNOS5_DRD_PHYRESUME			0x34
#define EXYNOS5_DRD_LINKPORT			0x44
#define EXYNOS8895_DRD_PHYSELECTION		0x6c

#define PHYTEST_POWERDOWN_SSP			BIT(3)
#define PHYTEST_POWERDOWN_HSP			BIT(2)

#define PHYCLKRST_EN_UTMISUSPEND		BIT(31)
#define PHYCLKRST_PORTRESET			BIT(1)
#define PHYCLKRST_COMMONONN			BIT(0)

#define PHYSEL_UTMI_CLK				BIT(4)
#define PHYSEL_PIPE_CLK				BIT(3)
#define PHYSEL_UTMI				BIT(2)
#define PHYSEL_PIPE				BIT(1)
#define PHYSEL_SIDEBAND				BIT(0)

#define PHYRESUME_FORCE_QACT			BIT(9)

#define LINKSYSTEM_FORCE_VBUSVALID		BIT(8)
#define LINKSYSTEM_FORCE_BVALID			BIT(7)

#define PHYUTMI_VBUSVLDEXTSEL			BIT(10)
#define PHYUTMI_VBUSVLDEXT			BIT(9)
#define PHYUTMI_OTGDISABLE			BIT(6)
#define PHYUTMI_DRVVBUS				BIT(4)
#define PHYUTMI_FORCESUSPEND			BIT(1)
#define PHYUTMI_FORCESLEEP			BIT(0)

#define UTMI_DP_PULLDOWN			BIT(3)
#define UTMI_DM_PULLDOWN			BIT(2)

void phy_exynos_usb_v3p1_enable(struct exynos_usbphy_info *info)
{
	void __iomem *regs_base = info->regs_base;
	u32 reg;
	u32 reg_hsp;
	bool ss_only_cap;
	int main_version;

	reg = readl(info->regs_base + EXYNOS5_DRD_PHYRESUME);
	reg |= PHYRESUME_FORCE_QACT;
	writel(reg, info->regs_base + EXYNOS5_DRD_PHYRESUME);

	/* Select the PHY MUX */
	reg = readl(info->regs_base + EXYNOS8895_DRD_PHYSELECTION);
	reg &= ~(PHYSEL_PIPE | PHYSEL_PIPE_CLK | PHYSEL_UTMI_CLK |
		 PHYSEL_UTMI | PHYSEL_SIDEBAND);
	writel(reg, info->regs_base + EXYNOS8895_DRD_PHYSELECTION);

	reg = readl(info->regs_base + EXYNOS5_DRD_PHYCLKRST);

	/* Assert a port reset */
	reg |= PHYCLKRST_PORTRESET;

	/* Select Reference clock source path */
	reg &= ~(0x03 << 2);
	reg |= (0x2 << 2);

	/* Select ref clk */
	writel(reg, info->regs_base + EXYNOS5_DRD_PHYCLKRST);

	reg = readl(info->regs_base + EXYNOS5_DRD_PHYCLKRST);
	reg |= PHYCLKRST_COMMONONN | PHYCLKRST_EN_UTMISUSPEND;
	writel(reg, info->regs_base + EXYNOS5_DRD_PHYCLKRST);

	reg = readl(info->regs_base + EXYNOS5_DRD_PHYCLKRST);
	/* Dessert the port reset */
	reg &= ~PHYCLKRST_PORTRESET;
	writel(reg, info->regs_base + EXYNOS5_DRD_PHYCLKRST);

	reg = readl(info->regs_base + EXYNOS5_DRD_PHYTEST);
	reg &= ~(PHYTEST_POWERDOWN_HSP | PHYTEST_POWERDOWN_SSP);
	writel(reg, info->regs_base + EXYNOS5_DRD_PHYTEST);

	reg = readl(info->regs_base + EXYNOS5_DRD_PHYUTMI);
	/* Enable UTMI+ */
	reg &= ~(PHYUTMI_FORCESUSPEND, PHYUTMI_FORCESLEEP, UTMI_DP_PULLDOWN, UTMI_DM_PULLDOWN);
	writel(reg, info->regs_base + EXYNOS5_DRD_PHYUTMI);

	reg = readl(info->regs_base + EXYNOS5_DRD_LINKSYSTEM);
	reg |= (LINKSYSTEM_FORCE_BVALID | LINKSYSTEM_FORCE_VBUSVALID);
	writel(reg, info->regs_base + EXYNOS5_DRD_LINKSYSTEM);

	reg = readl(info->regs_base + EXYNOS5_DRD_PHYUTMI);
	reg |= (PHYUTMI_VBUSVLDEXTSEL | PHYUTMI_VBUSVLDEXT | PHYUTMI_OTGDISABLE);
	reg &= ~PHYUTMI_DRVVBUS;
	writel(reg, info->regs_base + EXYNOS5_DRD_PHYUTMI);
}
