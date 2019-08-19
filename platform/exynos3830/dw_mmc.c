#include <dev/mmc.h>
#include <dev/dw_mmc.h>
#include <platform/sfr.h>
#include <platform/delay.h>
#if 0
#include <dev/pmic_s2mps_19_22.h>
#include <dev/speedy_multi.h>
#endif

#define MMC_EMBD_BASE				0x12100000
#define MMC_CARD_BASE				0x13500000

#define SDR_CH0                 0x03020000
#define DDR_CH0                 0x03030002

#define SDR_CH2                 0x03020001
#define DDR_CH2                 0x03030002

#define SDR_CH4                 0x00010001
#define DDR_CH4                 0x00010001

#define PHASE_DIVIDER_DEFAULT			4
#define FIFO_DEPTH_DEAFULT			0x20

#define MUX_CLKCMU_HSI1_MMC_CARD		0x1A33108C
#define MUX_CLKCMU_HSI1_MMC_CARD_USER		0x13000610
#define CLK_CON_DIV_CLKCMU_HSI1_MMC_CARD	0x1A331884

/* 800mhz PLL SHAED 0 */
#define CLK_CON_MUX_CLKCMU_CORE_MMC_EMBD	0x120e101c
#define CLK_CON_DIV_CLKCMU_CORE_MMC_EMBD	0x120e1828
#define PLL_CON0_MUX_CLKCMU_CORE_MMC_EMBD_USER	0x12000620

/* SDcard */
#define CLK_CON_MUX_CLK_CMU_HSI_MMC_CARD	0x120e1040
#define CLK_CON_DIV_CLK_CMU_HSI_MMC_CARD	0x120e184c
#define PLL_CON0_MUX_CLKCMU_HSI_MMC_CARD_USER	0x13400610

#define PLL_CON3_PLL_MMC			0x120e010c
#define PLL_CON3_PLL_SHARED0			0x120e014c
#define PLL_CON3_PLL_SHARED1			0x120e018c


/* get clock frequency from pll data */
unsigned long get_pll_clk(int pllreg)
{
	unsigned long r, m, p, s, fout, freq;

	r = readl(pllreg);
	/* MDIV [25:16] */
	m = (r >> 16) & 0x3ff;
	/* PDIV [13:8] */
	p = (r >> 8) & 0x3f;
	/* SDIV [2:0] */
	s = r & 0x7;

	freq = 26000000;

	/* FOUT = (MDIV * FIN) / (PDIV * 2^(SDIV)) */
	fout = (m * freq) / (p * (1 << s));

	return fout;
}

/* eMMC get clock frequency */
static unsigned int mmc_get_clk(void)
{
	u32 reg, div, original;
	reg = *(volatile u32 *)PLL_CON0_MUX_CLKCMU_CORE_MMC_EMBD_USER;
	if ((reg & (1 << 4)) == 0)
		return 26000000;
	else {
		div = *(volatile u32 *)CLK_CON_DIV_CLKCMU_CORE_MMC_EMBD & (0x1ff);
		reg = *(volatile u32 *)CLK_CON_MUX_CLKCMU_CORE_MMC_EMBD;
		switch(reg & 3) {
			case 0:
				original = 26000000;
				break;
			case 1:
				original = get_pll_clk(PLL_CON3_PLL_SHARED0) / 2;
				break;
			case 2:
				original = get_pll_clk(PLL_CON3_PLL_SHARED1) / 2;
				break;
			case 3:
				original = get_pll_clk(PLL_CON3_PLL_SHARED0) / 4;
				break;
			case 4:
				original = get_pll_clk(PLL_CON3_PLL_SHARED1) / 2;
				break;
			case 5:
				original = get_pll_clk(PLL_CON3_PLL_MMC);
				break;
		}
	}
	return original / (div + 1);
}

/* eMMC set clock frequency */
static void mmc_set_clk(unsigned int freq)
{
}

/* SD card get clock frequency */
static unsigned int sd_get_clk(void)
{

	u32 reg, div, original;

	reg = *(volatile u32 *)PLL_CON0_MUX_CLKCMU_HSI_MMC_CARD_USER;
	if ((reg & (1 << 4)) == 0)
		return 26000000;
	else {
		div = *(volatile u32 *)CLK_CON_DIV_CLK_CMU_HSI_MMC_CARD & (0x1ff);
		reg = *(volatile u32 *)CLK_CON_MUX_CLK_CMU_HSI_MMC_CARD;
		switch(reg & 3) {
			case 0:
				original = 26000000;
				break;
			case 1:
				original = get_pll_clk(PLL_CON3_PLL_SHARED0) / 2;
				break;
			case 2:
				original = get_pll_clk(PLL_CON3_PLL_SHARED1) / 2;
				break;
			case 3:
				original = get_pll_clk(PLL_CON3_PLL_SHARED0) / 4;
				break;
			case 4:
				original = get_pll_clk(PLL_CON3_PLL_SHARED1) / 2;
				break;
			case 5:
				original = get_pll_clk(PLL_CON3_PLL_MMC);
				break;
		}
	}
	return original / (div + 1);
}

/* SD card set clock frequency */
static void sd_set_clk(unsigned int freq)
{
}

/* SD card voltage switch 3.3V to 1.8V */
static void sd_voltage_switch(void)
{
#if 0
	unsigned char reg;

	reg = 0;
	speedy_write(CONFIG_SPEEDY0_BASE, S2MPS19_PM_ADDR, S2MPS19_PM_LDO2M_CTRL, reg);
	mdelay(10);
	reg = 0xc0;
	speedy_write(CONFIG_SPEEDY0_BASE, S2MPS19_PM_ADDR, S2MPS19_PM_LDO2M_CTRL, reg);
#endif
}

/* connection call back function and input board data */
int dwmci_board_get_host(struct dw_mci *host, int channel)
{
	switch(channel) {
		case 0 :
			host->ioaddr = (void __iomem *)MMC_EMBD_BASE;
			host->sdr = SDR_CH0;
			host->ddr = DDR_CH0;
			host->phase_devide = PHASE_DIVIDER_DEFAULT;
			host->secure = 0;
			host->mps_secure = 0xCFE2E000;
			host->set_clk = 0x0;
			host->get_clk = mmc_get_clk;
			host->bus_clock = 0;
			host->sd_voltage_switch = 0;;
			host->fifo_depth = FIFO_DEPTH_DEAFULT;
			break;
		case 2 :
			host->ioaddr = (void __iomem *)MMC_CARD_BASE;
			host->sdr = SDR_CH2;
			host->ddr = DDR_CH2;
			host->phase_devide = PHASE_DIVIDER_DEFAULT;
			host->secure = 0;
			host->mps_secure = 0;
			host->set_clk = 0x0;
			host->get_clk = sd_get_clk;
			host->bus_clock = 0;
//			host->sd_voltage_switch = sd_voltage_switch;
			host->fifo_depth = FIFO_DEPTH_DEAFULT;
			break;
		default :
			return -1;
	}
	return 0;
}
