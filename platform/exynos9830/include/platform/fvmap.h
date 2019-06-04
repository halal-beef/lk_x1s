/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __FVMAP_H__
#define __FVMAP_H__

#define ACPM_SRAM_BASE		0x02039000
#define ACPM_DVFS_OFFSET	0x00027000
#define ACPM_FVMAP_OFFSET	0x0000BC00
#define ACPM_FVMAP_BASE		ACPM_SRAM_BASE + ACPM_DVFS_OFFSET + ACPM_FVMAP_OFFSET
#define NUM_DOMAINS			16
#define ASV_G_LIT			0x2
#define ASV_G_MID			0x3
#define ASV_G_BIG			0x4
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

#endif /* __FVMAP_H__ */
