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
	/* Set LCD_TE_HW (GPA4_1) to Pull-down enable, TE is active high */
	ldr	x0, =EXYNOS3830_GPA4PUD
	ldr	w1, [x0]
	bic	w1, w1, #(0xF << 4)
	orr	w1, w1, #(0x1 << 4)
	str	w1, [x0]

	/* Set LCD_TE_HW (GPA4_1) to DISP_TES0 */
	ldr	x0, =EXYNOS3830_GPA4CON
	ldr	w1, [x0]
	bic	w1, w1, #(0xF << 4)
	orr	w1, w1, #(0x2 << 4)
	str	w1, [x0]

	ret

	.global display_panel_init
display_panel_init:
	/* Set MLCD_RSTB (GPG2_4) to Pull-up enable, Reset is active low */
	ldr	x0, =EXYNOS3830_GPG2PUD
	ldr	w1, [x0]
	bic	w1, w1, #(0xF << 16)
	orr	w1, w1, #(0x3 << 16)
	str	w1, [x0]

	/* Set MLCD_RSTB (GPG2_4) to Output */
	ldr	x0, =EXYNOS3830_GPG2CON
	ldr	w1, [x0]
	bic	w1, w1, #(0xF << 16)
	orr	w1, w1, #(0x1 << 16)
	str	w1, [x0]

	/* Set MLCD_RSTB (GPG2_4) to DRV 4X */
	ldr	x0, =EXYNOS3830_GPG2DRV
	ldr	w1, [x0]
	bic	w1, w1, #(0xF << 16)
	orr	w1, w1, #(0x3 << 16)
	str	w1, [x0]

	ret

	.global display_panel_reset
display_panel_reset:
	/* Set MLCD_RSTB (GPG2_4) to low */
	ldr	x0, =EXYNOS3830_GPG2DAT
	ldr	w1, [x0]
	bic	w1, w1, #(0x1 << 4)
	str	w1, [x0]

	ret

	.global display_panel_release
display_panel_release:
	/* Set MLCD_RSTB (GPG2_4) to high */
	ldr	x0, =EXYNOS3830_GPG2DAT
	ldr	w1, [x0]
	bic	w1, w1, #(0x1 << 4)
	orr	w1, w1, #(0x1 << 4)
	str	w1, [x0]

	ret

	.global display_panel_power
display_panel_power:
	/* Set LCD_BL_EN(GPG2_1) to Output */
	ldr     x0, =EXYNOS3830_GPG2CON
	ldr     w1, [x0]
	bic     w1, w1, #(0xF << 4)
	orr     w1, w1, #(0x1 << 4)
	str     w1, [x0]

	ldr     x0, =EXYNOS3830_GPG2DAT
	ldr     w1, [x0]
	bic     w1, w1, #(0x1 << 1)
	orr     w1, w1, #(0x1 << 1)
	str     w1, [x0]

	ret

