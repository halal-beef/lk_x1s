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
#include <platform/exynos9830.h>
#include <platform/fvmap.h>


void display_asv_g_info(void)
{
	int i, j;
	struct fvmap_header *fvmap = ((struct fvmap_header*)(ACPM_FVMAP_BASE));
	struct rate_volt_header *header;
	const char *domain_name[11] = { "", "", "LIT", "MID", "BIG", "", "", "", "", "", "G3D" };

	for (i = 0; i < NUM_DOMAINS; i++) {
		switch (fvmap[i].dvfs_type) {
			case ASV_G_LIT:
			case ASV_G_MID:
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
