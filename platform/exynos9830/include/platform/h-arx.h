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

#ifndef _HARX_H_
#define _HARX_H_

#include <stdbool.h>

#include <libfdt.h>
#include <lib/font_display.h>

/* Base address */
#define EXYNOS_HARX_BASE_ADDR			(0xC1000000)
#define EXYNOS_HARX_PLUGIN_BASE_ADDR		(0xC0400000)

/* Partition name */
#define EXYNOS_HARX_PART_NAME			"harx"
#define EXYNOS_HARX_PLUGIN_PART_NAME		"harx_rkp"

/* HVC_CMD_GET_HARX_INFO */
#define HARX_INFO_MAJOR_VERSION			(1)
#define HARX_INFO_MINOR_VERSION			(0)
#define HARX_INFO_VERSION			(0xE1200000 |			\
						(HARX_INFO_MAJOR_VERSION << 8) |\
						HARX_INFO_MINOR_VERSION)

/* PSCI method */
#define PSCI_METHOD_NAME			"method"
#define PSCI_METHOD_SMC				"smc"
#define PSCI_METHOD_HVC				"hvc"

/*
 * Callers don't have to write line feed(\n) when calling
 * this macro because print_lcd_update can't handle this
 * character.
 * So this macro calls printf("\n") additionally.
 */
#define harx_print_with_lcd(...)				\
	do {							\
		printf(__VA_ARGS__);				\
		printf("\n");					\
		print_lcd_update(FONT_GREEN, FONT_GRAY,		\
				__VA_ARGS__);			\
	} while (0)

#ifndef __ASSEMBLY__
/* HVC_CMD_GET_HARX_INFO */
extern bool is_harx_initialized;

enum harx_info_type {
	HARX_INFO_TYPE_VERSION = 0,
	HARX_INFO_TYPE_HARX_BASE,
	HARX_INFO_TYPE_HARX_SIZE
};

int load_and_init_harx(void);
int load_and_init_harx_plugin(const char *name, u64 plugin_addr);
int change_dt_psci_method(struct fdt_header *fdt_dtb);
#endif	/* __ASSEMBLY__ */
#endif	/* _HARX_H_ */
