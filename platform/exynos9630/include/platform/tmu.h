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

/* Exynos 9630 */
#define	EXYNOS9630_TMU_TRIMINFO_0_OFFSET	0x0000
#define	EXYNOS9630_TMU_TRIMINFO_1_OFFSET	0x0004
#define	EXYNOS9630_TMU_TRIMINFO_2_OFFSET	0x0008
#define	EXYNOS9630_TMU_TRIMINFO_3_OFFSET	0x000C
#define	EXYNOS9630_TMU_TRIMINFO_4_OFFSET	0x0010
#define	EXYNOS9630_TMU_TRIMINFO_5_OFFSET	0x0014
#define	EXYNOS9630_TMU_CONTROL_OFFSET		0x0050
#define	EXYNOS9630_TMU_CONTROL1_OFFSET		0x0054
#define	EXYNOS9630_TMU_AVG_CONTROL_OFFSET	0x0058
#define	EXYNOS9630_TMU_COUNTER_VALUE0_OFFSET	0x0074
#define	EXYNOS9630_TMU_COUNTER_VALUE1_OFFSET	0x0078
#define	EXYNOS9630_TMU_TRIM0_OFFSET		0x005C
#define	EXYNOS9630_TMU_CURRENT_TEMP0_1_OFFSET	0x0084
#define	EXYNOS9630_TMU_TEMP_RISE7_6_OFFSET	0x00D0
#define	EXYNOS9630_TMU_INTEN_OFFSET		0x00F0
#define	EXYNOS9630_TMU_REG_CURRENT_TEMP(p)	((p / 2) * 0x4 + EXYNOS9630_TMU_CURRENT_TEMP0_1_OFFSET)
#define	EXYNOS9630_TMU_SHIFT_CURRENT_TEMP(p)	((p % 2) * 16)

#define	EXYNOS9630_TMU_T_TRIM0_SHIFT		18
#define	EXYNOS9630_TMU_T_TRIM0_MASK		0xf
#define	EXYNOS9630_TMU_BGRI_TRIM_SHIFT		20
#define	EXYNOS9630_TMU_BGRI_TRIM_MASK		0xf
#define	EXYNOS9630_TMU_VREF_TRIM_SHIFT		12
#define	EXYNOS9630_TMU_VREF_TRIM_MASK		0xf
#define	EXYNOS9630_TMU_VBEI_TRIM_SHIFT		8
#define	EXYNOS9630_TMU_VBEI_TRIM_MASK		0xf

#define EXYNOS9630_TMU_TOP_PROBE		(9)

#endif /*__TMU_H__*/
