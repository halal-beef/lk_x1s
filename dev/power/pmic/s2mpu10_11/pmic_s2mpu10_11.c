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
#include <dev/pmic_s2mpu10_11.h>
#include <target/pmic.h>
#include <dev/lk_acpm_ipc.h>
#include <platform/gpio.h>
#include <platform/sfr.h>

static int chk_wtsr_smpl = 0;
static int read_int_first = 0;

void pmic_enable_manual_reset(pmic_mrdt deb_time)
{
	unsigned char reg;

	/* Disable Warm Reset */
	i3c_read(0, S2MPU10_PM_ADDR, S2MPU10_PM_CTRL3, &reg);
	reg &= ~WRSTEN;
	reg |= MRSEL;
	i3c_write(0, S2MPU10_PM_ADDR, S2MPU10_PM_CTRL3, reg);

	/* Enable Manual Reset */
	i3c_read(0, S2MPU10_PM_ADDR, S2MPU10_PM_CTRL1, &reg);
	reg |= MRSTB_EN;
	reg &= 0xF0;
	reg |= (0x0F & deb_time);
	reg &= ~WRSTBO_SEL;
	i3c_write(0, S2MPU10_PM_ADDR, S2MPU10_PM_CTRL1, reg);
}

void pmic_int_mask(unsigned int chan, unsigned int addr, unsigned int interrupt)
{
	unsigned char reg;
	i3c_write(chan, addr, interrupt, 0xFF);
	i3c_read(chan, addr, interrupt, &reg);
	printf("interrupt(0x%x) : 0x%x\n", interrupt, reg);
}

int get_pmic_rtc_time(char *buf)
{
	int i;
	u8 tmp;
	u8 time[NR_PMIC_RTC_CNT_REGS];

	i3c_read(0, S2MPU10_RTC_ADDR, S2MPU10_RTC_UPDATE, &tmp);
	tmp |= 0x1;
	i3c_write(0, S2MPU10_RTC_ADDR, S2MPU10_RTC_UPDATE, tmp);
	u_delay(40);

	for (i = 0; i < NR_PMIC_RTC_CNT_REGS; i++)
		i3c_read(0, S2MPU10_RTC_ADDR, (S2MPU10_RTC_SEC + i), &time[i]);

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

void read_smpl_wtsr(unsigned char read_int1, unsigned char read_int2, unsigned char read_pwronsrc, unsigned char read_wtsr_smpl)
{
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
	return;
}

int chk_smpl_wtsr_s2mpu10(void)
{
	if (!read_int_first)
		read_pmic_info();

	return chk_wtsr_smpl;
}
