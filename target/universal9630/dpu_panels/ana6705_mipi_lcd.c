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

#include <dpu/lcd_ctrl.h>
#include <dpu/exynos_panel.h>
#include <dpu/dsim.h>

#define MAX_BRIGHTNESS 255
#define MIN_BRIGHTNESS 0
#define DEFAULT_BRIGHTNESS 170

/* PANEL_ID : ID3[23:16]-ID2[15:8]-ID1[7:0] */
#define PANEL_ID       0x010080 /* value was confirmed when bringup */
extern struct exynos_panel_info ana6705_lcd_info;

static int ana6705_get_id(struct dsim_device *dsim)
{
	dsim_info("%s panel ID is (0x%08x)\n", __func__, PANEL_ID);
	return PANEL_ID;
}

static struct exynos_panel_info *ana6705_get_lcd_info(void)
{
	return &ana6705_lcd_info;
}

static int ana6705_probe(struct dsim_device *dsim)
{
	return 1;
}

static int ana6705_displayon(struct dsim_device *dsim)
{
	ana6705_lcd_init(dsim->id, dsim->lcd_info);
	ana6705_lcd_enable_exynos(dsim->id);
	return 1;
}

static int ana6705_suspend(struct dsim_device *dsim)
{
	return 1;
}

static int ana6705_resume(struct dsim_device *dsim)
{
	return 1;
}

struct dsim_lcd_driver ana6705_mipi_lcd_driver = {
	.get_id		= ana6705_get_id,
	.get_lcd_info	= ana6705_get_lcd_info,
	.probe		= ana6705_probe,
	.displayon	= ana6705_displayon,
	.suspend	= ana6705_suspend,
	.resume		= ana6705_resume,
};

#if 0
struct dsim_lcd_driver *decon_get_panel_info(void)
{
	return &s6e3fa0_mipi_lcd_driver;
}
#endif

