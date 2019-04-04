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
#include <dev/dpu/dpp.h>
#include <dev/dpu/lcd_ctrl.h>
#include <target/dpu_config.h>

int dpp_log_level = 6;

struct dpp_device *dpp_drvdata[MAX_DPP_CNT];


void dpp_dump(struct dpp_device *dpp)
{
	__dpp_dump(dpp->id, dpp->res.regs, dpp->res.dma_regs, dpp->attr);
}

static void dpp_get_params(struct dpp_device *dpp, struct dpp_params_info *p,  unsigned long addr)
{
	struct decon_lcd *lcd_info = decon_get_lcd_info();
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

	if (dpp->id == 0) {
		/* dpp 0 is for logo */
		p->rot = DPP_ROT_NORMAL;
	} else if (dpp->id == 1) {
		/* dpp 2 is for font */
		p->rot = DPP_ROT_NORMAL;
	} else if (dpp->id == 2) {
		/* dpp 2 is for font */
		p->rot = DPP_ROT_NORMAL;
	}
	p->hdr = 0;
	p->min_luminance = 0;
	p->max_luminance = 0;
	p->is_4p = false;
	p->y_2b_strd = 0;
	p->c_2b_strd = 0;
	p->is_comp = false;
	p->is_scale = false;
	p->is_block = false;

	p->format = DECON_PIXEL_FORMAT_ARGB_8888;
	p->addr[0] = addr;
	p->addr[1] = 0x0;
	p->addr[2] = 0x0;
	p->addr[3] = 0x0;
	p->eq_mode = CSC_BT_601;

	p->h_ratio = (p->src.w << 20) / p->dst.w;
	p->v_ratio = (p->src.h << 20) / p->dst.h;
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
	case 0:
		dpp->attr = 0x50087;
		dpp_info("dpp-%d's attr is (0x%08x)\n", dpp->id, (u32)dpp->attr);
		break;
	case 1:
		dpp->attr = 0x500FF;
		dpp_info("dpp-%d's attr is (0x%08x)\n", dpp->id, (u32)dpp->attr);
		break;
	case 2:
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

	if (dpp->id == 0)
		dpp->res.regs = (void __iomem *)DPP_IDMAG0_BASE_ADDR;
	else if (dpp->id == 1)
		dpp->res.regs = (void __iomem *)DPP_IDMAVGRFS_BASE_ADDR;
	else if (dpp->id == 2)
		dpp->res.regs = (void __iomem *)DPP_IDMAG1_BASE_ADDR;
	else
		dpp->res.regs = 0;

	if (!dpp->res.regs) {
		dpp_err("failed to remap DPP SFR region\n");
		return -EINVAL;
	}

	if (dpp->id == 0)
		dpp->res.dma_regs = (void __iomem *)DPP_IDMAG0_DMA_ADDR;
	else if (dpp->id == 1)
		dpp->res.dma_regs = (void __iomem *)DPP_IDMAVGRFS_DMA_ADDR;
	else if (dpp->id == 2)
		dpp->res.dma_regs = (void __iomem *)DPP_IDMAG1_DMA_ADDR;
	else
		dpp->res.dma_regs = 0;

	if (!dpp->res.dma_regs) {
		dpp_err("failed to remap DPU_DMA SFR region\n");
		return -EINVAL;
	}

	dpp->res.dma_com_regs = (void __iomem *)DPP_IDMAG0_DMA_COM_ADDR;
	if (!dpp->res.dma_com_regs) {
		dpp_err("failed to remap DPU_DMA COMMON SFR region\n");
		return -EINVAL;
	}

	return 0;
}

int dpp_probe(unsigned int id, u32 addr)
{
	struct dpp_device *dpp;
	int ret = 0;

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
	if (dpp->id != 2)
		ret = dpp_set_config(dpp, addr);

	dpp->state = DPP_STATE_ON;
	dpp_info("dpp%d is probed successfully\n", dpp->id);

	return 0;

err_rsc:
	free(dpp);
err:
	return ret;
}
