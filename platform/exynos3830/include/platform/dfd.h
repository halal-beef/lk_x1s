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

#ifndef __DFD_H__
#define __DFD_H__

#define NR_CPUS				(8)
#define BIG_NR_CPUS			(4)
#define LITTLE_NR_CPUS			(4)

#define LITTLE_CORE_START		(0)
#define LITTLE_CORE_LAST		(3)
#define BIG_CORE_START			(4)
#define BIG_CORE_LAST			(7)

#define PMU_CPU_OUT_OFFSET		(0x1020)
#define PMU_CL0_NCPU_OUT_OFFSET		(0x1220)
#define PMU_CL1_NCPU_OUT_OFFSET		(0x1620)
/* PMU_CPU_OPTION bit field */
#define CPU_EN_DBGL1RSTDIS              (4)

/* PMU_CL_NONCPU_OPTION bit field */
#define NCPU_EN_DBGL3RSTDIS             (4)

/* PMU_CPU_OUT bit field */
#define CPU_CLR_DBGL1RSTDIS             (5)

/* PMU_CL_NONCPU_OUT bit field */
#define NCPU_CLR_DBGL3RSTDIS            (14)

#define RESET_SEQUENCER_OFFSET		(0x500)

#define DUMP_EN_BIT			(0)
#define DUMP_EN				(1 << DUMP_EN_BIT)

#define LLC_INIT_BIT			(24)
#define LLC_INIT			(1 << LLC_INIT_BIT)

#define DUMPGPR_BASE			(0x9000C800)
#define DUMPPC_BASE			(0x90010a00)
#define CPU_ON_PSCI_ID			(0xC4000003)
#define CPU_OFF_PSCI_ID			(0x84000002)
#define COREREG_OFFSET			(0x200)
#define REG_OFFSET			(0x4)

#define CPU0_LOGICAL_MAP		0x0000
#define CPU1_LOGICAL_MAP		0x0001
#define CPU2_LOGICAL_MAP		0x0002
#define CPU3_LOGICAL_MAP		0x0003
#define CPU4_LOGICAL_MAP		0x0100
#define CPU5_LOGICAL_MAP		0x0101
#define CPU6_LOGICAL_MAP		0x0102
#define CPU7_LOGICAL_MAP		0x0103

#define FLUSH_SKIP			0x0
#define FLUSH_LEVEL1			0x1
#define FLUSH_LEVEL2			0x2
#define FLUSH_LEVEL3			0x3

/*******************************************************************************
 * MPIDR macros
 ******************************************************************************/
#define MPIDR_CPU_MASK		MPIDR_AFFLVL_MASK
#define MPIDR_CLUSTER_MASK	(MPIDR_AFFLVL_MASK << MPIDR_AFFINITY_BITS)
#define MPIDR_AFFINITY_BITS	8
#define MPIDR_AFFLVL_MASK	0xff
#define MPIDR_AFF0_SHIFT	0
#define MPIDR_AFF1_SHIFT	8
#define MPIDR_AFF2_SHIFT	16
#define MPIDR_AFF3_SHIFT	32
#define MT_BIT_OFFSET		(24)

#define RAMDUMP_BOOT_CNT_MAGIC	0xFACEDB90

// #define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#ifndef __ASSEMBLY__
enum pt_gpr {
	X0,
	X1,
	X2,
	X3,
	X4,
	X5,
	X6,
	X7,
	X8,
	X19,
	X20,
	X21,
	X22,
	X23,
	X24,
	X25,
	X26,
	X27,
	X28,
	X29,
	X30,
	SP_EL1,
	GPR_END,
};

enum pt_reg {
	SP = 31,
	PC,
	PSTATE,
	POWER_STATE,
};

enum {
	DEBUG_LEVEL_NONE = -1,
	DEBUG_LEVEL_LOW = 0,
	DEBUG_LEVEL_MID = 1,
};
#define DEBUG_LEVEL_PREFIX	(0xDB9 << 16)
extern int debug_level;

void dfd_verify_enable(void);
extern void set_debug_level(const char *buf);
extern void set_debug_level_by_env(void);
extern void dbg_snapshot_fdt_init(void);
extern int debug_snapshot_getvar_item(const char *name, char *response);
extern unsigned long dbg_snapshot_get_item_paddr(const char *name);
extern unsigned long dbg_snapshot_get_item_size(const char *name);
void dbg_snapshot_boot_cnt(void);
void llc_flush(u32 invway);
unsigned int clear_llc_init_state(void);

#ifdef CONFIG_RAMDUMP_GPR
void dfd_entry_point(void);
void dfd_display_reboot_reason(void);
void dfd_display_rst_stat(void);
void dfd_display_core_stat(void);
u64 cpu_boot(u64 id, u64 cpu, u64 fn);
void dfd_run_post_processing(void);
void write_back_cache(int cpu);
void dfd_set_dump_en_for_cacheop(int en);
void llc_flush_disable(void);
#else
inline static void dfd_display_reboot_reason(void)
{
}

inline static void dfd_display_rst_stat(void)
{
}

inline static void dfd_display_core_stat(void)
{
}

inline static void dfd_run_post_processing(void)
{
}

inline static void dfd_set_dump_en_for_cacheop(int en)
{
}

inline static void llc_flush_disable(void)
{
}
#endif
#endif /*__ASSEMBLY__ */
#endif /* __DFD_H__ */
