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

#ifndef __BLIC_LM36274_H__
#define __BLIC_LM36274_H__

/* SCL */
#define GPP03CON		*(volatile unsigned int *)(0x139B0020)
#define GPP03DAT		*(volatile unsigned int *)(0x139B0024)
#define GPP03PUD		*(volatile unsigned int *)(0x139B0028)

/* SDA */
#define GPP02CON		*(volatile unsigned int *)(0x139B0020)
#define GPP02DAT		*(volatile unsigned int *)(0x139B0024)
#define GPP02PUD		*(volatile unsigned int *)(0x139B0028)

#define GPIO_DAT_LM36274	GPP02DAT
#define GPIO_DAT_SHIFT		(3)
#define GPIO_DAT_SHIFT2		(2)
#define GPIO_PUD_LM36274	GPP03PUD &= ~(0xf << (GPIO_DAT_SHIFT * 4)); GPP02PUD &= ~(0xf << (GPIO_DAT_SHIFT2 * 4))

#define IIC_LM36274_ESCL_Hi	GPP03DAT |= (0x1 << (GPIO_DAT_SHIFT))
#define IIC_LM36274_ESCL_Lo	GPP03DAT &= ~(0x1 << (GPIO_DAT_SHIFT))
#define IIC_LM36274_ESDA_Hi	GPP02DAT |= (0x1 << GPIO_DAT_SHIFT2)
#define IIC_LM36274_ESDA_Lo	GPP02DAT &= ~(0x1 << GPIO_DAT_SHIFT2)

#define IIC_LM36274_ESCL_INP	GPP03CON &= ~(0xf << ((GPIO_DAT_SHIFT) * 4))
#define IIC_LM36274_ESCL_OUTP	GPP03CON = (GPP03CON & ~(0xf << ((GPIO_DAT_SHIFT) * 4))) \
	                                   | (0x1 << ((GPIO_DAT_SHIFT) * 4))
#define IIC_LM36274_ESDA_INP	GPP02CON &= ~(0xf << (GPIO_DAT_SHIFT2 * 4))
#define IIC_LM36274_ESDA_OUTP	GPP02CON = (GPP02CON & ~(0xf << (GPIO_DAT_SHIFT2 * 4))) \
	                                   | (0x1 << (GPIO_DAT_SHIFT2 * 4))

#define DELAY			100

/* LM36274 slave address */
#define LM36274_W_ADDR		0x22
#define LM36274_R_ADDR		0x23

void IIC_LM36274_ESetport(void);
void IIC_LM36274_ERead(unsigned char ChipId,
                        unsigned char IicAddr, unsigned char *IicData);
void IIC_LM36274_EWrite(unsigned char ChipId,
                         unsigned char IicAddr, unsigned char IicData);
void blic_init_LM36274(void);

#endif /*__BLIC_LM36274_H__*/
