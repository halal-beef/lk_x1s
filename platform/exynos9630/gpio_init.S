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
	ret

	.global xbootldo_gpio_init
xbootldo_gpio_init:
	/* Set XBOOTLDO (GPA0_0) to Pull-up/down disable */
	ldr	x0, =EXYNOS9630_GPA0PUD
	ldr	w1, [x0]
	bic	w1, w1, #(0xF << 0)
	str	w1, [x0]

	/* Set XBOOTLDO (GPA0_0) to Output */
	ldr	x0, =EXYNOS9630_GPA0CON
	ldr	w1, [x0]
	bic	w1, w1, #(0xF << 0)
	orr	w1, w1, #(0x1 << 0)
	str	w1, [x0]

	/* Set XBOOTLDO (GPA0_0) to High */
	ldr	x0, =EXYNOS9630_GPA0DAT
	ldr	w1, [x0]
	orr	w1, w1, #(0x1 << 0)
	str	w1, [x0]
	ret
