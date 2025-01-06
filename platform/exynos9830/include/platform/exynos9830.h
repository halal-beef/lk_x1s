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

/* CHIP REV */
#define CHIPID_REV_OFFSET				0x10

#define REVISION_MASK					(0xF)
#define MAIN_REVISION_SHIFT				(20)
#define SUB_REVISION_SHIFT				(16)

#define SOC_REVISION_EVT0				(0)
#define SOC_REVISION_EVT1				(1)

#define BOOT_DEV_INFO					EXYNOS9830_POWER_INFORM3
#define BOOT_DEV					readl(EXYNOS9830_POWER_INFORM3)

#define DRAM_BASE					0x80000000
#define DRAM_BASE2					0x880000000

#define CFG_FASTBOOT_MMC_BUFFER				(0xC2000000)
#define CONFIG_LK_TEXT_BASE				(0x90000000)

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

#define CONFIG_PRINT_TIMESTAMP
#define CONFIG_RAMDUMP_GPR
#define CONFIG_RAMDUMP_OFFSET				(0x7D900000)
#define CONFIG_RAMDUMP_LOG_OFFSET			(0x10000)

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

#define REBOOT_MODE_FASTBOOT_USER       		0xFA

/* display GPIO for TE */
#define EXYNOS9830_GPC0CON				(EXYNOS9830_GPIO_PERIC1_BASE + 0x00A0)
#define EXYNOS9830_GPC0DAT				(EXYNOS9830_GPIO_PERIC1_BASE + 0x00A4)
#define EXYNOS9830_GPC0PUD				(EXYNOS9830_GPIO_PERIC1_BASE + 0x00A8)
/* displya GPIO for reset panel(3HA8) */
#define EXYNOS9830_GPP5CON				(EXYNOS9830_GPIO_PERIC1_BASE + 0x0000)
#define EXYNOS9830_GPP5DAT				(EXYNOS9830_GPIO_PERIC1_BASE + 0x0004)
#define EXYNOS9830_GPP5PUD				(EXYNOS9830_GPIO_PERIC1_BASE + 0x0008)
#define EXYNOS9830_GPP5DRV				(EXYNOS9830_GPIO_PERIC1_BASE + 0x000C)
/* power button GPIO */
#define EXYNOS9830_GPA2CON				(EXYNOS9830_GPIO_ALIVE_BASE + 0x0040)
/* displya GPIO for reset panel(3HA9 @ UNIV) */
#define EXYNOS9830_GPA3CON				(EXYNOS9830_GPIO_ALIVE_BASE + 0x0060)
#define EXYNOS9830_GPA3DAT				(EXYNOS9830_GPIO_ALIVE_BASE + 0x0064)
#define EXYNOS9830_GPA3PUD				(EXYNOS9830_GPIO_ALIVE_BASE + 0x0068)
#define EXYNOS9830_GPA3DRV				(EXYNOS9830_GPIO_ALIVE_BASE + 0x006C)
/* Maran V2 panel power en : POLED_VCI_EN: GPA1[2]) of 3HA8 */
#define EXYNOS9830_GPA1CON				(EXYNOS9830_GPIO_ALIVE_BASE + 0x0020)
#define EXYNOS9830_GPA1DAT				(EXYNOS9830_GPIO_ALIVE_BASE + 0x0024)

#define EXYNOS9830_ADC_BASE				(0x15C40000)

#define SEC_DEBUG_MAGIC_INFORM          0x0808
#define SEC_DEBUG_PANIC_INFORM          0x080C

#define SEC_REBOOT_END_OFFSET           (16)

#define SEC_RESET_REASON_PREFIX         0x12345600

#define SEC_REBOOT_NORMAL               0x4E
#define SEC_RESET_REASON_UNKNOWN        (SEC_RESET_REASON_PREFIX | 0x00)
#define SEC_RESET_REASON_DOWNLOAD       (SEC_RESET_REASON_PREFIX | 0x01)
#define SEC_RESET_REASON_RECOVERY       (SEC_RESET_REASON_PREFIX | 0x04)
#define SEC_RESET_REASON_BOOTLOADER     (SEC_RESET_REASON_PREFIX | 0x0D)
#define REBOOT_MODE_FASTBOOT            0xFA
#define REBOOT_MODE_LK3RD               0x4C // Custom reboot mode

#endif	/* __EXYNOS9830_H__ */
