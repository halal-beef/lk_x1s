/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <reg.h>
#include "uart_simple.h"
#include <dev/ufs.h>
#include <dev/boot.h>
#include <pit.h>
#include <dev/interrupt/arm_gic.h>
#include <dev/timer/arm_generic.h>
#include <platform/interrupts.h>
#include <platform/sfr.h>
#include <platform/smc.h>
#include <platform/speedy.h>
#include <platform/pmic_s2mpu09.h>
#include <platform/if_pmic_s2mu004.h>
#include <platform/tmu.h>
#include <platform/dfd.h>

#include <lib/font_display.h>
#include <lib/logo_display.h>
#include <target/dpu_config.h>
#include <stdio.h>

#define ARCH_TIMER_IRQ		30

void speedy_gpio_init(void);
void xbootldo_gpio_init(void);
void fg_init_s2mu004(void);
void initialize_font_fb(void);
void display_trip_info(void);

unsigned int s5p_chip_id[4] = {0x0, 0x0, 0x0, 0x0};
unsigned int charger_mode = 0;
unsigned int board_id = 0;
unsigned int board_rev = 0;
unsigned int dram_info[24] = {0, 0, 0, 0};
unsigned long long dram_size_info = 0;

unsigned int get_charger_mode(void)
{
	return charger_mode;
}

static void read_chip_id(void)
{
	s5p_chip_id[0] = readl(EXYNOS9610_PRO_ID + CHIPID0_OFFSET);
	s5p_chip_id[1] = readl(EXYNOS9610_PRO_ID + CHIPID1_OFFSET) & 0xFFFF;
}

static int check_charger_connect(void)
{
	unsigned char read_pwronsrc = 0;
	unsigned int rst_stat = readl(EXYNOS9610_POWER_RST_STAT);

	if (rst_stat == PIN_RESET) {
		speedy_init();
		speedy_read(S2MPU09_PM_ADDR, S2MPU09_PM_PWRONSRC, &read_pwronsrc);

		/* Check USB or TA connected and PWRONSRC(USB)  */
		if(read_pwronsrc & ACOK)
			charger_mode = 1;
		else
			charger_mode = 0;
	} else {
		charger_mode = 0;
	}

	return 0;
}

void arm_generic_timer_disable(void)
{
	mask_interrupt(ARCH_TIMER_IRQ);
}

void platform_early_init(void)
{
	initialize_font_fb();

	printf("LK build date: %s, time: %s\n", __DATE__, __TIME__);

	arm_gic_init();
	writel(1 << 8, EXYNOS9610_MCT_G_TCON);
	arm_generic_timer_init(ARCH_TIMER_IRQ, 26000000);
}

void platform_init(void)
{
	display_tmu_info();
	display_trip_info();
}
