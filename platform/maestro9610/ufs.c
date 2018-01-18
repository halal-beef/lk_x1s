/*
 * Copyright (C) 2017 Samsung Electronics
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
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
