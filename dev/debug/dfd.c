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
#include <dev/debug/dss.h>
#include <platform/sizes.h>
#include <platform/sfr.h>
#include <platform/smc.h>
#include <platform/delay.h>
#include <platform/dfd.h>

unsigned int dump_en_before_reset = 0xFFFFFFFF;

unsigned int dfd_get_dump_en_before_reset(void)
{
	static bool once = false;
	if ((dump_en_before_reset == 0xFFFFFFFF) && !once) {
		dump_en_before_reset = readl(EXYNOS_PMU_RESET_SEQ_CONFIG);
		once = true;
	}

	return dump_en_before_reset;
}

bool is_cache_disable_mode(void)
{
	unsigned int rst_stat = readl(EXYNOS_PMU_RST_STAT);
	unsigned int dump_en = dfd_get_dump_en_before_reset();

	return ((rst_stat & (WARM_RESET | LITTLE_WDT_RESET)) && (dump_en & DUMP_EN));
}

static inline void pmu_set_bit_atomic(u32 offset, u32 bit)
{
        writel(bit, EXYNOS_PMU_BASE + (offset | 0xc000));
}

static inline void pmu_clr_bit_atomic(u32 offset, u32 bit)
{
        writel(bit, EXYNOS_PMU_BASE + (offset | 0x8000));
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
		printf("retboot reason: 0x%x - Kernel PANIC\n", ret);
		print_lcd_update(FONT_YELLOW, FONT_RED, "0x%x - Kernel PANIC", ret);
		dfd_display_panic_reason();
		break;
	case RAMDUMP_SIGN_NORMAL_REBOOT:
		printf("retboot reason: 0x%x - User Reboot(S/W Reboot)\n", ret);
		print_lcd_update(FONT_WHITE, FONT_BLACK, "0x%x - User Reboot(S/W Reboot)", ret);
		break;
	case RAMDUMP_SIGN_BL_REBOOT:
		printf("0x%x - BL Reboot(S/W Reboot)\n", ret);
		print_lcd_update(FONT_WHITE, FONT_BLACK, "0x%x - BL Reboot(S/W Reboot)", ret);
		break;
	case RAMDUMP_SIGN_FORCE_REBOOT:
		printf("retboot reason: 0x%x - Forced Reboot(S/W Reboot)\n", ret);
		print_lcd_update(FONT_WHITE, FONT_BLUE, "0x%x - Forced Reboot(S/W Reboot)", ret);
		break;
	case RAMDUMP_SIGN_SAFE_FAULT:
		printf("retboot reason: 0x%x - Safe Kernel PANIC\n", ret);
		print_lcd_update(FONT_YELLOW, FONT_RED, "0x%x - Safe Kernel PANIC", ret);
		dfd_display_panic_reason();
		break;
	case RAMDUMP_SIGN_RESET:
	default:
		printf("retboot reason: 0x%x - Power/Emergency Reset\n", ret);
		print_lcd_update(FONT_YELLOW, FONT_RED, "0x%x - Power/Emergency Reset", ret);
		break;
	}
}

void dfd_display_core_stat(void)
{
	int val;
	u32 ret;

	printf("Core stat at previous(IRAM)\n");
	for (val = 0; val < NR_CPUS; val++) {
		ret = readl(CORE_STAT + (val * REG_OFFSET));
		printf("Core%d: ", val);
		switch (ret) {
			case RUNNING:
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
				printf("Unknown: 0x%x\n", ret);
				break;
		}
	}

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

void dfd_set_dump_en(int en)
{
	if (en)
		pmu_set_bit_atomic(RESET_SEQUENCER_OFFSET, DUMP_EN_BIT);
	else
		pmu_clr_bit_atomic(RESET_SEQUENCER_OFFSET, DUMP_EN_BIT);
}

__WEAK void dfd_soc_run_post_processing()
{
	/* WARNING : NOT MODIFY THIS FUNCTION
	 * please implementate on the platform.
	 */
}

__WEAK void dfd_soc_get_dbgc_version()
{
	/* WARNING : NOT MODIFY THIS FUNCTION
	 * please implementate on the platform.
	 */
}

void dfd_run_post_processing(void)
{
	dfd_soc_run_post_processing();
}

void dfd_get_dbgc_version(void)
{
	dfd_soc_get_dbgc_version();
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
			break;
		}
	}

	/* Update debug_level to reserved region */
	writel(debug_level | DEBUG_LEVEL_PREFIX, CONFIG_RAMDUMP_DEBUG_LEVEL);
	printf("debug level: %s\n", debug_level_val[debug_level]);
}
