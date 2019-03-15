/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __TMU_H__
#define __TMU_H__

void display_tmu_info(void);
void display_trip_info(void);

#define	EXYNOS_TMU_HWTRIP_TEMP			(115)

/* Exynos 9830 */
#define EXYNOS9830_TMU_TRIMINFO_CONFIG		0x0000
#define EXYNOS9830_TMU_TRIMINFO_0		0x0010
#define EXYNOS9830_TMU_TRIMINFO(i) (EXYNOS9830_TMU_TRIMINFO_0 + (i) * 0x4)

#define EXYNOS9830_TMU_CONTROL			0x0050
#define EXYNOS9830_TMU_CONTROL1			0x0054
#define EXYNOS9830_TMU_AVG_CONTROL		0x0058

#define EXYNOS9830_TMU_COUNTER_VALUE0		0x0074
#define EXYNOS9830_TMU_COUNTER_VALUE1		0x0078
#define EXYNOS9830_TMU_TRIM0			0x005C

#define EXYNOS9830_TMU_CURRENT_TEMP1_0		0x0084
#define EXYNOS9830_TMU_CURRENT_TEMP_REG(i)	(EXYNOS9830_TMU_CURRENT_TEMP1_0 + 0x4 * ((i) / 2))
#define EXYNOS9830_TMU_CURRENT_TEMP_SHIFT(i)	(((i) % 2) * 16)
#define EXYNOS9830_TMU_CURRENT_TEMP_MASK	0x1ff

#define EXYNOS9830_TMU_P0_TEMP_RISE7_6		0x00D0
#define EXYNOS9830_TMU_TEMP_RISE7_6_REG(i)	(EXYNOS9830_TMU_P0_TEMP_RISE7_6 + 0x50 * (i))
#define EXYNOS9830_TMU_TEMP_RISE7_6_SHIFT	16
#define EXYNOS9830_TMU_TEMP_RISE7_6_MASK	0x1ff

#define EXYNOS9830_TMU_INTEN_P0			0x00F0
#define EXYNOS9830_TMU_INTEN_REG(i) (EXYNOS9830_TMU_INTEN_P0 + 0x50 * (i))
#define EXYNOS9830_TMU_INTEN_SHIFT		7
#define EXYNOS9830_TMU_INTEN_MASK		0x1

#define EXYNOS9830_TMU_T_TRIM0_SHIFT		18
#define EXYNOS9830_TMU_T_TRIM0_MASK		0xf
#define EXYNOS9830_TMU_BGRI_TRIM_SHIFT		20
#define EXYNOS9830_TMU_BGRI_TRIM_MASK		0xf
#define EXYNOS9830_TMU_VREF_TRIM_SHIFT		12
#define EXYNOS9830_TMU_VREF_TRIM_MASK		0xf
#define EXYNOS9830_TMU_VBEI_TRIM_SHIFT		8
#define EXYNOS9830_TMU_VBEI_TRIM_MASK		0xf

#define EXYNOS9830_TMU_TOP_PROBE		(9)
#define EXYNOS9830_TMU_SUB_PROBE		(10)

#define EXYNOS9830_PMU_PSHOLD_CONTROL		0x030c
#define EXYNOS9830_PMU_PS_HOLD_HW_TRIP		0x0820
#define EXYNOS9830_PMU_PS_HOLD_SW_TRIP		0x0824
#define EXYNOS9830_PMU_TMU_TOP_TRIP		0x0e20
#define EXYNOS9830_PMU_TMU_SUB_TRIP		0x0e24
#endif /*__TMU_H__*/
