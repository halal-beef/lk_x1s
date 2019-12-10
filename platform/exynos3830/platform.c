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
#include <stdlib.h>
#include <string.h>
#include <dev/boot.h>
#include <dev/rpmb.h>
#include <part.h>
#include <lib/sysparam.h>
#include <lib/console.h>
#include <dev/interrupt/arm_gic.h>
#include <dev/timer/arm_generic.h>
#include <platform/interrupts.h>
#include <platform/sfr.h>
#include <platform/smc.h>
#include <target/pmic.h>
#include <platform/tmu.h>
#include <platform/ldfw.h>
#include <platform/gpio.h>
#include <platform/bl_sys_info.h>
#include <platform/dram_training.h>
#include <platform/mmu/mmu_func.h>
#include <platform/fastboot.h>
#include <platform/sfr.h>
#include <platform/uart.h>
#include <platform/board_rev.h>
#include <dev/mmc.h>
#include <dev/debug/dss.h>
#include <platform/secure_boot.h>

#include <lib/font_display.h>
#include <lib/logo_display.h>
#include <part_gpt.h>
#include <target/dpu_config.h>
#include <stdio.h>

#include <dev/lk_acpm_ipc.h>
#include "flexpmu_dbg.h"

#define ARCH_TIMER_IRQ		30

unsigned int s5p_chip_id[4] = {0x0, 0x0, 0x0, 0x0};
unsigned int charger_mode = 0;
unsigned int board_id = CONFIG_BOARD_ID;
unsigned int board_rev = 0;
unsigned int dram_info[24] = {0, 0, 0, 0};
unsigned long long dram_size_info = 0;
unsigned int secure_os_loaded = 0;

unsigned int get_charger_mode(void)
{
	return charger_mode;
}

static void read_chip_id(void)
{
	s5p_chip_id[0] = readl(EXYNOS3830_PRO_ID + CHIPID0_OFFSET);
	s5p_chip_id[1] = readl(EXYNOS3830_PRO_ID + CHIPID1_OFFSET) & 0xFFFF;
}

static void display_rst_stat(u32 rst_stat)
{
	u32 temp = rst_stat & (WARM_RESET | LITTLE_WDT_RESET | BIG_WDT_RESET | PIN_RESET | SWRESET);

	switch(temp) {
	case WARM_RESET:
		printf("rst_stat:0x%x / WARMRESET\n", rst_stat);
		break;
	case LITTLE_WDT_RESET:
		printf("rst_stat:0x%x / CL0_WDTRESET\n", rst_stat);
		break;
	case BIG_WDT_RESET:
		printf("rst_stat:0x%x / CL2_WDTRESET\n", rst_stat);
		break;
	case PIN_RESET:
		printf("rst_stat:0x%x / PINRESET\n", rst_stat);
		break;
	case SWRESET:
		printf("rst_stat:0x%x / SWRESET\n", rst_stat);
		break;
	default:
		printf("rst_stat:0x%x\n", rst_stat);
		break;
	}
}

static void read_dram_info(void)
{
	char type[16];
	char rank_num[20];
	char manufacturer[20];
#ifdef CONFIG_DISPLAY_DRAWFONT
	unsigned int M5 = 0, M6 = 0, M7 = 0, M8 = 0;
#endif
	unsigned int tmp = 0;

	printf("%s %d\n", __func__, __LINE__);
	/* 1. Type */
	dram_info[0] = readl(DRAM_INFO);
	tmp = dram_info[0] & 0xF;
	printf("%s %d\n", __func__, __LINE__);

	switch (tmp) {
	case 0x0:
		strcpy(type, "LPDDR4");
		break;
	case 0x2:
		strcpy(type, "LPDDR4X");
		break;
	default:
		printf("Type None!\n");
	}

	printf("%s %d\n", __func__, __LINE__);
	/* 2. rank_num */
	dram_info[0] = readl(DRAM_INFO);
	tmp = (dram_info[0] >> 4) & 0xF;

	printf("%s %d\n", __func__, __LINE__);
	switch (tmp) {
	case 0x0:
		strcpy(rank_num, "1RANK");
		break;
	case 0x3:
		strcpy(rank_num, "2RANK");
		break;
	default:
		printf("Rank_num None!\n");
	}

	printf("%s %d\n", __func__, __LINE__);
	/* 3. manufacturer */
	dram_info[0] = readl(DRAM_INFO);
	tmp = (dram_info[0] >> 8) & 0xFF;
#ifdef CONFIG_DISPLAY_DRAWFONT
	M5 = tmp;
#endif

	printf("%s %d\n", __func__, __LINE__);
	switch (tmp) {
	case 0x01:
		strcpy(manufacturer, "Samsung");
		break;
	case 0x06:
		strcpy(manufacturer, "SK hynix");
		break;
	case 0xFF:
		strcpy(manufacturer, "Micron");
		break;
	default:
		printf("Manufacturer None!\n");
	}

	printf("%s %d\n", __func__, __LINE__);
	dram_info[1] = readl(DRAM_INFO + 0x4);
	dram_info[2] = readl(DRAM_SIZE_INFO);
	dram_info[3] = readl(DRAM_SIZE_INFO + 0x4);
	dram_size_info |= (unsigned long long)(dram_info[2]);
	dram_size_info |= (unsigned long long)(dram_info[3]) << 32;
	/* Set to GB */
	dram_size_info = dram_size_info / 1024 / 1024 / 1024;

#ifdef CONFIG_DISPLAY_DRAWFONT
	M6 = dram_info[1] & 0xFF;
	M7 = (dram_info[1] >> 8) & 0xFF;
	M8 = (dram_info[0] & 0x3) | (((dram_info[0] >> 20) & 0xF) << 2) | ((dram_info[0]  >> 16 & 0x3) << 6);
#endif

#ifdef CONFIG_DISPLAY_DRAWFONT
	print_lcd(FONT_WHITE, FONT_BLACK, "DRAM %lu GB %s %s %s M5=0x%02x M6=0x%02x M7=0x%02x M8=0x%02x",
		dram_size_info,	type, rank_num, manufacturer,
		M5, M6, M7, M8);
#endif
}

#define EL3_MON_VERSION_STR_SIZE (180)

static void print_el3_monitor_version(void)
{
	char el3_mon_ver[EL3_MON_VERSION_STR_SIZE] = { 0, };

	if (*(unsigned int *)DRAM_BASE == 0xabcdef) {
		/* This booting is from eMMC/UFS. not T32 */
		get_el3_mon_version(el3_mon_ver, EL3_MON_VERSION_STR_SIZE);
		printf("\nEL3 Monitor information: \n");
		printf("%s\n\n", el3_mon_ver);
	}
}

static int check_charger_connect(void)
{
	unsigned char read_pwronsrc = 0;
	unsigned int rst_stat = readl(EXYNOS3830_POWER_RST_STAT);

	if ((rst_stat & PIN_RESET) == PIN_RESET) {
		i3c_read(0, S2MPU12_PM_ADDR, S2MPU12_PM_PWRONSRC, &read_pwronsrc);

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

#ifdef CONFIG_EXYNOS_BOOTLOADER_DISPLAY
extern int display_drv_init(void);
void display_panel_init(void);

static void initialize_fbs(void)
{
	memset((void *)CONFIG_DISPLAY_LOGO_BASE_ADDRESS, 0, LCD_WIDTH * LCD_HEIGHT * 4);
	memset((void *)CONFIG_DISPLAY_FONT_BASE_ADDRESS, 0, LCD_WIDTH * LCD_HEIGHT * 4);
}
#endif

void arm_generic_timer_disable(void)
{
	mask_interrupt(ARCH_TIMER_IRQ);
}

void platform_early_init(void)
{
#if defined(CONFIG_UART_LOG_MODE) || defined(CONFIG_MMU_ENABLE)
	unsigned int rst_stat = readl(EXYNOS3830_POWER_RST_STAT);
#endif
#if defined(CONFIG_MMU_ENABLE)
	unsigned int dfd_en = readl(EXYNOS3830_POWER_RESET_SEQUENCER_CONFIGURATION);
#endif

#if defined(CONFIG_MMU_ENABLE)
	if (!((rst_stat & (WARM_RESET | LITTLE_WDT_RESET)) &&
			dfd_en & EXYNOS3830_EDPCSR_DUMP_EN)) {
		invalidate_dcache_all();
		cpu_common_init();
		clean_invalidate_dcache_all();
	}
#endif

	read_chip_id();

#ifdef CONFIG_EXYNOS_BOOTLOADER_DISPLAY
	display_panel_init();
	initialize_fbs();
#endif
	set_first_boot_device_info();

#if defined(CONFIG_UART_LOG_MODE)
	if (get_current_boot_device() == BOOT_USB ||
		rst_stat & (WARM_RESET | LITTLE_WDT_RESET)) {
		uart_log_mode = 1;
	}
#endif

	uart_console_init();
	printf("LK build date: %s, time: %s\n", __DATE__, __TIME__);
	dss_boot_cnt();

	arm_gic_init();
	writel(1 << 8, EXYNOS3830_MCT_G_TCON);
	arm_generic_timer_init(ARCH_TIMER_IRQ, 26000000);
}

extern bool is_xct_boot(void);
extern int init_fastboot_variables(void);

void platform_init(void)
{
	unsigned int rst_stat = readl(EXYNOS3830_POWER_RST_STAT);
	int ret = 0;

	display_flexpmu_dbg();

	display_rst_stat(rst_stat);
#if defined(CONFIG_AB_UPDATE)
	struct bl_sys_info *bl_sys = (struct bl_sys_info *)BL_SYS_INFO;

	printf("bootloader partition start block(512 Byte): %d\n",
			bl_sys->bl1_info.epbl_start * (UFS_BSIZE / MMC_BSIZE));
#endif

	if (*(unsigned int *)DRAM_BASE == 0xabcdef) {
		pmic_init();
		read_pmic_info();
		s2mu106_charger_init();
		fg_init_s2mu106();
	}

	if (*(unsigned int *)DRAM_BASE == 0xabcdef)
		check_charger_connect();

	ret = get_board_rev();
	ret < 0 ? (board_rev = 0):(board_rev = ret);

	mmc_init();
	part_init();
	if (is_first_boot() && *(unsigned int *)DRAM_BASE == 0xabcdef)
		dss_fdt_init();

#if defined(CONFIG_UART_LOG_MODE)
	if (get_current_boot_device() != BOOT_USB &&
		*(unsigned int *)DRAM_BASE == 0xabcdef) {
		unsigned int env_val = 0;

		if (sysparam_read("uart_log_enable", &env_val, sizeof(env_val)) > 0) {
			if (env_val == UART_LOG_MODE_FLAG)
				uart_log_mode = 1;
			else
				uart_log_mode = 1;
		} else
			uart_log_mode = 1;
	}
#endif

#ifdef CONFIG_EXYNOS_BOOTLOADER_DISPLAY
	/* If the display_drv_init function is not called before,
	 * you must use the print_lcd function.
	 */
	print_lcd(FONT_RED, FONT_BLACK, "LK Display is enabled!");
	ret = display_drv_init();
	if (ret == 0 && is_first_boot() && *(unsigned int *)DRAM_BASE == 0xabcdef)
		show_boot_logo();

	/* If the display_drv_init function is called,
	 * you must use the print_lcd_update function.
	 */
	print_lcd_update(FONT_BLUE, FONT_BLACK, "LK display is enabled!");
#endif

	dfd_display_reboot_reason();
	dfd_display_core_stat();

	read_dram_info();

	display_tmu_info();
	display_trip_info();

	/*
	if ((get_current_boot_device() != BOOT_USB) &&
		*(unsigned int *)DRAM_BASE == 0xabcdef)
		init_fastboot_variables();
	if (is_xct_boot())
		return;
	*/

	if (*(unsigned int *)DRAM_BASE == 0xabcdef) {
		unsigned int dfd_en = readl(EXYNOS3830_POWER_RESET_SEQUENCER_CONFIGURATION);

		if ((rst_stat & (WARM_RESET | LITTLE_WDT_RESET)) &&
			(dfd_en & EXYNOS3830_EDPCSR_DUMP_EN)) {
			/* if it's a case of ramdump, do not load ldfw/sp */
			printf("Dumpgpr mode. do not load ldfw/sp .\n");
			goto by_dumpgpr_out;
		}

		/* read secure chip state */
		if (read_secure_chip() == 0)
			printf("Secure boot is disabled (non-secure chip)\n");
		else if (read_secure_chip() == 1)
			printf("Secure boot is enabled (test key)\n");
		else if (read_secure_chip() == 2)
			printf("Secure boot is enabled (secure chip)\n");
		else
			printf("Can not read secure chip state\n");

		if (!init_keystorage())
			printf("keystorage: init done successfully.\n");
		else
			printf("keystorage: init failed.\n");

		if (!init_ldfws()) {
			printf("ldfw: init done successfully.\n");
#if defined(CONFIG_USE_RPMB)
#if defined(CONFIG_RPMB_TA)
			rpmb_key_TA();
#else
			rpmb_key_programming();
#endif
#if defined(CONFIG_USE_AVB20)
			rpmb_load_boot_table();
#endif
#endif
		} else {
			printf("ldfw: init failed.\n");
		}

		ret = init_sp();
		if (!ret)
			printf("secure_payload: init done successfully.\n");
		else
			printf("secure_payload: init failed.\n");

by_dumpgpr_out:
		print_el3_monitor_version();
	}

	if (rst_stat & (WARM_RESET | LITTLE_WDT_RESET))
		dfd_run_post_processing();
}
