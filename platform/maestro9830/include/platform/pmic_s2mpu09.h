/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __S2MPU09_PMIC_H__
#define __S2MPU09_PMIC_H__

/* S2MPU09 slave address */
#define S2MPU09_COMMON_ADDR	0x0
#define S2MPU09_PM_ADDR		0x1
#define S2MPU09_RTC_ADDR	0x2

/* S2MPU09 Register Address */
#define S2MPU09_COMMON_CHIPID	0x000
#define S2MPU09_PM_INT1			0x000
#define S2MPU09_PM_INT2			0x001
#define S2MPU09_PM_PWRONSRC		0x00C
#define S2MPU09_PM_OFFSRC		0x00D
#define S2MPU09_PM_RTC_BUF		0x00F
#define S2MPU09_PM_CTRL1		0x010
#define S2MPU09_PM_CTRL3		0x012
#define S2MPU09_PM_LDO38_CTRL		0x05F
#define S2MPU09_PM_LDO39_CTRL		0x060


#define S2MPU09_RTC_WTSR_SMPL	0x001
#define S2MPU09_RTC_UPDATE		0x002
#define S2MPU09_RTC_CAP_SEL		0x003
#define S2MPU09_RTC_MSEC		0x004
#define S2MPU09_RTC_SEC			0x005
#define S2MPU09_RTC_MIN			0x006
#define S2MPU09_RTC_HOUR		0x007
#define S2MPU09_RTC_WEEK		0x008
#define S2MPU09_RTC_DAY			0x009
#define S2MPU09_RTC_MON			0x00A
#define S2MPU09_RTC_YEAR		0x00B

/* RTC Counter Register offsets */
enum {
	PMIC_RTC_SEC = 0,
	PMIC_RTC_MIN,
	PMIC_RTC_HOUR,
	PMIC_RTC_WEEK,
	PMIC_RTC_DATE,
	PMIC_RTC_MONTH,
	PMIC_RTC_YEAR,
	NR_PMIC_RTC_CNT_REGS,
};

/*
 * PWRONSRC
 */
#define ACOK		(0x1 << 2)

/*
 * RTC_BUF
 */
#define _32KHZPERI_EN	(0x1 << 2)
#define _32KHZAP_EN	(0x1 << 0)

/*
 * CTRL1
 */
#define MRSTB_EN	(0x1 << 4)

/*
 * CTRL3
 */
#define WRSTBIEN	(0x1 << 6)
#define WRSTEN		(0x1 << 4)

/*
 * LDOx_CTRL
 */
#define S2MPU09_OUTPUT_ON_NORMAL	(0x3 << 6)

void pmic_init(void);
void display_pmic_info_s2mpu09(void);

#endif /*__S2MPU09_PMIC_H__*/

