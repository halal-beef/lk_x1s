/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 *
 * No part of this software, either material or conceptual may be copied or
 * distributed, transmitted, transcribed, stored in a retrieval system or
 * translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed to third parties
 * without the express written permission of Samsung Electronics.
 *
 */
#ifndef _EXYNOS9830_DFD_H
#define _EXYNOS9830_DFD_H

#define NR_CPUS				(8)
#define NR_BIG_CPUS			(4)
#define NR_LITTLE_CPUS			(4)

#define EDPCSR_DUMP_EN			(1 << 0)

#define DUMPGPR_EN			(0x1 << 0)
#define DUMPGPR_EN_MASK			(0x1 << 0)
#define CACHE_RESET_EN			(0xF << 4)
#define CACHE_RESET_EN_MASK		(0xF << 4)
#define CLUSTER_RSTCON_EN_MASK		(0xF << 8)

#define L2RSTDISABLE_BIG_EN		(1 << 11)
#define L1RSTDISABLE_BIG_EN		(1 << 10)
#define L2RSTDISABLE_LITTLE_EN		(1 << 9)
#define L1RSTDISABLE_LITTLE_EN		(1 << 8)

#define L2RSTDISABLE_BIG_CLR		(1 << 7)
#define L1RSTDISABLE_BIG_CLR		(1 << 6)
#define L2RSTDISABLE_LITTLE_CLR		(1 << 5)
#define L1RSTDISABLE_LITTLE_CLR		(1 << 4)

#define DFD_RST_DISABLE_EN		(L2RSTDISABLE_LITTLE_EN |       \
	                                 L1RSTDISABLE_LITTLE_EN |        \
	                                 L2RSTDISABLE_BIG_EN |           \
	                                 L1RSTDISABLE_BIG_EN)

#define DFD_RST_DISABLE_CLR		(L2RSTDISABLE_LITTLE_CLR |      \
	                                 L1RSTDISABLE_LITTLE_CLR |       \
	                                 L2RSTDISABLE_BIG_CLR |          \
	                                 L1RSTDISABLE_BIG_CLR)


#define CPU_ON_PSCI_ID			(0xC4000003)
#define CPU_OFF_PSCI_ID			(0x84000002)
#define RESET_SEQUENCER_OFFSET		(0x500)
#define COREREG_OFFSET			(0x200)
#define REG_OFFSET			(0x4)
#define L2_STATUS_LOCAL_PWR_CFG_MASK	(0x07)
#define L2_STATUS_POWER_ON		(0x07)

#define BIG_DUMP_PC_ADDRESS		(0x10940000)    /* CPU CLUSTER 1 */
#define LITTLE_DUMP_PC_ADDRESS		(0x10930000)    /* CPU CLUSTER 0 */
#define DUMP_GPR_OFFSET			(0x40)

#define BIG_CORE_START			(4)
#define BIG_CORE_LAST			(7)

#define LITTLE_CORE_START		(0)
#define LITTLE_CORE_LAST		(3)

#define CLEAR				(0 << 0)
#define RESET				(1 << 0)
#define RESERVED			(1 << 1)
#define HOTPLUG				(1 << 2)
#define C2_STATE			(1 << 3)
#define CLUSTER_OFF			(1 << 4)

#define CONFIG_PHY_DRAM_NS_BASE		(0xBFFFE000)
#define CORE_STAT			(CONFIG_PHY_DRAM_NS_BASE + 0x28)
#define CPU0_CORE_STAT			(CORE_STAT + 0x00)
#define CPU1_CORE_STAT			(CORE_STAT + 0x04)
#define CPU2_CORE_STAT			(CORE_STAT + 0x08)
#define CPU3_CORE_STAT			(CORE_STAT + 0x0C)
#define CPU4_CORE_STAT			(CORE_STAT + 0x10)
#define CPU5_CORE_STAT			(CORE_STAT + 0x14)
#define CPU6_CORE_STAT			(CORE_STAT + 0x18)
#define CPU7_CORE_STAT			(CORE_STAT + 0x1C)

#define CPU0_LOGICAL_MAP		0x0
#define CPU1_LOGICAL_MAP		0x1
#define CPU2_LOGICAL_MAP		0x2
#define CPU3_LOGICAL_MAP		0x3
#define CPU4_LOGICAL_MAP		0x100
#define CPU5_LOGICAL_MAP		0x101
#define CPU6_LOGICAL_MAP		0x102
#define CPU7_LOGICAL_MAP		0x103

#define FLUSH_SKIP			0x0
#define FLUSH_LEVEL1			0x1
#define FLUSH_LEVEL2			0x2
#define FLUSH_LEVEL3			0x3

#define CPU0_DEBUG_BASE			0x16410000
#define CPU1_DEBUG_BASE			0x16510000
#define CPU2_DEBUG_BASE			0x16610000
#define CPU3_DEBUG_BASE			0x16710000
#define CPU4_DEBUG_BASE			0x16810000
#define CPU5_DEBUG_BASE			0x16910000
#define CPU6_DEBUG_BASE			0x16A10000
#define CPU7_DEBUG_BASE			0x16B10000

#define PMUDBG_CL0_CPU0_STATUS		(EXYNOS9830_POWER_BASE + 0x0F08)
#define PMUDBG_CL1_CPU0_STATUS		(EXYNOS9830_POWER_BASE + 0x0F20)
#define PMUDBG_CPU_STAT_MASK		(0xFF << 16)

#define DEBUG_EDPRSR			0x314
#define EDPRSR_HALTED			0x10
#define EDPRSR_POWERUP			(1 << 0)
#define EDPRSR_DBG_MODE			(1 << 4)
#define DEBUG_DBGDTRRX_EL0		0x80
#define DEBUG_EDITR			0x84
#define DEBUG_EDSCR			0x88
#define DEBUG_EDRCR			0x90
#define EDRCR_CBRRQ			(1 << 4)
#define EDSCR_ITE			(1 << 24)
#define EDSCR_TXFULL			(1 << 29)
#define EDSCR_EL0_AARCH_MODE		(1 << 10)
#define EL0_AARCH32_MODE		(0)
#define EL0_AARCH64_MODE		(1)
#define EDSCR_EL_MASK			(3 << 8)
#define EDSCR_EL0			(0)

#define DEBUG_DBGDTRTX_EL0		0x8C
#define DEBUG_OSLOCK_OFFSET		0xFB0
#define OSLOCK_MAGIC			0xc5acce55
#define DEBUG_OSLAR_EL1			0x300
#define DEBUG_CTILAR			0x10FB0
#define DEBUG_CTIOUTEN0			0x100A0
#define DEBUG_CTIOUTEN1			0x100A4
#define DEBUG_CTIAPPSET			0x10014
#define DEBUG_CTICONTROL		0x10000
#define DEBUG_CTIINTACK			0x10010
#define DEBUG_CTIAPPSET			0x10014
#define DEBUG_CTIAPPCLEAR		0x10018
#define DEBUG_CTIAPPPULSE		0x1001C

#define BIG_CPU0_CONFIGURATION		0x2200
#define BIG_CPU0_STATUS			0x2204
#define BIG_CPU0_OPTION			0x2208
#define BIG_CPU0_RESET			0x220C
#define BIG_NONCPU_RESET		0x242c
#define BIG_NONCPU_OPTION		0x2428
#define CPU_RESET_DISABLE_SOFTRESET	0x041C
#define CPU_RESET_DISABLE_WDTRESET	0x0414

#define WDT_RESET_LPI			0x0418
#define ENABLE_WDT_RESET_LPI		(1 << 0)
#define TIMEOUT_RESET_LPI		0x0420
#define STATUS_TIMEOUT_RESET_LPI	(1 << 4)
#define ENABLE_TIMEOUT_LPI		(1 << 0)

#define LOCAL_PWR_CFG			(0x0F)
#define OPTION_RESET_VALUE		(0x1001010A)
#define OPTION_USE_WFI			(1 << 16)
#define OPTION_USE_SMPEN		(1 << 28)
#define PEND_BIG			(1 << 0)
#define PEND_LITTLE			(1 << 1)
#define DEFAULT_VAL_CPU_RESET_DISABLE	0xFFFFFFFC
#define EMULATION			(1 << 31)

#define RESET_DISABLE_WDT_CPUPORESET	(1 << 12)
#define RESET_DISABLE_CORERESET		(1 << 9)
#define RESET_DISABLE_CPUPORESET	(1 << 8)

#define RESET_DISABLE_WDT_L2RESET	(1 << 31)
#define RESET_DISABLE_WDT_PRESET_DBG	(1 << 25)
#define RESET_DISABLE_PRESET_DBG	(1 << 18)
#define RESET_DISABLE_L2RESET		(1 << 16)

#define RAMDUMP_BOOT_CNT_MAGIC	0xFACEDB90

#ifndef __ASSEMBLY__
#ifdef CONFIG_RAMDUMP_GPR
void dfd_run_dump_gpr(void);
void dfd_set_dump_gpr(int en);
u64 dfd_entry_dump_gpr(void);
void __dfd_dump_gpr(int cpu, u32 reg, u32 val);
void dfd_display_reboot_reason(void);
void dfd_display_core_stat(void);
u32 dfd_get_pmudbg_stat(u32 cpu);
void debug_snapshot_fdt_init(void);
int debug_snapshot_getvar_item(const char *name, char *response);
void debug_snapshot_boot_cnt(void);

#else
#define dfd_run_dump_gpr()			do { } while (0)
#define dfd_set_dump_gpr(a)			do { } while (0)
#define dfd_entry_dump_gpr()			do { } while (0)
#define __dfd_dump_gpr(a, b, c)			do { } while (0)
#define dfd_display_reboot_reason()		do { } while (0)
#define dfd_display_core_stat()			do { } while (0)
#define dfd_get_pmudbg_stat(a)			do { } while (0)
#define debug_snapshot_fdt_init()		do { } while (0)
#define debug_snapshot_getvar_item(a, b)	do { } while (0)
#define debug_snapshot_boot_cnt()		do { } while (0)
#endif // ifdef CONFIG_RAMDUMP_GPR
#endif //__ASSEMBLY__
#endif // ifndef _EXYNOS9830_DFD_H
