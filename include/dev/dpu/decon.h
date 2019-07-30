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

#ifndef __DECON_CORE_H__
#define __DECON_CORE_H__

#include <stdio.h>
#include <errno.h>
#include <dev/dpu/exynos_panel.h>
#include <dev/dpu/dsim.h>
#include <dev/dpu/mipi_dsi_cmd.h>
#include <platform/sfr.h>
#include <platform/display_sfr.h>
#include <platform/delay.h>
#include <platform/dpu_cal/regs-decon.h>
#include <platform/dpu_cal/decon_cal.h>

#ifndef __iomem
#define __iomem
#endif

typedef u32 dma_addr_t;
typedef uint32_t __u32;


#define SUCCESS_EXYNOS_SMC	0

#define MAX_DECON_CNT		3
#define MAX_DECON_WIN		6
#define MAX_DPP_SUBDEV		7

#define DECON_MODULE_NAME	"exynos-decon"
#define MAX_NAME_SIZE		32
#define MAX_PLANE_CNT		3
#define MAX_PLANE_ADDR_CNT	4
#define DECON_ENTER_HIBER_CNT	4
#define MIN_BLK_MODE_WIDTH	144
#define MIN_BLK_MODE_HEIGHT	16
#define VSYNC_TIMEOUT_MSEC	200
#define DEFAULT_BPP		32
#define MIN_WIN_BLOCK_WIDTH	8
#define MIN_WIN_BLOCK_HEIGHT	1
#define FD_TRY_CNT		3
#define VALID_FD_VAL		3
#define DECON_TRACE_BUF_SIZE	40

#define DECON_WIN_UPDATE_IDX	MAX_DECON_WIN
#define MAX_BUF_MEMMAP		512

#ifndef KHZ
#define KHZ (1000)
#endif
#ifndef MHZ
#define MHZ (1000*1000)
#endif
#ifndef MSEC
#define MSEC (1000)
#endif

#define SHADOW_UPDATE_TIMEOUT	(300 * 1000) /* 300ms */
#define IDLE_WAIT_TIMEOUT	(50 * 1000) /* 50ms */
#define RUN_WAIT_TIMEOUT	IDLE_WAIT_TIMEOUT
#define DSC_INIT_XMIT_DELAY	0x200

#define EINT_PEND(x)		((x == 0) ? 2 : ((x == 1) ? 4 : 1))

#define MAX_DSC_SLICE_CNT	4

extern int decon_log_level;
#define decon_err(fmt, ...)							\
	do {									\
		if (decon_log_level >= 3)					\
			printf(fmt, ##__VA_ARGS__);				\
	} while (0)

#define decon_warn(fmt, ...)							\
	do {									\
		if (decon_log_level >= 4)					\
			printf(fmt, ##__VA_ARGS__);				\
	} while (0)

#define decon_info(fmt, ...)							\
	do {									\
		if (decon_log_level >= 6)					\
			printf(fmt, ##__VA_ARGS__);				\
	} while (0)

#define decon_dbg(fmt, ...)							\
	do {									\
		if (decon_log_level >= 7)					\
			printf(fmt, ##__VA_ARGS__);				\
	} while (0)

/* TODO : array drvdata */
extern struct decon_device *decon_drvdata[MAX_DECON_CNT];
extern struct dsim_device *dsim0_for_decon;

enum decon_hold_scheme {
	/*  should be set to this value in case of DSIM video mode */
	DECON_VCLK_HOLD_ONLY		= 0x00,
	/*  should be set to this value in case of DSIM command mode */
	DECON_VCLK_RUNNING_VDEN_DISABLE = 0x01,
	DECON_VCLK_HOLD_VDEN_DISABLE	= 0x02,
	/*  should be set to this value in case of HDMI, eDP */
	DECON_VCLK_NOT_AFFECTED		= 0x03,
};

enum decon_win_alpha_coef {
	BND_COEF_ZERO			= 0x0,
	BND_COEF_ONE			= 0x1,
	BND_COEF_AF			= 0x2,
	BND_COEF_1_M_AF		= 0x3,
	BND_COEF_AB			= 0x4,
	BND_COEF_1_M_AB		= 0x5,
	BND_COEF_PLNAE_ALPHA0		= 0x6,
	BND_COEF_1_M_PLNAE_ALPHA0	= 0x7,
	BND_COEF_PLNAE_ALPHA1		= 0x8,
	BND_COEF_1_M_PLNAE_ALPHA1	= 0x9,
	BND_COEF_ALPHA_MULT		= 0xA,
	BND_COEF_1_M_ALPHA_MULT	= 0xB,
};

enum decon_win_alpha_sel {
	ALPHA_MULT_SRC_SEL_ALPHA0 = 0,
	ALPHA_MULT_SRC_SEL_ALPHA1 = 1,
	ALPHA_MULT_SRC_SEL_AF = 2,
	ALPHA_MULT_SRC_SEL_AB = 3,
};

enum decon_merger_mode {
	DECON_LRM_NO		= 0x0,
	DECON_LRM_NOSWAP_RF	= 0x4,
	DECON_LRM_NOSWAP_LF	= 0x5,
	DECON_LRM_SWAP_RF	= 0x6,
	DECON_LRM_SWAP_LF	= 0x7,
};

enum decon_te_src {
	DECON_TE_FROM_DDI0 = 0,
	DECON_TE_FROM_DDI1,
	DECON_TE_FROM_DDI2,
	DECON_TE_FROM_USB,
};

/*
 * DECON_STATE_ON : disp power on, decon/dsim clock on & lcd on
 * DECON_HIBER : disp power off, decon/dsim clock off & lcd on
 * DECON_STATE_OFF : disp power off, decon/dsim clock off & lcd off
 */
enum decon_state {
	DECON_STATE_INIT = 0,
	DECON_STATE_ON,
	DECON_STATE_DOZE,
	DECON_STATE_HIBER,
	DECON_STATE_DOZE_SUSPEND,
	DECON_STATE_OFF,
	DECON_STATE_TUI,
};

/* To find a proper CLOCK ratio */
enum decon_clk_id {
	CLK_ID_VCLK = 0,
	CLK_ID_ECLK,
	CLK_ID_ACLK,
	CLK_ID_PCLK,
	CLK_ID_DPLL, /* DPU_PLL */
	CLK_ID_RESOLUTION,
	CLK_ID_MIC_RATIO,
	CLK_ID_DSC_RATIO,
	CLK_ID_MAX,
};

enum decon_dsc_id {
	DECON_DSC_ENC0 = 0x0,
	DECON_DSC_ENC1 = 0x1,
	DECON_DSC_ENC2 = 0x2,
};

enum decon_share_path {
	SHAREPATH_DQE_USE		= 0x0,
	SHAREPATH_VG0_USE		= 0x1,
	SHAREPATH_VG1_USE		= 0x2,
	SHAREPATH_VGF1_USE		= 0x3,
	SHAREPATH_VGF0_USE		= 0x4,
};

enum dpp_rotate {
	DPP_ROT_NORMAL = 0x0,
	DPP_ROT_XFLIP,
	DPP_ROT_YFLIP,
	DPP_ROT_180,
	DPP_ROT_90,
	DPP_ROT_90_XFLIP,
	DPP_ROT_90_YFLIP,
	DPP_ROT_270,
};

enum dpp_csc_eq {
	/* eq_mode : 6bits [5:0] */
	CSC_STANDARD_SHIFT = 0,
	CSC_BT_601 = 0,
	CSC_BT_709 = 1,
	CSC_BT_2020 = 2,
	CSC_DCI_P3 = 3,
	CSC_BT_601_625 = 4,
	CSC_BT_601_625_UNADJUSTED = 5,
	CSC_BT_601_525 = 6,
	CSC_BT_601_525_UNADJUSTED = 7,
	CSC_BT_2020_CONSTANT_LUMINANCE = 8,
	CSC_BT_470M = 9,
	CSC_FILM = 10,
	CSC_ADOBE_RGB = 11,
	CSC_STANDARD_UNSPECIFIED = 63,
	/* eq_mode : 3bits [8:6] */
	CSC_RANGE_SHIFT = 6,
	CSC_RANGE_LIMITED = 0x0,
	CSC_RANGE_FULL = 0x1,
	CSC_RANGE_UNSPECIFIED = 7,
};

enum dpp_comp_src {
	DPP_COMP_SRC_NONE = 0,
	DPP_COMP_SRC_G2D,
	DPP_COMP_SRC_GPU
};

enum dpp_comp_type {
	COMP_TYPE_NONE = 0,
	COMP_TYPE_AFBC,
	COMP_TYPE_SBWC,
	COMP_TYPE_CSET,
};

enum dpp_hdr_standard {
	DPP_HDR_OFF = 0,
	DPP_HDR_ST2084,
	DPP_HDR_HLG,
};

enum decon_color_mode {
	HAL_COLOR_MODE_NATIVE = 0,
	HAL_COLOR_MODE_NUM_MAX,
};

struct decon_color_mode_info {
	int index;
	u32 color_mode;
};

struct decon_clocks {
	unsigned long decon[CLK_ID_DPLL + 1];
};

struct decon_dma_buf_data {
	struct ion_handle		*ion_handle;
	struct dma_buf			*dma_buf;
	struct dma_buf_attachment	*attachment;
	struct sg_table			*sg_table;
	dma_addr_t			dma_addr;
	struct sync_file		*fence;
};

struct decon_win_rect {
	int x;
	int y;
	u32 w;
	u32 h;
};

struct decon_rect {
	u32 left;
	u32 top;
	u32 right;
	u32 bottom;
};

struct dpp_params {
	dma_addr_t addr[MAX_PLANE_CNT];
	enum dpp_rotate rot;
	enum dpp_csc_eq eq_mode;
	enum dpp_comp_src comp_src;
	enum dpp_hdr_standard hdr_std;
	u32 min_luminance;
	u32 max_luminance;
};

struct decon_frame {
	int x;
	int y;
	u32 w;
	u32 h;
	u32 f_w;
	u32 f_h;
};

struct decon_win_config {
	enum {
		DECON_WIN_STATE_DISABLED = 0,
		DECON_WIN_STATE_COLOR,
		DECON_WIN_STATE_BUFFER,
		DECON_WIN_STATE_UPDATE,
		DECON_WIN_STATE_CURSOR,
		DECON_WIN_STATE_MRESOL = 0x10000,
	} state;

	/* Reusability:This struct is used for IDMA and ODMA */
	union {
		__u32 color;
		struct {
			int				fd_idma[3];
			int				acq_fence;
			int				rel_fence;
			int				plane_alpha;
			enum decon_blending		blending;
			/*
			 * TODO: channel will be changed to channel number in the future.
			 *
			 * Although the variable name is channel, it indicates
			 * DPP channel number currently.
			 */
			u32				channel;
			enum decon_pixel_format		format;
			struct dpp_params		dpp_parm;
			/* no read area of IDMA */
			struct decon_win_rect		block_area;
			struct decon_win_rect		transparent_area;
			struct decon_win_rect		opaque_area;
			/* source framebuffer coordinates */
			struct decon_frame		src;
		};
	};

	/* destination OSD coordinates */
	struct decon_frame dst;
	bool protection;
	bool compression;
};

struct decon_reg_data {
	u32 num_of_window;
	int plane_cnt[MAX_DECON_WIN + 1];
	struct decon_rect blender_bg;
	struct decon_win_config dpp_config[MAX_DECON_WIN + 1];
	struct decon_win_rect block_rect[MAX_DECON_WIN];
	struct decon_window_regs win_regs[MAX_DECON_WIN];
	struct decon_dma_buf_data dma_buf_data[MAX_DECON_WIN + 1][MAX_PLANE_CNT];

	/*
	 * If window update size is changed, that size has to be applied to
	 * DECON, DSIM and panel in case of below
	 * - full size -> partial size
	 * - partial size -> different partial size
	 * - partial size -> full size
	 *
	 * need_update flag indicates whether changes are applied to hw or not
	 */
	bool need_update;
	/* current update region */
	struct decon_rect up_region;
	/* protected contents playback */
	bool protection[MAX_DECON_WIN + 1];
	/* cursor async */
	bool is_cursor_win[MAX_DECON_WIN];
	int cursor_win;

	bool mres_update;
	u32 lcd_width;
	u32 lcd_height;
	int mres_idx;
};

struct decon_win_config_data {
	int	retire_fence;
	int	fd_odma;
	struct decon_win_config config[MAX_DECON_WIN + 1];
};

enum hwc_ver {
	HWC_INIT = 0,
	HWC_1_0 = 1,
	HWC_2_0 = 2,
};

struct decon_disp_info {
	enum hwc_ver ver;
	enum decon_psr_mode psr_mode;
	struct lcd_mres_info mres_info;
	u32 chip_ver;
	unsigned char reverved[128];
};

struct dpu_size_info {
	u32 w_in;
	u32 h_in;
	u32 w_out;
	u32 h_out;
};

struct decon_resources {
	void __iomem *regs;
	void __iomem *ss_regs;
};

struct decon_dt_info {
	enum decon_psr_mode psr_mode;
	enum decon_trig_mode trig_mode;
	enum decon_dsi_mode dsi_mode;
	enum decon_out_type out_type;
	//unsigned int out_idx[MAX_DSIM_CNT];
	unsigned int out_idx;
	unsigned int max_win;
	unsigned int dft_win;
	unsigned int dft_ch;
	unsigned int dpp_cnt;
	unsigned int dsim_cnt;
	unsigned int decon_cnt;
	int chip_ver;

	void __iomem *ss_regs; /* TODO : check */
};

struct decon_device {
	int	id;
	enum decon_state state;
	struct decon_dt_info *dt;
	struct decon_win *win[MAX_DECON_WIN];
	struct decon_resources res;
	struct exynos_panel_info *lcd_info;
};

/* TODO : add num of decon */
static inline struct decon_device *get_decon_drvdata(u32 id)
{
	return decon_drvdata[id];
}

/* register access subroutines */
static inline u32 decon_read(u32 id, u32 reg_id)
{
	struct decon_device *decon = get_decon_drvdata(id);
	return readl(decon->res.regs + reg_id);
}

static inline u32 decon_read_mask(u32 id, u32 reg_id, u32 mask)
{
	u32 val = decon_read(id, reg_id);
	val &= (mask);
	return val;
}

static inline void decon_write(u32 id, u32 reg_id, u32 val)
{
	struct decon_device *decon = get_decon_drvdata(id);
	writel(val, decon->res.regs + reg_id);
}

static inline void decon_write_mask(u32 id, u32 reg_id, u32 val, u32 mask)
{
	u32 old = decon_read(id, reg_id);

	val = (val & mask) | (old & ~mask);
	decon_write(id, reg_id, val);
}

static inline u32 dsc_read(u32 dsc_id, u32 reg_id)
{
	struct decon_device *decon = get_decon_drvdata(0);
	u32 dsc_offset = dsc_id ? DSC1_OFFSET : DSC0_OFFSET;

	return readl(decon->res.regs + dsc_offset + reg_id);
}

static inline void dsc_write(u32 dsc_id, u32 reg_id, u32 val)
{
	struct decon_device *decon = get_decon_drvdata(0);
	u32 dsc_offset = dsc_id ? DSC1_OFFSET : DSC0_OFFSET;

	writel(val, decon->res.regs + dsc_offset + reg_id);
}

static inline void dsc_write_mask(u32 dsc_id, u32 reg_id, u32 val, u32 mask)
{
	u32 old = dsc_read(dsc_id, reg_id);

	val = (val & mask) | (old & ~mask);
	dsc_write(dsc_id, reg_id, val);
}

static inline u32 sysreg_read(u32 id, u32 reg_id)
{
	struct decon_device *decon = get_decon_drvdata(id);
	return readl(decon->res.ss_regs + reg_id);
}

static inline u32 sysreg_read_mask(u32 id, u32 reg_id, u32 mask)
{
	u32 val = sysreg_read(id, reg_id);
	val &= (mask);
	return val;
}

static inline void sysreg_write(u32 id, u32 reg_id, u32 val)
{
	struct decon_device *decon = get_decon_drvdata(id);
	writel(val, decon->res.ss_regs + reg_id);
}

static inline void sysreg_write_mask(u32 id, u32 reg_id, u32 val, u32 mask)
{
	u32 old = sysreg_read(id, reg_id);

	val = (val & mask) | (old & ~mask);
	sysreg_write(id, reg_id, val);
}

/* common function API */
bool decon_validate_x_alignment(struct decon_device *decon, int x, u32 w,
		u32 bits_per_pixel);
int decon_wait_for_vsync(struct decon_device *decon, u32 timeout);

/* internal only function API */
u32 wincon(u32 transp_len, u32 a0, u32 a1, int plane_alpha,
		enum decon_blending blending, int idx);

static inline u32 win_start_pos(int x, int y)
{
	return (WIN_STRPTR_Y_F(y) | WIN_STRPTR_X_F(x));
}

static inline u32 win_end_pos(int x, int y,  u32 xres, u32 yres)
{
	return (WIN_ENDPTR_Y_F(y + yres - 1) | WIN_ENDPTR_X_F(x + xres - 1));
}

void decon_show_buffer_update(struct decon_device *decon,
		struct dsim_device *dsim, u32 color);
void decon_show_buffer(struct decon_device *decon,
		struct dsim_device *dsim, u32 color);
void decon_string_update(void);

/* tui feature support external to security driver(gud) */
int decon_tui_protection(bool tui_en);
int decon_reg_stop_nreset(u32 id, struct decon_mode_info *psr);
void decon_reg_update_req_global(u32 id);

int decon_reg_wait_linecnt_is_zero_timeout(u32 id, int dsi_idx, unsigned long timeout);

/* helper functions */
int dpu_get_sd_by_drvname(struct decon_device *decon, char *drvname);
u32 dpu_translate_fmt_to_dpp(u32 format);
u32 dpu_get_bpp(enum decon_pixel_format fmt);
int dpu_get_meta_plane_cnt(enum decon_pixel_format format);
int dpu_get_plane_cnt(enum decon_pixel_format format, enum dpp_hdr_standard std);
u32 dpu_get_alpha_len(int format);
void decon_to_psr_info(struct decon_device *decon, struct decon_mode_info *psr);
void decon_to_init_param(struct decon_device *decon, struct decon_param *p);
bool decon_intersect(struct decon_rect *r1, struct decon_rect *r2);
int decon_intersection(struct decon_rect *r1,
		struct decon_rect *r2, struct decon_rect *r3);
bool is_decon_rect_differ(struct decon_rect *r1, struct decon_rect *r2);
bool is_rgb32(int format);
bool is_scaling(struct decon_win_config *config);
bool is_full(struct decon_rect *r, struct exynos_panel_info *lcd);
bool is_decon_opaque_format(int format);
//void __iomem *dpu_get_sysreg_addr(void);
//void size_t dpu_get_sysreg_addr(void);
void decon_dpp_stop(struct decon_device *decon, bool do_reset);

#endif /* __DECON_CORE_H__ */
