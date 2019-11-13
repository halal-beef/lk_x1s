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

#include <platform/sfr.h>

	.global display_te_init
display_te_init:
	/* Set LCD_TE_HW (GPG0_1) to Pull-down enable, TE is active high */
	ldr	x0, =EXYNOS9630_GPG0PUD
	ldr	w1, [x0]
	bic	w1, w1, #(0xF << 4)
	orr	w1, w1, #(0x1 << 4)
	str	w1, [x0]

	/* Set LCD_TE_HW (GPG0_1) to DISP_TES0 */
	ldr	x0, =EXYNOS9630_GPG0CON
	ldr	w1, [x0]
	bic	w1, w1, #(0xF << 4)
	orr	w1, w1, #(0x2 << 4)
	str	w1, [x0]

	ret

	.global display_panel_init
display_panel_init:
	/* Set MLCD_RSTB (GPQ0_2) to Pull-up enable, Reset is active low */
	ldr	x0, =EXYNOS9630_GPQ0PUD
	ldr	w1, [x0]
	bic	w1, w1, #(0xF << 8)
	orr	w1, w1, #(0x3 << 8)
	str	w1, [x0]

	/* Set MLCD_RSTB (GPQ0_2) to Output */
	ldr	x0, =EXYNOS9630_GPQ0CON
	ldr	w1, [x0]
	bic	w1, w1, #(0xF << 8)
	orr	w1, w1, #(0x1 << 8)
	str	w1, [x0]

	/* Set MLCD_RSTB (GPQ0_2) to DRV 4X */
	ldr	x0, =EXYNOS9630_GPQ0DRV
	ldr	w1, [x0]
	bic	w1, w1, #(0xF << 8)
	orr	w1, w1, #(0x3 << 8)
	str	w1, [x0]

	ret

	.global display_panel_reset
display_panel_reset:
	/* Set MLCD_RSTB (GPQ0_2) to low */
	ldr	x0, =EXYNOS9630_GPQ0DAT
	ldr	w1, [x0]
	bic	w1, w1, #(0x1 << 2)
	str	w1, [x0]

	ret

	.global display_panel_release
display_panel_release:
	/* Set MLCD_RSTB (GPQ0_2) to high */
	ldr	x0, =EXYNOS9630_GPQ0DAT
	ldr	w1, [x0]
	bic	w1, w1, #(0x1 << 2)
	orr	w1, w1, #(0x1 << 2)
	str	w1, [x0]

	ret

	.global display_panel_power
display_panel_power:
        /* Set UB_DDR_EN (GPF1_6) to Output */
        ldr             x0, =EXYNOS9630_GPF1CON
        ldr             w1, [x0]
        bic             w1, w1, #(0xF << 24)
        orr             w1, w1, #(0x1 << 24)
        str             w1, [x0]

        ldr             x0, =EXYNOS9630_GPF1DAT
        ldr             w1, [x0]
        bic             w1, w1, #(0x1 << 6)
        orr             w1, w1, #(0x1 << 6)
        str             w1, [x0]

        ret

