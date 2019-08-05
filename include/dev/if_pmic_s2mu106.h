/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __IF_PMIC_S2MU106_H__
#define __IF_PMIC_S2MU106_H__

#define GPP9BASE	(0x10730000)
#define GPP9CON		*(volatile unsigned int *)(GPP9BASE + 0x80)
#define GPP9DAT		*(volatile unsigned int *)(GPP9BASE + 0x84)
#define GPP9PUD		*(volatile unsigned int *)(GPP9BASE + 0x88)

/* SDA: GPP9_6, SCL: GPP9_7 */
#define GPIO_DAT_S2MU106	GPP9DAT
#define GPIO_DAT_SHIFT		(6)
#define GPIO_PUD_S2MU106	GPP9PUD &= ~(0xff << (GPIO_DAT_SHIFT*4))

#define IIC_S2MU106_ESCL_Hi	GPP9DAT |= (0x1 << (GPIO_DAT_SHIFT+1))
#define IIC_S2MU106_ESCL_Lo	GPP9DAT &= ~(0x1 << (GPIO_DAT_SHIFT+1))
#define IIC_S2MU106_ESDA_Hi	GPP9DAT |= (0x1 << GPIO_DAT_SHIFT)
#define IIC_S2MU106_ESDA_Lo	GPP9DAT &= ~(0x1 << GPIO_DAT_SHIFT)

#define IIC_S2MU106_ESCL_INP	GPP9CON &= ~(0xf << ((GPIO_DAT_SHIFT+1)*4))
#define IIC_S2MU106_ESCL_OUTP	GPP9CON = (GPP9CON & ~(0xf << ((GPIO_DAT_SHIFT+1)*4))) \
					| (0x1 << ((GPIO_DAT_SHIFT+1)*4))
#define IIC_S2MU106_ESDA_INP	GPP9CON &= ~(0xf << (GPIO_DAT_SHIFT*4))
#define IIC_S2MU106_ESDA_OUTP	GPP9CON = (GPP9CON & ~(0xf << (GPIO_DAT_SHIFT*4))) \
					| (0x1 << (GPIO_DAT_SHIFT*4))

#define DELAY 100

/* S2MU106 muic slave address */
#define S2MU106_MUIC_W_ADDR	0x7C
#define S2MU106_MUIC_R_ADDR	0x7D

#define S2MU106_AFC_INT		0x0
#define S2MU106_MUIC_INT1	0x1
#define S2MU106_MUIC_INT2	0x2
#define S2MU106_PM_VAL_UP1	0x3
#define S2MU106_PM_VAL_UP2	0x4
#define S2MU106_PM_INT1		0x5
#define S2MU106_PM_INT2		0x6
#define S2MU106_MST_INT		0x7

#define S2MU106_AFC_CTRL1		0x2B
#define S2MU106_AFC_CTRL2		0x2C
#define S2MU106_AFC_LOGIC_CTRL2		0x41
#define S2MU106_DEVICE_TYPE1	0x47
#define S2MU106_DEVICE_APPLE		0x4E
#define S2MU106_MUIC_CTRL1		0x6D
#define S2MU106_MANUAL_SW_CTRL		0x70
#define S2MU106_LDOADC_VSETL		0x7A

/* S2MU106 charger slave address */
#define S2MU106_CHG_W_ADDR 0x7A
#define S2MU106_CHG_R_ADDR 0x7B

#define S2MU106_CHG_CTRL0	0x18
#define S2MU106_CHG_CTRL1	0x19
#define S2MU106_CHG_CTRL2	0x1A
#define S2MU106_CHG_CTRL3	0x1B
#define S2MU106_CHG_CTRL4	0x1C
#define S2MU106_CHG_CTRL5	0x1D
#define S2MU106_CHG_CTRL6	0x1E
#define S2MU106_CHG_CTRL7	0x1F
#define S2MU106_CHG_CTRL8	0x20
#define S2MU106_CHG_CTRL9	0x21
#define S2MU106_CHG_CTRL10	0x22

#define S2MU106_CHG_MASK_MODE	0x0F

#define S2MU106_CHG_MODE_OFF	0
#define S2MU106_CHG_MODE_BUCK	1
#define S2MU106_CHG_MODE_CHG	3

enum charger_mode {
	CHG_MODE_OFF,
	CHG_MODE_BUCK,
	CHG_MODE_CHG,
};

void IIC_S2MU106_ESetport(void);
void IIC_S2MU106_ERead(unsigned char ChipId,
		unsigned char IicAddr, unsigned char *IicData);
void IIC_S2MU106_EWrite(unsigned char ChipId,
		unsigned char IicAddr, unsigned char IicData);
void muic_sw_open(void);
void muic_sw_usb(void);
void muic_sw_uart(void);
int s2mu106_muic_get_vbus(void);
void init_muic_interrupt(void);
void s2mu106_charger_set_mode(int mode);
int s2mu106_charger_get_mode(void);
void s2mu106_charger_reg_init(void);
void s2mu106_charger_init(void);
void s2mu106_charger_set_input_current(int input_curr);
int s2mu106_charger_get_input_current(void);
void s2mu106_charger_set_charging_current(int chg_curr);
int s2mu106_charger_get_charging_current(void);
#endif /*__IF_PMIC_S2MU106_H__*/
