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
#define DEFAULT_BRIGHTNESS 0

extern struct exynos_panel_info ea8076_lcd_info;

static int ea8076_get_id(struct dsim_device *dsim)
{
	dsim_info("%s     id %d\n", __func__,0x00034090);
	return 0x00034090; /* ea8076 */
}

static struct exynos_panel_info *ea8076_get_lcd_info(void)
{
	dsim_info("%s\n", __func__);
	return &ea8076_lcd_info;
}

static int ea8076_probe(struct dsim_device *dsim)
{
	return 1;
}

static int ea8076_displayon(struct dsim_device *dsim)
{
	dsim_info("%s     \n", __func__);
	ea8076_lcd_init(dsim->id, dsim->lcd_info);
	ea8076_lcd_enable_exynos(dsim->id);
	return 1;
}

static int ea8076_suspend(struct dsim_device *dsim)
{
	return 1;
}

static int ea8076_resume(struct dsim_device *dsim)
{
	return 1;
}

struct dsim_lcd_driver ea8076_mipi_lcd_driver = {
	.get_id		= ea8076_get_id,
	.get_lcd_info	= ea8076_get_lcd_info,
	.probe		= ea8076_probe,
	.displayon	= ea8076_displayon,
	.suspend	= ea8076_suspend,
	.resume		= ea8076_resume,
};

#if 0
struct dsim_lcd_driver *decon_get_panel_info(void)
{
	return &s6e3fa0_mipi_lcd_driver;
}
#endif
