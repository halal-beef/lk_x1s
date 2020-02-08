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


#define PMU_CPU_STATUS			(0x1004)
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
#define CLUSTER0_STATUS			(0x1 << 1)
#define CLUSTER1_STATUS			(0x1 << 2)

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

#define CONFIG_PHY_DRAM_NS_BASE         (0xBFFFE000)
#define CORE_STAT                       (CONFIG_PHY_DRAM_NS_BASE + 0x2C)

#define RUNNING                         (0 << 0)
#define RESET                           (1 << 0)
#define RESERVED                        (1 << 1)
#define HOTPLUG                         (1 << 2)
#define C2_STATE                        (1 << 3)
#define CLUSTER_OFF                     (1 << 4)

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

#define DBGC_VERSION_LEN	(48)

#define PMU_CPU_STAT_MASK              	0x1
#define PMU_CPU_ON                     	0x1

#define DBGPRSR				(0x314)
#define DBGOSLAR			(0x300)
#define DBGLAR				(0xfb0)
#define DBGDSCR				(0x088)
#define DBGITR				(0x084)
#define DBGDTRRX			(0x080)
#define DBGDTRTX			(0x08c)
#define DBGRCR				(0x090)

/* CTI registers */
#define CTI_OFFSET			(0x10000)
#define CTIOUTEN(n)			(0xA0 + (n * 0x4))
#define CTIINEN(n)			(0x20 + (n * 0x4))
#define CTICONTROL			(0x000)
#define CTIINTACK			(0x010)
#define CTIAPPSET			(0x014)
#define CTIAPPCLEAR			(0x018)
#define CTIAPPPULSE			(0x01c)
#define CTIGATE 			(0x140)

#define CTICH0				(0x1)
#define CTICH1				(0x2)
#define CTICH2				(0x4)
#define CTICH3				(0x8)

#define CTIAPPSETCH0			(0x1)
#define CTICONTROLEN			(0x1)

#define PMLAR				(0xfb0)
#define PMPCSR				(0x200)

#define DBGPRSR_HALTED			(1 << 4)
#define DBGPRSR_R			(1 << 2)
#define DBGPRSR_PU			(1 << 0)
#define DBGDSCR_TXFULL			(1 << 29)
#define DBGDSCR_RXFULL			(1 << 30)
#define DBGDSCR_ITE			(1 << 24)
#define DBGRCR_CSE			(1 << 2)
#define DBGRCR_CSPA			(1 << 3)

#define CORE_POWER_UP			(0x1)
#define OS_UNLOCK			(0x0)
#define MAGIC				(0xC5ACCE55)

#define DBGDSCR_EL0_AARCH_MODE         (1 << 10)
#define EL0_AARCH32_MODE               (0)
#define EL0_AARCH64_MODE               (1)

#define DCC_TIMEOUT			1000

#define MSR_DTR_XX			0xd5130400
#define MRS_XX_DTR			0xd5330400
#define ITR_OFFSET			0x084
#define EDSCR_OFFSET			0x088
#define DBGDTRRX_OFFSET			0x080
#define DBGDTRTX_OFFSET			0x08c

#define DLR_EL0                         0x33451
#define DSPSR_EL0                       0x33450
#define CDBGDR0_EL3			0x36f00
#define CDBGDR1_EL3			0x36f01
#define CDBGDR2_EL3			0x36f02
#define CDBGDCT_EL3			0x16f20
#define CDBGICT_EL3			0x16f21
#define CDBGTT_EL3			0x16f22
#define CDBGDCD_EL3			0x16f40
#define CDBGICD_EL3			0x16f41
#define CDBGTD_EL3			0x16f42
#define AN_DC_SET_END			0x80
#define AN_DC_WAY_END			0x4
#define AN_DC_OFFSET_END		0x8

#define AN_IC_SET_END			0x80
#define AN_IC_WAY_END			0x4
#define AN_IC_OFFSET_END		0x10

#define AN_L2TLB_SET_END		0x100
#define AN_L2TLB_WAY_END		0x4

#define IDATA0_EL3			0x36f00
#define IDATA1_EL3      	        0x36f01
#define IDATA2_EL3              	0x36f02
#define DDATA0_EL3              	0x36f10
#define DDATA1_EL3              	0x36f11
#define DDATA2_EL3              	0x36f12
#define RAMINDEX_ENYO           	0x16f00

#define DCPS1				0xd4a00001
#define DCPS2				0xd4a00002
#define DCPS3				0xd4a00003

#define AN_DUMP_ADDR			(dss_get_item_paddr("log_arrdumpreset"))
#define AN_CORE_DUMP_SIZE		(128 * SZ_1K)

#ifndef __ASSEMBLY__
struct EDSCR_BIT_INFO {
	unsigned int STATUS:6;		//bit 5:0
	unsigned int ERR:1;		//bit 6:6
	unsigned int A:1;		//bit 7:7
	unsigned int EL:2;		//bit 9:8
	unsigned int RW:4;		//bit 13:10
	unsigned int HDE:1;		//bit 14:14
	unsigned int DUMMY0:1;		//bit 15:15
	unsigned int SDD:1;		//bit 16:16
	unsigned int DUMMY1:1;		//bit 17:17
	unsigned int NS:1;		//bit 18:18
	unsigned int DUMMY2:1;		//bit 19:19
	unsigned int MA:1;		//bit 20:20
	unsigned int TDA:1;		//bit 21:21
	unsigned int INTdis:2;		//bit 23:22
	unsigned int ITE:1;		//bit 24:24
	unsigned int PipeAdv:1;		//bit 25:25
	unsigned int TXU:1;		//bit 26:26
	unsigned int RXO:1;		//bit 27:27
	unsigned int ITO:1;		//bit 28:28
	unsigned int TXfull:1;		//bit 29:29
	unsigned int RXfull:1;		//bit 30:30
	unsigned int DUMMY3:1;		//bit 31:31
};

struct SPR_ADDR_BIT_INFO {
	unsigned int OP2:3;		//bit 2:0
	unsigned int DUMMY0:1;
	unsigned int CRm:4;		//bit 7:4
	unsigned int CRn:4;		//bit 11:8
	unsigned int OP1:3;		//bit 14:12
	unsigned int DUMMY1:1;
	unsigned int OP0:4;		//bit 19:16
};

union SPR_ADDR_REG {
	u32 word;
	struct SPR_ADDR_BIT_INFO bits;
};

union EDSCR_REG {
	u32 word;
	struct EDSCR_BIT_INFO bits;
};

struct CDBGDCT_BIT_INFO{
	unsigned int DUMMY:3;		//bit 2:0
	unsigned int OFFSET:3;		//bit 5:3
	unsigned int SET:24;		//bit 29:6
	unsigned int WAY:2;		//bit 31:30
};

union CDBGDCT_REG {
	u32 word;
	struct CDBGDCT_BIT_INFO bits;
};

struct CDBGICT_BIT_INFO{
	unsigned int DUMMY:2;		//bit 1:0
	unsigned int OFFSET:4;		//bit 5:2
	unsigned int SET:7;		    //bit 12:6
	unsigned int DUMMY1:17;		//bit 29:13
	unsigned int WAY:2;		//bit 31:30
};

union CDBGICT_REG {
	u32 word;
	struct CDBGICT_BIT_INFO bits;
};

struct CDBGTT_BIT_INFO{
	unsigned int INDEX:9;		//bit 8:0
	unsigned int DUMMY:21;		//bit 29:9
	unsigned int WAY:2;		//bit 31:30
};

union CDBGTT_REG {
	u32 word;
	struct CDBGTT_BIT_INFO bits;
};

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

void llc_flush(u32 invway);
unsigned int clear_llc_init_state(void);
void dbg_snapshot_early_init(void);
#ifdef CONFIG_RAMDUMP_GPR
void dfd_entry_point(void);
u64 cpu_boot(u64 id, u64 cpu, u64 fn);
void dfd_soc_run_post_processing(void);
void dfd_soc_get_dbgc_version(void);
void llc_flush_disable(void);
void dfd_set_sjtag_status(void);
#else
inline static void dfd_run_post_processing(void)
{
}

inline static void llc_flush_disable(void)
{
}
inline static void dfd_set_sjtag_status(void)
{
}
#endif
#endif /*__ASSEMBLY__ */
#endif /* __DFD_H__ */
