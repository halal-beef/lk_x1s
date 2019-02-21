/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
#include <debug.h>
#include <reg.h>
#include <sys/types.h>
#include <platform/delay.h>
#include <platform/exynos9830.h>
#include <platform/tmu.h>

void display_tmu_info(void)
{
	unsigned int tmu_base = EXYNOS9610_TMU_TOP_BASE;
	unsigned int tmu_probe_num = EXYNOS9610_TMU_TOP_PROBE;
	unsigned int ps_hold_control;

	unsigned int trim, ctrl, con1, avgc, offset;
	unsigned int is_2point_calib;
	unsigned int t_buf_vref_sel;
	unsigned int t_buf_slope_sel;
	unsigned int avg_mode;
	unsigned int rising_threshold;
	unsigned int counter_value;
	unsigned int t_bgri_trim, t_vref_trim, t_vbei_trim;

	int fuse_25;
	int fuse_85;
	int tmu_temp, temp_code, trip_code;

	trim = readl(tmu_base + EXYNOS9610_TMU_TRIMINFO_0_OFFSET);
	fuse_25 = trim & 0x1ff;
	fuse_85 = (trim >> 9) & 0x1ff;
	t_buf_vref_sel = (trim >> 18) & 0x1f;
	is_2point_calib = (trim >> 23) & 0x1;

	trim = readl(tmu_base + EXYNOS9610_TMU_TRIMINFO_1_OFFSET);
	t_buf_slope_sel = (trim >> 18) & 0xf;

	trim = readl(tmu_base + EXYNOS9610_TMU_TRIMINFO_2_OFFSET);
	avg_mode = (trim >> 18) & 0x7;

	/* set BUF_VREF_SEL, BUF_SLOPE_SEL */
	ctrl = readl(tmu_base + EXYNOS9610_TMU_CONTROL_OFFSET);
	ctrl &= ~((0x1f << 24) | (0x1 << 12) | (0xf << 8) | (0x1 << 0));
	ctrl |= t_buf_vref_sel << 24;
	ctrl |= t_buf_slope_sel << 8;
	writel(ctrl, tmu_base + EXYNOS9610_TMU_CONTROL_OFFSET);

	/* set NUM_PROBE */
	con1 = readl(tmu_base + EXYNOS9610_TMU_CONTROL1_OFFSET);
	con1 &= ~((0xf << 16) | (0x1 << 10));
	con1 |= tmu_probe_num << 16;
	writel(con1, tmu_base + EXYNOS9610_TMU_CONTROL1_OFFSET);

	/* set AVG_MODE */
	avgc = readl(tmu_base + EXYNOS9610_TMU_AVG_CONTROL_OFFSET);
	avgc &= ~((0x1 << 4) | (0x7 << 0));
	if (avg_mode)
		avgc |= (0x1 << 4) | avg_mode;
	writel(avgc, tmu_base + EXYNOS9610_TMU_AVG_CONTROL_OFFSET);

        /* set COUNTER_VALUE */
        counter_value = readl(tmu_base + EXYNOS9610_TMU_COUNTER_VALUE0_OFFSET);
        counter_value &= ~0xffff;
        counter_value |= 0x0514;
        writel(counter_value, tmu_base + EXYNOS9610_TMU_COUNTER_VALUE0_OFFSET);

        counter_value = readl(tmu_base + EXYNOS9610_TMU_COUNTER_VALUE1_OFFSET);
        counter_value &= ~(0xffff << 16);
        counter_value |= (0x0514 << 16);
        writel(counter_value, tmu_base + EXYNOS9610_TMU_COUNTER_VALUE1_OFFSET);

	/* set TRIM0 BGR_I/VREF/VBE_I */
	/* write TRIM0 values read from TMU_TOP to each TMU_TOP and TMU_SUB */
	trim = readl(tmu_base + EXYNOS9610_TMU_TRIMINFO_3_OFFSET);
	t_bgri_trim = (trim >> 18) & 0xf;
	trim = readl(tmu_base + EXYNOS9610_TMU_TRIMINFO_3_OFFSET);
	t_vref_trim = (trim >> 14) & 0xf;
	trim = readl(tmu_base + EXYNOS9610_TMU_TRIMINFO_3_OFFSET);
	t_vbei_trim = (trim >> 10) & 0xf;

	trim = readl(tmu_base + EXYNOS9610_TMU_TRIM0_OFFSET);
	trim &= ~(EXYNOS9610_TMU_BGRI_TRIM_MASK << EXYNOS9610_TMU_BGRI_TRIM_SHIFT);
	trim &= ~(EXYNOS9610_TMU_VREF_TRIM_MASK << EXYNOS9610_TMU_VREF_TRIM_SHIFT);
	trim &= ~(EXYNOS9610_TMU_VBEI_TRIM_MASK << EXYNOS9610_TMU_VBEI_TRIM_SHIFT);
	trim |= (t_bgri_trim << EXYNOS9610_TMU_BGRI_TRIM_SHIFT);
	trim |= (t_vref_trim << EXYNOS9610_TMU_VREF_TRIM_SHIFT);
	trim |= (t_vbei_trim << EXYNOS9610_TMU_VBEI_TRIM_SHIFT);
	writel(trim, tmu_base + EXYNOS9610_TMU_TRIM0_OFFSET);

	/* enable HW TRIP */
	if (is_2point_calib) {
		trip_code = (EXYNOS_TMU_HWTRIP_TEMP - 25) *
			(fuse_85 - fuse_25) / (85 - 25) + fuse_25;
	} else
		trip_code = EXYNOS_TMU_HWTRIP_TEMP + fuse_25 - 25;

	rising_threshold = readl(tmu_base + EXYNOS9610_TMU_TEMP_RISE7_6_OFFSET);
	rising_threshold &= ~(0x1ff << 16);
	rising_threshold |= trip_code << 16;
	writel(rising_threshold, tmu_base + EXYNOS9610_TMU_TEMP_RISE7_6_OFFSET);
	writel((0x1 << 7), tmu_base + EXYNOS9610_TMU_INTEN_OFFSET);
	ctrl |= (0x1 << 12);

	/* enable CORE_EN */
	ctrl |= 0x1;
	writel(ctrl, tmu_base + EXYNOS9610_TMU_CONTROL_OFFSET);

	if (!fuse_25)
		printf("[TMU]Not calibrated\n");

	u_delay(10000);

	offset = EXYNOS9610_TMU_CURRENT_TEMP0_1_OFFSET;
	temp_code = readl(tmu_base + offset) & 0x1ff;

	if (is_2point_calib) {
		tmu_temp = (temp_code - fuse_25) *
			(85 - 25) / (fuse_85 - fuse_25) + 25;
	} else
		tmu_temp = temp_code - fuse_25 + 25;

	printf("[TMU:%d]\n", tmu_temp);

	/* enable PMU HW TRIP */
	ps_hold_control = readl(EXYNOS9610_POWER_BASE + 0x330c);
	ps_hold_control |= (1 << 31);
	writel(ps_hold_control, EXYNOS9610_POWER_BASE + 0x330c);
}

void display_trip_info(void)
{
	unsigned int hw_trip, sw_trip;

	hw_trip = readl(EXYNOS9610_POWER_BASE + 0x0820) & 0x1;
	sw_trip = readl(EXYNOS9610_POWER_BASE + 0x0824) & 0x1;

	if (hw_trip && sw_trip) {
		printf("\n SW/HW TRIP Detected.\n");
		writel(0x0, EXYNOS9610_POWER_BASE + 0x0820);
		writel(0x0, EXYNOS9610_POWER_BASE + 0x0824);
	} else if (hw_trip) {
		printf("\n HW TRIP Detected.\n");
		writel(0x0, EXYNOS9610_POWER_BASE + 0x0820);
	} else if (sw_trip) {
		printf("\n SW TRIP Detected.\n");
		writel(0x0, EXYNOS9610_POWER_BASE + 0x0824);
	}
}
