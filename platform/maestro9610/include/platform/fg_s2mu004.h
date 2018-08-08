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

void IIC_S2MU004_FG_EWrite(unsigned char ChipId,
		unsigned char IicAddr, unsigned char IicData);
void IIC_S2MU004_FG_ERead(unsigned char ChipId,
		unsigned char IicAddr, unsigned char *IicData);
void IIC_S2MU004_FG_ESetport(void);

void fg_init_s2mu004(void);

#endif /*__FG_S2MU004_H__*/
