/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __IF_PMIC_S2MU004_H__
#define __IF_PMIC_S2MU004_H__

#define GPP0BASE	(0x139b0000)
#define GPP0CON		*(volatile unsigned int *)(GPP0BASE + 0x0)
#define GPP0DAT		*(volatile unsigned int *)(GPP0BASE + 0x4)
#define GPP0PUD		*(volatile unsigned int *)(GPP0BASE + 0x8)

/* SDA: GPP0_2, SCL: GPP0_3 */
#define GPIO_DAT_S2MU004	GPP0DAT
#define GPIO_DAT_SHIFT		(2)
#define GPIO_PUD_S2MU004	GPP0PUD &= ~(0xff << (GPIO_DAT_SHIFT*4))

#define IIC_S2MU004_ESCL_Hi	GPP0DAT |= (0x1 << (GPIO_DAT_SHIFT+1))
#define IIC_S2MU004_ESCL_Lo	GPP0DAT &= ~(0x1 << (GPIO_DAT_SHIFT+1))
#define IIC_S2MU004_ESDA_Hi	GPP0DAT |= (0x1 << GPIO_DAT_SHIFT)
#define IIC_S2MU004_ESDA_Lo	GPP0DAT &= ~(0x1 << GPIO_DAT_SHIFT)

#define IIC_S2MU004_ESCL_INP	GPP0CON &= ~(0xf << ((GPIO_DAT_SHIFT+1)*4))
#define IIC_S2MU004_ESCL_OUTP	GPP0CON = (GPP0CON & ~(0xf << ((GPIO_DAT_SHIFT+1)*4))) \
					| (0x1 << ((GPIO_DAT_SHIFT+1)*4))
#define IIC_S2MU004_ESDA_INP	GPP0CON &= ~(0xf << (GPIO_DAT_SHIFT*4))
#define IIC_S2MU004_ESDA_OUTP	GPP0CON = (GPP0CON & ~(0xf << (GPIO_DAT_SHIFT*4))) \
					| (0x1 << (GPIO_DAT_SHIFT*4))

#define DELAY		100

/* S2MU004 slave address */
#define S2MU004_W_ADDR	0x7A
#define S2MU004_R_ADDR	0x7B


#define S2MU004_CHG_CTRL0		0x10
#define S2MU004_CHG_CTRL1		0x11
#define S2MU004_CHG_CTRL2		0x12
#define S2MU004_CHG_CTRL3		0x13
#define S2MU004_CHG_CTRL4		0x14
#define S2MU004_CHG_CTRL5		0x15
#define S2MU004_CHG_CTRL6		0x16
#define S2MU004_CHG_CTRL7		0x17
#define S2MU004_CHG_CTRL8		0x18
#define S2MU004_CHG_CTRL9		0x19
#define S2MU004_CHG_CTRL10		0x1A
#define S2MU004_CHG_CTRL11		0x1B
#define S2MU004_CHG_CTRL12		0x1C
#define S2MU004_CHG_CTRL13		0x1D
#define S2MU004_CHG_CTRL14		0x1E
#define S2MU004_CHG_UVLOCTRL		0xA5
#define S2MU004_CHG_STATUS0		0x0A
#define S2MU004_CHG_STATUS1		0x0B
#define S2MU004_CHG_STATUS2		0x0C
#define S2MU004_CHG_STATUS3		0x0D
#define S2MU004_CHG_STATUS4		0x0E
#define S2MU004_CHG_STATUS5		0x0F
#define S2MU004_PMIC_ID			0x82

#define TA_CHG_CURR			1000
#define USB_CHG_CURR			500

#define S2MU004_MUIC_CTRL1		0xC7
#define S2MU004_MANUAL_SW_CTRL		0xCA
#define S2MU004_MUIC_REG_DEVICE_APPLE   0x69

#define S2MU004_MUIC_DEV3_VBUS		(0x1 << 1)

void IIC_S2MU004_ESetport(void);
void IIC_S2MU004_ERead(unsigned char ChipId,
		unsigned char IicAddr, unsigned char *IicData);
void IIC_S2MU004_EWrite(unsigned char ChipId,
		unsigned char IicAddr, unsigned char IicData);
void chg_init_s2mu004(void);
void muic_sw_usb(void);
void muic_sw_uart(void);
int muic_get_vbus(void);

#endif /*__IF_PMIC_S2MU004_H__*/
