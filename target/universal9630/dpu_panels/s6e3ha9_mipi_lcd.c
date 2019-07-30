/* drivers/video/exynos/decon/panels/s6e3ha9_mipi_lcd.c
 *
 * Samsung SoC MIPI LCD driver.
 *
 * Copyright (c) 2014 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <dev/dpu/lcd_ctrl.h>
#include <dev/dpu/exynos_panel.h>
#include <dev/dpu/dsim.h>

#define MAX_BRIGHTNESS 255
#define MIN_BRIGHTNESS 0
#define DEFAULT_BRIGHTNESS 0

/* PANEL_ID : ID3[23:16]-ID2[15:8]-ID1[7:0] */
#define PANEL_ID       0x1310a1 /* value was confirmed when bringup */
extern struct exynos_panel_info s6e3ha9_lcd_info;

static int s6e3ha9_get_id(struct dsim_device *dsim)
{
	dsim_info("%s panel ID is (0x%08x)\n", __func__, PANEL_ID);
	return PANEL_ID;
}

static struct exynos_panel_info *s6e3ha9_get_lcd_info(void)
{
	return &s6e3ha9_lcd_info;
}

static int s6e3ha9_probe(struct dsim_device *dsim)
{
	return 1;
}

static int s6e3ha9_displayon(struct dsim_device *dsim)
{
	s6e3ha9_lcd_init(dsim->id, dsim->lcd_info);
	s6e3ha9_lcd_enable_exynos(dsim->id);
	return 1;
}

static int s6e3ha9_suspend(struct dsim_device *dsim)
{
	return 1;
}

static int s6e3ha9_resume(struct dsim_device *dsim)
{
	return 1;
}

struct dsim_lcd_driver s6e3ha9_mipi_lcd_driver = {
	.get_id		= s6e3ha9_get_id,
	.get_lcd_info	= s6e3ha9_get_lcd_info,
	.probe		= s6e3ha9_probe,
	.displayon	= s6e3ha9_displayon,
	.suspend	= s6e3ha9_suspend,
	.resume		= s6e3ha9_resume,
};

#if 0
struct dsim_lcd_driver *decon_get_panel_info(void)
{
	return &s6e3fa0_mipi_lcd_driver;
}
#endif

