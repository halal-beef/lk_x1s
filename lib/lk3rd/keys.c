/*
 * Copyright (c) 2024 Igor Belwon <igor.belwon@mentallysanemainliners.org>
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 *
 */

#include <platform/gpio.h>
#include "include/lk3rd/keys.h"

void setup_keys(struct exynos_gpio_bank *bank, int type)
{
	if(type == BANK_GPA0)
	{
		/* Setup pullups */
		exynos_gpio_set_pull(bank, GPIO_VOLUP, GPIO_PULL_UP);
		exynos_gpio_set_pull(bank, GPIO_VOLDOWN, GPIO_PULL_UP);

		/* Pins are buttons, so input. */
		exynos_gpio_cfg_pin(bank, GPIO_VOLUP, GPIO_INPUT);
		exynos_gpio_cfg_pin(bank, GPIO_VOLDOWN, GPIO_INPUT);
	}
	else if(type == BANK_GPA2)
	{
		/* Volume Down is GPIO4, same as Power Button. */
		exynos_gpio_set_pull(bank, GPIO_POWER, GPIO_PULL_UP);
		exynos_gpio_cfg_pin(bank, GPIO_POWER, GPIO_INPUT);
	}
}