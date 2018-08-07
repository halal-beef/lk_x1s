#include <reg.h>
#include <pit.h>
#include <arch/ops.h>
#include <lib/font_display.h>
#include <platform/sfr.h>
#include <platform/smc.h>
#include <platform/delay.h>
#include <platform/dfd.h>
#include <lib/font_display.h>

#define DEBUG_PRINT
extern u64 cpu_boot(u64 id, u64 cpu, u64 fn);

enum pt_reg {
	SP = 31,
	PC,
	PSTATE,
	ESR_EL1,
};

void wfi(void)
{
	asm volatile("wfi");
}

static void dfd_print_pcval(int nCpuId)
{
	u32 pc_reg;
	u64 tgpr;
	u64 *pcsr_src = (u64 *)0x0203F860;

	if (nCpuId >= LITTLE_CORE_START && nCpuId <= LITTLE_CORE_LAST)
		pc_reg = (LITTLE_DUMP_PC_ADDRESS + (nCpuId * 8));
	else
		pc_reg = (BIG_DUMP_PC_ADDRESS + ((nCpuId - BIG_CORE_START) * 8));

	tgpr = readl(pc_reg + 0x04);
	/* padding 0xff in BIG pc_val */
	if ((tgpr & 0xFFFFFF00) == 0x0001FF00)
		tgpr = tgpr | 0xFFFFFF00;
	tgpr = tgpr << 32;
	tgpr = tgpr | readl(pc_reg);

	printf("core %d: pcreg:0x%x = 0x%llx\n", nCpuId, pc_reg, tgpr);
	printf("core %d: pcsr: 0x%llx = 0x%llx\n", nCpuId, (u64)(&pcsr_src[nCpuId]), pcsr_src[nCpuId]);
}

static void dfd_display_panic_reason(void)
{
	char *str = (char *)CONFIG_RAMDUMP_PANIC_REASON;
	int is_string = 0;
	int cnt = 0;

	for (cnt = 0; cnt < CONFIG_RAMDUMP_PANIC_LOGSZ; cnt++,str++)
		if (0x0 == *str)
			is_string = 1;

	if (!is_string) {
		str = (char *)CONFIG_RAMDUMP_PANIC_REASON;
		str[CONFIG_RAMDUMP_PANIC_LOGSZ - 1] = 0x0;
	}

	printf("%s\n", (char *)CONFIG_RAMDUMP_PANIC_REASON);
	print_lcd_update(FONT_YELLOW, FONT_RED, "%s", CONFIG_RAMDUMP_PANIC_REASON);
}

void dfd_display_reboot_reason(void)
{
	u32 ret;

	ret = readl(CONFIG_RAMDUMP_REASON);
	printf("reboot reason: ");
	print_lcd_update(FONT_WHITE, FONT_BLACK, "reboot reason: ");

	switch (ret) {
	case RAMDUMP_SIGN_PANIC:
		printf("0x%x - Kernel PANIC\n", ret);
		print_lcd_update(FONT_YELLOW, FONT_RED, "0x%x - Kernel PANIC", ret);
		dfd_display_panic_reason();
		break;
	case RAMDUMP_SIGN_NORMAL_REBOOT:
		printf("0x%x - User Reboot(S/W Reboot)\n", ret);
		print_lcd_update(FONT_WHITE, FONT_BLACK, "0x%x - User Reboot(S/W Reboot)", ret);
		break;
	case RAMDUMP_SIGN_FORCE_REBOOT:
		printf("0x%x - Forced Reboot(S/W Reboot)\n", ret);
		print_lcd_update(FONT_WHITE, FONT_BLUE, "0x%x - Forced Reboot(S/W Reboot)", ret);
		break;
	case RAMDUMP_SIGN_SAFE_FAULT:
		printf("0x%x - Safe Kernel PANIC\n", ret);
		print_lcd_update(FONT_YELLOW, FONT_RED, "0x%x - Safe Kernel PANIC", ret);
		dfd_display_panic_reason();
		break;
	case RAMDUMP_SIGN_RESET:
	default:
		printf("0x%x - Power/Emergency Reset\n", ret);
		print_lcd_update(FONT_YELLOW, FONT_RED, "0x%x - Power/Emergency Reset", ret);
		break;
	}
}

static const u32 core_stat[NR_CPUS] = {
	CPU0_CORE_STAT,
	CPU1_CORE_STAT,
	CPU2_CORE_STAT,
	CPU3_CORE_STAT,
	CPU4_CORE_STAT,
	CPU5_CORE_STAT,
	CPU6_CORE_STAT,
	CPU7_CORE_STAT,
};

void dfd_display_core_stat(void)
{
	int val;
	u32 ret, ret2;

	printf("Core stat at previous(IRAM)\n" );
	for (val = 0; val < NR_CPUS; val++) {
		ret = readl(core_stat[val]);
		printf("Core%d: 0x%x :", val, core_stat[val]);
		switch(ret) {
		case CLEAR:
			printf("Running\n");
			break;
		case RESET:
			printf("Reset\n");
			break;
		case RESERVED:
			printf("Reserved\n");
			break;
		case HOTPLUG:
			printf("Hotplug\n");
			break;
		case C2_STATE:
			printf("C2\n");
			break;
		case CLUSTER_OFF:
			printf("CLUSTER_OFF\n");
			break;
		default:
			printf("Unknown: 0x%x 0x%x\n", core_stat[val], ret);
			break;
		}
	}
	printf("Core stat at previous(KERNEL)\n" );
	for (val = 0; val < NR_CPUS; val++) {
		ret = readl(CONFIG_RAMDUMP_CORE_POWER_STAT + (val * REG_OFFSET));
		printf("Core%d: ", val);
		switch(ret) {
		case RAMDUMP_SIGN_ALIVE:
			printf("Alive");
			break;
		case RAMDUMP_SIGN_DEAD:
			printf("Dead");
			break;
		case RAMDUMP_SIGN_RESET:
		default:
			printf("Power/Emergency Reset: 0x%x", ret);
			break;
		}

		ret2 = readl(CONFIG_RAMDUMP_CORE_PANIC_STAT + (val * REG_OFFSET));
		switch(ret2) {
		case RAMDUMP_SIGN_PANIC:
			printf("/PANIC\n");
			break;
		case RAMDUMP_SIGN_RESET:
		default:
			printf("\n");
			break;
		}
	}
}

static void dfd_set_big_cluster_rstcon(void)
{
	u32 reg_val, val;

	reg_val = readl(EXYNOS9610_POWER_BASE + CPU_RESET_DISABLE_SOFTRESET);
	if (reg_val & (PEND_BIG | PEND_LITTLE)) {
		reg_val &= ~(PEND_BIG | PEND_LITTLE);
		writel(reg_val, EXYNOS9610_POWER_BASE + CPU_RESET_DISABLE_SOFTRESET);
	}
	reg_val = readl(EXYNOS9610_POWER_BASE + CPU_RESET_DISABLE_WDTRESET);
	if (reg_val & (PEND_BIG | PEND_LITTLE)) {
		reg_val &= ~(PEND_BIG | PEND_LITTLE);
		writel(reg_val, EXYNOS9610_POWER_BASE + CPU_RESET_DISABLE_WDTRESET);
	}

	/* reset enable for BIG */
	for (val = 0; val < NR_BIG_CPUS; val++) {
		reg_val = readl(EXYNOS9610_POWER_BASE + BIG_CPU0_RESET + val * 0x80);
		reg_val &= ~(RESET_DISABLE_WDT_CPUPORESET);
		reg_val &= ~(RESET_DISABLE_CORERESET | RESET_DISABLE_CPUPORESET);
		writel(reg_val, EXYNOS9610_POWER_BASE + BIG_CPU0_RESET + val * 0x80);
	}
	reg_val = readl(EXYNOS9610_POWER_BASE + BIG_NONCPU_RESET);
	reg_val &= ~(RESET_DISABLE_L2RESET | RESET_DISABLE_WDT_L2RESET);
	reg_val &= ~(RESET_DISABLE_WDT_PRESET_DBG | RESET_DISABLE_PRESET_DBG);
	writel(reg_val, EXYNOS9610_POWER_BASE + BIG_NONCPU_RESET);
}

void dfd_set_dump_gpr(int en)
{
	u32 val;

	if (en & CACHE_RESET_EN_MASK)
		val = (DUMPGPR_EN_MASK & en) | DFD_RST_DISABLE_EN;
	else
		val = (DUMPGPR_EN_MASK & en) | DFD_RST_DISABLE_CLR;

	writel(val, EXYNOS9610_POWER_RESET_SEQUENCER_CONFIGURATION);
	val = readl(EXYNOS9610_POWER_RESET_SEQUENCER_CONFIGURATION);

	printf("%sable dumpGPR - %x\n", (val & DUMPGPR_EN_MASK) ? "En" : "Dis", val);
}

static int dfd_wait_complete(unsigned int core)
{
	u32 ret;
	u32 loop = 1000;

	do {
		ret = readl(CONFIG_RAMDUMP_DUMP_GPR_WAIT);
		if (ret & core)
			return 0;
		u_delay(1000);
	} while(loop-- > 0);

	printf("Failed to wait complete - ret:%x core:%x\n", ret, core);
	return -1;
}

static void dfd_print_pc_gpr_little(unsigned int nCpuId)
{
	u64 *cpu_reg_dst;
	u32 idx;

	cpu_reg_dst = (u64 *)(CONFIG_RAMDUMP_COREREG + ((u64)nCpuId * COREREG_OFFSET));

#ifdef DEBUG_PRINT
	/* print gpr x0 ~ x8 */
	for (idx = 0; idx < 9; idx++)
		printf("core %d: reg: %d, 0x%llx\n", nCpuId, idx, cpu_reg_dst[idx]);

	/* print gpr x19 ~ x30 */
	for (idx = 19; idx < 31; idx++)
		printf("core %d: reg: %d, 0x%llx\n", nCpuId, idx, cpu_reg_dst[idx]);

	/* stack pointer is in index 31 */
	printf("core %d: reg: sp: 0x%llx\n", nCpuId, cpu_reg_dst[SP]);
	/* PC value is in index 32 */
	printf("core %d: reg: pc: 0x%llx\n", nCpuId, cpu_reg_dst[PC]);
#endif
	return;
}

static const u32 dbg_base[NR_CPUS] = {
	CPU0_DEBUG_BASE,
	CPU1_DEBUG_BASE,
	CPU2_DEBUG_BASE,
	CPU3_DEBUG_BASE,
	CPU4_DEBUG_BASE,
	CPU5_DEBUG_BASE,
	CPU6_DEBUG_BASE,
	CPU7_DEBUG_BASE,
};

u32 dfd_get_pmudbg_stat(u32 cpu)
{
	u32 reg;

	if (cpu <= LITTLE_CORE_LAST)
		reg = PMUDBG_CL0_CPU0_STATUS + (REG_OFFSET * cpu);
	else
		reg = PMUDBG_CL1_CPU0_STATUS + (REG_OFFSET * (cpu - BIG_CORE_START));

	return readl(reg);
}

static void dfd_set_cache_flush_level(void)
{
	u32 stat, ret1, ret2, ret3;
	int little_on = -1, big_on = -1, val;

	/* copy IRAM core stat to DRAM */
	for (val = 0; val < NR_CPUS; val++) {
		ret1 = readl(core_stat[val]);
		ret2 = readl(CONFIG_RAMDUMP_CORE_PANIC_STAT + (val * REG_OFFSET));
		ret3 = dfd_get_pmudbg_stat(val);
		if ((ret2 == RAMDUMP_SIGN_PANIC) || ((ret3 & PMUDBG_CPU_STAT_MASK) != 0x70000)) {
			stat = (FLUSH_SKIP << 16) | ret1;
		} else {
			stat = (FLUSH_LEVEL1 << 16) | ret1;
			if (val >= LITTLE_CORE_START && val <= LITTLE_CORE_LAST)
					little_on = val;
			else
					big_on = val;
		}
		writel(stat, CONFIG_RAMDUMP_GPR_POWER_STAT + (val * REG_OFFSET));
		printf("Core %d: Initial policy - Cache Flush Level %u\n", val, (u32)(stat >> 16));

		/* clear IRAM core stat to run next booting naturally */
		if (val == 0)
			ret1 = RESERVED;
		else
			ret1 = HOTPLUG;

		writel(ret1, core_stat[val]);
	}

	/* conclude core which runs cache flush level 2 in little cluster */
	if (little_on < 0) {
		/* core0 runs cache flush level 2 */
		val = (FLUSH_LEVEL2 << 16);
		little_on = 0;
	} else {
		val = (FLUSH_LEVEL3 << 16);
	}

	stat = readl(CONFIG_RAMDUMP_GPR_POWER_STAT + (little_on * REG_OFFSET));
	stat = (stat & 0xFFFF) | val;
	writel(stat, CONFIG_RAMDUMP_GPR_POWER_STAT + (little_on * REG_OFFSET));
	printf("Core %d: Cache Flush Level changed => %u\n", little_on, (u32)(stat >> 16));

	if (big_on >= 0) {
		stat = readl(CONFIG_RAMDUMP_GPR_POWER_STAT + (big_on * REG_OFFSET));
		stat = (stat & 0xFFFF) | (FLUSH_LEVEL3 << 16);
		writel(stat, CONFIG_RAMDUMP_GPR_POWER_STAT + (big_on * REG_OFFSET));
		printf("Core %d: Cache Flush Level changed => %u\n", big_on, (u32)(stat >> 16));
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

void dfd_secondary_dump_gpr(int cpu)
{
	u32 val, reg;

        reg = CONFIG_RAMDUMP_COREREG + ((u64)cpu * COREREG_OFFSET);
        val = cpu <= LITTLE_CORE_LAST ? LITTLE_DUMP_PC_ADDRESS : BIG_DUMP_PC_ADDRESS;

	if (cpu >= LITTLE_CORE_START && cpu <= LITTLE_CORE_LAST)
		__dfd_dump_gpr(cpu, reg, val);

	/* Get Cache Flush Level */
	val = readl(CONFIG_RAMDUMP_GPR_POWER_STAT + (cpu * REG_OFFSET)) >> 16;
	switch (val) {
	case 0:
		break;
	case 1:
		/* FLUSH_LEVEL1 : L1 cache flush (local cache flush) */
		dfd_flush_dcache_level(val - 1, 0);
		break;
	case 2:
		/* FLUSH_LEVEL2 : L2 cache flush (cluster cache flush) */
		dfd_flush_dcache_level(val - 1, 0);
		break;
	case 3:
		/* FLUSH_LEVEL3 : L1, L2 cache flush (all cache flush) */
		dfd_flush_dcache_all();
		break;
	default:
		break;
	}
	/* Write own bit to inform finishing dumpGPR */
	val = readl(CONFIG_RAMDUMP_DUMP_GPR_WAIT);
	writel((val | (1 << cpu)), CONFIG_RAMDUMP_DUMP_GPR_WAIT);

	if (cpu != 0) {
		cpu_boot(CPU_OFF_PSCI_ID, 0, 0);
		do {
			wfi();
		} while (1);
	}
}

static void dfd_check_lpi(void)
{
	u32 val = readl(EXYNOS9610_POWER_BASE + TIMEOUT_RESET_LPI);

	printf("Check LPI timeout status = 0x%08x\n", val);
	if (val & STATUS_TIMEOUT_RESET_LPI)
		printf("LPI timeout is occurred\n");
}

static int dfd_check_pre_stat(u32 cpu)
{
	u32 val;

	/* Condition check : Core Power off / Panic at previous time */
	val = readl(CONFIG_RAMDUMP_CORE_PANIC_STAT + (cpu * REG_OFFSET));
	if (val == RAMDUMP_SIGN_PANIC) {
		printf("Core %d: already dump core in panic, Skip Dump GPR: 0x%x\n", cpu, val);
		val = readl(CONFIG_RAMDUMP_DUMP_GPR_WAIT);
		writel((val | (1 << cpu)), CONFIG_RAMDUMP_DUMP_GPR_WAIT);
		return 1;
	}

	val = dfd_get_pmudbg_stat(cpu);
	if ((val & PMUDBG_CPU_STAT_MASK) != 0x70000) {
		printf("Core %d: Power Offed at previous time: 0x%x\n", cpu, val);
		val = readl(CONFIG_RAMDUMP_DUMP_GPR_WAIT);
		writel((val | (1 << cpu)), CONFIG_RAMDUMP_DUMP_GPR_WAIT);
		return 1;
	}

	return 0;
}

void dfd_run_dump_gpr(void)
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

	u64 *cpu_dst;
	u64 (*cpu_src)[35];
	u32 cpu;
	u32 reg;
	u32 need_cache_flush;
	int ret = 0, i;

	for (cpu = LITTLE_CORE_START; cpu <= BIG_CORE_LAST; cpu++)
		dfd_print_pcval(cpu);

	/* Check reset_sequencer_configuration register */
	reg = readl(EXYNOS9610_POWER_RESET_SEQUENCER_CONFIGURATION);
	if (!(reg & EDPCSR_DUMP_EN))
		return;

	/* Initialization to use waiting for complete Dump GPR of other cores */
	writel(0, CONFIG_RAMDUMP_DUMP_GPR_WAIT);

	printf("---------------------------------------------------------\n");
	printf("Dump GPR_EN & Watchdog or Warm Reset Detected\n");

	dfd_check_lpi();
	if (reg & DFD_RST_DISABLE_EN)
		dfd_set_cache_flush_level();

	printf("dumpGPR for little cluster\n");
	for (cpu = LITTLE_CORE_START; cpu <= LITTLE_CORE_LAST; cpu++) {

		if (dfd_check_pre_stat(cpu))
			continue;

		if (cpu == 0) {
			/* CPU 0 jumps just calling function */
			dfd_secondary_dump_gpr(cpu);
		} else {
			/* ON other cpus */
			ret = cpu_boot(CPU_ON_PSCI_ID,
					cpu_logical_map[cpu], (u64)dfd_entry_dump_gpr);
			if (ret) {
				printf("ERROR: Core %d: failed to power on : 0x%x\n", cpu, ret);
				ret = readl(CONFIG_RAMDUMP_DUMP_GPR_WAIT);
				writel((ret | (1 << cpu)), CONFIG_RAMDUMP_DUMP_GPR_WAIT);
				continue;
			}
			ret = dfd_wait_complete(1 << cpu);
		}
		if (!ret) {
			dfd_print_pc_gpr_little(cpu);
			printf("Core %d: finished to dump GPR\n", cpu);
		}
	}

	printf("dumpGPR for big cluster\n");
	for (cpu = BIG_CORE_START; cpu <= BIG_CORE_LAST; cpu++) {
		if (dfd_check_pre_stat(cpu))
			continue;

		/* Copy BIG core's GPR from SRAM to DRAM */
		cpu_dst = (u64 *)(CONFIG_RAMDUMP_COREREG + ((u64)cpu * COREREG_OFFSET));
		cpu_src = (u64(*)[35])0x0203F400;

		/* cpu_dst[0]~cpu_dst[34] <-- cpu_src[cpu-BIG_CORE_START][0] ~ cpu_src[cpu-BIG_CORE_START][34] */
		for (i = 0; i < 35; i++)
			cpu_dst[i] = cpu_src[cpu-BIG_CORE_START][i];

#ifdef DEBUG_PRINT
		/* print gpr x0 ~ x30, sp, pc, pstate, esr */
		for (i = 0; i < 31; i++)
			printf("core %d: reg: %d, 0x%llx\n", cpu, i, cpu_dst[i]);

		/* stack pointer is in index 31 */
		printf("core %d: reg: sp: 0x%llx\n", cpu, cpu_dst[SP]);
		/* PC value is in index 32 */
		printf("core %d: reg: pc: 0x%llx\n", cpu, cpu_dst[PC]);
		/* PSTATE value is in index 33 */
		printf("core %d: reg: pstate: 0x%llx\n", cpu, cpu_dst[PSTATE]);
		/* ESR value is in index 34 */
		printf("core %d: reg: esr: 0x%llx\n", cpu, cpu_dst[ESR_EL1]);
#endif
		printf("Core %d: finished to dump GPR\n", cpu);

		/* BIG core Cache Flush */
		printf("Cache flush of core %d\n", cpu);
		need_cache_flush = (readl(CONFIG_RAMDUMP_GPR_POWER_STAT + (cpu * REG_OFFSET))) >> 16;
		if (need_cache_flush) {
			/* ON BIG cpus */
			ret = cpu_boot(CPU_ON_PSCI_ID,
					cpu_logical_map[cpu], (u64)dfd_entry_dump_gpr);
			if (ret) {
				printf("ERROR: Core %d: failed to power on : 0x%x\n", cpu, ret);
				ret = readl(CONFIG_RAMDUMP_DUMP_GPR_WAIT);
				writel((ret | (1 << cpu)), CONFIG_RAMDUMP_DUMP_GPR_WAIT);
				continue;
			}

			ret = dfd_wait_complete(1 << cpu);
			if (!ret)
				printf("Core %d: finished Cache Flush: %x\n", cpu, need_cache_flush);
		}
	}

	dfd_set_dump_gpr(0);
	printf("---------------------------------------------------------\n");
}

void reset_prepare_board(void)
{
	printf("%s: disable asserting rstdisable L1/L2 cache\n", __func__);
	dfd_set_dump_gpr(0);
	dfd_set_big_cluster_rstcon();
}
