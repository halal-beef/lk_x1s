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
	if(current_action == ACTION_END)
	{
		current_action = 0;
	}

	if(current_action == -1)
	{
		current_action = ACTION_END - 1;
	}

	draw_menu(current_action);
}

void platform_prepare_reboot(void);
void platform_do_reboot(const char *cmd_buf);

void do_reboot(enum action action)
{
	platform_prepare_reboot();

	switch(action)
	{
		case ACTION_START:
		case ACTION_POWEROFF:
			// Not implemented.
			platform_do_reboot("");
			break;
		case ACTION_REBOOT_BOOTLOADER:
			platform_do_reboot("reboot-bootloader");
			break;
		case ACTION_REBOOT_RECOVERY:
			platform_do_reboot("reboot-recovery");
			break;
		case ACTION_REBOOT_FASTBOOTD:
			platform_do_reboot("reboot-fastboot");
			break;
		case ACTION_REBOOT_DOWNLOAD:
			platform_do_reboot("reboot-download");
			break;
	}

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
