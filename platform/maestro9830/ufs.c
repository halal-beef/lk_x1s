/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <reg.h>
#include <dev/ufs.h>

struct ufs_host;

#define UFS_SCLK       166000000
#define CNT_VAL_1US_MASK       0x3ff
#define UFSHCI_VS_1US_TO_CNT_VAL                       0x110C
#define UFSHCI_VS_UFSHCI_V2P1_CTRL                     0X118C
#define BIT(x) (1<<(x))
#define IA_TICK_SEL                            BIT(16)

#define MUX_CLKCMU_UFS_EMBD_CON 0x1A331098
#define DIV_CLKCMU_UFS_EMBD_MUX 0x1A331890

#define UFS_CLKCMU_TIMEOUT 100

void ufs_vs_set_1us_to_cnt(struct ufs_host *ufs)
{
	u32 nVal;
	u32 cnt_val;

	/* IA_TICK_SEL : 1(1us_TO_CNT_VAL) */
	nVal = readl(ufs->ioaddr + UFSHCI_VS_UFSHCI_V2P1_CTRL);
	nVal |= IA_TICK_SEL;
	writel(nVal, ufs->ioaddr + UFSHCI_VS_UFSHCI_V2P1_CTRL);

	cnt_val = UFS_SCLK / 1000000;
	writel(cnt_val & CNT_VAL_1US_MASK, ufs->ioaddr + UFSHCI_VS_1US_TO_CNT_VAL);
}

void ufs_set_unipro_clk(struct ufs_host *ufs)
{
	int timeout = 0;

	writel(3, DIV_CLKCMU_UFS_EMBD_MUX);

	do {
		timeout += 1;
	} while ((readl(DIV_CLKCMU_UFS_EMBD_MUX) & 0x10000) && timeout < UFS_CLKCMU_TIMEOUT);
	if (timeout == UFS_CLKCMU_TIMEOUT)
		printf("ERROR(UFS): DIV_CLKCMU_UFS_EMBD_MUX setting timeout occured\n");

	timeout = 0;

	writel(1, MUX_CLKCMU_UFS_EMBD_CON);

	do {
		timeout += 1;
	} while ((readl(MUX_CLKCMU_UFS_EMBD_CON) & 0x10000) && timeout < UFS_CLKCMU_TIMEOUT);
	if (timeout == UFS_CLKCMU_TIMEOUT)
		printf("ERROR(UFS): MUX_CLKCMU_UFS_EMBD_CON setting timeout occured\n");
	ufs_vs_set_1us_to_cnt(ufs);
}

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
	ufs->irq = 249;
	ufs->ioaddr = (void __iomem *)0x13100000;
	ufs->vs_addr = (void __iomem *)(0x13100000 + 0x1100);
	ufs->fmp_addr = (void __iomem *)0x132A0000;
	ufs->unipro_addr = (void __iomem *)0x13180000;
	ufs->phy_pma = (void __iomem *)(0x13100000 + 0x4000);

	/* power source changed compared with before */
	/* XBOOTLDO GPG1[0] */
	ufs->dev_pwr_addr = (void __iomem *)(0x10730000 + 0xc0);
	ufs->dev_pwr_shift = 0;

	ufs->phy_iso_addr = (void __iomem *)(0x15860000 + 0x724);

	ufs->host_index = host_index;

	ufs->mclk_rate = 166 * (1000 * 1000);
	ufs_set_unipro_clk(ufs);

	// TODO:
	//set_ufs_clk(host_index);
	//err = exynos_pinmux_config(PERIPH_ID_UFS0, PINMUX_FLAG_NONE);

	/* GPIO configurations, rst_n and refclk */
	reg = *(volatile u32 *)0x13040048;
	reg &= ~(0xFF);
	*(volatile u32 *)0x13040048 = reg;

	reg = *(volatile u32 *)0x13040040;
	reg &= ~(0xFF);
	reg |= 0x22;
	*(volatile u32 *)0x13040040 = reg;

	return 0;
}

void ufs_pre_vendor_setup(struct ufs_host *ufs)
{

}
