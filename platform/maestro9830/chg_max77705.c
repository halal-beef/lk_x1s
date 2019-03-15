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

//#include <common.h>
//#include <asm/arch/cpu.h>
#include <stdio.h>
#include <platform/chg_max77705.h>

static void Delay(void)
{
	unsigned long i = 0;

	for (i = 0; i < DELAY; i++)
		;
}

static void IIC_MAX77705_SCLH_SDAH(void)
{
	IIC_MAX77705_ESCL_Hi;
	IIC_MAX77705_ESDA_Hi;
	Delay();
}

static void IIC_MAX77705_SCLH_SDAL(void)
{
	IIC_MAX77705_ESCL_Hi;
	IIC_MAX77705_ESDA_Lo;
	Delay();
}

static void IIC_MAX77705_SCLL_SDAH(void)
{
	IIC_MAX77705_ESCL_Lo;
	IIC_MAX77705_ESDA_Hi;
	Delay();
}

static void IIC_MAX77705_SCLL_SDAL(void)
{
	IIC_MAX77705_ESCL_Lo;
	IIC_MAX77705_ESDA_Lo;
	Delay();
}

static void IIC_MAX77705_ELow(void)
{
	IIC_MAX77705_SCLL_SDAL();
	IIC_MAX77705_SCLH_SDAL();
	IIC_MAX77705_SCLH_SDAL();
	IIC_MAX77705_SCLL_SDAL();
}

static void IIC_MAX77705_EHigh(void)
{
	IIC_MAX77705_SCLL_SDAH();
	IIC_MAX77705_SCLH_SDAH();
	IIC_MAX77705_SCLH_SDAH();
	IIC_MAX77705_SCLL_SDAH();
}

static void IIC_MAX77705_EStart(void)
{
	IIC_MAX77705_SCLH_SDAH();
	IIC_MAX77705_SCLH_SDAL();
	Delay();
	IIC_MAX77705_SCLL_SDAL();
}

static void IIC_MAX77705_EEnd(void)
{
	IIC_MAX77705_SCLL_SDAL();
	IIC_MAX77705_SCLH_SDAL();
	Delay();
	IIC_MAX77705_SCLH_SDAH();
}

static void IIC_MAX77705_EAck_write(void)
{
	unsigned long ack = 0;

	/* Function <- Input */
	IIC_MAX77705_ESDA_INP;

	IIC_MAX77705_ESCL_Lo;
	Delay();
	IIC_MAX77705_ESCL_Hi;
	Delay();
	ack = GPIO_DAT_MAX77705;
	IIC_MAX77705_ESCL_Hi;
	Delay();
	IIC_MAX77705_ESCL_Hi;
	Delay();

	/* Function <- Output (SDA) */
	IIC_MAX77705_ESDA_OUTP;

	ack = (ack >> GPIO_DAT_SHIFT2) & 0x1;

	IIC_MAX77705_SCLL_SDAL();
}

static void IIC_MAX77705_EAck_read(void)
{
	/* Function <- Output */
	IIC_MAX77705_ESDA_OUTP;

	IIC_MAX77705_ESCL_Lo;
	IIC_MAX77705_ESCL_Lo;
	IIC_MAX77705_ESDA_Hi;
	IIC_MAX77705_ESCL_Hi;
	IIC_MAX77705_ESCL_Hi;
	/* Function <- Input (SDA) */
	IIC_MAX77705_ESDA_INP;

	IIC_MAX77705_SCLL_SDAL();
}

void IIC_MAX77705_ESetport(void)
{
	/* Pull Up/Down Disable SCL, SDA */
	GPIO_PUD_MAX77705;

	IIC_MAX77705_ESCL_Hi;
	IIC_MAX77705_ESDA_Hi;

	/* Function <- Output (SCL) */
	IIC_MAX77705_ESCL_OUTP;
	/* Function <- Output (SDA) */
	IIC_MAX77705_ESDA_OUTP;

	Delay();
}

void IIC_MAX77705_EWrite(unsigned char ChipId,
                         unsigned char IicAddr, unsigned char IicData)
{
	unsigned long i = 0;

	IIC_MAX77705_EStart();

	/* write chip id */
	for (i = 7; i > 0; i--) {
		if ((ChipId >> i) & 0x0001)
			IIC_MAX77705_EHigh();
		else
			IIC_MAX77705_ELow();
	}

	/* write */
	IIC_MAX77705_ELow();

	/* ACK */
	IIC_MAX77705_EAck_write();

	/* write reg. addr. */
	for (i = 8; i > 0; i--) {
		if ((IicAddr >> (i - 1)) & 0x0001)
			IIC_MAX77705_EHigh();
		else
			IIC_MAX77705_ELow();
	}

	/* ACK */
	IIC_MAX77705_EAck_write();

	/* write reg. data. */
	for (i = 8; i > 0; i--) {
		if ((IicData >> (i - 1)) & 0x0001)
			IIC_MAX77705_EHigh();
		else
			IIC_MAX77705_ELow();
	}

	/* ACK */
	IIC_MAX77705_EAck_write();

	IIC_MAX77705_EEnd();
}

void IIC_MAX77705_ERead(unsigned char ChipId,
                        unsigned char IicAddr, unsigned char *IicData)
{
	unsigned long i = 0;
	unsigned long reg = 0;
	unsigned char data = 0;

	IIC_MAX77705_EStart();

	/* write chip id */
	for (i = 7; i > 0; i--) {
		if ((ChipId >> i) & 0x0001)
			IIC_MAX77705_EHigh();
		else
			IIC_MAX77705_ELow();
	}

	/* write */
	IIC_MAX77705_ELow();

	/* ACK */
	IIC_MAX77705_EAck_write();

	/* write reg. addr. */
	for (i = 8; i > 0; i--) {
		if ((IicAddr >> (i - 1)) & 0x0001)
			IIC_MAX77705_EHigh();
		else
			IIC_MAX77705_ELow();
	}

	/* ACK */
	IIC_MAX77705_EAck_write();

	IIC_MAX77705_EStart();

	/* write chip id */
	for (i = 7; i > 0; i--) {
		if ((ChipId >> i) & 0x0001)
			IIC_MAX77705_EHigh();
		else
			IIC_MAX77705_ELow();
	}

	/* read */
	IIC_MAX77705_EHigh();
	/* ACK */
	IIC_MAX77705_EAck_write();

	/* read reg. data. */
	IIC_MAX77705_ESDA_INP;

	IIC_MAX77705_ESCL_Lo;
	IIC_MAX77705_ESCL_Lo;
	Delay();

	for (i = 8; i > 0; i--) {
		IIC_MAX77705_ESCL_Lo;
		IIC_MAX77705_ESCL_Lo;
		Delay();
		IIC_MAX77705_ESCL_Hi;
		IIC_MAX77705_ESCL_Hi;
		Delay();
		reg = GPIO_DAT_MAX77705;
		IIC_MAX77705_ESCL_Hi;
		IIC_MAX77705_ESCL_Hi;
		Delay();
		IIC_MAX77705_ESCL_Lo;
		IIC_MAX77705_ESCL_Lo;
		Delay();

		reg = (reg >> GPIO_DAT_SHIFT2) & 0x1;

		data |= reg << (i - 1);
	}

	/* ACK */
	IIC_MAX77705_EAck_read();
	IIC_MAX77705_ESDA_OUTP;

	IIC_MAX77705_EEnd();

	*IicData = data;
}

void chg_init_max77705(void)
{
	unsigned char reg, reg1;

	IIC_MAX77705_ESetport();

	/* Enable charging. */
	IIC_MAX77705_ERead(MAX77705_R_ADDR, 0xB7, &reg);
	printf("MAX77705 charging status: 0x%x\n", reg);
	IIC_MAX77705_EWrite(MAX77705_W_ADDR, 0xB7, reg | (1 << 0));
	IIC_MAX77705_ERead(MAX77705_R_ADDR, 0xB7, &reg);
	IIC_MAX77705_ERead(MAX77705_R_ADDR, 0xC3, &reg1);
	printf("MAX77705 enabled charging: 0x%x, 0x%x\n", reg, reg1);
}
