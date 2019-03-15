/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __SUB_PMIC_H__
#define __SUB_PMIC_H__

#define GPP1CON			(*(volatile unsigned int *)(0x10430020))
#define GPP1DAT			(*(volatile unsigned int *)(0x10430024))
#define GPP1PUD			(*(volatile unsigned int *)(0x10430028))

#define IIC_ESCL_Hi		(GPP1DAT |= (0x1 << 4))
#define IIC_ESCL_Lo		(GPP1DAT &= ~(0x1 << 4))
#define IIC_ESDA_Hi		(GPP1DAT |= (0x1 << 5))
#define IIC_ESDA_Lo		(GPP1DAT &= ~(0x1 << 5))

#define IIC_ESCL_INP		(GPP1CON &= ~(0xf << 16))
#define IIC_ESCL_OUTP		(GPP1CON = (GPP1CON & ~(0xf << 16)) | (0x1 << 16))
#define IIC_ESDA_INP		(GPP1CON &= ~(0xf << 20))
#define IIC_ESDA_OUTP		(GPP1CON = (GPP1CON & ~(0xf << 20)) | (0x1 << 20))

#define IIC_GPIO_DAT		GPP1DAT
#define IIC_GPIO_DAT_SHIFT	(5)
#define IIC_DIS_GPIO_PUD	(GPP1PUD &= ~(0xff << 16))

#define IIC_DELAY		100

/* S2MPB02 slave address */
#define S2MPB02_W_ADDR		0xB2

/* S2MPB02 Register Address */
#define S2MPB02_BUCK1_CTRL	0x0B
#define S2MPB02_BUCK1_OUT	0x0C
#define S2MPB02_LDO6_CTRL	0x23
#define S2MPB02_LDO9_CTRL	0x26
#define S2MPB02_LDO11_CTRL	0x28
#define S2MPB02_LDO18_CTRL	0x2F

#define Buck1_EN		(0x2 << 6)

extern void sub_pmic_s2mpb02_init(void);

#endif /*__SUB_PMIC_H__*/
