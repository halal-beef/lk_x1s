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

/* Exynos 9610 */
#define	EXYNOS9610_TMU_TRIMINFO_0_OFFSET	0x0000
#define	EXYNOS9610_TMU_TRIMINFO_1_OFFSET	0x0004
#define	EXYNOS9610_TMU_TRIMINFO_2_OFFSET	0x0008
#define	EXYNOS9610_TMU_TRIMINFO_3_OFFSET	0x000C
#define	EXYNOS9610_TMU_TRIMINFO_4_OFFSET	0x0010
#define	EXYNOS9610_TMU_TRIMINFO_5_OFFSET	0x0014
#define	EXYNOS9610_TMU_CONTROL_OFFSET		0x0020
#define	EXYNOS9610_TMU_CONTROL1_OFFSET		0x0024
#define	EXYNOS9610_TMU_AVG_CONTROL_OFFSET	0x0038
#define	EXYNOS9610_TMU_COUNTER_VALUE0_OFFSET	0x0030
#define	EXYNOS9610_TMU_COUNTER_VALUE1_OFFSET	0x0034
#define	EXYNOS9610_TMU_TRIM0_OFFSET		0x003C
#define	EXYNOS9610_TMU_CURRENT_TEMP0_1_OFFSET	0x0040
#define	EXYNOS9610_TMU_CURRENT_TEMP2_4_OFFSET	0x0044
#define	EXYNOS9610_TMU_CURRENT_TEMP5_7_OFFSET	0x0048
#define	EXYNOS9610_TMU_TEMP_RISE7_6_OFFSET	0x0050
#define	EXYNOS9610_TMU_INTEN_OFFSET		0x0110

#define	EXYNOS9610_TMU_T_TRIM0_SHIFT		18
#define	EXYNOS9610_TMU_T_TRIM0_MASK		0xf
#define	EXYNOS9610_TMU_BGRI_TRIM_SHIFT		20
#define	EXYNOS9610_TMU_BGRI_TRIM_MASK		0xf
#define	EXYNOS9610_TMU_VREF_TRIM_SHIFT		12
#define	EXYNOS9610_TMU_VREF_TRIM_MASK		0xf
#define	EXYNOS9610_TMU_VBEI_TRIM_SHIFT		8
#define	EXYNOS9610_TMU_VBEI_TRIM_MASK		0xf

#define EXYNOS9610_TMU_TOP_PROBE		(2)

#endif /*__TMU_H__*/
