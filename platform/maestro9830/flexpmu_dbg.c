#include <debug.h>
#include <reg.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdlib.h>
#include <lib/console.h>

#include "flexpmu_dbg.h"
#include "include/platform/exynos9830.h"

enum dbg_index {
	CPU_SEQ_STATUS,
	PWR_SEQ_STATUS,
	RUNNING_SEQUENCER,
	SEQUENCER_INFO,
	TZ_SETTING_STATUS,
	TZ_INPUT_OUTPUT,
	DRAM_INIT_STATUS,
	DRAM_INIT_INPUT_OUTPUT,
	SCI_INIT_STATUS,
	SCI_INIT_INPUT_OUTPUT,
	SW_FLAG,
	IRQ_STATUS,
	IRQ_INFO,
	AP_APM_IPC_STATUS,
	AP_APM_IPC_DATA0,
	AP_APM_IPC_DATA1,
	APSOC_SEQ_TOTAL_COUNT,
	MIF_SEQ_TOTAL_COUNT,
	IPC_VTS0,
	IPC_VTS1,
	LOCALPWR,
	MIF_SEQ_CONTROL,
	APSOC_SLEEP_SEQ_COUNT,
	MIF_SLEEP_SEQ_COUNT,
	APSOC_SICD_SEQ_COUNT,
	MIF_SICD_SEQ_COUNT,
	CURRENT_POWER_MODE,
};

struct dbg_list flexpmu_dbg[] = {
	FLEXPMU_DBG("CPU_SEQ_STATUS"),
	FLEXPMU_DBG("PWR_SEQ_STATUS"),
	FLEXPMU_DBG("RUNNING_SEQUENCER"),
	FLEXPMU_DBG("SEQUENCER_INFO"),
	FLEXPMU_DBG("TZ_SETTING_STATUS"),
	FLEXPMU_DBG("TZ_INPUT_OUTPUT"),
	FLEXPMU_DBG("DRAM_INIT_STATUS"),
	FLEXPMU_DBG("DRAM_INIT_INPUT_OUTPUT"),
	FLEXPMU_DBG("SCI_INIT_STATUS"),
	FLEXPMU_DBG("SCI_INIT_INPUT_OUTPUT"),
	FLEXPMU_DBG("SW_FLAG"),
	FLEXPMU_DBG("IRQ_STATUS"),
	FLEXPMU_DBG("IRQ_INFO"),
	FLEXPMU_DBG("AP_APM_IPC_STATUS"),
	FLEXPMU_DBG("AP_APM_IPC_DATA0"),
	FLEXPMU_DBG("AP_APM_IPC_DATA1"),
	FLEXPMU_DBG("APSOC_SEQ_TOTAL_COUNT"),
	FLEXPMU_DBG("MIF_SEQ_TOTAL_COUNT"),
	FLEXPMU_DBG("IPC_VTS0"),
	FLEXPMU_DBG("IPC_VTS1"),
	FLEXPMU_DBG("LOCALPWR"),
	FLEXPMU_DBG("MIF_SEQ_CONTROL"),
	FLEXPMU_DBG("APSOC_SLEEP_SEQ_COUNT"),
	FLEXPMU_DBG("MIF_SLEEP_SEQ_COUNT"),
	FLEXPMU_DBG("APSOC_SICD_SEQ_COUNT"),
	FLEXPMU_DBG("MIF_SICD_SEQ_COUNT"),
	FLEXPMU_DBG("CURRENT_POWER_MODE"),
};

static const char *up_sequencer[] = {
	"DONE",
	"CPU0",
	"CPU1",
	"CPU2",
	"CPU3",
	"CPU4",
	"CPU5",
	"CPU6",
	"CPU7",
	"NON_CPU CL0",
	"NON_CPU CL1",
	"NON_CPU CL2",
	"UNKNOWN",
	"L3FLUSH ABORT",
	"SOC_SEQ",
	"MIF_SEQ",
};

static const char *down_sequencer[] = {
	"DONE",
	"CPU0",
	"CPU1",
	"CPU2",
	"CPU3",
	"CPU4",
	"CPU5",
	"CPU6",
	"CPU7",
	"NON_CPU CL0",
	"NON_CPU CL1",
	"NON_CPU CL2",
	"L3FLUSH_START",
	"UNKNOWN",
	"SOC_SEQ",
	"MIF_SEQ",
};

static const char *power_mode_name[] = {
	"NO_POWER_MODE",
	"NO_POWER_MODE",
	"NO_POWER_MODE",
	"SICD",
	"SLEEP",
	"USBL2_SLEEP",
};

#define DONE_INDEX                              (0x21444E45)
static const char *access_type[] = {
	"READ",
	"WRITE",
	"COND_READ",
	"COND_WRITE",
	"SAVE_RESTORE",
	"COND_SAVE_RESTORE",
	"WAIT",
	"WAIT_TWO",
	"CHECK_SKIP",
	"COND_CHECK_SKIP",
	"WRITE_WAIT",
	"WRITE_RETURN",
	"DELAY",
	"CHECK_CHIPID",
	"EXT_FUNC",
};

static const char *master[] = {
	"CP",
	"AUD",
	"none",
	"VTS",
	"AP",
};

static void print_cpu_seq_status(struct dbg_list *dbg)
{
	int i, j;

	printf("%s%s - ", FLEXPMU_DBG_LOG, dbg->name);
	for (j = 0; j < 2; j++) {
		for (i = 0; i < 4; i++) {
			printf("cpu%d:%s", i + (j << 2),
					dbg->u8_data[i + (j << 2)] ? "on" : "off");

			if ((j == 1) && (i == 3))
				printf("\n");
			else
				printf(", ");
		}
	}
}

static void print_pwr_seq_status(struct dbg_list *dbg)
{
	printf("%s%s - APSOC_SEQ_%s, MIF_SEQ_%s, CL0_%s, CL1_%s, CL2_%s\n",
			FLEXPMU_DBG_LOG, dbg->name,
			dbg->u8_data[0] ? "up" : "down",
			dbg->u8_data[1] ? "up" : "down",
			dbg->u8_data[4] ? "on" : "off",
			dbg->u8_data[5] ? "on" : "off",
			dbg->u8_data[6] ? "on" : "off");
}

static void print_running_sequencer(struct dbg_list *dbg)
{
	if (dbg->u32_data[0] < ARRAY_SIZE(up_sequencer))
		printf("%s[UP] %s - %s\n", FLEXPMU_DBG_LOG, dbg->name,
				up_sequencer[dbg->u32_data[0]]);
	else
		printf("%s[UP] %s - %u\n", FLEXPMU_DBG_LOG, dbg->name,
				dbg->u32_data[0]);

	if (dbg->u32_data[1] < ARRAY_SIZE(down_sequencer))
		printf("%s[DOWN] %s - %s\n", FLEXPMU_DBG_LOG, dbg->name,
				down_sequencer[dbg->u32_data[1]]);
	else
		printf("%s[DOWN] %s - %u\n", FLEXPMU_DBG_LOG, dbg->name,
				dbg->u32_data[1]);
}

static void print_sequencer_info(struct dbg_list *dbg)
{

	if (dbg->u32_data[1] != DONE_INDEX) {
		if (dbg->u32_data[0] < ARRAY_SIZE(access_type))
			printf("%saccess_type: %s - %s, index: %u\n", FLEXPMU_DBG_LOG, dbg->name,
					access_type[dbg->u32_data[0]],
					dbg->u32_data[1]);
		else
			printf("%saccess_type: %s - %u, index: %u\n", FLEXPMU_DBG_LOG, dbg->name,
					dbg->u32_data[0],
					dbg->u32_data[1]);
	} else {
		printf("%saccess_type: %s - %u, DONE\n", FLEXPMU_DBG_LOG, dbg->name,
					dbg->u32_data[0]);
	}
}

static void print_tz_setting_status(struct dbg_list *dbg)
{
	if (dbg->u32_data[1] == 0)
		printf("%sSAC STATUS: done\n", FLEXPMU_DBG_LOG);
	else
		printf("%sSAC STATUS: running\n", FLEXPMU_DBG_LOG);
}

static void print_tz_input_output(struct dbg_list *dbg)
{
	printf("%sSAC input: 0x%x\n", FLEXPMU_DBG_LOG, dbg->u32_data[1]);
	printf("%sSAC return: 0x%x\n", FLEXPMU_DBG_LOG, dbg->u32_data[0]);
}

static void print_dram_init_setting_status(struct dbg_list *dbg)
{
	if (dbg->u32_data[1] == 0)
		printf("%sdram_init STATUS: done\n", FLEXPMU_DBG_LOG);
	else
		printf("%sdram_init STATUS: running\n", FLEXPMU_DBG_LOG);
}

static void print_dram_init_input_output(struct dbg_list *dbg)
{
	printf("%sdram_init input: 0x%x\n", FLEXPMU_DBG_LOG, dbg->u32_data[1]);
	printf("%sdram_init return: 0x%x\n", FLEXPMU_DBG_LOG, dbg->u32_data[0]);
}

static void print_sci_init_setting_status(struct dbg_list *dbg)
{
	if (dbg->u32_data[1] == 0)
		printf("%ssci_init STATUS: done\n", FLEXPMU_DBG_LOG);
	else
		printf("%ssci_init STATUS: running\n", FLEXPMU_DBG_LOG);
}

static void print_sci_init_input_output(struct dbg_list *dbg)
{
	printf("%ssci_init input: 0x%x\n", FLEXPMU_DBG_LOG, dbg->u32_data[1]);
	printf("%ssci_init return: 0x%x\n", FLEXPMU_DBG_LOG, dbg->u32_data[0]);
}


static void print_sw_flag(struct dbg_list *dbg)
{
	int i;

	printf("%shotplug_flag - ", FLEXPMU_DBG_LOG);

	for (i = 0; i < 8; i++)
		printf("cpu%d:%s, ",  i,
				((dbg->u8_data[0] & (1 << i)) >> i) ? "out" : "in");
	printf("\n");

	printf("%sMIF REF_COUNT:%u\n", FLEXPMU_DBG_LOG, (unsigned int)dbg->u8_data[7]);

	printf("%sMIF REQ_MASTER - ", FLEXPMU_DBG_LOG);
	for (i = 0; i < 5; i++) {
		if (dbg->u8_data[6] & (1 << i))
			printf("%s, ", master[i]);
	}
	printf("\n");
}

static void print_powermode_count(struct dbg_list *dbg)
{
	printf("%s%s - %u\n", FLEXPMU_DBG_LOG, dbg->name,
			dbg->u32_data[1]);
}

static void print_last_powermode(struct dbg_list *dbg)
{
	if (dbg->u32_data[1] == APSOC_RUNNING) {
		printf("%s NO POWER MODE\n", FLEXPMU_DBG_LOG);
	} else {
		if (dbg->u32_data[1] < ARRAY_SIZE(power_mode_name))
			printf("%s%s - %s\n", FLEXPMU_DBG_LOG, dbg->name,
					power_mode_name[dbg->u32_data[1]]);
		else
			printf("%s%s - %u\n", FLEXPMU_DBG_LOG, dbg->name,
					dbg->u32_data[1]);
	}
}

static void print_pmudbg_registers(void)
{
	printf("%s%s - 0x%x\n", FLEXPMU_DBG_LOG, "CLUSTER0_CPU0_STATES", readl(EXYNOS9830_PMUDBG_BASE));
	printf("%s%s - 0x%x\n", FLEXPMU_DBG_LOG, "CLUSTER0_CPU1_STATES", readl(EXYNOS9830_PMUDBG_BASE + 0x4));
	printf("%s%s - 0x%x\n", FLEXPMU_DBG_LOG, "CLUSTER0_CPU2_STATES", readl(EXYNOS9830_PMUDBG_BASE + 0x8));
	printf("%s%s - 0x%x\n", FLEXPMU_DBG_LOG, "CLUSTER0_CPU3_STATES", readl(EXYNOS9830_PMUDBG_BASE + 0xc));
	printf("%s%s - 0x%x\n", FLEXPMU_DBG_LOG, "CLUSTER0_NONCPU_STATES", readl(EXYNOS9830_PMUDBG_BASE + 0x10));
	printf("%s%s - 0x%x\n", FLEXPMU_DBG_LOG, "CLUSTER1_CPU0_STATES", readl(EXYNOS9830_PMUDBG_BASE + 0x14));
	printf("%s%s - 0x%x\n", FLEXPMU_DBG_LOG, "CLUSTER1_CPU1_STATES", readl(EXYNOS9830_PMUDBG_BASE + 0x18));
	printf("%s%s - 0x%x\n", FLEXPMU_DBG_LOG, "CLUSTER1_NONCPU_STATES", readl(EXYNOS9830_PMUDBG_BASE + 0x1c));
	printf("%s%s - 0x%x\n", FLEXPMU_DBG_LOG, "CLUSTER2_CPU0_STATES", readl(EXYNOS9830_PMUDBG_BASE + 0x20));
	printf("%s%s - 0x%x\n", FLEXPMU_DBG_LOG, "CLUSTER2_CPU1_STATES", readl(EXYNOS9830_PMUDBG_BASE + 0x24));
	printf("%s%s - 0x%x\n", FLEXPMU_DBG_LOG, "CLUSTER2_NONCPU_STATES", readl(EXYNOS9830_PMUDBG_BASE + 0x28));
	printf("%s%s - 0x%x\n", FLEXPMU_DBG_LOG, "MIF_STATES", readl(EXYNOS9830_PMUDBG_BASE + 0xe8));
	printf("%s%s - 0x%x\n", FLEXPMU_DBG_LOG, "TOP_STATES", readl(EXYNOS9830_PMUDBG_BASE + 0xec));
}

static void print_acpm_last_gpr(void)
{
	if (readl(EXYNOS9830_DUMP_GPR_BASE) == 0xAAAAAAAA)
		return ;

	printf("ACPM Dump GPR\n");
	printf("ACPM R0~R3  : %08x %08x %08x %08x\n", readl(EXYNOS9830_DUMP_GPR_BASE),
						readl(EXYNOS9830_DUMP_GPR_BASE + 0x4),
						readl(EXYNOS9830_DUMP_GPR_BASE + 0x8),
						readl(EXYNOS9830_DUMP_GPR_BASE + 0xC));
	printf("ACPM R4~R7  : %08x %08x %08x %08x\n", readl(EXYNOS9830_DUMP_GPR_BASE + 0x10),
						readl(EXYNOS9830_DUMP_GPR_BASE + 0x14),
						readl(EXYNOS9830_DUMP_GPR_BASE + 0x18),
						readl(EXYNOS9830_DUMP_GPR_BASE + 0x1C));
	printf("ACPM R8~R11 : %08x %08x %08x %08x\n", readl(EXYNOS9830_DUMP_GPR_BASE + 0x20),
						readl(EXYNOS9830_DUMP_GPR_BASE + 0x24),
						readl(EXYNOS9830_DUMP_GPR_BASE + 0x28),
						readl(EXYNOS9830_DUMP_GPR_BASE + 0x2C));
	printf("ACPM R12~R15: %08x %08x %08x %08x\n", readl(EXYNOS9830_DUMP_GPR_BASE + 0x30),
						readl(EXYNOS9830_DUMP_GPR_BASE + 0x34),
						readl(EXYNOS9830_DUMP_GPR_BASE + 0x38),
						readl(EXYNOS9830_DUMP_GPR_BASE + 0x3C));
}

void display_flexpmu_dbg(void)
{
	unsigned int i;
	unsigned int rst_stat = readl(EXYNOS9830_POWER_RST_STAT);

	if (rst_stat == (PORESET | PIN_RESET)) {
		printf("%sCold boot.\n", FLEXPMU_DBG_LOG);
		return;
	}

	for (i = 0; i < ARRAY_SIZE(flexpmu_dbg); i++) {
		flexpmu_dbg[i].u32_data[0] =
			readl(EXYNOS9830_FLEXPMU_DBG_BASE + (i * FLEXPMU_DBG_ENTRY_SIZE) + 0x8);
		flexpmu_dbg[i].u32_data[1] =
			readl(EXYNOS9830_FLEXPMU_DBG_BASE + (i * FLEXPMU_DBG_ENTRY_SIZE) + 0xc);
	}

	print_pmudbg_registers();
	print_running_sequencer(&flexpmu_dbg[RUNNING_SEQUENCER]);
	print_powermode_count(&flexpmu_dbg[APSOC_SEQ_TOTAL_COUNT]);
	print_powermode_count(&flexpmu_dbg[MIF_SEQ_TOTAL_COUNT]);
	print_powermode_count(&flexpmu_dbg[APSOC_SLEEP_SEQ_COUNT]);
	print_powermode_count(&flexpmu_dbg[MIF_SLEEP_SEQ_COUNT]);
	print_powermode_count(&flexpmu_dbg[APSOC_SICD_SEQ_COUNT]);
	print_powermode_count(&flexpmu_dbg[MIF_SICD_SEQ_COUNT]);
	print_last_powermode(&flexpmu_dbg[CURRENT_POWER_MODE]);
	print_cpu_seq_status(&flexpmu_dbg[CPU_SEQ_STATUS]);

	if (!flexpmu_dbg[RUNNING_SEQUENCER].u32_data[0] && !flexpmu_dbg[RUNNING_SEQUENCER].u32_data[1])
		goto acpm_gpr;

	print_pwr_seq_status(&flexpmu_dbg[PWR_SEQ_STATUS]);
	print_running_sequencer(&flexpmu_dbg[RUNNING_SEQUENCER]);
	print_sequencer_info(&flexpmu_dbg[SEQUENCER_INFO]);

	print_tz_setting_status(&flexpmu_dbg[TZ_SETTING_STATUS]);
	print_tz_input_output(&flexpmu_dbg[TZ_INPUT_OUTPUT]);
	print_dram_init_setting_status(&flexpmu_dbg[DRAM_INIT_STATUS]);
	print_dram_init_input_output(&flexpmu_dbg[DRAM_INIT_INPUT_OUTPUT]);
	print_sci_init_setting_status(&flexpmu_dbg[SCI_INIT_STATUS]);
	print_sci_init_input_output(&flexpmu_dbg[SCI_INIT_INPUT_OUTPUT]);

	print_sw_flag(&flexpmu_dbg[SW_FLAG]);
	print_last_powermode(&flexpmu_dbg[CURRENT_POWER_MODE]);

acpm_gpr:
	print_acpm_last_gpr();
}
