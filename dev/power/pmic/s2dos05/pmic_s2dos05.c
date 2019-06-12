/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <stdio.h>
#include <dev/pmic_s2dos05.h>

static void Delay(void)
{
	unsigned long i = 0;

	for (i = 0; i < DELAY; i++)
		;
}

static void IIC_S2DOS05_SCLH_SDAH(void)
{
	IIC_S2DOS05_ESCL_Hi;
	IIC_S2DOS05_ESDA_Hi;
	Delay();
}

static void IIC_S2DOS05_SCLH_SDAL(void)
{
	IIC_S2DOS05_ESCL_Hi;
	IIC_S2DOS05_ESDA_Lo;
	Delay();
}

static void IIC_S2DOS05_SCLL_SDAH(void)
{
	IIC_S2DOS05_ESCL_Lo;
	IIC_S2DOS05_ESDA_Hi;
	Delay();
}

static void IIC_S2DOS05_SCLL_SDAL(void)
{
	IIC_S2DOS05_ESCL_Lo;
	IIC_S2DOS05_ESDA_Lo;
	Delay();
}

static void IIC_S2DOS05_ELow(void)
{
	IIC_S2DOS05_SCLL_SDAL();
	IIC_S2DOS05_SCLH_SDAL();
	IIC_S2DOS05_SCLH_SDAL();
	IIC_S2DOS05_SCLL_SDAL();
}

static void IIC_S2DOS05_EHigh(void)
{
	IIC_S2DOS05_SCLL_SDAH();
	IIC_S2DOS05_SCLH_SDAH();
	IIC_S2DOS05_SCLH_SDAH();
	IIC_S2DOS05_SCLL_SDAH();
}

static void IIC_S2DOS05_EStart(void)
{
	IIC_S2DOS05_SCLH_SDAH();
	IIC_S2DOS05_SCLH_SDAL();
	Delay();
	IIC_S2DOS05_SCLL_SDAL();
}

static void IIC_S2DOS05_EEnd(void)
{
	IIC_S2DOS05_SCLL_SDAL();
	IIC_S2DOS05_SCLH_SDAL();
	Delay();
	IIC_S2DOS05_SCLH_SDAH();
}

static void IIC_S2DOS05_EAck_write(void)
{
	unsigned long ack = 0;

	/* Function <- Input */
	IIC_S2DOS05_ESDA_INP;

	IIC_S2DOS05_ESCL_Lo;
	Delay();
	IIC_S2DOS05_ESCL_Hi;
	Delay();
	ack = GPIO_DAT_S2DOS05;
	IIC_S2DOS05_ESCL_Hi;
	Delay();
	IIC_S2DOS05_ESCL_Hi;
	Delay();

	/* Function <- Output (SDA) */
	IIC_S2DOS05_ESDA_OUTP;

	ack = (ack >> S2DOS05_GPIO_DAT_SHIFT) & 0x1;

	IIC_S2DOS05_SCLL_SDAL();
}

static void IIC_S2DOS05_EAck_read(void)
{
	/* Function <- Output */
	IIC_S2DOS05_ESDA_OUTP;

	IIC_S2DOS05_ESCL_Lo;
	IIC_S2DOS05_ESCL_Lo;
	IIC_S2DOS05_ESDA_Hi;
	IIC_S2DOS05_ESCL_Hi;
	IIC_S2DOS05_ESCL_Hi;
	/* Function <- Input (SDA) */
	IIC_S2DOS05_ESDA_INP;

	IIC_S2DOS05_SCLL_SDAL();
}

void IIC_S2DOS05_ESetport(void)
{
	/* Pull Up/Down Disable SCL, SDA */
	GPIO_PUD_S2DOS05;

	IIC_S2DOS05_ESCL_Hi;
	IIC_S2DOS05_ESDA_Hi;

	/* Function <- Output (SCL) */
	IIC_S2DOS05_ESCL_OUTP;
	/* Function <- Output (SDA) */
	IIC_S2DOS05_ESDA_OUTP;

	Delay();
}

void IIC_S2DOS05_EWrite(unsigned char ChipId,
                        unsigned char IicAddr, unsigned char IicData)
{
	unsigned long i = 0;

	IIC_S2DOS05_EStart();

	/* write chip id */
	for (i = 7; i > 0; i--) {
		if ((ChipId >> i) & 0x0001)
			IIC_S2DOS05_EHigh();
		else
			IIC_S2DOS05_ELow();
	}

	/* write */
	IIC_S2DOS05_ELow();

	/* ACK */
	IIC_S2DOS05_EAck_write();

	/* write reg. addr. */
	for (i = 8; i > 0; i--) {
		if ((IicAddr >> (i - 1)) & 0x0001)
			IIC_S2DOS05_EHigh();
		else
			IIC_S2DOS05_ELow();
	}

	/* ACK */
	IIC_S2DOS05_EAck_write();

	/* write reg. data. */
	for (i = 8; i > 0; i--) {
		if ((IicData >> (i - 1)) & 0x0001)
			IIC_S2DOS05_EHigh();
		else
			IIC_S2DOS05_ELow();
	}

	/* ACK */
	IIC_S2DOS05_EAck_write();

	IIC_S2DOS05_EEnd();
}

void IIC_S2DOS05_ERead(unsigned char ChipId,
                       unsigned char IicAddr, unsigned char *IicData)
{
	unsigned long i = 0;
	unsigned long reg = 0;
	unsigned char data = 0;

	IIC_S2DOS05_EStart();

	/* write chip id */
	for (i = 7; i > 0; i--) {
		if ((ChipId >> i) & 0x0001)
			IIC_S2DOS05_EHigh();
		else
			IIC_S2DOS05_ELow();
	}

	/* write */
	IIC_S2DOS05_ELow();

	/* ACK */
	IIC_S2DOS05_EAck_write();

	/* write reg. addr. */
	for (i = 8; i > 0; i--) {
		if ((IicAddr >> (i - 1)) & 0x0001)
			IIC_S2DOS05_EHigh();
		else
			IIC_S2DOS05_ELow();
	}

	/* ACK */
	IIC_S2DOS05_EAck_write();

	IIC_S2DOS05_EStart();

	/* write chip id */
	for (i = 7; i > 0; i--) {
		if ((ChipId >> i) & 0x0001)
			IIC_S2DOS05_EHigh();
		else
			IIC_S2DOS05_ELow();
	}

	/* read */
	IIC_S2DOS05_EHigh();
	/* ACK */
	IIC_S2DOS05_EAck_write();

	/* read reg. data. */
	IIC_S2DOS05_ESDA_INP;

	IIC_S2DOS05_ESCL_Lo;
	IIC_S2DOS05_ESCL_Lo;
	Delay();

	for (i = 8; i > 0; i--) {
		IIC_S2DOS05_ESCL_Lo;
		IIC_S2DOS05_ESCL_Lo;
		Delay();
		IIC_S2DOS05_ESCL_Hi;
		IIC_S2DOS05_ESCL_Hi;
		Delay();
		reg = GPIO_DAT_S2DOS05;
		IIC_S2DOS05_ESCL_Hi;
		IIC_S2DOS05_ESCL_Hi;
		Delay();
		IIC_S2DOS05_ESCL_Lo;
		IIC_S2DOS05_ESCL_Lo;
		Delay();

		reg = (reg >> S2DOS05_GPIO_DAT_SHIFT) & 0x1;

		data |= reg << (i - 1);
	}

	/* ACK */
	IIC_S2DOS05_EAck_read();
	IIC_S2DOS05_ESDA_OUTP;

	IIC_S2DOS05_EEnd();

	*IicData = data;
}

void pmic_init_s2dos05(void)
{
	unsigned char reg;

	IIC_S2DOS05_ESetport();

	/* Display power set up */
	IIC_S2DOS05_ERead(S2DOS05_ADDR, S2DOS05_REG_EN, &reg);
	printf("S2DOS05_REG_EN def: 0x%x\n", reg);
	reg |= (LDO1_EN | LDO4_EN | BUCK_EN);
	IIC_S2DOS05_EWrite(S2DOS05_ADDR, S2DOS05_REG_EN, reg);
	IIC_S2DOS05_ERead(S2DOS05_ADDR, S2DOS05_REG_EN, &reg);
	printf("S2DOS05_REG_EN set: 0x%x\n", reg);
}
