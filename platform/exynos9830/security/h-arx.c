/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 *
 * Alternatively, this program is free software in case of open source project
 * you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.

*/

#include <sys/types.h>
#include <pit.h>

#include <platform/h-arx.h>
#include <platform/hvc.h>
#include <platform/smc.h>

static int load_el2_module(const char *part_name,
			   u64 addr,
			   u64 *size)
{
	struct pit_entry *ptn = NULL;
	int ret = 0;

	ptn = pit_get_part_info(part_name);
	if (ptn == NULL) {
		printf("[EL2_MODULE] ERROR: There is no %s partition\n",
			part_name);
		return -1;
	}

	ret = pit_access(ptn, PIT_OP_LOAD, addr, 0);
	if (ret) {
		printf("[EL2_MODULE] ERROR: PIT(%s) loading error [ret = %d]\n",
			part_name, ret);
		return -1;
	}

	*size = pit_get_length(ptn);
	if (size == 0) {
		printf("[EL2_MODULE] ERROR: pit_get_lenght fail\n");
		return -1;
	}

	return 0;
}

int load_and_init_harx(void)
{
	u64 size = 0;
	u64 ret = 0;

	if (load_el2_module(EXYNOS_HARX_PART_NAME,
			    EXYNOS_HARX_BASE_ADDR,
			    &size)) {
		printf("[H-Arx] ERROR: Fail to load H-Arx binary\n");
		return -1;
	}

	harx_print_with_lcd("[H-Arx] Loading done");

	/* Initialize & Verify H-Arx */
	ret = exynos_smc(SMC_CMD_HARX_INITIALIZATION,
			 EXYNOS_HARX_BASE_ADDR,
			 size,
			 0);
	if (ret) {
		printf("[H-Arx] ERROR: Fail to initialize H-Arx [ret = %llx]\n",
			ret);
		return -1;
	}

	harx_print_with_lcd("[H-Arx] Complete to initialization");

	return 0;
}

int load_and_init_harx_plugin(const char *name, u64 plugin_addr)
{
	u64 size = 0;
	u64 ret = 0;

	if (load_el2_module(name, plugin_addr, &size)) {
		printf("[H-Arx Plug-in] ERROR: Fail to load %s binary\n",
			name);
		return -1;
	}

	harx_print_with_lcd("[H-Arx Plug-in] %s plug-in loading done", name);

	/* Verify & Register plug-in */
	ret = exynos_hvc(HVC_CMD_REGISTER_HARX_PLUGIN,
			 plugin_addr,
			 0, 0, 0);
	if (ret) {
		printf("[H-Arx Plug-in] This binary(%s) is not H-Arx plug-in "
			"[ret = %llx]\n",
			name, ret);
		return -1;
	}

	harx_print_with_lcd("[H-Arx Plug-in] %s plug-in registration done", name);

	return 0;
}
