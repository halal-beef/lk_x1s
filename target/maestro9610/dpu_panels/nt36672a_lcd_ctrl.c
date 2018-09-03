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
#include "nt36672a_param.h"


#define GAMMA_PARAM_SIZE 26

#define NT36672A_VIDEO_VBP	2
#define NT36672A_VIDEO_VFP	10
#define NT36672A_VIDEO_VSA	10
#define NT36672A_VIDEO_HBP	20
#define NT36672A_VIDEO_HFP	40
#define NT36672A_VIDEO_HSA	40

#define NT36672A_HORIZONTAL	1080
#define NT36672A_VERTICAL	2246

struct decon_lcd nt36672a_lcd_info = {
	.mode = DECON_VIDEO_MODE,
	.vfp = NT36672A_VIDEO_VFP,
	.vbp = NT36672A_VIDEO_VBP,
	.hfp = NT36672A_VIDEO_HFP,
	.hbp = NT36672A_VIDEO_HBP,
	.vsa = NT36672A_VIDEO_VSA,
	.hsa = NT36672A_VIDEO_HSA,
	.xres = NT36672A_HORIZONTAL,
	.yres = NT36672A_VERTICAL,

	/* Mhz */
	.hs_clk = 1100,
	.esc_clk = 20,
	.dphy_pms = {1, 100, 1, 0}, /* pmsk */
	.vt_compensation = 14,	/* for underrun detect at video mode */

	/* Maybe, width and height will be removed */
	.width = 80,
	.height = 120,

	.fps = 60,
	.mic_enabled = 0,
	.mic_ver = 0,

	.dsc_enabled = 0,
	.dsc_slice_num = 0,
	.dsc_cnt = 0,
	.dsc_slice_h = 40,
	.data_lane = 4,
};

#if 0
struct decon_lcd *decon_get_lcd_info(void)
{
	return &nt36672a_lcd_info;
}
#endif


/*
 * NT36672A lcd init sequence
 *
 * Parameters
 *	- mic : if mic is enabled, MIC_ENABLE command must be sent
 *	- mode : LCD init sequence depends on command or video mode
 */
void nt36672a_lcd_init(unsigned int id, struct decon_lcd *lcd)
{
	mdelay(12);

	if (dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE_PARAM,
				SEQ_CMD_0[0],
				SEQ_CMD_0[1]) < 0)
		dsim_err("fail to send SEQ_CMD_0 command.\n");
	mdelay(12);

	if (dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE_PARAM,
				SEQ_CMD_1[0],
				SEQ_CMD_1[1]) < 0)
		dsim_err("fail to send SEQ_CMD_1 command.\n");
	mdelay(12);

	if (dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE_PARAM,
				SEQ_CMD_2[0],
				SEQ_CMD_2[1]) < 0)
		dsim_err("fail to send SEQ_CMD_2 command.\n");
	mdelay(12);

	if (dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE, SEQ_SLEEP_OUT[0], 0) < 0)
		dsim_err("fail to send SEQ_SLEEP_OUT command.\n");
	mdelay(120);
}

void nt36672a_lcd_enable_exynos(unsigned int id)
{
	if (dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE, SEQ_DISPLAY_ON[0], 0) < 0)
		dsim_err("fail to send SEQ_DISPLAY_ON command.\n");
}

void nt36672a_lcd_disable(int id)
{
	if (dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE, SEQ_DISPLAY_OFF[0], 0) < 0)
		dsim_err("fail to send SEQ_DISPLAY_OFF command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE, SEQ_SLEEP_IN[0], 0) < 0)
		dsim_err("fail to send SEQ_SLEEP_IN command.\n");
	mdelay(100);
}

/*
 * Set gamma values
 *
 * Parameter
 *	- backlightlevel : It is from 0 to 26.
 */
int nt36672a_lcd_gamma_ctrl(unsigned int id, unsigned int backlightlevel)
{
	return 0;
}

int nt36672a_lcd_gamma_update(int id)
{
	return 0;
}
