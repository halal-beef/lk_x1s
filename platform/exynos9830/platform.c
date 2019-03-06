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
#include <dev/rpmb.h>
#include <pit.h>
#include <dev/interrupt/arm_gic.h>
#include <dev/timer/arm_generic.h>
#include <platform/interrupts.h>
#include <platform/sfr.h>
#include <platform/smc.h>
#include <platform/pmic_s2mps_19_22.h>
#ifdef CONFIG_SUB_PMIC_S2DOS05
#include <target/pmic_s2dos05.h>
#else
#include <platform/sub_pmic_s2mpb02.h>
#endif
#include <platform/dfd.h>
#include <platform/ldfw.h>
#include <platform/secure_boot.h>
#include "flexpmu_dbg.h"
#include <platform/tmu.h>
#include <platform/chg_max77705.h>

#include <lib/font_display.h>
#include <lib/logo_display.h>
#include <target/dpu_config.h>
#include <stdio.h>

#ifdef EXYNOS_ACPM_BASE
#include "acpm.h"
#endif

#define ARCH_TIMER_IRQ 30

void speedy_gpio_init(void);
void xbootldo_gpio_init(void);
void fg_init_s2mu004(void);

unsigned int s5p_chip_id[4] = { 0x0, 0x0, 0x0, 0x0 };
unsigned int charger_mode = 0;
unsigned int board_id = CONFIG_BOARD_ID;
unsigned int board_rev = 0;
unsigned int dram_info[24] = { 0, 0, 0, 0 };
unsigned long long dram_size_info = 0;
unsigned int secure_os_loaded = 0;

unsigned int get_charger_mode(void)
{
	return charger_mode;
}

static void read_chip_id(void)
{
	s5p_chip_id[0] = readl(EXYNOS9830_PRO_ID + CHIPID0_OFFSET);
	s5p_chip_id[1] = readl(EXYNOS9830_PRO_ID + CHIPID1_OFFSET) & 0xFFFF;
}

static void read_dram_info(void)
{
	char type[16];
	char rank_num[20];
	char manufacturer[20];
	unsigned int M5 = 0, M6 = 0, M7 = 0, M8 = 0;
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
	M5 = tmp;

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

	M6 = dram_info[1] & 0xFF;
	M7 = (dram_info[1] >> 8) & 0xFF;
	M8 = (dram_info[0] & 0x3) | (((dram_info[0] >> 20) & 0xF) << 2) | ((dram_info[0]  >> 16 & 0x3) << 6);

#ifdef CONFIG_EXYNOS_BOOTLOADER_DISPLAY
	print_lcd(FONT_WHITE, FONT_BLACK, "DRAM %lu GB %s %s %s M5=0x%02x M6=0x%02x M7=0x%02x M8=0x%02x",
	          dram_size_info, type, rank_num, manufacturer,
	          M5, M6, M7, M8);
#endif
}

static void load_secure_payload(void)
{
	unsigned long ret = 0;
	unsigned int boot_dev = 0;
	unsigned int dfd_en = readl(EXYNOS9830_POWER_RESET_SEQUENCER_CONFIGURATION);
	unsigned int rst_stat = readl(EXYNOS9830_POWER_RST_STAT);

	if (*(unsigned int *)DRAM_BASE != 0xabcdef) {
		printf("Running on DRAM by TRACE32: skip load_secure_payload()\n");
	} else {
		if (is_first_boot()) {
			boot_dev = get_boot_device();

			/*
			 * In case WARM Reset/Watchdog Reset and DumpGPR is enabled,
			 * Secure payload doesn't have to be loaded.
			 */
			if (!((rst_stat & (WARM_RESET | LITTLE_WDT_RESET)) &&
			      (dfd_en & EXYNOS9830_EDPCSR_DUMP_EN))) {
				ret = load_sp_image(boot_dev);
				if (ret) {
					/*
					 * 0xFEED0002 : Signature check fail
					 * 0xFEED0020 : Anti-rollback check fail
					 */
					printf("Fail to load Secure Payload!! [ret = 0x%lX]\n", ret);
				} else {
					printf("Secure Payload is loaded successfully!\n");
					secure_os_loaded = 1;
				}
			}

			/*
			 * If boot device is eMMC, emmc_endbootop() should be
			 * implemented because secure payload is the last image
			 * in boot partition.
			 */
			if (boot_dev == BOOT_EMMC)
				emmc_endbootop();
		} else {
			/* second_boot = 1; */
		}
	}
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
	read_chip_id();

	speedy_gpio_init();
	xbootldo_gpio_init();
#ifdef CONFIG_EXYNOS_BOOTLOADER_DISPLAY
	display_panel_init();
	initialize_fbs();
#endif
	set_first_boot_device_info();

	uart_test_function();
	printf("LK build date: %s, time: %s\n", __DATE__, __TIME__);
	debug_snapshot_boot_cnt();

	arm_gic_init();
	writel(1 << 8, EXYNOS9830_MCT_G_TCON);
	arm_generic_timer_init(ARCH_TIMER_IRQ, 26000000);
}

static void print_acpm_version(void)
{
#ifdef EXYNOS_ACPM_BASE
	unsigned int i;
	unsigned int plugins, num_plugins, plugin_address, plugin_ops_address;
	struct plugin *plugin;
	struct plugin_ops *plugin_ops;
	char *build_info;

	/* Check ACPM STACK Magic */
	if (readl(EXYNOS_ACPM_MAGIC) != ACPM_MAGIC_VALUE)
		return;

	build_info = (char *)EXYNOS_ACPM_APSHARE + APSHARE_BUILDINFO_OFFSET;
	printf("ACPM: Framework's  version is %s %s\n", build_info,
	       build_info + BUILDINFO_ELEMENT_SIZE);

	plugins = readl(EXYNOS_ACPM_APSHARE);
	num_plugins = readl(EXYNOS_ACPM_APSHARE + 4);

	for (i = 1; i < num_plugins; i++) {
		plugin_address = plugins + sizeof(struct plugin) * i;
		if (readl(get_acpm_plugin_element(plugin, is_attached)) == 1) {
			plugin_ops_address = readl(get_acpm_plugin_element(plugin, plugin_ops));
			build_info = (char *)get_acpm_plugin_element(plugin_ops, info);
			printf("ACPM: Plugin(id:%d) version is %s %s\n",
			       (int)readl(get_acpm_plugin_element(plugin, id)),
			       build_info, build_info + BUILDINFO_ELEMENT_SIZE);
		}
	}
#endif /* ifdef EXYNOS_ACPM_BASE */
}

void platform_init(void)
{
	u32 ret = 0;

	display_flexpmu_dbg();
	print_acpm_version();

	pmic_init();
	display_pmic_info();
#ifdef CONFIG_SUB_PMIC_S2DOS05
	pmic_init_s2dos05();
#else
	sub_pmic_s2mpb02_init();
#endif

	/*
	 * check_charger_connect();
	 */

	if (get_boot_device() == BOOT_UFS) {
		printf("get_boot_device() == BOOT_UFS\n");
		ufs_init(2);
		ret = ufs_set_configuration_descriptor();
		if (ret == 1)
			ufs_init(2);
	}
	pit_init();
	debug_snapshot_fdt_init();
#if 0
#ifdef CONFIG_EXYNOS_BOOTLOADER_DISPLAY
	/* If the display_drv_init function is not called before,
	 * you must use the print_lcd function.
	 */
	print_lcd(FONT_RED, FONT_BLACK, "LK Display is enabled!");
	ret = display_drv_init();
	if (ret == 0 && is_first_boot())
		show_boot_logo();

	/* If the display_drv_init function is called,
	 * you must use the print_lcd_update function.
	 */
	//print_lcd_update(FONT_BLUE, FONT_BLACK, "LK display is enabled!");
#endif
	read_dram_info();

	dfd_display_reboot_reason();
#endif
	if (*(unsigned int *)DRAM_BASE == 0xabcdef) {
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

		if (!init_ldfws())
			printf("ldfw: init done successfully.\n");
		else
			printf("ldfw: init failed.\n");

#if defined(CONFIG_USE_RPMB)
		rpmb_key_programming();
#if defined(CONFIG_USE_AVB20)
		rpmb_load_boot_table();
#endif
#endif
		ret = (u32)init_sp();
		if (!ret)
			printf("secure_payload: init done successfully.\n");
		else
			printf("secure_payload: init failed.\n");

		print_el3_monitor_version();
	}

	display_tmu_info();
	display_trip_info();

	chg_init_max77705();
}
