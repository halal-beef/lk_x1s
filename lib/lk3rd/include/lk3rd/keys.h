#ifndef KEYS_H
#define KEYS_H

/* Key GPIOs */
#define GPIO_VOLUP 3
#define GPIO_VOLDOWN 4
#define GPIO_POWER GPIO_VOLDOWN

/* Pinctrl banks */
#define BANK_GPA0 0
#define BANK_GPA1 1
#define BANK_GPA2 2

void setup_keys(struct exynos_gpio_bank*, int);

#endif /* KEYS_H */
