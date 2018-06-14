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
#include <platform/fg_s2mu004.h>

#define min(a, b) (a) < (b) ? a : b

static void Delay(void)
{
	unsigned long i = 0;
	for (i = 0; i < DELAY; i++)
		;
}

static void IIC_S2MU004_FG_SCLH_SDAH(void)
{
	IIC_S2MU004_FG_ESCL_Hi;
	IIC_S2MU004_FG_ESDA_Hi;
	Delay();
}

static void IIC_S2MU004_FG_SCLH_SDAL(void)
{
	IIC_S2MU004_FG_ESCL_Hi;
	IIC_S2MU004_FG_ESDA_Lo;
	Delay();
}

static void IIC_S2MU004_FG_SCLL_SDAH(void)
{
	IIC_S2MU004_FG_ESCL_Lo;
	IIC_S2MU004_FG_ESDA_Hi;
	Delay();
}

static void IIC_S2MU004_FG_SCLL_SDAL(void)
{
	IIC_S2MU004_FG_ESCL_Lo;
	IIC_S2MU004_FG_ESDA_Lo;
	Delay();
}

static void IIC_S2MU004_FG_ELow(void)
{
	IIC_S2MU004_FG_SCLL_SDAL();
	IIC_S2MU004_FG_SCLH_SDAL();
	IIC_S2MU004_FG_SCLH_SDAL();
	IIC_S2MU004_FG_SCLL_SDAL();
}

static void IIC_S2MU004_FG_EHigh(void)
{
	IIC_S2MU004_FG_SCLL_SDAH();
	IIC_S2MU004_FG_SCLH_SDAH();
	IIC_S2MU004_FG_SCLH_SDAH();
	IIC_S2MU004_FG_SCLL_SDAH();
}

static void IIC_S2MU004_FG_EStart(void)
{
	IIC_S2MU004_FG_SCLH_SDAH();
	IIC_S2MU004_FG_SCLH_SDAL();
	Delay();
	IIC_S2MU004_FG_SCLL_SDAL();
}

static void IIC_S2MU004_FG_EEnd(void)
{
	IIC_S2MU004_FG_SCLL_SDAL();
	IIC_S2MU004_FG_SCLH_SDAL();
	Delay();
	IIC_S2MU004_FG_SCLH_SDAH();
}

static void IIC_S2MU004_FG_EAck_write(void)
{
	unsigned long ack = 0;

	/* Function <- Input */
	IIC_S2MU004_FG_ESDA_INP;

	IIC_S2MU004_FG_ESCL_Lo;
	Delay();
	IIC_S2MU004_FG_ESCL_Hi;
	Delay();
	ack = GPIO_DAT_S2MU004;
	IIC_S2MU004_FG_ESCL_Hi;
	Delay();
	IIC_S2MU004_FG_ESCL_Hi;
	Delay();

	/* Function <- Output (SDA) */
	IIC_S2MU004_FG_ESDA_OUTP;

	ack = (ack >> GPIO_DAT_SHIFT) & 0x1;

	IIC_S2MU004_FG_SCLL_SDAL();
}

static void IIC_S2MU004_FG_EAck_read(void)
{
	/* Function <- Output */
	IIC_S2MU004_FG_ESDA_OUTP;

	IIC_S2MU004_FG_ESCL_Lo;
	IIC_S2MU004_FG_ESCL_Lo;
	IIC_S2MU004_FG_ESDA_Hi;
	IIC_S2MU004_FG_ESCL_Hi;
	IIC_S2MU004_FG_ESCL_Hi;
	/* Function <- Input (SDA) */
	IIC_S2MU004_FG_ESDA_INP;

	IIC_S2MU004_FG_SCLL_SDAL();
}

void IIC_S2MU004_FG_ESetport(void)
{
	/* Pull Up/Down Disable SCL, SDA */
	GPIO_PUD_S2MU004;

	IIC_S2MU004_FG_ESCL_Hi;
	IIC_S2MU004_FG_ESDA_Hi;

	/* Function <- Output (SCL) */
	IIC_S2MU004_FG_ESCL_OUTP;
	/* Function <- Output (SDA) */
	IIC_S2MU004_FG_ESDA_OUTP;

	Delay();
}

void IIC_S2MU004_FG_EWrite(unsigned char ChipId,
		unsigned char IicAddr, unsigned char IicData)
{
	unsigned long i = 0;

	IIC_S2MU004_FG_EStart();

	/* write chip id */
	for (i = 7; i > 0; i--) {
		if ((ChipId >> i) & 0x0001)
			IIC_S2MU004_FG_EHigh();
		else
			IIC_S2MU004_FG_ELow();
	}

	/* write */
	IIC_S2MU004_FG_ELow();

	/* ACK */
	IIC_S2MU004_FG_EAck_write();

	/* write reg. addr. */
	for (i = 8; i > 0; i--) {
		if ((IicAddr >> (i-1)) & 0x0001)
			IIC_S2MU004_FG_EHigh();
		else
			IIC_S2MU004_FG_ELow();
	}

	/* ACK */
	IIC_S2MU004_FG_EAck_write();

	/* write reg. data. */
	for (i = 8; i > 0; i--) {
		if ((IicData >> (i-1)) & 0x0001)
			IIC_S2MU004_FG_EHigh();
		else
			IIC_S2MU004_FG_ELow();
	}

	/* ACK */
	IIC_S2MU004_FG_EAck_write();

	IIC_S2MU004_FG_EEnd();
}

void IIC_S2MU004_FG_ERead(unsigned char ChipId,
		unsigned char IicAddr, unsigned char *IicData)
{
	unsigned long i = 0;
	unsigned long reg = 0;
	unsigned char data = 0;

	IIC_S2MU004_FG_EStart();

	/* write chip id */
	for (i = 7; i > 0; i--) {
		if ((ChipId >> i) & 0x0001)
			IIC_S2MU004_FG_EHigh();
		else
			IIC_S2MU004_FG_ELow();
	}

	/* write */
	IIC_S2MU004_FG_ELow();

	/* ACK */
	IIC_S2MU004_FG_EAck_write();

	/* write reg. addr. */
	for (i = 8; i > 0; i--) {
		if ((IicAddr >> (i-1)) & 0x0001)
			IIC_S2MU004_FG_EHigh();
		else
			IIC_S2MU004_FG_ELow();
	}

	/* ACK */
	IIC_S2MU004_FG_EAck_write();

	IIC_S2MU004_FG_EStart();

	/* write chip id */
	for (i = 7; i > 0; i--) {
		if ((ChipId >> i) & 0x0001)
			IIC_S2MU004_FG_EHigh();
		else
			IIC_S2MU004_FG_ELow();
	}

	/* read */
	IIC_S2MU004_FG_EHigh();
	/* ACK */
	IIC_S2MU004_FG_EAck_write();

	/* read reg. data. */
	IIC_S2MU004_FG_ESDA_INP;

	IIC_S2MU004_FG_ESCL_Lo;
	IIC_S2MU004_FG_ESCL_Lo;
	Delay();

	for (i = 8; i > 0; i--) {
		IIC_S2MU004_FG_ESCL_Lo;
		IIC_S2MU004_FG_ESCL_Lo;
		Delay();
		IIC_S2MU004_FG_ESCL_Hi;
		IIC_S2MU004_FG_ESCL_Hi;
		Delay();
		reg = GPIO_DAT_S2MU004;
		IIC_S2MU004_FG_ESCL_Hi;
		IIC_S2MU004_FG_ESCL_Hi;
		Delay();
		IIC_S2MU004_FG_ESCL_Lo;
		IIC_S2MU004_FG_ESCL_Lo;
		Delay();

		reg = (reg >> GPIO_DAT_SHIFT) & 0x1;

		data |= reg << (i-1);
	}

	/* ACK */
	IIC_S2MU004_FG_EAck_read();
	IIC_S2MU004_FG_ESDA_OUTP;

	IIC_S2MU004_FG_EEnd();

	*IicData = data;
}

int fg_reset = 0;
extern u8 is_factory_mode;

static u16 ReadFGRegisterWord(u8 addr)
{
	u8 msb = 0;
	u8 lsb = 0;
	IIC_S2MU004_FG_ERead(S2MU004_FG_SLAVE_ADDR_R, addr++, &lsb);
	IIC_S2MU004_FG_ERead(S2MU004_FG_SLAVE_ADDR_R, addr, &msb);
	return ((msb << 8) | lsb);
}

u32 fuel_gauge_read_vcell(void)
{
	u16 data, check_data;
	u32 vbat = 0;
	u8 vbat_src;
	int i;

	for (i = 0; i < 50; i++) {
		data = ReadFGRegisterWord(S2MU004_REG_RVBAT);
		check_data = ReadFGRegisterWord(S2MU004_REG_RVBAT);
		if (data == check_data)
			break;
	}

	vbat = (data * 1000) >> 13;

	IIC_S2MU004_FG_ERead(S2MU004_FG_SLAVE_ADDR_R, S2MU004_REG_CTRL0, &vbat_src);
	printf("%s: vbat (%d), src(0x%02X)\n", __func__, vbat, (vbat_src & 0x30) >> 4);

	return vbat;
}

u32 fuel_gauge_read_soc(void)
{
	u16 data, check_data;
	u16 compliment;
	int rsoc, i;

	for (i = 0; i < 50; i++) {
		data = ReadFGRegisterWord(S2MU004_REG_RSOC_CC);
		check_data = ReadFGRegisterWord(S2MU004_REG_RSOC_CC);

		if (data == check_data)
			break;
	}

	compliment = data;

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

static int s2mu004_get_avgcurrent(void)
{
	u16 data, check_data;
	u16 compliment;
	int curr = 0, i;

	IIC_S2MU004_FG_EWrite(S2MU004_FG_SLAVE_ADDR_W, S2MU004_REG_MONOUT_SEL, 0x26);

	for (i = 0; i < 50; i++) {
		data = ReadFGRegisterWord(S2MU004_REG_MONOUT);
		check_data = ReadFGRegisterWord(S2MU004_REG_MONOUT);

		if (data == check_data)
			break;
	}

	compliment = data;

	printf("%s: MONOUT(0x%4x)\n", __func__, compliment);

	if (compliment & (0x1 << 15)) { /* Charging */
		curr = ((~compliment) & 0xFFFF) + 1;
		curr = (curr * 1000) >> 12;
	} else { /* dischaging */
		curr = compliment & 0x7FFF;
		curr = (curr * (-1000)) >> 12;
	}
	IIC_S2MU004_FG_EWrite(S2MU004_FG_SLAVE_ADDR_W, S2MU004_REG_MONOUT_SEL, 0x10);
	printf("%s: avg current (%d)mA\n", __func__, curr);

	return curr;
}

static int s2mu004_get_avgvbat(void)
{
	u16 data, check_data;
	u32 new_vbat, old_vbat = 0;
	int cnt, i;

	for (cnt = 0; cnt < 5; cnt++) {
		for (i = 0; i < 50; i++) {
			data = ReadFGRegisterWord(S2MU004_REG_RVBAT);
			check_data = ReadFGRegisterWord(S2MU004_REG_RVBAT);

			if (data == check_data)
				break;
		}

		new_vbat = (data * 1000) >> 13;

		if (cnt == 0)
			old_vbat = new_vbat;
		else
			old_vbat = new_vbat / 2 + old_vbat / 2;
	}

	printf("%s: avgvbat (%d)\n", __func__, old_vbat);

	return old_vbat;
}

int s2mu004_get_current(void)
{
	u16 data, check_data;
	u16 compliment;
	int curr = 0, i;

	for (i = 0; i < 50; i++) {
		data = ReadFGRegisterWord(S2MU004_REG_RCUR_CC);
		check_data = ReadFGRegisterWord(S2MU004_REG_RCUR_CC);

		if (data == check_data)
			break;
	}

	compliment = data;

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

void WA_0_issue_at_init(void)
{
	int a = 0;
	u8 v_4e = 0, v_4f =0, temp1, temp2;
	double ad;

	IIC_S2MU004_FG_ERead(S2MU004_FG_SLAVE_ADDR_R, 0x4F, &v_4f);
	IIC_S2MU004_FG_ERead(S2MU004_FG_SLAVE_ADDR_R, 0x4E, &v_4e);

	a = (v_4f & 0x0F) << 8;
	a += v_4e;
	a = a << 3;

	ad = (double)a / (double)(1 << 16);
	ad = ad + 0.038;  // to change offset voltage for initial soc from 20mV to 38mv because of load current

	a = (int)(ad * (1<<16));

	if (a < 0) {
		a = -a;
		a = (a ^ 0xfffff) + 1;
	}

	a = a >> 3;
	printf("%s: a = 0x%x\n", __func__, a);

	a = a & 0xfff;

	// 0x4f[3:0] = (a & 0xF00) >> 8
	temp1 = v_4f & 0xF0;
	temp2 = (u8)((a&0xF00) >> 8);
	temp1 |= temp2;
	IIC_S2MU004_FG_EWrite(S2MU004_FG_SLAVE_ADDR_W, 0x4F, temp1);

	// 0x4E[7:0] = a & 0xFF
	temp2 = (u8)(a&0xFF);
	IIC_S2MU004_FG_EWrite(S2MU004_FG_SLAVE_ADDR_W, 0x4E, temp2);

	IIC_S2MU004_FG_EWrite(S2MU004_FG_SLAVE_ADDR_W, 0x40, 0xFF);

	IIC_S2MU004_FG_EWrite(S2MU004_FG_SLAVE_ADDR_W, 0x1E, 0x0F);
	u_delay(300000);

	// 0x4F[3:0] = v_4f[3:0]
	IIC_S2MU004_FG_ERead(S2MU004_FG_SLAVE_ADDR_R, 0x4F, &temp1);
	temp1 &= 0xF0;
	temp1 |= (v_4f & 0x0F);
	IIC_S2MU004_FG_EWrite(S2MU004_FG_SLAVE_ADDR_W, 0x4F, temp1);
	IIC_S2MU004_FG_EWrite(S2MU004_FG_SLAVE_ADDR_W, 0x4E, v_4e);

	IIC_S2MU004_FG_EWrite(S2MU004_FG_SLAVE_ADDR_W, 0x40, 0x08);
}

void Initialization_routine(int isCorrupted)
{
	int i;
	u8 temp = 0;
	int revision = 0;
	printf("%s: S2MU004 Fuel gauge init(%d)\n", __func__, isCorrupted);

	IIC_S2MU004_FG_ERead(S2MU004_FG_SLAVE_ADDR_W, 0x27, &temp);
	temp |= 0x10;
	IIC_S2MU004_FG_EWrite(S2MU004_FG_SLAVE_ADDR_W, 0x27, temp);

	IIC_S2MU004_FG_ERead(S2MU004_FG_SLAVE_ADDR_W, 0x45, &temp);
	temp &= 0xF0;
	temp |= 0x07;
	IIC_S2MU004_FG_EWrite(S2MU004_FG_SLAVE_ADDR_W, 0x45, temp);
	IIC_S2MU004_FG_EWrite(S2MU004_FG_SLAVE_ADDR_W, 0x44, 0xAE);

	IIC_S2MU004_FG_EWrite(S2MU004_FG_SLAVE_ADDR_W, 0x0E,VAL_0E);
	IIC_S2MU004_FG_EWrite(S2MU004_FG_SLAVE_ADDR_W, 0x0F,VAL_0F);
	IIC_S2MU004_FG_EWrite(S2MU004_FG_SLAVE_ADDR_W, 0x10,VAL_10);
	IIC_S2MU004_FG_EWrite(S2MU004_FG_SLAVE_ADDR_W, 0x11,VAL_11);

	/* write data version */
	IIC_S2MU004_FG_ERead(S2MU004_FG_SLAVE_ADDR_R, S2MU004_REG_FG_ID, &temp);
	temp &= 0xF0;
	/* If jig or corrupted state, then do not write proper data version for next booting */
	if (isCorrupted)
		temp |= 0x0F;
	else
		temp |= (0x0F & FG_MODEL_DATA_VERSION);
	IIC_S2MU004_FG_EWrite(S2MU004_FG_SLAVE_ADDR_W, S2MU004_REG_FG_ID, temp);

	IIC_S2MU004_FG_ERead(S2MU004_FG_SLAVE_ADDR_R, S2MU004_REG_FG_ID, &temp);
	revision = (temp & 0xF0) >> 4;

	printf("%s: S2MU004 Fuel gauge revision: %d, reg 0x48: 0x%x\n", __func__, revision, temp);

	for(i = 0x92; i <= 0xE9; i++)
		IIC_S2MU004_FG_EWrite(S2MU004_FG_SLAVE_ADDR_W, i, model_param1[i - 0x92]);
	for(i = 0xEA; i <= 0xFF; i++)
		IIC_S2MU004_FG_EWrite(S2MU004_FG_SLAVE_ADDR_W, i, model_param2[i - 0xEA]);

//	IIC_S2MU004_FG_EWrite(S2MU004_FG_SLAVE_ADDR_W, 0x1F, 0x01);
	IIC_S2MU004_FG_EWrite(S2MU004_FG_SLAVE_ADDR_W, 0x21, 0x13);
	IIC_S2MU004_FG_EWrite(S2MU004_FG_SLAVE_ADDR_W, 0x14, 0x40);

	IIC_S2MU004_FG_ERead(S2MU004_FG_SLAVE_ADDR_R, 0x45, &temp);
	temp &= 0xF0;
	temp |= VAL_45;
	IIC_S2MU004_FG_EWrite(S2MU004_FG_SLAVE_ADDR_W, 0x45, temp);
	IIC_S2MU004_FG_EWrite(S2MU004_FG_SLAVE_ADDR_W, 0x44, VAL_44);

	IIC_S2MU004_FG_ERead(S2MU004_FG_SLAVE_ADDR_R, 0x27, &temp);
	temp |= 0x10;
	IIC_S2MU004_FG_EWrite(S2MU004_FG_SLAVE_ADDR_W, 0x27, temp);

	IIC_S2MU004_FG_EWrite(S2MU004_FG_SLAVE_ADDR_W, 0x4B, 0x0B);
	IIC_S2MU004_FG_EWrite(S2MU004_FG_SLAVE_ADDR_W, 0x4A, 0x10);

	IIC_S2MU004_FG_ERead(S2MU004_FG_SLAVE_ADDR_R, 0x03, &temp);
	temp |= 0x10;
	IIC_S2MU004_FG_EWrite(S2MU004_FG_SLAVE_ADDR_W, 0x03, temp);

	IIC_S2MU004_FG_EWrite(S2MU004_FG_SLAVE_ADDR_W, 0x40, 0x04);

	/* Work around for issue SOC = 0% at initialization */
	WA_0_issue_at_init();
}

static int s2mu004_get_soc_from_ocv(int target_ocv)
{
	int soc = 0;
	int ocv = target_ocv * 10;
	int *soc_arr;
	int *ocv_arr;

	int high_index = TABLE_SIZE - 1;
	int low_index = 0;
	int mid_index = 0;

	soc_arr = soc_arr_val;
	ocv_arr = ocv_arr_val;

	if(ocv <= ocv_arr[TABLE_SIZE - 1]) {
		soc = soc_arr[TABLE_SIZE - 1];
		goto soc_ocv_mapping;
	} else if (ocv >= ocv_arr[0]) {
		soc = soc_arr[0];
		goto soc_ocv_mapping;
	}
	while (low_index <= high_index) {
		mid_index = (low_index + high_index) >> 1;
		if(ocv_arr[mid_index] > ocv)
			low_index = mid_index + 1;
		else if(ocv_arr[mid_index] < ocv)
			high_index = mid_index - 1;
		else {
			soc = soc_arr[mid_index];
			goto soc_ocv_mapping;
		}
	}
	soc = soc_arr[high_index];
	soc += ((soc_arr[low_index] - soc_arr[high_index]) *
			(ocv - ocv_arr[high_index])) /
			(ocv_arr[low_index] - ocv_arr[high_index]);

soc_ocv_mapping:
	printf("%s: ocv (%d), soc (%d)\n", __func__, ocv, soc);
	return soc;
}

static void WA_0_issue_at_init1(int target_ocv)
{
	int a = 0;
	u8 v_4e = 0, v_4f =0, temp1, temp2;
	int FG_volt, UI_volt, offset;

	/* Step 1: [Surge test]  get UI voltage (0.1mV)*/
	UI_volt = target_ocv * 10;

	IIC_S2MU004_FG_EWrite(S2MU004_FG_SLAVE_ADDR_W, 0x1E, 0x0F);
	u_delay(50000);

	/* Step 2: [Surge test] get FG voltage (0.1mV) */
	FG_volt = fuel_gauge_read_vcell() * 10;

	/* Step 3: [Surge test] get offset */
	offset = UI_volt - FG_volt;
	printf("%s: UI_volt(%d), FG_volt(%d), offset(%d)\n",
			__func__, UI_volt, FG_volt, offset);

	/* Step 4: [Surge test] */
	IIC_S2MU004_FG_ERead(S2MU004_FG_SLAVE_ADDR_R, 0x4F, &v_4f);
	IIC_S2MU004_FG_ERead(S2MU004_FG_SLAVE_ADDR_R, 0x4E, &v_4e);
	printf("%s: v_4f(0x%x), v_4e(0x%x)\n", __func__, v_4f, v_4e);

	a = (v_4f & 0x0F) << 8;
	a += v_4e;
	a = a << 3;
	printf("%s: a before add offset (0x%x)\n", __func__, a);

	a += (offset << 16) / 10000;
	printf("%s: a after add offset (0x%x)\n", __func__, a);

	a &= 0x7FFF;
	a = a >> 3;
	a &= 0xfff;
	printf("%s: (a >> 3)&0xFFF (0x%x)\n", __func__, a);

	/* modify 0x4f[3:0] */
	temp1 = v_4f & 0xF0;
	temp2 = (u8)((a&0xF00) >> 8);
	temp1 |= temp2;
	IIC_S2MU004_FG_EWrite(S2MU004_FG_SLAVE_ADDR_W, 0x4F, temp1);

	/* modify 0x4E[7:0] */
	temp2 = (u8)(a & 0xFF);
	IIC_S2MU004_FG_EWrite(S2MU004_FG_SLAVE_ADDR_W, 0x4E, temp2);

	/* restart and dumpdone */
	IIC_S2MU004_FG_EWrite(S2MU004_FG_SLAVE_ADDR_W, 0x1E, 0x0F);
	u_delay(300000);

	printf("%s: S2MU004 VBAT : %d\n", __func__, fuel_gauge_read_vcell() * 10);

	/* recovery 0x4E and 0x4F */
	IIC_S2MU004_FG_ERead(S2MU004_FG_SLAVE_ADDR_R, 0x4F, &temp1);
	temp1 &= 0xF0;
	temp1 |= (v_4f & 0x0F);
	IIC_S2MU004_FG_EWrite(S2MU004_FG_SLAVE_ADDR_W, 0x4F, temp1);
	IIC_S2MU004_FG_EWrite(S2MU004_FG_SLAVE_ADDR_W, 0x4E, v_4e);
}

void fg_init_s2mu004(void)
{
	u8 por_state;
	int avg_current = 0, avg_vbat = 0, vbat = 0, curr = 0,rsoc = 0;
	int target_soc = 0, ocv_pwroff = 0;
	u8 data_ver, temp;

	/* initial SOC calculation
	 * in case plug in TA -> remove battery -> plug in another battery
	 * We need to reset FG
	 */
	IIC_S2MU004_FG_ESetport();

	IIC_S2MU004_FG_ERead(S2MU004_FG_SLAVE_ADDR_R, 0x25, &temp);
	temp &= 0xCF;
	temp |= 0x10;
	IIC_S2MU004_FG_EWrite(S2MU004_FG_SLAVE_ADDR_W, 0x25, temp);
	IIC_S2MU004_FG_ERead(S2MU004_FG_SLAVE_ADDR_R, 0x25, &temp);
	printf("%s: 0x25 -> 0x%x\n", __func__, temp);

	IIC_S2MU004_FG_ERead(S2MU004_FG_SLAVE_ADDR_R, 0x1F, &por_state); /* reset condition : 0x1F[4] */

	IIC_S2MU004_FG_ERead(S2MU004_FG_SLAVE_ADDR_R, S2MU004_REG_FG_ID, &data_ver); /* read rsvd : 0x48[3:0] */
	data_ver &= 0x0F;
	printf("%s: POR state = 0x%02x, data_ver = 0x%x(0x%x)\n",
		__func__, por_state, data_ver, FG_MODEL_DATA_VERSION);

	if (por_state & 0x10 || data_ver != FG_MODEL_DATA_VERSION ) {
		Initialization_routine(0);

		por_state &= ~0x10;
		IIC_S2MU004_FG_EWrite(S2MU004_FG_SLAVE_ADDR_W, 0x1F, por_state);
		printf("%s: Fuelgauge Reset\n", __func__);
		fg_reset = 1;
	} else {
		printf("%s: Fuelgauge is Already initialized\n", __func__);
	}

	printf("%s: SOC(%d), VCELL(%d)\n", __func__, fuel_gauge_read_soc(), fuel_gauge_read_vcell());

	/* work-around for soc error */
	avg_current = s2mu004_get_avgcurrent();
	avg_vbat = s2mu004_get_avgvbat();
	vbat = fuel_gauge_read_vcell();
	curr = s2mu004_get_current();
	rsoc = fuel_gauge_read_soc();

	ocv_pwroff = avg_vbat - avg_current * 15 / 100;
	target_soc = s2mu004_get_soc_from_ocv(ocv_pwroff);
	if ((avg_vbat > 4000 && abs(target_soc - rsoc) > 3000) || \
		(avg_vbat > 3675 && avg_vbat <= 4000 && abs(target_soc - rsoc) > 5000) || \
		(avg_vbat <= 3675 && abs(target_soc - rsoc) > 3000)) {

		printf("%s : Bootloader F/G reset Start\n", __func__);

		WA_0_issue_at_init1(ocv_pwroff);
		u_delay(100000);
		fg_reset = 1;
	}
}

