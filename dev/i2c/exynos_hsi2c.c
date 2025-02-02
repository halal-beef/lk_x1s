#include <lk/reg.h>
#include <platform.h>
#include <platform/delay.h>
#include <platform/gpio.h>
#include <string.h>
#include <platform/hsi2c.h>
#include <dev/exynos_hsi2c.h>

int hsi2c_log_level = 3;

static struct hsi2c_chan_info *chan_info;

static void set_bits(unsigned int val, unsigned int addr, unsigned int offset)
{
	unsigned int reg_val;

	reg_val = readl(addr);
	reg_val &= ~(0xf << offset);
	reg_val |= val << offset;
	writel(reg_val, addr);
}

static void hsi2c_gpio_set(int chan)
{
	struct exynos_gpio_bank *bank_scl, *bank_sda;
	unsigned int scl_pin;
	unsigned int sda_pin;
	unsigned int con_val;

	bank_scl = (struct exynos_gpio_bank *)chan_info[chan].gpio_scl;
	bank_sda = (struct exynos_gpio_bank *)chan_info[chan].gpio_sda;

	scl_pin = chan_info[chan].scl_pin;
	sda_pin = chan_info[chan].sda_pin;
	con_val = chan_info[chan].con_val;

	exynos_gpio_cfg_pin(bank_scl, scl_pin, GPIO_FUNC(con_val));
	exynos_gpio_cfg_pin(bank_sda, sda_pin, GPIO_FUNC(con_val));

	hsi2c_dbg("hsi2c: con - scl(0x%08x, %u) -> %u\n",
		bank_scl->con, scl_pin, con_val);

	hsi2c_dbg("hsi2c: con - sda(0x%08x, %u) -> %u\n",
		bank_sda->con, sda_pin, con_val);
}

static void hsi2c_usi_sw_conf(int chan)
{
	unsigned int sysreg_addr;

	sysreg_addr = chan_info[chan].sysreg_addr;
	set_bits(I2C_SW_CONF, sysreg_addr, 0);

	hsi2c_dbg("hsi2c: %s: sysreg(0x%08x) -> %u\n", __func__, sysreg_addr, readl(sysreg_addr));
}

static void hsi2c_set_timing(unsigned int hsi2c_addr, unsigned int ipclk, int mode)
{
	unsigned int op_clk;

	unsigned int hs_div, uTSCL_H_HS, uTSTART_HD_HS;
	unsigned int fs_div, uTSCL_H_FS, uTSTART_HD_FS;
	unsigned int utemp;

	if (mode == I2C_STAND_SPD) {
		op_clk = I2C_STAND_TX_CLOCK;

		fs_div = ipclk / (op_clk * 16);
		fs_div &= 0xFF;
		utemp = readl(hsi2c_addr + I2C_TIMING_FS3);
		utemp &= ~0x00FF0000;
		utemp |= fs_div << 16;
		writel(utemp, hsi2c_addr + I2C_TIMING_FS3);

		uTSCL_H_FS = (25 * (ipclk / (1000 * 1000))) / ((fs_div + 1) * 10);
		uTSCL_H_FS = (0xFF << uTSCL_H_FS) & 0xFF;
		utemp = readl(hsi2c_addr + I2C_TIMING_FS2);
		utemp &= ~0x000000FF;
		utemp |= uTSCL_H_FS << 0;
		writel(utemp, hsi2c_addr + I2C_TIMING_FS2);

		uTSTART_HD_FS = (25 * (ipclk / (1000 * 1000))) / ((fs_div + 1) * 10) - 1;
		uTSTART_HD_FS = (0xFF << uTSTART_HD_FS) & 0xFF;

		utemp = readl(hsi2c_addr + I2C_TIMING_FS1);
		utemp &= ~0x00FF0000;
		utemp |= uTSTART_HD_FS << 16;
		writel(utemp, hsi2c_addr + I2C_TIMING_FS1);
	} else if (mode == I2C_FAST_PLUS_SPD) {
		op_clk = I2C_FAST_PLUS_TX_CLOCK;

		fs_div = ipclk / (op_clk * 16) - 1;

		fs_div &= 0xFF;
		utemp = readl(hsi2c_addr + I2C_TIMING_FS3);
		utemp &= ~0x00FF0000;
		utemp |= fs_div << 16;
		writel(utemp, hsi2c_addr + I2C_TIMING_FS3);

		uTSCL_H_FS = (4 * (ipclk / (1000 * 1000))) / ((fs_div + 1) * 10);
		uTSCL_H_FS = (0xFF << uTSCL_H_FS) & 0xFF;
		utemp = readl(hsi2c_addr + I2C_TIMING_FS2);
		utemp &= ~0x000000FF;
		utemp |= uTSCL_H_FS << 0;
		writel(utemp, hsi2c_addr + I2C_TIMING_FS2);

		uTSTART_HD_FS = (4 * (ipclk / (1000 * 1000))) / ((fs_div + 1) * 10) - 1;
		uTSTART_HD_FS = (0xFF << uTSTART_HD_FS) & 0xFF;
		utemp = readl(hsi2c_addr + I2C_TIMING_FS1);
		utemp &= ~0x00FF0000;
		utemp |= uTSTART_HD_FS << 16;
		writel(utemp, hsi2c_addr + I2C_TIMING_FS1);
	} else if (mode == I2C_HIGH_SPD) {
		/* ipclk's unit is Hz, op_clk's unit is Hz */
		op_clk = I2C_HS_TX_CLOCK;
		hs_div = ipclk / (op_clk * 15);
		hs_div &= 0xFF;
		utemp = readl(hsi2c_addr + I2C_TIMING_HS3);
		utemp &= ~0x00FF0000;
		utemp |= hs_div << 16;
		writel(utemp, hsi2c_addr + I2C_TIMING_HS3);

		uTSCL_H_HS = ((7 * ipclk) / (1000 * 1000)) / ((hs_div + 1) * 100);
		/* make to 0 into TSCL_H_HS from LSB */
		uTSCL_H_HS = (0xFFFFFFFF >> uTSCL_H_HS) << uTSCL_H_HS;
		uTSCL_H_HS &= 0xFF;
		utemp = readl(hsi2c_addr + I2C_TIMING_HS2);
		utemp &= ~0x000000FF;
		utemp |= uTSCL_H_HS << 0;
		writel(utemp, hsi2c_addr + I2C_TIMING_HS2);

		uTSTART_HD_HS = (7 * ipclk / (1000 * 1000)) / ((hs_div + 1) * 100) - 1;
		/* make to 0 into uTSTART_HD_HS from LSB */
		uTSTART_HD_HS = (0xFFFFFFFF >> uTSTART_HD_HS) << uTSTART_HD_HS;
		uTSTART_HD_HS &= 0xFF;
		utemp = readl(hsi2c_addr + I2C_TIMING_HS1);
		utemp &= ~0x00FF0000;
		utemp |= uTSTART_HD_HS << 16;
		writel(utemp, hsi2c_addr + I2C_TIMING_HS1);

	} else {
		/* Fast speed mode */
		/* ipclk's unit is Hz, op_clk's unit is Hz */
		op_clk = I2C_FS_TX_CLOCK;
		fs_div = ipclk / (op_clk * 15);
		fs_div &= 0xFF;
		utemp = readl(hsi2c_addr + I2C_TIMING_FS3);
		utemp &= ~0x00FF0000;
		utemp |= fs_div << 16;
		writel(utemp, hsi2c_addr + I2C_TIMING_FS3);

		uTSCL_H_FS = ((9 * ipclk) / (1000 * 1000)) / ((fs_div + 1) * 10);
		/* make to 0 into TSCL_H_FS from LSB */
		uTSCL_H_FS = (0xFFFFFFFF >> uTSCL_H_FS) << uTSCL_H_FS;
		uTSCL_H_FS &= 0xFF;
		utemp = readl(hsi2c_addr + I2C_TIMING_FS2);
		utemp &= ~0x000000FF;
		utemp |= uTSCL_H_FS << 0;

		uTSTART_HD_FS = (9 * ipclk / (1000 * 1000)) / ((fs_div + 1) * 10) - 1;
		/* make to 0 into uTSTART_HD_FS from LSB */
		uTSTART_HD_FS = (0xFFFFFFFF >> uTSTART_HD_FS) << uTSTART_HD_FS;
		uTSTART_HD_FS &= 0xFF;
		utemp = readl(hsi2c_addr + I2C_TIMING_FS1);
		utemp &= ~0x00FF0000;
		writel(utemp, hsi2c_addr + I2C_TIMING_FS1);
	}

}

static void hsi2c_clock_setup(struct i2c_info *i2c)
{
	unsigned int ipclk = chan_info[i2c->chan].clk;

	hsi2c_dbg("hsi2c %08x: %s: speed_mode=%d\n",
		  i2c->base, __func__, i2c->mode);

	if (i2c->mode == I2C_HIGH_SPD)
		hsi2c_set_timing(i2c->base, ipclk, I2C_FAST_SPD);
	hsi2c_set_timing(i2c->base, ipclk, i2c->mode);
}

static void hsi2c_i2c_reset(struct i2c_info *i2c)
{
	unsigned int i2c_ctl;
	unsigned int i2c_conf;

	hsi2c_dbg("hsi2c %08x: %s\n", i2c->base, __func__);
	/* Set and clear the bit for reset */
	i2c_ctl = readl(i2c->base + CTL);
	i2c_ctl |= HSI2C_SW_RST;
	writel(i2c_ctl, i2c->base + CTL);

	i2c_ctl = readl(i2c->base + CTL);
	i2c_ctl &= ~HSI2C_SW_RST;
	writel(i2c_ctl, i2c->base + CTL);

	/* We don't expect calculations to fail during the run */
	hsi2c_clock_setup(i2c);
	/* Initialize the configure registers */
	i2c_conf = readl(i2c->base + I2C_CONF);

	writel(HSI2C_MASTER, i2c->base + CTL);
	writel(HSI2C_TRAILING_COUNT, i2c->base + TRAILING_CTL);

	if (i2c->mode == I2C_HIGH_SPD) {
		writel(HSI2C_MASTER_ID(MASTER_ID(i2c->chan)),
					i2c->base + I2C_ADDR);
		i2c_conf |= HSI2C_HS_MODE;
	}

	i2c_conf |= HSI2C_AUTO_MODE;
	writel(i2c_conf, i2c->base + I2C_CONF);
}

static int hsi2c_xfer_msg(struct i2c_info *i2c, struct i2c_msg *msg, int stop)
{
	unsigned int i2c_ctl;
	unsigned int i2c_auto_conf;
	unsigned int i2c_timeout;
	unsigned int i2c_addr;
	unsigned int i2c_fifo_ctl;
	unsigned int int_en;
	unsigned int int_status;
	unsigned int fifo_status;
	unsigned int trans_status;
	unsigned char byte;
	unsigned int  msg_ptr = 0;
	int ret = 0;

	hsi2c_dbg("hsi2c %08x: %s\n", i2c->base, __func__);

	i2c_ctl = readl(i2c->base + CTL);
	i2c_auto_conf = readl(i2c->base + I2C_AUTO_CONF);
	i2c_timeout = readl(i2c->base + I2C_TIMEOUT);
	i2c_timeout &= ~HSI2C_TIMEOUT_EN;
	writel(i2c_timeout, i2c->base + I2C_TIMEOUT);

	/*
	 * In case of short length request it'd be better to set
	 * trigger level as msg length
	 */
	if (msg->len >= FIFO_TRIG_CRITERIA) {
		i2c_fifo_ctl = HSI2C_RXFIFO_EN | HSI2C_TXFIFO_EN |
			HSI2C_RXFIFO_TRIGGER_LEVEL(FIFO_TRIG_CRITERIA) |
			HSI2C_TXFIFO_TRIGGER_LEVEL(FIFO_TRIG_CRITERIA);
	} else {
		i2c_fifo_ctl = HSI2C_RXFIFO_EN | HSI2C_TXFIFO_EN |
			HSI2C_RXFIFO_TRIGGER_LEVEL(msg->len) |
			HSI2C_TXFIFO_TRIGGER_LEVEL(msg->len);
	}

	writel(i2c_fifo_ctl, i2c->base + FIFO_CTL);

	if (msg->flags == I2C_M_RD) {
		i2c_ctl &= ~HSI2C_TXCHON;
		i2c_ctl |= HSI2C_RXCHON;

		i2c_auto_conf |= HSI2C_READ_WRITE;
	} else {
		i2c_ctl &= ~HSI2C_RXCHON;
		i2c_ctl |= HSI2C_TXCHON;

		i2c_auto_conf &= ~HSI2C_READ_WRITE;
	}

	if (stop == 1)
		i2c_auto_conf |= HSI2C_STOP_AFTER_TRANS;
	else
		i2c_auto_conf &= ~HSI2C_STOP_AFTER_TRANS;

	i2c_addr = readl(i2c->base + I2C_ADDR);
	i2c_addr &= ~(0x3ff << 10);
	i2c_addr &= ~(0x3ff << 0);
	i2c_addr &= ~(0xff << 24);
	i2c_addr |= ((msg->addr & 0x7f) << 10);
	writel(i2c_addr, i2c->base + I2C_ADDR);

	writel(i2c_ctl, i2c->base + CTL);

	writel(HSI2C_INT_TRANSFER_DONE, i2c->base + INT_EN);

	i2c_auto_conf &= ~(0xffff);
	i2c_auto_conf |= msg->len;
	writel(i2c_auto_conf, i2c->base + I2C_AUTO_CONF);

	i2c_auto_conf = readl(i2c->base + I2C_AUTO_CONF);
	i2c_auto_conf |= HSI2C_MASTER_RUN;
	writel(i2c_auto_conf, i2c->base + I2C_AUTO_CONF);

	ret = -1;
	if (msg->flags == I2C_M_RD) {
		hsi2c_dbg("hsi2c %08x: %s: msg - Read\n", i2c->base, __func__);
		lk_time_t timeout;

		timeout = current_time() + MSEC_PER_SEC;
		/* 1sec polling */
		while (current_time() <= timeout) {
			if ((readl(i2c->base + FIFO_STAT) &
				HSI2C_RX_FIFO_EMPTY) == 0) {
				/* RX FIFO is not empty */
				byte = (unsigned char)readl
					(i2c->base + RXDATA);
				msg->buf[msg_ptr++] = byte;
				hsi2c_dbg("hsi2c %08x: %s: read data 0x%x from RXDATA reg\n",
					  i2c->base, __func__, byte);

			}

			if (msg_ptr >= msg->len) {
				int_status = readl(i2c->base + INT_STAT);

				if (int_status & HSI2C_INT_TRANSFER_DONE) {
					writel(int_status, i2c->base + INT_STAT);
					ret = 0;
					break;
				}
			}
		}

		/* RX Err */
		if (ret == -1)
			goto err;
	} else {
		unsigned int fifo_status;
		lk_time_t timeout;

		hsi2c_dbg("hsi2c %08x: %s: msg - Write\n", i2c->base,  __func__);
		timeout = current_time() + MSEC_PER_SEC;
		/* 1sec polling */
		while ((current_time() <= timeout) &&
			(msg_ptr < msg->len)) {
			if ((readl(i2c->base + FIFO_STAT)
				& HSI2C_TX_FIFO_LVL_MASK) < EXYNOS_FIFO_SIZE) {
				byte = msg->buf[msg_ptr++];
				writel(byte, i2c->base + TXDATA);
				hsi2c_dbg("hsi2c %08x: %s: write data 0x%x to TXDATA reg\n",
					  i2c->base, __func__, byte);
			}
		}

		while (current_time() <= timeout) {
			int_status = readl(i2c->base + INT_STAT);
			fifo_status = readl(i2c->base + FIFO_STAT);

			if ((int_status & HSI2C_INT_TRANSFER_DONE) &&
				(fifo_status & HSI2C_TX_FIFO_EMPTY)) {
				writel(int_status, i2c->base + INT_STAT);
				ret = 0;
				break;
			}
			u_delay(1);
		}

		/* TX Err */
		if (ret == -1)
			goto err;
	}

	return 0;

 err:
	hsi2c_err("hsi2c %08x: %s: %s Error\n",
		  i2c->base, __func__, ((msg->flags & I2C_M_RD) ? "RX" : "TX"));

	/* Dump i2c register */
	i2c_ctl = readl(i2c->base + CTL);
	i2c_fifo_ctl = readl(i2c->base + FIFO_CTL);
	int_en = readl(i2c->base + INT_EN);

	int_status = readl(i2c->base + INT_STAT);
	fifo_status = readl(i2c->base + FIFO_STAT);
	trans_status = readl(i2c->base + I2C_TRANS_STATUS);

	hsi2c_err("hsi2c %08x: ctl=0x%x, fifoctl=0x%x, int_en=0x%x\n",
		  i2c->base, i2c_ctl, i2c_fifo_ctl, int_en);
	hsi2c_err("hsi2c %08x: int_stat=0x%x, fifo_stat=0x%x, trans_stat=0x%x\n",
		  i2c->base, int_status, fifo_status, trans_status);

	hsi2c_i2c_reset(i2c);
	return ret;
}

int hsi2c_xfer(struct i2c_info *i2c, struct i2c_msg *msgs, int num)
{
	struct i2c_msg *msgs_ptr = msgs;
	int ret, retry;
	int i = 0;
	int stop = 0;

	if (!i2c->init_done) {
		hsi2c_err("hsi2c %08x: %s: channel(%d) not initialized\n",
			  i2c->base, __func__, i2c->chan);
		return -1;
	}

	for (retry = 0; retry < 5; retry++) {
		for (i = 0; i < num; i++) {
			if (i == num - 1)
				stop = 1;

			ret = hsi2c_xfer_msg(i2c, msgs_ptr, stop);
			msgs_ptr++;

			/* transfer error */
			if (ret == -1) {
				msgs_ptr = msgs;
				break;
			}
		}

		if ((i == num) && (ret != -1))
			break;
		hsi2c_dbg("hsi2c %08x: %s: retrying transfer (%d)\n",
			  i2c->base, __func__, retry);

		u_delay(100);
	}

	if (i == num) {
		ret  = num;
	} else {
		ret = -1;
		hsi2c_err("hsi2c %08x: %s: xfer message failed\n",
			  i2c->base, __func__);
	}

	return ret;
}

int hsi2c_write(struct i2c_info *i2c, unsigned int addr,
		     unsigned int reg, unsigned int len, unsigned char *data)
{
	int ret;
	unsigned char buf[256];
	struct i2c_msg msg[] = {
		{
			.addr = addr,
			.flags = I2C_M_WR,
			.len = len + 1,
			.buf = buf,
		}
	};

	buf[0] = reg & 0xff;
	memcpy(buf + 1, data, len);

	hsi2c_dbg("hsi2c %08x: %s: msg[0]: addr=0x%x, flags=%u, len=%u, buf=0x%x\n",
		  i2c->base, __func__, msg[0].addr, msg[0].flags, msg[0].len, msg[0].buf[0]);

	ret = hsi2c_xfer(i2c, msg, 1);
	return ret;
}

int hsi2c_read(struct i2c_info *i2c, unsigned int addr,
		    unsigned int reg, unsigned int len, unsigned char *data)
{
	int ret;
	unsigned char buf;
	struct i2c_msg msg[] = {
		{
			.addr = addr,
			.flags = I2C_M_WR,
			.len = 1,
			.buf = &buf,
		},
		{
			.addr = addr,
			.flags = I2C_M_RD,
			.len = len,
			.buf = data,
		},
	};

	buf = reg & 0xff;

	hsi2c_dbg("hsi2c %08x: %s: msg[0]: addr=0x%x, flags=%u, len=%u, buf=0x%x\n",
		  i2c->base, __func__, msg[0].addr, msg[0].flags, msg[0].len, msg[0].buf[0]);
	hsi2c_dbg("hsi2c %08x: %s: msg[1]: addr=0x%x, flags=%u, len=%u\n",
		  i2c->base, __func__, msg[1].addr, msg[1].flags, msg[1].len);

	ret = hsi2c_xfer(i2c, msg, 2);
	return ret;
}

void hsi2c_init(struct i2c_info *i2c)
{
	hsi2c_info("hsi2c: initilizing hsi2c chan%d\n", i2c->chan);
	if (i2c->chan < 0 || (MAX_HSI2C_CHAN && i2c->chan >= MAX_HSI2C_CHAN)) {
		hsi2c_err("hsi2c: wrong channel number(%d)\n", i2c->chan);
		return;
	}

	chan_info = match_hsi2c_chan_info();
	if (!chan_info)
		hsi2c_err("hsi2c: cannot get hsi2c register info\n");

	i2c->base = chan_info[i2c->chan].hsi2c_addr;
	hsi2c_dbg("hsi2c: hsi2c_addr=0x%08x\n", i2c->base);

	hsi2c_gpio_set(i2c->chan);
	hsi2c_usi_sw_conf(i2c->chan);

	writel(USI_RESET, i2c->base + USI_CON);

	hsi2c_i2c_reset(i2c);

	i2c->init_done = 1;
}
