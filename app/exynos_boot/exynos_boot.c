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
#include <platform/sfr.h>
#include <platform/charger.h>
#include <platform/fastboot.h>
#include <platform/dfd.h>
#include <platform/gpio.h>
#include <dev/boot.h>

int cmd_boot(int argc, const cmd_args *argv);

static void exynos_boot_task(const struct app_descriptor *app, void *args)
{
	unsigned int rst_stat = readl(EXYNOS_POWER_RST_STAT);
	struct exynos_gpio_bank *bank = (struct exynos_gpio_bank *)EXYNOS9830_GPA0CON;
	int gpio = 3;	/* Volume Up */
	int val;

	printf("RST_STAT: 0x%x\n", rst_stat);

	if (*(unsigned int *)DRAM_BASE != 0xabcdef) {
		printf("Running on DRAM by TRACE32: skip auto booting\n");
		do_fastboot(0, 0);
		return;
	}

	/* Volume up set Input & Pull up */
	exynos_gpio_set_pull(bank, gpio, GPIO_PULL_UP);
	exynos_gpio_cfg_pin(bank, gpio, GPIO_INPUT);
	val = exynos_gpio_get_value(bank, gpio);
	if (!is_first_boot() || (rst_stat & (WARM_RESET | LITTLE_WDT_RESET | BIG_WDT_RESET)) ||
		((readl(CONFIG_RAMDUMP_SCRATCH) == CONFIG_RAMDUMP_MODE) && get_charger_mode() == 0) ||
		!val) {
		do_fastboot(0, 0);
	} else {
		cmd_boot(0, 0);
	}

	return;
}

APP_START(exynos_boot)
	.entry = exynos_boot_task,
	.flags = 0,
APP_END
