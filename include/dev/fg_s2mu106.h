/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.


 * Alternatively, this program is free software in case of open source project
 * you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.

*/

#ifndef __FG_S2MU106_H__
#define __FG_S2MU106_H__

#define GPP1BASE_FG	(0x10430000)
#define GPP1CON_FG		*(volatile unsigned int *)(GPP1BASE_FG + 0x20)
#define GPP1DAT_FG		*(volatile unsigned int *)(GPP1BASE_FG + 0x24)
#define GPP1PUD_FG		*(volatile unsigned int *)(GPP1BASE_FG + 0x28)

/* SDA: GPP1_0, SCL: GPP1_1 */
#define GPIO_DAT_FG_S2MU106	GPP1DAT_FG
#define GPIO_DAT_FG_SHIFT		(0)
#define GPIO_PUD_FG_S2MU106	GPP1PUD_FG &= ~(0xff << (GPIO_DAT_FG_SHIFT*4))

#define IIC_S2MU106_FG_ESCL_Hi	GPP1DAT_FG |= (0x1 << (GPIO_DAT_FG_SHIFT+1))
#define IIC_S2MU106_FG_ESCL_Lo	GPP1DAT_FG &= ~(0x1 << (GPIO_DAT_FG_SHIFT+1))
#define IIC_S2MU106_FG_ESDA_Hi	GPP1DAT_FG |= (0x1 << GPIO_DAT_FG_SHIFT)
#define IIC_S2MU106_FG_ESDA_Lo	GPP1DAT_FG &= ~(0x1 << GPIO_DAT_FG_SHIFT)

#define IIC_S2MU106_FG_ESCL_INP	GPP1CON_FG &= ~(0xf << ((GPIO_DAT_FG_SHIFT+1)*4))
#define IIC_S2MU106_FG_ESCL_OUTP	GPP1CON_FG = (GPP1CON_FG & ~(0xf << ((GPIO_DAT_FG_SHIFT+1)*4))) \
					| (0x1 << ((GPIO_DAT_FG_SHIFT+1)*4))
#define IIC_S2MU106_FG_ESDA_INP	GPP1CON_FG &= ~(0xf << (GPIO_DAT_FG_SHIFT*4))
#define IIC_S2MU106_FG_ESDA_OUTP	GPP1CON_FG = (GPP1CON_FG & ~(0xf << (GPIO_DAT_FG_SHIFT*4))) \
					 | (0x1 << (GPIO_DAT_FG_SHIFT*4))

#define DELAY		100

#define S2MU106_FG_SLAVE_ADDR_W		0x76
#define S2MU106_FG_SLAVE_ADDR_R		0x77

#define S2MU106_REG_STATUS		0x00
#define S2MU106_REG_IRQ			0x02
#define S2MU106_REG_RVBAT		0x04
#define S2MU106_REG_RCUR_CC		0x06
#define S2MU106_REG_RSOC_CC		0x08
#define S2MU106_REG_MONOUT		0x0A
#define S2MU106_REG_MONOUT_SEL		0x0C
#define S2MU106_REG_RBATCAP_OCV		0x0E
#define S2MU106_REG_RBATCAP		0x10
#define S2MU106_REG_RZADJ_CHG		0x12
#define S2MU106_REG_REDV		0x14
#define S2MU106_REG_RZADJ		0x15
#define S2MU106_REG_RBATZ0		0x16
#define S2MU106_REG_RBATZ1		0x18
#define S2MU106_REG_IRQ_LVL		0x1A
#define S2MU106_REG_TEMP_IRQ_LVL	0x1C
#define S2MU106_REG_START		0x1E
#define S2MU106_REG_SOFT_RESET		0x1F
#define S2MU106_REG_FG_ID		0x48
#define S2MU106_REG_CTRL0		0x25

#define abs(x) (((x)<0) ? -(x):(x))

#define TABLE_SIZE	22

void IIC_S2MU106_FG_EWrite(unsigned char ChipId,
		unsigned char IicAddr, unsigned char IicData);
void IIC_S2MU106_FG_ERead(unsigned char ChipId,
		unsigned char IicAddr, unsigned char *IicData);
void IIC_S2MU106_FG_ESetport(void);

void fg_init_s2mu106(void);
#if 0
void dead_battery_recovery(void);
#endif
#endif /*__FG_S2MU106_H__*/
