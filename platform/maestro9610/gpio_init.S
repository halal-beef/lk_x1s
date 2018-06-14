/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
#include <platform/sfr.h>

	.global display_gpio_init
display_gpio_init:
#if 1
	/* defined (CONFIG_MACH_MAESTRO9610) */
	/* Set MLCD_RSTB (GPG1_4) to Pull-up/down disable */
	ldr	x0, =EXYNOS9610_GPG1PUD
	ldr	w1, [x0]
	bic	w1, w1, #(0xF << 16)
	str	w1, [x0]

	/* Set MLCD_RSTB (GPG1_4) to Output */
	ldr	x0, =EXYNOS9610_GPG1CON
	ldr	w1, [x0]
	bic	w1, w1, #(0xF << 16)
	orr	w1, w1, #(0x1 << 16)
	str	w1, [x0]

	/* Set MLCD_RSTB (GPG1_4) to High */
	ldr	x0, =EXYNOS9610_GPG1DAT
	ldr	w1, [x0]
	orr	w1, w1, #(0x1 << 4)
	str	w1, [x0]

	/* Set LCD_TE_HW (GPC2_3) to Pull-up/down disable */
	ldr	x0, =EXYNOS9610_GPC2PUD
	ldr	w1, [x0]
	bic	w1, w1, #(0xF << 12)
	str	w1, [x0]

	/* Set LCD_TE_HW (GPC2_3) to Output */
	ldr	x0, =EXYNOS9610_GPC2CON
	ldr	w1, [x0]
	bic	w1, w1, #(0xF << 12)
	orr	w1, w1, #(0x1 << 12)
	str	w1, [x0]

	/* Set LCD_TE_HW (GPC2_3) to High */
	ldr	x0, =EXYNOS9610_GPC2DAT
	ldr	w1, [x0]
	orr	w1, w1, #(0x1 << 3)
	str	w1, [x0]
#endif
	ret

	.global speedy_gpio_init
speedy_gpio_init:
	/* Set PMIC_SPEEDY GPIO (GPQ0_2) to Pull-up */
	ldr	x0, =EXYNOS9610_GPQ0PUD
	ldr	w1, [x0]
	bic	w1, w1, #(0xF << 8)
	orr	w1, w1, #(0x3 << 8)
	str	w1, [x0]

	/* Set PMIC_SPEEDY GPIO(GPQ0_2) to Output */
	ldr	x0, =EXYNOS9610_GPQ0CON
	ldr	w1, [x0]
	bic	w1, w1, #(0xF << 8)
	orr	w1, w1, #(0x2 << 8)
	str	w1, [x0]
	ret
