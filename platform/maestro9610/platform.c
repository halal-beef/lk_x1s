#include <reg.h>
#include "uart_simple.h"
#include <dev/ufs.h>
#include <dev/boot.h>
#include <pit.h>
#include <platform/sfr.h>
#include <platform/smc.h>
#include <platform/pmic_s2mpu09.h>
#include <platform/if_pmic_s2mu004.h>

void speedy_gpio_init(void);
void display_gpio_init(void);

static void load_secure_payload(void)
{
	unsigned long ret = 0;
	unsigned int boot_dev = 0;
	unsigned int dfd_en = readl(EXYNOS9610_POWER_RESET_SEQUENCER_CONFIGURATION);
	unsigned int rst_stat = readl(EXYNOS9610_POWER_RST_STAT);

	if (*(unsigned int *)DRAM_BASE != 0xabcdef) {
		printf("Running on DRAM by TRACE32: skip load_secure_payload()\n");
	} else {
		if (is_first_boot()) {
			boot_dev = readl(EXYNOS9610_POWER_INFORM3);

			/*
			 * In case WARM Reset/Watchdog Reset and DumpGPR is enabled,
			 * Secure payload doesn't have to be loaded.
			 */
			if (!((rst_stat & (WARM_RESET | LITTLE_WDT_RESET)) &&
				dfd_en & EXYNOS9610_EDPCSR_DUMP_EN)) {
				ret = load_sp_image(boot_dev);
				if (ret)
					/*
					 * 0xFEED0002 : Signature check fail
					 * 0xFEED0020 : Anti-rollback check fail
					 */
					printf("Fail to load Secure Payload!! [ret = 0x%X]\n", ret);
				else
					printf("Secure Payload is loaded successfully!\n");
			}

			/*
			 * If boot device is eMMC, emmc_endbootop() should be
			 * implemented because secure payload is the last image
			 * in boot partition.
			 */
			if (boot_dev == BOOT_EMMC)
				emmc_endbootop();
		} else {
			/* second_boot = 1; */
		}
	}
}

void platform_early_init(void)
{
	speedy_gpio_init();
	display_gpio_init();

	set_first_boot_device_info();

	if (is_first_boot())
		muic_sw_uart();
	uart_test_function();
}

void platform_init(void)
{
	pmic_init();
	display_pmic_info_s2mpu09();

	load_secure_payload();

	ufs_init(2);
	ufs_set_configuration_descriptor();
	pit_init();
}
