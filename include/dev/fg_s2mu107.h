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

#ifndef __FG_S2MU107_H__
#define __FG_S2MU107_H__

#define GPP1BASE	(0x10040020)
#define GPP1CON		*(volatile unsigned int *)(GPP1BASE + 0x0)
#define GPP1DAT		*(volatile unsigned int *)(GPP1BASE + 0x4)
#define GPP1PUD		*(volatile unsigned int *)(GPP1BASE + 0x8)

/* SDA: GPP1_3, SCL: GPP1_2 */
#define GPIO_DAT_FG_S2MU107	GPP1DAT
#define GPIO_DAT_FG_SHIFT		(3)
#define GPIO_PUD_FG_S2MU107	GPP1PUD &= ~(0xff << ((GPIO_DAT_FG_SHIFT-1)*4))

#define IIC_S2MU107_FG_ESCL_Hi	GPP1DAT |= (0x1 << (GPIO_DAT_FG_SHIFT-1))
#define IIC_S2MU107_FG_ESCL_Lo	GPP1DAT &= ~(0x1 << (GPIO_DAT_FG_SHIFT-1))
#define IIC_S2MU107_FG_ESDA_Hi	GPP1DAT |= (0x1 << GPIO_DAT_FG_SHIFT)
#define IIC_S2MU107_FG_ESDA_Lo	GPP1DAT &= ~(0x1 << GPIO_DAT_FG_SHIFT)

#define IIC_S2MU107_FG_ESCL_INP	GPP1CON &= ~(0xf << ((GPIO_DAT_FG_SHIFT-1)*4))
#define IIC_S2MU107_FG_ESCL_OUTP	GPP1CON = (GPP1CON & ~(0xf << ((GPIO_DAT_FG_SHIFT-1)*4))) \
					| (0x1 << ((GPIO_DAT_FG_SHIFT-1)*4))
#define IIC_S2MU107_FG_ESDA_INP	GPP1CON &= ~(0xf << (GPIO_DAT_FG_SHIFT*4))
#define IIC_S2MU107_FG_ESDA_OUTP	GPP1CON = (GPP1CON & ~(0xf << (GPIO_DAT_FG_SHIFT*4))) \
					 | (0x1 << (GPIO_DAT_FG_SHIFT*4))

#define DELAY		100

#define S2MU107_FG_SLAVE_ADDR_W		0x76
#define S2MU107_FG_SLAVE_ADDR_R		0x77

#define S2MU107_REG_STATUS		0x00
#define S2MU107_REG_IRQ			0x02
#define S2MU107_REG_RVBAT		0x04
#define S2MU107_REG_RCUR_CC		0x06
#define S2MU107_REG_RSOC_CC		0x08
#define S2MU107_REG_MONOUT		0x0A
#define S2MU107_REG_MONOUT_SEL		0x0C
#define S2MU107_REG_RBATCAP_OCV		0x0E
#define S2MU107_REG_RBATCAP		0x10
#define S2MU107_REG_RZADJ_CHG		0x12
#define S2MU107_REG_REDV		0x14
#define S2MU107_REG_RZADJ		0x15
#define S2MU107_REG_RBATZ0		0x16
#define S2MU107_REG_RBATZ1		0x18
#define S2MU107_REG_IRQ_LVL		0x1A
#define S2MU107_REG_TEMP_IRQ_LVL	0x1C
#define S2MU107_REG_START		0x1E
#define S2MU107_REG_SOFT_RESET		0x1F
#define S2MU107_REG_FG_ID		0x48
#define S2MU107_REG_CTRL0		0x25

#define abs(x) (((x)<0) ? -(x):(x))

#define TABLE_SIZE	22

void IIC_S2MU107_FG_EWrite(unsigned char ChipId,
		unsigned char IicAddr, unsigned char IicData);
void IIC_S2MU107_FG_ERead(unsigned char ChipId,
		unsigned char IicAddr, unsigned char *IicData);
void IIC_S2MU107_FG_ESetport(void);

void fg_init_s2mu107(void);
#endif /*__FG_S2MU107_H__*/
