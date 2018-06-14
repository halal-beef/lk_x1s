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
#include <platform/speedy.h>
#include <platform/delay.h>
#include <platform/pmic_s2mpu09.h>

void pmic_init (void)
{
	unsigned char reg;

	speedy_init();

	/* Disable Manual Reset */
	speedy_read(S2MPU09_PM_ADDR, S2MPU09_PM_CTRL1, &reg);
	reg &= ~MRSTB_EN;
	speedy_write(S2MPU09_PM_ADDR, S2MPU09_PM_CTRL1, reg);

	/* Enable Warm Reset */
	speedy_read(S2MPU09_PM_ADDR, S2MPU09_PM_CTRL3, &reg);
	reg |= WRSTEN;
	speedy_write(S2MPU09_PM_ADDR, S2MPU09_PM_CTRL3, reg);

	/* Enable AP warm reset detection */
	speedy_read(S2MPU09_PM_ADDR, S2MPU09_PM_CTRL3, &reg);
	reg |= WRSTBIEN;
	speedy_write(S2MPU09_PM_ADDR, S2MPU09_PM_CTRL3, reg);

	/* PERI 32kHz on, AP 32kHz on */
	speedy_read(S2MPU09_PM_ADDR, S2MPU09_PM_RTC_BUF, &reg);
	reg |= (_32KHZPERI_EN | _32KHZAP_EN);
	speedy_write(S2MPU09_PM_ADDR, S2MPU09_PM_RTC_BUF, reg);

#if 1
	/* defined (CONFIG_MACH_MAESTRO9610) */
	/* Enable LCD power */
	speedy_read(S2MPU09_PM_ADDR, S2MPU09_PM_LDO38_CTRL, &reg);
	reg |= S2MPU09_OUTPUT_ON_NORMAL;
	speedy_write(S2MPU09_PM_ADDR, S2MPU09_PM_LDO38_CTRL, reg);

	speedy_read(S2MPU09_PM_ADDR, S2MPU09_PM_LDO39_CTRL, &reg);
	reg |= S2MPU09_OUTPUT_ON_NORMAL;
	speedy_write(S2MPU09_PM_ADDR, S2MPU09_PM_LDO39_CTRL, reg);
#endif
}

void display_pmic_rtc_time(void)
{
#ifndef CONFIG_SG_BINARY
	int i;
	u8 tmp;
	u8 time[NR_PMIC_RTC_CNT_REGS];

	speedy_read(S2MPU09_RTC_ADDR, S2MPU09_RTC_UPDATE, &tmp);
	tmp |= 0x1;
	speedy_write(S2MPU09_RTC_ADDR, S2MPU09_RTC_UPDATE, tmp);
	u_delay(40);

	for (i = 0; i < NR_PMIC_RTC_CNT_REGS; i++)
		speedy_read(S2MPU09_RTC_ADDR, (S2MPU09_RTC_SEC + i), &time[i]);

	printf("RTC TIME: %d-%02d-%02d %02d:%02d:%02d(0x%02x)%s\n",
		time[PMIC_RTC_YEAR] + 2000, time[PMIC_RTC_MONTH],
		time[PMIC_RTC_DATE], time[PMIC_RTC_HOUR] & 0x1f, time[PMIC_RTC_MIN],
		time[PMIC_RTC_SEC], time[PMIC_RTC_WEEK],
		time[PMIC_RTC_HOUR] & (1 << 6) ? "PM" : "AM");
#endif
}
void display_pmic_info_s2mpu09 (void)
{
	unsigned char read_int1, read_int2, read_pwronsrc, read_offsrc, read_ctrl1, read_ctrl3, read_wtsr_smpl;
	unsigned char read_rtc_buf;
#if 1
	/* defined (CONFIG_MACH_MAESTRO9610) */
	unsigned char read_ldo38_ctrl, read_ldo39_ctrl;
#endif

	/* read PMIC PM */
	speedy_read(S2MPU09_PM_ADDR, S2MPU09_PM_INT1, &read_int1);
	speedy_read(S2MPU09_PM_ADDR, S2MPU09_PM_INT2, &read_int2);
	speedy_read(S2MPU09_PM_ADDR, S2MPU09_PM_PWRONSRC, &read_pwronsrc);
	speedy_read(S2MPU09_PM_ADDR, S2MPU09_PM_OFFSRC, &read_offsrc);
	speedy_read(S2MPU09_PM_ADDR, S2MPU09_PM_RTC_BUF, &read_rtc_buf);
	speedy_read(S2MPU09_PM_ADDR, S2MPU09_PM_CTRL1, &read_ctrl1);
	speedy_read(S2MPU09_PM_ADDR, S2MPU09_PM_CTRL3, &read_ctrl3);
#if 1
	/* defined (CONFIG_MACH_MAESTRO9610) */
	speedy_read(S2MPU09_PM_ADDR, S2MPU09_PM_LDO38_CTRL, &read_ldo38_ctrl);
	speedy_read(S2MPU09_PM_ADDR, S2MPU09_PM_LDO39_CTRL, &read_ldo39_ctrl);
#endif

	/* read PMIC RTC */
	speedy_read(S2MPU09_RTC_ADDR, S2MPU09_RTC_WTSR_SMPL, &read_wtsr_smpl);

	printf("S2MPU09_PM_INT1: 0x%x\n", read_int1);
	printf("S2MPU09_PM_INT2: 0x%x\n", read_int2);
	printf("S2MPU09_PM_PWRONSRC: 0x%x\n", read_pwronsrc);
	printf("S2MPU09_PM_OFFSRC: 0x%x\n", read_offsrc);
	printf("S2MPU09_PM_RTC_BUF: 0x%x\n", read_rtc_buf);
	printf("S2MPU09_PM_CTRL1: 0x%x\n", read_ctrl1);
	printf("S2MPU09_PM_CTRL3: 0x%x\n", read_ctrl3);
	printf("S2MPU09_RTC_WTSR_SMPL: 0x%x\n", read_wtsr_smpl);
#if 1
	/* defined (CONFIG_MACH_MAESTRO9610) */
	printf("S2MPU09_PM_LDO38_CTRL: 0x%x\n", read_ldo38_ctrl);
	printf("S2MPU09_PM_LDO39_CTRL: 0x%x\n", read_ldo39_ctrl);
#endif

	display_pmic_rtc_time();
}
