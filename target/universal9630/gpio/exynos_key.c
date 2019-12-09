#include <stdio.h>
#include <platform/gpio.h>
#include <target/exynos_key.h>

void exynos_key_init(void)
{
	struct exynos_gpio_bank *bank = (struct exynos_gpio_bank *)VOL_UP_KEY_GPIO_BANK;
	exynos_gpio_set_pull(bank, VOL_UP_KEY_GPIO, GPIO_PULL_NONE);
	exynos_gpio_cfg_pin(bank, VOL_UP_KEY_GPIO, GPIO_INPUT);
	return;
}

int exynos_key_read(int num)
{
	struct exynos_gpio_bank *bank = (struct exynos_gpio_bank *)VOL_UP_KEY_GPIO_BANK;
	int val = 1;
	int i;
	for (i = 0; i < num; i ++) {
		val = exynos_gpio_get_value(bank, VOL_UP_KEY_GPIO);
		printf("Volume up key: %d\n", val);
	}
	return val;
}

int exynos_key_check(void)
{
	int return_val = KEY_NOT_DETECT;
	if (exynos_key_read( KEY_DETECT_NUM ) == 0) {
		printf("Volume up key is pressed. Entering fastboot mode!\n");
		return_val = KEY_DETECT;
	}
	return return_val;
}
