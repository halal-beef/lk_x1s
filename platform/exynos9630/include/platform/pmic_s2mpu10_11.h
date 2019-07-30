/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __S2MPU10_PMIC_H__
#define __S2MPU10_PMIC_H__

/* S2MPU10 slave address */
#define S2MPU10_COMMON_ADDR	0x0
#define S2MPU10_PM_ADDR		0x1
#define S2MPU10_RTC_ADDR	0x2

/* S2MPU10 Register Address */
#define S2MPU10_COMMON_CHIPID		0x000
#define S2MPU10_PM_INT1			0x000
#define S2MPU10_PM_INT2			0x001
#define S2MPU10_PM_INT3			0x002
#define S2MPU10_PM_INT4			0x003
#define S2MPU10_PM_INT5			0x004
#define S2MPU10_PM_INT6			0x005
#define S2MPU10_PM_PWRONSRC		0x00E
#define S2MPU10_PM_OFFSRC		0x00F
#define S2MPU10_PM_RTC_BUF		0x011
#define S2MPU10_PM_CTRL1		0x012
#define S2MPU10_PM_CTRL3		0x014

#define S2MPU10_PM_LDO21_CTRL		0x04A
#define S2MPU10_PM_LDO22_CTRL		0x04B
#define S2MPU10_PM_LDO23_CTRL		0x04C

#define S2MPU10_PM_INT1M		0x006
#define S2MPU10_PM_INT2M		0x007
#define S2MPU10_PM_INT3M		0x008
#define S2MPU10_PM_INT4M		0x009
#define S2MPU10_PM_INT5M		0x00A
#define S2MPU10_PM_INT6M		0x00B

#define S2MPU10_RTC_WTSR_SMPL	0x001
#define S2MPU10_RTC_UPDATE		0x002
#define S2MPU10_RTC_CAP_SEL		0x003
#define S2MPU10_RTC_MSEC		0x004
#define S2MPU10_RTC_SEC			0x005
#define S2MPU10_RTC_MIN			0x006
#define S2MPU10_RTC_HOUR		0x007
#define S2MPU10_RTC_WEEK		0x008
#define S2MPU10_RTC_DAY			0x009
#define S2MPU10_RTC_MON			0x00A
#define S2MPU10_RTC_YEAR		0x00B
#define S2MPU10_PM_ETC_OTP		0x013

/* S2MPU11 Register Address */
#define S2MPU11_PM_ADDR			0x1
#define S2MPU11_PM_INT1M		0x003
#define S2MPU11_PM_INT2M		0x004
#define S2MPU11_PM_INT3M		0x005
#
/* If ignore SMPL Detection, activate below define */
/* #define S2MPU10_PM_IGNORE_SMPL_DETECT */

/* If ignore WTSR Detection, activate below define */
/* #define S2MPU10_PM_IGNORE_WTSR_DETECT */

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

/* WTSR SMPL Detect */
enum {
	PMIC_DETECT_NONE = 0,
	PMIC_DETECT_WTSR,
	PMIC_DETECT_SMPL,
};

/* Manual Reset Debounce Time */
typedef enum {
	PMIC_MRDT_3 = 0,
	PMIC_MRDT_4,
	PMIC_MRDT_5,
	PMIC_MRDT_6,
	PMIC_MRDT_7,
	PMIC_MRDT_8,
	PMIC_MRDT_9,
	PMIC_MRDT_10,
	PMIC_MRDT_13,
	PMIC_MRDT_14,
	PMIC_MRDT_15,
	PMIC_MRDT_16,
	PMIC_MRDT_17,
	PMIC_MRDT_18,
	PMIC_MRDT_19,
	PMIC_MRDT_20,
} pmic_mrdt;

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
#define MRSEL		(0x1 << 5)
#define WRSTEN		(0x1 << 4)

/*
 * LDOx_CTRL
 */
#define S2MPU10_OUTPUT_ON_NORMAL	(0x3 << 6)

void pmic_init(void);
void pmic_enable_manual_reset(pmic_mrdt);
void read_pmic_info_s2mpu10(void);
int chk_smpl_wtsr_s2mpu10(void);
int get_pmic_rtc_time(char *buf);

#endif /*__S2MPU10_PMIC_H__*/

