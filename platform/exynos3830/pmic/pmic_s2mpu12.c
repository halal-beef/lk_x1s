/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <debug.h>
#include <sys/types.h>
#include <platform/delay.h>
#include <platform/pmic_s2mpu12.h>
#include <platform/if_pmic_s2mu107.h>
#include <dev/lk_acpm_ipc.h>
#include <platform/gpio.h>
#include <platform/sfr.h>

static int chk_wtsr_smpl = 0;
static int read_int_first = 0;

void pmic_enable_manual_reset(pmic_mrdt deb_time)
{
	unsigned char reg;

	/* Disable Warm Reset */
	i3c_read(0, S2MPU12_PM_ADDR, S2MPU12_PM_CTRL3, &reg);
	reg &= ~WRSTEN;
	reg |= MRSEL;
	i3c_write(0, S2MPU12_PM_ADDR, S2MPU12_PM_CTRL3, reg);

	/* Enable Manual Reset */
	i3c_read(0, S2MPU12_PM_ADDR, S2MPU12_PM_CTRL1, &reg);
	reg |= MRSTB_EN;
	reg &= 0xF0;
	reg |= (0x0F & deb_time);
	i3c_write(0, S2MPU12_PM_ADDR, S2MPU12_PM_CTRL1, reg);
}

void pmic_int_mask(unsigned int chan, unsigned int addr, unsigned int interrupt)
{
	unsigned char reg;
	i3c_write(chan, addr, interrupt, 0xFF);
	i3c_read(chan, addr, interrupt, &reg);
	printf("interrupt(0x%x) : 0x%x\n", interrupt, reg);
}

void pmic_init (void)
{
	unsigned char reg;

	/* Disable manual reset */
	i3c_read(0, S2MPU12_PM_ADDR, S2MPU12_PM_CTRL1, &reg);
	reg &= ~MRSTB_EN;
	i3c_write(0, S2MPU12_PM_ADDR, S2MPU12_PM_CTRL1, reg);

	/* Enable warm reset */
	i3c_read(0, S2MPU12_PM_ADDR, S2MPU12_PM_CTRL3, &reg);
	reg |= WRSTEN;
	reg &= ~MRSEL;
	i3c_write(0, S2MPU12_PM_ADDR, S2MPU12_PM_CTRL3, reg);

	/* Enable eMMC power */
	i3c_read(0, S2MPU12_PM_ADDR, S2MPU12_PM_LDO2_CTRL, &reg);
	reg |= 0xC0;
	i3c_write(0, S2MPU12_PM_ADDR, S2MPU12_PM_LDO2_CTRL, reg);

	i3c_read(0, S2MPU12_PM_ADDR, S2MPU12_PM_LDO23_CTRL, &reg);
	reg |= 0xC0;
	i3c_write(0, S2MPU12_PM_ADDR, S2MPU12_PM_LDO23_CTRL, reg);

	/* Enable USB power */
	i3c_read(0, S2MPU12_PM_ADDR, S2MPU12_PM_LDO11_CTRL, &reg);
	reg |= 0xC0;
	i3c_write(0, S2MPU12_PM_ADDR, S2MPU12_PM_LDO11_CTRL, reg);

	/* Enable LCD power */
	i3c_read(0, S2MPU12_PM_ADDR, S2MPU12_PM_LDO28_CTRL, &reg);
	reg |= 0xC0;
	i3c_write(0, S2MPU12_PM_ADDR, S2MPU12_PM_LDO28_CTRL, reg);

	/* Enable TSP power */
	i3c_read(0, S2MPU12_PM_ADDR, S2MPU12_PM_LDO27_CTRL, &reg);
	reg |= 0xC0;
	i3c_write(0, S2MPU12_PM_ADDR, S2MPU12_PM_LDO27_CTRL, reg);
}

int get_pmic_rtc_time(char *buf)
{
	int i;
	u8 tmp;
	u8 time[NR_PMIC_RTC_CNT_REGS];

	i3c_read(0, S2MPU12_RTC_ADDR, S2MPU12_RTC_UPDATE, &tmp);
	tmp |= 0x1;
	i3c_write(0, S2MPU12_RTC_ADDR, S2MPU12_RTC_UPDATE, tmp);
	u_delay(40);

	for (i = 0; i < NR_PMIC_RTC_CNT_REGS; i++)
		i3c_read(0, S2MPU12_RTC_ADDR, (S2MPU12_RTC_SEC + i), &time[i]);

	printf("RTC TIME: %d-%02d-%02d %02d:%02d:%02d(0x%02x)%s\n",
			time[PMIC_RTC_YEAR] + 2000, time[PMIC_RTC_MONTH],
			time[PMIC_RTC_DATE], time[PMIC_RTC_HOUR] & 0x1f, time[PMIC_RTC_MIN],
			time[PMIC_RTC_SEC], time[PMIC_RTC_WEEK],
			time[PMIC_RTC_HOUR] & (1 << 6) ? "PM" : "AM");

	if (buf == NULL)
		return 0;

	sprintf(buf, "%04d%02d%02d%02d%02d%02d%s",
			time[PMIC_RTC_YEAR] + 2000, time[PMIC_RTC_MONTH],
			time[PMIC_RTC_DATE], time[PMIC_RTC_HOUR] & 0x1f, time[PMIC_RTC_MIN],
			time[PMIC_RTC_SEC], time[PMIC_RTC_HOUR] & (1 << 6) ? "PM" : "AM");

	return 0;
}

void read_pmic_info_s2mpu12 (void)
{
	unsigned char read_int1, read_int2, read_int;
	unsigned char read_ldo2_ctrl, read_ldo11_ctrl, read_ldo23_ctrl;
	unsigned char read_ldo27_ctrl, read_ldo28_ctrl;
	unsigned char read_pwronsrc, read_offsrc, read_wtsr_smpl;

	i3c_read(0, S2MPU12_PM_ADDR, S2MPU12_PM_INT1, &read_int1);
	i3c_read(0, S2MPU12_PM_ADDR, S2MPU12_PM_INT2, &read_int2);
	i3c_read(0, S2MPU12_PM_ADDR, S2MPU12_PM_INT3, &read_int);
	i3c_read(0, S2MPU12_PM_ADDR, S2MPU12_PM_INT4, &read_int);
	i3c_read(0, S2MPU12_PM_ADDR, S2MPU12_PM_INT5, &read_int);
	i3c_read(0, S2MPU12_PM_ADDR, S2MPU12_PM_INT6, &read_int);
	i3c_read(0, S2MPU12_PM_ADDR, S2MPU12_PM_PWRONSRC, &read_pwronsrc);
	i3c_read(0, S2MPU12_PM_ADDR, S2MPU12_PM_OFFSRC, &read_offsrc);

	i3c_read(0, S2MPU12_PM_ADDR, S2MPU12_PM_LDO2_CTRL, &read_ldo2_ctrl);
	i3c_read(0, S2MPU12_PM_ADDR, S2MPU12_PM_LDO11_CTRL, &read_ldo11_ctrl);
	i3c_read(0, S2MPU12_PM_ADDR, S2MPU12_PM_LDO23_CTRL, &read_ldo23_ctrl);
	i3c_read(0, S2MPU12_PM_ADDR, S2MPU12_PM_LDO27_CTRL, &read_ldo27_ctrl);
	i3c_read(0, S2MPU12_PM_ADDR, S2MPU12_PM_LDO28_CTRL, &read_ldo28_ctrl);
	/* read PMIC RTC */
	i3c_read(0, S2MPU12_RTC_ADDR, S2MPU12_RTC_WTSR_SMPL, &read_wtsr_smpl);

	printf("S2MPU12_PM_INT1: 0x%x\n", read_int1);
	printf("S2MPU12_PM_INT2: 0x%x\n", read_int2);
	printf("S2MPU12_PM_PWRONSRC: 0x%x\n", read_pwronsrc);
	printf("S2MPU12_PM_OFFSRC: 0x%x\n", read_offsrc);
	printf("S2MPU12_PM_LDO2_CTRL: 0x%x\n", read_ldo2_ctrl);
	printf("S2MPU12_PM_LDO11_CTRL: 0x%x\n", read_ldo11_ctrl);
	printf("S2MPU12_PM_LDO23_CTRL: 0x%x\n", read_ldo23_ctrl);
	printf("S2MPU12_PM_LDO27_CTRL: 0x%x\n", read_ldo27_ctrl);
	printf("S2MPU12_PM_LDO28_CTRL: 0x%x\n", read_ldo28_ctrl);
	printf("S2MPU12_RTC_WTSR_SMPL : 0x%x\n", read_wtsr_smpl);

	if ((read_pwronsrc & (1 << 7)) && (read_int2 & (1 << 5)) && !(read_int1 & (1 << 7))) {
		/* WTSR detect condition - WTSR_ON && WTSR_INT && ! MRB_INT */
		chk_wtsr_smpl = PMIC_DETECT_WTSR;
		printf("WTSR detected\n");
	} else if ((read_pwronsrc & (1 << 6)) && (read_int2 & (1 << 3)) && (read_wtsr_smpl & (1 << 7))) {
		/* SMPL detect condition - SMPL_ON && SMPL_INT && SMPL_EN */
		chk_wtsr_smpl = PMIC_DETECT_SMPL;
		printf("SMPL detected\n");
	} else {
		chk_wtsr_smpl = PMIC_DETECT_NONE;
	}

	read_int_first = 1;
	get_pmic_rtc_time(NULL);
}

int chk_smpl_wtsr_s2mpu12(void)
{
	if (!read_int_first)
		read_pmic_info_s2mpu12();

	return chk_wtsr_smpl;
}
