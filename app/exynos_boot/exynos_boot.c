/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <lk/debug.h>
#include <lk/reg.h>
#include <app.h>
#include <dev/usb/gadget.h>
#include <lib/console.h>
#include <lib/fastboot.h>
#include <lib/font_display.h>
#include <platform/delay.h>
#include <platform/sfr.h>
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
	struct exynos_gpio_bank *bank = (struct exynos_gpio_bank *)EXYNOS_GPA0CON;
	int gpio = 4;	/* Volume down */
	int val;

	/* Volume down set Input & Pull up */
	exynos_gpio_set_pull(bank, gpio, GPIO_PULL_UP);
	exynos_gpio_cfg_pin(bank, gpio, GPIO_INPUT);
	mdelay(50);
	val = exynos_gpio_get_value(bank, gpio);

	if(0)
	{
		//writel(0, EXYNOS9830_POWER_SYSIP_DAT0); // Clear reboot reason
		start_usb_gadget();
		return;
	}

	if (!val)
		start_usb_gadget();
	else
		printf("wtf\n");
		//cmd_boot(0, 0);

	return;
}

APP_START(exynos_boot)
	.entry = exynos_boot_task,
	.flags = 0,
APP_END
