/*
 * drivers/video/decon/panels/s6e3ha9_lcd_ctrl.c
 *
 * Samsung SoC MIPI LCD CONTROL functions
 *
 * Copyright (c) 2014 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <dpu/lcd_ctrl.h>
#include <dpu/mipi_dsi_cmd.h>
#include <dpu/dsim.h>
#include "s6e3ha9_param.h"

/* Porch values. It depends on command or video mode */
#define S6E3HA9_CMD_VBP	15
#define S6E3HA9_CMD_VFP	1
#define S6E3HA9_CMD_VSA	1
#define S6E3HA9_CMD_HBP	1
#define S6E3HA9_CMD_HFP	1
#define S6E3HA9_CMD_HSA	1

/* These need to define */
#define S6E3HA9_VIDEO_VBP	15
#define S6E3HA9_VIDEO_VFP	1
#define S6E3HA9_VIDEO_VSA	1
#define S6E3HA9_VIDEO_HBP	20
#define S6E3HA9_VIDEO_HFP	20
#define S6E3HA9_VIDEO_HSA	20

#define S6E3HA9_HORIZONTAL	1440
#define S6E3HA9_VERTICAL	3040

#define CONFIG_FB_I80_COMMAND_MODE

struct exynos_panel_info s6e3ha9_lcd_info = {
	.mode = DECON_MIPI_COMMAND_MODE,
	.vfp = S6E3HA9_CMD_VFP,
	.vbp = S6E3HA9_CMD_VBP,
	.hfp = S6E3HA9_CMD_HFP,
	.hbp = S6E3HA9_CMD_HBP,
	.vsa = S6E3HA9_CMD_VSA,
	.hsa = S6E3HA9_CMD_HSA,
	.xres = S6E3HA9_HORIZONTAL,
	.yres = S6E3HA9_VERTICAL,

	/* Maybe, width and height will be removed */
	.width = 70,
	.height = 121,

	/* Mhz */
	.hs_clk = 898,
	.esc_clk = 20,


	.dphy_pms = {2, 138, 2, 0x2762}, /* pmsk */
	.cmd_underrun_cnt = {3022},

	/* Maybe, width and height will be removed */
	.fps = 60,
	.data_lane = 4,

	.dsc = {1, 1, 2, 40, 720, 240},
};

/*
 * 3HA9 lcd init sequence
 *
 * Parameters
 *	- mic_enabled : if mic is enabled, MIC_ENABLE command must be sent
 *	- mode : LCD init sequence depends on command or video mode
 */

void s6e3ha9_lcd_init(unsigned int id, struct exynos_panel_info *lcd)
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

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)SEQ_TSP_HSYNC,
				ARRAY_SIZE(SEQ_TSP_HSYNC)) < 0)
		dsim_err("fail to write SEQ_TSP_HSYNC command.\n");

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

	dsim_dbg("%s -\n", __func__);
}

void s6e3ha9_lcd_enable_exynos(unsigned int id)
{
	if (dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE,
				(unsigned long)SEQ_DISPLAY_ON[0], 0) < 0)
		dsim_err("fail to send SEQ_DISPLAY_ON command.\n");
}

void s6e3ha9_lcd_disable(unsigned int id)
{
	/* This function needs to implement */
}

/*
 * Set gamma values
 *
 * Parameter
 *	- backlightlevel : It is from 0 to 26.
 */
int s6e3ha9_lcd_gamma_ctrl(unsigned int id, unsigned int backlightlevel)
{
	/* This will be implemented */
	return 0;
}

int s6e3ha9_lcd_gamma_update(int id)
{
	/* This will be implemented */
	return 0;
}

int dsim_write_by_panel(int id, const u8 *cmd, u32 cmdSize)
{
	int ret;

	if (cmdSize == 1)
		ret = dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE, cmd[0], 0);
	else if (cmdSize == 2)
		ret = dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE_PARAM, cmd[0], cmd[1]);
	else
		ret = dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)cmd, cmdSize);

	return ret;
}

int dsim_read_from_panel(int id, u8 addr, u32 size, u8 *buf)
{
	int ret;

	ret = dsim_rd_data(id, MIPI_DSI_DCS_READ, (u32)addr, size, buf);

	return ret;
}

static int s6e3ha9_wqhd_dump(int dsim)
{
	int ret = 0;

	dsim_info(" + %s\n", __func__);
	dsim_info(" - %s\n", __func__);
	return ret;

}

int s6e3ha9_lcd_dump(int id)
{
	s6e3ha9_wqhd_dump(id);
	return 0;
}

