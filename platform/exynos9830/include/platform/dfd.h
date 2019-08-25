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
#ifndef _EXYNOS_DFD_H_
#define _EXYNOS_DFD_H_

#define SCAN2DRAM_SOLUTION

#define NR_CPUS				(8)
#define BIG_NR_CPUS			(2)
#define LITTLE_NR_CPUS			(4)

#define LITTLE_CORE_START		(0)
#define LITTLE_CORE_LAST		(3)
#define MID_CORE_START			(4)
#define MID_CORE_LAST			(5)
#define BIG_CORE_START			(6)
#define BIG_CORE_LAST			(7)

#define RESET_SEQUENCER_OFFSET		(0x500)

#define DUMP_EN_BIT			(0)
#define DUMP_EN				(1 << DUMP_EN_BIT)

#define LLC_INIT_BIT			(24)
#define LLC_INIT			(1 << LLC_INIT_BIT)

#define CPU_ON_PSCI_ID			(0xC4000003)
#define CPU_OFF_PSCI_ID			(0x84000002)
#define COREREG_OFFSET			(0x200)
#define REG_OFFSET			(0x4)

#define CPU0_LOGICAL_MAP		0x0000
#define CPU1_LOGICAL_MAP		0x0001
#define CPU2_LOGICAL_MAP		0x0002
#define CPU3_LOGICAL_MAP		0x0003
#define CPU4_LOGICAL_MAP		0x0004
#define CPU5_LOGICAL_MAP		0x0005
#define CPU6_LOGICAL_MAP		0x0100
#define CPU7_LOGICAL_MAP		0x0101

#define FLUSH_SKIP			0x0
#define FLUSH_LEVEL1			0x1
#define FLUSH_LEVEL2			0x2
#define FLUSH_LEVEL3			0x3

#define DBGC_VERSION_LEN		(48)
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

#define INTGR_AP_TO_DBGC	(0x1c)

#ifndef __ASSEMBLY__
enum pt_reg {
	SP = 31,
	PC,
	PSTATE,
	PCSR,
	POWER_STATE,
	NS,
};

struct dfd_ipc_cmd_raw {
	u32 cmd                 :16;
	u32 response            :1;
	u32 overlay             :1;
	u32 ret                 :1;
	u32 ok                  :1;
	u32 busy                :1;
	u32 manual_polling      :1;
	u32 one_way             :1;
	u32 reserved            :1;
	u32 id                  :4;
	u32 size                :4;
};

struct dfd_ipc_cmd {
	union {
		struct dfd_ipc_cmd_raw cmd_raw;
		unsigned int buffer[4];
	};
};

enum pp_ipc_cmd_id {
	PP_IPC_CMD_ID_ENTER_HALT,
	PP_IPC_CMD_ID_RUN_DUMP,
	PP_IPC_CMD_ID_EXIT_HALT,
	PP_IPC_CMD_ID_RUN_GPR,
	PP_IPC_CMD_ID_RUN_ARR_TAG,
	PP_IPC_CMD_ID_START,
	PP_IPC_CMD_ID_FINISH,

};

enum frmk_ipc_cmd {
	IPC_CMD_COPY_DEBUG_LOG = 0xcb10,
	IPC_CMD_DEBUG_LOG_INFO,
	IPC_CMD_POST_PROCESSING = 0x8080,
};

void llc_flush(u32 invway);
unsigned int clear_llc_init_state(void);
void dfd_soc_run_post_processing(void);
void dfd_soc_get_dbgc_version(void);

#ifdef CONFIG_RAMDUMP_GPR
void dfd_entry_point(void);
u64 cpu_boot(u64 id, u64 cpu, u64 fn);
void write_back_cache(int cpu);
void llc_flush_disable(void);
#else
inline static void write_back_cache(int cpu)
{
}

inline static void llc_flush_disable(void)
{
}
#endif
#endif /*__ASSEMBLY__ */
#endif /* _EXYNOS9830_DFD_H */
