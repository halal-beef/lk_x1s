#include <debug.h>
#include <reg.h>
#include <app.h>
#include <lib/console.h>
#include <platform/sfr.h>
#include <platform/charger.h>
#include <platform/fastboot.h>
#include <dev/boot.h>

int cmd_boot(int argc, const cmd_args *argv);

static void exynos_boot_task(const struct app_descriptor *app, void *args)
{
	unsigned int rst_stat = readl(EXYNOS9610_POWER_RST_STAT);

	printf("RST_STAT: 0x%x\n", rst_stat);

	if (*(unsigned int *)DRAM_BASE != 0xabcdef) {
		printf("Running on DRAM by TRACE32: skip auto booting\n");
		return;
	}

	if (!is_first_boot() || (rst_stat & (WARM_RESET | LITTLE_WDT_RESET | BIG_WDT_RESET)) ||
		((readl(CONFIG_RAMDUMP_SCRATCH) == CONFIG_RAMDUMP_MODE) && get_charger_mode() == 0))
		do_fastboot(0, 0);
	else
		cmd_boot(0, 0);

	return;
}

APP_START(exynos_boot)
	.entry = exynos_boot_task,
	.flags = 0,
APP_END
