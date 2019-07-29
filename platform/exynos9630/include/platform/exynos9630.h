/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __EXYNOS9630_H__
#define __EXYNOS9630_H__

#include <target/board_info.h>

/* SFR base address. */
#define EXYNOS9630_PRO_ID								0x10000000
#define EXYNOS9630_GPIO_PERI_BASE 							0x10040000
#define EXYNOS9630_GPP2CON								(EXYNOS9630_GPIO_PERI_BASE + 0x0040)
#define EXYNOS9630_GPP2DAT								(EXYNOS9630_GPIO_PERI_BASE + 0x0044)
#define EXYNOS9630_GPP2PUD								(EXYNOS9630_GPIO_PERI_BASE + 0x0048)
#define EXYNOS9630_GPIO_PERIC_BASE							0x10040000
#define EXYNOS9630_MCT_BASE								0x10050000
#define EXYNOS9630_MCT_G_TCON								(EXYNOS9630_MCT_BASE + 0x0240)
#define EXYNOS9630_WDT_MASK_RESET_BIT							(23)
#define EXYNOS9630_WDT_FREQ								(26000000)
#define EXYNOS9630_WDT_INIT_PRESCALER							(128)
#define EXYNOS9630_WDT_BASE								0x10060000
#define EXYNOS9630_WDT_WTCON								(EXYNOS9630_WDT_BASE + 0x0000)
#define EXYNOS9630_WDT_WTDAT								(EXYNOS9630_WDT_BASE + 0x0004)
#define EXYNOS9630_WDT_WTCNT								(EXYNOS9630_WDT_BASE + 0x0008)
#define EXYNOS9630_WDT_WTCLRINT								(EXYNOS9630_WDT_BASE + 0x000C)
#define EXYNOS9630_WDT_PRESCALE(x)							((x) << 8)
#define EXYNOS9630_WDT_PRESCALE_MASK							(0xff << 8)

#define EXYNOS9630_WDT_WTCON_RSTEN							(1 << 0)
#define EXYNOS9630_WDT_WTCON_INTEN							(1 << 2)
#define EXYNOS9630_WDT_WTCON_ENABLE							(1 << 5)

#define EXYNOS9630_WDT_WTCON_DIV16							(0 << 3)
#define EXYNOS9630_WDT_WTCON_DIV32							(1 << 3)
#define EXYNOS9630_WDT_WTCON_DIV64							(2 << 3)
#define EXYNOS9630_WDT_WTCON_DIV128							(3 << 3)

#define EXYNOS9630_GPIO_ALIVE_BASE 						0x10E50000
#define EXYNOS9630_GPA0CON							(EXYNOS9630_GPIO_ALIVE_BASE + 0x0000)
#define EXYNOS9630_GPA0DAT							(EXYNOS9630_GPIO_ALIVE_BASE + 0x0004)
#define EXYNOS9630_GPA0PUD							(EXYNOS9630_GPIO_ALIVE_BASE + 0x0008)
#define EXYNOS9630_GPA1CON							(EXYNOS9630_GPIO_ALIVE_BASE + 0x0020)
#define EXYNOS9630_GPA1DAT							(EXYNOS9630_GPIO_ALIVE_BASE + 0x0024)
#define EXYNOS9630_GPA1PUD							(EXYNOS9630_GPIO_ALIVE_BASE + 0x0028)
#define EXYNOS9630_GPQ0CON 							(EXYNOS9630_GPIO_ALIVE_BASE + 0x0040)
#define EXYNOS9630_GPQ0PUD 							(EXYNOS9630_GPIO_ALIVE_BASE + 0x0048)
#define EXYNOS9630_POWER_BASE							0x10E60000
#define EXYNOS9630_POWER_RST_STAT 						(EXYNOS9630_POWER_BASE + 0x0404)
#define EXYNOS9630_POWER_MASK_WDT_RESET_REQUEST					(EXYNOS9630_POWER_BASE + 0x040C)
#define EXYNOS9630_POWER_RESET_SEQUENCER_CONFIGURATION				(EXYNOS9630_POWER_BASE + 0x0500)
#define EXYNOS9630_POWER_INFORM3 						(EXYNOS9630_POWER_BASE + 0x080C)
#define EXYNOS9630_POWER_SYSIP_DAT0 						(EXYNOS9630_POWER_BASE + 0x0810)
#define EXYNOS9630_POWER_DREX_CALIBRATION7						(EXYNOS9630_POWER_BASE + 0x09BC)
#define EXYNOS9630_POWER_MIPI_PHY_M4S4_CONTROL					(EXYNOS9630_POWER_BASE + 0x070C)
#define EXYNOS9630_EDPCSR_DUMP_EN						(1 << 0)
#define EXYNOS9630_SYSTEM_CONFIGURATION 					(EXYNOS9630_POWER_BASE + 0x3A00)

#define EXYNOS9630_TMU_TOP_BASE							0x10080000
#define EXYNOS9630_UART_BASE 							0x13800000
#define EXYNOS9630_PWMTIMER_BASE						0x10090000
#define EXYNOS9630_SYSREG_DPU							0x14821000

#define EXYNOS_POWER_SYSIP_DAT0				EXYNOS9630_POWER_SYSIP_DAT0
#define EXYNOS_POWER_SYSTEM_CONFIGURATION			EXYNOS9630_SYSTEM_CONFIGURATION

/* CHIP ID */
#define CHIPID0_OFFSET								0x4
#define CHIPID1_OFFSET								0x8

#define BOOT_DEV_INFO								EXYNOS9630_POWER_INFORM3
#define BOOT_DEV								readl(EXYNOS9630_POWER_INFORM3)

#define DRAM_BASE								0x80000000
#define DRAM_BASE2								0x880000000

#define CFG_FASTBOOT_MMC_BUFFER							(0xC0000000)
#define CONFIG_LK_TEXT_BASE							(0xFF000000)

/* iRAM information */
#define IRAM_BASE								(0x02020000)
#define IRAM_NS_BASE								(IRAM_BASE + 0x18000)
#define BL_SYS_INFO								(IRAM_NS_BASE + 0x800)
#define BL_SYS_INFO_DRAM_SIZE							(BL_SYS_INFO + 0x48)
#define CONFIG_IRAM_STACK							(IRAM_NS_BASE + 0x1000)
#define DRAM_INFO								(IRAM_BASE + 0x2C000)
#define DRAM_SIZE_INFO								(IRAM_BASE + 0x18848)

#define CONFIG_DRAM_TRAINING_AREA_BASE						(0x0212F000)
#define CONFIG_DRAM_TRAINING_AREA_SIZE						0x4000
#define CONFIG_DRAM_TRAINING_AREA_BLOCK_OFFSET					528

#define SWRESET									(1 << 30)
#define WARM_RESET								(1 << 29)
#define LITTLE_WDT_RESET							(1 << 24)
#define BIG_WDT_RESET								(1 << 25)
#define PIN_RESET								(1 << 16)

#define CONFIG_PRINT_TIMESTAMP

/* DEBUG CORE MAILBOX register */
#define EXYNOS9630_DBG_MBOX_BASE			(0x10EC0000)
#define EXYNOS9630_DBG_MBOX_SR				(EXYNOS9630_DBG_MBOX_BASE + 0x80)
#define EXYNOS9630_DBG_MBOX_SRn(x)			(EXYNOS9630_DBG_MBOX_BASE + (0x80 + (x * 4)))
#define EXYNOS9630_DBG_MBOX_FW_CH			(EXYNOS9630_DBG_MBOX_SR + (4 * 16))

#define CONFIG_RAMDUMP_GPR
#define CONFIG_RAMDUMP_MODE          			0xD
#define CONFIG_RAMDUMP_OFFSET				(0x70000000)
#define CONFIG_RAMDUMP_LOG_OFFSET			(0x10000)
#define CONFIG_RAMDUMP_BASE				(DRAM_BASE + CONFIG_RAMDUMP_OFFSET)
#define CONFIG_RAMDUMP_LOGBUF   		        (CONFIG_RAMDUMP_BASE + CONFIG_RAMDUMP_LOG_OFFSET)
#define CONFIG_RAMDUMP_LOGSZ    		        0x200000
#define CONFIG_RAMDUMP_PANIC_LOGSZ     			 0x400

#define CONFIG_RAMDUMP_SCRATCH				(CONFIG_RAMDUMP_BASE + 0x100)
#define CONFIG_RAMDUMP_DEBUG_TEST			(CONFIG_RAMDUMP_BASE + 0x110)
#define CONFIG_RAMDUMP_DEBUG_TEST_CASE			(CONFIG_RAMDUMP_BASE + 0x120)
#define CONFIG_RAMDUMP_DEBUG_TEST_NEXT			(CONFIG_RAMDUMP_BASE + 0x128)
#define CONFIG_RAMDUMP_DEBUG_TEST_PANIC			(CONFIG_RAMDUMP_BASE + 0x130)
#define CONFIG_RAMDUMP_DEBUG_TEST_WDT			(CONFIG_RAMDUMP_BASE + 0x140)
#define CONFIG_RAMDUMP_DEBUG_TEST_WTSR			(CONFIG_RAMDUMP_BASE + 0x150)
#define CONFIG_RAMDUMP_DEBUG_TEST_SMPL			(CONFIG_RAMDUMP_BASE + 0x160)
#define CONFIG_RAMDUMP_DEBUG_TEST_CURR			(CONFIG_RAMDUMP_BASE + 0x170)
#define CONFIG_RAMDUMP_DEBUG_TEST_TOTAL			(CONFIG_RAMDUMP_BASE + 0x178)
#define CONFIG_RAMDUMP_DEBUG_LEVEL			(CONFIG_RAMDUMP_BASE + 0x180)
#define CONFIG_RAMDUMP_LASTBUF				(CONFIG_RAMDUMP_BASE + 0x200)
#define CONFIG_RAMDUMP_BL_BOOT_CNT_MAGIC		(CONFIG_RAMDUMP_BASE + 0x250)
#define CONFIG_RAMDUMP_BL_BOOT_CNT			(CONFIG_RAMDUMP_BASE + 0x254)
#define CONFIG_RAMDUMP_KERNEL_BOOT_CNT_MAGIC		(CONFIG_RAMDUMP_BASE + 0x260)
#define CONFIG_RAMDUMP_KERNEL_BOOT_CNT			(CONFIG_RAMDUMP_BASE + 0x264)
#define CONFIG_RAMDUMP_REASON				(CONFIG_RAMDUMP_BASE + 0x300)
#define CONFIG_RAMDUMP_DUMP_GPR_DEBUG			(CONFIG_RAMDUMP_BASE + 0x320)
#define CONFIG_RAMDUMP_DUMP_GPR_WAIT			(CONFIG_RAMDUMP_BASE + 0x380)
#define CONFIG_RAMDUMP_WAKEUP_WAIT			(CONFIG_RAMDUMP_BASE + 0x390)
#define CONFIG_RAMDUMP_CORE_POWER_STAT			(CONFIG_RAMDUMP_BASE + 0x400)
#define CONFIG_RAMDUMP_GPR_POWER_STAT			(CONFIG_RAMDUMP_BASE + 0x480)
#define CONFIG_RAMDUMP_CORE_PANIC_STAT			(CONFIG_RAMDUMP_BASE + 0x500)
#define CONFIG_RAMDUMP_DSS_ITEM_INFO			(CONFIG_RAMDUMP_BASE + 0x900)
#define CONFIG_RAMDUMP_PANIC_REASON			(CONFIG_RAMDUMP_BASE + 0xC00)

#define CONFIG_RAMDUMP_COREREG				(CONFIG_RAMDUMP_BASE + 0x2000)
#define CONFIG_RAMDUMP_DBGC_VERSION			(CONFIG_RAMDUMP_BASE + 0x3000)
#define CONFIG_RAMDUMP_STACK				(CONFIG_RAMDUMP_BASE + 0x10000)

#define RAMDUMP_SIGN_RESET				0x0
#define RAMDUMP_SIGN_RESERVED				0x1
#define RAMDUMP_SIGN_SCRATCH				0xD
#define RAMDUMP_SIGN_ALIVE				0xFACE
#define RAMDUMP_SIGN_DEAD				0xDEAD
#define RAMDUMP_SIGN_PANIC				0xBABA
#define RAMDUMP_SIGN_NORMAL_REBOOT			0xCAFE
#define RAMDUMP_SIGN_FORCE_REBOOT			0xDAFE
#define RAMDUMP_SIGN_SAFE_FAULT				0xFAFA
#define RAMDUMP_SIGN_DEBUG_TEST				0xDB6
#define RAMDUMP_SIGN_BL_REBOOT          		0xBADA

#define RAMDUMP_REBOOT_TBD				0x0
#define RAMDUMP_REBOOT_WTSR				0x1
#define RAMDUMP_REBOOT_SMPL				0x2
#define RAMDUMP_REBOOT_WDT				0x3
#define RAMDUMP_REBOOT_PANIC				0x4

/* AVB2.0 */
#define CONFIG_USE_AVB20
#if defined(CONFIG_USE_AVB20)
#define CONFIG_AVB_HW_HASH
#define CONFIG_AVB_ROT
#define CONFIG_AVB_CMDLINE
#undef CONFIG_AVB_RP_UPDATE
#undef CONFIG_OTP_RP_UPDATE
#endif

#define BOOT_BASE			0x94000000
#define KERNEL_BASE			0x80080000
#define RAMDISK_BASE			0x84000000
#define DT_BASE				0x8A000000
#define DTBO_BASE			0x8B000000
#define ECT_BASE			0x90000000
#define ECT_SIZE			0x32000

#define FB_MODE_FLAG              	0x00FB00DE
#define UART_LOG_MODE_FLAG        	0x0A4200DE

/* GPIO */
#define CONFIG_GPIO_4BIT_PUD

/*
 * AB Update Support only
 * Below GLOBAL VALUE can be defined in
 * project/{PROJECT}.mk
 */
#ifdef SUPPORT_AB_UPDATE
#define CONFIG_AB_UPDATE
#endif

/*
 * reboot mode
 */
#define REBOOT_MODE_RECOVERY	0xFF
#define REBOOT_MODE_FACTORY	0xFD
#define REBOOT_MODE_FASTBOOT	0xFC

/* display GPIO for TE : GPG0[1] */
#define EXYNOS9630_GPG0CON				(EXYNOS9630_GPIO_PERIC_BASE + 0x0080)
#define EXYNOS9630_GPG0DAT				(EXYNOS9630_GPIO_PERIC_BASE + 0x0084)
#define EXYNOS9630_GPG0PUD				(EXYNOS9630_GPIO_PERIC_BASE + 0x0088)

/* display GPIO for reset panel : GPQ0[2] */
//#define EXYNOS9630_GPQ0CON				(EXYNOS9630_GPIO_ALIVE_BASE + 0x0040)
#define EXYNOS9630_GPQ0DAT				(EXYNOS9630_GPIO_ALIVE_BASE + 0x0044)
//#define EXYNOS9630_GPQ0PUD				(EXYNOS9630_GPIO_ALIVE_BASE + 0x0048)
#define EXYNOS9630_GPQ0DRV				(EXYNOS9630_GPIO_ALIVE_BASE + 0x004C)

/* display GPIO for vdd_disp_1p6_en signal : GPP3[2] */
#define EXYNOS9630_GPP3CON				(EXYNOS9630_GPIO_PERIC_BASE + 0x0060)
#define EXYNOS9630_GPP3DAT				(EXYNOS9630_GPIO_PERIC_BASE + 0x0064)
#define EXYNOS9630_GPP3PUD				(EXYNOS9630_GPIO_PERIC_BASE + 0x0068)
#define EXYNOS9630_GPP3DRV				(EXYNOS9630_GPIO_PERIC_BASE + 0x006C)

#endif	/* __EXYNOS9630_H__ */
