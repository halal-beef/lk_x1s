/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __DVFS_INFO_H__
#define __DVFS_INFO_H__

#define ACPM_SRAM_BASE		0x02039000
#define ACPM_DVFS_OFFSET	0x00020C00
#define ACPM_FVMAP_OFFSET	0x0000CC00
#define ACPM_FVMAP_BASE		ACPM_SRAM_BASE + ACPM_DVFS_OFFSET + ACPM_FVMAP_OFFSET
#define NUM_DOMAINS			16
#define ASV_G_LIT			0x2
#define ASV_G_BIG			0x3
#define ASV_G_G3D			0xa
#define VOLT_STEP			6250

struct fvmap_header {
	unsigned char dvfs_type;
	unsigned char num_of_lv;
	unsigned char num_of_members;
	unsigned char num_of_pll;
	unsigned char num_of_mux;
	unsigned char num_of_div;
	unsigned short gearratio;
	unsigned char init_lv;
	unsigned char num_of_gate;
	unsigned char reserved[2];
	unsigned short block_addr[3];
	unsigned short o_members;
	unsigned short o_ratevolt;
	unsigned short o_tables;
};

struct rate_volt {
	unsigned int rate;
	unsigned int volt;
};

struct rate_volt_header {
	struct rate_volt table[0];
};

void display_asv_g_info(void);
void display_dvfs_info(void);

#define ASV_TABLE_BASE	(0x10009000)
#define ID_TABLE_BASE	(0x10000000)

struct asv_tbl_info {
	unsigned bigcpu_asv_group:4;		// 0x10009000
	unsigned middlecpu_asv_group:4;
	unsigned littlecpu_asv_group:4;
	unsigned g3d_asv_group:4;
	unsigned mif_asv_group:4;
	unsigned int_asv_group:4;
	unsigned cam_asv_group:4;
	unsigned cp_asv_group:4;

	unsigned npu_asv_group:4;			// 0x10009004
	unsigned modem_asv_group:4;
	unsigned reserved_0:4;
	unsigned reserved_1:4;
	unsigned reserved_2:4;
	unsigned reserved_3:4;
	unsigned npu_modify_group:4;
	unsigned modem_modify_group:4;

	unsigned bigcpu_modify_group:4;		// 0x10009008
	unsigned middlecpu_modify_group:4;
	unsigned littlecpu_modify_group:4;
	unsigned g3d_modify_group:4;
	unsigned mif_modify_group:4;
	unsigned int_modify_group:4;
	unsigned cam_modify_group:4;
	unsigned cp_modify_group:4;

	unsigned asv_table_version:7;		// 0x1000900C
	unsigned asv_group_type:1;
};

struct id_tbl_info {
	unsigned reserved_0;
	unsigned chip_id;
	unsigned reserved_2:10;

	unsigned product_line:2;
	unsigned reserved_2_1:4;
	unsigned char ids_bigcpu:8;
	unsigned char ids_g3d:8;
	unsigned char ids_others:8; /* int, cam, disp, intcam, mfc, aud */
	unsigned asb_version:8;
	unsigned ids_npu:8;
	unsigned ids_litcpu:8;
	unsigned ids_mif:8;
	unsigned ids_cp:8;
	unsigned short sub_rev:4;
	unsigned short main_rev:4;
	unsigned reserved_7:32;
};

#define PORESET					(0x80000000)

#define ASV_INFO_ADDR_CNT	(sizeof(struct asv_tbl_info) / 4)
#define ID_INFO_ADDR_CNT	(sizeof(struct id_tbl_info) / 4)

#define MAX_VOLT_DOMAIN		(8)
#define LAST_VOLT_BASE		(0x90018600)
#define RGT_MIN_VAL		(0x18)
#define RGT_MIN_VOLT		(450000)
#define RGT_STEP_SIZE		(6250)

#define LAST_FREQ_BASE		(0x9001EB00)

struct last_freq_volt {
	char domain_name[8];
	unsigned int freq_offset;
	unsigned int volt_offset;
};

#endif /* __DVFS_INFO_H__ */
