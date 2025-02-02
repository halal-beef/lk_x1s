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

#include <lk/reg.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <platform/delay.h>

#include <dpu/exynos_panel.h>
#include <dpu/mipi_dsi_cmd.h>
#include <platform/sfr.h>
#include <dpu/display_sfr.h>
#include <dpu/cal/regs-dsim.h>
#include <dpu/cal/dsim_cal.h>
#include <target/dpu_io_ctrl.h>


// #define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

extern int dsim_log_level;

/* TODO : add variable dsim drvdata */
extern struct dsim_device *dsim0_for_decon;

#define MAX_DSIM_CNT				2
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

extern struct dsim_device *dsim_drvdata[MAX_DSIM_CNT];

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
	//int lcd_power[2];
	//int lcd_reset;
	//int irq;
	void __iomem *regs;
	void __iomem *ss_regs;
	void __iomem *phy_regs;
	void __iomem *phy_regs_ex;
};

struct dsim_device {
	int id;
	enum dsim_state state;
	struct dsim_resources res;

	unsigned int data_lane;
	u32 data_lane_cnt;

	struct dsim_lcd_driver *panel_ops;
	struct lcd_driver *cm_panel_ops;
	struct exynos_panel_info *lcd_info;
	//struct exynos_panel_info lcd_info;

	struct dsim_clks clks;
	int total_underrun_cnt;

	//struct dphy_timing_value timing;
	//u32 reg_base;
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
#define NUM_OF_VERIFIED_PANEL	6
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
	struct exynos_panel_info *(*get_lcd_info)(void);
	int	(*probe)(struct dsim_device *dsim);
	int	(*suspend)(struct dsim_device *dsim);
	int	(*displayon)(struct dsim_device *dsim);
	int	(*resume)(struct dsim_device *dsim);
};

int dsim_write_data(struct dsim_device *dsim, u32 id, unsigned long d0, u32 d1);
int dsim_read_data(struct dsim_device *dsim, u32 id, u32 addr, u32 cnt, u8 *buf);
int dsim_wait_for_cmd_done(struct dsim_device *dsim);

int dsim_reset_panel(struct dsim_device *dsim);
int dsim_set_panel_power(struct dsim_device *dsim, u32 on);

void dsim_to_regs_param(struct dsim_device *dsim, struct dsim_regs *regs);

static inline struct dsim_device *get_dsim_drvdata(u32 id)
{
	return dsim_drvdata[id];
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
	struct dsim_device *dsim = get_dsim_drvdata(id);
	return readl(dsim->res.regs + reg_id);
}

static inline u32 dsim_read_mask(u32 id, u32 reg_id, u32 mask)
{
	u32 val = dsim_read(id, reg_id);
	val &= (mask);
	return val;
}

static inline void dsim_write(u32 id, u32 reg_id, u32 val)
{
	struct dsim_device *dsim = get_dsim_drvdata(id);
	writel(val, dsim->res.regs + reg_id);
}

static inline void dsim_write_mask(u32 id, u32 reg_id, u32 val, u32 mask)
{
	struct dsim_device *dsim = get_dsim_drvdata(id);
	u32 old = dsim_read(id, reg_id);

	val = (val & mask) | (old & ~mask);
	writel(val, dsim->res.regs + reg_id);
}

/* DPHY register access subroutines */
static inline u32 dsim_phy_read(u32 id, u32 reg_id)
{
	struct dsim_device *dsim = get_dsim_drvdata(id);

	return readl(dsim->res.phy_regs + reg_id);
}

static inline u32 dsim_phy_read_mask(u32 id, u32 reg_id, u32 mask)
{
	u32 val = dsim_phy_read(id, reg_id);

	val &= (mask);
	return val;
}

static inline void dsim_phy_extra_write(u32 id, u32 reg_id, u32 val)
{
	struct dsim_device *dsim = get_dsim_drvdata(id);

	writel(val, dsim->res.phy_regs_ex + reg_id);
}

static inline void dsim_phy_write(u32 id, u32 reg_id, u32 val)
{
	struct dsim_device *dsim = get_dsim_drvdata(id);

	writel(val, dsim->res.phy_regs + reg_id);
}

static inline void dsim_phy_write_mask(u32 id, u32 reg_id, u32 val, u32 mask)
{
	struct dsim_device *dsim = get_dsim_drvdata(id);
	u32 old = dsim_phy_read(id, reg_id);

	val = (val & mask) | (old & ~mask);
	writel(val, dsim->res.phy_regs + reg_id);
	/* printk("offset : 0x%8x, value : 0x%x\n", reg_id, val); */
}

/* FOR extern APIs list */
int dsim_probe(u32 dev_id);
void dsim_reg_sw_reset(u32 id);
int dsim_reg_set_clocks(u32 id, struct dsim_clks *clks,
		struct stdphy_pms *dphy_pms, u32 en);
int dsim_reg_set_lanes(u32 id, u32 lanes, u32 en);
void dsim_reg_set_link_clock(u32 id, u32 en);
void dsim_reg_set_esc_clk_on_lane(u32 id, u32 en, u32 lane);
void dsim_reg_enable_word_clock(u32 id, u32 en);
void dpu_sysreg_dphy_reset(void __iomem *sysreg, u32 dsim_id, u32 rst);
int dsim_disable(struct dsim_device *dsim);
#endif /* __SAMSUNG_DSIM_H__ */
