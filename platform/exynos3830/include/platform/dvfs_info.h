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

void display_asv_g_info(void);
void display_dvfs_info(void);

#define ASV_TABLE_BASE	(0x10009000)
#define ID_TABLE_BASE	(0x10000000)

struct asv_tbl_info {
	unsigned reserved_0:4;
	int      reserved_1:4;
	unsigned littlecpu_asv_group:4;
	unsigned g3d_asv_group:4;
	unsigned mif_asv_group:4;
	unsigned int_asv_group:4;
	unsigned cam_disp_asv_group:4;
	unsigned cp_asv_group:4;
	int      reserved_2:32;
	unsigned reserved_3:4;
	int      reserved_4:4;
	unsigned littlecpu_modify_group:4;
	unsigned g3d_modify_group:4;
	unsigned mif_modify_group:4;
	unsigned int_modify_group:4;
	unsigned cam_disp_modify_group:4;
	unsigned cp_modify_group:4;
	int      asv_table_version:7;
	int      asv_group_type:1;
	int      reserved_5:24;
};

struct id_tbl_info {
	unsigned reserved_0; // product ID 32
	unsigned reserved_1;
	
	unsigned reserved_2:10;
	unsigned product_line:2;
	unsigned reserved_3:4;
	unsigned ids_big:8;
	unsigned ids_g3d:8;

	unsigned ids_others:8; /* little, int, mif, disp */
	unsigned asb_version:8;
	unsigned ids_cp:8;
	unsigned reserved_4:8;

	unsigned reserved_5;
};

#define PORESET					(0x40000000)

#define ASV_INFO_ADDR_CNT	(sizeof(struct asv_tbl_info) / 4)
#define ID_INFO_ADDR_CNT	(sizeof(struct id_tbl_info) / 4)

#define MAX_VOLT_DOMAIN		(7)
#define LAST_VOLT_BASE		(0x90010E00)
#define RGT_MIN_VAL		(0x20)
#define RGT_MIN_VOLT		(500000)
#define RGT_STEP_SIZE		(6250)

#define LAST_FREQ_BASE		(0x900167D0)

struct last_freq_volt {
	char domain_name[8];
	unsigned int freq_offset;
	unsigned int volt_offset;
};

#endif /* __DVFS_INFO_H__ */
