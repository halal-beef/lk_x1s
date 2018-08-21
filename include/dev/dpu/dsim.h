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

#ifndef __SAMSUNG_DSIM_H__
#define __SAMSUNG_DSIM_H__

#include <reg.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <platform/delay.h>

#include <dev/dpu/decon_lcd.h>
#include <dev/dpu/mipi_dsi_cmd.h>
#include <platform/display_sfr.h>
#include <platform/dpu_cal/regs-dsim.h>
#include <target/dpu_io_ctrl.h>

extern int dsim_log_level;
/* TODO : add variable dsim drvdata */
extern struct dsim_device *dsim0_for_decon;

#define DSIM_DDI_ID_LEN			3

#define DSIM_PIXEL_FORMAT_RGB24			0x3E
#define DSIM_PIXEL_FORMAT_RGB18_PACKED		0x1E
#define DSIM_PIXEL_FORMAT_RGB18			0x2E
#define DSIM_PIXEL_FORMAT_RGB30_PACKED		0x0D
#define DSIM_RX_FIFO_MAX_DEPTH		64
#define MAX_DSIM_CNT			2
#define MAX_DSIM_DATALANE_CNT		4

#define MIPI_WR_TIMEOUT			msecs_to_jiffies(50)
#define MIPI_RD_TIMEOUT			msecs_to_jiffies(100)

#define dsim_err(fmt, ...)							\
	do {									\
		if (dsim_log_level >= 3) {					\
			printf(fmt, ##__VA_ARGS__);			\
		}								\
	} while (0)

#define dsim_warn(fmt, ...)							\
	do {									\
		if (dsim_log_level >= 4) {					\
			printf(fmt, ##__VA_ARGS__);			\
		}								\
	} while (0)

#define dsim_info(fmt, ...)							\
	do {									\
		if (dsim_log_level >= 6)					\
			printf(fmt, ##__VA_ARGS__);			\
	} while (0)

#define dsim_dbg(fmt, ...)							\
	do {									\
		if (dsim_log_level >= 7)					\
			printf(fmt, ##__VA_ARGS__);			\
	} while (0)

#define call_panel_ops(q, op, args...)					\
	(((q)->panel_ops->op) ? ((q)->panel_ops->op(args)) : 0)

#define call_config_ops(q, op, args...)					\
	(((q)->config_ops->op) ? ((q)->config_ops->op(args)) :		\
	printf("%s: Operation("#op")isn't mapped\n", __func__))


/* define video timer interrupt */
enum {
	DSIM_VBP = 0,
	DSIM_VSYNC,
	DSIM_V_ACTIVE,
	DSIM_VFP,
};

/* define dsi bist pattern */
enum {
	DSIM_COLOR_BAR = 0,
	DSIM_GRAY_GRADATION,
	DSIM_USER_DEFINED,
	DSIM_PRB7_RANDOM,
};

/* define DSI lane types. */
enum {
	DSIM_LANE_CLOCK	= (1 << 0),
	DSIM_LANE_DATA0	= (1 << 1),
	DSIM_LANE_DATA1	= (1 << 2),
	DSIM_LANE_DATA2	= (1 << 3),
	DSIM_LANE_DATA3	= (1 << 4),
};

/* DSI Error report bit definitions */
enum {
	MIPI_DSI_ERR_SOT			= (1 << 0),
	MIPI_DSI_ERR_SOT_SYNC			= (1 << 1),
	MIPI_DSI_ERR_EOT_SYNC			= (1 << 2),
	MIPI_DSI_ERR_ESCAPE_MODE_ENTRY_CMD	= (1 << 3),
	MIPI_DSI_ERR_LOW_POWER_TRANSMIT_SYNC	= (1 << 4),
	MIPI_DSI_ERR_HS_RECEIVE_TIMEOUT		= (1 << 5),
	MIPI_DSI_ERR_FALSE_CONTROL		= (1 << 6),
	/* Bit 7 is reserved */
	MIPI_DSI_ERR_ECC_SINGLE_BIT		= (1 << 8),
	MIPI_DSI_ERR_ECC_MULTI_BIT		= (1 << 9),
	MIPI_DSI_ERR_CHECKSUM			= (1 << 10),
	MIPI_DSI_ERR_DATA_TYPE_NOT_RECOGNIZED	= (1 << 11),
	MIPI_DSI_ERR_VCHANNEL_ID_INVALID	= (1 << 12),
	MIPI_DSI_ERR_INVALID_TRANSMIT_LENGTH	= (1 << 13),
	/* Bit 14 is reserved */
	MIPI_DSI_ERR_PROTOCAL_VIOLATION		= (1 << 15),
	/* DSI_PROTOCAL_VIOLATION[15] is for protocol violation that is caused EoTp
	 * missing So this bit is egnored because of not supportung @S.LSI AP
	 */
	/* FALSE_ERROR_CONTROL[6] is for detect invalid escape or turnaround sequence.
	 * This bit is not supporting @S.LSI AP because of non standard
	 * ULPS enter/exit sequence during power-gating
	 */
	/* Bit [14],[7] is reserved */
	MIPI_DSI_ERR_BIT_MASK			= (0x3f3f), /* Error_Range[13:0] */
};

/* operation state of dsim driver */
enum dsim_state {
	DSIM_STATE_INIT,
	DSIM_STATE_ON,		/* HS clock was enabled. */
	DSIM_STATE_ULPS,	/* DSIM was entered ULPS state */
	DSIM_STATE_OFF		/* DSIM is suspend state */
};

enum dphy_charic_value {
	M_PLL_CTRL1,
	M_PLL_CTRL2,
	B_DPHY_CTRL2,
	B_DPHY_CTRL3,
	B_DPHY_CTRL4,
	M_DPHY_CTRL1,
	M_DPHY_CTRL2,
	M_DPHY_CTRL3,
	M_DPHY_CTRL4
};

struct dsim_pll_param {
	u32 p;
	u32 m;
	u32 s;
	u32 k;
	u32 pll_freq; /* in/out parameter: Mhz */
};

struct dsim_clks {
	u32 hs_clk;
	u32 esc_clk;
	u32 byte_clk;
	u32 word_clk;
};

struct dphy_timing_value {
	u32 bps;
	u32 clk_prepare;
	u32 clk_zero;
	u32 clk_post;
	u32 clk_trail;
	u32 hs_prepare;
	u32 hs_zero;
	u32 hs_trail;
	u32 lpx;
	u32 hs_exit;
	u32 b_dphyctl;
};

struct dsim_resources {
	struct clk *pclk;
	struct clk *dphy_esc;
	struct clk *dphy_byte;
	struct clk *rgb_vclk0;
	struct clk *pclk_disp;
	int lcd_power[2];
	int lcd_reset;
	int irq;
	u32 regs;
	u32 ss_regs;
	u32 phy_regs;
};

struct dsim_device {
	int id;
	enum dsim_state state;
	struct dsim_resources res;

	unsigned int data_lane;
	u32 data_lane_cnt;

	struct dsim_lcd_driver *panel_ops;
	struct lcd_driver *cm_panel_ops;
	struct decon_lcd *lcd_info;
	//struct decon_lcd lcd_info;

	struct dsim_clks clks;
	int total_underrun_cnt;

	struct dphy_timing_value timing;
	u32 reg_base;
	enum board_gpio_type board_type;
	struct exynos_display_config *config_ops;
};

/**
 * driver structure for mipi-dsi based lcd panel.
 *
 * this structure should be registered by lcd panel driver.
 * mipi-dsi driver seeks lcd panel registered through name field
 * and calls these callback functions in appropriate time.
 */
struct lcd_driver {
#define NUM_OF_VERIFIED_PANEL	4
	u32 panel_ids[NUM_OF_VERIFIED_PANEL];	/* unique ID for each panel */
	u32 id;					/* read ID */

	/* fill panel id to panel_ids arrary from panel driver each */
	int	(*fill_id)(struct dsim_device *dsim);
	/* read ddi's id(panel_ids[]) for matching expected id */
	int	(*read_id)(struct dsim_device *dsim);
	/* return panel_ops of matched panel driver */
	struct dsim_lcd_driver *(*get_panel_info)(struct dsim_device *dsim);
};


struct dsim_lcd_driver {
	int	(*get_id)(struct dsim_device *dsim);
	struct decon_lcd *(*get_lcd_info)(void);
	int	(*probe)(struct dsim_device *dsim);
	int	(*suspend)(struct dsim_device *dsim);
	int	(*displayon)(struct dsim_device *dsim);
	int	(*resume)(struct dsim_device *dsim);
};

int dsim_write_data(struct dsim_device *dsim, u32 id, unsigned long d0, u32 d1);
int dsim_read_data(struct dsim_device *dsim, u32 id, u32 addr, u32 cnt, u8 *buf);
int dsim_wait_for_cmd_done(struct dsim_device *dsim);

static inline struct dsim_device *get_dsim_drvdata(u32 id)
{
	return dsim0_for_decon;
}

static inline int dsim_rd_data(u32 id, u32 cmd_id, u32 addr, u32 size, u8 *buf)
{
	int ret;
	struct dsim_device *dsim = get_dsim_drvdata(id);

	ret = dsim_read_data(dsim, cmd_id, addr, size, buf);
	if (ret)
		return ret;

	return 0;
}

static inline int dsim_wr_data(u32 id, u32 cmd_id, unsigned long d0, u32 d1)
{
	int ret;
	struct dsim_device *dsim = get_dsim_drvdata(id);

	ret = dsim_write_data(dsim, cmd_id, d0, d1);
	if (ret)
		return ret;

	return 0;
}

static inline int dsim_wait_for_cmd_completion(u32 id)
{
	int ret;
	struct dsim_device *dsim = get_dsim_drvdata(id);

	ret = dsim_wait_for_cmd_done(dsim);

	return ret;
}

/* register access subroutines */
static inline u32 dsim_read(u32 id, u32 reg_id)
{
	return readl(DSIM0_BASE_ADDR + reg_id);
}

static inline u32 dsim_read_mask(u32 id, u32 reg_id, u32 mask)
{
	u32 val = dsim_read(id, reg_id);

	val &= (mask);
	return val;
}

static inline void dsim_write(u32 id, u32 reg_id, u32 val)
{
	writel(val, DSIM0_BASE_ADDR + reg_id);
}

static inline void dsim_write_mask(u32 id, u32 reg_id, u32 val, u32 mask)
{
	u32 old = dsim_read(id, reg_id);

	val = (val & mask) | (old & ~mask);
	writel(val, DSIM0_BASE_ADDR + reg_id);
}

/* DPHY register access subroutines */
static inline u32 dsim_phy_read(u32 id, u32 reg_id)
{
	return readl(DPHY_BASE_ADDR + reg_id);
}

static inline u32 dsim_phy_read_mask(u32 id, u32 reg_id, u32 mask)
{
	u32 val = dsim_phy_read(id, reg_id);

	val &= (mask);
	return val;
}
static inline void dsim_phy_write(u32 id, u32 reg_id, u32 val)
{
	writel(val, DPHY_BASE_ADDR + reg_id);
}

static inline void dsim_phy_write_mask(u32 id, u32 reg_id, u32 val, u32 mask)
{
	u32 old = dsim_phy_read(id, reg_id);

	val = (val & mask) | (old & ~mask);
	writel(val, DPHY_BASE_ADDR + reg_id);
}

/* FOR internel APIs list */
int dsim_reg_init(u32 id, struct decon_lcd *lcd_info,
			u32 data_lane_cnt, struct dsim_clks *clks);
int dsim_reg_set_clocks(u32 id, struct dsim_clks *clks,
			struct stdphy_pms *dphy_pms, u32 en);
int dsim_reg_set_lanes(u32 id, u32 lanes, u32 en);
void dsim_reg_set_int(u32 id, u32 en);
u32 dsim_reg_rx_fifo_is_empty(u32 id);
u32 dsim_reg_get_rx_fifo(u32 id);
int dsim_reg_rx_err_handler(u32 id, u32 rx_fifo);
void dsim_reg_sw_reset(u32 id);
void dsim_reg_dphy_reset(u32 id);
void dsim_reg_dphy_resetn(u32 id, u32 en);
int dsim_reg_exit_ulps_and_start(u32 id, u32 ddi_type, u32 lanes);
int dsim_reg_stop_and_enter_ulps(u32 id, u32 ddi_type, u32 lanes);
void dsim_reg_start(u32 id);
void dsim_reg_stop(u32 id, u32 lanes);
void dsim_reg_wr_tx_payload(u32 id, u32 payload);
u32 dsim_reg_header_fifo_is_empty(u32 id);
void dsim_reg_clear_int(u32 id, u32 int_src);
void dsim_reg_set_fifo_ctrl(u32 id, u32 cfg);
void dsim_reg_enable_shadow_read(u32 id, u32 en);
u32 dsim_reg_is_writable_fifo_state(u32 id);
void dsim_reg_wr_tx_header(u32 id, u32 data_id, unsigned long data0, u32 data1, u32 bta_type);
void dsim_set_bist(u32 id, u32 en);
void dsim_reg_set_partial_update(u32 id, struct decon_lcd *lcd_info);
void dsim_reg_set_bta_type(u32 id, u32 bta_type);
void dsim_reg_set_num_of_transfer(u32 id, u32 num_of_transfer);

void dsim_reg_function_reset(u32 id);
void dsim_reg_set_esc_clk_on_lane(u32 id, u32 en, u32 lane);
void dsim_reg_enable_word_clock(u32 id, u32 en);
void dsim_reg_set_esc_clk_prescaler(u32 id, u32 en, u32 p);
u32 dsim_reg_is_pll_stable(u32 id);
void dsim_reg_set_link_clock(u32 id, u32 en);
void dsim_reg_set_video_mode(u32 id, u32 mode);
void dsim_reg_enable_shadow(u32 id, u32 en);

/* FOR extern APIs list */
extern int dsim_probe(u32 dev_id);

#endif /* __SAMSUNG_DSIM_H__ */
