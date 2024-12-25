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
#include <dev/ufs.h>
#include <dev/boot.h>
#include <dev/rpmb.h>
#include <pit.h>
#include <part.h>
#include <dev/interrupt/arm_gic.h>
#include <dev/timer/arm_generic.h>
#include <platform/interrupts.h>
#include <platform/sfr.h>
#include <platform/uart.h>
#include <platform/smc.h>
#include <dev/pmic_s2mps_19_22.h>
#ifdef CONFIG_SUB_PMIC_S2DOS05
#include <dev/pmic_s2dos05.h>
#else
#include <dev/sub_pmic_s2mpb02.h>
#endif
#include <dev/if_pmic_s2mu106.h>
#include <dev/fg_s2mu106.h>
#include <dev/debug/dss.h>
#include <platform/ldfw.h>
#include <platform/secure_boot.h>
#include <platform/h-arx.h>
#include <platform/power/flexpmu_dbg.h>
#include <platform/tmu.h>
#include <dev/chg_max77705.h>
#include <platform/dvfs_info.h>
#include <platform/mmu/mmu_func.h>
#include <dev/mmc.h>

#include <lib/font_display.h>
#include <lib/logo_display.h>
#include <lib/easter_eggs.h>
#include <target/dpu_config.h>
#include <stdio.h>
#include <platform/b_rev.h>

#ifdef EXYNOS_ACPM_BASE
#include <platform/power/acpm.h>
#endif

#include <platform/chip_rev.h>

#define ARCH_TIMER_IRQ 30

unsigned int s5p_chip_id[4] = { 0x0, 0x0, 0x0, 0x0 };
struct chip_rev_info s5p_chip_rev;

unsigned int board_id = CONFIG_BOARD_ID;
unsigned int board_rev = CONFIG_BOARD_REV;

unsigned int secure_os_loaded = 0;

static void read_chip_id(void)
{
	s5p_chip_id[0] = readl(EXYNOS9830_PRO_ID + CHIPID0_OFFSET);
	s5p_chip_id[1] = readl(EXYNOS9830_PRO_ID + CHIPID1_OFFSET) & 0xFFFF;
}

static void read_chip_rev(void)
{
	unsigned int val = readl(EXYNOS9830_PRO_ID + CHIPID_REV_OFFSET);
	s5p_chip_rev.main = (val >> 20) & 0xf;
	s5p_chip_rev.sub = (val >> 16) & 0xf;
}

unsigned int get_charger_mode(void)
{
	return 0;
}

static void display_rst_stat(u32 rst_stat)
{
	u32 temp = rst_stat & (WARM_RESET | LITTLE_WDT_RESET | BIG_WDT_RESET | PIN_RESET);

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
	default:
		printf("rst_stat:0x%x\n", rst_stat);
		break;
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

void arm_generic_timer_disable(void)
{
	mask_interrupt(ARCH_TIMER_IRQ);
}

void platform_early_init(void)
{
	unsigned int rst_stat = readl(EXYNOS9830_POWER_RST_STAT);
	unsigned int dfd_en = readl(EXYNOS9830_POWER_RESET_SEQUENCER_CONFIGURATION);

	if (!((rst_stat & (WARM_RESET | LITTLE_WDT_RESET)) &&
			dfd_en & EXYNOS9830_EDPCSR_DUMP_EN)) {
		invalidate_dcache_all();
		cpu_common_init();
		clean_invalidate_dcache_all();
	}

	read_chip_id();
	read_chip_rev();

#ifdef CONFIG_EXYNOS_BOOTLOADER_DISPLAY
	display_panel_init();
	initialize_fbs();
#endif
	set_first_boot_device_info();

	uart_console_init();
	printf("LK build date: %s, time: %s\n", __DATE__, __TIME__);
	dss_boot_cnt();

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
	u32 rst_stat = readl(POWER_RST_STAT);

	show_dummy_unlock_warning();

	display_flexpmu_dbg();
	print_acpm_version();

	display_rst_stat(rst_stat);
	pmic_init();
	display_pmic_info();
#ifdef CONFIG_SUB_PMIC_S2DOS05
	pmic_init_s2dos05();
#else
	sub_pmic_s2mpb02_init();
#endif
#ifdef CONFIG_S2MU106_CHARGER
	s2mu106_charger_init();
	fg_init_s2mu106();
#endif

	/*
	 * check_charger_connect();
	 */
	if (get_boot_device() == BOOT_UFS) {
		ufs_alloc_memory();
		ufs_init(2);
		ret = ufs_set_configuration_descriptor();
		if (ret == 1)
			ufs_init(2);
	}

	/*
	 * Initialize mmc for all channel.
	 * Sometimes need mmc device when it is not boot device.
	 * So always call mmc_init().
	 */
#ifndef CONFIG_SKIP_MMC_INIT
	mmc_init(MMC_CHANNEL_SD);
#else
	print_lcd_update(FONT_WHITE, FONT_BLACK, "Device does not have an SD card slot! Skip SD init");
#endif
	part_init();

	dss_fdt_init();
	dfd_get_dbgc_version();
	if (rst_stat & (WARM_RESET | LITTLE_WDT_RESET))
		dfd_run_post_processing();

	dfd_display_core_stat();
	if (*(unsigned int *)DRAM_BASE == 0xabcdef) {
		unsigned int dfd_en = readl(EXYNOS9830_POWER_RESET_SEQUENCER_CONFIGURATION);
		unsigned int rst_stat = readl(EXYNOS9830_POWER_RST_STAT);

		/* read secure chip state */
		switch (read_secure_chip())
		{
		case 0:
			printf("Secure boot is disabled (non-secure chip)\n");
			break;
		case 1:
			printf("Secure boot is enabled (test key)\n");
			break;
		case 2:
			printf("Secure boot is enabled (secure chip)\n");
			break;
		
		default:
			printf("Can not read secure chip state\n");
			break;
		}

		if ((rst_stat & (WARM_RESET | LITTLE_WDT_RESET)) &&
		    (dfd_en & EXYNOS9830_EDPCSR_DUMP_EN)) {
			/* in case of dumpgpr, do not load ldfw/sp */
			printf("Dumpgpr mode. do not load ldfw/sp .\n");
			goto by_dumpgpr_out;
		}

		if (!init_keystorage())
			printf("keystorage: init done successfully.\n");
		else
			printf("keystorage: init failed.\n");

		if (!init_ssp())
			printf("ssp: init done successfully.\n");
		else
			printf("ssp: init failed.\n");

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

		/* Enabling H-Arx */
		if (s5p_chip_rev.main >= SOC_REVISION_EVT1) {
			if (load_and_init_harx()) {
				printf("CAN NOT enter EL2\n");
			} else {
				if (load_and_init_harx_plugin(EXYNOS_HARX_PLUGIN_PART_NAME,
								EXYNOS_HARX_PLUGIN_BASE_ADDR))
					printf("There is no H-Arx plug-in\n");
			}
		}

by_dumpgpr_out:
		print_el3_monitor_version();
	}

	display_tmu_info();
	display_trip_info();

	display_dvfs_info();

	chg_init_max77705();
}
