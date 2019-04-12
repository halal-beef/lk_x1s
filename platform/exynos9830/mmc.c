#include <dev/mmc.h>
#include <dev/dw_mmc.h>
#include <dev/speedy_multi.h>
#include <dev/boot.h>
#include <platform/delay.h>
#include <platform/sfr.h>
#include <platform/pmic_s2mps_19_22.h>

#define MUX_CLKCMU_HSI1_MMC_CARD		0x1A33108C
#define MUX_CLKCMU_HSI1_MMC_CARD_USER		0x13000610
#define CLK_CON_DIV_CLKCMU_HSI1_MMC_CARD	0x1A331884

#define MASK_DIVRATIO				(0x1FF)

#define GPF1_CON				0x13040020
#define GPF1_PUD				0x13040028
#define GPF1_DRV				0x1304002C

#define USE_SD					1

/* power on/off mmc channel */
void mmc_power_set(unsigned int channel, unsigned int enable)
{
	unsigned char reg;

	switch (channel) {
		case 0:
			return;
		case 1:
			return;
		case 2:
			speedy_read(CONFIG_SPEEDY0_BASE, S2MPS19_PM_ADDR, S2MPS19_PM_LDO2M_CTRL, &reg);
			if (enable)
				reg |= S2MPS_OUTPUT_ON_NORMAL;
			else
				reg &= ~S2MPS_OUTPUT_ON_NORMAL;
			speedy_write(CONFIG_SPEEDY0_BASE, S2MPS19_PM_ADDR, S2MPS19_PM_LDO2M_CTRL, reg);
			speedy_read(CONFIG_SPEEDY0_BASE, S2MPS19_PM_ADDR, S2MPS19_PM_LDO15M_CTRL, &reg);
			if (enable)
				reg |= S2MPS_OUTPUT_ON_NORMAL;
			else
				reg &= ~S2MPS_OUTPUT_ON_NORMAL;
			speedy_write(CONFIG_SPEEDY0_BASE, S2MPS19_PM_ADDR, S2MPS19_PM_LDO15M_CTRL, reg);
			return;
		case 3:
			return;
		default :
			return;
	}
}

/* set gpio for mmc channel */
void mmc_gpio_set(unsigned int channel, unsigned int enable)
{
	u32 reg;

	switch (channel) {
		case 0:
			return;
		case 1:
			return;
		case 2:
			reg = *(volatile u32 *)GPF1_CON;
			reg &= ~(0x00FFFFFF);
			reg |= 0x00222222;
			*(volatile u32 *)GPF1_CON = reg;

			reg = *(volatile u32 *)GPF1_PUD;
			reg &= ~(0x00FFFFFF);
			reg |= 0x00333330;
			*(volatile u32 *)GPF1_PUD = reg;

			reg = *(volatile u32 *)GPF1_DRV;
			reg &= ~(0x00FFFFFF);
			reg |= 0x00222222;
			*(volatile u32 *)GPF1_DRV = reg;
			return;
		case 3:
			return;
		default :
			return;
	}
}

/* set default clock for mmc channel */
void mmc_clock_set(unsigned int channel, unsigned int enable)
{
	u32 reg;
	switch (channel) {
		case 0:
			return;
		case 1:
			return;
		case 2:
			reg = *(volatile u32 *)CLK_CON_DIV_CLKCMU_HSI1_MMC_CARD;
			reg &= ~MASK_DIVRATIO;
			reg |= 0x0;
			*(volatile u32 *)CLK_CON_DIV_CLKCMU_HSI1_MMC_CARD = reg;
			return;
		case 3:
			return;
		default :
			return;
	}
}

/*  Establish a board environment to operate the mmc device. */
int mmc_board_init(struct mmc *mmc, unsigned int channel)
{
	mmc->channel = channel;
	switch (channel) {
		case 0:
			/* eMMC device turned only when it is boot device */
			if (get_boot_device() == BOOT_EMMC) {
				printf("get_boot_device() == BOOT_EMMC\n");
				break;
			}
			return -1;
		case 1:
			/* SDIO not used */
			return -1;
		case 2:
#ifdef USE_SD
			/* The SD card can be turned on regardless of the boot device. */
			mmc_power_set(channel, 1);
			mmc_gpio_set(channel, 1);
			mmc_clock_set(channel, 1);
			dwmci_init(mmc, channel);
			break;
#else
			return -1;
#endif
		case 3:
			return -1;
		default:
			return -1;
	}
	return 0;
}

/*  Establish a board environment to operate the mmc device when mmc reinitialize. */
int mmc_board_reinit(struct mmc *mmc)
{
	unsigned int channel = mmc->channel;
	switch (channel) {
		case 0:
			return -1;
		case 1:
			return -1;
		case 2:
			/* power off and power on */
			mmc_power_set(channel, 0);
			mdelay(1);
			mmc_power_set(channel, 1);
			mmc_gpio_set(channel, 1);
			mmc_clock_set(channel, 1);
			break;
		case 3:
			return -1;
		default:
			break;
	}
	return 0;
}
