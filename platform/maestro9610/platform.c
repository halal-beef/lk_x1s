#include <reg.h>
#include "uart_simple.h"
#include <dev/ufs.h>
#include <dev/boot.h>
#include <pit.h>
#include <platform/sfr.h>
#include <platform/smc.h>
#include <platform/speedy.h>
#include <platform/pmic_s2mpu09.h>
#include <platform/fg_s2mu004.h>
#include <platform/if_pmic_s2mu004.h>
#include <platform/tmu.h>

#include <lib/font_display.h>
#include <lib/logo_display.h>
#include <target/dpu_config.h>
#include <stdio.h>

void speedy_gpio_init(void);

unsigned int s5p_chip_id[4] = {0x0, 0x0, 0x0, 0x0};
unsigned int charger_mode = 0;
unsigned int board_id = 0;
unsigned int board_rev = 0;

unsigned int get_charger_mode(void)
{
	return charger_mode;
}

static void read_chip_id(void)
{
	s5p_chip_id[0] = readl(EXYNOS9610_PRO_ID + CHIPID0_OFFSET);
	s5p_chip_id[1] = readl(EXYNOS9610_PRO_ID + CHIPID1_OFFSET) & 0xFFFF;
}

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

static int check_charger_connect(void)
{
	unsigned char read_pwronsrc = 0;
	unsigned int rst_stat = readl(EXYNOS9610_POWER_RST_STAT);

	if (rst_stat == PIN_RESET) {
		speedy_init();
		speedy_read(S2MPU09_PM_ADDR, S2MPU09_PM_PWRONSRC, &read_pwronsrc);

		/* Check USB or TA connected and PWRONSRC(USB)  */
		if(read_pwronsrc & ACOK)
			charger_mode = 1;
		else
			charger_mode = 0;
	} else {
		charger_mode = 0;
	}

	return 0;
}

#ifdef CONFIG_EXYNOS_BOOTLOADER_DISPLAY
extern int display_drv_init(void);
void display_panel_init(void);

static void initialize_fbs(void)
{
	memset(CONFIG_DISPLAY_LOGO_BASE_ADDRESS, 0, LCD_WIDTH * LCD_HEIGHT * 4);
	memset(CONFIG_DISPLAY_FONT_BASE_ADDRESS, 0, LCD_WIDTH * LCD_HEIGHT * 4);
}
#endif

void platform_early_init(void)
{
	unsigned int rst_stat = readl(EXYNOS9610_POWER_RST_STAT);

	read_chip_id();

	speedy_gpio_init();
#ifdef CONFIG_EXYNOS_BOOTLOADER_DISPLAY
	display_panel_init();
	initialize_fbs();
#endif
	set_first_boot_device_info();

	if (is_first_boot() && !(rst_stat & (WARM_RESET | LITTLE_WDT_RESET)))
		muic_sw_uart();
	uart_test_function();
}

void platform_init(void)
{
	pmic_init();
	fg_init_s2mu004();
	check_charger_connect();
	display_pmic_info_s2mpu09();

	load_secure_payload();

	ufs_init(2);
	ufs_set_configuration_descriptor();
	pit_init();

#ifdef CONFIG_EXYNOS_BOOTLOADER_DISPLAY
	display_drv_init();
	show_boot_logo();
#endif
	display_tmu_info();
	display_trip_info();
}
