/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.

 * Alternatively, this program is free software in case of open source project
 * you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.

*/

#include <reg.h>
#include <string.h>
#include <stdio.h>
#include <lib/console.h>
#include <lib/fastboot.h>
#include <platform/sfr.h>
#include <platform/dfd.h>

#if 0
#define DBG_ENV_NAME "dbg_test"

static char *env_name = DBG_ENV_NAME;

void dfd_verify_enable(void)
{
	int ret;
	char buf[16];
	char commands[CONFIG_SYS_CBSIZE];

	memset(buf, 0, sizeof(buf));
	ret = getenv_s(env_name, buf, sizeof(buf));
	if (ret > 0) {
		snprintf(commands, sizeof(commands),
				"fdt set /exynos-debug-test enabled \"dbg_test\"");
		run_command(commands, 0);
	}
}
#endif

void dfd_verify_fn(unsigned int rst_stat, unsigned int reason)
{
	unsigned int test_case;
	unsigned int curr_test_num;
	unsigned int total_test_num;

	if (reason == RAMDUMP_REBOOT_TBD) {
		if ((rst_stat & (WARM_RESET | LITTLE_WDT_RESET))) {
			if (rst_stat & LITTLE_WDT_RESET)
				reason = RAMDUMP_REBOOT_WDT;
			else
				writel(RAMDUMP_SIGN_RESET, CONFIG_RAMDUMP_DEBUG_TEST);
		} else if (readl(CONFIG_RAMDUMP_SCRATCH) == CONFIG_RAMDUMP_MODE) {
			if (readl(CONFIG_RAMDUMP_REASON) == RAMDUMP_SIGN_PANIC)
				reason = RAMDUMP_REBOOT_PANIC;
		}
	}

	test_case = readl(CONFIG_RAMDUMP_DEBUG_TEST_CASE);

	switch (reason) {
	case RAMDUMP_REBOOT_WTSR:
		writel(readl(CONFIG_RAMDUMP_DEBUG_TEST_WTSR) |
				(1 << test_case), CONFIG_RAMDUMP_DEBUG_TEST_WTSR);
		break;
	case RAMDUMP_REBOOT_SMPL:
		writel(readl(CONFIG_RAMDUMP_DEBUG_TEST_SMPL) |
				(1 << test_case), CONFIG_RAMDUMP_DEBUG_TEST_SMPL);
		break;
	case RAMDUMP_REBOOT_WDT:
		writel(readl(CONFIG_RAMDUMP_DEBUG_TEST_WDT) |
				(1 << test_case), CONFIG_RAMDUMP_DEBUG_TEST_WDT);
		break;
	case RAMDUMP_REBOOT_PANIC:
		writel(readl(CONFIG_RAMDUMP_DEBUG_TEST_PANIC) |
				(1 << test_case), CONFIG_RAMDUMP_DEBUG_TEST_PANIC);
		break;
	default:
		break;
	}
	writel(test_case + 1, CONFIG_RAMDUMP_DEBUG_TEST_CASE);
	if (test_case > readl(CONFIG_RAMDUMP_DEBUG_TEST_NEXT)) {
		printf("DEBUG TEST: ERROR DETECTED!\n");
		writel(RAMDUMP_SIGN_RESET, CONFIG_RAMDUMP_DEBUG_TEST);
		do_fastboot(0, 0);
	}
	if (reason) {
		curr_test_num = readl(CONFIG_RAMDUMP_DEBUG_TEST_CURR) + 1;
		total_test_num = readl(CONFIG_RAMDUMP_DEBUG_TEST_TOTAL);
		writel(curr_test_num, CONFIG_RAMDUMP_DEBUG_TEST_CURR);

		printf("DEBUG TEST: TEST ON GOING! [%d/%d]\n",
						curr_test_num, total_test_num);
		printf("DEBUG TEST: this time reboot status[0x%x]\n", reason);
		writel(RAMDUMP_SIGN_RESET, CONFIG_RAMDUMP_SCRATCH);
		//run_command("reset", 0);
	}
}
