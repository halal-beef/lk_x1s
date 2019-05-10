/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __B_REV_GPIO_H__

#define B_REV_GPIO_LINES	1

/*
	HW_REV_M - 0: Advanced, 1: Mass
 */

struct b_rev_gpio_info b_rev_gpio[B_REV_GPIO_LINES] = {
	{
		.bank = (struct exynos_gpio_bank *) EXYNOS9830_GPC0CON,
		.bit = 3,
	},
};

#endif /*__B_REV_GPIO_H__*/
