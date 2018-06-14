/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __FG_S2MU004_H__
#define __FG_S2MU004_H__

#define GPP0BASE	(0x139b0000)
#define GPP0CON		*(volatile unsigned int *)(GPP0BASE + 0x0)
#define GPP0DAT		*(volatile unsigned int *)(GPP0BASE + 0x4)
#define GPP0PUD		*(volatile unsigned int *)(GPP0BASE + 0x8)

/* SDA: GPP0_0, SCL: GPP0_1 */
#define GPIO_DAT_S2MU004	GPP0DAT
#define GPIO_DAT_SHIFT		(0)
#define GPIO_PUD_S2MU004	GPP0PUD &= ~(0xff << (GPIO_DAT_SHIFT*4))

#define IIC_S2MU004_FG_ESCL_Hi	GPP0DAT |= (0x1 << (GPIO_DAT_SHIFT+1))
#define IIC_S2MU004_FG_ESCL_Lo	GPP0DAT &= ~(0x1 << (GPIO_DAT_SHIFT+1))
#define IIC_S2MU004_FG_ESDA_Hi	GPP0DAT |= (0x1 << GPIO_DAT_SHIFT)
#define IIC_S2MU004_FG_ESDA_Lo	GPP0DAT &= ~(0x1 << GPIO_DAT_SHIFT)

#define IIC_S2MU004_FG_ESCL_INP	GPP0CON &= ~(0xf << ((GPIO_DAT_SHIFT+1)*4))
#define IIC_S2MU004_FG_ESCL_OUTP	GPP0CON = (GPP0CON & ~(0xf << ((GPIO_DAT_SHIFT+1)*4))) \
					| (0x1 << ((GPIO_DAT_SHIFT+1)*4))
#define IIC_S2MU004_FG_ESDA_INP	GPP0CON &= ~(0xf << (GPIO_DAT_SHIFT*4))
#define IIC_S2MU004_FG_ESDA_OUTP	GPP0CON = (GPP0CON & ~(0xf << (GPIO_DAT_SHIFT*4))) \
					 | (0x1 << (GPIO_DAT_SHIFT*4))

#define DELAY		100

#define S2MU004_FG_SLAVE_ADDR_W		0x76
#define S2MU004_FG_SLAVE_ADDR_R		0x77

#define S2MU004_REG_STATUS		0x00
#define S2MU004_REG_IRQ			0x02
#define S2MU004_REG_RVBAT		0x04
#define S2MU004_REG_RCUR_CC		0x06
#define S2MU004_REG_RSOC_CC		0x08
#define S2MU004_REG_MONOUT		0x0A
#define S2MU004_REG_MONOUT_SEL		0x0C
#define S2MU004_REG_RBATCAP_OCV		0x0E
#define S2MU004_REG_RBATCAP		0x10
#define S2MU004_REG_RZADJ_CHG		0x12
#define S2MU004_REG_REDV		0x14
#define S2MU004_REG_RZADJ		0x15
#define S2MU004_REG_RBATZ0		0x16
#define S2MU004_REG_RBATZ1		0x18
#define S2MU004_REG_IRQ_LVL		0x1A
#define S2MU004_REG_TEMP_IRQ_LVL	0x1C
#define S2MU004_REG_START		0x1E
#define S2MU004_REG_SOFT_RESET		0x1F
#define S2MU004_REG_CTRL0		0x25
#define S2MU004_REG_FG_ID		0x48
#define S2MU004_REG_TEMP_A0		0x5A
#define S2MU004_REG_TEMP_B0		0x5C
#define S2MU004_REG_TEMP_C0		0x5E
#define S2MU004_REG_TEMP_D0		0x60

#define abs(x) (((x)<0) ? -(x):(x))

#define TABLE_SIZE	22

/* FG Accumulative rate - Default value(0x800)*/
#define S2MU005_REG_OX45_VALUE_EVT2 0x08
#define S2MU005_REG_OX44_VALUE_EVT2 0x00

#define FG_POWER_CHECK_DELAY			1000
#define FG_BOOTABLE_VOL			3500

#define FG_MODEL_DATA_VERSION 0x0 /* 0x1~0xE:data version, 0x0:default, 0xF:jig or corrupted */
#define VAL_0E	0xD0
#define VAL_0F	0x20
#define VAL_10	0x34
#define VAL_11	0x08

#define VAL_45	0x8
#define VAL_44	0x00

	/* 0x92 ~ 0xe9: BAT_PARAM */
static u8 model_param1[] = {
	204, 11, 61, 11, 175, 10, 34, 10, 169, 9,
	55, 9, 206, 8, 107, 8, 230, 7, 166, 7,
	77, 7, 234, 6, 174, 6, 130, 6, 95, 6,
	67, 6, 36, 6, 253, 5, 197, 5, 147, 5,
	42, 5, 168, 1, 247, 8, 136, 8, 24, 8,
	169, 7, 58, 7, 202, 6, 91, 6, 236, 5,
	124, 5, 13, 5, 158, 4, 47, 4, 191, 3,
	80, 3, 225, 2, 113, 2, 2, 2, 147, 1,
	36, 1, 180, 0, 69, 0, 214, 15
};

static u8 model_param2[] = {
	62, 62, 61, 60, 61, 60, 59, 60, 60, 58,
	58, 58, 58, 58, 58, 58, 59, 60, 62, 64,
	76, 154
};

int soc_arr_val[TABLE_SIZE] = {
	11205, 10662, 10119, 9575, 9032, 8488, 7945, 7402, 6858, 6315,
	5771, 5228, 4685, 4141, 3598, 3054, 2511, 1968, 1424, 881,
	337, -206
}; // * 0.01%
int ocv_arr_val[TABLE_SIZE] = {
	44747, 44050, 43353, 42664, 42077, 41520, 41006, 40522, 39872, 39562,
	39127, 38645, 38349, 38135, 37966, 37828, 37676, 37485, 37211, 36970,
	36454, 32069
}; // *0.1mV

void IIC_S2MU004_FG_EWrite(unsigned char ChipId,
		unsigned char IicAddr, unsigned char IicData);
void IIC_S2MU004_FG_ERead(unsigned char ChipId,
		unsigned char IicAddr, unsigned char *IicData);
void IIC_S2MU004_FG_ESetport(void);

#endif /*__FG_S2MU004_H__*/
