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
#include <dpu/lcd_ctrl.h>
#include <target/dpu_config.h>

int dpp_log_level = 6;

struct dpp_device *dpp_drvdata[MAX_DPP_CNT];


void dpp_dump(struct dpp_device *dpp)
{
	__dpp_dump(dpp->id, dpp->res.regs, dpp->res.dma_regs, dpp->attr);
}

static void dpp_get_params(struct dpp_device *dpp, struct dpp_params_info *p,  unsigned long addr)
{
	struct exynos_panel_info *lcd_info = decon_get_lcd_info();

	p->src.x = 0;
	p->src.y = 0;
	p->src.w = lcd_info->xres;
	p->src.h = lcd_info->yres;
	p->src.f_w = lcd_info->xres;
	p->src.f_h = lcd_info->yres;

	p->dst.x = 0;
	p->dst.y = 0;
	p->dst.w = lcd_info->xres;
	p->dst.h = lcd_info->yres;
	p->dst.f_w = lcd_info->xres;
	p->dst.f_h = lcd_info->yres;

	p->format = DECON_PIXEL_FORMAT_BGRA_8888;
	p->addr[0] = addr;
	p->addr[1] = 0x0;
	p->addr[2] = 0x0;
	p->addr[3] = 0x0;

	if (dpp->id == LOGO_DPP) {
		/* dpp 0 is for logo */
		p->rot = DPP_ROT_XFLIP;
	} else if (dpp->id == FONT_DPP) {
		/* dpp 2 is for font */
		p->rot = DPP_ROT_NORMAL;
	}
#if 0
	else if (dpp->id == 2) {
		/* dpp 2 is for font */
		p->rot = DPP_ROT_XFLIP;
		//p->rot = DPP_ROT_NORMAL;
	}
#endif

	p->hdr = 0;
	p->min_luminance = 0;
	p->max_luminance = 0;
	p->is_block = false;
	p->is_comp = false;
	p->is_scale = false;

	p->h_ratio = (p->src.w << 20) / p->dst.w;
	p->v_ratio = (p->src.h << 20) / p->dst.h;

#if 0
	p->eq_mode = CSC_BT_601;
	p->hdr = config->dpp_parm.hdr_std;
	p->max_luminance = config->dpp_parm.max_luminance;
	p->min_luminance = config->dpp_parm.min_luminance;
	p->yhd_y2_strd = 0;
	p->ypl_c2_strd = 0;
	p->chd_strd = 0;
	p->cpl_strd = 0;

	/*
        * buffer and base_addr relationship in SBWC (cf. 8+2)
        * <buffer fd> fd[0]: Y-payload / fd[1]: C-payload
        * <base addr> [0]-Y8:Y_HD / [1]-C8:Y_PL / [2]-Y2:C_HD / [3]-C2:C_PL
        *  [1] -> [3] C-payload
        *  [0] -> [1] Y-payload
        *         [0] Y-header : [1] + Y_PL_SIZE
        *         [2] C-header : [3] + C_PL_SIZE
        *
        * TODO :
        * replace PL/HD SIZE & STRIDE macro of videodev2_exynos_media.h
        */
	if (is_rotation(config)) {
		src_w = p->src.h;
		src_h = p->src.w;
	} else {
		src_w = p->src.w;
		src_h = p->src.h;
	}
	dst_w = p->dst.w;
	dst_h = p->dst.h;

	p->h_ratio = (src_w << 20) / dst_w;
	p->v_ratio = (src_h << 20) / dst_h;

	if ((p->h_ratio != (1 << 20)) || (p->v_ratio != (1 << 20)))
		p->is_scale = true;
	else
		p->is_scale = false;

	if ((config->dpp_parm.rot != DPP_ROT_NORMAL) || (p->is_scale) ||
			IS_YUV(fmt_info) || (p->block.w < res->blk_w.min) ||
			(p->block.h < res->blk_h.min))
		p->is_block = false;
	else
		p->is_block = true;
#endif
}

/*
 * TODO: h/w limitation will be changed in KC
 * This function must be modified for KC after releasing DPP constraints
 */
static int dpp_check_limitation(struct dpp_device *dpp, struct dpp_params_info *p)
{
	struct dpp_img_format vi;

	dpp_select_format(dpp, &vi, p);

	return 0;
}

static int dpp_set_config(struct dpp_device *dpp, unsigned long addr)
{
	struct dpp_params_info params;
	int ret = 0;

	if (dpp->state == DPP_STATE_OFF) {
		dpp_dbg("dpp%d is started\n", dpp->id);
		dpp_reg_init(dpp->id, dpp->attr);
	}

	/* parameters from decon driver are translated for dpp driver */
	dpp_get_params(dpp, &params, addr);

	/* all parameters must be passed dpp hw limitation */
#if 0
	ret = dpp_check_limitation(dpp, &params);
	if (ret)
		goto err;
#endif
	/* set all parameters to dpp hw */
	dpp_reg_configure_params(dpp->id, &params, dpp->attr);

	/*
	 * It's only for DPP BIST mode test
	 * dma_reg_set_ch_map(0, dpp->id, true);
	 * dma_reg_set_test_pattern(0, 0, pat_dat[0]);
	 * dma_reg_set_test_pattern(0, 1, pat_dat[1]);
	 */

	dpp->state = DPP_STATE_ON;
err:
	return ret;
}

static void dpp_parse_dt(unsigned int id, struct dpp_device *dpp)
{
	dpp->id = id;
	switch (dpp->id) {
	case LOGO_DPP:
		dpp->attr = 0x50087;
		dpp_info("dpp-%d's attr is (0x%08x)\n", dpp->id, (u32)dpp->attr);
		break;
	case FONT_DPP:
		dpp->attr = 0x50087;
		dpp_info("dpp-%d's attr is (0x%08x)\n", dpp->id, (u32)dpp->attr);
		break;
	default:
		dpp->attr = 0;
		dpp_info("Unsupported dpp-%d\n", dpp->id);
		break;
	}
}

static int dpp_init_resources(struct dpp_device *dpp)
{
	dpp_info("dpp(%d) init resources.\n", dpp->id);

	if (dpp->id == LOGO_DPP)
		dpp->res.regs = (void __iomem *)DPP_IDMAG0_BASE_ADDR;
	else if (dpp->id == FONT_DPP)
		dpp->res.regs = (void __iomem *)DPP_IDMAG1_BASE_ADDR;
	else
		dpp->res.regs = 0;

	if (!dpp->res.regs) {
		dpp_err("failed to remap DPP SFR region\n");
		return -EINVAL;
	}

	if (dpp->id == LOGO_DPP)
		dpp->res.dma_regs = (void __iomem *)DPP_IDMAG0_DMA_ADDR;
	else if (dpp->id == FONT_DPP)
		dpp->res.dma_regs = (void __iomem *)DPP_IDMAG1_DMA_ADDR;
	else
		dpp->res.dma_regs = 0;

	if (!dpp->res.dma_regs) {
		dpp_err("failed to remap DPU_DMA SFR region\n");
		return -EINVAL;
	}

	if (dpp->id == LOGO_DPP) {
		dpp->res.dma_com_regs = (void __iomem *)DPP_IDMAG0_DMA_COM_ADDR;
		if (!dpp->res.dma_com_regs) {
			dpp_err("failed to remap DPU_DMA COMMON SFR region\n");
			return -EINVAL;
		}
	}

	return 0;
}

int dpp_probe(unsigned int id, u32 addr)
{
	struct dpp_device *dpp;
	int ret = 0;

	dpp_info("%s\n", __func__);
	dpp = calloc(1, sizeof(struct dpp_device));
	if (!dpp) {
		/* dpp_err("Failed to allocate local dpp mem\n"); */
		return -ENOMEM;
	}
	dpp_parse_dt(id, dpp);

	dpp_drvdata[id] = dpp;

	ret = dpp_init_resources(dpp);
	if (ret)
		goto err_rsc;

	dpp->state = DPP_STATE_OFF;

	ret = dpp_set_config(dpp, addr);

	dpp->state = DPP_STATE_ON;
	dpp_info("dpp%d is probed successfully\n", dpp->id);

	return 0;

err_rsc:
	free(dpp);
err:
	return ret;
}
