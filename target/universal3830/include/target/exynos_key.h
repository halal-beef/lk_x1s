#ifndef __EXYNOS_KEY_H__
#define __EXYNOS_KEY_H__

#include <platform/sfr.h>
#include <platform/gpio.h>

#define VOL_UP_KEY_GPIO_BANK	EXYNOS3830_GPA0CON
#define VOL_UP_KEY_GPIO			7

#define VOL_DOWN_KEY_GPIO_BANK	EXYNOS3830_GPA1CON
#define VOL_DOWN_KEY_GPIO		0

#define KEY_DETECT				0
#define KEY_NOT_DETECT			1

#define KEY_DETECT_NUM			10

void exynos_key_init(void);
int exynos_key_read(int num);
int exynos_key_check(void);

#endif /* __EXYNOS_KEY_H__ */
