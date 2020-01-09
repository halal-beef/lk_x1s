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
#include <dev/blic_lm36274.h>

static void Delay(void)
{
	unsigned long i = 0;

	for (i = 0; i < DELAY; i++)
		;
}

static void IIC_LM36274_SCLH_SDAH(void)
{
	IIC_LM36274_ESCL_Hi;
	IIC_LM36274_ESDA_Hi;
	Delay();
}

static void IIC_LM36274_SCLH_SDAL(void)
{
	IIC_LM36274_ESCL_Hi;
	IIC_LM36274_ESDA_Lo;
	Delay();
}

static void IIC_LM36274_SCLL_SDAH(void)
{
	IIC_LM36274_ESCL_Lo;
	IIC_LM36274_ESDA_Hi;
	Delay();
}

static void IIC_LM36274_SCLL_SDAL(void)
{
	IIC_LM36274_ESCL_Lo;
	IIC_LM36274_ESDA_Lo;
	Delay();
}

static void IIC_LM36274_ELow(void)
{
	IIC_LM36274_SCLL_SDAL();
	IIC_LM36274_SCLH_SDAL();
	IIC_LM36274_SCLH_SDAL();
	IIC_LM36274_SCLL_SDAL();
}

static void IIC_LM36274_EHigh(void)
{
	IIC_LM36274_SCLL_SDAH();
	IIC_LM36274_SCLH_SDAH();
	IIC_LM36274_SCLH_SDAH();
	IIC_LM36274_SCLL_SDAH();
}

static void IIC_LM36274_EStart(void)
{
	IIC_LM36274_SCLH_SDAH();
	IIC_LM36274_SCLH_SDAL();
	Delay();
	IIC_LM36274_SCLL_SDAL();
}

static void IIC_LM36274_EEnd(void)
{
	IIC_LM36274_SCLL_SDAL();
	IIC_LM36274_SCLH_SDAL();
	Delay();
	IIC_LM36274_SCLH_SDAH();
}

static void IIC_LM36274_EAck_write(void)
{
	unsigned long ack = 0;

	/* Function <- Input */
	IIC_LM36274_ESDA_INP;

	IIC_LM36274_ESCL_Lo;
	Delay();
	IIC_LM36274_ESCL_Hi;
	Delay();
	ack = GPIO_DAT_LM36274;
	IIC_LM36274_ESCL_Hi;
	Delay();
	IIC_LM36274_ESCL_Hi;
	Delay();

	/* Function <- Output (SDA) */
	IIC_LM36274_ESDA_OUTP;

	ack = (ack >> GPIO_DAT_SHIFT2) & 0x1;

	IIC_LM36274_SCLL_SDAL();
}

static void IIC_LM36274_EAck_read(void)
{
	/* Function <- Output */
	IIC_LM36274_ESDA_OUTP;

	IIC_LM36274_ESCL_Lo;
	IIC_LM36274_ESCL_Lo;
	IIC_LM36274_ESDA_Hi;
	IIC_LM36274_ESCL_Hi;
	IIC_LM36274_ESCL_Hi;
	/* Function <- Input (SDA) */
	IIC_LM36274_ESDA_INP;

	IIC_LM36274_SCLL_SDAL();
}

void IIC_LM36274_ESetport(void)
{
	/* Pull Up/Down Disable SCL, SDA */
	GPIO_PUD_LM36274;

	IIC_LM36274_ESCL_Hi;
	IIC_LM36274_ESDA_Hi;

	/* Function <- Output (SCL) */
	IIC_LM36274_ESCL_OUTP;
	/* Function <- Output (SDA) */
	IIC_LM36274_ESDA_OUTP;

	Delay();
}

void IIC_LM36274_EWrite(unsigned char ChipId,
                         unsigned char IicAddr, unsigned char IicData)
{
	unsigned long i = 0;

	IIC_LM36274_EStart();

	/* write chip id */
	for (i = 7; i > 0; i--) {
		if ((ChipId >> i) & 0x0001)
			IIC_LM36274_EHigh();
		else
			IIC_LM36274_ELow();
	}

	/* write */
	IIC_LM36274_ELow();

	/* ACK */
	IIC_LM36274_EAck_write();

	/* write reg. addr. */
	for (i = 8; i > 0; i--) {
		if ((IicAddr >> (i - 1)) & 0x0001)
			IIC_LM36274_EHigh();
		else
			IIC_LM36274_ELow();
	}

	/* ACK */
	IIC_LM36274_EAck_write();

	/* write reg. data. */
	for (i = 8; i > 0; i--) {
		if ((IicData >> (i - 1)) & 0x0001)
			IIC_LM36274_EHigh();
		else
			IIC_LM36274_ELow();
	}

	/* ACK */
	IIC_LM36274_EAck_write();

	IIC_LM36274_EEnd();
}

void IIC_LM36274_ERead(unsigned char ChipId,
                        unsigned char IicAddr, unsigned char *IicData)
{
	unsigned long i = 0;
	unsigned long reg = 0;
	unsigned char data = 0;

	IIC_LM36274_EStart();

	/* write chip id */
	for (i = 7; i > 0; i--) {
		if ((ChipId >> i) & 0x0001)
			IIC_LM36274_EHigh();
		else
			IIC_LM36274_ELow();
	}

	/* write */
	IIC_LM36274_ELow();

	/* ACK */
	IIC_LM36274_EAck_write();

	/* write reg. addr. */
	for (i = 8; i > 0; i--) {
		if ((IicAddr >> (i - 1)) & 0x0001)
			IIC_LM36274_EHigh();
		else
			IIC_LM36274_ELow();
	}

	/* ACK */
	IIC_LM36274_EAck_write();

	IIC_LM36274_EStart();

	/* write chip id */
	for (i = 7; i > 0; i--) {
		if ((ChipId >> i) & 0x0001)
			IIC_LM36274_EHigh();
		else
			IIC_LM36274_ELow();
	}

	/* read */
	IIC_LM36274_EHigh();
	/* ACK */
	IIC_LM36274_EAck_write();

	/* read reg. data. */
	IIC_LM36274_ESDA_INP;

	IIC_LM36274_ESCL_Lo;
	IIC_LM36274_ESCL_Lo;
	Delay();

	for (i = 8; i > 0; i--) {
		IIC_LM36274_ESCL_Lo;
		IIC_LM36274_ESCL_Lo;
		Delay();
		IIC_LM36274_ESCL_Hi;
		IIC_LM36274_ESCL_Hi;
		Delay();
		reg = GPIO_DAT_LM36274;
		IIC_LM36274_ESCL_Hi;
		IIC_LM36274_ESCL_Hi;
		Delay();
		IIC_LM36274_ESCL_Lo;
		IIC_LM36274_ESCL_Lo;
		Delay();

		reg = (reg >> GPIO_DAT_SHIFT2) & 0x1;

		data |= reg << (i - 1);
	}

	/* ACK */
	IIC_LM36274_EAck_read();
	IIC_LM36274_ESDA_OUTP;

	IIC_LM36274_EEnd();

	*IicData = data;
}

void blic_init_LM36274(void)
{
	IIC_LM36274_ESetport();

	/* Enable backlight ic */
	IIC_LM36274_EWrite(LM36274_W_ADDR, 0x0c, 0x28);
	IIC_LM36274_EWrite(LM36274_W_ADDR, 0x0d, 0x26);
	IIC_LM36274_EWrite(LM36274_W_ADDR, 0x0e, 0x26);
	IIC_LM36274_EWrite(LM36274_W_ADDR, 0x09, 0xbe);
	IIC_LM36274_EWrite(LM36274_W_ADDR, 0x02, 0x69);
	IIC_LM36274_EWrite(LM36274_W_ADDR, 0x03, 0x0d);
	IIC_LM36274_EWrite(LM36274_W_ADDR, 0x11, 0x75);
	IIC_LM36274_EWrite(LM36274_W_ADDR, 0x04, 0x04);
	IIC_LM36274_EWrite(LM36274_W_ADDR, 0x05, 0xee);
	IIC_LM36274_EWrite(LM36274_W_ADDR, 0x10, 0x07);
	IIC_LM36274_EWrite(LM36274_W_ADDR, 0x08, 0x13);
	printf("LM36274 enabled \n");
}
