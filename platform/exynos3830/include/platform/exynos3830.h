/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __EXYNOS3830_H__
#define __EXYNOS3830_H__

#include <target/board_info.h>

/* SFR base address. */
#define EXYNOS3830_PRO_ID								0x10000000
#define EXYNOS3830_MCT_BASE								0x10040000
#define EXYNOS3830_MCT_G_TCON								(EXYNOS3830_MCT_BASE + 0x0240)
#define EXYNOS3830_WDT_MASK_RESET_BIT							(23)
#define EXYNOS3830_WDT_FREQ								(26000000)
#define EXYNOS3830_WDT_INIT_PRESCALER							(128)
#define EXYNOS3830_WDT_BASE								0x10050000
#define EXYNOS3830_WDT_WTCON								(EXYNOS3830_WDT_BASE + 0x0000)
#define EXYNOS3830_WDT_WTDAT								(EXYNOS3830_WDT_BASE + 0x0004)
#define EXYNOS3830_WDT_WTCNT								(EXYNOS3830_WDT_BASE + 0x0008)
#define EXYNOS3830_WDT_WTCLRINT								(EXYNOS3830_WDT_BASE + 0x000C)
#define EXYNOS3830_WDT_PRESCALE(x)							((x) << 8)
#define EXYNOS3830_WDT_PRESCALE_MASK							(0xff << 8)

#define EXYNOS3830_WDT_WTCON_RSTEN							(1 << 0)
#define EXYNOS3830_WDT_WTCON_INTEN							(1 << 2)
#define EXYNOS3830_WDT_WTCON_ENABLE							(1 << 5)

#define EXYNOS3830_WDT_WTCON_DIV16							(0 << 3)
#define EXYNOS3830_WDT_WTCON_DIV32							(1 << 3)
#define EXYNOS3830_WDT_WTCON_DIV64							(2 << 3)
#define EXYNOS3830_WDT_WTCON_DIV128							(3 << 3)

#define EXYNOS3830_GPIO_ALIVE_BASE 						0x11850000
#define EXYNOS3830_GPIO_PERI_BASE 						0x139B0000
#define EXYNOS3830_GPA0CON							(EXYNOS3830_GPIO_ALIVE_BASE + 0x0000)
#define EXYNOS3830_GPA0DAT							(EXYNOS3830_GPIO_ALIVE_BASE + 0x0004)
#define EXYNOS3830_GPA0PUD							(EXYNOS3830_GPIO_ALIVE_BASE + 0x0008)
#define EXYNOS3830_GPA1CON							(EXYNOS3830_GPIO_ALIVE_BASE + 0x0020)
#define EXYNOS3830_GPA1DAT							(EXYNOS3830_GPIO_ALIVE_BASE + 0x0024)
#define EXYNOS3830_GPA1PUD							(EXYNOS3830_GPIO_ALIVE_BASE + 0x0028)
#define EXYNOS3830_GPQ0CON 							(EXYNOS3830_GPIO_ALIVE_BASE + 0x00A0)
#define EXYNOS3830_GPQ0PUD 							(EXYNOS3830_GPIO_ALIVE_BASE + 0x00A8)
#define EXYNOS3830_POWER_BASE							0x11860000
#define EXYNOS3830_POWER_RST_STAT 						(EXYNOS3830_POWER_BASE + 0x0404)
#define EXYNOS3830_POWER_RESET_SEQUENCER_CONFIGURATION				(EXYNOS3830_POWER_BASE + 0x0500)
#define EXYNOS3830_POWER_INFORM3 						(EXYNOS3830_POWER_BASE + 0x080C)
#define EXYNOS3830_POWER_SYSIP_DAT0 						(EXYNOS3830_POWER_BASE + 0x0810)
#define EXYNOS3830_POWER_DREX_CALIBRATION7						(EXYNOS3830_POWER_BASE + 0x09BC)
#define EXYNOS3830_POWER_MIPI_PHY_M4S4_CONTROL					(EXYNOS3830_POWER_BASE + 0x070C)
#define EXYNOS3830_EDPCSR_DUMP_EN						(1 << 0)
#define EXYNOS3830_SYSTEM_CONFIGURATION 					(EXYNOS3830_POWER_BASE + 0x3A00)
#define EXYNOS3830_ADC_BASE							(0x11C40000)
#define EXYNOS3830_TMU_TOP_BASE							0x10070000
#define EXYNOS3830_UART_BASE 							0x13820000
#define EXYNOS3830_PWMTIMER_BASE						0x13970000
#define EXYNOS3830_SYSREG_DPU							0x13020000


/* CHIP ID */
#define CHIPID0_OFFSET								0x4
#define CHIPID1_OFFSET								0x8

#define BOOT_DEV_INFO								EXYNOS3830_POWER_INFORM3
#define BOOT_DEV								readl(EXYNOS3830_POWER_INFORM3)

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

#define CONFIG_BL1_BLOCK_SIZE					24
#define CONFIG_EPBL_BLOCK_SIZE					152
#define CONFIG_BL2_BLOCK_SIZE					512

#define CONFIG_DRAM_TRAINING_AREA_BASE						(0x14B00000)
#define CONFIG_DRAM_TRAINING_AREA_SIZE						0x4000
#define CONFIG_DRAM_TRAINING_AREA_BLOCK_OFFSET				(CONFIG_BL1_BLOCK_SIZE + CONFIG_EPBL_BLOCK_SIZE + CONFIG_BL2_BLOCK_SIZE)

#define EXYNOS3830_ACPM_BASE							(0x02039000)
#define EXYNOS3830_ACPM_MAGIC							(EXYNOS3830_ACPM_BASE + 0x3000)
#define EXYNOS3830_ACPM_APSHARE							(EXYNOS3830_ACPM_BASE + 0x7000)

#define CONFIG_RAMDUMP_SRAM_LOG_BASE                    			(0x14B04000 + 0x28000)
#define CONFIG_RAMDUMP_BL_LOG_MAGIC						(0xb1109)

#define SWRESET									(1 << 29)
#define WARM_RESET								(1 << 28)
#define LITTLE_WDT_RESET							(1 << 24)
#define BIG_WDT_RESET								(1 << 23)
#define APM_WDT_RESET								(1 << 20)
#define PIN_RESET								(1 << 16)

#define CONFIG_PRINT_TIMESTAMP

#define CONFIG_MMU_ENABLE

#define CONFIG_RAMDUMP_GPR
#define CONFIG_RAMDUMP_OFFSET				(0x70000000)
#define CONFIG_RAMDUMP_LOG_OFFSET			(0x10000)

#define EXYNOS_MAILBOX_AP2APM           (0x11900000)

/* AVB2.0 */
#define CONFIG_USE_AVB20
#if defined(CONFIG_USE_AVB20)
#define CONFIG_AVB_HW_HASH
#define CONFIG_AVB_ROT
#define CONFIG_AVB_CMDLINE
#define CONFIG_AVB_LCD_LOG
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
#define REBOOT_MODE_FASTBOOT_USER	0xFA

/* Use Boot.img */
#define CONFIG_BOOT_IMAGE_SUPPORT

/* display GPIO for TE : GPA4[1] */
#define EXYNOS3830_GPA4CON				(EXYNOS3830_GPIO_ALIVE_BASE + 0x0080)
#define EXYNOS3830_GPA4DAT				(EXYNOS3830_GPIO_ALIVE_BASE + 0x0084)
#define EXYNOS3830_GPA4PUD				(EXYNOS3830_GPIO_ALIVE_BASE + 0x0088)

/* display GPIO for reset panel : GPG2[5] */
#define EXYNOS3830_GPG2CON                              (EXYNOS3830_GPIO_PERI_BASE + 0x00A0)
#define EXYNOS3830_GPG2DAT                              (EXYNOS3830_GPIO_PERI_BASE + 0x00A4)
#define EXYNOS3830_GPG2PUD                              (EXYNOS3830_GPIO_PERI_BASE + 0x00A8)
#define EXYNOS3830_GPG2DRV                              (EXYNOS3830_GPIO_PERI_BASE + 0x00AC)

/* display GPIO for lcd_ldo_en signal : GPG1[1] */
#define EXYNOS3830_GPG1CON                            	(EXYNOS3830_GPIO_PERI_BASE + 0x0080)
#define EXYNOS3830_GPG1DAT                              (EXYNOS3830_GPIO_PERI_BASE + 0x0084)
#define EXYNOS3830_GPG1PUD                            	(EXYNOS3830_GPIO_PERI_BASE + 0x0088)
#define EXYNOS3830_GPG1DRV                              (EXYNOS3830_GPIO_PERI_BASE + 0x008C)

/* Skip SCSI for mmc, exynos3830 */
#define CONFIG_NOT_SCSI

#endif	/* __EXYNOS3830_H__ */
