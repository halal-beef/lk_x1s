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

#ifndef __DISPLAY_CONFIG_H__
#define __DISPLAY_CONFIG_H__

/* Enable BOOTLOADER DISPLAY : call display function at platform.c */
//#define CONFIG_EXYNOS_BOOTLOADER_DISPLAY

/* Enable DSIM BIST FOR TEST : DSIM BIST CONFIG needs CONFIG_EXYNOS_BOOTLOADER_DISPLAY */
//#define CONFIG_EXYNOS_DSIM_BIST
#if defined(CONFIG_EXYNOS_DSIM_BIST)
#define CONFIG_EXYNOS_BOOTLOADER_DISPLAY
#endif

/* Enable DMA BIST FOR TEST : DMA BIST CONFIG needs CONFIG_EXYNOS_BOOTLOADER_DISPLAY */
//#define CONFIG_EXYNOS_DMA_BIST
#if defined(CONFIG_EXYNOS_DMA_BIST)
#define CONFIG_EXYNOS_BOOTLOADER_DISPLAY
#endif

/* Enable FONT : FONT CONFIG needs CONFIG_EXYNOS_BOOTLOADER_DISPLAY */
//#define CONFIG_DISPLAY_DRAWFONT
#if defined(CONFIG_DISPLAY_DRAWFONT)
#define CONFIG_EXYNOS_BOOTLOADER_DISPLAY
#endif

/* Enable LOGO : LOGO CONFIG needs CONFIG_EXYNOS_BOOTLOADER_DISPLAY */
//#define CONFIG_DISPLAY_DRAWLOGO
#if defined(CONFIG_DISPLAY_DRAWLOGO)
#define CONFIG_EXYNOS_BOOTLOADER_DISPLAY
#endif

/* Enable LOGO using PIT : PIT CONFIG needs CONFIG_EXYNOS_BOOTLOADER_DISPLAY */
#if defined(CONFIG_DISPLAY_DRAWLOGO)
#define CONFIG_PIT
#endif
#if defined(CONFIG_PIT)
#define CONFIG_EXYNOS_BOOTLOADER_DISPLAY
#endif

#define CONFIG_DISPLAY_LOGO_BASE_ADDRESS	0xf9800000
#define CONFIG_DISPLAY_FONT_BASE_ADDRESS	0xfac00000
#define CONFIG_DISPLAY_TEMP_BASE_ADDRESS	0xfc000000

#define LCD_WIDTH		1080
#define LCD_HEIGHT		1920


/* =================================================== */
/* Below configs can be used by display member only.
 * Those are debug featue for display bring up
 */

/* If you want to test winmap, fix WINMAP to 1 */
#define WINMAP 0

/* =================================================== */

/* Platform Dependency */
#include <dpu/decon.h>
#include <dpu/display_sfr.h>

/* TODO : divide platform info and target info */
//#define NUM_OF_DPP   4
#define MAX_DPP_CNT	4
#define LOGO_DPP	2
#define FONT_DPP	1
#define DFT_DSIM	0
#define DFT_DECON	0


/* DECON Properties */
static struct decon_dt_info decon_dt = {
	.max_win	= 4,
	.dft_win	= 0,	/* window for font should be top window, It means it's zero */
	.dft_ch		= LOGO_DPP,
	.psr_mode	= 2,	/* 0: video mode, 1: DP command mode, 2: MIPI command mode */
	.trig_mode	= 0,	/* 0: hw trigger, 1: sw trigger */
	.dsi_mode	= 0,	/* 0: single dsi, 1: dual dsi */
	.out_type	= 0,	/* 0: DSI, 1: eDP, 2:HDMI, 3: WB */
	.out_idx	= 0,	/* 0: DSI0, 1: DSI1, 2: DSI2 */
	.ss_regs	= (void __iomem *)DPU_SYSREG_BASE_ADDR,
};
/* TODO : add dsim_dt and parse function at dsim_drv */


/* GET platdata API list */
static inline struct decon_dt_info *get_decon_pdata(void)
{
	return &decon_dt;
}
#endif /* __DISPLAY_CONFIG_H__ */
