/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <debug.h>
#include <reg.h>
#include <app.h>
#include <part.h>
#include <stdlib.h>
#include <lib/console.h>
#include <lib/font_display.h>
#include <platform/mmu/mmu_func.h>
#include <lib/sysparam.h>
#include <platform/wdt_recovery.h>
#include <target/pmic.h>
#include <platform/sfr.h>
#include <platform/charger.h>
#include <platform/fastboot.h>
#include <platform/xct.h>
#include <dev/boot.h>
#include <dev/usb/gadget.h>
#include <dev/debug/dss.h>
#include <dev/debug/dss_store_ramdump.h>
#include <platform/gpio.h>
#include <platform/smc.h>
#include <lib/font_display.h>
#include <lib/logo_display.h>
#include <target/exynos_key.h>

#define CONFIG_PIT_IMAMGE_BASE 0x80100000
#define CONFIG_FWBL1_IMAMGE_BASE 0x80200000
#define CONFIG_EPBL_IMAMGE_BASE 0x80300000
#define CONFIG_BL2_IMAMGE_BASE 0x80400000
#define CONFIG_LK_IMAMGE_BASE 0x80500000
#define CONFIG_EL3_MON_IMAMGE_BASE 0x80700000
#define CONFIG_BOOT_IMAMGE_BASE 0x80800000
#define CONFIG_DTBO_IMAMGE_BASE 0x83000000
#define CONFIG_RAMDISK_IMAMGE_BASE 0x83100000

int cmd_boot(int argc, const cmd_args *argv);
int ab_update_slot_info_bootloader(void);
static void do_memtester(unsigned int loop);
extern unsigned int uart_log_mode;
extern unsigned int board_rev;

static void exynos_boot_task(const struct app_descriptor *app, void *args)
{
	unsigned int rst_stat = readl(EXYNOS_POWER_RST_STAT);
	int chk_wtsr_smpl;

	print_lcd_update(FONT_WHITE, FONT_BLACK, "Board revision : 0x%X", board_rev);

	if (*(unsigned int *)DRAM_BASE != 0xabcdef) {
		printf("Running on DRAM by TRACE32: skip auto booting\n");

		start_usb_gadget();
		return;
	}

	exynos_key_init();

	if ( exynos_key_check() == KEY_DETECT ) {
		start_usb_gadget();
		return;
	}


#ifdef CONFIG_WDT_RECOVERY_USB_BOOT
	clear_wdt_recovery_settings();
#endif
	/* check SMPL & WTSR */
	chk_wtsr_smpl = chk_smpl_wtsr();
	if (chk_wtsr_smpl == PMIC_DETECT_WTSR) {
		print_lcd_update(FONT_RED, FONT_BLACK, "WTSR DETECTED");
		printf("WTSR detected\n");
#ifdef S2MPU10_PM_IGNORE_WTSR_DETECT
		print_lcd_update(FONT_RED, FONT_BLACK, ",But Ignore WTSR DETECTION");
		printf(", but ignored by build config\n");
		writel(0, CONFIG_RAMDUMP_SCRATCH);
#endif
	} else if (chk_wtsr_smpl == PMIC_DETECT_SMPL) {
		print_lcd_update(FONT_RED, FONT_BLACK, "SMPL DETECTED");
		printf("SMPL detected\n");
#ifdef S2MPU10_PM_IGNORE_SMPL_DETECT
		print_lcd_update(FONT_RED, FONT_BLACK, ",But Ignore SMPL DETECTION");
		printf(", but ignored by build config\n");
		writel(0, CONFIG_RAMDUMP_SCRATCH);
#endif
	}

	/* Fastboot upload or download check */
	if (!is_first_boot()) {
		pmic_enable_manual_reset(PMIC_MRDT_3);
		printf("Entering fastboot: not first_boot\n");
		goto download;
	} else if (rst_stat & (WARM_RESET | LITTLE_WDT_RESET | BIG_WDT_RESET)) {
		printf("Entering fastboot: Abnormal RST_STAT: 0x%x\n", rst_stat);
		sdm_encrypt_secdram();
		dfd_set_dump_en(0);
		goto fastboot;
	} else if (readl(EXYNOS_POWER_SYSIP_DAT0) == REBOOT_MODE_FASTBOOT) {
		printf("Entering fastboot: reboot bootloader command\n");
		writel(0, CONFIG_RAMDUMP_SCRATCH);
		writel(0, EXYNOS_POWER_SYSIP_DAT0);
		goto download;
	} else if ((readl(CONFIG_RAMDUMP_SCRATCH) == CONFIG_RAMDUMP_MODE) && get_charger_mode() == 0) {
		printf("Entering fastboot: Ramdump_Scratch & Charger\n");
		sdm_encrypt_secdram();
		goto fastboot;
	} else {
		goto reboot;
	}

download:
	uart_log_mode = 1;
#ifndef RAMDUMP_MODE_OFF
	debug_store_ramdump();
#endif
	start_usb_gadget();
	return;

fastboot:
	uart_log_mode = 1;
#ifndef RAMDUMP_MODE_OFF
	debug_store_ramdump();
	start_usb_gadget();
	return;
#endif

reboot:

	/* Turn on dumpEN for DumpGPR */
#ifdef RAMDUMP_MODE_OFF
	dfd_set_dump_en(0);
	set_debug_level("low");
#else
	dfd_set_dump_en(1);
	set_debug_level("mid");
#endif
	cmd_boot(0, 0);
	return;
}

APP_START(exynos_boot)
	.entry = exynos_boot_task,
	.flags = 0,
APP_END
