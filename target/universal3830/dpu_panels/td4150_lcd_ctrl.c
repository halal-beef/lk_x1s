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
#include <dpu/mipi_dsi_cmd.h>
#include <dpu/dsim.h>
#include "td4150_param.h"

#define TD4150_VDO_VBP		4
#define TD4150_VDO_VFP		24
#define TD4150_VDO_VSA		2
#define TD4150_VDO_HBP		80
#define TD4150_VDO_HFP		508
#define TD4150_VDO_HSA		2

#define TD4150_HORIZONTAL	720
#define TD4150_VERTICAL		1600

struct exynos_panel_info td4150_lcd_info = {
	.mode = DECON_VIDEO_MODE,
	.vfp = TD4150_VDO_VFP,
	.vbp = TD4150_VDO_VBP,
	.hfp = TD4150_VDO_HFP,
	.hbp = TD4150_VDO_HBP,
	.vsa = TD4150_VDO_VSA,
	.hsa = TD4150_VDO_HSA,
	.xres = TD4150_HORIZONTAL,
	.yres = TD4150_VERTICAL,

	/* Mhz */
	.hs_clk = 830,
	.esc_clk = 20,
	.dphy_pms = {2, 255, 2, 25206}, /* pmsk */

	//.vt_compensation = 0,

	.width = 68,
	.height = 147,

	.fps = 60,

	.dsc = {0, 0, 0, 40, 720, 240},
	.data_lane = 4,
};

static void dsi_write(u32 id, const unsigned char *wbuf, int size)
{
	if (size == 1)
		dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE, wbuf[0], 0);
	else if (size == 2)
		dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE_PARAM, wbuf[0], wbuf[1]);
	else
		dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)wbuf, size);
}

void td4150_lcd_init(unsigned int id, struct exynos_panel_info *lcd)
{
	dsi_write(id, SEQ_TD4150_B0_OPEN, ARRAY_SIZE(SEQ_TD4150_B0_OPEN));
	dsi_write(id, SEQ_TD4150_B6, ARRAY_SIZE(SEQ_TD4150_B6));
	dsi_write(id, SEQ_TD4150_B7, ARRAY_SIZE(SEQ_TD4150_B7));
	dsi_write(id, SEQ_TD4150_B8, ARRAY_SIZE(SEQ_TD4150_B8));
	dsi_write(id, SEQ_TD4150_B9, ARRAY_SIZE(SEQ_TD4150_B9));
	dsi_write(id, SEQ_TD4150_BA, ARRAY_SIZE(SEQ_TD4150_BA));
	dsi_write(id, SEQ_TD4150_BB, ARRAY_SIZE(SEQ_TD4150_BB));
	dsi_write(id, SEQ_TD4150_BC, ARRAY_SIZE(SEQ_TD4150_BC));
	dsi_write(id, SEQ_TD4150_BD, ARRAY_SIZE(SEQ_TD4150_BD));
	dsi_write(id, SEQ_TD4150_BE, ARRAY_SIZE(SEQ_TD4150_BE));
	dsi_write(id, SEQ_TD4150_C0, ARRAY_SIZE(SEQ_TD4150_C0));
	dsi_write(id, SEQ_TD4150_C1, ARRAY_SIZE(SEQ_TD4150_C1));
	dsi_write(id, SEQ_TD4150_C2, ARRAY_SIZE(SEQ_TD4150_C2));
	dsi_write(id, SEQ_TD4150_C3, ARRAY_SIZE(SEQ_TD4150_C3));
	dsi_write(id, SEQ_TD4150_C4, ARRAY_SIZE(SEQ_TD4150_C4));
	dsi_write(id, SEQ_TD4150_C5, ARRAY_SIZE(SEQ_TD4150_C5));
	dsi_write(id, SEQ_TD4150_C6, ARRAY_SIZE(SEQ_TD4150_C6));
	dsi_write(id, SEQ_TD4150_C7, ARRAY_SIZE(SEQ_TD4150_C7));
	dsi_write(id, SEQ_TD4150_C8, ARRAY_SIZE(SEQ_TD4150_C8));
	dsi_write(id, SEQ_TD4150_C9, ARRAY_SIZE(SEQ_TD4150_C9));
	dsi_write(id, SEQ_TD4150_CA, ARRAY_SIZE(SEQ_TD4150_CA));
	dsi_write(id, SEQ_TD4150_CB, ARRAY_SIZE(SEQ_TD4150_CB));
	dsi_write(id, SEQ_TD4150_CE, ARRAY_SIZE(SEQ_TD4150_CE));
	dsi_write(id, SEQ_TD4150_CF, ARRAY_SIZE(SEQ_TD4150_CF));
	dsi_write(id, SEQ_TD4150_D0, ARRAY_SIZE(SEQ_TD4150_D0));
	dsi_write(id, SEQ_TD4150_D1, ARRAY_SIZE(SEQ_TD4150_D1));
	dsi_write(id, SEQ_TD4150_D2, ARRAY_SIZE(SEQ_TD4150_D2));
	dsi_write(id, SEQ_TD4150_D3, ARRAY_SIZE(SEQ_TD4150_D3));
	dsi_write(id, SEQ_TD4150_E5, ARRAY_SIZE(SEQ_TD4150_E5));
	dsi_write(id, SEQ_TD4150_D6, ARRAY_SIZE(SEQ_TD4150_D6));
	dsi_write(id, SEQ_TD4150_D7, ARRAY_SIZE(SEQ_TD4150_D7));
	dsi_write(id, SEQ_TD4150_D9, ARRAY_SIZE(SEQ_TD4150_D9));
	dsi_write(id, SEQ_TD4150_DD, ARRAY_SIZE(SEQ_TD4150_DD));
	dsi_write(id, SEQ_TD4150_DE, ARRAY_SIZE(SEQ_TD4150_DE));
	dsi_write(id, SEQ_TD4150_E3, ARRAY_SIZE(SEQ_TD4150_E3));
	dsi_write(id, SEQ_TD4150_E9, ARRAY_SIZE(SEQ_TD4150_E9));
	dsi_write(id, SEQ_TD4150_EA, ARRAY_SIZE(SEQ_TD4150_EA));
	dsi_write(id, SEQ_TD4150_EB, ARRAY_SIZE(SEQ_TD4150_EB));
	dsi_write(id, SEQ_TD4150_EC, ARRAY_SIZE(SEQ_TD4150_EC));
	dsi_write(id, SEQ_TD4150_ED, ARRAY_SIZE(SEQ_TD4150_ED));
	dsi_write(id, SEQ_TD4150_EE, ARRAY_SIZE(SEQ_TD4150_EE));
	dsi_write(id, SEQ_TD4150_B0_CLOSE, ARRAY_SIZE(SEQ_TD4150_B0_CLOSE));
	dsi_write(id, SEQ_SLEEP_OUT, ARRAY_SIZE(SEQ_SLEEP_OUT));
}

void td4150_lcd_enable_exynos(unsigned int id)
{
	mdelay(100);
	dsi_write(id, SEQ_DISPLAY_ON, ARRAY_SIZE(SEQ_DISPLAY_ON));
	dsi_write(id, SEQ_TD4150_BRIGHTNESS_ON, ARRAY_SIZE(SEQ_TD4150_BRIGHTNESS_ON));
	dsi_write(id, SEQ_TD4150_BRIGHTNESS_DEF, ARRAY_SIZE(SEQ_TD4150_BRIGHTNESS_DEF));
}

void td4150_lcd_disable(int id)
{
	if (dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE,
				SEQ_DISPLAY_OFF[0], 0) < 0)
		dsim_err("fail to send SEQ_DISPLAY_ON command.\n");
	mdelay(20);
}

void td4150_lcd_exit(unsigned int id, struct exynos_panel_info *lcd)
{
	if (dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE,
				SEQ_DISPLAY_OFF[0], 0) < 0)
		dsim_err("fail to send SEQ_DISPLAY_ON command.\n");
	mdelay(20);

	if (dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE,
				SEQ_SLEEP_IN[0], 0) < 0)
		dsim_err("fail to send SEQ_SLEEP_OUT command.\n");
	mdelay(100);
}

int td4150_lcd_gamma_ctrl(unsigned int id, unsigned int backlightlevel)
{
	return 0;
}

int td4150_lcd_gamma_update(int id)
{
	return 0;
}
