/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <dev/ufs.h>

struct ufs_host;

int ufs_board_init(int host_index, struct ufs_host *ufs)
{
	u32 reg;
	//u32 err;

	if (host_index) {
		printf("Currently multi UFS host is not supported!\n");
		return -1;
	}

	/* mmio */
	sprintf(ufs->host_name,"ufs%d", host_index);
	ufs->irq = 157;
	ufs->ioaddr = (void __iomem *)0x13520000;
	ufs->vs_addr = (void __iomem *)(0x13520000 + 0x1100);
	ufs->fmp_addr = (void __iomem *)0x13530000;
	ufs->unipro_addr = (void __iomem *)(0x13520000 - 0x10000);
	ufs->phy_pma = (void __iomem *)(0x13520000 + 0x4000);

	/* power source changed compared with before */
	ufs->dev_pwr_addr = (void __iomem *)(0x139B0000 + 0x144);
	ufs->dev_pwr_shift = 0;

	ufs->phy_iso_addr = (void __iomem *)(0x11860000 + 0x724);

	ufs->host_index = host_index;

	ufs->mclk_rate = 166 * (1000 * 1000);

	// TODO:
	//set_ufs_clk(host_index);
	//err = exynos_pinmux_config(PERIPH_ID_UFS0, PINMUX_FLAG_NONE);

	/* GPIO configurations, rst_n and refclk */
	reg = *(volatile u32 *)0x13490008;
	reg &= ~(0xFF);
	*(volatile u32 *)0x13490008 = reg;

	reg = *(volatile u32 *)0x13490000;
	reg &= ~(0xFF);
	reg |= 0x33;
	*(volatile u32 *)0x13490000 = reg;

	return 0;
}

void ufs_pre_vendor_setup(struct ufs_host *ufs)
{

}
