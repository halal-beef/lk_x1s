/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __SFR_H__
#define __SFR_H__

#include "exynos9830.h"

#define EXYNOS_SYSREG_DPU	EXYNOS9830_SYSREG_DPU
#define EXYNOS_POWER_RST_STAT	EXYNOS9830_POWER_RST_STAT

#define EXYNOS_PMU_BASE					EXYNOS9830_POWER_BASE
#define EXYNOS_POWER_SYSIP_DAT0				EXYNOS9830_POWER_SYSIP_DAT0
#define EXYNOS_POWER_SYSTEM_CONFIGURATION		EXYNOS9830_POWER_SYSTEM_CONFIGURATION
#define EXYNOS_PMU_RESET_SEQ_CONFIG			EXYNOS9830_POWER_RESET_SEQUENCER_CONFIGURATION
#define EXYNOS_PMU_RST_STAT				EXYNOS9830_POWER_RST_STAT

#define EXYNOS_PWMTIMER_BASE				EXYNOS9830_PWMTIMER_BASE

/* DEBUG CORE MAILBOX register */
#define EXYNOS_DBG_MBOX_BASE				(EXYNOS9830_DBG_MBOX_BASE)
#define EXYNOS_DBG_MBOX_SR				(EXYNOS_DBG_MBOX_BASE + 0x80)
#define EXYNOS_DBG_MBOX_SRn(x)				(EXYNOS_DBG_MBOX_BASE + (0x80 + (x * 4)))
#define EXYNOS_DBG_MBOX_FW_CH				(EXYNOS_DBG_MBOX_SR + (4 * 16))

#define CONFIG_RAMDUMP_MODE          			0xD
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
#define CONFIG_RAMDUMP_DPM_BASE                         (CONFIG_RAMDUMP_BASE + 0x5000)
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

#define RAMDUMP_BOOT_CNT_MAGIC				0xFACEDB90

#define EXYNOS_ADC_BASE					EXYNOS9830_ADC_BASE

#define EXYNOS_UART_BASE				EXYNOS9830_UART_BASE
#define UART_SRCCLK					133250000
#define UART_BAUDRATE					115200
#define UARTGPIO_CON					EXYNOS9830_GPP4CON /* UART_DEBUG0, Speedy */
#define UARTGPIO_CON_MASK				0xFF
#define UARTGPIO_CON_SET				0x33
#define UARTGPIO_CON_BASE_BIT				(6 * 4)

#endif	/* __SFR_H__ */