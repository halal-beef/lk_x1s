#include <dev/mmc.h>
#include <dev/dw_mmc.h>
#include <dev/boot.h>
#include <platform/delay.h>
#include <platform/sfr.h>
#include <dev/lk_acpm_ipc.h>
#include <target/pmic.h>
#if 0
#include <dev/speedy_multi.h>
#include <dev/pmic_s2mps_19_22.h>
#endif

/* eMMC CLK */
/* 800mhz PLL SHAED 0 */
#define CLK_CON_MUX_CLKCMU_CORE_MMC_EMBD	0x120e101c
#define CLK_CON_DIV_CLKCMU_CORE_MMC_EMBD	0x120e1828

/* SDcard */
#define CLK_CON_MUX_CLK_CMU_HSI_MMC_CARD	0x120e1040
#define CLK_CON_DIV_CLK_CMU_HSI_MMC_CARD	0x120e184c

#define MASK_DIVRATIO				(0x1FF)

/* eMMC GPIO_CORE */
#define GPF0_CON				0x12070000
#define GPF0_DAT				0x12070004
#define GPF0_PUD				0x12070008
#define GFF0_DRV				0x1207000c

#define GPF1_CON				0x12070020
#define GPF1_PUD				0x12070028
#define GPF1_DRV				0x1207002c

/* SD GPIO_HSI */
#define GPF2_CON				0x13430000
#define GPF2_PUD				0x13430008
#define GPF2_DRV				0x1343000c
/* power on/off mmc channel */
void mmc_power_set(unsigned int channel, unsigned int enable)
{
	unsigned char reg;

	switch (channel) {
		case 0:
			/* Enable eMMC power */
			i3c_read(0, S2MPU12_PM_ADDR, S2MPU12_PM_LDO2_CTRL, &reg);
			if (enable)
				reg |= 0xC0;
			else
				reg &= 0xC0;
			i3c_write(0, S2MPU12_PM_ADDR, S2MPU12_PM_LDO2_CTRL, reg);

			i3c_read(0, S2MPU12_PM_ADDR, S2MPU12_PM_LDO23_CTRL, &reg);
			if (enable)
				reg |= 0xC0;
			else
				reg &= 0xC0;
			i3c_write(0, S2MPU12_PM_ADDR, S2MPU12_PM_LDO23_CTRL, reg);
			return;
		case 1:
			return;
		case 2:
#if 0
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
#endif
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
			reg = *(volatile u32 *)GPF0_CON;
			reg &= ~(0x00FFFF);
			reg |= 0x2222;
			*(volatile u32 *)GPF0_CON = reg;

			reg = *(volatile u32 *)GPF0_PUD;
			reg &= ~(0x00FFFF);
			reg |= 0x0130;
			*(volatile u32 *)GPF1_PUD = reg;

			reg = *(volatile u32 *)GPF1_CON;
			reg &= ~(0xFFFFFFFF);
			reg |= 0x22222222;
			*(volatile u32 *)GPF1_CON = reg;

			reg = *(volatile u32 *)GPF1_PUD;
			reg &= ~(0xFFFFFFFF);
			reg |= 0x33333333;
			*(volatile u32 *)GPF1_PUD = reg;

			return;
		case 1:
			return;
		case 2:
			reg = *(volatile u32 *)GPF2_CON;
			reg &= ~(0x00FFFFFF);
			reg |= 0x00222222;
			*(volatile u32 *)GPF2_CON = reg;

			reg = *(volatile u32 *)GPF2_PUD;
			reg &= ~(0x00FFFFFF);
			reg |= 0x00333330;
			*(volatile u32 *)GPF2_PUD = reg;

			reg = *(volatile u32 *)GPF1_DRV;
			reg &= ~(0x00FFFFFF);
			reg |= 0x00222222;
			*(volatile u32 *)GPF2_DRV = reg;
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
			reg = *(volatile u32 *)CLK_CON_DIV_CLKCMU_CORE_MMC_EMBD;
			reg &= ~MASK_DIVRATIO;
			reg |= 0x1;
			*(volatile u32 *)CLK_CON_DIV_CLKCMU_CORE_MMC_EMBD = reg;

			reg = *(volatile u32 *)CLK_CON_MUX_CLKCMU_CORE_MMC_EMBD;
			reg &= ~0x3;
			reg |= 0x1;
			*(volatile u32 *)CLK_CON_MUX_CLKCMU_CORE_MMC_EMBD = reg;
			return;
		case 1:
			return;
		case 2:
			reg = *(volatile u32 *)CLK_CON_DIV_CLK_CMU_HSI_MMC_CARD;
			reg &= ~MASK_DIVRATIO;
			reg |= 0x0;
			*(volatile u32 *)CLK_CON_DIV_CLK_CMU_HSI_MMC_CARD = reg;

			reg = *(volatile u32 *)CLK_CON_MUX_CLK_CMU_HSI_MMC_CARD;
			reg &= ~0x3;
			reg |= 0x1;
			*(volatile u32 *)CLK_CON_MUX_CLK_CMU_HSI_MMC_CARD = reg;
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
			mmc_power_set(channel, 1);
			mmc_gpio_set(channel, 1);
			mmc_clock_set(channel, 1);
			dwmci_init(mmc, channel);
			break;

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

void mmc_emmc_reset(struct mmc *mmc)
{
#if 0
	u32 reg;
	reg = *(volatile u32 *)GPF0_DAT;
	reg &= ~(0x1 << 12);
	*(volatile u32 *)GPF0_CON = reg;

	delay(1);

	reg = *(volatile u32 *)GPF0_DAT;
	reg |= (0x1 << 12);
	*(volatile u32 *)GPF0_CON = reg;
#endif
}

/*  Establish a board environment to operate the mmc device when mmc reinitialize. */
int mmc_board_reinit(struct mmc *mmc)
{
	unsigned int channel = mmc->channel;
	switch (channel) {
		case 0:
			/* power off and power on */
			mmc_emmc_reset(mmc);
			mmc_gpio_set(channel, 1);
			mmc_clock_set(channel, 1);
			return -1;
		case 1:
			return -1;
		case 2:
#if 0
			/* power off and power on */
			mmc_power_set(channel, 0);
			mdelay(1);
			mmc_power_set(channel, 1);
			mmc_gpio_set(channel, 1);
			mmc_clock_set(channel, 1);
#endif
			break;
		case 3:
			return -1;
		default:
			break;
	}
	return 0;
}
