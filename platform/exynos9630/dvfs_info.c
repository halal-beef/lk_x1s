/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
#include <lk/debug.h>
#include <lk/reg.h>
#include <sys/types.h>
#include <platform/delay.h>
#include <platform/exynos9630.h>
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
	for (i = 0; i < ID_INFO_ADDR_CNT; i++)
		*(p_table + i) = (unsigned int)regs[i];

	printf("\n### display_asv_info:\n");

	printf("asv_table_version : %d\n", asv_tbl.asv_table_version);
	printf("  big cpu grp : %d, ids : %d\n",
			asv_tbl.bigcpu_asv_group + asv_tbl.bigcpu_modify_group,
			id_tbl.ids_bigcpu);
	printf("  little cpu grp : %d, ids : %d\n",
			asv_tbl.littlecpu_asv_group + asv_tbl.littlecpu_modify_group,
			id_tbl.ids_litcpu);
	printf("  g3d grp : %d, ids : %d\n",
			asv_tbl.g3d_asv_group + asv_tbl.g3d_modify_group,
			id_tbl.ids_g3d);
	printf("  mif grp : %d, ids : %d\n",
			asv_tbl.mif_asv_group + asv_tbl.mif_modify_group,
			id_tbl.ids_mif);
	printf("  int grp : %d, ids : %d\n",
			asv_tbl.int_asv_group + asv_tbl.int_modify_group,
			id_tbl.ids_others);
	printf("  cam grp : %d, ids : %d\n",
			asv_tbl.cam_asv_group + asv_tbl.cam_modify_group,
			id_tbl.ids_others);
	printf("  cp grp : %d, ids : %d\n",
			asv_tbl.cp_asv_group + asv_tbl.cp_modify_group,
			id_tbl.ids_cp);
	printf("  npu grp : %d, ids : %d\n",
			asv_tbl.npu_asv_group + asv_tbl.npu_modify_group,
			id_tbl.ids_npu);
	printf("  product_line : %d\n", id_tbl.product_line);
	printf("  asb_version : %d\n", id_tbl.asb_version);
}

void display_asv_g_info(void)
{
	int i, j;
	struct fvmap_header *fvmap = ((struct fvmap_header*)(ACPM_FVMAP_BASE));
	struct rate_volt_header *header;
	const char *domain_name[11] = { "", "", "LIT", "BIG", "", "", "", "", "", "", "G3D" };

	printf("\n### display_asv_g_info:\n");

	for (i = 0; i < NUM_DOMAINS; i++) {
		switch (fvmap[i].dvfs_type) {
			case ASV_G_LIT:
			case ASV_G_BIG:
			case ASV_G_G3D:
				printf("dvfs type : %s\n", domain_name[fvmap[i].dvfs_type]);
				header = (void *)ACPM_FVMAP_BASE + fvmap[i].o_ratevolt;
				for (j = 0; j < fvmap[i].num_of_lv; j++) {
					if (j % 10 == 0 && j > 0) printf("\n");
					printf("%8d ", header->table[j].volt);
				}
				printf("\n");
				/* Do Something */
				break;
			default:
				break;
		}
	}
}

void display_last_freq_volt(void)
{
	unsigned int i;
	unsigned int rgt_val = 0, rgt_volt = 0, freq = 0;

	struct last_freq_volt last_fv[MAX_VOLT_DOMAIN] = {
		{"BIG", LAST_FREQ_BASE + 0x0, LAST_VOLT_BASE + 0x1d},	//buck2m
		{"LIT", LAST_FREQ_BASE + 0x4, LAST_VOLT_BASE + 0x20},	//buck3m
		{"G3D", LAST_FREQ_BASE + 0x8, LAST_VOLT_BASE + 0x112},	//buck2s
		{"MIF", LAST_FREQ_BASE + 0xc, LAST_VOLT_BASE + 0x1a},	//buck1m
		{"INT", LAST_FREQ_BASE + 0x10, LAST_VOLT_BASE + 0x23},	//buck4m
		{"CAM", LAST_FREQ_BASE + 0x14, LAST_VOLT_BASE + 0x110},	//buck1s
		{"CP", LAST_FREQ_BASE + 0x18, LAST_VOLT_BASE + 0x118},	//buck5s
		{"NPU", LAST_FREQ_BASE + 0x1c, LAST_VOLT_BASE + 0x25},	//buck5m
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
	rst_stat = readl(EXYNOS9630_POWER_RST_STAT);

	printf("========================================\n");

	display_asv_info();
	display_asv_g_info();
	display_last_freq_volt();
	printf("\n========================================\n");
}
