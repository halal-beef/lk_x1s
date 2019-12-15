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
#include <reg.h>
#include <string.h>
#include <arch/ops.h>
#include <lib/font_display.h>
#include <platform/sizes.h>
#include <platform/sfr.h>
#include <platform/smc.h>
#include <platform/delay.h>
#include <platform/dfd.h>
#include <dev/debug/dss.h>

#define TIMEOUT	100000

const u32 dbg_base[NR_CPUS] = {
	0x16410000, 0x16510000, 0x16610000, 0x16710000,
	0x16810000, 0x16910000, 0x16A10000, 0x16B10000
};

static inline void pmu_set_bit_atomic(u32 offset, u32 bit)
{
        writel(bit, EXYNOS_PMU_BASE + (offset | 0xc000));
}

static inline void pmu_clr_bit_atomic(u32 offset, u32 bit)
{
        writel(bit, EXYNOS_PMU_BASE + (offset | 0x8000));
}

static u32 pmu_cpu_offset(u32 cpu)
{
	u32 tmp = 0x0;

	if (cpu >= BIG_CORE_START) {
		cpu -= 4;
		tmp = 0x400;
	}

	return tmp + (0x80 * cpu);
}

static int dfd_wait_complete(u32 cpu)
{
	u32 ret;
	u32 loop = 1000;

	do {
		ret = readl(CONFIG_RAMDUMP_DUMP_GPR_WAIT);
		if (ret & (1 << cpu))
			return 1;

		udelay(100);
	} while (loop--);

	return 0;
}

static int dfd_check_panic_stat(u32 cpu)
{
	u32 val = readl(CONFIG_RAMDUMP_CORE_PANIC_STAT + (cpu * REG_OFFSET));

	if (val == RAMDUMP_SIGN_PANIC)
		return 1;
	else if (val == RAMDUMP_SIGN_RESERVED)
		return 2;
	return 0;
}

static void dfd_get_gpr(u64 cpu)
{
	int i, idx;
	u64 *cpureg = (u64 *)(CONFIG_RAMDUMP_COREREG + (cpu * COREREG_OFFSET));
	u64 *iram = (u64 *)DUMPGPR_BASE;

	for (i = X0, idx = X0; i < GPR_END; ++i) {
		if (i == X19)
			idx = 19;
		cpureg[idx++] = iram[i];
	}
}

static void dfd_get_pc_value(void)
{
	u64 i;

	for (i = 0; i < NR_CPUS; i++) {
		u64 *cpu_reg = (u64 *)(CONFIG_RAMDUMP_COREREG + (i * COREREG_OFFSET));
		u64 iram = DUMPPC_BASE + (i * 0x10);

		cpu_reg[PC] = readq(iram);
		cpu_reg[POWER_STATE] = readl(iram + 0x8);

		if (((cpu_reg[PC] >> 40) & 0xff) == 0xff)
			cpu_reg[PC] |= 0xffffff0000000000;
		printf("Core%llu: reg : 0x%llX\n", i, cpu_reg[PC]);
	}
}

/*
 * FLUSH_SKIP : skip cache flush
 * FLUSH_LEVEL 1 : L1, L2 cache flush (local cache flush)
 * FLUSH_LEVEL 2 : L3 cache flush (cluster cache flush)
 * FLUSH_LEVEL 3 : L1, L2, L3 cache flush (all cache flush)
 */
static void dfd_set_cache_flush_level(void)
{
	int cl0_on = -1, cl1_on = -1;
	u64 cpu, val, stat;
	u64 *cpu_reg;

	for (cpu = 0; cpu < NR_CPUS; cpu++) {
		u32 offset = cpu * REG_OFFSET;
		cpu_reg = (u64 *)(CONFIG_RAMDUMP_COREREG + (cpu * COREREG_OFFSET));
		if (!cpu_reg[POWER_STATE]) {
			stat = FLUSH_SKIP;
		} else {
			stat = FLUSH_LEVEL1;
			if (cpu <= LITTLE_CORE_LAST)
				cl0_on = cpu;
			else
				cl1_on = cpu;
		}
		writel(stat, CONFIG_RAMDUMP_GPR_POWER_STAT + offset);
	}

	/* conclude core which runs cache flush level 2 in little cluster */
	if (cl0_on <= 0) {
		/* core0 runs cache flush level 2 */
		val = FLUSH_LEVEL2;
		cl0_on = 0;
	} else {
		val = FLUSH_LEVEL3;
	}

	stat = val;
	writel(stat, CONFIG_RAMDUMP_GPR_POWER_STAT + (cl0_on * REG_OFFSET));

	if (cl1_on >= 0) {
		stat = FLUSH_LEVEL3;
		writel(stat, CONFIG_RAMDUMP_GPR_POWER_STAT + (cl1_on * REG_OFFSET));
	}
}

static void dfd_flush_dcache_level(u32 level, u32 invalidate)
{
	__arch_clean_and_invalidate_dcache_level(level, invalidate);
}

static void dfd_flush_dcache_all(void)
{
	arch_clean_and_invalidate_dcache_all();
}

void dfd_secondary_cpu_cache_flush(u32 cpu)
{
	u32 val;

	dfd_get_gpr(cpu);
	do {
		val = readl(CONFIG_RAMDUMP_WAKEUP_WAIT);
		if (val & (1 << cpu))
			break;
	} while (1);

	/* Get Cache Flush Level */
	val = readl(CONFIG_RAMDUMP_GPR_POWER_STAT + (cpu * REG_OFFSET));
	switch (val) {
	case 0:
		break;
	case 1:
		/* FLUSH_LEVEL1 */
		dfd_flush_dcache_level(val - 1, 0);
		break;
	case 2:
		/* FLUSH_LEVEL2 */
		dfd_flush_dcache_level(val - 1, 0);
		break;
	case 3:
		/* FLUSH_LEVEL3 (all cache flush) */
		dfd_flush_dcache_all();
		break;
	default:
		break;
	}
	/* Write own bit to inform finishing dumpGPR */
	val = readl(CONFIG_RAMDUMP_DUMP_GPR_WAIT);
	writel((val | (1 << cpu)), CONFIG_RAMDUMP_DUMP_GPR_WAIT);
off:
	if (cpu != 0) {
		//cpu_boot(CPU_OFF_PSCI_ID, 0, 0);
		do {
		//	wfi();
		} while (1);
	}
}

static void dfd_clear_reset_disable(void)
{
	for (int cpu = 0; cpu < NR_CPUS; cpu++)
		pmu_clr_bit_atomic(PMU_CPU_OUT_OFFSET + pmu_cpu_offset(cpu), CPU_CLR_DBGL1RSTDIS);

	pmu_clr_bit_atomic(PMU_CL0_NCPU_OUT_OFFSET, NCPU_CLR_DBGL3RSTDIS);
	pmu_clr_bit_atomic(PMU_CL1_NCPU_OUT_OFFSET, NCPU_CLR_DBGL3RSTDIS);
}

static bool dfd_is_cpu_power_up(u32 cpu)
{
	u32 pmu_status = readl(EXYNOS_PMU_BASE + PMU_CPU_STATUS + pmu_cpu_offset(cpu));

	return ((pmu_status & PMU_CPU_STAT_MASK) == PMU_CPU_ON);
}

static bool dfd_is_cpu_halted(u32 cpu)
{
	u32 dbg_prsr = readl(dbg_base[cpu] + DBGPRSR);

	return ((dbg_prsr & (DBGPRSR_HALTED | DBGPRSR_R | DBGPRSR_PU))
				== (DBGPRSR_HALTED | DBGPRSR_PU));
}

static void dfd_os_unlock(u32 cpu)
{
	writel(MAGIC, dbg_base[cpu] + DBGLAR);
	writel(OS_UNLOCK, dbg_base[cpu] + DBGOSLAR);
}

static void dfd_enter_debug_state(u32 cpu)
{
	writel(MAGIC, dbg_base[cpu] + CTI_OFFSET + DBGLAR);
	writel(0x0, dbg_base[cpu] + CTI_OFFSET + CTIGATE);
	writel(CTICH0, dbg_base[cpu] + CTI_OFFSET + CTIOUTEN(0));
	writel(CTICH1, dbg_base[cpu] + CTI_OFFSET + CTIOUTEN(1));
	writel(CTIAPPSETCH0, dbg_base[cpu] + CTI_OFFSET + CTIAPPSET);
	writel(CTICONTROLEN, dbg_base[cpu] + CTI_OFFSET + CTICONTROL);
}

static void dfd_exit_debug_state(u32 cpu)
{
	writel(CTIAPPSETCH0, dbg_base[cpu] + CTI_OFFSET + CTIAPPCLEAR);
	writel(CTICH0, dbg_base[cpu] + CTI_OFFSET + CTIINTACK);
	writel(CTICH1, dbg_base[cpu] + CTI_OFFSET + CTIAPPPULSE);
	writel(!CTICONTROLEN, dbg_base[cpu] + CTI_OFFSET + CTICONTROL);
}

static u32 dfd_check_edscr(u32 cpu)
{
	union EDSCR_REG edscr_reg;

	do {
		edscr_reg.word = readl(dbg_base[cpu] + EDSCR_OFFSET);
	} while (edscr_reg.bits.ITE == 0);

	return edscr_reg.word;
}

static void dfd_write_instruction(u32 instr, u32 cpu)
{
	union EDSCR_REG edscr_reg;
	edscr_reg.word = dfd_check_edscr(cpu);
	if (edscr_reg.bits.ERR | edscr_reg.bits.TXU | edscr_reg.bits.RXO)
		writel(1 << 2, dbg_base[cpu] + DBGRCR);

	writel(instr, dbg_base[cpu] + ITR_OFFSET);
}

static u64 dfd_read_gpr_aarch64(u32 gpr_number, u32 cpu)
{
	u32 instr;
	u32 low;
	u64 high;

	instr = MSR_DTR_XX + gpr_number;
	dfd_write_instruction(instr, cpu);
	dfd_check_edscr(cpu);
	high = readl(dbg_base[cpu] + DBGDTRRX_OFFSET);
	low = readl(dbg_base[cpu] + DBGDTRTX_OFFSET);

	return (high << 32) | low;
}

static void dfd_write_gpr_aarch64(u64 value, u32 gpr_number, u32 cpu)
{
	u32 instr;
	u32 low = value;
	u32 high = value >> 32;

	writel(high, dbg_base[cpu] + DBGDTRTX_OFFSET);
	writel(low, dbg_base[cpu] + DBGDTRRX_OFFSET);
	instr = MRS_XX_DTR + gpr_number;
	dfd_write_instruction(instr, cpu);
	dfd_check_edscr(cpu);
}

static u64 dfd_read_spr(u32 addr, u32 cpu)
{
	u32 instr_mrs;
	union SPR_ADDR_REG spr_addr;
	u64 temp_reg, result_reg;

	temp_reg = dfd_read_gpr_aarch64(0, cpu);
	spr_addr.word = addr;
	instr_mrs = (0xd52 << 20) | ((spr_addr.bits.OP0) << 19) |
		(spr_addr.bits.OP1 << 16) | (spr_addr.bits.CRn << 12) |
		(spr_addr.bits.CRm << 8) | (spr_addr.bits.OP2 << 5);
	dfd_write_instruction(instr_mrs, cpu);
	dfd_check_edscr(cpu);
	result_reg = dfd_read_gpr_aarch64(0, cpu);
	dfd_write_gpr_aarch64(temp_reg, 0, cpu);

	return result_reg;
}

static void dfd_write_spr(u64 value, u32 addr, u32 cpu)
{
	u32 instr_msr;
	union SPR_ADDR_REG spr_addr;
	u64 temp_reg;

	temp_reg = dfd_read_gpr_aarch64(0, cpu);
	spr_addr.word = addr;
	dfd_write_gpr_aarch64(value, 0, cpu);
	instr_msr = (0xd50 << 20) | ((spr_addr.bits.OP0) << 19) |
		(spr_addr.bits.OP1 << 16) | (spr_addr.bits.CRn << 12) |
		(spr_addr.bits.CRm << 8) | (spr_addr.bits.OP2 << 5);
	dfd_write_instruction(instr_msr, cpu);
	dfd_check_edscr(cpu);
	dfd_write_gpr_aarch64(temp_reg, 0, cpu);
}

static void dfd_arrdump_ananke_dcache(u32 cpu, u32 **parray_offset)
{
	u32 set, way, offset;
	u64 tag0, tag1;
	u64 data0, data1;
	union CDBGDCT_REG cdbgdct;

	for (set = 0; set < AN_DC_SET_END; set++) {
		for (way = 0; way < AN_DC_WAY_END; way++) {
			cdbgdct.word = 0;
			cdbgdct.bits.SET = set;
			cdbgdct.bits.WAY = way;

			dfd_write_spr(cdbgdct.word, CDBGDCT_EL3, cpu);
			tag0 = dfd_read_spr(CDBGDR0_EL3, cpu);
			tag1 = dfd_read_spr(CDBGDR1_EL3, cpu);

			writel((u32)tag0, (*parray_offset)++);
			writel((u32)tag1, (*parray_offset)++);

			for (offset = 0; offset < AN_DC_OFFSET_END; offset++) {
				cdbgdct.bits.OFFSET = offset;
				dfd_write_spr(cdbgdct.word, CDBGDCD_EL3, cpu);
				data0 = dfd_read_spr(CDBGDR0_EL3, cpu);
				data1 = dfd_read_spr(CDBGDR1_EL3, cpu);

				writel((u32)data0, (*parray_offset)++);
				writel((u32)data1, (*parray_offset)++);
			}
		}
	}
}

static void dfd_arrdump_ananke_icache(u32 cpu, u32 **parray_offset)
{
	u32 set, way, offset;
	u64 tag0;
	u64 data0, data1;
	union CDBGICT_REG cdbgict;

	for (set = 0; set < AN_IC_SET_END; set++) {
		for (way = 0; way < AN_IC_WAY_END; way++) {
			cdbgict.word = 0;
			cdbgict.bits.SET = set;
			cdbgict.bits.WAY = way;
			dfd_write_spr(cdbgict.word, CDBGICT_EL3, cpu);

			tag0 = dfd_read_spr(CDBGDR0_EL3, cpu);
			writel((u32)tag0, (*parray_offset)++);

			for (offset = 0; offset < AN_IC_OFFSET_END; offset++) {
				cdbgict.bits.OFFSET = offset;
				dfd_write_spr(cdbgict.word, CDBGICD_EL3, cpu);
				data0 = dfd_read_spr(CDBGDR0_EL3, cpu);
				data1 = dfd_read_spr(CDBGDR1_EL3, cpu);

				writel((u32)data0, (*parray_offset)++);
				writel((u32)data1, (*parray_offset)++);
			}
		}
	}
}

static void dfd_arrdump_ananke_l2tlb(u32 cpu, u32 **parray_offset)
{
	u32 set, way;
	u64 tag0, tag1, tag2;
	u64 data0, data1;
	union CDBGTT_REG cdbgtt;

	for (set = 0; set < AN_L2TLB_SET_END; set++) {
		for (way = 0; way < AN_L2TLB_WAY_END; way++) {
			cdbgtt.word = 0;
			cdbgtt.bits.INDEX = set;
			cdbgtt.bits.WAY = way;

			dfd_write_spr(cdbgtt.word, CDBGTT_EL3, cpu);
			tag0 = dfd_read_spr(CDBGDR0_EL3, cpu);
			tag1 = dfd_read_spr(CDBGDR1_EL3, cpu);
			tag2 = dfd_read_spr(CDBGDR2_EL3, cpu);

			writel((u32)tag0, (*parray_offset)++);
			writel((u32)tag1, (*parray_offset)++);
			writel((u32)tag2, (*parray_offset)++);

			dfd_write_spr(cdbgtt.word, CDBGTD_EL3, cpu);
			data0 = dfd_read_spr(CDBGDR0_EL3, cpu);
			data1 = dfd_read_spr(CDBGDR1_EL3, cpu);

			writel((u32)data0, (*parray_offset)++);
			writel((u32)data1, (*parray_offset)++);
		}
	}
}

static int dfd_ananke_arrays(int cpu)
{
	u32 *array_offset, ret = 0;
	union EDSCR_REG	edscr;
	u64 dlr_el0 = 0;
	u64 dspsr_el0 = 0;

	dfd_enter_debug_state(cpu);
	mdelay(100);
	if (dfd_is_cpu_power_up(cpu) && dfd_is_cpu_halted(cpu)) {
		dfd_os_unlock(cpu);

		edscr.word = dfd_check_edscr(cpu);
		if (edscr.bits.EL != 0x3) {
			dlr_el0 = dfd_read_spr(DLR_EL0, cpu);
			dspsr_el0 = dfd_read_spr(DSPSR_EL0, cpu);
			dfd_write_instruction(DCPS3, cpu);
		}
		array_offset=(u32*)(AN_DUMP_ADDR + (AN_CORE_DUMP_SIZE * cpu));
		dfd_arrdump_ananke_dcache(cpu, &array_offset);
		dfd_arrdump_ananke_icache(cpu, &array_offset);
		dfd_arrdump_ananke_l2tlb(cpu, &array_offset);

		if (edscr.bits.EL != 0x3) {
			if (edscr.bits.EL == 0x2)
				dfd_write_instruction(DCPS2, cpu);
			else
				dfd_write_instruction(DCPS1, cpu);

			dfd_write_spr(dlr_el0, DLR_EL0, cpu);
			dfd_write_spr(dspsr_el0, DSPSR_EL0, cpu);
		}

		ret = 0;
	} else {
		ret = 1;
	}
	dfd_exit_debug_state(cpu);
	mdelay(100);

	return ret;
}

void dfd_soc_run_post_processing(void)
{
	u32 cpu_logical_map[NR_CPUS] = {
		CPU0_LOGICAL_MAP,
		CPU1_LOGICAL_MAP,
		CPU2_LOGICAL_MAP,
		CPU3_LOGICAL_MAP,
		CPU4_LOGICAL_MAP,
		CPU5_LOGICAL_MAP,
		CPU6_LOGICAL_MAP,
		CPU7_LOGICAL_MAP
	};
	u32 cpu, val, cpu_mask = 0;
	int ret;

	if (!(readl(EXYNOS3830_POWER_BASE + RESET_SEQUENCER_OFFSET) & DUMP_EN)) {
		printf("DUMP_EN disabled. Skip debug info.\n");
		return;
	}

	dfd_set_dump_en(0);
	dfd_clear_reset_disable();

	printf("---------------------------------------------------------\n");
	printf("Watchdog or Warm Reset Detected.\n");

	printf("Display PC value\n");
	dfd_get_pc_value();
	dfd_get_gpr(0);

	//llc_flush_disable();

	dfd_set_cache_flush_level();
	//Wake up secondary CPUs.
	for (cpu = 0; cpu < NR_CPUS; cpu++) {
		val = readl(CONFIG_RAMDUMP_GPR_POWER_STAT + (cpu * REG_OFFSET));
		if (val != FLUSH_SKIP)
			cpu_mask |= (1 << cpu);

		if (cpu == 0)
			continue;

		ret = cpu_boot(CPU_ON_PSCI_ID, cpu_logical_map[cpu], (u64)dfd_entry_point);
		if (ret) {
			printf("Core%d: ERR failed power on: 0x%x\n", cpu, ret);
			ret = readl(CONFIG_RAMDUMP_DUMP_GPR_WAIT);
			writel((ret | 1 << cpu), CONFIG_RAMDUMP_DUMP_GPR_WAIT);
			continue;
		}
		mdelay(100);
	}

	for (cpu = 0; cpu < NR_CPUS; cpu++) {
		if (cpu && cpu_mask & (1 << cpu))
			dfd_ananke_arrays(cpu);

		val = readl(CONFIG_RAMDUMP_WAKEUP_WAIT);
		writel(val | (1 << cpu), CONFIG_RAMDUMP_WAKEUP_WAIT);
		if (cpu == 0)
			dfd_secondary_cpu_cache_flush(cpu);

		if (!dfd_wait_complete(cpu)) {
			printf("Core%d: ERR wait timeout.\n", cpu);
			continue;
		}

		printf("Core%d: finished Cache Flush level:%d (0x%x)\n", cpu,
		(readl(CONFIG_RAMDUMP_GPR_POWER_STAT + (cpu * REG_OFFSET))),
		readl(CONFIG_RAMDUMP_DUMP_GPR_WAIT));
	}
	printf("---------------------------------------------------------\n");
}

unsigned int clear_llc_init_state(void)
{
	pmu_clr_bit_atomic(RESET_SEQUENCER_OFFSET, LLC_INIT_BIT);

	return readl(EXYNOS3830_POWER_BASE + RESET_SEQUENCER_OFFSET);
}
