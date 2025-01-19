/*
 * Copyright (c) 2024 Igor Belwon <igor.belwon@mentallysanemainliners.org>
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 *
 */

#include <dev/usb/gadget.h>
#include <dev/scsi.h>
#include <kernel/thread.h>

#include <lib/font_display.h>
#include <platform.h>
#include <platform/gpio.h>
#include <reg.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "include/lk3rd/keys.h"
#include "include/lk3rd/fastboot_menu.h"
#include "include/lk3rd/display.h"

enum action current_action = ACTION_START;

void notify_action_switch(int modifier)
{
	current_action = current_action + modifier;

	/* Rollover */
	if(current_action == 4)
	{
		current_action = 0;
	}

	if(current_action == -1)
	{
		current_action = 3;
	}

	draw_menu(current_action);
}

/*
 * The reboot modes aren't the real reboot modes for production boards.
 * We use the modes that were specified by this lk fork, as they are
 * ignored by the bootloader, but they succesfully get passed back
 * to us after a reboot. Thus, lk3rd is ran every reboot (except for dl mode).
*/
#define REBOOT_MODE_RECOVERY	0xFF

void do_reboot(enum action action)
{
	/*
	 * PON (Power off notification) to storage
	 */
	scsi_do_ssu();

	switch(action)
	{
		case ACTION_POWEROFF:
		case ACTION_REBOOTBOOTLOADER:
			// Not implemented.
			break;
		case ACTION_REBOOTRECOVERY:
			writel(REBOOT_MODE_RECOVERY, EXYNOS_POWER_SYSIP_DAT0);
			writel(0, CONFIG_RAMDUMP_SCRATCH);
			break;
		case ACTION_START:
			writel(CONFIG_RAMDUMP_MODE, CONFIG_RAMDUMP_SCRATCH);
			break;
	}

	writel(RAMDUMP_SIGN_BL_REBOOT, CONFIG_RAMDUMP_REASON);
	writel(0x2, EXYNOS_POWER_SYSTEM_CONFIGURATION);

	// Should never return.
	thread_exit(0);
}

void notify_action_start(void)
{
	do_reboot(current_action);
	return;
}

int fastboot_menu_entry(void *arg)
{
	struct exynos_gpio_bank *bank_volume = (struct exynos_gpio_bank *)EXYNOS9830_GPA0CON;
	struct exynos_gpio_bank *bank_power = (struct exynos_gpio_bank *)EXYNOS9830_GPA2CON;
	int volup, voldown, power, key_stuck = 0;

	setup_keys(bank_volume, BANK_GPA0);
	setup_keys(bank_power, BANK_GPA2);
	draw_menu(current_action);

	while (true)
	{
		volup = exynos_gpio_get_value(bank_volume, GPIO_VOLUP);
		voldown = exynos_gpio_get_value(bank_volume, GPIO_VOLDOWN);
		power = exynos_gpio_get_value(bank_power, GPIO_POWER);

		if(!key_stuck)
		{
			if (!volup)
			{
				notify_action_switch(-1);
				key_stuck = 1;
			}
			if (!voldown)
			{
				notify_action_switch(1);
				key_stuck = 1;
			}
			if (!power)
			{
				notify_action_start();
				key_stuck = 1;
			}
		}
		if(volup && voldown && power)
		{
			key_stuck = 0;
		}

		// Don't freeze up lk.
		thread_sleep(100);
	}
}
