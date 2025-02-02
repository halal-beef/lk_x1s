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


#include <lk/reg.h>
#include <platform/sfr.h>
#include <platform/gpio.h>

#define CON_MASK(x)	(0xf << ((x) << 2))
#define CON_SFR(x, v)	((v) << ((x) << 2))

#define DAT_MASK(x)	(0x1 << (x))
#define DAT_SET(x)	(0x1 << (x))

#define PULL_MASK(x)	(0xf << ((x) << 2))
#define PULL_MODE(x, v)	((v) << ((x) << 2))

#define DRV_MASK(x)	(0xf << ((x) << 2))
#define DRV_SET(x, m)	((m) << ((x) << 2))

#define RATE_MASK(x)	(0x1 << (x + 16))
#define RATE_SET(x)	(0x1 << (x + 16))

#ifdef CONFIG_GPIO_DAT_MASK
void exynos_gpio_dat_mask(struct exynos_gpio_bank *bank, unsigned int *dat)
{
	unsigned int i, con, dat_mask = 0;

	con = readl(&bank->con);
	for (i = 0; i < 8; i++)	{
		if (((con >> (i * 4)) & 0xf) == GPIO_OUTPUT)
			dat_mask |= 1 << i;
	}

	*dat &= dat_mask;
}

#else
void exynos_gpio_dat_mask(struct exynos_gpio_bank *bank, unsigned int *dat)
{
}
#endif /* ifdef CONFIG_GPIO_DAT_MASK */

void exynos_gpio_cfg_pin(struct exynos_gpio_bank *bank, int gpio, int cfg)
{
	unsigned int value;

	value = readl(&bank->con);
	value &= ~CON_MASK(gpio);
	value |= CON_SFR(gpio, cfg);
	writel(value, &bank->con);
}

void exynos_gpio_direction_output(struct exynos_gpio_bank *bank, int gpio, int en)
{
	unsigned int value;

	exynos_gpio_cfg_pin(bank, gpio, GPIO_OUTPUT);

	value = readl(&bank->dat);
	value &= ~DAT_MASK(gpio);
	if (en)
		value |= DAT_SET(gpio);
	exynos_gpio_dat_mask(bank, &value);
	writel(value, &bank->dat);
}

void exynos_gpio_direction_input(struct exynos_gpio_bank *bank, int gpio)
{
	exynos_gpio_cfg_pin(bank, gpio, GPIO_INPUT);
}

void exynos_gpio_set_value(struct exynos_gpio_bank *bank, int gpio, int en)
{
	unsigned int value;

	value = readl(&bank->dat);
	value &= ~DAT_MASK(gpio);
	if (en)
		value |= DAT_SET(gpio);
	exynos_gpio_dat_mask(bank, &value);
	writel(value, &bank->dat);
}

unsigned int exynos_gpio_get_value(struct exynos_gpio_bank *bank, int gpio)
{
	unsigned int value;

	value = readl(&bank->dat);
	return !!(value & DAT_MASK(gpio));
}

void exynos_gpio_set_pull(struct exynos_gpio_bank *bank, int gpio, int mode)
{
	unsigned int value;

	value = readl(&bank->pull);
	value &= ~PULL_MASK(gpio);

	switch (mode) {
	case GPIO_PULL_NONE:
	case GPIO_PULL_DOWN:
	case GPIO_PULL_UP:
		value |= PULL_MODE(gpio, mode);
		break;
	default:
		break;
	}

	writel(value, &bank->pull);
}

void exynos_gpio_set_drv(struct exynos_gpio_bank *bank, int gpio, int mode)
{
	unsigned int value;

	value = readl(&bank->drv);
	value &= ~DRV_MASK(gpio);

	value |= DRV_SET(gpio, mode);

	writel(value, &bank->drv);
}

void exynos_gpio_set_rate(struct exynos_gpio_bank *bank, int gpio, int mode)
{
	unsigned int value;

	value = readl(&bank->drv);
	value &= ~RATE_MASK(gpio);

	switch (mode) {
	case GPIO_DRV_FAST:
	case GPIO_DRV_SLOW:
		value |= RATE_SET(gpio);
		break;
	default:
		return;
	}

	writel(value, &bank->drv);
}

struct exynos_gpio_bank *exynos_gpio_get_bank(unsigned gpio)
{
	int bank = gpio / GPIO_PER_BANK;

	bank *= sizeof(struct exynos_gpio_bank);

	return (struct exynos_gpio_bank *)(exynos_gpio_base(gpio) + bank);
}

int exynos_gpio_get_pin(unsigned gpio)
{
	return gpio % GPIO_PER_BANK;
}

/* Common GPIO API */

int gpio_request(unsigned gpio, const char *label)
{
	return 0;
}

int gpio_free(unsigned gpio)
{
	return 0;
}

int gpio_direction_input(unsigned gpio)
{
	exynos_gpio_direction_input(exynos_gpio_get_bank(gpio),
	                            exynos_gpio_get_pin(gpio));
	return 0;
}

int gpio_direction_output(unsigned gpio, int value)
{
	exynos_gpio_direction_output(exynos_gpio_get_bank(gpio),
	                             exynos_gpio_get_pin(gpio), value);
	return 0;
}

int gpio_get_value(unsigned gpio)
{
	return (int)exynos_gpio_get_value(exynos_gpio_get_bank(gpio),
	                                  exynos_gpio_get_pin(gpio));
}

int gpio_set_value(unsigned gpio, int value)
{
	exynos_gpio_set_value(exynos_gpio_get_bank(gpio),
	                      exynos_gpio_get_pin(gpio), value);

	return 0;
}
