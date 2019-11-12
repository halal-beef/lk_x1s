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

#include <target/lcd_module.h>

#define GAMMA_PARAM_SIZE	26

#define S6E3FA0_CMD_VBP		10
#define S6E3FA0_CMD_VFP		3
#define S6E3FA0_CMD_VSA		1
#define S6E3FA0_CMD_HBP		2
#define S6E3FA0_CMD_HFP		2
#define S6E3FA0_CMD_HSA		2

#define S6E3FA0_HORIZONTAL	1080
#define S6E3FA0_VERTICAL	1920

//#define CONFIG_DECON_LCD_VIDEO_MODE

struct exynos_panel_info common_lcd_info = {
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
	.hs_clk = 898,
	.esc_clk = 20,

	.dphy_pms = {2, 276, 2, 20165}, /* pmsk */
	.cmd_underrun_cnt = {583},
	/* Maybe, width and height will be removed */
	.width = 70,
	.height = 121,

	.fps = 60,
//	.mic_enabled = 0,
//	.mic_ver = 0,

	.dsc = {0, 0, 0, 40, 720, 240},
//	.dsc_enabled = 1,
//	.dsc_cnt = 1,
//	.dsc_slice_num = 2,
//	.dsc_slice_h = 40,
	.data_lane = 4,
};

struct exynos_panel_info *common_get_lcd_info(void)
{
	return &common_lcd_info;
}

extern struct dsim_lcd_driver s6e3fa0_mipi_lcd_driver;
extern struct dsim_lcd_driver nt36672a_mipi_lcd_driver;
extern struct dsim_lcd_driver s6e3ha8_mipi_lcd_driver;
extern struct dsim_lcd_driver s6e3ha9_mipi_lcd_driver;

struct dsim_lcd_driver *panel_list[NUM_OF_VERIFIED_PANEL] = {
	&s6e3fa0_mipi_lcd_driver,
	&nt36672a_mipi_lcd_driver,
	&s6e3ha8_mipi_lcd_driver,
	&s6e3ha9_mipi_lcd_driver,
};

/* fill panel id to panel_ids arrary from panel driver each */
int cm_fill_id(struct dsim_device *dsim)
{
	int i;

	for (i = 0; i < NUM_OF_VERIFIED_PANEL; i++) {
		if (panel_list[i] == NULL)
			break;
		dsim->cm_panel_ops->panel_ids[i] = panel_list[i]->get_id(dsim);
	}

	return 0;
}

u32 ddi_id;
/* read ddi's id(panel_ids[]) for matching expected id */
int cm_read_id(struct dsim_device *dsim)
{
	int err = 0;
	u32 id = 0, i;

	u8 buf[DSIM_DDI_ID_LEN] = {0, };

	/* dsim sends the request for the lcd id and gets it buffer */
	err = dsim_read_data(dsim, MIPI_DSI_DCS_READ,
		MIPI_DCS_GET_DISPLAY_ID, DSIM_DDI_ID_LEN, buf);
	if (err < 0) {
		printf("Failed to read panel id!\n");
		return -EINVAL;
	} else {
		for (i = 0; i < DSIM_DDI_ID_LEN; i++) {
			//id |= buf[i] << (24 - i * 8);	/* LSB is left */
			id |= buf[i] << (i * 8);	/* LSB is right */
			printf("id : 0x%06x\n", id);
		}

		/*
		 * [ HACK : 2019-06-08 ]
		 * use same value with PANEL_ID of [s6e3ha8_mipi_lcd.c]
		 * remove following constant id value if value is confirmed
		 */
		//id = 0x430491;

		printf("Suceeded to read panel id : 0x%06x\n", id);
		ddi_id = id;
	}

	return dsim->cm_panel_ops->id = id;
}

/* return panel_ops of matched panel driver */
struct dsim_lcd_driver *cm_get_panel_info(struct dsim_device *dsim)
{
	int i;
	int pre_defined_id = 0;
	int read_id = 0;

	cm_read_id(dsim);

	read_id = dsim->cm_panel_ops->id & 0xff00ff;
	for (i = 0; i < NUM_OF_VERIFIED_PANEL; i++) {
		if (dsim->cm_panel_ops->panel_ids[i] == 0)
			break;
		else {
			pre_defined_id = dsim->cm_panel_ops->panel_ids[i] & 0xff00ff;
			if (pre_defined_id == read_id)
				return panel_list[i];
		}
	}

	return NULL;
}

struct lcd_driver common_lcd_driver = {
	.fill_id	= cm_fill_id,
	.read_id	= cm_read_id,
	.get_panel_info	= cm_get_panel_info,
};

struct lcd_driver *get_lcd_drv_ops(void)
{
	return &common_lcd_driver;
}

