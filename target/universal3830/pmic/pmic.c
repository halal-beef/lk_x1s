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
#include <platform/delay.h>
#include <target/pmic.h>
#include <dev/lk_acpm_ipc.h>
#include <platform/gpio.h>
#include <platform/sfr.h>
#include <target/board_info.h>

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

#if defined(CONFIG_UNIVERSAL3830)
	/* Enable LCD power */
	i3c_read(0, S2MPU12_PM_ADDR, S2MPU12_PM_LDO26_CTRL, &reg);
	reg |= 0xC0;
	i3c_write(0, S2MPU12_PM_ADDR, S2MPU12_PM_LDO26_CTRL, reg);
#endif

	/* Enable LCD power */
	i3c_read(0, S2MPU12_PM_ADDR, S2MPU12_PM_LDO28_CTRL, &reg);
	reg |= 0xC0;
	i3c_write(0, S2MPU12_PM_ADDR, S2MPU12_PM_LDO28_CTRL, reg);

	/* Enable TSP power */
	i3c_read(0, S2MPU12_PM_ADDR, S2MPU12_PM_LDO27_CTRL, &reg);
	reg |= 0xC0;
	i3c_write(0, S2MPU12_PM_ADDR, S2MPU12_PM_LDO27_CTRL, reg);
}

void read_pmic_info (void)
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

	read_smpl_wtsr(read_int1, read_int2, read_pwronsrc, read_wtsr_smpl);

	get_pmic_rtc_time(NULL);

	return;
}

int chk_smpl_wtsr(void)
{
	return chk_smpl_wtsr_s2mpu12();
}
