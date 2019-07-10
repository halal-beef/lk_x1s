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
#include <pit.h>
#include <arch/ops.h>
#include <lib/font_display.h>
#include <platform/sizes.h>
#include <platform/sfr.h>
#include <platform/smc.h>
#include <platform/delay.h>
#include <platform/dfd.h>
#ifdef SCAN2DRAM_SOLUTION
#include <platform/dfd_compact.h>
#endif

#define TIMEOUT	100000
s64 sec_area_base;
s64 sec_area_end;

void wfi(void)
{
	asm volatile ("wfi");
}

static inline void pmu_set_bit_atomic(u32 offset, u32 bit)
{
        writel(bit, EXYNOS9830_POWER_BASE + (offset | 0xc000));
}

static inline void pmu_clr_bit_atomic(u32 offset, u32 bit)
{
        writel(bit, EXYNOS9830_POWER_BASE + (offset | 0x8000));
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

static void dfd_display_panic_reason(void)
{
	char *str = (char *)CONFIG_RAMDUMP_PANIC_REASON;
	int is_string = 0;
	int cnt = 0;

	for (cnt = 0; cnt < CONFIG_RAMDUMP_PANIC_LOGSZ; cnt++, str++)
		if (*str == 0x0)
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
	print_lcd_update(FONT_WHITE, FONT_BLACK, "reboot reason: ");

	switch (ret) {
	case RAMDUMP_SIGN_PANIC:
		printf("reboot reason: 0x%x - Kernel PANIC\n", ret);
		print_lcd_update(FONT_YELLOW, FONT_RED, "0x%x - Kernel PANIC", ret);
		dfd_display_panic_reason();
		break;
	case RAMDUMP_SIGN_NORMAL_REBOOT:
		printf("reboot reason: 0x%x - User Reboot(S/W Reboot)\n", ret);
		print_lcd_update(FONT_WHITE, FONT_BLACK, "0x%x - User Reboot(S/W Reboot)", ret);
		break;
	case RAMDUMP_SIGN_BL_REBOOT:
		printf("reboot resaon: 0x%x - BL Reboot(S/W Reboot)\n", ret);
		print_lcd_update(FONT_WHITE, FONT_BLACK, "0x%x - BL Reboot(S/W Reboot)", ret);
		break;
	case RAMDUMP_SIGN_FORCE_REBOOT:
		printf("reboot reason: 0x%x - Forced Reboot(S/W Reboot)\n", ret);
		print_lcd_update(FONT_WHITE, FONT_BLUE, "0x%x - Forced Reboot(S/W Reboot)", ret);
		break;
	case RAMDUMP_SIGN_SAFE_FAULT:
		printf("reboot reason: 0x%x - Safe Kernel PANIC\n", ret);
		print_lcd_update(FONT_YELLOW, FONT_RED, "0x%x - Safe Kernel PANIC", ret);
		dfd_display_panic_reason();
		break;
	case RAMDUMP_SIGN_RESET:
	default:
		printf("reboot reason: 0x%x - Power/Emergency Reset\n", ret);
		print_lcd_update(FONT_YELLOW, FONT_RED, "0x%x - Power/Emergency Reset", ret);
		break;
	}
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

void dfd_display_core_stat(void)
{
	int val;
	u32 ret;

	printf("Core stat at previous(KERNEL)\n");
	for (val = 0; val < NR_CPUS; val++) {
		ret = readl(CONFIG_RAMDUMP_CORE_POWER_STAT + (val * REG_OFFSET));
		printf("Core%d: ", val);
		switch (ret) {
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

		ret = readl(CONFIG_RAMDUMP_CORE_PANIC_STAT + (val * REG_OFFSET));
		switch (ret) {
		case RAMDUMP_SIGN_PANIC:
			printf("/PANIC\n");
			break;
		case RAMDUMP_SIGN_RESET:
		case RAMDUMP_SIGN_RESERVED:
		default:
			printf("\n");
			break;
		}
	}
}

static void dfd_display_pc_value(u32 reg)
{
	int i;
	u64 cpu_reg;
	u64 val;

	for (i = 0; i < NR_CPUS; i++) {
		cpu_reg = (CONFIG_RAMDUMP_COREREG + (i * COREREG_OFFSET) + (0x8 * reg));
		val = readq(cpu_reg);
#ifdef SCAN2DRAM_SOLUTION
		printf("Core%d: reg : 0x%llX", i, val);
		if (!val || dfd_check_panic_stat(i)) {
			printf("\n");
			continue;
		}
		cpu_reg = (CONFIG_RAMDUMP_COREREG + (i * COREREG_OFFSET) + (0x8 * PSTATE));
		val = readl(cpu_reg);
		printf(", EL%d, ", (int)(val >> 2) & 0x3);

		cpu_reg = (CONFIG_RAMDUMP_COREREG + (i * COREREG_OFFSET) + (0x8 * NS));
		val = readl(cpu_reg);
		printf("%sSecure\n", val ? "Non" : "");
#else
		printf("Core%d: reg : 0x%llX\n", i, val);
#endif
	}
}

void dfd_set_dump_en_for_cacheop(int en)
{
	if (en)
		pmu_set_bit_atomic(RESET_SEQUENCER_OFFSET, DUMP_EN_BIT);
	else
		pmu_clr_bit_atomic(RESET_SEQUENCER_OFFSET, DUMP_EN_BIT);
}

/*
 * FLUSH_SKIP : skip cache flush
 * FLUSH_LEVEL 1 : L1, L2 cache flush (local cache flush)
 * FLUSH_LEVEL 2 : L3 cache flush (cluster cache flush)
 * FLUSH_LEVEL 3 : L1, L2, L3 cache flush (all cache flush)
 */
static void dfd_set_cache_flush_level(void)
{
	int cpu, little_on = -1, big_on = -1;
	u64 val, stat;
	u64 *cpu_reg;

	for (cpu = 0; cpu < NR_CPUS; cpu++) {
		u32 offset = cpu * REG_OFFSET;
		cpu_reg = (u64 *)(CONFIG_RAMDUMP_COREREG + ((u64)cpu * COREREG_OFFSET));
		if (!cpu_reg[POWER_STATE]) {
			stat = FLUSH_SKIP;
		} else {
			if (cpu <= MID_CORE_LAST) {
				stat = FLUSH_LEVEL2;
				little_on = cpu;
			} else {
				stat = FLUSH_LEVEL1;
				big_on = cpu;
			}
		}
		writel(stat, CONFIG_RAMDUMP_GPR_POWER_STAT + offset);
		printf("Core%d: Initial policy - Cache Flush Level %llu\n", cpu, stat);
	}

	/* conclude core which runs cache flush level 2 in little cluster */
	if (little_on < 0) {
		/* core0 runs cache flush level 2 */
		val = FLUSH_LEVEL2;
		little_on = 0;
	} else {
		val = FLUSH_LEVEL3;
	}

	stat = readl(CONFIG_RAMDUMP_GPR_POWER_STAT + (little_on * REG_OFFSET));
	stat |= val;
	writel(stat, CONFIG_RAMDUMP_GPR_POWER_STAT + (little_on * REG_OFFSET));
	printf("Core%d: Cache Flush Level changed => %llu\n", little_on, stat);

	if (big_on >= 0) {
		stat = readl(CONFIG_RAMDUMP_GPR_POWER_STAT + (big_on * REG_OFFSET));
		stat |= FLUSH_LEVEL3;
		writel(stat, CONFIG_RAMDUMP_GPR_POWER_STAT + (big_on * REG_OFFSET));
		printf("Core%d: Cache Flush Level changed => %llu\n", big_on, stat);
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

	do {
		val = readl(CONFIG_RAMDUMP_WAKEUP_WAIT);
		if (val & (1 << cpu))
			break;
	} while (1);

	if (cpu >= BIG_CORE_START && cpu <= BIG_CORE_LAST)
		goto off;

	/* Get Cache Flush Level */
	val = readl(CONFIG_RAMDUMP_GPR_POWER_STAT + (cpu * REG_OFFSET));
	switch (val) {
	case 0:
		break;
	case 1:
		/* FLUSH_LEVEL1 : L1, L2 cache flush (local cache flush) */
		dfd_flush_dcache_level(val - 1, 0);
		break;
	case 2:
		/* FLUSH_LEVEL2 : L3 cache flush (cluster cache flush) */
		dfd_flush_dcache_level(val - 2, 0);
		dfd_flush_dcache_level(val - 1, 0);
		break;
	case 3:
		/* FLUSH_LEVEL3 : L1, L2, L3 cache flush (all cache flush) */
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
		cpu_boot(CPU_OFF_PSCI_ID, 0, 0);
		do {
			wfi();
		} while (1);
	}
}

static void get_sec_info(bool on)
{
	if (on) {
		s64 sec_area_length = exynos_smc(SMC_CMD_GET_SOC_INFO,
				SOC_INFO_TYPE_SEC_DRAM_SIZE, 0, 0);
		sec_area_base = exynos_smc(SMC_CMD_GET_SOC_INFO,
				SOC_INFO_TYPE_SEC_DRAM_BASE, 0, 0);

		if (sec_area_base == ERROR_INVALID_TYPE) {
			printf("get secure memory base addr error!!\n");
			return;
		}

		if (sec_area_length == ERROR_INVALID_TYPE) {
			printf("get secure memory size error!!\n");
			return;
		}
		sec_area_end = sec_area_base + sec_area_length - 1;
	} else {
		sec_area_base = 0;
		sec_area_end = 0;
	}
}

static void dfd_ipc_read_buffer(void *dest, const void *src, int len)
{
	const unsigned int *sp = src;
	unsigned int *dp = dest;
	int i;

	for (i = 0; i < len; i++)
		*dp++ = readl(sp++);
}

static void dfd_ipc_write_buffer(void *dest, const void *src, int len)
{
	const unsigned int *sp = src;
	unsigned int *dp = dest;
	int i;

	for (i = 0; i < len; i++)
		writel(*sp++, dp++);
}

static void dfd_interrupt_gen(void)
{
	writel(1, EXYNOS9830_DBG_MBOX_BASE + INTGR_AP_TO_DBGC);
}

static int dfd_ipc_send_data_polling(struct dfd_ipc_cmd *cmd)
{
	int timeout = TIMEOUT;
	int id;

	if (!cmd)
		return -1;

	id = cmd->cmd_raw.id;
	cmd->cmd_raw.manual_polling = 1;
	cmd->cmd_raw.one_way = 0;
	cmd->cmd_raw.response = 0;

	dfd_ipc_write_buffer((void *)EXYNOS9830_DBG_MBOX_FW_CH, cmd, 4);
	dfd_interrupt_gen();

	do {
		(id == PP_IPC_CMD_ID_RUN_DUMP) ? mdelay(100) : udelay(100);
		dfd_ipc_read_buffer(cmd->buffer, (void *)EXYNOS9830_DBG_MBOX_FW_CH, 4);
	} while (!(cmd->cmd_raw.response) && timeout--);

	if (!cmd->cmd_raw.response) {
		printf("%s: id:%d timeout error\n", __func__, cmd->cmd_raw.id);
		return -1;
	}

	dfd_ipc_read_buffer(cmd->buffer, (void *)EXYNOS9830_DBG_MBOX_FW_CH, 4);
	return 0;
}

static void dfd_ipc_fill_buffer(struct dfd_ipc_cmd *cmd, u32 data1, u32 data2, u32 data3)
{
	cmd->buffer[1] = data1;
	cmd->buffer[2] = data2;
	cmd->buffer[3] = data3;
}

void dfd_run_post_processing(void)
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
	struct dfd_ipc_cmd cmd;
	u32 arr_addr = (u32)dbg_snapshot_get_item_paddr("log_arrdumpreset");
	u32 s2d_addr = (u32)dbg_snapshot_get_item_paddr("log_s2d");
	u32 s2d_size = (u32)dbg_snapshot_get_item_size("log_s2d");

	memset(&cmd, 0, sizeof(struct dfd_ipc_cmd));
	printf("---------------------------------------------------------\n");
	printf("Watchdog or Warm Reset Detected.\n");

	cmd.cmd_raw.cmd = IPC_CMD_POST_PROCESSING;
	/* Initialization to use waiting for complete Dump GPR of other cores */
	writel(0, CONFIG_RAMDUMP_DUMP_GPR_WAIT);
	writel(0, CONFIG_RAMDUMP_WAKEUP_WAIT);

	cmd.cmd_raw.id = PP_IPC_CMD_ID_START;
	dfd_ipc_fill_buffer(&cmd, s2d_addr, s2d_size, 0);
	ret = dfd_ipc_send_data_polling(&cmd);
#ifdef SCAN2DRAM_SOLUTION
	if (!ret && cmd.buffer[1]) {
		printf("S2D Magic Detected - s2d sanity pass.\n");
	} else {
		printf("S2D Magic didn't detect - s2d sanity fail!\n");
		goto done;
	}
#if 0
	//send Postprocessing Command. Id value is GET_GPR send with s2d dump address.
	cmd.cmd_raw.id = PP_IPC_CMD_ID_RUN_GPR;
	dfd_ipc_fill_buffer(&cmd, (u64)dfd_compact_cl0_bin, (u64)dfd_compact_cl2_bin, 0);
	printf("Try to get GPR. (0x%llx, 0x%llx) - ",
			(u64)dfd_compact_cl0_bin, (u64)dfd_compact_cl2_bin);
	printf("%s(0x%x)!\n", dfd_ipc_send_data_polling(&cmd) < 0 ? "Failed" : "Finish", cmd.buffer[1]);
#endif
#endif
	printf("Display PC value\n");
	dfd_display_pc_value(PC);

skip_gpr:
	if (!(readl(EXYNOS9830_POWER_BASE + RESET_SEQUENCER_OFFSET) & DUMP_EN)) {
		printf("DUMP_EN disabled, Skip cache flush.\n");
		goto finish;
	}

	llc_flush_disable();

	/* Following code is array dump and cache flush. (related cache operation) */
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
	}

	mdelay(100);

	//Send Postprocessing Command. ID value is RUN DUMP.
	cmd.cmd_raw.id = PP_IPC_CMD_ID_RUN_DUMP;
	dfd_ipc_fill_buffer(&cmd, arr_addr, cpu_mask & 0x3f, 0);
	printf("Try to get Arraydump of power on cores - ");
	printf("%s(0x%x)!\n", dfd_ipc_send_data_polling(&cmd) < 0 ? "Failed" : "Finish", cmd.buffer[1]);

	get_sec_info(true);
	//when receiving ipc, cpu0 is running. Run cache flush
	for (cpu = 0; cpu < NR_CPUS; cpu++) {
		val = readl(CONFIG_RAMDUMP_WAKEUP_WAIT);
		writel(val | (1 << cpu), CONFIG_RAMDUMP_WAKEUP_WAIT);
		if (cpu == 0)
			dfd_secondary_cpu_cache_flush(cpu);

		if (cpu >= BIG_CORE_START && cpu <= BIG_CORE_LAST)
			write_back_cache(cpu);

		if (!dfd_wait_complete(cpu)) {
			printf("Core%d: ERR wait timeout.\n", cpu);
			continue;
		}

		printf("Core%d: finished Cache Flush level:%d (0x%x)\n", cpu,
		(readl(CONFIG_RAMDUMP_GPR_POWER_STAT + (cpu * REG_OFFSET))),
		readl(CONFIG_RAMDUMP_DUMP_GPR_WAIT));
	}
	get_sec_info(false);
finish:
	cmd.cmd_raw.id = PP_IPC_CMD_ID_FINISH;
	dfd_ipc_fill_buffer(&cmd, 0, 0, 0);
	dfd_ipc_send_data_polling(&cmd);
done:
	printf("---------------------------------------------------------\n");
}

void dfd_get_dbgc_version(void)
{
	u32 flag;
	char str[DBGC_VERSION_LEN];
	struct dfd_ipc_cmd cmd;

	flag = readl(EXYNOS9830_DBG_MBOX_SRn(0));
	writel(0, EXYNOS9830_DBG_MBOX_SRn(0));
	if (flag == 0xDB9C5A1D) {
		memcpy(str, (void *)CONFIG_RAMDUMP_DBGC_VERSION, DBGC_VERSION_LEN);
		str[DBGC_VERSION_LEN - 1] = '\0';
		printf("DBGCORE: VERSION: %s\n", str);
	} else {
		printf("DBGCORE: boot fail!\n");
		return;
	}

	memset(&cmd, 0, sizeof(struct dfd_ipc_cmd));
	cmd.cmd_raw.cmd = IPC_CMD_DEBUG_LOG_INFO;
	dfd_ipc_fill_buffer(&cmd, dbg_snapshot_get_item_paddr("log_dbgc"),
			dbg_snapshot_get_item_size("log_dbgc"), 0);
	dfd_ipc_send_data_polling(&cmd);
}

int dfd_get_revision(void)
{
	char str[DBGC_VERSION_LEN];

	memcpy(str, (void *)CONFIG_RAMDUMP_DBGC_VERSION, DBGC_VERSION_LEN);
	str[DBGC_VERSION_LEN - 1] = '\0';

	if (strstr(str, "EVT0"))
		return 0;
	else if (strstr(str, "EVT1"))
		return 1;
	else
		return -1;
}

unsigned int clear_llc_init_state(void)
{
	pmu_clr_bit_atomic(RESET_SEQUENCER_OFFSET, LLC_INIT_BIT);

	return readl(EXYNOS9830_POWER_BASE + RESET_SEQUENCER_OFFSET);
}

void reset_prepare_board(void)
{
	dfd_set_dump_en_for_cacheop(0);
	/* Clear debug level when reset */
	writel(0, CONFIG_RAMDUMP_DEBUG_LEVEL);
}

const char *debug_level_val[] = {
	"low",
	"mid",
};

void set_debug_level(const char *buf)
{
	int i, debug_level = DEBUG_LEVEL_MID;

	if (!buf)
		return;

	for (i = 0; i < (int)ARRAY_SIZE(debug_level_val); i++) {
		if (!strncmp(buf, debug_level_val[i],
			strlen(debug_level_val[i]))) {
			debug_level = i;
			goto debug_level_print;
		}
	}

debug_level_print:
	/* Update debug_level to reserved region */
	writel(debug_level | DEBUG_LEVEL_PREFIX, CONFIG_RAMDUMP_DEBUG_LEVEL);
	printf("debug level: %s\n", debug_level_val[debug_level]);
}

void set_debug_level_by_env(void)
{
#if 0
	char buf[16] = {0, };
	int ret;

	ret = getenv_s("debug_level", buf, sizeof(buf));
	if (ret > 0) {
		printf("Change ");
		set_debug_level(buf);
	}
#endif
}

void avb_print_lcd(const char *str, char *color)
{
	uint32_t font_color;

	switch (color[0]) {
	case 'o':
		font_color = FONT_ORANGE;
		break;
	case 'y':
		font_color = FONT_YELLOW;
		break;
	case 'r':
		font_color = FONT_RED;
		break;
	case 'g':
		font_color = FONT_GREEN;
		break;
	default:
		print_lcd_update(FONT_WHITE, FONT_BLACK,
				"%s : color parsing fail\n", __func__);
		return;
	}
	print_lcd_update(font_color, FONT_BLACK, str);
}
