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

extern struct exynos_panel_info td4150_lcd_info;
#define PANEL_ID       0x1e1121 /* value was confirmed when bringup */

void td4150_lcd_init(unsigned int id, struct exynos_panel_info *lcd);
void td4150_lcd_exit(unsigned int id, struct exynos_panel_info *lcd);
void td4150_lcd_enable(unsigned int id);
void td4150_lcd_disable(unsigned int id);
int td4150_lcd_gamma_ctrl(unsigned int id, unsigned int backlightlevel);
int td4150_lcd_gamma_update(int id);

static int td4150_get_id(struct dsim_device *dsim)
{
	dsim_info("%s id %x\n", __func__, PANEL_ID);
	return PANEL_ID; /* td4150 */
}

static struct exynos_panel_info *td4150_get_lcd_info(void)
{
	dsim_info("%s\n", __func__);
	return &td4150_lcd_info;
}

static int td4150_probe(struct dsim_device *dsim)
{
	td4150_lcd_init(dsim->id, dsim->lcd_info);
	return 1;
}

static int td4150_displayon(struct dsim_device *dsim)
{
	dsim_info("%s\n", __func__);
	td4150_lcd_enable_exynos(dsim->id);
	return 1;
}

static int td4150_displayoff(struct dsim_device *dsim)
{
	dsim_info("%s\n", __func__);
	td4150_lcd_disable(dsim->id);
	return 1;
}

static int td4150_suspend(struct dsim_device *dsim)
{
	td4150_lcd_exit(dsim->id, dsim->lcd_info);
	return 1;
}

static int td4150_resume(struct dsim_device *dsim)
{
	return 1;
}

struct dsim_lcd_driver td4150_mipi_lcd_driver = {
	.get_id		= td4150_get_id,
	.get_lcd_info	= td4150_get_lcd_info,
	.probe		= td4150_probe,
	.displayon	= td4150_displayon,
	.suspend	= td4150_suspend,
	.resume		= td4150_resume,
};
