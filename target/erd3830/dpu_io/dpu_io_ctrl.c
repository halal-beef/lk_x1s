/* Copyright (c) 2018 Samsung Electronics Co, Ltd.

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.

 * Copyright@ Samsung Electronics Co. LTD
 * Manseok Kim <manseoks.kim@samsung.com>

 * Alternatively, Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <lk/reg.h>
#include <stdio.h>
#include <platform/sfr.h>
#include <platform/delay.h>
#include <target/dpu_io_ctrl.h>

void display_te_init(void);
void display_panel_reset(void);
void display_panel_release(void);
void display_panel_power(void);
/*
 * ########## Machine dependency ##########
 */
static void mipi_phy_control(unsigned int dev_index,
					unsigned int enable)
{
	unsigned int addr, cfg = 0;

	/* DPHY isolation with PMU */
	addr = EXYNOS3830_POWER_MIPI_PHY_M4S4_CONTROL;

	cfg = readl(addr);
	if (enable)
		cfg |= EXYNOS_MIPI_PHY_ENABLE;
	else
		cfg &= ~(EXYNOS_MIPI_PHY_ENABLE);

	writel(cfg, addr);
}


/* MIPI-PHY related with master interface */
static void set_mipi_phy_control(unsigned int enable)
{
	mipi_phy_control(0, enable);
}

/* Set GPIO internal interrupt when DECON are using HW trigger */
static void set_gpio_hw_te(void)
{
	/* HW TE: GPC2_3(TE_DECON_F) */
	display_te_init();
}


/*
 * ########## Board dependency ##########
 */

/* Configure and set a GPIO for LCD_RESET */
static void set_gpio_lcd_reset(enum board_gpio_type gpio_type)
{
	switch (gpio_type) {
	case BOARD_ESPRESSO:
		printf("%s: ESPRESSO\n", __func__);
		return;
	case BOARD_BTYPE:
		/* GPIO: LCD RESET */
		printf("%s: BTYPE\n", __func__);
		break;
	case BOARD_BTYPE_REV01:
		/* GPIO: LCD RESET */
		printf("%s: BTYPE_REV01\n", __func__);
		break;
	default:
		printf("%s: Does not support it!!\n", __func__);
		return;
	}

	/* RESET: "0" -> "1" */
	display_panel_reset();
	mdelay(5);
	display_panel_release();
	mdelay(10);
}

/* Configure and set a GPIO for LCD_POWER_ON */
static void set_gpio_lcd_power(enum board_gpio_type gpio_type)
{
	/* Enable Power */
	printf("%s: +\n", __func__);
	display_panel_power();
	printf("%s: -\n", __func__);
	mdelay(10);
}

struct exynos_display_config display_config = {
	.set_mipi_phy = set_mipi_phy_control,
	.set_gpio_hw_te = set_gpio_hw_te,
	.set_gpio_lcd_reset = set_gpio_lcd_reset,
	.set_gpio_lcd_power = set_gpio_lcd_power,
};

/* Get type of board */
enum board_gpio_type get_exynos_board_type(void)
{
	return BOARD_BTYPE;
}

/* Get display configuration callbacks */
struct exynos_display_config *get_exynos_display_config(void)
{
	return &display_config;
}
