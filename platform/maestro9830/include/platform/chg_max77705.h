/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 *
 *
 * Alternatively, this program is free software in case of open source project
 * you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __CHG_MAX77705_H__
#define __CHG_MAX77705_H__

#define GPM12CON		*(volatile unsigned int *)(0x10430020)
#define GPM12DAT		*(volatile unsigned int *)(0x10430024)
#define GPM12PUD		*(volatile unsigned int *)(0x10430028)

#define GPM13CON		*(volatile unsigned int *)(0x10430020)
#define GPM13DAT		*(volatile unsigned int *)(0x10430024)
#define GPM13PUD		*(volatile unsigned int *)(0x10430028)

#define GPIO_DAT_MAX77705	GPM13DAT
#define GPIO_DAT_SHIFT		(6)
#define GPIO_DAT_SHIFT2		(7)
#define GPIO_PUD_MAX77705	GPM12PUD &= ~(0xf << (GPIO_DAT_SHIFT * 4)); GPM13PUD &= ~(0xf << (GPIO_DAT_SHIFT2 * 4))

#define IIC_MAX77705_ESCL_Hi	GPM12DAT |= (0x1 << (GPIO_DAT_SHIFT))
#define IIC_MAX77705_ESCL_Lo	GPM12DAT &= ~(0x1 << (GPIO_DAT_SHIFT))
#define IIC_MAX77705_ESDA_Hi	GPM13DAT |= (0x1 << GPIO_DAT_SHIFT2)
#define IIC_MAX77705_ESDA_Lo	GPM13DAT &= ~(0x1 << GPIO_DAT_SHIFT2)

#define IIC_MAX77705_ESCL_INP	GPM12CON &= ~(0xf << ((GPIO_DAT_SHIFT) * 4))
#define IIC_MAX77705_ESCL_OUTP	GPM12CON = (GPM12CON & ~(0xf << ((GPIO_DAT_SHIFT) * 4))) \
	                                   | (0x1 << ((GPIO_DAT_SHIFT) * 4))
#define IIC_MAX77705_ESDA_INP	GPM13CON &= ~(0xf << (GPIO_DAT_SHIFT2 * 4))
#define IIC_MAX77705_ESDA_OUTP	GPM13CON = (GPM13CON & ~(0xf << (GPIO_DAT_SHIFT2 * 4))) \
	                                   | (0x1 << (GPIO_DAT_SHIFT2 * 4))

#define DELAY			100

/* MAX77705 slave address */
#define MAX77705_W_ADDR		0xD2
#define MAX77705_R_ADDR		0xD3

void IIC_MAX77705_ESetport(void);
void IIC_MAX77705_ERead(unsigned char ChipId,
                        unsigned char IicAddr, unsigned char *IicData);
void IIC_MAX77705_EWrite(unsigned char ChipId,
                         unsigned char IicAddr, unsigned char IicData);
void chg_init_max77705(void);

#endif /*__CHG_MAX77705_H__*/
