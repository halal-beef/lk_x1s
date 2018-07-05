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

#ifndef __SAMSUNG_DPP_H__
#define __SAMSUNG_DPP_H__

#include <errno.h>

#include <dev/dpu/decon.h>
#include <platform/dpu_cal/regs-dpp.h>
#include <platform/display_sfr.h>

extern int dpp_log_level;

#define DPP_MODULE_NAME		"exynos-dpp"
#define MAX_DPP_CNT		4

/* about 1msec @ ACLK=630MHz */
#define INIT_RCV_NUM		630000

#define SRC_SIZE_MULTIPLE	1
#define SRC_WIDTH_MIN		16
#define SRC_WIDTH_MAX		65534
#define SRC_HEIGHT_MIN		16
#define SRC_HEIGHT_MAX		8190
#define IMG_SIZE_MULTIPLE	1
#define IMG_WIDTH_MIN		16
#define IMG_WIDTH_MAX		4096
#define IMG_HEIGHT_MIN		16
#define IMG_HEIGHT_MAX		4096
#define SRC_OFFSET_MULTIPLE	1

#define SCALED_WIDTH_MIN	16
#define SCALED_WIDTH_MAX	4096
#define SCALED_HEIGHT_MIN	16
#define SCALED_HEIGHT_MAX	4096
#define SCALED_SIZE_MULTIPLE	1
#define SCALED_SIZE_MULTIPLE	1

#define BLK_WIDTH_MIN		4
#define BLK_WIDTH_MAX		4096
#define BLK_HEIGHT_MIN		1
#define BLK_HEIGHT_MAX		4096
#define BLK_SIZE_MULTIPLE	1
#define BLK_SIZE_MULTIPLE	1

#define DST_SIZE_MULTIPLE	1
#define DST_SIZE_WIDTH_MIN	16
#define DST_SIZE_WIDTH_MAX	8190
#define DST_SIZE_HEIGHT_MIN	16
#define DST_SIZE_HEIGHT_MAX	8190
#define DST_OFFSET_MULTIPLE	1
#define DST_IMG_MULTIPLE	1
#define DST_IMG_WIDTH_MIN	16
#define DST_IMG_WIDTH_MAX	4096
#define DST_IMG_HEIGHT_MIN	16
#define DST_IMG_HEIGHT_MAX	4096


#define IS_ALIGNED(x, a)                (((x) & ((typeof(x))(a) - 1)) == 0)
#define check_align(width, height, align_w, align_h)\
	(IS_ALIGNED(width, align_w) && IS_ALIGNED(height, align_h))

#define is_normal(config) (0)
#define is_yuv(config) (false)
#define is_yuv422(config) (false)
#define is_yuv420(config) (false)
#define is_vgr(dpp) (false)

#define dpp_err(fmt, ...)							\
	do {									\
		if (dpp_log_level >= 3) {					\
			printf(fmt, ##__VA_ARGS__);			\
		}								\
	} while (0)

#define dpp_warn(fmt, ...)							\
	do {									\
		if (dpp_log_level >= 4) {					\
			printf(fmt, ##__VA_ARGS__);			\
		}								\
	} while (0)

#define dpp_info(fmt, ...)							\
	do {									\
		if (dpp_log_level >= 6)					\
			printf(fmt, ##__VA_ARGS__);			\
	} while (0)

#define dpp_dbg(fmt, ...)							\
	do {									\
		if (dpp_log_level >= 7)					\
			printf(fmt, ##__VA_ARGS__);			\
	} while (0)

/* TODO: This will be removed */
struct dpp_size_constraints {
	u32		src_mul_w;
	u32		src_mul_h;
	u32		src_w_min;
	u32		src_w_max;
	u32		src_h_min;
	u32		src_h_max;
	u32		img_mul_w;
	u32		img_mul_h;
	u32		img_w_min;
	u32		img_w_max;
	u32		img_h_min;
	u32		img_h_max;
	u32		blk_w_min;
	u32		blk_w_max;
	u32		blk_h_min;
	u32		blk_h_max;
	u32		blk_mul_w;
	u32		blk_mul_h;
	u32		src_mul_x;
	u32		src_mul_y;
	u32		sca_w_min;
	u32		sca_w_max;
	u32		sca_h_min;
	u32		sca_h_max;
	u32		sca_mul_w;
	u32		sca_mul_h;
	u32		dst_mul_w;
	u32		dst_mul_h;
	u32		dst_w_min;
	u32		dst_w_max;
	u32		dst_h_min;
	u32		dst_h_max;
	u32		dst_mul_x;
	u32		dst_mul_y;
};

struct dpp_img_format {
	u32		vgr;
	u32		normal;
	u32		rot;
	u32		scale;
	u32		format;
	u32		afbc_en;
	u32		yuv;
	u32		yuv422;
	u32		yuv420;
};

enum dpp_cfg_err {
	DPP_CFG_ERR_SCL_POS		= (1 << 4),
	DPP_CFG_ERR_SCALE_RATIO		= (1 << 3),
	DPP_CFG_ERR_ODD_SIZE		= (1 << 2),
	DPP_CFG_ERR_MAX_SIZE		= (1 << 1),
	DPP_CFG_ERR_MIN_SIZE		= (1 << 0),
};

enum dpp_csc_defs {
	/* csc_type */
	DPP_CSC_BT_601 = 0,
	DPP_CSC_BT_709 = 1,
	/* csc_range */
	DPP_CSC_NARROW = 0,
	DPP_CSC_WIDE = 1,
	/* csc_mode */
	CSC_COEF_HARDWIRED = 0,
	CSC_COEF_CUSTOMIZED = 1,
	/* csc_id used in csc_3x3_t[] : increase by even value */
	DPP_CSC_ID_BT_2020 = 0,
	DPP_CSC_ID_DCI_P3 = 2,
};

enum dpp_state {
	DPP_STATE_ON,
	DPP_STATE_OFF,
};

enum dpp_reg_area {
	REG_AREA_DPP = 0,
	REG_AREA_DMA,
	REG_AREA_DMA_COM,
};

struct dpp_resources {
	struct clk *gate;
	void __iomem *regs;
	void __iomem *dma_regs;
	void __iomem *dma_com_regs;
	int irq;
	int dma_irq;
};

struct dpp_device {
	int id;
	enum dpp_state state;
	struct dpp_resources res;
	struct decon_win_config *config;
};

struct dpp_params_info {
	struct decon_frame src;
	struct decon_frame dst;
	struct decon_win_rect block;
	u32 rot;
	bool is_comp;
	bool is_scale;
	bool is_block;
	enum decon_pixel_format format;
	dma_addr_t addr[MAX_PLANE_CNT];
	enum dpp_csc_eq eq_mode;
	int h_ratio;
	int v_ratio;
};

extern struct dpp_device *dpp_drvdata[2];

static inline struct dpp_device *get_dpp_drvdata(u32 id)
{
	if (id >= MAX_DPP_CNT)
		return NULL;
	else
		return dpp_drvdata[id];
}

static inline u32 dpp_read(u32 id, u32 reg_id)
{
	struct dpp_device *dpp = get_dpp_drvdata(id);

	return readl(dpp->res.regs + reg_id);
}

static inline u32 dpp_read_mask(u32 id, u32 reg_id, u32 mask)
{
	u32 val = dpp_read(id, reg_id);

	val &= (~mask);
	return val;
}

static inline void dpp_write(u32 id, u32 reg_id, u32 val)
{
	struct dpp_device *dpp = get_dpp_drvdata(id);

	writel(val, dpp->res.regs + reg_id);
}

static inline void dpp_write_mask(u32 id, u32 reg_id, u32 val, u32 mask)
{
	struct dpp_device *dpp = get_dpp_drvdata(id);
	u32 old = dpp_read(id, reg_id);

	val = (val & mask) | (old & ~mask);
	writel(val, dpp->res.regs + reg_id);
}

/* DPU_DMA Common part */
static inline u32 dma_com_read(u32 id, u32 reg_id)
{
	struct dpp_device *dpp = get_dpp_drvdata(0);

	return readl(dpp->res.dma_com_regs + reg_id);
}

static inline u32 dma_com_read_mask(u32 id, u32 reg_id, u32 mask)
{
	u32 val = dma_com_read(id, reg_id);

	val &= (~mask);
	return val;
}

static inline void dma_com_write(u32 id, u32 reg_id, u32 val)
{
	struct dpp_device *dpp = get_dpp_drvdata(0);

	writel(val, dpp->res.dma_com_regs + reg_id);
}

static inline void dma_com_write_mask(u32 id, u32 reg_id, u32 val, u32 mask)
{
	struct dpp_device *dpp = get_dpp_drvdata(0);
	u32 old = dma_com_read(id, reg_id);

	val = (val & mask) | (old & ~mask);
	writel(val, dpp->res.dma_com_regs + reg_id);
}

/* DPU_DMA */
static inline u32 dma_read(u32 id, u32 reg_id)
{
	struct dpp_device *dpp = get_dpp_drvdata(id);

	return readl(dpp->res.dma_regs + reg_id);
}

static inline u32 dma_read_mask(u32 id, u32 reg_id, u32 mask)
{
	u32 val = dma_read(id, reg_id);

	val &= (~mask);
	return val;
}

static inline void dma_write(u32 id, u32 reg_id, u32 val)
{
	struct dpp_device *dpp = get_dpp_drvdata(id);

	writel(val, dpp->res.dma_regs + reg_id);
}

static inline void dma_write_mask(u32 id, u32 reg_id, u32 val, u32 mask)
{
	struct dpp_device *dpp = get_dpp_drvdata(id);
	u32 old = dma_read(id, reg_id);

	val = (val & mask) | (old & ~mask);
	writel(val, dpp->res.dma_regs + reg_id);
}

static inline void dpp_select_format(struct dpp_device *dpp,
			struct dpp_img_format *vi, struct dpp_params_info *p)
{
	struct decon_win_config *config = dpp->config;

	vi->vgr = is_vgr(dpp);
	vi->normal = is_normal(config);
	vi->rot = p->rot;
	vi->scale = p->is_scale;
	vi->format = p->format;
	vi->afbc_en = p->is_comp;
	vi->yuv = is_yuv(config);
	vi->yuv422 = is_yuv422(config);
	vi->yuv420 = is_yuv420(config);
}

int dpp_probe(unsigned int id, unsigned long addr);
/* DPU DMA low-level APIs exposed to DPP driver */
u32 dma_reg_get_irq_status(u32 id);
void dma_reg_clear_irq(u32 id, u32 irq);

/* BIST mode */
void dma_reg_set_test_pattern(u32 id, u32 pat_id, u32 pat_dat[4]);
void dma_reg_set_ch_map(u32 id, u32 dpp_id, u32 to_pat);
void dma_reg_set_test_en(u32 id, u32 en);

/* DPP low-level APIs exposed to DPP driver */
void dpp_reg_init(u32 id);
int dpp_reg_deinit(u32 id, bool reset);
void dpp_reg_configure_params(u32 id, struct dpp_params_info *p);
u32 dpp_reg_get_irq_status(u32 id);
void dpp_reg_clear_irq(u32 id, u32 irq);
void dpp_constraints_params(struct dpp_size_constraints *vc,
					struct dpp_img_format *vi);
int dpp_reg_wait_idle_status(int id, unsigned long timeout);
void dma_reg_set_recovery_num(u32 id, u32 rcv_num);

/* DPU DMA DEBUG */
void dma_reg_set_debug(u32 id);
void dma_reg_set_common_debug(u32 id);

#endif /* __SAMSUNG_DPP_H__ */
