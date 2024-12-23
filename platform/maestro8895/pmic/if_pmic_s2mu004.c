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
#include <platform/if_pmic_s2mu004.h>

#define GPP0BASE	(0x139b0000)
#define GPP0CON		*(volatile unsigned int *)(GPP0BASE + 0x0)
#define GPP0DAT		*(volatile unsigned int *)(GPP0BASE + 0x4)
#define GPP0PUD		*(volatile unsigned int *)(GPP0BASE + 0x8)

/* SDA: GPP0_2, SCL: GPP0_3 */
#define GPIO_DAT_S2MU004	GPP0DAT
#define GPIO_DAT_SHIFT		(2)
#define GPIO_PUD_S2MU004	GPP0PUD &= ~(0xff << (GPIO_DAT_SHIFT*4))

#define IIC_S2MU004_ESCL_Hi	GPP0DAT |= (0x1 << (GPIO_DAT_SHIFT+1))
#define IIC_S2MU004_ESCL_Lo	GPP0DAT &= ~(0x1 << (GPIO_DAT_SHIFT+1))
#define IIC_S2MU004_ESDA_Hi	GPP0DAT |= (0x1 << GPIO_DAT_SHIFT)
#define IIC_S2MU004_ESDA_Lo	GPP0DAT &= ~(0x1 << GPIO_DAT_SHIFT)

#define IIC_S2MU004_ESCL_INP	GPP0CON &= ~(0xf << ((GPIO_DAT_SHIFT+1)*4))
#define IIC_S2MU004_ESCL_OUTP	GPP0CON = (GPP0CON & ~(0xf << ((GPIO_DAT_SHIFT+1)*4))) \
					| (0x1 << ((GPIO_DAT_SHIFT+1)*4))
#define IIC_S2MU004_ESDA_INP	GPP0CON &= ~(0xf << (GPIO_DAT_SHIFT*4))
#define IIC_S2MU004_ESDA_OUTP	GPP0CON = (GPP0CON & ~(0xf << (GPIO_DAT_SHIFT*4))) \
					| (0x1 << (GPIO_DAT_SHIFT*4))

#define DELAY		100

u8 is_factory_mode;

static void Delay(void)
{
	unsigned long i = 0;
	for (i = 0; i < DELAY; i++)
		;
}

static void IIC_S2MU004_SCLH_SDAH(void)
{
	IIC_S2MU004_ESCL_Hi;
	IIC_S2MU004_ESDA_Hi;
	Delay();
}

static void IIC_S2MU004_SCLH_SDAL(void)
{
	IIC_S2MU004_ESCL_Hi;
	IIC_S2MU004_ESDA_Lo;
	Delay();
}

static void IIC_S2MU004_SCLL_SDAH(void)
{
	IIC_S2MU004_ESCL_Lo;
	IIC_S2MU004_ESDA_Hi;
	Delay();
}

static void IIC_S2MU004_SCLL_SDAL(void)
{
	IIC_S2MU004_ESCL_Lo;
	IIC_S2MU004_ESDA_Lo;
	Delay();
}

static void IIC_S2MU004_ELow(void)
{
	IIC_S2MU004_SCLL_SDAL();
	IIC_S2MU004_SCLH_SDAL();
	IIC_S2MU004_SCLH_SDAL();
	IIC_S2MU004_SCLL_SDAL();
}

static void IIC_S2MU004_EHigh(void)
{
	IIC_S2MU004_SCLL_SDAH();
	IIC_S2MU004_SCLH_SDAH();
	IIC_S2MU004_SCLH_SDAH();
	IIC_S2MU004_SCLL_SDAH();
}

static void IIC_S2MU004_EStart(void)
{
	IIC_S2MU004_SCLH_SDAH();
	IIC_S2MU004_SCLH_SDAL();
	Delay();
	IIC_S2MU004_SCLL_SDAL();
}

static void IIC_S2MU004_EEnd(void)
{
	IIC_S2MU004_SCLL_SDAL();
	IIC_S2MU004_SCLH_SDAL();
	Delay();
	IIC_S2MU004_SCLH_SDAH();
}

static void IIC_S2MU004_EAck_write(void)
{
	unsigned long ack = 0;

	/* Function <- Input */
	IIC_S2MU004_ESDA_INP;

	IIC_S2MU004_ESCL_Lo;
	Delay();
	IIC_S2MU004_ESCL_Hi;
	Delay();
	ack = GPIO_DAT_S2MU004;
	IIC_S2MU004_ESCL_Hi;
	Delay();
	IIC_S2MU004_ESCL_Hi;
	Delay();

	/* Function <- Output (SDA) */
	IIC_S2MU004_ESDA_OUTP;

	ack = (ack >> GPIO_DAT_SHIFT) & 0x1;

	IIC_S2MU004_SCLL_SDAL();
}

static void IIC_S2MU004_EAck_read(void)
{
	/* Function <- Output */
	IIC_S2MU004_ESDA_OUTP;

	IIC_S2MU004_ESCL_Lo;
	IIC_S2MU004_ESCL_Lo;
	IIC_S2MU004_ESDA_Hi;
	IIC_S2MU004_ESCL_Hi;
	IIC_S2MU004_ESCL_Hi;
	/* Function <- Input (SDA) */
	IIC_S2MU004_ESDA_INP;

	IIC_S2MU004_SCLL_SDAL();
}

void IIC_S2MU004_ESetport(void)
{
	/* Pull Up/Down Disable SCL, SDA */
	GPIO_PUD_S2MU004;

	IIC_S2MU004_ESCL_Hi;
	IIC_S2MU004_ESDA_Hi;

	/* Function <- Output (SCL) */
	IIC_S2MU004_ESCL_OUTP;
	/* Function <- Output (SDA) */
	IIC_S2MU004_ESDA_OUTP;

	Delay();
}

void IIC_S2MU004_EWrite(unsigned char ChipId,
		unsigned char IicAddr, unsigned char IicData)
{
	unsigned long i = 0;

	IIC_S2MU004_EStart();

	/* write chip id */
	for (i = 7; i > 0; i--) {
		if ((ChipId >> i) & 0x0001)
			IIC_S2MU004_EHigh();
		else
			IIC_S2MU004_ELow();
	}

	/* write */
	IIC_S2MU004_ELow();

	/* ACK */
	IIC_S2MU004_EAck_write();

	/* write reg. addr. */
	for (i = 8; i > 0; i--) {
		if ((IicAddr >> (i-1)) & 0x0001)
			IIC_S2MU004_EHigh();
		else
			IIC_S2MU004_ELow();
	}

	/* ACK */
	IIC_S2MU004_EAck_write();

	/* write reg. data. */
	for (i = 8; i > 0; i--) {
		if ((IicData >> (i-1)) & 0x0001)
			IIC_S2MU004_EHigh();
		else
			IIC_S2MU004_ELow();
	}

	/* ACK */
	IIC_S2MU004_EAck_write();

	IIC_S2MU004_EEnd();
}

void IIC_S2MU004_ERead(unsigned char ChipId,
		unsigned char IicAddr, unsigned char *IicData)
{
	unsigned long i = 0;
	unsigned long reg = 0;
	unsigned char data = 0;

	IIC_S2MU004_EStart();

	/* write chip id */
	for (i = 7; i > 0; i--) {
		if ((ChipId >> i) & 0x0001)
			IIC_S2MU004_EHigh();
		else
			IIC_S2MU004_ELow();
	}

	/* write */
	IIC_S2MU004_ELow();

	/* ACK */
	IIC_S2MU004_EAck_write();

	/* write reg. addr. */
	for (i = 8; i > 0; i--) {
		if ((IicAddr >> (i-1)) & 0x0001)
			IIC_S2MU004_EHigh();
		else
			IIC_S2MU004_ELow();
	}

	/* ACK */
	IIC_S2MU004_EAck_write();

	IIC_S2MU004_EStart();

	/* write chip id */
	for (i = 7; i > 0; i--) {
		if ((ChipId >> i) & 0x0001)
			IIC_S2MU004_EHigh();
		else
			IIC_S2MU004_ELow();
	}

	/* read */
	IIC_S2MU004_EHigh();
	/* ACK */
	IIC_S2MU004_EAck_write();

	/* read reg. data. */
	IIC_S2MU004_ESDA_INP;

	IIC_S2MU004_ESCL_Lo;
	IIC_S2MU004_ESCL_Lo;
	Delay();

	for (i = 8; i > 0; i--) {
		IIC_S2MU004_ESCL_Lo;
		IIC_S2MU004_ESCL_Lo;
		Delay();
		IIC_S2MU004_ESCL_Hi;
		IIC_S2MU004_ESCL_Hi;
		Delay();
		reg = GPIO_DAT_S2MU004;
		IIC_S2MU004_ESCL_Hi;
		IIC_S2MU004_ESCL_Hi;
		Delay();
		IIC_S2MU004_ESCL_Lo;
		IIC_S2MU004_ESCL_Lo;
		Delay();

		reg = (reg >> GPIO_DAT_SHIFT) & 0x1;

		data |= reg << (i-1);
	}

	/* ACK */
	IIC_S2MU004_EAck_read();
	IIC_S2MU004_ESDA_OUTP;

	IIC_S2MU004_EEnd();

	*IicData = data;
}

/*set float voltage */
void set_regulation_voltage(int float_voltage)
{
	int data;
	u8 temp;

	IIC_S2MU004_ERead(S2MU004_R_ADDR, S2MU004_CHG_CTRL6, &temp);
	printf("[DEBUG]%s: S2MU004_CHG_CTRL6 => 0x%x\n", __func__, temp);

	temp &= ~0x3F;

	if (float_voltage <= 3900)
		data = 0;
	else if ((float_voltage > 3900) && (float_voltage <= 4530))
		data = (float_voltage - 3900) / 10;
	else
		data = 0x3F;

	IIC_S2MU004_EWrite(S2MU004_W_ADDR, S2MU004_CHG_CTRL6, (data | temp));
	IIC_S2MU004_ERead(S2MU004_R_ADDR, S2MU004_CHG_CTRL6, &temp);
	printf("[DEBUG]%s: float_voltage change to %d, 0x%x\n", __func__, float_voltage, temp);
}
#if 0
int check_factory_mode(void)
{
	unsigned char reg;

	if (s2mu004_get_jig_state()) {
		/* CHGIN UVLO Level 4.0V */
		IIC_S2MU004_ERead(S2MU004_R_ADDR, S2MU004_CHG_UVLOCTRL, &reg);
		reg &= ~0xC0;
		IIC_S2MU004_EWrite(S2MU004_W_ADDR, S2MU004_CHG_UVLOCTRL, reg);
		IIC_S2MU004_ERead(S2MU004_R_ADDR, S2MU004_CHG_UVLOCTRL, &reg);
		printf("[DEBUG]%s: uvlo : 0x%x\n", __func__, reg);

		/* Factory Mode Enable */
		IIC_S2MU004_ERead(S2MU004_R_ADDR, S2MU004_CHG_CTRL0, &reg);
		reg |= 0x1 << 4; /* 0x10[4] = 1 */
		IIC_S2MU004_EWrite(S2MU004_W_ADDR, S2MU004_CHG_CTRL0, reg);

		/* Set Input Current to Maximum */
		IIC_S2MU004_ERead(S2MU004_R_ADDR, S2MU004_CHG_CTRL2, &reg);
		reg |= 0x7F;
		IIC_S2MU004_EWrite(S2MU004_W_ADDR, S2MU004_CHG_CTRL2, reg);

		/* Buck switching frequency setting to 2MHz */
		IIC_S2MU004_ERead(S2MU004_R_ADDR, S2MU004_CHG_CTRL12, &reg);
		reg &= ~0x7;
		reg |= 0x6; /* 0x1C[2:0] = b110 */
		IIC_S2MU004_EWrite(S2MU004_W_ADDR, S2MU004_CHG_CTRL12, reg);

		IIC_S2MU004_ERead(S2MU004_R_ADDR, 0x8B, &reg);
		reg |= 0xC1; /* 0x8B[7],[6],[0] = 1 */
		IIC_S2MU004_EWrite(S2MU004_W_ADDR, 0x8B, reg);

		/* Set force sync mode */
		IIC_S2MU004_ERead(S2MU004_R_ADDR, 0x91, &reg);
		reg |= 0x40; /* 0x91[6] = 1 */
		IIC_S2MU004_EWrite(S2MU004_W_ADDR, 0x91, reg);

		is_factory_mode = 1;
		printf("%s: Enter FACTORY MODE !!!!!\n", __func__);
	} else {
		/* Factory Mode Disable */
		IIC_S2MU004_ERead(S2MU004_R_ADDR, S2MU004_CHG_CTRL0, &reg);
		reg &= 0xEF; /* 0x10[4] = 0 */
		IIC_S2MU004_EWrite(S2MU004_W_ADDR, S2MU004_CHG_CTRL0, reg);

		/* Buck switching frequency setting to default */
		IIC_S2MU004_ERead(S2MU004_R_ADDR, S2MU004_CHG_CTRL12, &reg);
		reg &= ~0x7;
		reg |= 0x2; /* 0x1C[2:0]=b010 */
		IIC_S2MU004_EWrite(S2MU004_W_ADDR, S2MU004_CHG_CTRL12, reg);

		IIC_S2MU004_ERead(S2MU004_R_ADDR, 0x8B, &reg);
		reg &= ~0xC1;
		reg |= 0x01; /* 0x8B[7],[6]=0,[0]=1 */
		IIC_S2MU004_EWrite(S2MU004_W_ADDR, 0x8B, reg);

		/* not force sync mode */
		IIC_S2MU004_ERead(S2MU004_R_ADDR, 0x91, &reg);
		reg &= 0xBF; /* 0x91[6]=0 */
		IIC_S2MU004_EWrite(S2MU004_W_ADDR, 0x91, reg);

		is_factory_mode = 0;
	}

	return is_factory_mode;
}
#endif
void set_charger_current(int set_current)
{
	u8 chg_curr = 0;
	IIC_S2MU004_ERead(S2MU004_R_ADDR, S2MU004_CHG_CTRL9, &chg_curr);
	chg_curr &= ~0x7F;

	if (set_current <= 100)
		chg_curr |= 0x03;
	else if (set_current > 100 && set_current <= 3150)
		chg_curr |= (set_current / 25) - 1;
	else
		chg_curr |= 0x7D;

	IIC_S2MU004_EWrite(S2MU004_W_ADDR, S2MU004_CHG_CTRL9, chg_curr);

	IIC_S2MU004_ERead(S2MU004_R_ADDR, S2MU004_CHG_CTRL9, &chg_curr);
	printf("%s : S2MU004_CHG_CTRL9 : (%d)0x%02x\n",
			__func__, set_current, chg_curr);

	IIC_S2MU004_ERead(S2MU004_R_ADDR, S2MU004_CHG_CTRL8, &chg_curr);
	chg_curr &= ~0x7F;

	if (set_current <= 100)
		chg_curr |= 0x03;
	else if (set_current > 100 && set_current <= 1000)
		chg_curr |= (set_current / 25) - 1;
	else
		chg_curr |= 0x3F; /* Max current 1.6A 0x27; */

	IIC_S2MU004_EWrite(S2MU004_W_ADDR, S2MU004_CHG_CTRL8, chg_curr);
	IIC_S2MU004_ERead(S2MU004_R_ADDR, S2MU004_CHG_CTRL8, &chg_curr);
	printf("%s : S2MU004_CHG_CTRL8 : (%d)0x%02x\n",
			__func__, set_current, chg_curr);
}

/* set input current limit */
void set_input_current(int set_current)
{
	u8 in_curr = 0;

	IIC_S2MU004_ERead(S2MU004_R_ADDR, S2MU004_CHG_CTRL2, &in_curr);
	in_curr &= ~0x7F;

	if (set_current <= 100)
		in_curr = 0x02;
	else if (set_current > 100 && set_current <= 2500)
		in_curr = (set_current - 50) / 25;
	else
		in_curr = 0x12;

	IIC_S2MU004_EWrite(S2MU004_W_ADDR, S2MU004_CHG_CTRL2, in_curr);

	IIC_S2MU004_ERead(S2MU004_R_ADDR, S2MU004_CHG_CTRL2, &in_curr);
	printf("%s : S2MU004_CHG_CTRL2 : 0x%02x\n",
				__func__, in_curr);
}

void set_charger_state(int en_buck, int en_chg)
{
	u8 reg_data;

	IIC_S2MU004_ERead(S2MU004_R_ADDR, S2MU004_CHG_CTRL0, &reg_data);
	reg_data &= 0xF0; /* default all off */
	if (en_buck) {
		if (en_chg)
			reg_data |= 0x03; /* charger mode */
		else
			reg_data |= 0x01; /* buck mode */
	}
	IIC_S2MU004_EWrite(S2MU004_W_ADDR, S2MU004_CHG_CTRL0, reg_data);

	IIC_S2MU004_ERead(S2MU004_R_ADDR, S2MU004_CHG_CTRL0, &reg_data);
	printf("%s : S2MU004_CHG_CTRL0 : 0x%x\n",
		__func__, reg_data);

}

void chg_init_s2mu004(void)
{
	unsigned char reg;
	unsigned char pmic_rev;

	IIC_S2MU004_ESetport();

	IIC_S2MU004_ERead(S2MU004_R_ADDR, S2MU004_PMIC_ID, &reg);
	pmic_rev = (reg & 0xF0) >> 4;
	printf("%s: s2mu004 0x%02X (rev:0x%x)\n",
		__func__, reg, pmic_rev);

	/* SYSTEM_CLK_ON_OF_MODE data VIO reset by VBUS flucturation */
	IIC_S2MU004_ERead(S2MU004_R_ADDR, 0xC6, &reg);
	reg &= ~0x40;
	IIC_S2MU004_EWrite(S2MU004_W_ADDR, 0xC6, reg);

	/* MRSTBTMR set 1sec */
	IIC_S2MU004_ERead(S2MU004_R_ADDR, 0x71, &reg);
	reg &= 0xF8;
	IIC_S2MU004_EWrite(S2MU004_W_ADDR, 0x71, reg);
#if 0
	if (check_factory_mode())
		return;
#endif
	/* disable watchdog. */
	IIC_S2MU004_ERead(S2MU004_R_ADDR, S2MU004_CHG_CTRL13, &reg);
	reg &= 0xFC;
	IIC_S2MU004_EWrite(S2MU004_W_ADDR, S2MU004_CHG_CTRL13, reg);

	/* Float voltage setting */
	set_regulation_voltage(4350);

	/* EOC setting, 250mA */
	IIC_S2MU004_EWrite(S2MU004_W_ADDR, S2MU004_CHG_CTRL11, 0x06);

	/* Set VSYSMIN to 3.4V */
	IIC_S2MU004_ERead(S2MU004_R_ADDR, S2MU004_CHG_CTRL7, &reg);
	reg &= 0xE7;
	IIC_S2MU004_EWrite(S2MU004_W_ADDR, S2MU004_CHG_CTRL7, reg);

	/* CHGIN UVLO Level 4.3V */
	IIC_S2MU004_ERead(S2MU004_R_ADDR, S2MU004_CHG_UVLOCTRL, &reg);
	reg &= ~0xC0;
	reg |= 0x40;
	IIC_S2MU004_EWrite(S2MU004_W_ADDR, S2MU004_CHG_UVLOCTRL, reg);

	/* SYSTEM_CLK_ON_OFF_MODE data - VIO reset by VBUS fluctuation */
	IIC_S2MU004_ERead(S2MU004_R_ADDR, 0xC6 ,&reg);
	reg &= ~0x40;
	IIC_S2MU004_EWrite(S2MU004_W_ADDR, 0xC6, reg);

	/* ULDO enable */
	IIC_S2MU004_ERead(S2MU004_R_ADDR, 0x72 ,&reg);
	reg |= 0x80;
	IIC_S2MU004_EWrite(S2MU004_W_ADDR, 0x72, reg);

	/* maximum charging current setting - 2000mA */
	set_charger_current(2000);

	/* input current limit setting with USB Charger as a default */
	set_input_current(USB_CHG_CURR);

	/* enable charger */
	set_charger_state(1, 1);
}

void muic_sw_usb (void)
{
	unsigned char reg;

	IIC_S2MU004_ESetport();
	IIC_S2MU004_ERead(S2MU004_R_ADDR, S2MU004_MUIC_CTRL1, &reg);
	reg &= ~(0x1 << 2);
	IIC_S2MU004_EWrite(S2MU004_W_ADDR, S2MU004_MUIC_CTRL1, reg);
	IIC_S2MU004_ERead(S2MU004_R_ADDR, S2MU004_MANUAL_SW_CTRL, &reg);
	reg &= ~(0x7 << 2);
	reg &= ~(0x7 << 5);
	reg |= 0x1 << 2;
	reg |= 0x1 << 5;
	IIC_S2MU004_EWrite(S2MU004_W_ADDR, S2MU004_MANUAL_SW_CTRL, reg);
}

void muic_sw_uart (void)
{
	unsigned char reg;

	IIC_S2MU004_ESetport();
	IIC_S2MU004_ERead(S2MU004_R_ADDR, S2MU004_MUIC_CTRL1, &reg);
	reg &= ~(0x1 << 2);
	IIC_S2MU004_EWrite(S2MU004_W_ADDR, S2MU004_MUIC_CTRL1, reg);
	IIC_S2MU004_ERead(S2MU004_R_ADDR, S2MU004_MANUAL_SW_CTRL, &reg);
	reg &= ~(0x7 << 2);
	reg &= ~(0x7 << 5);
	reg |= 0x2 << 2;
	reg |= 0x2 << 5;
	IIC_S2MU004_EWrite(S2MU004_W_ADDR, S2MU004_MANUAL_SW_CTRL, reg);
}

int muic_get_vbus(void)
{
	u8 vbvolt = 0;

	IIC_S2MU004_ESetport();
	IIC_S2MU004_ERead(S2MU004_R_ADDR, S2MU004_MUIC_REG_DEVICE_APPLE, &vbvolt);
	vbvolt &= S2MU004_MUIC_DEV3_VBUS;

	printf("%s: vbvolt: 0x%02x\n", __func__, vbvolt);

	return vbvolt;
}
