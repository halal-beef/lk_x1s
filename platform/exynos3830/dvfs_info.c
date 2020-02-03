/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
#include <debug.h>
#include <reg.h>
#include <sys/types.h>
#include <platform/delay.h>
#include <platform/exynos3830.h>
#include <platform/dvfs_info.h>
#include <platform/smc.h>

static struct asv_tbl_info asv_tbl;
static struct id_tbl_info id_tbl;
static unsigned int rst_stat;

void display_asv_info(void)
{
	unsigned int i;
	unsigned int *p_table;
	unsigned int *regs;
	u64 tmp;

	p_table = (unsigned int *)&asv_tbl;

	for (i = 0; i < ASV_INFO_ADDR_CNT; i++) {
		tmp = exynos_smc_read(SMC_CMD_REG, SMC_REG_ID_SFR_R(ASV_TABLE_BASE + 0x4 * i));
		*(p_table + i) = (unsigned int)tmp;
	}

	p_table = (unsigned int *)&id_tbl;

	regs = (unsigned int *)ID_TABLE_BASE;
	for (i = 0; i < ID_INFO_ADDR_CNT; i++) {
		*(p_table + i) = (unsigned int)regs[i];
	}

	printf("\n### display_asv_info:\n");
	printf("asv_table_version : %d\n", asv_tbl.asv_table_version);
	printf("  littlecpu grp : %d, id = %d\n", asv_tbl.littlecpu_asv_group, id_tbl.ids_others);
	printf("  g3d grp : %d, id = %d\n", asv_tbl.g3d_asv_group, id_tbl.ids_g3d);
	printf("  mif grp : %d, id = %d\n", asv_tbl.mif_asv_group, id_tbl.ids_others);
	printf("  int grp : %d, id = %d\n", asv_tbl.int_asv_group, id_tbl.ids_others);
	printf("  cam_disp grp : %d, id = %d\n", asv_tbl.cam_disp_asv_group, id_tbl.ids_others);
	printf("  cp grp : %d, id = %d\n", asv_tbl.cp_asv_group, id_tbl.ids_cp);
	printf("  asb version : %d\n", id_tbl.asb_version);
	printf("  product line : %d\n", id_tbl.product_line);
}

void display_last_freq_volt(void)
{
	unsigned int i;
	unsigned int rgt_val = 0, rgt_volt = 0, freq = 0;

	struct last_freq_volt last_fv[MAX_VOLT_DOMAIN] = {
		{"BIG", LAST_FREQ_BASE + 0x0, LAST_VOLT_BASE + 0x19},	//buck1
		{"LIT", LAST_FREQ_BASE + 0x4, LAST_VOLT_BASE + 0x19},	//buck1
		{"MIF", LAST_FREQ_BASE + 0x8, LAST_VOLT_BASE + 0x1c},	//buck2
		{"INT", LAST_FREQ_BASE + 0xc, LAST_VOLT_BASE + 0x1c},	//buck2
		{"G3D", LAST_FREQ_BASE + 0x10, LAST_VOLT_BASE + 0x1c},	//buck2
		{"AUD", LAST_FREQ_BASE + 0x14, LAST_VOLT_BASE + 0x1c},	//buck2
		{"DISP", LAST_FREQ_BASE + 0x18, LAST_VOLT_BASE + 0x1c},	//buck2
	};

	if (rst_stat == (PORESET | PIN_RESET))
		return ;

	printf("\n### display_last_freq_volt:\n");

	for (i = 0; i < MAX_VOLT_DOMAIN; i++) {
		freq = readl(last_fv[i].freq_offset);
		rgt_val = readb(last_fv[i].volt_offset);
		rgt_volt = RGT_MIN_VOLT + (rgt_val - RGT_MIN_VAL) * RGT_STEP_SIZE;

		printf("[%s] freq: %d [Mhz], volt: %d [uV]\n",
				last_fv[i].domain_name, freq, rgt_volt);
	}

}

void display_dvfs_info(void)
{
	rst_stat = readl(EXYNOS3830_POWER_RST_STAT);

	printf("========================================\n");

	display_asv_info();
	display_last_freq_volt();
	printf("\n========================================\n");
}
