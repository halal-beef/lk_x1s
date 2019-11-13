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
#include "s6e3fa9octa_param.h"


#define S6E3FA9OCTA_CMD_VBP		15
#define S6E3FA9OCTA_CMD_VFP		3
#define S6E3FA9OCTA_CMD_VSA		1
#define S6E3FA9OCTA_CMD_HBP		2
#define S6E3FA9OCTA_CMD_HFP		2
#define S6E3FA9OCTA_CMD_HSA		2

#define S6E3FA9OCTA_HORIZONTAL	1080
#define S6E3FA9OCTA_VERTICAL	2400

#define CONFIG_DECON_LCD_CMD_MODE

struct exynos_panel_info s6e3fa9octa_lcd_info = {
	.mode = DECON_MIPI_COMMAND_MODE,
	.vfp = S6E3FA9OCTA_CMD_VFP,
	.vbp = S6E3FA9OCTA_CMD_VBP,
	.hfp = S6E3FA9OCTA_CMD_HFP,
	.hbp = S6E3FA9OCTA_CMD_HBP,
	.vsa = S6E3FA9OCTA_CMD_VSA,
	.hsa = S6E3FA9OCTA_CMD_HSA,
	.xres = S6E3FA9OCTA_HORIZONTAL,
	.yres = S6E3FA9OCTA_VERTICAL,

	/* Mhz */
	.hs_clk = 1176,
	.esc_clk = 16,

	.dphy_pms = {2, 181, 2, 0}, /* pmsk */
	.cmd_underrun_cnt = {1653},

	/* Maybe, width and height will be removed */
	.width = 70,
	.height = 121,

	.fps = 60,

	.dsc = {0, 0, 0, 40, 720, 240},
//	.dsc_enabled = 1,
//	.dsc_cnt = 1,
//	.dsc_slice_num = 2,
//	.dsc_slice_h = 40,
//	.dsc_dec_sw = 720;
//	.dsc_enc_sw = 240;
	.data_lane = 4,
};

#if 0
struct exynos_panel_info *decon_get_lcd_info(void)
{
	return &s6e3fa9octa_lcd_info;
}
#endif

void s6e3fa9octa_lcd_init(unsigned int id, struct exynos_panel_info *lcd)
{
	int ret = 0;
	u32 buf = 0;

	dsim_dbg("%s +\n", __func__);

	/* 6. Sleep Out(11h) */
	/* 7. Wait 10ms */
	if (dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE,
				(unsigned long)SEQ_SLEEP_OUT[0], 0) < 0)
		dsim_err("fail to send SEQ_SLEEP_OUT command.\n");

	mdelay(10);

	/* ID READ */
	ret = dsim_rd_data(id, MIPI_DSI_DCS_READ,
			MIPI_DCS_GET_DISPLAY_ID, DSIM_DDI_ID_LEN, (u8 *)&buf);
	if (ret < 0)
		dsim_err("failed to read panel id(%d)\n", ret);
	else
		dsim_info("suceeded to read panel id : 0x%06x\n", buf);

	/* 8. Common Setting */
	/* Testkey Enable */
	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)SEQ_TEST_KEY_ON_F0,
				ARRAY_SIZE(SEQ_TEST_KEY_ON_F0)) < 0)
		dsim_err("fail to write SEQ_TEST_KEY_ON_F0 command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)SEQ_TEST_KEY_ON_FC,
				ARRAY_SIZE(SEQ_TEST_KEY_ON_FC)) < 0)
		dsim_err("fail to write SEQ_TEST_KEY_ON_FC command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)SEQ_PAGE_ADDR_SET_2A,
				ARRAY_SIZE(SEQ_PAGE_ADDR_SET_2A)) < 0)
		dsim_err("fail to write SEQ_PAGE_ADDR_SET_2A command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)SEQ_PAGE_ADDR_SET_2B,
				ARRAY_SIZE(SEQ_PAGE_ADDR_SET_2B)) < 0)
		dsim_err("fail to write SEQ_PAGE_ADDR_SET_2B command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)SEQ_TSP_VSYNC_ON,
				ARRAY_SIZE(SEQ_TSP_VSYNC_ON)) < 0)
		dsim_err("fail to write SEQ_TSP_VSYNC_ON command.\n");

	/* 10. Brightness Setting */
	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)SEQ_HBM_OFF,
				ARRAY_SIZE(SEQ_HBM_OFF)) < 0)
		dsim_err("fail to write SEQ_HBM_OFF command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)SEQ_ELVSS_SET,
				ARRAY_SIZE(SEQ_ELVSS_SET)) < 0)
		dsim_err("fail to write SEQ_ELVSS_SET command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)SEQ_BRIGHTNESS,
				ARRAY_SIZE(SEQ_BRIGHTNESS)) < 0)
		dsim_err("fail to write SEQ_BRIGHTNESS command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)SEQ_ACL_OFF,
				ARRAY_SIZE(SEQ_ACL_OFF)) < 0)
		dsim_err("fail to write SEQ_ACL_OFF command.\n");

	/* Testkey Disable */
	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)SEQ_TEST_KEY_OFF_F0,
				ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0)) < 0)
		dsim_err("fail to write SEQ_TEST_KEY_OFF_F0 command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)SEQ_TEST_KEY_OFF_FC,
				ARRAY_SIZE(SEQ_TEST_KEY_OFF_FC)) < 0)
		dsim_err("fail to write SEQ_TEST_KEY_ON_FC command.\n");

	/* 8.1.1 TE(Vsync) ON/OFF */
	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)SEQ_TEST_KEY_ON_F0,
				ARRAY_SIZE(SEQ_TEST_KEY_ON_F0)) < 0)
		dsim_err("fail to write SEQ_TEST_KEY_ON_F0 command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)SEQ_TE_ON,
				ARRAY_SIZE(SEQ_TE_ON)) < 0)
		dsim_err("fail to write SEQ_TE_ON command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)SEQ_TEST_KEY_OFF_F0,
				ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0)) < 0)
		dsim_err("fail to write SEQ_TEST_KEY_OFF_F0 command.\n");

	/* 11. Wait 110ms */
	mdelay(110);

	dsim_dbg("%s -\n", __func__);
}

void s6e3fa9octa_lcd_enable_exynos(unsigned int id)
{
	if (dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE,
				(unsigned long)SEQ_DISPLAY_ON[0], 0) < 0)
		dsim_err("fail to send SEQ_DISPLAY_ON command.\n");
}

void s6e3fa9octa_lcd_disable(int id)
{
	/* This function needs to implement */
}

int s6e3fa9octa_lcd_gamma_ctrl(unsigned int id, unsigned int backlightlevel)
{
	return 0;
}

int s6e3fa9octa_lcd_gamma_update(int id)
{
	return 0;
}

