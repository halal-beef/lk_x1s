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
#include <sys/types.h>
#include <platform/delay.h>
#include <platform/if_pmic_s2mu107.h>
#include <platform/gpio.h>
#include <platform/interrupts.h>
#include <platform/exynos9630.h>
#include <reg.h>

#define m_delay(a) u_delay((a) * 1000)

unsigned char rev_id;
unsigned char es_id;

static void Delay(void)
{
	unsigned long i = 0;
	for (i = 0; i < DELAY; i++)
		;
}

static void IIC_S2MU107_SCLH_SDAH(void)
{
	IIC_S2MU107_ESCL_Hi;
	IIC_S2MU107_ESDA_Hi;
	Delay();
}

static void IIC_S2MU107_SCLH_SDAL(void)
{
	IIC_S2MU107_ESCL_Hi;
	IIC_S2MU107_ESDA_Lo;
	Delay();
}

static void IIC_S2MU107_SCLL_SDAH(void)
{
	IIC_S2MU107_ESCL_Lo;
	IIC_S2MU107_ESDA_Hi;
	Delay();
}

static void IIC_S2MU107_SCLL_SDAL(void)
{
	IIC_S2MU107_ESCL_Lo;
	IIC_S2MU107_ESDA_Lo;
	Delay();
}

static void IIC_S2MU107_ELow(void)
{
	IIC_S2MU107_SCLL_SDAL();
	IIC_S2MU107_SCLH_SDAL();
	IIC_S2MU107_SCLH_SDAL();
	IIC_S2MU107_SCLL_SDAL();
}

static void IIC_S2MU107_EHigh(void)
{
	IIC_S2MU107_SCLL_SDAH();
	IIC_S2MU107_SCLH_SDAH();
	IIC_S2MU107_SCLH_SDAH();
	IIC_S2MU107_SCLL_SDAH();
}

static void IIC_S2MU107_EStart(void)
{
	IIC_S2MU107_SCLH_SDAH();
	IIC_S2MU107_SCLH_SDAL();
	Delay();
	IIC_S2MU107_SCLL_SDAL();
}

static void IIC_S2MU107_EEnd(void)
{
	IIC_S2MU107_SCLL_SDAL();
	IIC_S2MU107_SCLH_SDAL();
	Delay();
	IIC_S2MU107_SCLH_SDAH();
}

static void IIC_S2MU107_EAck_write(void)
{
	unsigned long ack = 0;

	/* Function <- Input */
	IIC_S2MU107_ESDA_INP;

	IIC_S2MU107_ESCL_Lo;
	Delay();
	IIC_S2MU107_ESCL_Hi;
	Delay();
	ack = GPIO_DAT_S2MU107;
	IIC_S2MU107_ESCL_Hi;
	Delay();
	IIC_S2MU107_ESCL_Hi;
	Delay();

	/* Function <- Output (SDA) */
	IIC_S2MU107_ESDA_OUTP;

	ack = (ack >> GPIO_DAT_SHIFT) & 0x1;

	IIC_S2MU107_SCLL_SDAL();
}

static void IIC_S2MU107_EAck_read(void)
{
	/* Function <- Output */
	IIC_S2MU107_ESDA_OUTP;

	IIC_S2MU107_ESCL_Lo;
	IIC_S2MU107_ESCL_Lo;
	IIC_S2MU107_ESDA_Hi;
	IIC_S2MU107_ESCL_Hi;
	IIC_S2MU107_ESCL_Hi;
	/* Function <- Input (SDA) */
	IIC_S2MU107_ESDA_INP;

	IIC_S2MU107_SCLL_SDAL();
}

void IIC_S2MU107_ESetport(void)
{
	/* Pull Up/Down Disable SCL, SDA */
	GPIO_PUD_S2MU107;

	IIC_S2MU107_ESCL_Hi;
	IIC_S2MU107_ESDA_Hi;

	/* Function <- Output (SCL) */
	IIC_S2MU107_ESCL_OUTP;
	/* Function <- Output (SDA) */
	IIC_S2MU107_ESDA_OUTP;

	Delay();
}

void IIC_S2MU107_EWrite(unsigned char ChipId,
		unsigned char IicAddr, unsigned char IicData)
{
	unsigned long i = 0;

	IIC_S2MU107_EStart();

	/* write chip id */
	for (i = 7; i > 0; i--) {
		if ((ChipId >> i) & 0x0001)
			IIC_S2MU107_EHigh();
		else
			IIC_S2MU107_ELow();
	}

	/* write */
	IIC_S2MU107_ELow();

	/* ACK */
	IIC_S2MU107_EAck_write();

	/* write reg. addr. */
	for (i = 8; i > 0; i--) {
		if ((IicAddr >> (i-1)) & 0x0001)
			IIC_S2MU107_EHigh();
		else
			IIC_S2MU107_ELow();
	}

	/* ACK */
	IIC_S2MU107_EAck_write();

	/* write reg. data. */
	for (i = 8; i > 0; i--) {
		if ((IicData >> (i-1)) & 0x0001)
			IIC_S2MU107_EHigh();
		else
			IIC_S2MU107_ELow();
	}

	/* ACK */
	IIC_S2MU107_EAck_write();

	IIC_S2MU107_EEnd();
}

void IIC_S2MU107_ERead(unsigned char ChipId,
		unsigned char IicAddr, unsigned char *IicData)
{
	unsigned long i = 0;
	unsigned long reg = 0;
	unsigned char data = 0;

	IIC_S2MU107_EStart();

	/* write chip id */
	for (i = 7; i > 0; i--) {
		if ((ChipId >> i) & 0x0001)
			IIC_S2MU107_EHigh();
		else
			IIC_S2MU107_ELow();
	}

	/* write */
	IIC_S2MU107_ELow();

	/* ACK */
	IIC_S2MU107_EAck_write();

	/* write reg. addr. */
	for (i = 8; i > 0; i--) {
		if ((IicAddr >> (i-1)) & 0x0001)
			IIC_S2MU107_EHigh();
		else
			IIC_S2MU107_ELow();
	}

	/* ACK */
	IIC_S2MU107_EAck_write();

	IIC_S2MU107_EStart();

	/* write chip id */
	for (i = 7; i > 0; i--) {
		if ((ChipId >> i) & 0x0001)
			IIC_S2MU107_EHigh();
		else
			IIC_S2MU107_ELow();
	}

	/* read */
	IIC_S2MU107_EHigh();
	/* ACK */
	IIC_S2MU107_EAck_write();

	/* read reg. data. */
	IIC_S2MU107_ESDA_INP;

	IIC_S2MU107_ESCL_Lo;
	IIC_S2MU107_ESCL_Lo;
	Delay();

	for (i = 8; i > 0; i--) {
		IIC_S2MU107_ESCL_Lo;
		IIC_S2MU107_ESCL_Lo;
		Delay();
		IIC_S2MU107_ESCL_Hi;
		IIC_S2MU107_ESCL_Hi;
		Delay();
		reg = GPIO_DAT_S2MU107;
		IIC_S2MU107_ESCL_Hi;
		IIC_S2MU107_ESCL_Hi;
		Delay();
		IIC_S2MU107_ESCL_Lo;
		IIC_S2MU107_ESCL_Lo;
		Delay();

		reg = (reg >> GPIO_DAT_SHIFT) & 0x1;

		data |= reg << (i-1);
	}

	/* ACK */
	IIC_S2MU107_EAck_read();
	IIC_S2MU107_ESDA_OUTP;

	IIC_S2MU107_EEnd();

	*IicData = data;
}

void s2mu107_sc_set_mode(int mode)
{
    u8 reg, val, tmp = 0;

    if (mode != S2MU107_CHG_MODE_OFF &&
            mode != S2MU107_CHG_MODE_BUCK &&
            mode != S2MU107_CHG_MODE_CHG) {
        printf("%s: wrong charger mode! Stop!\n",
                __func__);
        return;
    }

    IIC_S2MU107_ESetport();

	IIC_S2MU107_ERead(S2MU107_CHG_R_ADDR, S2MU107_SC_CTRL0, &val);

	if((mode == S2MU107_CHG_MODE_OFF) || (mode == S2MU107_CHG_MODE_BUCK)) {
		/* Set Async mode */
		IIC_S2MU107_ERead(S2MU107_CHG_R_ADDR, 0x3A, &reg);
		reg &= ~0x03;
		reg |= (0x3 << 0);
		IIC_S2MU107_EWrite(S2MU107_CHG_W_ADDR, 0x3A, reg);
		m_delay(20);
	}
	if(((val & 0x0F) == S2MU107_CHG_MODE_CHG) && (mode == S2MU107_CHG_MODE_BUCK)) {
		/* T_EN_CCR */
		IIC_S2MU107_ERead(S2MU107_CHG_R_ADDR, 0x34, &reg);
		reg &= ~0x0C;
		reg |= (0x3 << 2);
		IIC_S2MU107_EWrite(S2MU107_CHG_W_ADDR, 0x34, reg);
		tmp = 1;
	}

    val &= ~0x0F;
    val |= mode;
    IIC_S2MU107_EWrite(S2MU107_CHG_W_ADDR, S2MU107_SC_CTRL0, val);

	if(tmp == 1) {
		m_delay(1);
		/* T_EN_CCR */
		IIC_S2MU107_ERead(S2MU107_CHG_R_ADDR, 0x34, &reg);
		reg &= ~0x0C;
		reg |= (0x1 << 2);
		IIC_S2MU107_EWrite(S2MU107_CHG_W_ADDR, 0x34, reg);
	}
	if(mode == S2MU107_CHG_MODE_CHG) {
		/* Set Auto Sync mode */
		IIC_S2MU107_ERead(S2MU107_CHG_R_ADDR, 0x3A, &reg);
		reg &= ~0x03;
		reg |= (0x1 << 0);
		IIC_S2MU107_EWrite(S2MU107_CHG_W_ADDR, 0x3A, reg);
		printf("%s: Set Auto Sync mode 0x3A = 0x%02x\n", __func__, reg);
	}

    IIC_S2MU107_ERead(S2MU107_CHG_R_ADDR, S2MU107_SC_CTRL0, &reg);
    printf("%s: S2MU107_SC_CTRL0 = 0x%02x\n", __func__, reg);
}

void s2mu107_sc_init(void)
{
	unsigned char reg;

	IIC_S2MU107_ESetport();

	IIC_S2MU107_ERead(S2MU107_COMMON_R_ADDR, 0xF5, &reg);
	rev_id = reg & 0x0F;
	es_id = (reg & 0xF0) >> 4;
	printf("%s: rev_id(%d), es_id(%d), 0xF5(0x%x)\n", __func__, rev_id, es_id, reg);

	if(rev_id == 0) {
		/* open input TR */
		IIC_S2MU107_ERead(S2MU107_CHG_R_ADDR, 0x5F, &reg);
		reg |= (1 << 5);
		IIC_S2MU107_EWrite(S2MU107_CHG_W_ADDR, 0x5F, reg);

		/* Buck NTR off */
		IIC_S2MU107_ERead(S2MU107_CHG_R_ADDR, 0x3A, &reg);
		reg &= 0xFC;
		reg |= (0x3 << 0);
		IIC_S2MU107_EWrite(S2MU107_CHG_W_ADDR, 0x3A, reg);
		printf("%s, 0x3A(%x)\n", __func__, reg);
	}

	if(rev_id <= 1) {
		/* OTG w/a */
		IIC_S2MU107_ERead(S2MU107_CHG_R_ADDR, 0x90, &reg);
		reg &= ~(1 << 2);
		IIC_S2MU107_EWrite(S2MU107_CHG_W_ADDR, 0x90, reg);

		/* OTG w/a */
		IIC_S2MU107_ERead(S2MU107_CHG_R_ADDR, 0x7D, &reg);
		reg &= ~(1 << 3);
		IIC_S2MU107_EWrite(S2MU107_CHG_W_ADDR, 0x7D, reg);

		/* change Buck switching slope */
		IIC_S2MU107_ERead(S2MU107_CHG_R_ADDR, 0x89, &reg);
		reg &= 0x0F;
		reg |= (0xA << 4);
		IIC_S2MU107_EWrite(S2MU107_CHG_W_ADDR, 0x89, reg);

		/* manual reset disable */
		IIC_S2MU107_ERead(S2MU107_COMMON_R_ADDR, 0xE5, &reg);
		reg &= ~(1 << 3);
		IIC_S2MU107_EWrite(S2MU107_COMMON_W_ADDR, 0xE5, reg);
		printf("%s, 0xE5(%x)\n", __func__, reg);
	}

	if((rev_id == 0) || ((rev_id == 1) && (es_id == 0))) {
		/* change Min off time */
		IIC_S2MU107_ERead(S2MU107_CHG_R_ADDR, 0x81, &reg);
		reg &= 0x1F;
		reg |= (0x7 << 5);
		IIC_S2MU107_EWrite(S2MU107_CHG_W_ADDR, 0x81, reg);

		/* change IN2BAT voltage trim */
		IIC_S2MU107_ERead(S2MU107_CHG_R_ADDR, 0x6D, &reg);
		reg &= 0x0F;
		reg |= (0xE << 4);
		IIC_S2MU107_EWrite(S2MU107_CHG_W_ADDR, 0x6D, reg);

		/* change WCIN2BAT voltage trim */
		IIC_S2MU107_ERead(S2MU107_CHG_R_ADDR, 0x70, &reg);
		reg &= 0x0F;
		reg |= (0xE << 4);
		IIC_S2MU107_EWrite(S2MU107_CHG_W_ADDR, 0x70, reg);
		printf("%s, 0x70(%x)\n", __func__, reg);
	}

	if(rev_id == 1) {
		/* FLED Work-around */
		IIC_S2MU107_ERead(S2MU107_CHG_R_ADDR, 0xD5, &reg);
		reg |= (1 << 3);
		IIC_S2MU107_EWrite(S2MU107_CHG_W_ADDR, 0xD5, reg);

		/* FLED Work-around */
		IIC_S2MU107_ERead(S2MU107_CHG_R_ADDR, 0x76, &reg);
		reg |= (1 << 5);
		IIC_S2MU107_EWrite(S2MU107_CHG_W_ADDR, 0x76, reg);

		/* FLED Work-around */
		IIC_S2MU107_ERead(S2MU107_COMMON_R_ADDR, 0x24, &reg);
		reg |= (1 << 6);
		IIC_S2MU107_EWrite(S2MU107_COMMON_W_ADDR, 0x24, reg);
		printf("%s, 0x24(%x)\n", __func__, reg);
	}

	if((rev_id == 1) && (es_id == 1)) {
		/* WCIN IVR 4.5V */
		IIC_S2MU107_ERead(S2MU107_CHG_R_ADDR, 0x1C, &reg);
		reg &= 0xF8;
		reg |= (0x5 << 0);
		IIC_S2MU107_EWrite(S2MU107_CHG_W_ADDR, 0x1C, reg);

		/* CHGIN IVR 4.5V */
		IIC_S2MU107_ERead(S2MU107_COMMON_R_ADDR, 0x1C, &reg);
		reg &= 0xC7;
		reg |= (0x5 << 3);
		IIC_S2MU107_EWrite(S2MU107_COMMON_W_ADDR, 0x1C, reg);
		printf("%s, 0x1C(%x)\n", __func__, reg);
	}

	/* Async <-> Sync Debounce time set */
	IIC_S2MU107_ERead(S2MU107_CHG_R_ADDR, 0x98, &reg);
	reg &= 0x0F;
	reg |= (0x3 << 4);
	IIC_S2MU107_EWrite(S2MU107_CHG_W_ADDR, 0x98, reg);

	s2mu107_sc_set_mode(S2MU107_CHG_MODE_CHG);
}

void muic_sw_open (void)
{
	unsigned char reg;

	IIC_S2MU107_ESetport();
	IIC_S2MU107_ERead(S2MU107_MUIC_R_ADDR, S2MU107_MUIC_CTRL1, &reg);
	reg &= ~(0x1 << 2);
	IIC_S2MU107_EWrite(S2MU107_MUIC_W_ADDR, S2MU107_MUIC_CTRL1, reg);
	IIC_S2MU107_ERead(S2MU107_MUIC_R_ADDR, S2MU107_MANUAL_SW_CTRL, &reg);
	reg &= ~(0x7 << 2);
	reg &= ~(0x7 << 5);
	reg |= 0x0 << 2;
	reg |= 0x0 << 5;
	IIC_S2MU107_EWrite(S2MU107_MUIC_W_ADDR, S2MU107_MANUAL_SW_CTRL, reg);
}

void muic_sw_usb (void)
{
	unsigned char reg;

	IIC_S2MU107_ESetport();
	IIC_S2MU107_ERead(S2MU107_MUIC_R_ADDR, S2MU107_MUIC_CTRL1, &reg);
	reg &= ~(0x1 << 2);
	IIC_S2MU107_EWrite(S2MU107_MUIC_W_ADDR, S2MU107_MUIC_CTRL1, reg);
	IIC_S2MU107_ERead(S2MU107_MUIC_R_ADDR, S2MU107_MANUAL_SW_CTRL, &reg);
	reg &= ~(0x7 << 2);
	reg &= ~(0x7 << 5);
	reg |= 0x1 << 2;
	reg |= 0x1 << 5;
	IIC_S2MU107_EWrite(S2MU107_MUIC_W_ADDR, S2MU107_MANUAL_SW_CTRL, reg);
}

void muic_sw_uart (void)
{
	unsigned char reg;

	IIC_S2MU107_ESetport();
	IIC_S2MU107_ERead(S2MU107_MUIC_R_ADDR, S2MU107_MUIC_CTRL1, &reg);
	reg &= ~(0x1 << 2);
	IIC_S2MU107_EWrite(S2MU107_MUIC_W_ADDR, S2MU107_MUIC_CTRL1, reg);
	IIC_S2MU107_ERead(S2MU107_MUIC_R_ADDR, S2MU107_MANUAL_SW_CTRL, &reg);
	reg &= ~(0x7 << 2);
	reg &= ~(0x7 << 5);
	reg |= 0x2 << 2;
	reg |= 0x2 << 5;
	IIC_S2MU107_EWrite(S2MU107_MUIC_W_ADDR, S2MU107_MANUAL_SW_CTRL, reg);
}
