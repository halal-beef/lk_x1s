/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __EXYNOS9830_H__
#define __EXYNOS9830_H__

#include <target/board_info.h>

/* SFR base address. */
#define EXYNOS9830_PRO_ID				0x10000000
#define EXYNOS9830_MCT_BASE				0x10040000
#define EXYNOS9830_MCT_G_TCON				(EXYNOS9830_MCT_BASE + 0x0240)
#define EXYNOS9830_GPIO_PERIC0_BASE			0x10430000
#define EXYNOS9830_GPIO_PERIC1_BASE			0x10730000
#define EXYNOS9830_GPP4CON				(EXYNOS9830_GPIO_PERIC0_BASE + 0x0080)
#define EXYNOS9830_UART_BASE				0x10540000
#define EXYNOS9830_GPIO_ALIVE_BASE			0x15850000
#define EXYNOS9830_GPA0CON				(EXYNOS9830_GPIO_ALIVE_BASE + 0x0000)
#define EXYNOS9830_POWER_BASE				0x15860000
#define EXYNOS9830_POWER_SYSTEM_CONFIGURATION		(EXYNOS9830_POWER_BASE + 0x3A00)
#define EXYNOS9830_POWER_RST_STAT			(EXYNOS9830_POWER_BASE + 0x0404)
#define EXYNOS9830_POWER_RESET_SEQUENCER_CONFIGURATION	(EXYNOS9830_POWER_BASE + 0x0500)
#define EXYNOS9830_POWER_INFORM3			(EXYNOS9830_POWER_BASE + 0x080C)
#define EXYNOS9830_POWER_SYSIP_DAT0			(EXYNOS9830_POWER_BASE + 0x0810)
#define EXYNOS9830_POWER_MIPI_PHY_M4S4_CONTROL		(EXYNOS9830_POWER_BASE + 0x0710)
#define EXYNOS9830_EDPCSR_DUMP_EN			(1 << 0)
#define EXYNOS9830_PWMTIMER_BASE			0x106F0000
#define EXYNOS9830_SYSREG_DPU				0x19021000

#define CONFIG_SPEEDY0_BASE				0x15940000
#define CONFIG_SPEEDY1_BASE				0x15950000

#define EXYNOS9830_TMU_TOP_BASE				0x10090000
#define EXYNOS9830_TMU_SUB_BASE				0x100A0000

/* CHIP ID */
#define CHIPID0_OFFSET					0x4
#define CHIPID1_OFFSET					0x8

#define BOOT_DEV_INFO					EXYNOS9830_POWER_INFORM3
#define BOOT_DEV					readl(EXYNOS9830_POWER_INFORM3)

#define DRAM_BASE					0x80000000
#define DRAM_BASE2					0x880000000

#define CFG_FASTBOOT_MMC_BUFFER				(0xC0000000)
#define CONFIG_LK_TEXT_BASE				(0xF8800000)

/* iRAM information */
#define IRAM_BASE					(0x02020000)
#define IRAM_NS_BASE					(IRAM_BASE + 0x18000)
#define BL_SYS_INFO					(IRAM_NS_BASE + 0x800)
#define BL_SYS_INFO_DRAM_SIZE				(BL_SYS_INFO + 0x48)
#define CONFIG_IRAM_STACK				(IRAM_NS_BASE + 0x1000)
#define DRAM_INFO					(IRAM_BASE + 0x2C000)
#define DRAM_SIZE_INFO					(IRAM_BASE + 0x18848)

#define POWER_RST_STAT					(EXYNOS9830_POWER_RST_STAT)
#define WARM_RESET					(1 << 28)
#define LITTLE_WDT_RESET				(1 << 24)
#define BIG_WDT_RESET					(1 << 23)
#define PIN_RESET					(1 << 16)

/* DEBUG CORE MAILBOX register */
#define EXYNOS9830_DBG_MBOX_BASE			(0x158D0000)
#define EXYNOS9830_DBG_MBOX_SR				(EXYNOS9830_DBG_MBOX_BASE + 0x80)
#define EXYNOS9830_DBG_MBOX_SRn(x)			(EXYNOS9830_DBG_MBOX_BASE + (0x80 + (x * 4)))
#define EXYNOS9830_DBG_MBOX_FW_CH			(EXYNOS9830_DBG_MBOX_SR + (4 * 16))

#define CONFIG_PRINT_TIMESTAMP
#define CONFIG_RAMDUMP_GPR
#define CONFIG_RAMDUMP_MODE          			0xD
#define CONFIG_RAMDUMP_OFFSET				(0x7D900000)
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
#define RAMDUMP_SIGN_BL_REBOOT				0xBADA
#define RAMDUMP_SIGN_DEBUG_TEST				0xDB6

#define RAMDUMP_REBOOT_TBD				0x0
#define RAMDUMP_REBOOT_WTSR				0x1
#define RAMDUMP_REBOOT_SMPL				0x2
#define RAMDUMP_REBOOT_WDT				0x3
#define RAMDUMP_REBOOT_PANIC				0x4

/* AVB2.0 */
#define CONFIG_USE_AVB20
#define CONFIG_AVB_HW_HASH
#define CONFIG_AVB_LCD_LOG

#define BOOT_BASE					0x94000000
#define KERNEL_BASE					0x80080000
#define RAMDISK_BASE					0x84000000
#define DT_BASE						0x8A000000
#define DTBO_BASE					0x8B000000
#define ECT_BASE					0x90000000
#define ECT_SIZE					0x32000

/* ACPM information */
#define EXYNOS_ACPM_BASE				(0x02039000)
#define EXYNOS_ACPM_APSHARE				(EXYNOS_ACPM_BASE + 0x7000)
#define EXYNOS_ACPM_MAGIC				(EXYNOS_ACPM_BASE + 0x3000)
#define ACPM_MAGIC_VALUE				(0xE800)

/* display GPIO for TE */
#define EXYNOS9830_GPC0CON				(EXYNOS9830_GPIO_PERIC1_BASE + 0x00A0)
#define EXYNOS9830_GPC0DAT				(EXYNOS9830_GPIO_PERIC1_BASE + 0x00A4)
#define EXYNOS9830_GPC0PUD				(EXYNOS9830_GPIO_PERIC1_BASE + 0x00A8)
/* displya GPIO for reset panel(3HA8) */
#define EXYNOS9830_GPP5CON				(EXYNOS9830_GPIO_PERIC1_BASE + 0x0000)
#define EXYNOS9830_GPP5DAT				(EXYNOS9830_GPIO_PERIC1_BASE + 0x0004)
#define EXYNOS9830_GPP5PUD				(EXYNOS9830_GPIO_PERIC1_BASE + 0x0008)
#define EXYNOS9830_GPP5DRV				(EXYNOS9830_GPIO_PERIC1_BASE + 0x000C)
/* displya GPIO for reset panel(3HA9 @ UNIV) */
#define EXYNOS9830_GPA3CON				(EXYNOS9830_GPIO_ALIVE_BASE + 0x0060)
#define EXYNOS9830_GPA3DAT				(EXYNOS9830_GPIO_ALIVE_BASE + 0x0064)
#define EXYNOS9830_GPA3PUD				(EXYNOS9830_GPIO_ALIVE_BASE + 0x0068)
#define EXYNOS9830_GPA3DRV				(EXYNOS9830_GPIO_ALIVE_BASE + 0x006C)
/* Maran V2 panel power en : POLED_VCI_EN: GPA1[2]) of 3HA8 */
#define EXYNOS9830_GPA1CON				(EXYNOS9830_GPIO_ALIVE_BASE + 0x0020)
#define EXYNOS9830_GPA1DAT				(EXYNOS9830_GPIO_ALIVE_BASE + 0x0024)

#endif	/* __EXYNOS9830_H__ */
