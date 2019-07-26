/* Copyright (c) 2018 Samsung Electronics Co, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Copyright@ Samsung Electronics Co. LTD
 * Manseok Kim <manseoks.kim@samsung.com>
 *
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

#include <dev/dpu/lcd_ctrl.h>
#include <dev/dpu/exynos_panel.h>
#include <dev/dpu/dsim.h>

#define MAX_BRIGHTNESS		255
#define MIN_BRIGHTNESS		0
#define DEFAULT_BRIGHTNESS	0

extern struct exynos_panel_info nt36672a_lcd_info;

static int nt36672a_get_id(struct dsim_device *dsim)
{
	return 0xff216102; /* nt36672a */
}

static struct exynos_panel_info *nt36672a_get_lcd_info(void)
{
	dsim_info("%s\n", __func__);
	return &nt36672a_lcd_info;
}

static int nt36672a_probe(struct dsim_device *dsim)
{
	return 1;
}

static int nt36672a_displayon(struct dsim_device *dsim)
{
	dsim_info("%s +\n", __func__);
	nt36672a_lcd_init(dsim->id, dsim->lcd_info);
	nt36672a_lcd_enable_exynos(dsim->id);
	dsim_info("%s -\n", __func__);
	return 1;
}

static int nt36672a_suspend(struct dsim_device *dsim)
{
	return 0;
}

static int nt36672a_resume(struct dsim_device *dsim)
{
	return 0;
}

struct dsim_lcd_driver nt36672a_mipi_lcd_driver = {
	.get_id		= nt36672a_get_id,
	.get_lcd_info	= nt36672a_get_lcd_info,
	.probe		= nt36672a_probe,
	.displayon	= nt36672a_displayon,
	.suspend	= nt36672a_suspend,
	.resume		= nt36672a_resume,
};

#if 0
struct dsim_lcd_driver *decon_get_panel_info(void)
{
	return &nt36672a_mipi_lcd_driver;
}
#endif
