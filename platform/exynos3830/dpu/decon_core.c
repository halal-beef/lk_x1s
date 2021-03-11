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
#include <dpu/dpp.h>
#include <dpu/decon.h>
#include <dpu/dsim.h>

#include <dpu/lcd_ctrl.h>
#include <lib/logo_display.h>
#include <lib/font_display.h>
#include <target/dpu_config.h>


int decon_log_level = 6;
extern struct dsim_device *dsim0_for_decon;
extern struct dpp_device *dpp_drvdata[MAX_DPP_CNT];
struct decon_device *decon_drvdata[MAX_DECON_CNT];

unsigned int win_fb0 = CONFIG_DISPLAY_FONT_BASE_ADDRESS;
unsigned int win_fb1 = CONFIG_DISPLAY_LOGO_BASE_ADDRESS;

/* ---------- CHECK FUNCTIONS ----------- */
extern void dsim_dump(struct dsim_device *dsim);
extern void dpp_dump(struct dpp_device *dpp);
void decon_dump(struct decon_device *decon)
{
	void __iomem *base_regs = get_decon_drvdata(0)->res.regs;

	__decon_dump(decon->id, decon->res.regs, base_regs,
			decon->lcd_info->dsc.en);
}

void decon_to_psr_info(struct decon_device *decon, struct decon_mode_info *psr)
{
	psr->psr_mode = decon->dt->psr_mode;
	psr->trig_mode = decon->dt->trig_mode;
	psr->dsi_mode = decon->dt->dsi_mode;
	psr->out_type = decon->dt->out_type;
}

void decon_to_init_param(struct decon_device *decon, struct decon_param *p)
{
	struct exynos_panel_info *lcd_info = decon_get_lcd_info();

	p->lcd_info = lcd_info;
	p->psr.psr_mode = decon->dt->psr_mode;
	p->psr.trig_mode = decon->dt->trig_mode;
	p->psr.dsi_mode = decon->dt->dsi_mode;
	p->psr.out_type = decon->dt->out_type;
	p->nr_windows = decon->dt->max_win;
	p->disp_ss_regs = decon->dt->ss_regs;

	decon_dbg("%s: psr(%d) trig(%d) dsi(%d) out(%d) wins(%d) LCD[%d %d] disp_ss[%p]\n",
			__func__, p->psr.psr_mode, p->psr.trig_mode,
			p->psr.dsi_mode, p->psr.out_type, p->nr_windows,
			p->lcd_info->xres, p->lcd_info->yres, p->disp_ss_regs);
}

/* ---------- FB_IOCTL INTERFACE ----------- */
static void decon_parse_pdata(struct decon_device *decon, u32 dev_id)
{
	struct decon_dt_info *dt = get_decon_pdata();

	decon->id = dev_id;
	decon->dt->max_win = dt->max_win;
	decon->dt->dft_win = dt->dft_win;
	decon->dt->dft_ch = dt->dft_ch;
	decon->dt->psr_mode = dt->psr_mode;
	decon->dt->trig_mode = dt->trig_mode;
	decon->dt->dsi_mode = dt->dsi_mode;
	decon->dt->out_idx = dt->out_idx;
	decon->dt->out_type = dt->out_type;

	decon->dt->dpp_cnt = dt->dpp_cnt;
	decon->dt->dsim_cnt = dt->dsim_cnt;
	decon->dt->decon_cnt = dt->decon_cnt;
	decon_info("decon-%s: max win%d, %s mode, %s trigger\n",
			decon->id ? "ext" : "int",
			decon->dt->max_win,
			decon->dt->psr_mode ? "command" : "video",
			decon->dt->trig_mode ? "sw" : "hw");
	decon_info("%s dsi mode\n", decon->dt->dsi_mode ? "dual" : "single");
}

u32 wincon(u32 transp_len, u32 a0, u32 a1,
	int plane_alpha, enum decon_blending blending, int idx)
{
	u32 data = 0;

	data |= WIN_EN_F(idx);

	return data;
}

void decon_show_buffer_update(struct decon_device *decon,
		struct dsim_device *dsim, u32 color)
{
	struct decon_mode_info psr = {0};

	decon_to_psr_info(decon, &psr);

	decon_reg_all_win_shadow_update_req(decon->id);
	decon_reg_start(decon->id, &psr);
	if (decon_reg_wait_update_done_and_mask(decon->id, &psr,
				SHADOW_UPDATE_TIMEOUT) < 0) {
		decon_err("%s: wait_for_update_timeout\n", __func__);
#if defined(EXYNOS_DPU_DUMP)
		decon_dump(decon);
		dsim_dump(dsim_drvdata[0]);
		dpp_dump(dpp_drvdata[LOGO_DPP]);
		dpp_dump(dpp_drvdata[FONT_DPP]);
#endif
	}
}

void decon_show_buffer(struct decon_device *decon,
		struct dsim_device *dsim, u32 color)
{
#if defined(CONFIG_DISPLAY_DRAWLOGO) || defined(CONFIG_DISPLAY_DRAWFONT)
	struct decon_window_regs win_regs = {0};
#endif
	struct decon_mode_info psr = {0};
	struct decon_param p;

	/* stop smmu before proceeding for buffer rendering */
	/* TODO : make sysmmu ctrl to dpu io ctrl */
//	writel(0, SYSMMU_DPU_BASE_ADDR);

	decon_to_init_param(decon, &p);
	decon_reg_init(decon->id, decon->dt->out_idx, &p);

#ifdef CONFIG_DISPLAY_DRAWLOGO
	/* window 2 config */
	win_regs.wincon = wincon(0x8, 0xFF, 0xFF, 0xFF, DECON_BLENDING_NONE, decon->dt->dft_win);
	/* common config */
	win_regs.start_pos = win_start_pos(0, 0);
	win_regs.end_pos = win_end_pos(0, 0, p.lcd_info->xres, p.lcd_info->yres);
	decon_info("Display 1st window(%d) xres(%d) yres(%d) win_start_pos(%x) win_end_pos(%x)\n",
			decon->dt->dft_win, p.lcd_info->xres, p.lcd_info->yres, win_regs.start_pos,
			win_regs.end_pos);

	win_regs.colormap = 0x0;
	win_regs.pixel_count = p.lcd_info->xres * p.lcd_info->yres;
	win_regs.whole_w = p.lcd_info->xres;
	win_regs.whole_h = p.lcd_info->yres;
	win_regs.offset_x = 0;
	win_regs.offset_y = 0;
	win_regs.format = DECON_PIXEL_FORMAT_ARGB_8888;

	win_regs.ch = LOGO_DPP;
	win_regs.plane_alpha = 0;
	win_regs.blend = DECON_BLENDING_NONE;
	decon_reg_set_window_control(decon->id, decon->dt->dft_win, &win_regs, false);
#endif

#ifdef CONFIG_DISPLAY_DRAWFONT
	/* window 3 config */
	win_regs.wincon = wincon(0x8, 0xFF, 0xFF, 0xFF, DECON_BLENDING_COVERAGE, decon->dt->dft_win + 1);
	/* common config */
	win_regs.start_pos = win_start_pos(0, 0);
	win_regs.end_pos = win_end_pos(0, 0, p.lcd_info->xres, p.lcd_info->yres);
	decon_info("Display 2st window(%d) xres(%d) yres(%d) win_start_pos(%x) win_end_pos(%x)\n",
			decon->dt->dft_win + 1, p.lcd_info->xres, p.lcd_info->yres, win_regs.start_pos,
			win_regs.end_pos);

	win_regs.colormap = 0x0;
	win_regs.pixel_count = p.lcd_info->xres * p.lcd_info->yres;
	win_regs.whole_w = p.lcd_info->xres;
	win_regs.whole_h = p.lcd_info->yres;
	win_regs.offset_x = 0;
	win_regs.offset_y = 0;
	win_regs.format = DECON_PIXEL_FORMAT_ARGB_8888;

	/* TODO : add 2nd dft idma to dt data */
	win_regs.ch = FONT_DPP;
	win_regs.plane_alpha = 0xff;
	win_regs.blend = DECON_BLENDING_COVERAGE;

	/* TODO : add 2nd dft win to dt data */
	decon_reg_set_window_control(decon->id, decon->dt->dft_win + 1, &win_regs, false);
#endif
	/* disable irq and clear */
	decon_reg_set_int(decon->id, &psr, 0);

	/* shadow update request */
	decon_reg_all_win_shadow_update_req(decon->id);

	decon_to_psr_info(decon, &psr);
	call_panel_ops(dsim, displayon, dsim);
	decon_reg_start(decon->id, &psr);

	/* Enable irq and clear */
	decon_reg_set_int(decon->id, &psr, 1);

	if (decon_reg_wait_update_done_and_mask(decon->id, &psr,
				SHADOW_UPDATE_TIMEOUT) < 0)
		decon_err("%s: wait_for_update_timeout\n", __func__);
}

void decon_show_color_map(struct decon_device *decon,
		struct dsim_device *dsim)
{
	decon_info("KMS ::::: color map!!!\n");
	struct decon_window_regs win_regs = {0};
	struct decon_mode_info psr = {0};
	struct decon_param p;

	decon_to_init_param(decon, &p);
	decon_reg_init(decon->id, decon->dt->out_idx, &p);


	/* window config */
	win_regs.wincon = wincon(0x8, 0xFF, 0xFF, 0xFF, DECON_BLENDING_NONE, decon->dt->dft_win);
	/* common config */
	win_regs.start_pos = win_start_pos(0, 0);
	win_regs.end_pos = win_end_pos(0, 0, p.lcd_info->xres, p.lcd_info->yres);
	decon_info("Colormap xres %d yres %d win_start_pos %x win_end_pos %x\n",
			p.lcd_info->xres, p.lcd_info->yres, win_regs.start_pos,
			win_regs.end_pos);
	win_regs.colormap = 0xff0000;
	win_regs.pixel_count = p.lcd_info->xres * p.lcd_info->yres;
	win_regs.whole_w = p.lcd_info->xres;
	win_regs.whole_h = p.lcd_info->yres;
	win_regs.offset_x = 0;
	win_regs.offset_y = 0;
	win_regs.ch = LOGO_DPP;
	decon_info("pixel_count(%d), whole_w(%d), whole_h(%d), x(%d), y(%d)\n",
			win_regs.pixel_count, win_regs.whole_w,
			win_regs.whole_h, win_regs.offset_x,
			win_regs.offset_y);

	decon_reg_set_window_control(decon->id, decon->dt->dft_win, &win_regs, true);

	/* disable irq and clear */
	decon_reg_set_int(decon->id, &psr, 0);

	/* shadow update request */
	decon_reg_all_win_shadow_update_req(decon->id);

	decon_to_psr_info(decon, &psr);
	call_panel_ops(dsim, displayon, dsim);
	decon_reg_start(decon->id, &psr);

	/* Enable irq and clear */
	decon_reg_set_int(decon->id, &psr, 1);

	if (decon_reg_wait_update_done_and_mask(decon->id, &psr,
				SHADOW_UPDATE_TIMEOUT) < 0)
		decon_err("%s: wait_for_update_timeout\n", __func__);
}

/* --------- DRIVER INITIALIZATION ---------- */
static int decon_probe(u32 dev_id)
{
	struct decon_device *decon;
	struct dsim_device *dsim;
	struct exynos_panel_info *lcd_info = decon_get_lcd_info();

	if (dev_id > DFT_DECON) {
		decon_err("does not support (%u) decon device\n", dev_id);
		return -ERANGE;
	}

	decon = calloc(1, sizeof(struct decon_device));
	if (!decon) {
		/* decon_err("no memory for decon device\n"); */
		return -ENOMEM;
	}

	decon->dt = calloc(1, sizeof(struct decon_dt_info));
	if (!decon->dt) {
		/* decon_err("no memory for DECON dt data\n"); */
		free(decon);
		return -ENOMEM;
	}
	decon_parse_pdata(decon, dev_id);
	decon->lcd_info = lcd_info;

	/* TODO : move to dt data */
	decon->res.regs = (void __iomem *)DECON0_BASE_ADDR;
	decon->res.ss_regs = (void __iomem *)DPU_SYSREG_BASE_ADDR;

	decon_drvdata[dev_id] = decon;
	dsim = dsim_drvdata[0];

	if (WINMAP > 0) {
		decon_show_color_map(decon, dsim);
#if defined(EXYNOS_DPU_DUMP)
		decon_dump(decon);
		dsim_dump(dsim);
#endif
		return 0;
	}

	decon_show_buffer(decon, dsim, LUT_COLOR_1);
#if defined(EXYNOS_DPU_DUMP)
	decon_dump(decon);
	dsim_dump(dsim_drvdata[0]);
	dpp_dump(dpp_drvdata[LOGO_DPP]);
	dpp_dump(dpp_drvdata[FONT_DPP]);
#endif
	decon->state = DECON_STATE_ON;
	decon_info("decon%d registered successfully\n", decon->id);
	return 0;
}

void decon_string_update(void)
{
	struct decon_device *decon = decon_drvdata[0];
	struct dsim_device *dsim = dsim0_for_decon;
	if (decon != NULL && dsim != NULL && !WINMAP
		&& decon->state == DECON_STATE_ON)
		decon_show_buffer_update(decon, dsim, LUT_COLOR_1);
}

int decon_resize_align(unsigned int xsize, unsigned int ysize)
{
	struct dpp_params_info p;
	struct decon_device *decon = decon_drvdata[0];
	struct decon_window_regs win_regs = {0};
	unsigned long attr = 0;


	/* dpp */
	p.src.x = 0;
	p.src.y = 0;
	p.src.w = xsize;
	p.src.h = ysize;
	p.src.f_w = xsize;
	p.src.f_h = ysize;

	p.rot = DPP_ROT_XFLIP;

	p.is_comp = false;
	p.format = DECON_PIXEL_FORMAT_ARGB_8888;
	p.addr[0] = win_fb1;
	p.eq_mode = CSC_BT_601;

	p.is_scale = false;

	p.is_block = false;
	/* Each bit indicates DPP attribute */
	/* 0:AFBC 1:BLOCK 2:FLIP 3:ROT 4:CSC 5:SCALE 6:HDR 7:HDR10 */
	/* 16:IDMA 17:ODMA 18:DPP */
	attr = 0x50087; /* DPP/IDMA/HDR10/FLIP/BLOCK/AFBC */
	dpp_reg_configure_params(0, &p, attr);

	/* decon */
	/* common config */
	win_regs.start_pos = win_start_pos((LCD_WIDTH - xsize) / 2, (LCD_HEIGHT - ysize) / 2);
	win_regs.end_pos = win_end_pos((LCD_WIDTH - xsize) / 2, (LCD_HEIGHT - ysize) / 2, xsize, ysize);
	win_regs.colormap = 0x0;
	win_regs.pixel_count = xsize * ysize;
	win_regs.whole_w = xsize;
	win_regs.whole_h = ysize;
	win_regs.offset_x = 0;
	win_regs.offset_y = 0;
	win_regs.format = DECON_PIXEL_FORMAT_ARGB_8888;

	/* window 0 dedicated config */
	win_regs.wincon = wincon(0x8, 0xFF, 0xFF, 0xFF, DECON_BLENDING_NONE, decon->dt->dft_win);
	win_regs.ch = LOGO_DPP;
	win_regs.plane_alpha = 0;
	win_regs.blend = DECON_BLENDING_NONE;

	decon_info("Display xres(%d) yres(%d) win_start_pos(%x) win_end_pos(%x)\n",
			xsize, ysize, win_regs.start_pos, win_regs.end_pos);

	decon_reg_set_window_control(decon->id, decon->dt->dft_win, &win_regs, false);

	/* shadow update request */
	decon_reg_update_req_window(0, decon->dt->dft_win);

	return 0;
}

/* --------- DRIVER DISPLAY START ---------- */
int display_drv_init(void)
{
	int ret = 0;

	ret = dsim_probe(DFT_DSIM);
	if (ret < 0) {
		decon_err("dsim%d probe was failed\n", DFT_DSIM);
		return ret;
	}

#ifdef CONFIG_DISPLAY_DRAWLOGO

	/* DPP0 G0 for logo framebuffer */

	ret = dpp_probe(LOGO_DPP, win_fb1);
	if (ret < 0) {
		decon_err("dpp%d probe was failed\n", LOGO_DPP);
		return ret;
	}
#endif

#ifdef CONFIG_DISPLAY_DRAWFONT
	/* DPP1 G1 for font framebuffer */
	ret = dpp_probe(FONT_DPP, win_fb0);
	if (ret < 0) {
		decon_err("dpp%d probe was failed\n", FONT_DPP);
		return ret;
	}
#endif

	ret = decon_probe(DFT_DECON);
	if (ret < 0) {
		decon_err("decon%d probe was failed\n", DFT_DECON);
		return ret;
	}

	return ret;
}

static int decon_disable(struct decon_device *decon)
{
	struct decon_mode_info psr = {0};

	if (decon->state == DECON_STATE_OFF)
		return 0;

	decon_to_psr_info(decon, &psr);
	decon_reg_stop(decon->id, 0, &psr, true, decon->lcd_info->fps);

	decon->state = DECON_STATE_OFF;

	decon_info("decon disable\n");

	return 0;
}

int display_drv_deinit(void)
{
	struct decon_device *decon = decon_drvdata[0];
	struct dsim_device *dsim = dsim0_for_decon;

	decon_info("%s +\n", __func__);

	if (decon->state == DECON_STATE_OFF)
		return 0;

	call_panel_ops(dsim, suspend, dsim);

	decon_disable(decon);

	dsim_disable(dsim);

	decon_info("%s -\n", __func__);

	return 0;
}
