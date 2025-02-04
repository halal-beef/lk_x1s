/*
 * Copyright (c) 2012-2015 Travis Geiselbrecht
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 */
#include <arch.h>
#include <lk/err.h>
#include <lk/debug.h>
#include <lk/trace.h>
#include <dev/interrupt/arm_gic.h>
#include <dev/timer/arm_generic.h>
#include <dev/uart.h>
#include <lk/init.h>
#include <kernel/vm.h>
#include <kernel/spinlock.h>
#include <platform.h>
#include <platform/gic.h>
#include <platform/interrupts.h>
#include <platform/exynos9820.h>
#include <platform/chip_rev.h>
#include <libfdt.h>
#include <dev/boot.h>
#include <platform/uart.h>
#include <platform/sizes.h>
//#include <dev/debug/dss.h>

#define ARCH_TIMER_IRQ 30

unsigned int s5p_chip_id[4] = { 0x0, 0x0, 0x0, 0x0 };
struct chip_rev_info s5p_chip_rev;

static void read_chip_id(void)
{
	s5p_chip_id[0] = readl(EXYNOS9820_PRO_ID + CHIPID0_OFFSET);
	s5p_chip_id[1] = readl(EXYNOS9820_PRO_ID + CHIPID1_OFFSET) & 0xFFFF;
}

static void read_chip_rev(void)
{
	unsigned int val = readl(EXYNOS9820_PRO_ID + CHIPID_REV_OFFSET);
	s5p_chip_rev.main = (val >> 20) & 0xf;
	s5p_chip_rev.sub = (val >> 16) & 0xf;
}

int clear_screen(int);
//void initialize_font_fb(void);

void platform_early_init(void)
{
	// Temporary, since we do not have panel driver
	writel(0x1281, DECON0_BASE_ADDRESS + HW_SW_TRIG_CONTROL);

	//initialize_font_fb();
	clear_screen(0xff);
	platform_dputc('\n');
	platform_dputc('\n');
	platform_dputc('\n');
	platform_dputc('\n');
	platform_dputc('h');
	platform_dputc('e');
	platform_dputc('l');
	platform_dputc('l');
	platform_dputc('o');
	platform_dputc(' ');
	platform_dputc('l');
	platform_dputc('k');
	platform_dputc('3');
	platform_dputc('r');
	platform_dputc('d');
	platform_dputc('!');

	//read_chip_id();
	//read_chip_rev();

	//set_first_boot_device_info();

	uart_console_init();
	printf("LK build date: %s, time: %s\n", __DATE__, __TIME__);
	//dss_boot_cnt();

	arm_gic_init();
	writel(1 << 8, EXYNOS9820_MCT_G_TCON);
	arm_generic_timer_init(ARCH_TIMER_IRQ, 26000000);
}

void platform_init(void)
{

}
