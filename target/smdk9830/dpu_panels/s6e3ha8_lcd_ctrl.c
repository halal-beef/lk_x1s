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
#include <dev/dpu/lcd_ctrl.h>
#include <dev/dpu/mipi_dsi_cmd.h>
#include <dev/dpu/dsim.h>
#include "s6e3ha8_param.h"


#define S6E3HA8_CMD_VBP		15
#define S6E3HA8_CMD_VFP		8
#define S6E3HA8_CMD_VSA		1
#define S6E3HA8_CMD_HBP		2
#define S6E3HA8_CMD_HFP		2
#define S6E3HA8_CMD_HSA		2

#define S6E3HA8_HORIZONTAL	1440
#define S6E3HA8_VERTICAL	2960

#define CONFIG_DECON_LCD_CMD_MODE

struct exynos_panel_info s6e3ha8_lcd_info = {
	.mode = DECON_MIPI_COMMAND_MODE,
	.vfp = S6E3HA8_CMD_VFP,
	.vbp = S6E3HA8_CMD_VBP,
	.hfp = S6E3HA8_CMD_HFP,
	.hbp = S6E3HA8_CMD_HBP,
	.vsa = S6E3HA8_CMD_VSA,
	.hsa = S6E3HA8_CMD_HSA,
	.xres = S6E3HA8_HORIZONTAL,
	.yres = S6E3HA8_VERTICAL,

	/* Mhz */
	.hs_clk = 898,
	.esc_clk = 20,

	.dphy_pms = {2, 138, 2, 0x2762}, /* pmsk */
	.cmd_underrun_cnt = {3022},

	/* Maybe, width and height will be removed */
	.width = 70,
	.height = 121,

	.fps = 60,

	.dsc = {1, 2, 2, 40, 720, 240},
//	.dsc_enabled = 1,
//	.dsc_slice_num = 2,
//	.dsc_cnt = 2,
//	.dsc_slice_h = 40,
//	.dsc_enc_sw = 240;
//	.dsc_dec_sw = 720;
	.data_lane = 4,
};

#if 0
struct exynos_panel_info *decon_get_lcd_info(void)
{
	return &s6e3ha8_lcd_info;
}
#endif

void s6e3ha8_lcd_init(unsigned int id, struct exynos_panel_info *lcd)
{
	dsim_dbg("%s +\n", __func__);

	mdelay(5);

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)SEQ_TEST_KEY_ON_F0,
				ARRAY_SIZE(SEQ_TEST_KEY_ON_F0)) < 0)
		dsim_err("fail to write SEQ_TEST_KEY_ON_F0 command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)SEQ_TEST_KEY_ON_FC,
				ARRAY_SIZE(SEQ_TEST_KEY_ON_FC)) < 0)
		dsim_err("fail to write SEQ_TEST_KEY_ON_FC command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DSC_PRA, (unsigned long)SEQ_DSC_EN[0], 0) < 0)
		dsim_err("fail to write SEQ_DSC_EN command.\n");

	switch (lcd->dsc.slice_num) {
	case 2:
		if (dsim_wr_data(id, MIPI_DSI_DSC_PPS, (unsigned long)SEQ_PPS_SLICE2,
					ARRAY_SIZE(SEQ_PPS_SLICE2)) < 0)
			dsim_err("fail to write SEQ_PPS_SLICE2 command.\n");
		break;
	default:
		dsim_err("fail to set MIPI_DSI_DSC_PPS command(no slice).\n");
		break;
	}

	if (dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE,
				(unsigned long)SEQ_SLEEP_OUT[0], 0) < 0)
		dsim_err("fail to send SEQ_SLEEP_OUT command.\n");

	mdelay(120);

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE,
			(unsigned long)SEQ_TSP_HSYNC, ARRAY_SIZE(SEQ_TSP_HSYNC)) < 0)
		dsim_err("fail to write SEQ_TSP_HSYNC command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)SEQ_SET_AREA,
				ARRAY_SIZE(SEQ_SET_AREA)) < 0)
		dsim_err("fail to write SEQ_SET_AREA command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE,
				(unsigned long)SEQ_TE_ON[0], 0) < 0)
		dsim_err("fail to send SEQ_TE_ON command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE_PARAM,
				(unsigned long)SEQ_ERR_FG[0], (u32)SEQ_ERR_FG[1]) < 0)
		dsim_err("fail to send SEQ_ERR_FG command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)SEQ_TE_START_SETTING,
				ARRAY_SIZE(SEQ_TE_START_SETTING)) < 0)
		dsim_err("fail to write SEQ_TE_START_SETTING command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)SEQ_FFC,
				ARRAY_SIZE(SEQ_FFC)) < 0)
		dsim_err("fail to write SEQ_FFC command.\n");

	/* enable and set brightness */
	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE,
				(unsigned long)SEQ_SET_WRITE_CTRL_DISP,
				ARRAY_SIZE(SEQ_SET_WRITE_CTRL_DISP)) < 0)
		dsim_err("fail to send SEQ_SET_WRITE_CTRL_DISP command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE,
				(unsigned long)SEQ_SET_BRIGHTNESS,
				ARRAY_SIZE(SEQ_SET_BRIGHTNESS)) < 0)
		dsim_err("fail to send SEQ_SET_BRIGHTNESS command.\n");

	dsim_dbg("%s -\n", __func__);
}

void s6e3ha8_lcd_enable_exynos(unsigned int id)
{
	if (dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE,
				(unsigned long)SEQ_DISPLAY_ON[0], 0) < 0)
		dsim_err("fail to send SEQ_DISPLAY_ON command.\n");
}

void s6e3ha8_lcd_disable(int id)
{
	/* This function needs to implement */
}

int s6e3ha8_lcd_gamma_ctrl(unsigned int id, unsigned int backlightlevel)
{
	return 0;
}

int s6e3ha8_lcd_gamma_update(int id)
{
	return 0;
}

