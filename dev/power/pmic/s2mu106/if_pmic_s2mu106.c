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
#include <dev/if_pmic_s2mu106.h>
#include <platform/gpio.h>
#include <platform/interrupts.h>
#include <platform/sfr.h>
#include <reg.h>

#define SMALL_CHARGER_CURRENT 100
void set_charger_mode(int mode);
void set_charger_current(int input, int charging);

static void Delay(void)
{
	unsigned long i = 0;
	for (i = 0; i < DELAY; i++)
		;
}

static void IIC_S2MU106_SCLH_SDAH(void)
{
	IIC_S2MU106_ESCL_Hi;
	IIC_S2MU106_ESDA_Hi;
	Delay();
}

static void IIC_S2MU106_SCLH_SDAL(void)
{
	IIC_S2MU106_ESCL_Hi;
	IIC_S2MU106_ESDA_Lo;
	Delay();
}

static void IIC_S2MU106_SCLL_SDAH(void)
{
	IIC_S2MU106_ESCL_Lo;
	IIC_S2MU106_ESDA_Hi;
	Delay();
}

static void IIC_S2MU106_SCLL_SDAL(void)
{
	IIC_S2MU106_ESCL_Lo;
	IIC_S2MU106_ESDA_Lo;
	Delay();
}

static void IIC_S2MU106_ELow(void)
{
	IIC_S2MU106_SCLL_SDAL();
	IIC_S2MU106_SCLH_SDAL();
	IIC_S2MU106_SCLH_SDAL();
	IIC_S2MU106_SCLL_SDAL();
}

static void IIC_S2MU106_EHigh(void)
{
	IIC_S2MU106_SCLL_SDAH();
	IIC_S2MU106_SCLH_SDAH();
	IIC_S2MU106_SCLH_SDAH();
	IIC_S2MU106_SCLL_SDAH();
}

static void IIC_S2MU106_EStart(void)
{
	IIC_S2MU106_SCLH_SDAH();
	IIC_S2MU106_SCLH_SDAL();
	Delay();
	IIC_S2MU106_SCLL_SDAL();
}

static void IIC_S2MU106_EEnd(void)
{
	IIC_S2MU106_SCLL_SDAL();
	IIC_S2MU106_SCLH_SDAL();
	Delay();
	IIC_S2MU106_SCLH_SDAH();
}

static void IIC_S2MU106_EAck_write(void)
{
	unsigned long ack = 0;

	/* Function <- Input */
	IIC_S2MU106_ESDA_INP;

	IIC_S2MU106_ESCL_Lo;
	Delay();
	IIC_S2MU106_ESCL_Hi;
	Delay();
	ack = GPIO_DAT_S2MU106;
	IIC_S2MU106_ESCL_Hi;
	Delay();
	IIC_S2MU106_ESCL_Hi;
	Delay();

	/* Function <- Output (SDA) */
	IIC_S2MU106_ESDA_OUTP;

	ack = (ack >> GPIO_DAT_SHIFT) & 0x1;

	IIC_S2MU106_SCLL_SDAL();
}

static void IIC_S2MU106_EAck_read(void)
{
	/* Function <- Output */
	IIC_S2MU106_ESDA_OUTP;

	IIC_S2MU106_ESCL_Lo;
	IIC_S2MU106_ESCL_Lo;
	IIC_S2MU106_ESDA_Hi;
	IIC_S2MU106_ESCL_Hi;
	IIC_S2MU106_ESCL_Hi;
	/* Function <- Input (SDA) */
	IIC_S2MU106_ESDA_INP;

	IIC_S2MU106_SCLL_SDAL();
}

void IIC_S2MU106_ESetport(void)
{
	/* Pull Up/Down Disable SCL, SDA */
	GPIO_PUD_S2MU106;

	IIC_S2MU106_ESCL_Hi;
	IIC_S2MU106_ESDA_Hi;

	/* Function <- Output (SCL) */
	IIC_S2MU106_ESCL_OUTP;
	/* Function <- Output (SDA) */
	IIC_S2MU106_ESDA_OUTP;

	Delay();
}

void IIC_S2MU106_EWrite(unsigned char ChipId,
		unsigned char IicAddr, unsigned char IicData)
{
	unsigned long i = 0;

	IIC_S2MU106_EStart();

	/* write chip id */
	for (i = 7; i > 0; i--) {
		if ((ChipId >> i) & 0x0001)
			IIC_S2MU106_EHigh();
		else
			IIC_S2MU106_ELow();
	}

	/* write */
	IIC_S2MU106_ELow();

	/* ACK */
	IIC_S2MU106_EAck_write();

	/* write reg. addr. */
	for (i = 8; i > 0; i--) {
		if ((IicAddr >> (i-1)) & 0x0001)
			IIC_S2MU106_EHigh();
		else
			IIC_S2MU106_ELow();
	}

	/* ACK */
	IIC_S2MU106_EAck_write();

	/* write reg. data. */
	for (i = 8; i > 0; i--) {
		if ((IicData >> (i-1)) & 0x0001)
			IIC_S2MU106_EHigh();
		else
			IIC_S2MU106_ELow();
	}

	/* ACK */
	IIC_S2MU106_EAck_write();

	IIC_S2MU106_EEnd();
}

void IIC_S2MU106_ERead(unsigned char ChipId,
		unsigned char IicAddr, unsigned char *IicData)
{
	unsigned long i = 0;
	unsigned long reg = 0;
	unsigned char data = 0;

	IIC_S2MU106_EStart();

	/* write chip id */
	for (i = 7; i > 0; i--) {
		if ((ChipId >> i) & 0x0001)
			IIC_S2MU106_EHigh();
		else
			IIC_S2MU106_ELow();
	}

	/* write */
	IIC_S2MU106_ELow();

	/* ACK */
	IIC_S2MU106_EAck_write();

	/* write reg. addr. */
	for (i = 8; i > 0; i--) {
		if ((IicAddr >> (i-1)) & 0x0001)
			IIC_S2MU106_EHigh();
		else
			IIC_S2MU106_ELow();
	}

	/* ACK */
	IIC_S2MU106_EAck_write();

	IIC_S2MU106_EStart();

	/* write chip id */
	for (i = 7; i > 0; i--) {
		if ((ChipId >> i) & 0x0001)
			IIC_S2MU106_EHigh();
		else
			IIC_S2MU106_ELow();
	}

	/* read */
	IIC_S2MU106_EHigh();
	/* ACK */
	IIC_S2MU106_EAck_write();

	/* read reg. data. */
	IIC_S2MU106_ESDA_INP;

	IIC_S2MU106_ESCL_Lo;
	IIC_S2MU106_ESCL_Lo;
	Delay();

	for (i = 8; i > 0; i--) {
		IIC_S2MU106_ESCL_Lo;
		IIC_S2MU106_ESCL_Lo;
		Delay();
		IIC_S2MU106_ESCL_Hi;
		IIC_S2MU106_ESCL_Hi;
		Delay();
		reg = GPIO_DAT_S2MU106;
		IIC_S2MU106_ESCL_Hi;
		IIC_S2MU106_ESCL_Hi;
		Delay();
		IIC_S2MU106_ESCL_Lo;
		IIC_S2MU106_ESCL_Lo;
		Delay();

		reg = (reg >> GPIO_DAT_SHIFT) & 0x1;

		data |= reg << (i-1);
	}

	/* ACK */
	IIC_S2MU106_EAck_read();
	IIC_S2MU106_ESDA_OUTP;

	IIC_S2MU106_EEnd();

	*IicData = data;
}

void muic_sw_init(void)
{
	unsigned char reg;
	volatile int i;

	IIC_S2MU106_ESetport();

	IIC_S2MU106_ERead(S2MU106_MUIC_R_ADDR, S2MU106_AFC_CTRL1, &reg);
	if (reg) {
		IIC_S2MU106_EWrite(S2MU106_MUIC_W_ADDR, S2MU106_AFC_CTRL1, 0);
		IIC_S2MU106_EWrite(S2MU106_MUIC_W_ADDR, S2MU106_AFC_CTRL2, 0);
		IIC_S2MU106_EWrite(S2MU106_MUIC_W_ADDR, S2MU106_AFC_LOGIC_CTRL2, 0x1);
		IIC_S2MU106_EWrite(S2MU106_MUIC_W_ADDR, S2MU106_LDOADC_VSETL, 0x7C);
		for(i = 0; i < 50000; i++);
	}
}

void muic_sw_open (void)
{
	unsigned char reg;
	IIC_S2MU106_ESetport();
	IIC_S2MU106_ERead(S2MU106_MUIC_R_ADDR, S2MU106_MUIC_CTRL1, &reg);
	reg &= ~(0x1 << 2);
	IIC_S2MU106_EWrite(S2MU106_MUIC_W_ADDR, S2MU106_MUIC_CTRL1, reg);
	IIC_S2MU106_ERead(S2MU106_MUIC_R_ADDR, S2MU106_MANUAL_SW_CTRL, &reg);
	reg &= ~(0x7 << 2);
	reg &= ~(0x7 << 5);
	reg |= 0x0 << 2;
	reg |= 0x0 << 5;
	IIC_S2MU106_EWrite(S2MU106_MUIC_W_ADDR, S2MU106_MANUAL_SW_CTRL, reg);
}

void muic_sw_usb (void)
{
	unsigned char reg;

	muic_sw_init();

	IIC_S2MU106_ESetport();
	IIC_S2MU106_ERead(S2MU106_MUIC_R_ADDR, S2MU106_MUIC_CTRL1, &reg);
	reg &= ~(0x1 << 2);
	IIC_S2MU106_EWrite(S2MU106_MUIC_W_ADDR, S2MU106_MUIC_CTRL1, reg);
	IIC_S2MU106_ERead(S2MU106_MUIC_R_ADDR, S2MU106_MANUAL_SW_CTRL, &reg);
	reg &= ~(0x7 << 2);
	reg &= ~(0x7 << 5);
	reg |= 0x1 << 2;
	reg |= 0x1 << 5;
	IIC_S2MU106_EWrite(S2MU106_MUIC_W_ADDR, S2MU106_MANUAL_SW_CTRL, reg);
}

void muic_sw_uart (void)
{
	unsigned char reg;

	muic_sw_init();

	IIC_S2MU106_ESetport();
	IIC_S2MU106_ERead(S2MU106_MUIC_R_ADDR, S2MU106_MUIC_CTRL1, &reg);
	reg &= ~(0x1 << 2);
	IIC_S2MU106_EWrite(S2MU106_MUIC_W_ADDR, S2MU106_MUIC_CTRL1, reg);
	IIC_S2MU106_ERead(S2MU106_MUIC_R_ADDR, S2MU106_MANUAL_SW_CTRL, &reg);
	reg &= ~(0x7 << 2);
	reg &= ~(0x7 << 5);
	reg |= 0x2 << 2;
	reg |= 0x2 << 5;
	IIC_S2MU106_EWrite(S2MU106_MUIC_W_ADDR, S2MU106_MANUAL_SW_CTRL, reg);
}

int s2mu106_muic_get_vbus(void)
{
	unsigned char reg;
	int ret = 0;

	IIC_S2MU106_ESetport();
	IIC_S2MU106_ERead(S2MU106_MUIC_R_ADDR, S2MU106_DEVICE_APPLE, &reg);

	printf("%s: 0x4E = 0x%02x\n", __func__, reg);

	ret = (reg & 0x02) ? 1 : 0;

	return ret;
}

#if 0
int s2mu106_muic_get_ta_attached(void)
{
	unsigned char reg;
	int ret = 0;

	IIC_S2MU106_ESetport();
	IIC_S2MU106_ERead(S2MU106_MUIC_R_ADDR, S2MU106_DEVICE_TYPE1, &reg);

	printf("%s: 0x47 = 0x%02x\n", __func__, reg);

	ret = (reg & 0x40) ? 1 : 0;

	return ret;
}

int s2mu106_muic_get_usb_attached(void)
{
	unsigned char reg;
	int ret = 0;

	IIC_S2MU106_ESetport();
	IIC_S2MU106_ERead(S2MU106_MUIC_R_ADDR, S2MU106_DEVICE_TYPE1, &reg);

	printf("%s: 0x47 = 0x%02x\n", __func__, reg);

	ret = (reg & 0x04) ? 1 : 0;

	return ret;
}

int s2mu106_muic_get_cdp_attached(void)
{
	unsigned char reg;
	int ret = 0;

	IIC_S2MU106_ESetport();
	IIC_S2MU106_ERead(S2MU106_MUIC_R_ADDR, S2MU106_DEVICE_TYPE1, &reg);

	printf("%s: 0x47 = 0x%02x\n", __func__, reg);

	ret = (reg & 0x20) ? 1 : 0;

	return ret;
}

int muic_get_ta(void)
{
	return s2mu106_muic_get_ta_attached();
}

void s2mu106_set_charger_by_type(void)
{
	set_charger_mode(CHG_MODE_CHG);
	if (s2mu106_muic_get_ta_attached()) {
		set_charger_current(1300, 1500);
	} else if (s2mu106_muic_get_cdp_attached()) {
		muic_sw_usb();
		set_charger_current(1300, 1500);
	} else if (s2mu106_muic_get_usb_attached()) {
		muic_sw_usb();
		set_charger_current(500, 500);
	}
}

void muic_interrupt_handler(void)
{
	unsigned char reg, reg_muic;

	printf("%s: Enter\n", __func__);
	IIC_S2MU106_ESetport();

	IIC_S2MU106_EWrite(S2MU106_CHG_W_ADDR, 0x7, 0xFB);
	IIC_S2MU106_ERead(S2MU106_MUIC_R_ADDR, S2MU106_MUIC_INT1, &reg_muic);
	IIC_S2MU106_ERead(S2MU106_MUIC_R_ADDR, S2MU106_MUIC_INT2, &reg);

	if (reg_muic & 0x1) {
		s2mu106_set_charger_by_type();
	} else if (reg_muic & 0x2) {
		printf("%s: open the path, for the next bc1.2\n", __func__);
		set_charger_mode(S2MU106_CHG_MODE_BUCK);
		muic_sw_open();
	} else {
		s2mu106_set_charger_by_type();
	}
}

void s2mu106_muic_check_cdp(void)
{

	unsigned char reg;

	IIC_S2MU106_ESetport();
	IIC_S2MU106_ERead(S2MU106_MUIC_R_ADDR, 0x47, &reg);
	if (reg & 0x20) {
		printf("%s dev_typ1(%#x): handle init cdp\n", __func__, reg);
		muic_sw_open();
		mdelay(10);
		IIC_S2MU106_EWrite(S2MU106_MUIC_W_ADDR, S2MU106_AFC_CTRL1, 0x99);
		mdelay(10);
		IIC_S2MU106_EWrite(S2MU106_MUIC_W_ADDR, S2MU106_AFC_CTRL1, 0x0);
	}
}
#endif

void s2mu106_charger_set_mode(int mode)
{
	u8 reg;

	if (mode != S2MU106_CHG_MODE_OFF &&
			mode != S2MU106_CHG_MODE_BUCK &&
			mode != S2MU106_CHG_MODE_CHG) {
		printf("%s: wrong charger mode! Stop!\n",
				__func__);
		return;
	}

	IIC_S2MU106_ESetport();
	IIC_S2MU106_ERead(S2MU106_CHG_R_ADDR, S2MU106_CHG_CTRL0, &reg);
	reg &= ~0x0F;
	reg |= mode;
	IIC_S2MU106_EWrite(S2MU106_CHG_W_ADDR, S2MU106_CHG_CTRL0, reg);

	IIC_S2MU106_ERead(S2MU106_CHG_R_ADDR, S2MU106_CHG_CTRL0, &reg);
	printf("%s: S2MU106_CHG_CTRL0 = 0x%02x\n", __func__,
			reg);
}

int s2mu106_charger_get_mode(void)
{
	u8 reg;
	int mode = 0;

	IIC_S2MU106_ESetport();
	IIC_S2MU106_ERead(S2MU106_CHG_R_ADDR, S2MU106_CHG_CTRL0, &reg);
	printf("%s: S2MU106_CHG_CTRL0 = 0x%02x\n", __func__,
			reg);

	mode = (reg & 0x0F);

	return mode;
}

void s2mu106_charger_set_input_current(int input_curr)
{
	u8 reg;

	if (input_curr < 100)
		reg = 0x02;
	else if (input_curr > 3000)
		reg = 0x76;
	else
		reg = (input_curr - 50) / 25;

	IIC_S2MU106_ESetport();
	IIC_S2MU106_EWrite(S2MU106_CHG_W_ADDR, S2MU106_CHG_CTRL1, reg);

	IIC_S2MU106_ERead(S2MU106_CHG_R_ADDR, S2MU106_CHG_CTRL1, &reg);
	printf("%s: S2MU106_CHG_CTRL1 = 0x%02x\n", __func__, reg);
}

int s2mu106_charger_get_input_current(void)
{
	int input_curr = -1;
	u8 reg;

	IIC_S2MU106_ESetport();
	IIC_S2MU106_ERead(S2MU106_CHG_R_ADDR, S2MU106_CHG_CTRL1, &reg);

	input_curr = reg * 25 + 50;

	printf("%s: input_current = %dmA(0x%02x)\n", __func__,
			input_curr, reg);

	return input_curr;
}

void s2mu106_charger_set_charging_current(int chg_curr)
{
	u8 reg, data;

	if (chg_curr < 50)
		data = 0x00;
	else if (chg_curr > 3200)
		data = 0x3F;
	else
		data = chg_curr / 50 - 1;

	IIC_S2MU106_ESetport();
	IIC_S2MU106_ERead(S2MU106_CHG_R_ADDR, S2MU106_CHG_CTRL7, &reg);
	reg &= ~0x3F;
	reg |= data;
	IIC_S2MU106_EWrite(S2MU106_CHG_W_ADDR, S2MU106_CHG_CTRL7, reg);

	IIC_S2MU106_ERead(S2MU106_CHG_R_ADDR, S2MU106_CHG_CTRL7, &reg);
	printf("%s: S2MU106_CHG_CTRL7 = 0x%02x\n", __func__, reg);
}

int s2mu106_charger_get_charging_current(void)
{
	int chg_curr = -1;
	u8 reg;

	IIC_S2MU106_ESetport();
	IIC_S2MU106_ERead(S2MU106_CHG_R_ADDR, S2MU106_CHG_CTRL7, &reg);

	chg_curr = ((reg & 0x3F) + 1) * 50;

	printf("%s: charging_current = %dmA(0x%02x)\n", __func__,
			chg_curr, reg);

	return chg_curr;
}

void s2mu106_charger_reg_init(void)
{
	u8 reg;

	IIC_S2MU106_ERead(S2MU106_CHG_R_ADDR, 0xec, &reg);
	reg &= ~0x80;
	IIC_S2MU106_EWrite(S2MU106_CHG_W_ADDR, 0xec, reg);
	printf("%s, 0xec(%x)\n", __func__, reg);

	IIC_S2MU106_ERead(S2MU106_CHG_R_ADDR, 0xe5, &reg);
	reg &= ~0x0F;
	reg |= 0x8;
	IIC_S2MU106_EWrite(S2MU106_CHG_W_ADDR, 0xe5, reg);
	printf("%s, 0xe5(%x)\n", __func__, reg);

	IIC_S2MU106_EWrite(S2MU106_CHG_W_ADDR, 0x7, 0x0);

	/* Disable Input OCP */
	IIC_S2MU106_ERead(S2MU106_CHG_R_ADDR, 0x3D, &reg);
	reg &= 0x7F;
	IIC_S2MU106_EWrite(S2MU106_CHG_W_ADDR, 0x3D, reg);
	printf("%s, 0x3D(%x)\n", __func__, reg);

	/* CHGIN IVR 4.5V */
	IIC_S2MU106_ERead(S2MU106_CHG_R_ADDR, S2MU106_CHG_CTRL4, &reg);
	reg |= 0x0C;
	IIC_S2MU106_EWrite(S2MU106_CHG_W_ADDR, S2MU106_CHG_CTRL4, reg);
	printf("%s, CTRL4(%x)\n", __func__, reg);
}

void s2mu106_charger_init(void)
{
	/* s2mu106 charger default mode is buck mode.
	 * To prevent 0V battery problem, set charger mode.
	 * Charger mode must be maintained until it finish
	 * DBR(Dead Battery Recovery).
	 */
	s2mu106_charger_set_mode(S2MU106_CHG_MODE_CHG);

	s2mu106_charger_reg_init();
	s2mu106_charger_set_charging_current(SMALL_CHARGER_CURRENT);

	//s2mu106_muic_check_cdp();
}


void set_charger_current(int input, int charging)
{
	s2mu106_charger_set_input_current(input);
	s2mu106_charger_set_charging_current(charging);
}

void set_charger_mode(int mode)
{
	int s2mu106_mode = -1;

	switch (mode) {
		case CHG_MODE_OFF:
			s2mu106_mode = S2MU106_CHG_MODE_OFF;
			break;
		case CHG_MODE_CHG:
			s2mu106_mode = S2MU106_CHG_MODE_CHG;
			break;
		case CHG_MODE_BUCK:
		default:
			s2mu106_mode = S2MU106_CHG_MODE_BUCK;
			break;
	}

	s2mu106_charger_set_mode(s2mu106_mode);
}

#define CHARGER_GPIO_INT_NUM	(17 + 32)
#if 0
static enum handler_return charger_gpio_interrupt(void *arg)
{
	printf("EXYNOS9610_WEINT_GPA2_PEND: %x\n", readl(EXYNOS9610_WEINT_GPA2_PEND));
	writel(readl(EXYNOS9610_WEINT_GPA2_PEND) | (1 << 1), EXYNOS9610_WEINT_GPA2_PEND);
	muic_interrupt_handler();
	return INT_NO_RESCHEDULE;
}

void init_muic_interrupt(void)
{
	exynos_gpio_cfg_pin((struct exynos_gpio_bank *)EXYNOS9610_GPA2CON, 1, 0xF);
	writel(readl(EXYNOS9610_GPA2PUD) & ~(0xF0), EXYNOS9610_GPA2PUD);
	writel(readl(EXYNOS9610_WEINT_GPA2_PEND) | (1 << 1), EXYNOS9610_WEINT_GPA2_PEND);
	writel(readl(EXYNOS9610_WEINT_GPA2_MASK) & ~(1 << 1), EXYNOS9610_WEINT_GPA2_MASK);
	writel((readl(EXYNOS9610_WEINT_GPA2_CON) & ~(0x7 << (4 * 1))) | (0x2 << (4 * 1)), EXYNOS9610_WEINT_GPA2_CON);
	printf("Enable charger GPIO interrupt!!!\n");
	register_int_handler(CHARGER_GPIO_INT_NUM, &charger_gpio_interrupt, NULL);
	unmask_interrupt(CHARGER_GPIO_INT_NUM);
	muic_interrupt_handler();
}
#endif
