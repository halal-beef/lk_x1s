/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <lk/debug.h>
#include <sys/types.h>
#include <platform/sfr.h>
#include <platform/delay.h>
#include <dev/pmic_s2mps_19_22.h>
#include <dev/speedy_multi.h>
#include <lib/fastboot.h>

extern unsigned int board_id;
extern unsigned int board_rev;

void pmic_init(void)
{
	unsigned char reg;

	speedy_init(CONFIG_SPEEDY0_BASE);
	speedy_init(CONFIG_SPEEDY1_BASE);

	/* Disable Manual Reset */
	speedy_read(CONFIG_SPEEDY0_BASE, S2MPS19_PM_ADDR, S2MPS19_PM_CTRL1, &reg);
	reg &= ~MRSTB_EN;
	speedy_write(CONFIG_SPEEDY0_BASE, S2MPS19_PM_ADDR, S2MPS19_PM_CTRL1, reg);

	/* Enable Warm Reset */
	speedy_read(CONFIG_SPEEDY0_BASE, S2MPS19_PM_ADDR, S2MPS19_PM_CTRL3, &reg);
	reg |= WRSTEN;
	/* HACK
	 * In case of universal9830 rev05, it can be warm reset by only the volume down key.
	 */
	if (board_id == 0x10 && board_rev == 0x5)
		reg |= MRSEL;
	speedy_write(CONFIG_SPEEDY0_BASE, S2MPS19_PM_ADDR, S2MPS19_PM_CTRL3, reg);

	/* LCD power */
	speedy_read(CONFIG_SPEEDY0_BASE, S2MPS19_PM_ADDR, S2MPS19_PM_LDO3M_CTRL, &reg);
	reg |= S2MPS_OUTPUT_ON_NORMAL;
	speedy_write(CONFIG_SPEEDY0_BASE, S2MPS19_PM_ADDR, S2MPS19_PM_LDO3M_CTRL, reg);

	speedy_read(CONFIG_SPEEDY0_BASE, S2MPS19_PM_ADDR, S2MPS19_PM_LDO4M_CTRL, &reg);
	reg |= S2MPS_OUTPUT_ON_NORMAL;
	speedy_write(CONFIG_SPEEDY0_BASE, S2MPS19_PM_ADDR, S2MPS19_PM_LDO4M_CTRL, reg);

	speedy_read(CONFIG_SPEEDY0_BASE, S2MPS19_PM_ADDR, S2MPS19_PM_LDO9M_CTRL, &reg);
	reg |= S2MPS_OUTPUT_ON_NORMAL;
	speedy_write(CONFIG_SPEEDY0_BASE, S2MPS19_PM_ADDR, S2MPS19_PM_LDO9M_CTRL, reg);

	speedy_read(CONFIG_SPEEDY1_BASE, S2MPS22_PM_ADDR, S2MPS22_PM_LDO4S_CTRL, &reg);
	reg |= S2MPS_OUTPUT_ON_NORMAL;
	speedy_write(CONFIG_SPEEDY1_BASE, S2MPS22_PM_ADDR, S2MPS22_PM_LDO4S_CTRL, reg);

	/* ICEN enable for PB02 & PB03 */
	speedy_read(CONFIG_SPEEDY0_BASE, S2MPS19_PM_ADDR, S2MPS19_PM_LDO16M_CTRL, &reg);
	reg |= S2MPS_OUTPUT_ON_NORMAL;
	speedy_write(CONFIG_SPEEDY0_BASE, S2MPS19_PM_ADDR, S2MPS19_PM_LDO16M_CTRL, reg);

	/* LDO 10, 11, 12 On for USB */
	speedy_read(CONFIG_SPEEDY0_BASE, S2MPS19_PM_ADDR, S2MPS19_PM_LDO10M_CTRL, &reg);
	reg |= S2MPS_OUTPUT_ON_TCXO;
	speedy_write(CONFIG_SPEEDY0_BASE, S2MPS19_PM_ADDR, S2MPS19_PM_LDO10M_CTRL, reg);

	speedy_read(CONFIG_SPEEDY0_BASE, S2MPS19_PM_ADDR, S2MPS19_PM_LDO11M_CTRL, &reg);
	reg |= S2MPS_OUTPUT_ON_TCXO;
	speedy_write(CONFIG_SPEEDY0_BASE, S2MPS19_PM_ADDR, S2MPS19_PM_LDO11M_CTRL, reg);

	speedy_read(CONFIG_SPEEDY0_BASE, S2MPS19_PM_ADDR, S2MPS19_PM_LDO12M_CTRL, &reg);
	reg |= S2MPS_OUTPUT_ON_TCXO;
	speedy_write(CONFIG_SPEEDY0_BASE, S2MPS19_PM_ADDR, S2MPS19_PM_LDO12M_CTRL, reg);

	/* Enable WTSR */
	speedy_read(CONFIG_SPEEDY0_BASE, S2MPS19_RTC_ADDR, S2MPS19_RTC_WTSR_SMPL, &reg);
	reg = (reg & 0xB8) | 0x43;
	speedy_write(CONFIG_SPEEDY0_BASE, S2MPS19_RTC_ADDR, S2MPS19_RTC_WTSR_SMPL, reg);
}

void display_pmic_info(void)
{
	unsigned char reg;
	unsigned char read_pwronsrc, read_int1, read_int2, read_wtsr_smpl;

	speedy_read(CONFIG_SPEEDY0_BASE, S2MPS19_PM_ADDR, S2MPS19_PM_INT1, &read_int1);
	printf("S2MPS19_PM_INT1: 0x%x\n", read_int1);

	speedy_read(CONFIG_SPEEDY0_BASE, S2MPS19_PM_ADDR, S2MPS19_PM_INT2, &read_int2);
	printf("S2MPS19_PM_INT2: 0x%x\n", read_int2);

	speedy_read(CONFIG_SPEEDY0_BASE, S2MPS19_PM_ADDR, S2MPS19_PM_PWRONSRC, &read_pwronsrc);
	printf("S2MPS19_PM_PWRONSRC: 0x%x\n", read_pwronsrc);

	speedy_read(CONFIG_SPEEDY0_BASE, S2MPS19_PM_ADDR, S2MPS19_PM_OFFSRC, &reg);
	printf("S2MPS19_PM_OFFSRC: 0x%x\n", reg);

	speedy_read(CONFIG_SPEEDY0_BASE, S2MPS19_RTC_ADDR, S2MPS19_RTC_WTSR_SMPL, &read_wtsr_smpl);
	printf("S2MPS19_RTC_WTSR_SMPL: 0x%x\n", read_wtsr_smpl);

	if ((read_pwronsrc & (1 << 7)) && (read_int2 & (1 << 5)) && !(read_int1 & (1 << 7))){
		set_do_fastboot(PMIC_WTSR);
		printf("WTSR detected\n");
	}
	else if ((read_pwronsrc & (1 << 6)) && (read_int2 & (1 << 3)) && (read_wtsr_smpl & (1 << 7))) {
		set_do_fastboot(PMIC_SMPL);
		printf("SMPL detected\n");
	}

	speedy_read(CONFIG_SPEEDY0_BASE, S2MPS19_PM_ADDR, S2MPS19_PM_CTRL1, &reg);
	printf("S2MPS19_PM_CTRL1: 0x%x\n", reg);

	speedy_read(CONFIG_SPEEDY0_BASE, S2MPS19_PM_ADDR, S2MPS19_PM_CTRL3, &reg);
	printf("S2MPS19_PM_CTRL3: 0x%x\n", reg);

	speedy_read(CONFIG_SPEEDY0_BASE, S2MPS19_PM_ADDR, S2MPS19_PM_RTC_BUF, &reg);
	printf("S2MPS19_PM_RTC_BUF: 0x%x\n", reg);

	speedy_read(CONFIG_SPEEDY1_BASE, S2MPS22_PM_ADDR, S2MPS22_PM_CTRL1, &reg);
	printf("S2MPS22_PM_CTRL1: 0x%x\n", reg);

	speedy_read(CONFIG_SPEEDY0_BASE, S2MPS19_PM_ADDR, S2MPS19_PM_LDO3M_CTRL, &reg);
	printf("S2MPS19_PM_LDO3M_CTRL: 0x%x\n", reg);

	speedy_read(CONFIG_SPEEDY0_BASE, S2MPS19_PM_ADDR, S2MPS19_PM_LDO4M_CTRL, &reg);
	printf("S2MPS19_PM_LDO4M_CTRL: 0x%x\n", reg);

	speedy_read(CONFIG_SPEEDY0_BASE, S2MPS19_PM_ADDR, S2MPS19_PM_LDO9M_CTRL, &reg);
	printf("S2MPS19_PM_LDO9M_CTRL: 0x%x\n", reg);

	speedy_read(CONFIG_SPEEDY1_BASE, S2MPS22_PM_ADDR, S2MPS22_PM_LDO4S_CTRL, &reg);
	printf("S2MPS22_PM_LDO4S_CTRL: 0x%x\n", reg);

	speedy_read(CONFIG_SPEEDY0_BASE, S2MPS19_PM_ADDR, S2MPS19_PM_RTC_BUF, &reg);
	printf("S2MPS19_PM_RTC_BUF: 0x%x\n", reg);

	display_pmic_rtc_time();
}

void display_pmic_rtc_time(void)
{
	int i;
	u8 tmp;
	u8 time[NR_PMIC_RTC_CNT_REGS];

	speedy_read(CONFIG_SPEEDY0_BASE, S2MPS19_RTC_ADDR, S2MPS19_RTC_UPDATE, &tmp);
	tmp |= 0x1;
	speedy_write(CONFIG_SPEEDY0_BASE, S2MPS19_RTC_ADDR, S2MPS19_RTC_UPDATE, tmp);
	udelay(40);

	for (i = 0; i < NR_PMIC_RTC_CNT_REGS; i++)
		speedy_read(CONFIG_SPEEDY0_BASE, S2MPS19_RTC_ADDR, (S2MPS19_RTC_SEC + i), &time[i]);

	printf("RTC TIME: %d-%02d-%02d %02d:%02d:%02d(0x%02x)%s\n",
	       time[PMIC_RTC_YEAR] + 2000, time[PMIC_RTC_MONTH],
	       time[PMIC_RTC_DATE], time[PMIC_RTC_HOUR] & 0x1f, time[PMIC_RTC_MIN],
	       time[PMIC_RTC_SEC], time[PMIC_RTC_WEEK],
	       time[PMIC_RTC_HOUR] & (1 << 6) ? "PM" : "AM");
}

int get_pmic_rtc_time(char *buf)
{
	int i;
	u8 tmp;
	u8 time[NR_PMIC_RTC_CNT_REGS];

	speedy_read(CONFIG_SPEEDY0_BASE, S2MPS19_RTC_ADDR, S2MPS19_RTC_UPDATE, &tmp);
	tmp |= 0x1;
	speedy_write(CONFIG_SPEEDY0_BASE, S2MPS19_RTC_ADDR, S2MPS19_RTC_UPDATE, tmp);
	udelay(40);


	for (i = 0; i < NR_PMIC_RTC_CNT_REGS; i++)
		speedy_read(CONFIG_SPEEDY0_BASE, S2MPS19_RTC_ADDR, (S2MPS19_RTC_SEC + i), &time[i]);

	printf("RTC TIME: %d-%02d-%02d %02d:%02d:%02d(0x%02x)%s\n",
	       time[PMIC_RTC_YEAR] + 2000, time[PMIC_RTC_MONTH],
	       time[PMIC_RTC_DATE], time[PMIC_RTC_HOUR] & 0x1f, time[PMIC_RTC_MIN],
	       time[PMIC_RTC_SEC], time[PMIC_RTC_WEEK],
	       time[PMIC_RTC_HOUR] & (1 << 6) ? "PM" : "AM");

	sprintf(buf, "%04d%02d%02d%02d%02d%02d%s",
		time[PMIC_RTC_YEAR] + 2000, time[PMIC_RTC_MONTH],
		time[PMIC_RTC_DATE], time[PMIC_RTC_HOUR] & 0x1f, time[PMIC_RTC_MIN],
		time[PMIC_RTC_SEC], time[PMIC_RTC_HOUR] & (1 << 6) ? "PM" : "AM");

	return 0;
}

void pmic_disable_wtsr(void) {
	unsigned char reg;

	speedy_read(CONFIG_SPEEDY0_BASE, S2MPS19_RTC_ADDR, S2MPS19_RTC_WTSR_SMPL, &reg);

	reg &= ~(1 << 6); // Clear bit 6 (WTSR)

	speedy_write(CONFIG_SPEEDY0_BASE, S2MPS19_RTC_ADDR, S2MPS19_RTC_WTSR_SMPL, reg);
}

void pmic_disable_smpl(void) {
	unsigned char reg;

	speedy_read(CONFIG_SPEEDY0_BASE, S2MPS19_RTC_ADDR, S2MPS19_RTC_WTSR_SMPL, &reg);

	reg &= ~(1 << 7); // Clear bit 7 (SMPL)
	reg &= ~(1 << 8); // Clear bit 8 (SUB_SMPL)

	speedy_write(CONFIG_SPEEDY0_BASE, S2MPS19_RTC_ADDR, S2MPS19_RTC_WTSR_SMPL, reg);
}

void pmic_shutdown(void) {
	unsigned char reg = 0x80;

	speedy_write(CONFIG_SPEEDY1_BASE, S2MPS22_PM_ADDR, S2MPS22_PM_CTRL1, reg);
}
