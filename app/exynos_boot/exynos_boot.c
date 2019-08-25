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
#include <dev/usb/gadget.h>
#include <lib/console.h>
#include <lib/fastboot.h>
#include <lib/font_display.h>
#include <platform/sfr.h>
#include <platform/charger.h>
#include <platform/gpio.h>
#include <platform/smc.h>
#include <platform/chip_rev.h>
#include <dev/boot.h>
#include <dev/debug/dss.h>
#include <dev/debug/dss_store_ramdump.h>

int cmd_boot(int argc, const cmd_args *argv);

static unsigned int need_do_fastboot = 0;
static const char *fastboot_reason[] = {
	"PMIC WTSR Detected!",
	"PMIC SMPL Detected!",
};

void set_do_fastboot(enum fastboot_type type)
{
	need_do_fastboot |= 1 << type;
}

static void print_fastboot_reason(void)
{
	unsigned int i = 0;

	for (i = 0; i < FASTBOOT_TYPE_END; i++) {
		if (need_do_fastboot & 1 << i) {
			printf("Fastboot Reason >> %s\n", fastboot_reason[i]);
		}
	}
}

static void exynos_boot_task(const struct app_descriptor *app, void *args)
{
	unsigned int rst_stat = readl(EXYNOS_POWER_RST_STAT);
	struct exynos_gpio_bank *bank = (struct exynos_gpio_bank *)EXYNOS9830_GPA0CON;
	int gpio = 3;	/* Volume Up */
	int val;

	if (*(unsigned int *)DRAM_BASE != 0xabcdef) {
		printf("Running on DRAM by TRACE32: skip auto booting\n");
		start_usb_gadget();
		return;
	}

	printf("AP revision is EVT%d\n", s5p_chip_rev.main);
	print_lcd(FONT_RED, FONT_BLACK, "AP revision is EVT%d", s5p_chip_rev.main);
	if (s5p_chip_rev.main != (unsigned int)dfd_get_revision()) {
		printf("Using invalid BL2!\n");
		printf("Current BL2 is for EVT%d\n", dfd_get_revision());
		print_lcd(FONT_RED, FONT_BLACK, "Using invalid BL2!");
		print_lcd(FONT_RED, FONT_BLACK, "Current BL2 is for EVT%d", dfd_get_revision());
		start_usb_gadget();
		return;
	}

	/* Volume up set Input & Pull up */
	exynos_gpio_set_pull(bank, gpio, GPIO_PULL_UP);
	exynos_gpio_cfg_pin(bank, gpio, GPIO_INPUT);
	mdelay(50);
	val = exynos_gpio_get_value(bank, gpio);
	if (!is_first_boot() ||
			(rst_stat & (WARM_RESET | LITTLE_WDT_RESET | BIG_WDT_RESET))) {
		dfd_set_dump_en(0);
		sdm_encrypt_secdram();
		goto ramdump;
	}
	if ((readl(CONFIG_RAMDUMP_SCRATCH) == CONFIG_RAMDUMP_MODE) &&
		get_charger_mode() == 0) {
		sdm_encrypt_secdram();
		goto ramdump;
	}

	if (need_do_fastboot) {
		sdm_encrypt_secdram();
		print_fastboot_reason();
		goto ramdump;
	}

	if (!val)
		start_usb_gadget();

#ifdef RAMDUMP_MODE_OFF
	dfd_set_dump_en(0);
	set_debug_level("low");
#else
	dfd_set_dump_en(1);
	set_debug_level("mid");
#endif
	cmd_boot(0, 0);
	return;
ramdump:
#ifdef RAMDUMP_MODE_OFF
	cmd_boot(0, 0);
#else
	debug_store_ramdump();
	dfd_set_dump_en(0);
	start_usb_gadget();
#endif
	return;
}

APP_START(exynos_boot)
	.entry = exynos_boot_task,
	.flags = 0,
APP_END
