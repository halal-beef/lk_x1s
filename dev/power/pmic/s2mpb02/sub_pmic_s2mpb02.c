/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <debug.h>
#include <dev/sub_pmic_s2mpb02.h>

void IIC_Delay(void)
{
	unsigned long i;

	for (i = 0; i < IIC_DELAY; i++)
		;
}

void IIC_SCLH_SDAH(void)
{
	IIC_ESCL_Hi;
	IIC_ESDA_Hi;
	IIC_Delay();
}

void IIC_SCLH_SDAL(void)
{
	IIC_ESCL_Hi;
	IIC_ESDA_Lo;
	IIC_Delay();
}

void IIC_SCLL_SDAH(void)
{
	IIC_ESCL_Lo;
	IIC_ESDA_Hi;
	IIC_Delay();
}

void IIC_SCLL_SDAL(void)
{
	IIC_ESCL_Lo;
	IIC_ESDA_Lo;
	IIC_Delay();
}

void IIC_ELow(void)
{
	IIC_SCLL_SDAL();
	IIC_SCLH_SDAL();
	IIC_SCLH_SDAL();
	IIC_SCLL_SDAL();
}

void IIC_EHigh(void)
{
	IIC_SCLL_SDAH();
	IIC_SCLH_SDAH();
	IIC_SCLH_SDAH();
	IIC_SCLL_SDAH();
}

void IIC_EStart(void)
{
	IIC_SCLH_SDAH();
	IIC_SCLH_SDAL();
	IIC_Delay();
	IIC_SCLL_SDAL();
}

void IIC_EEnd(void)
{
	IIC_SCLL_SDAL();
	IIC_SCLH_SDAL();
	IIC_Delay();
	IIC_SCLH_SDAH();
}

void IIC_EAck_write(void)
{
	unsigned long ack;

	/* Function <- Input */
	IIC_ESDA_INP;

	IIC_ESCL_Lo;
	IIC_Delay();
	IIC_ESCL_Hi;
	IIC_Delay();
	ack = IIC_GPIO_DAT;
	IIC_ESCL_Hi;
	IIC_Delay();
	IIC_ESCL_Hi;
	IIC_Delay();

	/* Function <- Output (SDA) */
	IIC_ESDA_OUTP;

	ack = (ack >> IIC_GPIO_DAT_SHIFT) & 0x1;

	IIC_SCLL_SDAL();
}

void IIC_EAck_read(void)
{
	/* Function <- Output */
	IIC_ESDA_OUTP;

	IIC_ESCL_Lo;
	IIC_ESCL_Lo;
	IIC_ESDA_Hi;
	IIC_ESCL_Hi;
	IIC_ESCL_Hi;
	/* Function <- Input (SDA) */
	IIC_ESDA_INP;

	IIC_SCLL_SDAL();
}

void IIC_ESetport(void)
{
	/* Pull Up/Down Disable SCL, SDA */
	IIC_DIS_GPIO_PUD;

	IIC_ESCL_Hi;
	IIC_ESDA_Hi;

	/* Function <- Output (SCL) */
	IIC_ESCL_OUTP;
	/* Function <- Output (SDA) */
	IIC_ESDA_OUTP;

	IIC_Delay();
}

void IIC_EWrite(unsigned char ChipId,
                unsigned char IicAddr, unsigned char IicData)
{
	unsigned long i;

	IIC_EStart();

	/* write chip id */
	for (i = 7; i > 0; i--) {
		if ((ChipId >> i) & 0x0001)
			IIC_EHigh();
		else
			IIC_ELow();
	}

	/* write */
	IIC_ELow();

	/* ACK */
	IIC_EAck_write();

	/* write reg. addr. */
	for (i = 8; i > 0; i--) {
		if ((IicAddr >> (i - 1)) & 0x0001)
			IIC_EHigh();
		else
			IIC_ELow();
	}

	/* ACK */
	IIC_EAck_write();

	/* write reg. data. */
	for (i = 8; i > 0; i--) {
		if ((IicData >> (i - 1)) & 0x0001)
			IIC_EHigh();
		else
			IIC_ELow();
	}

	/* ACK */
	IIC_EAck_write();

	IIC_EEnd();
}

void IIC_ERead(unsigned char ChipId,
               unsigned char IicAddr, unsigned char *IicData)
{
	unsigned long i, reg;
	unsigned char data = 0;

	IIC_EStart();

	/* write chip id */
	for (i = 7; i > 0; i--) {
		if ((ChipId >> i) & 0x0001)
			IIC_EHigh();
		else
			IIC_ELow();
	}

	/* write */
	IIC_ELow();

	/* ACK */
	IIC_EAck_write();

	/* write reg. addr. */
	for (i = 8; i > 0; i--) {
		if ((IicAddr >> (i - 1)) & 0x0001)
			IIC_EHigh();
		else
			IIC_ELow();
	}

	/* ACK */
	IIC_EAck_write();

	IIC_EStart();

	/* write chip id */
	for (i = 7; i > 0; i--) {
		if ((ChipId >> i) & 0x0001)
			IIC_EHigh();
		else
			IIC_ELow();
	}

	/* read */
	IIC_EHigh();
	/* ACK */
	IIC_EAck_write();

	/* read reg. data. */
	IIC_ESDA_INP;

	IIC_ESCL_Lo;
	IIC_ESCL_Lo;
	IIC_Delay();

	for (i = 8; i > 0; i--) {
		IIC_ESCL_Lo;
		IIC_ESCL_Lo;
		IIC_Delay();
		IIC_ESCL_Hi;
		IIC_ESCL_Hi;
		IIC_Delay();
		reg = IIC_GPIO_DAT;
		IIC_ESCL_Hi;
		IIC_ESCL_Hi;
		IIC_Delay();
		IIC_ESCL_Lo;
		IIC_ESCL_Lo;
		IIC_Delay();

		reg = (reg >> IIC_GPIO_DAT_SHIFT) & 0x1;

		data |= reg << (i - 1);
	}

	/* ACK */
	IIC_EAck_read();
	IIC_ESDA_OUTP;

	IIC_EEnd();

	*IicData = data;
}

void sub_pmic_s2mpb02_init(void)
{
	unsigned char reg;

	IIC_ESetport();

	/* WA for S2MPB02 write fail - write fake odd number value  before real value */
	IIC_EWrite(S2MPB02_W_ADDR, S2MPB02_BUCK1_CTRL, 0x59);
	IIC_EWrite(S2MPB02_W_ADDR, S2MPB02_BUCK1_CTRL, 0xD8);

	IIC_ERead(S2MPB02_W_ADDR, S2MPB02_BUCK1_CTRL, &reg);
	printf("S2MPB02_BUCK1_CTRL: 0x%x\n", reg);

	/* WA for S2MPB02 write fail - write fake odd number value  before real value */
	IIC_EWrite(S2MPB02_W_ADDR, S2MPB02_LDO6_CTRL, 0x31);
	IIC_EWrite(S2MPB02_W_ADDR, S2MPB02_LDO6_CTRL, 0xB0);

	IIC_ERead(S2MPB02_W_ADDR, S2MPB02_LDO6_CTRL, &reg);
	printf("S2MPB02_LDO6_CTRL: 0x%x\n", reg);

	/* WA for S2MPB02 write fail - write fake odd number value  before real value */
	IIC_EWrite(S2MPB02_W_ADDR, S2MPB02_LDO9_CTRL, 0x31);
	IIC_EWrite(S2MPB02_W_ADDR, S2MPB02_LDO9_CTRL, 0xB0);

	IIC_ERead(S2MPB02_W_ADDR, S2MPB02_LDO9_CTRL, &reg);
	printf("S2MPB02_LDO9_CTRL: 0x%x\n", reg);

	/* WA for S2MPB02 write fail - write fake odd number value  before real value */
	IIC_EWrite(S2MPB02_W_ADDR, S2MPB02_LDO11_CTRL, 0x59);
	IIC_EWrite(S2MPB02_W_ADDR, S2MPB02_LDO11_CTRL, 0xD8);

	IIC_ERead(S2MPB02_W_ADDR, S2MPB02_LDO11_CTRL, &reg);
	printf("S2MPB02_LDO11_CTRL: 0x%x\n", reg);

	/* WA for S2MPB02 write fail - write fake odd number value  before real value */
	IIC_EWrite(S2MPB02_W_ADDR, S2MPB02_LDO18_CTRL, 0x6D);
	IIC_EWrite(S2MPB02_W_ADDR, S2MPB02_LDO18_CTRL, 0xEC);

	IIC_ERead(S2MPB02_W_ADDR, S2MPB02_LDO18_CTRL, &reg);
	printf("S2MPB02_LDO18_CTRL: 0x%x\n", reg);
}
