/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __IF_PMIC_S2MU107_H__
#define __IF_PMIC_S2MU107_H__

#define GPP1BASE	(0x10040020)
#define GPP1CON		*(volatile unsigned int *)(GPP1BASE + 0x0)
#define GPP1DAT		*(volatile unsigned int *)(GPP1BASE + 0x4)
#define GPP1PUD		*(volatile unsigned int *)(GPP1BASE + 0x8)

/* SDA: GPP1_3, SCL: GPP1_2 */
#define GPIO_DAT_S2MU107	GPP1DAT
#define GPIO_DAT_SHIFT		(3)
#define GPIO_PUD_S2MU107	GPP1PUD &= ~(0xff << ((GPIO_DAT_SHIFT-1)*4))

#define IIC_S2MU107_ESCL_Hi	GPP1DAT |= (0x1 << (GPIO_DAT_SHIFT-1))
#define IIC_S2MU107_ESCL_Lo	GPP1DAT &= ~(0x1 << (GPIO_DAT_SHIFT-1))
#define IIC_S2MU107_ESDA_Hi	GPP1DAT |= (0x1 << GPIO_DAT_SHIFT)
#define IIC_S2MU107_ESDA_Lo	GPP1DAT &= ~(0x1 << GPIO_DAT_SHIFT)

#define IIC_S2MU107_ESCL_INP	GPP1CON &= ~(0xf << ((GPIO_DAT_SHIFT-1)*4))
#define IIC_S2MU107_ESCL_OUTP	GPP1CON = (GPP1CON & ~(0xf << ((GPIO_DAT_SHIFT-1)*4))) \
					| (0x1 << ((GPIO_DAT_SHIFT-1)*4))
#define IIC_S2MU107_ESDA_INP	GPP1CON &= ~(0xf << (GPIO_DAT_SHIFT*4))
#define IIC_S2MU107_ESDA_OUTP	GPP1CON = (GPP1CON & ~(0xf << (GPIO_DAT_SHIFT*4))) \
					| (0x1 << (GPIO_DAT_SHIFT*4))

#define DELAY 100

/* S2MU107 common slave address */
#define S2MU107_COMMON_W_ADDR 	0x74
#define S2MU107_COMMON_R_ADDR 	0x75

/* S2MU107 charger slave address */
#define S2MU107_CHG_W_ADDR 	0x7A
#define S2MU107_CHG_R_ADDR 	0x7B

#define S2MU107_SC_CTRL0	0x18
#define S2MU107_SC_CTRL1	0x19
#define S2MU107_SC_CTRL2	0x1A
#define S2MU107_SC_CTRL3	0x1B
#define S2MU107_SC_CTRL4	0x1C
#define S2MU107_SC_CTRL5	0x1D
#define S2MU107_SC_CTRL6	0x1E
#define S2MU107_SC_CTRL7	0x1F
#define S2MU107_SC_CTRL8	0x20
#define S2MU107_SC_CTRL9	0x21
#define S2MU107_SC_CTRL10	0x22

#define S2MU107_CHG_MASK_MODE	0x0F

#define S2MU107_CHG_MODE_OFF	0
#define S2MU107_CHG_MODE_BUCK	1
#define S2MU107_CHG_MODE_CHG	3

/* S2MU107 Muic Slave Address */
#define S2MU107_MUIC_W_ADDR 		(0x7C)
#define S2MU107_MUIC_R_ADDR 		(0x7D)

#define S2MU107_MUIC_CTRL1		(0x6C)
#define S2MU107_MANUAL_SW_CTRL		(0x6F)

enum charger_mode {
	CHG_MODE_OFF,
	CHG_MODE_BUCK,
	CHG_MODE_CHG,
};

void IIC_S2MU107_ESetport(void);
void IIC_S2MU107_ERead(unsigned char ChipId,
		unsigned char IicAddr, unsigned char *IicData);
void IIC_S2MU107_EWrite(unsigned char ChipId,
		unsigned char IicAddr, unsigned char IicData);

void s2mu107_sc_set_mode(int mode);
void s2mu107_sc_init(void);

#endif /*__IF_PMIC_S2MU107_H__*/
