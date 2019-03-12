/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __PMIC_S2DOS05_H__
#define __PMIC_S2DOS05_H__

#define GPP1CON         *(volatile unsigned int *)(0x10430020)
#define GPP1DAT         *(volatile unsigned int *)(0x10430024)
#define GPP1PUD         *(volatile unsigned int *)(0x10430028)

#define GPIO_DAT_S2DOS05	GPP1DAT
#define S2DOS05_GPIO_DAT_SHIFT		(1)
#define GPIO_PUD_S2DOS05	GPP1PUD &= ~(0xff << (0 * 4));

#define IIC_S2DOS05_ESCL_Hi	GPP1DAT |= (0x1 << 0)
#define IIC_S2DOS05_ESCL_Lo	GPP1DAT &= ~(0x1 << 0)
#define IIC_S2DOS05_ESDA_Hi	GPP1DAT |= (0x1 << 1)
#define IIC_S2DOS05_ESDA_Lo	GPP1DAT &= ~(0x1 << 1)

#define IIC_S2DOS05_ESCL_INP	GPP1CON &= ~(0xf << (0 * 4))
#define IIC_S2DOS05_ESCL_OUTP	GPP1CON = (GPP1CON & ~(0xf << (0 * 4))) \
					| (0x1 << (0 * 4))
#define IIC_S2DOS05_ESDA_INP	GPP1CON &= ~(0xf << (1 * 4))
#define IIC_S2DOS05_ESDA_OUTP	GPP1CON = (GPP1CON & ~(0xf << (1 * 4))) \
					| (0x1 << (1 * 4))

#define DELAY		100

/* S2DOS05 slave address */
#define S2DOS05_ADDR	0xC0

/* S2DOS05 Register Address */
#define S2DOS05_REG_EN		0x03
#define S2DOS05_LDO1_CFG	0x04
#define S2DOS05_LDO4_CFG	0x07

#define LDO_EN				(0x1 << 7)
#define LDO1_EN			(0x1 << 0)
#define LDO4_EN			(0x1 << 3)
#define BUCK_EN			(0x1 << 4)


void IIC_S2DOS05_ESetport(void);
void IIC_S2DOS05_ERead(unsigned char ChipId,
		unsigned char IicAddr, unsigned char *IicData);
void IIC_S2DOS05_EWrite(unsigned char ChipId,
		unsigned char IicAddr, unsigned char IicData);
void pmic_init_s2dos05(void);

#endif /*__PMIC_S2DOS05_H__*/
