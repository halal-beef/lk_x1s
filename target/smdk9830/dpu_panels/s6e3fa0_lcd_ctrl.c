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
#include "s6e3fa0_gamma.h"
#include "s6e3fa0_param.h"


#define GAMMA_PARAM_SIZE 26

#define S6E3FA0_CMD_VBP		10
#define S6E3FA0_CMD_VFP		3
#define S6E3FA0_CMD_VSA		1
#define S6E3FA0_CMD_HBP		2
#define S6E3FA0_CMD_HFP		2
#define S6E3FA0_CMD_HSA		2

#define S6E3FA0_VDO_VBP		2
#define S6E3FA0_VDO_VFP		20
#define S6E3FA0_VDO_VSA		2
#define S6E3FA0_VDO_HBP		20
#define S6E3FA0_VDO_HFP		20
#define S6E3FA0_VDO_HSA		20

#define S6E3FA0_HORIZONTAL	1080
#define S6E3FA0_VERTICAL	1920

#define CONFIG_DECON_LCD_VIDEO_MODE

struct exynos_panel_info s6e3fa0_lcd_info = {
#if defined(CONFIG_DECON_LCD_VIDEO_MODE)
	.mode = DECON_VIDEO_MODE,
	.vfp = S6E3FA0_VDO_VFP,
	.vbp = S6E3FA0_VDO_VBP,
	.hfp = S6E3FA0_VDO_HFP,
	.hbp = S6E3FA0_VDO_HBP,
	.vsa = S6E3FA0_VDO_VSA,
	.hsa = S6E3FA0_VDO_HSA,
	.xres = S6E3FA0_HORIZONTAL,
	.yres = S6E3FA0_VERTICAL,

	/* Mhz */
	.hs_clk = 1100,
	.esc_clk = 20,
	.dphy_pms = {6, 677, 2, 0}, /* pmsk */
	.vt_compensation = 39,	/* for underrun detect at video mode */
#else
	.mode = DECON_MIPI_COMMAND_MODE,
	.vfp = S6E3FA0_CMD_VFP,
	.vbp = S6E3FA0_CMD_VBP,
	.hfp = S6E3FA0_CMD_HFP,
	.hbp = S6E3FA0_CMD_HBP,
	.vsa = S6E3FA0_CMD_VSA,
	.hsa = S6E3FA0_CMD_HSA,
	.xres = S6E3FA0_HORIZONTAL,
	.yres = S6E3FA0_VERTICAL,

	/* Mhz */
	.hs_clk = 1100,
	.esc_clk = 20,

	.dphy_pms.p = 3,
	.dphy_pms.m = 127,
	.dphy_pms.s = 0,
	.cmd_underrun_cnt = {3022, },
#endif
	/* Maybe, width and height will be removed */
	.width = 70,
	.height = 121,

	.fps = 60,
//	.mic_enabled = 0,
//	.mic_ver = 0,

	.dsc = {0, 0, 0, 40, 720, 240},
//	.dsc_enabled = 0,
//	.dsc_slice_num = 0,
//	.dsc_cnt = 0,
//	.dsc_slice_h = 40,
	.data_lane = 4,
};

#if 0
struct exynos_panel_info *decon_get_lcd_info(void)
{
	return &s6e3fa0_lcd_info;
}
#endif

void s6e3fa0_lcd_init(unsigned int id, struct exynos_panel_info *lcd)
{
	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE,
				(unsigned long)SEQ_TEST_KEY_ON_F0,
				ARRAY_SIZE(SEQ_TEST_KEY_ON_F0)) < 0)
		dsim_err("failed to send SEQ_TEST_KEY_ON_F0 command.\n");
	udelay(12);

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE,
				(unsigned long)SEQ_TEST_KEY_ON_F1,
				ARRAY_SIZE(SEQ_TEST_KEY_ON_F1)) < 0)
		dsim_err("fail to send SEQ_TEST_KEY_ON_F1 command.\n");
	mdelay(12);

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE,
				(unsigned long)SEQ_TEST_KEY_ON_FC,
				ARRAY_SIZE(SEQ_TEST_KEY_ON_FC)) < 0)
		dsim_err("fail to send SEQ_TEST_KEY_ON_FC command.\n");
	mdelay(12);

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE,
				(unsigned long)SEQ_TEST_KEY_ON_ED,
				ARRAY_SIZE(SEQ_TEST_KEY_ON_ED)) < 0)
		dsim_err("fail to send SEQ_TEST_KEY_ON_ED command.\n");
	mdelay(12);


	if (lcd->mode == DECON_MIPI_COMMAND_MODE) {
		if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE,
				(unsigned long)SEQ_TEST_KEY_ON_FD,
				ARRAY_SIZE(SEQ_TEST_KEY_ON_FD)) < 0)
			dsim_err(
			"fail to send SEQ_TEST_KEY_ON_FD command.\n");
		mdelay(12);

		if (dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE_PARAM,
					SEQ_TEST_KEY_ON_F6[0],
					SEQ_TEST_KEY_ON_F6[1]) < 0)
			dsim_err(
			"fail to send SEQ_TEST_KEY_ON_F6 command.\n");
		mdelay(12);
	} else {
		if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE,
				(unsigned long)SEQ_TEST_KEY_ON_E7,
				ARRAY_SIZE(SEQ_TEST_KEY_ON_E7)) < 0)
			dsim_err("fail to send SEQ_TEST_KEY_ON_E7 command.\n");
		mdelay( 120);
	}

	if (dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE,
				SEQ_SLEEP_OUT[0], 0) < 0)
		dsim_err("fail to send SEQ_SLEEP_OUT command.\n");
	mdelay(20);

	if (lcd->mode == DECON_VIDEO_MODE) {
		if (dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE_PARAM,
					SEQ_TEST_KEY_ON_F2[0],
					SEQ_TEST_KEY_ON_F2[1]) < 0)
			dsim_err("fail to send SEQ_TEST_KEY_ON_F2 command.\n");
		mdelay( 12);
	}

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE,
				(unsigned long)SEQ_TEST_KEY_ON_EB,
				ARRAY_SIZE(SEQ_TEST_KEY_ON_EB)) < 0)
		dsim_err("fail to send SEQ_TEST_KEY_ON_EB command.\n");
	mdelay(12);

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE,
				(unsigned long)SEQ_TEST_KEY_ON_C0,
				ARRAY_SIZE(SEQ_TEST_KEY_ON_C0)) < 0)
		dsim_err("fail to send SEQ_TEST_KEY_ON_C0 command.\n");
	mdelay(12);

	if (lcd->mode == DECON_MIPI_COMMAND_MODE) {
		if (dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE,
				SEQ_TE_ON[0], 0) < 0)
			dsim_err("fail to send SEQ_TE_ON command.\n");
		mdelay(12);
	}
}

void s6e3fa0_lcd_enable_exynos(unsigned int id)
{
	if (dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE,
			SEQ_DISPLAY_ON[0], 0) < 0)
		dsim_err("fail to send SEQ_DISPLAY_ON command.\n");
}

void s6e3fa0_lcd_disable(int id)
{
	/* This function needs to implement */
}

int s6e3fa0_lcd_gamma_ctrl(unsigned int id, unsigned int backlightlevel)
{
	int ret;

	ret = dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE,
			(unsigned long)gamma22_table[backlightlevel],
			GAMMA_PARAM_SIZE);
	if (ret)
		dsim_err("failed to write gamma value.\n");

	return ret;
}

int s6e3fa0_lcd_gamma_update(int id)
{
	int ret;

	ret = dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE,
			(unsigned long)SEQ_GAMMA_UPDATE,
			ARRAY_SIZE(SEQ_GAMMA_UPDATE));
	if (ret)
		dsim_err("failed to update gamma value.\n");

	return ret;
}
