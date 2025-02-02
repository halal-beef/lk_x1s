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

#include <lk/reg.h>
#include <lk/debug.h>
#include <sys/types.h>
#include <platform/delay.h>
#include <dev/fg_s2mu106.h>
#include <dev/if_pmic_s2mu106.h>
//#include <platform/pmic_s2mpu09.h>
#include <platform/sfr.h>
//#include <platform/wdt_recovery.h>

#define min(a, b) (a) < (b) ? a : b
#define m_delay(a) u_delay((a) * 1000)
#define DEAD_BATTERY_VBAT 3500
#define DEAD_BATTERY_INPUT 500
#define DEAD_BATTERY_CHG 500
#define DEAD_BATTERY_DELAY 500

/* battery data for */
/* 0x92 ~ 0xe9: BAT_PARAM */
static u8 model_param1[] = {
	88, 11, 214, 10, 84, 10, 218, 9, 100, 9,
	243, 8, 140, 8, 34, 8, 205, 7, 123, 7,
	8, 7, 198, 6, 151, 6, 113, 6, 84, 6,
	57, 6, 15, 6, 234, 5, 175, 5, 133, 5,
	253, 4, 178, 1, 105, 8, 0, 8, 151, 7,
	45, 7, 196, 6, 90, 6, 241, 5, 135, 5,
	30, 5, 181, 4, 75, 4, 226, 3, 120, 3,
	15, 3, 166, 2, 60, 2, 211, 1, 105, 1,
	0, 1, 150, 0, 45, 0, 218, 15
};

static u8 model_param2[] = {
	33, 33, 33, 33, 33, 33, 33, 33, 33, 32,
	32, 33, 33, 33, 33, 34, 34, 34, 34, 36,
	38, 124
};

int soc_arr_val[TABLE_SIZE] = {
	10514, 10000, 9485, 8970, 8455, 7941, 7426, 6911, 6396, 5882,
	5367, 4852, 4338, 3823, 3308, 2793, 2279, 1764, 1249, 734,
	220, -185
}; // * 0.01%
int ocv_arr_val[TABLE_SIZE] = {
	44179, 43544, 42910, 42315, 41740, 41188, 40683, 40167, 39749, 39351,
	38791, 38469, 38237, 38054, 37911, 37778, 37574, 37390, 37106, 36900,
	36236, 32120
}; // *0.1mV

static int model_param_ver = 0x03;

/* For 0x0E ~ 0x11 */
static u8 batcap[] = {0xB0, 0x36, 0xAC, 0x0D};
/* For 0x44 ~ 0x45 */
static u8 accum[] = {0x00, 0x08};

static void Delay(void)
{
	unsigned long i = 0;
	for (i = 0; i < DELAY; i++)
		;
}

static void IIC_S2MU106_FG_SCLH_SDAH(void)
{
	IIC_S2MU106_FG_ESCL_Hi;
	IIC_S2MU106_FG_ESDA_Hi;
	Delay();
}

static void IIC_S2MU106_FG_SCLH_SDAL(void)
{
	IIC_S2MU106_FG_ESCL_Hi;
	IIC_S2MU106_FG_ESDA_Lo;
	Delay();
}

static void IIC_S2MU106_FG_SCLL_SDAH(void)
{
	IIC_S2MU106_FG_ESCL_Lo;
	IIC_S2MU106_FG_ESDA_Hi;
	Delay();
}

static void IIC_S2MU106_FG_SCLL_SDAL(void)
{
	IIC_S2MU106_FG_ESCL_Lo;
	IIC_S2MU106_FG_ESDA_Lo;
	Delay();
}

static void IIC_S2MU106_FG_ELow(void)
{
	IIC_S2MU106_FG_SCLL_SDAL();
	IIC_S2MU106_FG_SCLH_SDAL();
	IIC_S2MU106_FG_SCLH_SDAL();
	IIC_S2MU106_FG_SCLL_SDAL();
}

static void IIC_S2MU106_FG_EHigh(void)
{
	IIC_S2MU106_FG_SCLL_SDAH();
	IIC_S2MU106_FG_SCLH_SDAH();
	IIC_S2MU106_FG_SCLH_SDAH();
	IIC_S2MU106_FG_SCLL_SDAH();
}

static void IIC_S2MU106_FG_EStart(void)
{
	IIC_S2MU106_FG_SCLH_SDAH();
	IIC_S2MU106_FG_SCLH_SDAL();
	Delay();
	IIC_S2MU106_FG_SCLL_SDAL();
}

static void IIC_S2MU106_FG_EEnd(void)
{
	IIC_S2MU106_FG_SCLL_SDAL();
	IIC_S2MU106_FG_SCLH_SDAL();
	Delay();
	IIC_S2MU106_FG_SCLH_SDAH();
}

static void IIC_S2MU106_FG_EAck_write(void)
{
	unsigned long ack = 0;

	/* Function <- Input */
	IIC_S2MU106_FG_ESDA_INP;

	IIC_S2MU106_FG_ESCL_Lo;
	Delay();
	IIC_S2MU106_FG_ESCL_Hi;
	Delay();
	ack = GPIO_DAT_FG_S2MU106;
	IIC_S2MU106_FG_ESCL_Hi;
	Delay();
	IIC_S2MU106_FG_ESCL_Hi;
	Delay();

	/* Function <- Output (SDA) */
	IIC_S2MU106_FG_ESDA_OUTP;

	ack = (ack >> GPIO_DAT_FG_SHIFT_S2MU106) & 0x1;

	IIC_S2MU106_FG_SCLL_SDAL();
}

static void IIC_S2MU106_FG_EAck_read(void)
{
	/* Function <- Output */
	IIC_S2MU106_FG_ESDA_OUTP;

	IIC_S2MU106_FG_ESCL_Lo;
	IIC_S2MU106_FG_ESCL_Lo;
	IIC_S2MU106_FG_ESDA_Hi;
	IIC_S2MU106_FG_ESCL_Hi;
	IIC_S2MU106_FG_ESCL_Hi;
	/* Function <- Input (SDA) */
	IIC_S2MU106_FG_ESDA_INP;

	IIC_S2MU106_FG_SCLL_SDAL();
}

void IIC_S2MU106_FG_ESetport(void)
{
	/* Pull Up/Down Disable SCL, SDA */
	GPIO_PUD_FG_S2MU106;

	IIC_S2MU106_FG_ESCL_Hi;
	IIC_S2MU106_FG_ESDA_Hi;

	/* Function <- Output (SCL) */
	IIC_S2MU106_FG_ESCL_OUTP;
	/* Function <- Output (SDA) */
	IIC_S2MU106_FG_ESDA_OUTP;

	Delay();
}

void IIC_S2MU106_FG_EWrite(unsigned char ChipId,
		unsigned char IicAddr, unsigned char IicData)
{
	unsigned long i = 0;

	IIC_S2MU106_FG_EStart();

	/* write chip id */
	for (i = 7; i > 0; i--) {
		if ((ChipId >> i) & 0x0001)
			IIC_S2MU106_FG_EHigh();
		else
			IIC_S2MU106_FG_ELow();
	}

	/* write */
	IIC_S2MU106_FG_ELow();

	/* ACK */
	IIC_S2MU106_FG_EAck_write();

	/* write reg. addr. */
	for (i = 8; i > 0; i--) {
		if ((IicAddr >> (i-1)) & 0x0001)
			IIC_S2MU106_FG_EHigh();
		else
			IIC_S2MU106_FG_ELow();
	}

	/* ACK */
	IIC_S2MU106_FG_EAck_write();

	/* write reg. data. */
	for (i = 8; i > 0; i--) {
		if ((IicData >> (i-1)) & 0x0001)
			IIC_S2MU106_FG_EHigh();
		else
			IIC_S2MU106_FG_ELow();
	}

	/* ACK */
	IIC_S2MU106_FG_EAck_write();

	IIC_S2MU106_FG_EEnd();
}

void IIC_S2MU106_FG_ERead(unsigned char ChipId,
		unsigned char IicAddr, unsigned char *IicData)
{
	unsigned long i = 0;
	unsigned long reg = 0;
	unsigned char data = 0;

	IIC_S2MU106_FG_EStart();

	/* write chip id */
	for (i = 7; i > 0; i--) {
		if ((ChipId >> i) & 0x0001)
			IIC_S2MU106_FG_EHigh();
		else
			IIC_S2MU106_FG_ELow();
	}

	/* write */
	IIC_S2MU106_FG_ELow();

	/* ACK */
	IIC_S2MU106_FG_EAck_write();

	/* write reg. addr. */
	for (i = 8; i > 0; i--) {
		if ((IicAddr >> (i-1)) & 0x0001)
			IIC_S2MU106_FG_EHigh();
		else
			IIC_S2MU106_FG_ELow();
	}

	/* ACK */
	IIC_S2MU106_FG_EAck_write();

	IIC_S2MU106_FG_EStart();

	/* write chip id */
	for (i = 7; i > 0; i--) {
		if ((ChipId >> i) & 0x0001)
			IIC_S2MU106_FG_EHigh();
		else
			IIC_S2MU106_FG_ELow();
	}

	/* read */
	IIC_S2MU106_FG_EHigh();
	/* ACK */
	IIC_S2MU106_FG_EAck_write();

	/* read reg. data. */
	IIC_S2MU106_FG_ESDA_INP;

	IIC_S2MU106_FG_ESCL_Lo;
	IIC_S2MU106_FG_ESCL_Lo;
	Delay();

	for (i = 8; i > 0; i--) {
		IIC_S2MU106_FG_ESCL_Lo;
		IIC_S2MU106_FG_ESCL_Lo;
		Delay();
		IIC_S2MU106_FG_ESCL_Hi;
		IIC_S2MU106_FG_ESCL_Hi;
		Delay();
		reg = GPIO_DAT_FG_S2MU106;
		IIC_S2MU106_FG_ESCL_Hi;
		IIC_S2MU106_FG_ESCL_Hi;
		Delay();
		IIC_S2MU106_FG_ESCL_Lo;
		IIC_S2MU106_FG_ESCL_Lo;
		Delay();

		reg = (reg >> GPIO_DAT_FG_SHIFT_S2MU106) & 0x1;

		data |= reg << (i-1);
	}

	/* ACK */
	IIC_S2MU106_FG_EAck_read();
	IIC_S2MU106_FG_ESDA_OUTP;

	IIC_S2MU106_FG_EEnd();

	*IicData = data;
}

static u16 ReadFGRegisterWord(u8 addr)
{
	u8 msb = 0;
	u8 lsb = 0;
	IIC_S2MU106_FG_ERead(S2MU106_FG_SLAVE_ADDR_R, addr++, &lsb);
	IIC_S2MU106_FG_ERead(S2MU106_FG_SLAVE_ADDR_R, addr, &msb);
	return ((msb << 8) | lsb);
}

static u32 s2mu106_get_vbat(void)
{
	u16 data;
	u32 vbat = 0;

	data = ReadFGRegisterWord(S2MU106_REG_RVBAT);
	vbat = (data * 1000) >> 13;

	/* If vbat is larger than 6.1V, set vbat 0V */
	if (vbat > 6100)
		vbat = 0;

	printf("%s: vbat (%d), src(0x%02X)\n", __func__, vbat, data);

	return vbat;
}

static int s2mu106_get_avgvbat(void)
{
	u16 compliment;
	int avgvbat = 0;

	IIC_S2MU106_FG_EWrite(S2MU106_FG_SLAVE_ADDR_W, S2MU106_REG_MONOUT_SEL, 0x16);
	u_delay(50000);

	compliment = ReadFGRegisterWord(S2MU106_REG_MONOUT);

	printf("%s: MONOUT(0x%4x)\n", __func__, compliment);

	avgvbat = (compliment * 1000) >> 12;

	IIC_S2MU106_FG_EWrite(S2MU106_FG_SLAVE_ADDR_W, S2MU106_REG_MONOUT_SEL, 0x10);

	/* If avgvbat is larger than 6.1V, set avgvbat 0V */
	if (avgvbat > 6100)
		avgvbat = 0;

	printf("%s: avg vbat (%d)mV\n", __func__, avgvbat);

	return avgvbat;
}

static int s2mu106_get_current(void)
{
	u16 compliment;
	int curr = 0;

	compliment = ReadFGRegisterWord(S2MU106_REG_RCUR_CC);

	printf("%s: rCUR_CC(0x%4x)\n", __func__, compliment);

	if (compliment & (0x1 << 15)) { /* Charging */
		curr = ((~compliment) & 0xFFFF) + 1;
		curr = (curr * 1000) >> 12;
	} else { /* dischaging */
		curr = compliment & 0x7FFF;
		curr = (curr * (-1000)) >> 12;
	}

	printf("%s: current (%d)mA\n", __func__, curr);

	return curr;
}

static int s2mu106_get_avgcurrent(void)
{
	u16 compliment;
	int curr = 0;

	IIC_S2MU106_FG_EWrite(S2MU106_FG_SLAVE_ADDR_W, S2MU106_REG_MONOUT_SEL, 0x17);
	u_delay(50000);

	compliment = ReadFGRegisterWord(S2MU106_REG_MONOUT);

	printf("%s: MONOUT(0x%4x)\n", __func__, compliment);

	if (compliment & (0x1 << 15)) { /* Charging */
		curr = ((~compliment) & 0xFFFF) + 1;
		curr = (curr * 1000) >> 12;
	} else { /* dischaging */
		curr = compliment & 0x7FFF;
		curr = (curr * (-1000)) >> 12;
	}
	IIC_S2MU106_FG_EWrite(S2MU106_FG_SLAVE_ADDR_W, S2MU106_REG_MONOUT_SEL, 0x10);
	printf("%s: avg current (%d)mA\n", __func__, curr);

	return curr;
}

static u32 s2mu106_get_soc(void)
{
	u16 compliment;
	int rsoc;

	compliment = ReadFGRegisterWord(S2MU106_REG_RSOC_CC);

	/* data[] store 2's compliment format number */
	if (compliment & (0x1 << 15)) {
		/* Negative */
		rsoc = ((~compliment) & 0xFFFF) + 1;
		rsoc = (rsoc * (-10000)) >> 14;
	} else {
		rsoc = compliment & 0x7FFF;
		rsoc = (rsoc * 10000) >> 14;
	}

	printf("%s: raw capacity (0x%x:%d)\n", __func__,
			compliment, rsoc);

	return min(rsoc, 10000);
}

static int s2mu106_check_por(void)
{
	u8 por_state;
	u8 reg_1E;

	IIC_S2MU106_FG_ERead(S2MU106_FG_SLAVE_ADDR_R, 0x1F, &por_state);
	printf("%s: 0x1F = 0x%02x\n", __func__, por_state);

	IIC_S2MU106_FG_ERead(S2MU106_FG_SLAVE_ADDR_R, 0x1E, &reg_1E);
	printf("%s: 0x1E = 0x%02x\n", __func__, reg_1E);

	/* reset condition : 0x1F[4] or 0x1E != 0x03 */
	if ((por_state & 0x10) || (reg_1E != 0x03))
		return 1;
	else
		return 0;
}

static int s2mu106_check_param_ver(void)
{
	u8 param_ver = -1;

	IIC_S2MU106_FG_ERead(S2MU106_FG_SLAVE_ADDR_R, S2MU106_REG_FG_ID,
			&param_ver);
	param_ver &= 0x0F;

	printf("%s: parameter ver in IC: 0x%02x, in LK: 0x%02x\n", __func__,
			param_ver, model_param_ver);

	/* If parameter version is different, initialize FG */
	if (param_ver != model_param_ver)
		return 1;
	else
		return 0;
}

static void s2mu106_clear_por(void)
{
	u8 por_state;

	IIC_S2MU106_FG_ERead(S2MU106_FG_SLAVE_ADDR_R, 0x1F, &por_state);
	por_state &= ~0x10;
	IIC_S2MU106_FG_EWrite(S2MU106_FG_SLAVE_ADDR_W, 0x1F, por_state);

	IIC_S2MU106_FG_ERead(S2MU106_FG_SLAVE_ADDR_R, 0x1F, &por_state);
	printf("%s: 0x1F = 0x%02x\n", __func__, por_state);
}

static void s2mu106_set_battery_data(void)
{
	u8 temp;
	int i;

	/* Write battery cap. */
	IIC_S2MU106_FG_EWrite(S2MU106_FG_SLAVE_ADDR_W, 0x0E, batcap[0]);
	IIC_S2MU106_FG_EWrite(S2MU106_FG_SLAVE_ADDR_W, 0x0F, batcap[1]);
	IIC_S2MU106_FG_EWrite(S2MU106_FG_SLAVE_ADDR_W, 0x10, batcap[2]);
	IIC_S2MU106_FG_EWrite(S2MU106_FG_SLAVE_ADDR_W, 0x11, batcap[3]);

	/* Enable battery cap. */
	IIC_S2MU106_FG_ERead(S2MU106_FG_SLAVE_ADDR_R, 0x0C, &temp);
	temp |= 0x40;
	IIC_S2MU106_FG_EWrite(S2MU106_FG_SLAVE_ADDR_W, 0x0C,temp);

	/* Write battery accum. rate */
	IIC_S2MU106_FG_ERead(S2MU106_FG_SLAVE_ADDR_R, 0x45, &temp);
	temp &= 0xF0;
	temp |= accum[1];
	IIC_S2MU106_FG_EWrite(S2MU106_FG_SLAVE_ADDR_W, 0x45, temp);
	IIC_S2MU106_FG_EWrite(S2MU106_FG_SLAVE_ADDR_W, 0x44, accum[0]);

	/* Write battery table */
	for(i = 0x92; i <= 0xE9; i++)
		IIC_S2MU106_FG_EWrite(S2MU106_FG_SLAVE_ADDR_W, i, model_param1[i - 0x92]);
	for(i = 0xEA; i <= 0xFF; i++)
		IIC_S2MU106_FG_EWrite(S2MU106_FG_SLAVE_ADDR_W, i, model_param2[i - 0xEA]);

	/* Write battery parameter version
	 * Use reserved region of 0x48 for battery parameter version management
	 */
	IIC_S2MU106_FG_ERead(S2MU106_FG_SLAVE_ADDR_R, S2MU106_REG_FG_ID, &temp);
	temp &= 0xF0;
	temp |= (model_param_ver & 0x0F);
	IIC_S2MU106_FG_EWrite(S2MU106_FG_SLAVE_ADDR_W, S2MU106_REG_FG_ID, temp);
}

static void s2mu106_dumpdone(void)
{
	u8 temp;

	IIC_S2MU106_FG_EWrite(S2MU106_FG_SLAVE_ADDR_W, 0x14, 0x67);

	IIC_S2MU106_FG_ERead(S2MU106_FG_SLAVE_ADDR_R, 0x4B, &temp);
	temp &= 0x8F;
	IIC_S2MU106_FG_EWrite(S2MU106_FG_SLAVE_ADDR_W, 0x4B, temp);

	IIC_S2MU106_FG_EWrite(S2MU106_FG_SLAVE_ADDR_W, 0x4A, 0x10);

	IIC_S2MU106_FG_EWrite(S2MU106_FG_SLAVE_ADDR_W, 0x41, 0x04);
	IIC_S2MU106_FG_EWrite(S2MU106_FG_SLAVE_ADDR_W, 0x40, 0x08);

	IIC_S2MU106_FG_EWrite(S2MU106_FG_SLAVE_ADDR_W, 0x5C, 0x1A);

	IIC_S2MU106_FG_EWrite(S2MU106_FG_SLAVE_ADDR_W, 0x1E, 0x0F);
	u_delay(300000);

	IIC_S2MU106_FG_ERead(S2MU106_FG_SLAVE_ADDR_R, 0x1E, &temp);
	printf("%s: after dumpdone, 0x1E = 0x%02x\n", __func__, temp);
}

static void s2mu106_fg_reset(void)
{
	/* Set charger buck mode for initialize */
	s2mu106_charger_set_mode(S2MU106_CHG_MODE_BUCK);
	m_delay(100);

	/* Write battery data */
	s2mu106_set_battery_data();

	/* Do dumpdone */
	s2mu106_dumpdone();

	/* Recover charger mode */
	s2mu106_charger_set_mode(S2MU106_CHG_MODE_CHG);
}

static void s2mu106_fg_test_read(void)
{
#if 0
	u8 data = 0;
	char str[1016] = {0,};
	int i = 0;

	/* address 0x00 ~ 0x5B */
	for (i = 0x0; i <= 0x5F; i++) {
		IIC_S2MU106_FG_ERead(S2MU106_FG_SLAVE_ADDR_R, i, &data);
		sprintf(str+strlen(str), "%02x:%02x, ", i, data);

		if((i & 0x0F) == 0x0F) {
			printf("%s: %s\n", __func__, str);
			str[0] = '\0';
		}
	}

	/* address 0x92 ~ 0xff */
	for (i = 0x92; i <= 0xff; i++) {
		IIC_S2MU106_FG_ERead(S2MU106_FG_SLAVE_ADDR_R, i, &data);
		sprintf(str+strlen(str), "%02x:%02x, ", i, data);

		if((i & 0x0F) == 0x0F) {
			printf("%s: %s\n", __func__, str);
			str[0] = '\0';
		}
	}
#endif
}

static void s2mu106_fg_show_info(void)
{
	int vbat, avg_vbat;
	int curr, avg_curr;
	u32 soc;

	vbat = s2mu106_get_vbat();
	avg_vbat = s2mu106_get_avgvbat();
	curr = s2mu106_get_current();
	avg_curr = s2mu106_get_avgcurrent();
	soc = s2mu106_get_soc();

	printf("%s: vbat(%dmV), avg_vbat(%dmV)\n", __func__, vbat, avg_vbat);
	printf("%s: curr(%dmA), avg_curr(%dmA)\n", __func__, curr, avg_curr);
	printf("%s: SOC(%d.%d%%)\n", __func__, soc/100, soc%100);
}

void fg_init_s2mu106(void)
{
	IIC_S2MU106_FG_ESetport();

	u8 temp;

	/* Check Low VBAT */
	IIC_S2MU106_FG_ERead(S2MU106_FG_SLAVE_ADDR_R, 0x49, &temp);
	printf("%s: 0x49 = 0x%2x", __func__, temp);
	if ((temp & 0x80) == 0x80) {
		printf(" Low Vbat off detected!\n");
		temp &= (~0x80);
		IIC_S2MU106_FG_EWrite(S2MU106_FG_SLAVE_ADDR_W, 0x49, temp);
	} else
		printf("\n");

	/* Check need to initialize FG */
	if (s2mu106_check_por() || s2mu106_check_param_ver()) {
		/* If POR state or param ver. mismatch, reset */
		s2mu106_fg_reset();
		/* Clear POR state */
		s2mu106_clear_por();
	}
	else
		printf("%s: Fuelgauge is Already initialized\n", __func__);

	/* Read & print information */
	s2mu106_fg_test_read();
	s2mu106_fg_show_info();
}
#if 0
/*
 * Dead battery scenario not used. Insufficient driver support.
 */
void low_battery_shutdown(void)
{
	u32 val = 0;

	/* Disable WTSR */
	disable_smpl_wtsr_s2mpu09(0, 1);

	/* Set PSHOLD Low for power off*/
	val = readl(EXYNOS9610_POWER_BASE + 0x330C);
	val &= ~(1 << 8);
	writel(val, EXYNOS9610_POWER_BASE + 0x330C);
}

void dead_battery_recovery(void)
{
	int vbat = 0;
	int vbus = 0;
	int chg_mode = 0;

	IIC_S2MU106_FG_ESetport();

	/* When Shutdown occurs during Low voltage W/A,
	 * Voltage fix is not disabled. Disable voltage fix.
	 */
	IIC_S2MU106_FG_EWrite(S2MU106_FG_SLAVE_ADDR_W, 0x24, 0x00);

	/* Read POR state & check */
	if (s2mu106_check_por()) {
		/* FG is not initialized, Activate ADC */
		IIC_S2MU106_FG_EWrite(S2MU106_FG_SLAVE_ADDR_W, 0x1E, 0x0F);
		u_delay(300000);
	} else
		printf("%s: Fuelgauge is Already initialized\n", __func__);

	vbat = s2mu106_get_vbat();
	/* Get vbus status to check TA/USB is connected*/
	vbus = s2mu106_muic_get_vbus();

	if (vbat >= DEAD_BATTERY_VBAT) {
		printf("%s: Vbat is OK(%dmV)\n", __func__, vbat);
		return;
	} else if (!vbus) {
		printf("%s: Vbat is Low!(%dmV) & No Vbus. Shutdown.\n", __func__, vbat);
		low_battery_shutdown();
		return;
	}

	printf("%s: Start DBR! delay = %dms, VBUS = %d\n", __func__,
			DEAD_BATTERY_DELAY, vbus);
	wdt_stop();

	/* Set current & Turn on charger */
	s2mu106_charger_set_mode(S2MU106_CHG_MODE_CHG);
	s2mu106_charger_set_input_current(DEAD_BATTERY_INPUT);
	s2mu106_charger_set_charging_current(DEAD_BATTERY_CHG);

	while (1) {
		vbat = s2mu106_get_vbat();
		vbus = s2mu106_muic_get_vbus();
		chg_mode = s2mu106_charger_get_mode();

		if (vbat > DEAD_BATTERY_VBAT) {
			printf("%s: Vbat is OK(%dmV)\n", __func__, vbat);
			break;
		} else if ((vbat <= DEAD_BATTERY_VBAT) && vbus &&
				(chg_mode != S2MU106_CHG_MODE_BUCK)) {
			/* If cable is detached & attached between delay,
			 *  Charger is changed to BUCK MODE by IC itself.
			 *  At that case, do not keep charging.
			 */
			printf("%s: vbat is low!(%dmV), Keep charging(vbus = %d)\n",
					__func__, vbat, vbus);
			m_delay(DEAD_BATTERY_DELAY);
		} else {
			printf("%s: vbat is low!(%dmV), vbus = %d, chg_mode = %d\n",
					__func__, vbat, vbus, chg_mode);
			low_battery_shutdown();
			break;
		}
	}

	wdt_start(5);

}
#endif
