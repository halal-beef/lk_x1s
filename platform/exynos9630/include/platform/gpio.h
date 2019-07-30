/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */


#ifndef __ASM_ARCH_GPIO_H
#define __ASM_ARCH_GPIO_H

#ifndef __ASSEMBLY__
struct exynos_gpio_bank {
	unsigned int	con;
	unsigned int	dat;
	unsigned int	pull;
	unsigned int	drv;
	unsigned int	pdn_con;
	unsigned int	pdn_pull;
	unsigned char	res1[8];
};

/* Pin configurations */
#define GPIO_INPUT	0x0
#define GPIO_OUTPUT	0x1
#define GPIO_IRQ	0xf
#define GPIO_FUNC(x)	(x)

/* Pull mode */
#define GPIO_PULL_NONE	0x0
#define GPIO_PULL_DOWN	0x1
#define GPIO_PULL_UP	0x3

/* Drive Strength level */
#define GPIO_DRV_1X	0x0
#define GPIO_DRV_3X	0x1
#define GPIO_DRV_2X	0x2
#define GPIO_DRV_4X	0x3
#define GPIO_DRV_FAST	0x0
#define GPIO_DRV_SLOW	0x1

/* GPIO pins per bank  */
#define GPIO_PER_BANK 8

/* functions */
void exynos_gpio_cfg_pin(struct exynos_gpio_bank *bank, int gpio, int cfg);
void exynos_gpio_direction_output(struct exynos_gpio_bank *bank, int gpio, int en);
void exynos_gpio_direction_input(struct exynos_gpio_bank *bank, int gpio);
void exynos_gpio_set_value(struct exynos_gpio_bank *bank, int gpio, int en);
unsigned int exynos_gpio_get_value(struct exynos_gpio_bank *bank, int gpio);
void exynos_gpio_set_pull(struct exynos_gpio_bank *bank, int gpio, int mode);
void exynos_gpio_set_drv(struct exynos_gpio_bank *bank, int gpio, int mode);
void exynos_gpio_set_rate(struct exynos_gpio_bank *bank, int gpio, int mode);


static inline unsigned long exynos_gpio_base(int nr)
{
	return 0;
}

#endif
#endif
