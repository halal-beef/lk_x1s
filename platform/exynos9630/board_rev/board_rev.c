/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <stdio.h>
#include <platform/exynos9630.h>
#include <platform/b_rev.h>
#ifdef CONFIG_GET_B_REV_FROM_ADC
#include <dev/exynos_adc.h>
#include <target/b_rev_adc.h>
#endif
#ifdef CONFIG_GET_B_REV_FROM_GPIO
#include <platform/gpio.h>
#include <target/b_rev_gpio.h>
#endif

#ifdef CONFIG_GET_B_REV_FROM_ADC
int get_board_rev_adc(int *sh)
{
	int i, j;
	int rev = 0;
	*sh = 0;
	int hit_cnt = 0;
	for (i = 0; i < B_REV_ADC_LINES; i++) {
		int adc_v = exynos_adc_read_raw(b_rev_adc[i].ch);
		for (j = 0; j < b_rev_adc[i].levels;j++) {
			int min = b_rev_adc[i].table[j] - b_rev_adc[i].dt;
			int max = b_rev_adc[i].table[j] + b_rev_adc[i].dt;
			if (adc_v >= min && adc_v <= max) {
				rev = j <<  *sh;
				*sh += b_rev_adc[i].bits;
				hit_cnt++;
				continue;
			}
		}
	}

	if (hit_cnt != B_REV_ADC_LINES)
		return -1;
	return rev;
}
#endif

#ifdef CONFIG_GET_B_REV_FROM_GPIO
int get_board_rev_gpio(void)
{
	int i;
	int rev = 0;
	struct exynos_gpio_bank *bank;
	for (i = 0; i < B_REV_GPIO_LINES; i++) {
		int gpio = b_rev_gpio[i].bit;
		bank = b_rev_gpio[i].bank;
		exynos_gpio_cfg_pin(bank, gpio, GPIO_INPUT);
		exynos_gpio_set_pull(bank, gpio, GPIO_PULL_NONE);
		rev |= (exynos_gpio_get_value(bank, gpio) & 0x1) << i;
	}
	return rev;
}
#endif

int get_board_rev(void)
{
	int shift = 0, value = 0, res = 0;

	value = get_board_rev_adc(&shift);
	if (value < 0) {
		printf("%s: Failed to read adc board revision.\n", __func__);
		return -1;
	}

	res |= value;
	value = get_board_rev_gpio();
	if (value < 0) {
		printf("%s: Failed to read gpio board revision.\n", __func__);
		return -1;
	}

	res |= value << shift;

	return res;
}
