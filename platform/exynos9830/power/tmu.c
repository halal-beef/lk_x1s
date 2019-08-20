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
#include <lib/font_display.h>

void display_tmu_info(void)
{
	unsigned int tmu_base[2] = { EXYNOS9830_TMU_TOP_BASE,
		                     EXYNOS9830_TMU_SUB_BASE };
	unsigned int tmu_probe_num[2] = { EXYNOS9830_TMU_TOP_PROBE,
		                          EXYNOS9830_TMU_SUB_PROBE };
	unsigned int ps_hold_control;
	unsigned int i;

	for (i = 0; i < 2; i++) {
		unsigned int trim, ctrl, con1, avgc, offset, shift;
		unsigned int is_2point_calib;
		unsigned int buf_vref_sel;
		unsigned int buf_slope_sel;
		unsigned int avg_mode;
		unsigned int rising_threshold;
		unsigned int counter_value;
		unsigned int bgr_i_trim, vref_trim, vbe_i_trim;
		unsigned int triminfo;
		unsigned int main_sensor;

		int fuse_25;
		int fuse_85;
		int tmu_temp, temp_code, trip_code;

		/* GET OTP Informations */
		triminfo = readl(tmu_base[i] + EXYNOS9830_TMU_TRIMINFO_CONFIG);
		is_2point_calib = (triminfo >> 0) & 0x1;
		avg_mode = (triminfo >> 1) & 0x3;
		buf_slope_sel = (triminfo >> 3) & 0x1f;
		buf_vref_sel = (triminfo >> 8) & 0x1f;
		vbe_i_trim = (triminfo >> 13) & 0xf;
		vref_trim = (triminfo >> 17) & 0xf;
		bgr_i_trim = (triminfo >> 21) & 0xf;

		/* SET BUF_VREF_SEL, BUF_SLOPE_SEL to TMU_CONTROL */
		ctrl = readl(tmu_base[i] + EXYNOS9830_TMU_CONTROL);
		ctrl &= ~((0x1f << 24) | (0x1 << 12) | (0x1f << 4) | (0x1 << 0));
		ctrl |= buf_vref_sel << 24;
		ctrl |= buf_slope_sel << 4;
		writel(ctrl, tmu_base[i] + EXYNOS9830_TMU_CONTROL);

		/* set NUM_PROBE */
		con1 = readl(tmu_base[i] + EXYNOS9830_TMU_CONTROL1);
		con1 &= ~((0xf << 16) | (0x1 << 10));
		con1 |= tmu_probe_num[i] << 16;
		writel(con1, tmu_base[i] + EXYNOS9830_TMU_CONTROL1);

		/* set AVG_MODE */
		avgc = readl(tmu_base[i] + EXYNOS9830_TMU_AVG_CONTROL);
		avgc &= ~((0x1 << 4) | (0x7 << 0));
		if (avg_mode >= 0x2)
			avgc |= (0x1 << 4) | (0x1 << 2) | avg_mode;
		writel(avgc, tmu_base[i] + EXYNOS9830_TMU_AVG_CONTROL);

		/* set COUNTER_VALUE */
		counter_value = readl(tmu_base[i] + EXYNOS9830_TMU_COUNTER_VALUE0);
		counter_value &= ~0xffff;
		counter_value |= 0x028A;
		writel(counter_value, tmu_base[i] + EXYNOS9830_TMU_COUNTER_VALUE0);

		counter_value = readl(tmu_base[i] + EXYNOS9830_TMU_COUNTER_VALUE1);
		counter_value &= ~(0xffff << 16);
		counter_value |= (0x028A << 16);
		writel(counter_value, tmu_base[i] + EXYNOS9830_TMU_COUNTER_VALUE1);

		/*
		 * set TRIM0 BGR_I/VREF/VBE_I
		 * write TRIM0 values read from TMU_TOP to each TMU_TOP and TMU_SUB*/
		trim = readl(tmu_base[i] + EXYNOS9830_TMU_TRIM0);
		trim &= ~(EXYNOS9830_TMU_BGRI_TRIM_MASK << EXYNOS9830_TMU_BGRI_TRIM_SHIFT);
		trim &= ~(EXYNOS9830_TMU_VREF_TRIM_MASK << EXYNOS9830_TMU_VREF_TRIM_SHIFT);
		trim &= ~(EXYNOS9830_TMU_VBEI_TRIM_MASK << EXYNOS9830_TMU_VBEI_TRIM_SHIFT);
		trim |= (bgr_i_trim << EXYNOS9830_TMU_BGRI_TRIM_SHIFT);
		trim |= (vref_trim << EXYNOS9830_TMU_VREF_TRIM_SHIFT);
		trim |= (vbe_i_trim << EXYNOS9830_TMU_VBEI_TRIM_SHIFT);
		writel(trim, tmu_base[i] + EXYNOS9830_TMU_TRIM0);

		/* Get calibration information of sensor */
		main_sensor = (i == 0) ? 1 : 0;
		trim = readl(tmu_base[i] + EXYNOS9830_TMU_TRIMINFO(main_sensor));
		fuse_25 = trim & EXYNOS9830_TMU_CURRENT_TEMP_MASK;
		fuse_85 = (trim >> 9) & EXYNOS9830_TMU_CURRENT_TEMP_MASK;

		/* enable HW TRIP */
		if (is_2point_calib)
			trip_code = (EXYNOS_TMU_HWTRIP_TEMP - 25) *
			            (fuse_85 - fuse_25) / (85 - 25) + fuse_25;
		else
			trip_code = EXYNOS_TMU_HWTRIP_TEMP + fuse_25 - 25;

		rising_threshold = readl(tmu_base[i] + EXYNOS9830_TMU_TEMP_RISE7_6_REG(main_sensor));
		rising_threshold &= ~(EXYNOS9830_TMU_CURRENT_TEMP_MASK << 16);
		rising_threshold |= trip_code << 16;
		writel(rising_threshold, tmu_base[i] + EXYNOS9830_TMU_TEMP_RISE7_6_REG(main_sensor));
		writel((0x1 << 7), tmu_base[i] + EXYNOS9830_TMU_INTEN_REG(main_sensor));

		/* enable CORE_EN */
		ctrl |= (0x1 << 12);
		ctrl |= 0x1;
		writel(ctrl, tmu_base[i] + EXYNOS9830_TMU_CONTROL);

		if (!fuse_25) {
			printf("[TMU:%d] Not calibrated\n", i);
			continue;
		}
		mdelay(10);

		offset = EXYNOS9830_TMU_CURRENT_TEMP_REG(main_sensor);
		shift = EXYNOS9830_TMU_CURRENT_TEMP_SHIFT(main_sensor);

		temp_code = (readl(tmu_base[i] + offset) >> shift) & EXYNOS9830_TMU_CURRENT_TEMP_MASK;

		if (is_2point_calib)
			tmu_temp = (temp_code - fuse_25) *
			           (85 - 25) / (fuse_85 - fuse_25) + 25;
		else
			tmu_temp = temp_code - fuse_25 + 25;

		printf("[TMU:%d] %d\n", i, tmu_temp);
	}

	/* enable PMU HW TRIP */
	ps_hold_control = readl(EXYNOS9830_POWER_BASE + EXYNOS9830_PMU_PSHOLD_CONTROL);
	ps_hold_control |= (1 << 31);
	writel(ps_hold_control, EXYNOS9830_POWER_BASE + EXYNOS9830_PMU_PSHOLD_CONTROL);
}

void display_trip_info(void)
{
	unsigned int tmu_trip, pshold_trip;
	unsigned int tmu_top_trip, tmu_sub_trip;

	tmu_trip = readl(EXYNOS9830_POWER_BASE + EXYNOS9830_PMU_PS_HOLD_HW_TRIP) & 0x1;
	pshold_trip = readl(EXYNOS9830_POWER_BASE + EXYNOS9830_PMU_PS_HOLD_SW_TRIP) & 0x1;

	tmu_top_trip = readl(EXYNOS9830_POWER_BASE + EXYNOS9830_PMU_TMU_TOP_TRIP);
	tmu_sub_trip = readl(EXYNOS9830_POWER_BASE + EXYNOS9830_PMU_TMU_SUB_TRIP);

	/* Print trip information */
	if (tmu_trip && pshold_trip) {
		printf("\n TMU TRIP & PSHOLD low Detected.\n");
		print_lcd_update(FONT_GREEN, FONT_BLACK, "\n TMU TRIP & PSHOLD low Detected.\n");
		writel(0x0, EXYNOS9830_POWER_BASE + EXYNOS9830_PMU_PS_HOLD_HW_TRIP);
		writel(0x0, EXYNOS9830_POWER_BASE + EXYNOS9830_PMU_PS_HOLD_SW_TRIP);
	} else if (tmu_trip) {
		printf("\n TMU TRIP Detected.\n");
		print_lcd_update(FONT_GREEN, FONT_BLACK, "\n TMU TRIP Detected.\n");
		writel(0x0, EXYNOS9830_POWER_BASE + EXYNOS9830_PMU_PS_HOLD_HW_TRIP);
	} else if (pshold_trip) {
		printf("\n PSHOLD low Detected.\n");
		print_lcd_update(FONT_GREEN, FONT_BLACK, "\n PSHOLD low Detected.\n");
		writel(0x0, EXYNOS9830_POWER_BASE + EXYNOS9830_PMU_PS_HOLD_SW_TRIP);
	}

	/* Print trip sensor information */
	if (tmu_trip) {
		printf("[TMU TRIP] 0x%x, 0x%x\n", tmu_top_trip, tmu_sub_trip);
		print_lcd_update(FONT_GREEN, FONT_BLACK, "[TMU TRIP] 0x%x, 0x%x\n", tmu_top_trip, tmu_sub_trip);
		writel(0x0, EXYNOS9830_POWER_BASE + EXYNOS9830_PMU_TMU_TOP_TRIP);
		writel(0x0, EXYNOS9830_POWER_BASE + EXYNOS9830_PMU_TMU_SUB_TRIP);
	}
}
