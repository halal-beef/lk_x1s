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
#include "ea8076_param.h"


#define GAMMA_PARAM_SIZE 26

#define EA8076_CMD_VBP		15
#define EA8076_CMD_VFP		3
#define EA8076_CMD_VSA		1
#define EA8076_CMD_HBP		2
#define EA8076_CMD_HFP		2
#define EA8076_CMD_HSA		2

/* To be Modified */
#define EA8076_VDO_VBP		2
#define EA8076_VDO_VFP		20
#define EA8076_VDO_VSA		2
#define EA8076_VDO_HBP		20
#define EA8076_VDO_HFP		20
#define EA8076_VDO_HSA		20

#define EA8076_HORIZONTAL	1080
#define EA8076_VERTICAL		2340

//#define CONFIG_DECON_LCD_VIDEO_MODE

struct exynos_panel_info ea8076_lcd_info = {
#if defined(CONFIG_DECON_LCD_VIDEO_MODE)
	.mode = DECON_VIDEO_MODE,
	.vfp = EA8076_VDO_VFP,
	.vbp = EA8076_VDO_VBP,
	.hfp = EA8076_VDO_HFP,
	.hbp = EA8076_VDO_HBP,
	.vsa = EA8076_VDO_VSA,
	.hsa = EA8076_VDO_HSA,
	.xres = EA8076_HORIZONTAL,
	.yres = EA8076_VERTICAL,

	/* Mhz */
	.hs_clk = 1200,
	.esc_clk = 16,
	.dphy_pms = {6, 677, 2, 0}, /* pmsk */
	.vt_compensation = 39,	/* for underrun detect at video mode */
#else
	.mode = DECON_MIPI_COMMAND_MODE,
	.vfp = EA8076_CMD_VFP,
	.vbp = EA8076_CMD_VBP,
	.hfp = EA8076_CMD_HFP,
	.hbp = EA8076_CMD_HBP,
	.vsa = EA8076_CMD_VSA,
	.hsa = EA8076_CMD_HSA,
	.xres = EA8076_HORIZONTAL,
	.yres = EA8076_VERTICAL,

	/* Mhz */
	.hs_clk = 1200,
	.esc_clk = 16,

	.dphy_pms = {2, 185, 1, 0x9D8A}, /* pmsk */
	.cmd_underrun_cnt = {1695, },
#endif
	/* Maybe, width and height will be removed */
	.width = 68,
	.height = 147,

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

void ea8076_lcd_init(unsigned int id, struct exynos_panel_info *lcd)
{
	/* Sleep Out (11h) */
	if (dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE,
				SEQ_SLEEP_OUT[0], 0) < 0)
		dsim_err("fail to send SEQ_SLEEP_OUT command.\n");
	mdelay(10);

	/* Common Setting */
	/* Testkey Enable */
	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE,
				(unsigned long)SEQ_TEST_KEY_ON_F0,
				ARRAY_SIZE(SEQ_TEST_KEY_ON_F0)) < 0)
		dsim_err("failed to send SEQ_TEST_KEY_ON_F0 command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE,
				(unsigned long)SEQ_TEST_KEY_ON_FC,
				ARRAY_SIZE(SEQ_TEST_KEY_ON_FC)) < 0)
		dsim_err("failed to send SEQ_TEST_KEY_ON_FC command.\n");

	/* Ignoring dealing with ERR in DDI IC
	 *  This is setting for ignoring dealing with error in DDI IC
	 * Without this setting, Displaying is not normal.
	 * so DDI vendor recommended us to use this setting temporally.
	 * After launching official project, DDI vendor need to check this again.
	 */
	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE,
				(unsigned long)SEQ_ERR_IGNORE,
				ARRAY_SIZE(SEQ_ERR_IGNORE)) < 0)
		dsim_err("failed to send SEQ_ERR_IGNORE command.\n");

	/* TE ON */
	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE,
				(unsigned long)SEQ_TE_ON,
				ARRAY_SIZE(SEQ_TE_ON)) < 0)
		dsim_err("fail to send SEQ_TE_ON command.\n");

	/* PAGE ADDRESS SET */
	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE,
				(unsigned long)SEQ_PAGE_ADDRESS_SET,
				ARRAY_SIZE(SEQ_PAGE_ADDRESS_SET)) < 0)
		dsim_err("fail to send SEQ_PAGE_ADDRESS_SET command.\n");


	/* FFC SET */
	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE,
				(unsigned long)SEQ_TEST_KEY_ON_FC,
				ARRAY_SIZE(SEQ_TEST_KEY_ON_FC)) < 0)
		dsim_err("fail to send SEQ_TEST_KEY_ON_FC command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE,
				(unsigned long)SEQ_FFC_SET,
				ARRAY_SIZE(SEQ_FFC_SET)) < 0)
		dsim_err("fail to send SEQ_FFC_SET command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE,
				(unsigned long)SEQ_TEST_KEY_OFF_FC,
				ARRAY_SIZE(SEQ_TEST_KEY_OFF_FC)) < 0)
		dsim_err("fail to send SEQ_TEST_KEY_OFF_FC command.\n");

	/* Brightness Setting */
	/* HBM OFF */
	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE,
				(unsigned long)SEQ_BCTRL_ON,
				ARRAY_SIZE(SEQ_BCTRL_ON)) < 0)
		dsim_err("fail to send SEQ_BCTRL_ON command.\n");

	/* Brightness */
	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE,
				(unsigned long)SEQ_DFT_BRIGHTNESS,
				ARRAY_SIZE(SEQ_DFT_BRIGHTNESS)) < 0)
		dsim_err("fail to send SEQ_DFT_BRIGHTNESS command.\n");

	/* ACL OFF */
	if (dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE_PARAM,
				SEQ_ACL_OFF[0],
				SEQ_ACL_OFF[1]) < 0)
		dsim_err("fail to send SEQ_ACL_OFF command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE,
				(unsigned long)SEQ_TEST_KEY_OFF_F0,
				ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0)) < 0)
		dsim_err("failed to send SEQ_TEST_KEY_OFF_F0 command.\n");

	mdelay(110);
}

void ea8076_lcd_enable_exynos(unsigned int id)
{
	if (dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE,
				SEQ_DISPLAY_ON[0], 0) < 0)
		dsim_err("fail to send SEQ_DISPLAY_ON command.\n");
}

void ea8076_lcd_disable(int id)
{
	/* This function needs to implement */
}

int ea8076_lcd_gamma_ctrl(unsigned int id, unsigned int backlightlevel)
{
	return 0;
}

int ea8076_lcd_gamma_update(int id)
{
	return 0;
}
