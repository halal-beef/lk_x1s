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

#include <malloc.h>
#include <platform/delay.h>

#include <dev/dpu/dsim.h>
#include <dev/dpu/decon.h>
#include <dev/dpu/lcd_ctrl.h>
#include <target/dpu_config.h>
#include <target/lcd_module.h>

#define MIPI_CMD_TIMEOUT 50

int dsim_log_level = 6;

struct dsim_device *dsim0_for_decon;
struct dsim_device *dsim_drvdata[MAX_DSIM_CNT];


static int dsim_get_interrupt_src(struct dsim_device *dsim, u32 reg_id, unsigned int timeout)
{
	unsigned long delay_time = 1000;
	unsigned int reg = dsim_read(dsim->id, DSIM_INTSRC);

	do {
		reg = dsim_read(dsim->id, DSIM_INTSRC);
		if (reg & reg_id)
			break;
		timeout--;
		udelay(delay_time);
	} while (timeout);

	if (!timeout) {
		dsim_err("timeout!! wait for DSIM interrupt (0x%x)\n", reg);
		return -EBUSY;
	}

	dsim_reg_clear_int(dsim->id, reg);

	return 0;
}


static void dsim_long_data_wr(struct dsim_device *dsim, unsigned long d0, u32 d1)
{
	unsigned int data_cnt = 0, payload = 0;

	/* in case that data count is more then 4 */
	for (data_cnt = 0; data_cnt < d1; data_cnt += 4) {
		/*
		 * after sending 4bytes per one time,
		 * send remainder data less then 4.
		 */
		if ((d1 - data_cnt) < 4) {
			if ((d1 - data_cnt) == 3) {
				payload = *(u8 *)(d0 + data_cnt) |
				    (*(u8 *)(d0 + (data_cnt + 1))) << 8 |
					(*(u8 *)(d0 + (data_cnt + 2))) << 16;
			dsim_dbg("count = 3 payload = %x, %x %x %x\n",
				payload, *(u8 *)(d0 + data_cnt),
				*(u8 *)(d0 + (data_cnt + 1)),
				*(u8 *)(d0 + (data_cnt + 2)));
			} else if ((d1 - data_cnt) == 2) {
				payload = *(u8 *)(d0 + data_cnt) |
					(*(u8 *)(d0 + (data_cnt + 1))) << 8;
			dsim_dbg("count = 2 payload = %x, %x %x\n", payload,
				*(u8 *)(d0 + data_cnt),
				*(u8 *)(d0 + (data_cnt + 1)));
			} else if ((d1 - data_cnt) == 1) {
				payload = *(u8 *)(d0 + data_cnt);
			}

			dsim_reg_wr_tx_payload(dsim->id, payload);
		/* send 4bytes per one time. */
		} else {
			payload = *(u8 *)(d0 + data_cnt) |
				(*(u8 *)(d0 + (data_cnt + 1))) << 8 |
				(*(u8 *)(d0 + (data_cnt + 2))) << 16 |
				(*(u8 *)(d0 + (data_cnt + 3))) << 24;

			dsim_dbg("count = 4 payload = %x, %x %x %x %x\n",
				payload, *(u8 *)(d0 + data_cnt),
				*(u8 *)(d0 + (data_cnt + 1)),
				*(u8 *)(d0 + (data_cnt + 2)),
				*(u8 *)(d0 + (data_cnt + 3)));

			dsim_reg_wr_tx_payload(dsim->id, payload);
		}
	}
}

int dsim_write_data(struct dsim_device *dsim, u32 id, unsigned long d0, u32 d1)
{
	int ret = 0;

	if (dsim->state != DSIM_STATE_ON) {
		dsim_err("DSIM is not ready. state(%d)\n", dsim->state);
		return -EINVAL;
	}

	dsim_reg_clear_int(dsim->id, DSIM_INTSRC_SFR_PH_FIFO_EMPTY);

	switch (id) {
	/* short packet types of packet types for command. */
	case MIPI_DSI_GENERIC_SHORT_WRITE_0_PARAM:
	case MIPI_DSI_GENERIC_SHORT_WRITE_1_PARAM:
	case MIPI_DSI_GENERIC_SHORT_WRITE_2_PARAM:
	case MIPI_DSI_DCS_SHORT_WRITE:
	case MIPI_DSI_DCS_SHORT_WRITE_PARAM:
	case MIPI_DSI_SET_MAXIMUM_RETURN_PACKET_SIZE:
	case MIPI_DSI_DSC_PRA:
	case MIPI_DSI_COLOR_MODE_OFF:
	case MIPI_DSI_COLOR_MODE_ON:
	case MIPI_DSI_SHUTDOWN_PERIPHERAL:
	case MIPI_DSI_TURN_ON_PERIPHERAL:
		dsim_reg_wr_tx_header(dsim->id, id, d0, d1, false);
		if (dsim_get_interrupt_src(dsim, DSIM_INTSRC_SFR_PH_FIFO_EMPTY,
					MIPI_CMD_TIMEOUT) < 0) {
			dsim_err("MIPI DSIM (id=%d) write Timeout! 0x%lX\n",
					id, d0);
			return -ETIMEDOUT;
		}
		break;

	case MIPI_DSI_GENERIC_READ_REQUEST_0_PARAM:
	case MIPI_DSI_GENERIC_READ_REQUEST_1_PARAM:
	case MIPI_DSI_GENERIC_READ_REQUEST_2_PARAM:
	case MIPI_DSI_DCS_READ:
		dsim_reg_wr_tx_header(dsim->id, id, d0, d1, true);
		if (dsim_get_interrupt_src(dsim, DSIM_INTSRC_SFR_PH_FIFO_EMPTY,
					MIPI_CMD_TIMEOUT) < 0) {
			dsim_err("MIPI DSIM (id=%d) write Timeout! 0x%lX\n",
					id, d0);
			return -ETIMEDOUT;
		}
		break;

	/* long packet types of packet types for command. */
	case MIPI_DSI_GENERIC_LONG_WRITE:
	case MIPI_DSI_DCS_LONG_WRITE:
	case MIPI_DSI_DSC_PPS:
		dsim_long_data_wr(dsim, d0, d1);
		dsim_reg_wr_tx_header(dsim->id, id, d1 & 0xff,
				(d1 & 0xff00) >> 8, false);
		if (dsim_get_interrupt_src(dsim, DSIM_INTSRC_SFR_PH_FIFO_EMPTY,
					MIPI_CMD_TIMEOUT) < 0) {
			dsim_err("MIPI DSIM (id=%d) write Timeout! 0x%lX\n",
					id, d0);
			return -ETIMEDOUT;
		}
		break;

	default:
		dsim_info("data id %x is not supported.\n", id);
		ret = -EINVAL;
	}

	if ((dsim->state == DSIM_STATE_ON) && (ret == -ETIMEDOUT)) {
		dsim_err("0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
				readl(dsim->res.regs + DSIM_DPHY_STATUS),
				readl(dsim->res.regs + DSIM_INTSRC),
				readl(dsim->res.regs + DSIM_FIFOCTRL),
				readl(dsim->res.regs + DSIM_CMD_CONFIG));
	}

	return ret;
}

int dsim_read_data(struct dsim_device *dsim, u32 id, u32 addr, u32 cnt, u8 *buf)
{
	u32 rx_fifo, rx_size = 0;
	u32 i, j;
	int ret = 0;
	u32 rx_fifo_depth = DSIM_RX_FIFO_MAX_DEPTH;

	if (dsim->state != DSIM_STATE_ON) {
		dsim_err("DSIM is not ready. state(%d)\n", dsim->state);
		return -EINVAL;
	}

	dsim_reg_clear_int(dsim->id, DSIM_INTSRC_RX_DATA_DONE);

	/* Set the maximum packet size returned */
	dsim_write_data(dsim,
		MIPI_DSI_SET_MAXIMUM_RETURN_PACKET_SIZE, cnt, 0);

	/* Read request */
	dsim_write_data(dsim, id, addr, 0);

	if (dsim_get_interrupt_src(dsim, DSIM_INTSRC_RX_DATA_DONE,
				MIPI_CMD_TIMEOUT) < 0) {
		dsim_err("MIPI DSIM (id=%d) read Timeout! 0x%X\n",
				id, addr);
		return -ETIMEDOUT;
	}

	do {
		rx_fifo = dsim_reg_get_rx_fifo(dsim->id);

		/* Parse the RX packet data types */
		switch (rx_fifo & 0xff) {
		case MIPI_DSI_RX_ACKNOWLEDGE_AND_ERROR_REPORT:
			ret = dsim_reg_rx_err_handler(dsim->id, rx_fifo);
			if (ret < 0) {
				dsim_err("__dsim_dump\n");
				goto exit;
			}
			break;
		case MIPI_DSI_RX_END_OF_TRANSMISSION:
			dsim_dbg("EoTp was received from LCD module.\n");
			break;
		case MIPI_DSI_RX_DCS_SHORT_READ_RESPONSE_1BYTE:
		case MIPI_DSI_RX_DCS_SHORT_READ_RESPONSE_2BYTE:
		case MIPI_DSI_RX_GENERIC_SHORT_READ_RESPONSE_1BYTE:
		case MIPI_DSI_RX_GENERIC_SHORT_READ_RESPONSE_2BYTE:
			dsim_dbg("Short Packet was received from LCD module.\n");
			for (i = 0; i <= cnt; i++)
				buf[i] = (rx_fifo >> (8 + i * 8)) & 0xff;
			break;
		case MIPI_DSI_RX_DCS_LONG_READ_RESPONSE:
		case MIPI_DSI_RX_GENERIC_LONG_READ_RESPONSE:
			dsim_dbg("Long Packet was received from LCD module.\n");
			rx_size = (rx_fifo & 0x00ffff00) >> 8;
			dsim_dbg("rx fifo : %8x, response : %x, rx_size : %d\n",
					rx_fifo, rx_fifo & 0xff, rx_size);
			/* Read data from RX packet payload */
			for (i = 0; i < rx_size >> 2; i++) {
				rx_fifo = dsim_reg_get_rx_fifo(dsim->id);
				for (j = 0; j < 4; j++)
					buf[(i*4)+j] = (u8)(rx_fifo >> (j * 8)) & 0xff;
			}
			if (rx_size % 4) {
				rx_fifo = dsim_reg_get_rx_fifo(dsim->id);
				for (j = 0; j < rx_size % 4; j++)
					buf[4 * i + j] =
						(u8)(rx_fifo >> (j * 8)) & 0xff;
			}
			break;
		default:
			dsim_err("Packet format is invaild.\n");
			ret = -EBUSY;
			break;
		}
	} while (!dsim_reg_rx_fifo_is_empty(dsim->id) && --rx_fifo_depth);

	ret = rx_size;
	if (!rx_fifo_depth) {
		dsim_err("Check DPHY values about HS clk.\n");
		ret = -EBUSY;
	}
exit:

	return ret;
}

static void dsim_d_phy_onoff(struct dsim_device *dsim,
	unsigned int enable)
{
	call_config_ops(dsim, set_mipi_phy, enable);
}

int dsim_get_gpios(struct dsim_device *dsim)
{
	dsim_dbg("%s +\n", __func__);

	call_config_ops(dsim, set_gpio_hw_te);

	dsim_dbg("%s -\n", __func__);

	return 0;
}

int dsim_reset_panel(struct dsim_device *dsim)
{
	dsim_dbg("%s +\n", __func__);

	call_config_ops(dsim, set_gpio_lcd_reset, dsim->board_type);

	dsim_dbg("%s -\n", __func__);

	return 0;
}

int dsim_set_panel_power(struct dsim_device *dsim, u32 on)
{
	dsim_dbg("%s +\n", __func__);

	call_config_ops(dsim, set_gpio_lcd_power, dsim->board_type);

	dsim_dbg("%s -\n", __func__);

	return 0;
}

static int dsim_get_data_lanes(struct dsim_device *dsim)
{
	unsigned int i;

	if (dsim->data_lane_cnt > MAX_DSIM_DATALANE_CNT) {
		dsim_err("%d data lane couldn't be supported\n",
				dsim->data_lane_cnt);
		return -EINVAL;
	}

	dsim->data_lane = DSIM_LANE_CLOCK;
	for (i = 1; i < dsim->data_lane_cnt + 1; ++i)
		dsim->data_lane |= 1 << i;

	dsim_dbg("%s: lanes(0x%x)\n", __func__, dsim->data_lane);

	return 0;
}

static void dsim_device_to_clks(struct dsim_device *dsim,
		struct dsim_clks *clks)
{
	clks->hs_clk = dsim->lcd_info->hs_clk;
	clks->esc_clk = dsim->lcd_info->esc_clk;
}

static void clks_to_dsim_device(struct dsim_clks *clks,
		struct dsim_device *dsim)
{
	dsim->clks.hs_clk = clks->hs_clk;
	dsim->clks.esc_clk = clks->esc_clk;
	dsim->clks.byte_clk = clks->byte_clk;
}

/* TODO : Enable sysreg and dsim control */
void dpu_sysreg_set_dphy(u32 id, u32 sysreg)
{
	u32 val;

	val = 0; /*Selects reset from SYSREG_DISPAUD*/
	writel(val, sysreg + DISP_DPU_MIPI_PHY_CON);
}
#if 0
void dpu_sysreg_dphy_reset(u32 sysreg, u32 dsim_id, u32 rst)
{
	u32 old = readl(sysreg + DISP_DPU_MIPI_PHY_CON);
	u32 val = rst ? ~0 : 0;
	u32 mask = M_RESETN_M4S4_TOP_MASK;

	val = (val & mask) | (old & ~mask);
	writel(val, sysreg + DISP_DPU_MIPI_PHY_CON);
}
#endif

static int dsim_disable(struct dsim_device *dsim)
{
	int ret = 0;

	if (dsim->state == DSIM_STATE_OFF)
		return 0;

	//call_panel_ops(dsim, suspend, dsim);

	dsim_reg_stop(dsim->id, dsim->data_lane);

	/* DPHY power off */
	dsim_d_phy_onoff(dsim, 0);

	/* Panel power off */
	dsim_set_panel_power(dsim, 0);

	dsim->state = DSIM_STATE_OFF;

	return ret;
}

static int dsim_enable(struct dsim_device *dsim)
{
	struct dsim_clks clks = {0};
	enum dsim_state state;
	bool panel_ctrl;
	int ret = 0;

	if (dsim->state == DSIM_STATE_ON)
		return 0;

	/* DPHY power on */
	dsim_d_phy_onoff(dsim, 1);

	state = DSIM_STATE_ON;
	panel_ctrl = (state == DSIM_STATE_ON) ? true : false;

	dsim_device_to_clks(dsim, &clks);
	clks_to_dsim_device(&clks, dsim);

	dsim_reg_init(dsim->id, dsim->lcd_info, &dsim->clks, panel_ctrl);
	dsim_reg_start(dsim->id);

	dsim->state = DSIM_STATE_ON;

	return ret;
}

struct decon_lcd *decon_get_lcd_info(void)
{
	struct dsim_device *dsim;
	if (dsim0_for_decon)
		dsim = dsim0_for_decon;
	else {
		dsim_err("Fail to get dsim\n");
		return NULL;
	}

	if (!dsim->panel_ops) {
		dsim_err("Fail to get dsim panel_ops\n");
		return NULL;
	}

	return dsim->panel_ops->get_lcd_info();
}
#if 0
void dsim_to_regs_param(struct dsim_device *dsim, struct dsim_regs *regs)
{
	if (dsim->res.regs)
		regs->regs = dsim->res.regs;
	else
		regs->regs = NULL;

	if (dsim->res.phy_regs)
		regs->phy_regs = dsim->res.phy_regs;
	else
		regs->phy_regs = NULL;

	if (dsim->res.phy_regs_ex)
		regs->phy_regs_ex = dsim->res.phy_regs_ex;
	else
		regs->phy_regs_ex = NULL;
}
#endif
void dsim_dump(struct dsim_device *dsim)
{
	struct dsim_regs regs;

	dsim_info("=== DSIM SFR DUMP ===\n");

	dsim_to_regs_param(dsim, &regs);
	__dsim_dump(dsim->id, &regs);
}

int dsim_probe(u32 dev_id)
{
	int ret = 0;
	struct dsim_device *dsim = NULL;

	dsim = calloc(1, sizeof(struct dsim_device));
	if (!dsim) {
		ret = -ENOMEM;
		goto err;
	}

	/* Display configuration on U-BOOT */
	/* TODO : check necessity of board type */
	dsim->board_type = get_exynos_board_type();
	dsim->config_ops = get_exynos_display_config();

	dsim->id = dev_id;
	if (dsim->id == 0) {
		dsim0_for_decon = dsim;
		dsim_drvdata[dev_id] = dsim;
	} else {
		ret = -ENXIO;
		goto err;
	}
	/* TODO : get_gpios is needed to change name,
	 * here is a TE setting but TE setting may be moved to decon
	 */
	dsim_get_gpios(dsim);

	dsim->lcd_info = common_get_lcd_info();

	dsim->data_lane_cnt = dsim->lcd_info->data_lane;
	dsim_info("using data lane count(%d)\n", dsim->data_lane_cnt);

	dsim->res.ss_regs = (void __iomem *)EXYNOS9830_SYSREG_DPU;
	dsim->res.phy_regs = (void __iomem *)DPHY_BASE_ADDR;
	dsim->res.phy_regs_ex = (void __iomem *)DPHY_EX_BASE_ADDR;

	if (dsim->id == 0)
		dsim->res.regs = (void __iomem *)DSIM0_BASE_ADDR;
	else {
		ret = -ENXIO;
		goto err;
	}

	ret = dsim_get_data_lanes(dsim);
	if (ret) {
		ret = -EIO;
		goto err_dt;
	}

	dsim->state = DSIM_STATE_INIT;
	dsim_enable(dsim);

	dsim->cm_panel_ops = get_lcd_drv_ops();
	if (!dsim->cm_panel_ops) {
		dsim_err("dsim fail to get common panel operation\n");
		ret = -EINVAL;
		goto err;
	}
	dsim->cm_panel_ops->fill_id(dsim);

	dsim->panel_ops = dsim->cm_panel_ops->get_panel_info(dsim);
	if (!dsim->panel_ops) {
		dsim_err("dsim fail to get panel operation\n");
		ret = -EFAULT;
		goto err;
	}
	dsim_disable(dsim);

	dsim->lcd_info = decon_get_lcd_info();
	dsim_enable(dsim);
#if defined(CONFIG_EXYNOS_DSIM_BIST)
	/* TODO: This is for dsim BIST mode in zebu emulator. only for test*/
	call_panel_ops(dsim, displayon, dsim);
	dsim_reg_set_bist(dsim->id, true);
	dsim_info("START DSIM BIST\n");
#else
	call_panel_ops(dsim, probe, dsim);
#endif
	dsim_info("dsim%d driver(%s mode) has been probed.\n", dsim->id,
		dsim->lcd_info->mode == DECON_MIPI_COMMAND_MODE ? "cmd" : "video");

	//dsim_dump(dsim);
	return 0;

err:
	dsim_err("dsim has error.\n");
err_dt:
	free(dsim);
	dsim_err("dsim probe failed.\n");

	return ret;
}
