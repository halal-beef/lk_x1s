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
#include <app.h>
#include <lib/console.h>
#include <lib/fastboot.h>
#include <platform/sfr.h>
#include <platform/charger.h>
#include <platform/dfd.h>
#include <platform/gpio.h>
#include <platform/smc.h>
#include <dev/boot.h>

int cmd_boot(int argc, const cmd_args *argv);

static void exynos_boot_task(const struct app_descriptor *app, void *args)
{
	unsigned int rst_stat = readl(EXYNOS_POWER_RST_STAT);
	struct exynos_gpio_bank *bank = (struct exynos_gpio_bank *)EXYNOS9830_GPA0CON;
	int gpio = 3;	/* Volume Up */
	int val;

	if (*(unsigned int *)DRAM_BASE != 0xabcdef) {
		printf("Running on DRAM by TRACE32: skip auto booting\n");
		do_fastboot(0, 0);
		return;
	}

	/* Volume up set Input & Pull up */
	exynos_gpio_set_pull(bank, gpio, GPIO_PULL_UP);
	exynos_gpio_cfg_pin(bank, gpio, GPIO_INPUT);
	val = exynos_gpio_get_value(bank, gpio);
	if (!is_first_boot() ||
			(rst_stat & (WARM_RESET | LITTLE_WDT_RESET | BIG_WDT_RESET))) {
		dfd_set_dump_en_for_cacheop(0);
		sdm_encrypt_secdram();
		goto ramdump;
	}
	if ((readl(CONFIG_RAMDUMP_SCRATCH) == CONFIG_RAMDUMP_MODE) &&
		get_charger_mode() == 0) {
		sdm_encrypt_secdram();
		goto ramdump;
	}
	if (!val)
		do_fastboot(0, 0);

#ifdef RAMDUMP_MODE_OFF
	dfd_set_dump_en_for_cacheop(0);
	set_debug_level("low");
#else
	dfd_set_dump_en_for_cacheop(1);
	set_debug_level("mid");
#endif
	set_debug_level_by_env();
	cmd_boot(0, 0);
	return;
ramdump:
#ifdef RAMDUMP_MODE_OFF
	cmd_boot(0, 0);
#else
	do_fastboot(0, 0);
#endif
	return;
}

APP_START(exynos_boot)
	.entry = exynos_boot_task,
	.flags = 0,
APP_END
