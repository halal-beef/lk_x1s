#include <lk/reg.h>
#include <platform.h>
#include <platform/delay.h>
#include <platform/gpio.h>
#include <string.h>
#include <platform/i2c.h>
#include <dev/exynos_i2c.h>

int i2c_log_level = 3;

static struct i2c_chan_info *chan_info;

static int i2c_nextbyte(struct i2c_info *i2c, unsigned int i2cstat);

static void i2c_gpio_set(int chan)
{
	struct exynos_gpio_bank *bank_scl, *bank_sda;
	unsigned int scl_pin, sda_pin;

	bank_scl = (struct exynos_gpio_bank *)chan_info[chan].gpio_scl;
	bank_sda = (struct exynos_gpio_bank *)chan_info[chan].gpio_sda;
	scl_pin = chan_info[chan].scl_pin;
	sda_pin = chan_info[chan].sda_pin;

	exynos_gpio_cfg_pin(bank_scl, scl_pin, I2C_SCL_SDA);
	exynos_gpio_cfg_pin(bank_sda, sda_pin, I2C_SCL_SDA);

	i2c_dbg("i2c: con - scl(0x%08x, %u)\n", bank_scl->con, scl_pin);
	i2c_dbg("i2c: con - sda(0x%08x, %u)\n", bank_sda->con, sda_pin);
}

static void i2c_calcdivisor(unsigned int clkin, unsigned int wanted,
			unsigned int *div1, unsigned int *divs)
{
	unsigned int calc_divs = clkin / wanted;
	unsigned int calc_div1;
	unsigned int clk_prescaler;

	clk_prescaler = 16;

	if (calc_divs > (16*16))
		calc_div1 = 512;
	else
		calc_div1 = clk_prescaler;

	calc_divs += calc_div1 - 1;
	calc_divs /= calc_div1;

	if (calc_divs == 0)
		calc_divs = 1;
	if (calc_divs > (clk_prescaler + 1))
		calc_divs = clk_prescaler + 1;

	*divs = calc_divs;
	*div1 = calc_div1;

	i2c_dbg("i2c: freq(%u) = clkin(%u) / { divs(%u) * div1(%u) }\n",
		clkin / (calc_divs * calc_div1), clkin, calc_divs, calc_div1);
}

static void i2c_clockrate(unsigned int i2c_base, unsigned int clk, unsigned int clkin)
{
	unsigned int divs, div1;
	unsigned int target_frequency;
	unsigned int i2ccon;

	target_frequency = clk ? clk : I2C_STAND_TX_CLOCK;
	i2c_calcdivisor(clkin , target_frequency, &div1, &divs);

	i2ccon = readl(i2c_base + I2CCON);

	i2ccon &= ~(I2CCON_SCALEMASK | I2CCON_TXDIV_512);
	i2ccon |= (divs-1);

	if (div1 == 512)
		i2ccon |= I2CCON_TXDIV_512;

	writel(i2ccon, i2c_base + I2CCON);
	i2c_dbg("%s: i2c %08x: i2ccon = %08x-->%08x\n",
		__func__, i2c_base, i2ccon, readl(i2c_base + I2CCON));
}

/* i2c_bus_init
 *
 * initialise the controller, set the IO lines and frequency
*/

static int i2c_bus_init(struct i2c_info *i2c)
{
	unsigned int i2cstat, clkin;


	/* reset i2c bus to recover from "cannot get bus" */
	i2cstat = readl(i2c->base + I2CSTAT);
	i2cstat &= ~I2CSTAT_TXRXEN;
	writel(i2cstat, i2c->base + I2CSTAT);

	writel(0, i2c->base + I2CCON);
	writel(0, i2c->base + I2CSTAT);

	clkin = chan_info[i2c->chan].clk;
	i2c_clockrate(i2c->base, i2c->clk, clkin);

	return 0;
}

static inline void i2c_master_complete(struct i2c_info *i2c, int ret)
{
	i2c_dbg("i2c %08x: master_complete %d\n", i2c->base, ret);

	i2c->msg_ptr = 0;
	i2c->msg = NULL;
	i2c->msg_idx++;
	i2c->msg_num = 0;
	if (ret)
		i2c->msg_idx = ret;
}

static void i2c_disable_ack(struct i2c_info *i2c)
{
	unsigned int i2ccon;

	i2c_dbg("i2c %08x: %s\n", i2c->base, __func__);

	i2ccon = readl(i2c->base + I2CCON);
	i2ccon = i2ccon & ~I2CCON_ACKEN;
	writel(i2ccon, i2c->base + I2CCON);
}

static inline void i2c_disable_irq(struct i2c_info *i2c)
{
	unsigned int i2ccon;

	i2ccon = readl(i2c->base + I2CCON);
	i2ccon &= ~I2CCON_IRQEN;
	writel(i2ccon, i2c->base + I2CCON);
}

static inline void i2c_enable_irq(struct i2c_info *i2c)
{
	unsigned int i2ccon;

	i2ccon = readl(i2c->base + I2CCON);
	i2c_dbg("%s: i2c %08x: i2ccon = %08x\n",
		__func__, i2c->base, readl(i2c->base + I2CCON));
	i2ccon |= I2CCON_IRQEN;
	writel(i2ccon, i2c->base + I2CCON);
	i2c_dbg("%s: i2c %08x: i2ccon = %08x\n",
		__func__, i2c->base, readl(i2c->base + I2CCON));
}

static int is_ack(struct i2c_info *i2c)
{
	int tries;

	for (tries = 5000; tries; --tries) {
		/* Interrupt is pending */
		if (readl(i2c->base + I2CCON)
			& I2CCON_IRQPEND) {

			/* Master receive mode */
			if (i2c->state == STATE_READ)
				return 1;

			/* ACK is received */
			if (!(readl(i2c->base + I2CSTAT)
				& I2CSTAT_LASTBIT))
				return 1;
		}
		u_delay(10);
	}

	i2c_err("i2c %08x: %s: ack is not received\n", i2c->base, __func__);
	return 0;
}

static void i2c_message_start(struct i2c_info *i2c, struct i2c_msg *msg)
{
	unsigned int addr;
	unsigned int i2cstat;
	unsigned int i2ccon;

	addr = (msg->addr & 0x7f) << 1;
	i2cstat = I2CSTAT_TXRXEN;

	if (msg->flags & I2C_M_RD) {
		i2cstat |= I2CSTAT_MASTER_RX;
		addr |= 1;
	} else {
		i2cstat |= I2CSTAT_MASTER_TX;
	}

	i2ccon = readl(i2c->base + I2CCON);
	i2ccon |= I2CCON_ACKEN;
	writel(i2cstat, i2c->base + I2CSTAT);

	i2c_dbg("i2c %08x: START: %08x to I2CSTAT, %02x to DS\n",
		i2c->base, i2cstat, addr);
	writeb(addr, i2c->base + I2CDS);

	/* delay here to ensure the data byte has gotten onto the bus
	 * before the transaction is started */
	u_delay(1);

	writel(i2ccon, i2c->base + I2CCON);

	i2c_dbg("i2c %08x: i2ccon = %08x\n", i2c->base, i2ccon);
	i2cstat |= I2CSTAT_START;
	writel(i2cstat, i2c->base + I2CSTAT);

	if (i2c->state != STATE_START)
		return;
	while ((i2c->msg_num != 0) && is_ack(i2c)) {
		i2c_nextbyte(i2c, i2cstat);
		i2cstat = readl(i2c->base + I2CSTAT);

		if (i2cstat & I2CSTAT_ARBITR)
			i2c_err("i2c %08x: deal with arbitration loss\n",
				i2c->base);
	}
}

static void i2c_stop(struct i2c_info *i2c, int ret)
{
	unsigned int i2cstat;

	i2c_dbg("i2c %08x: %s\n", i2c->base, __func__);

	/* Stop the transfer */
	i2cstat = readl(i2c->base+ I2CSTAT);
	i2cstat &= ~I2CSTAT_START;
	writel(i2cstat, i2c->base + I2CSTAT);

	i2c_master_complete(i2c, ret);
	i2c_disable_irq(i2c);
}

/* is_lastmsg()
 * returns TRUE if the current message is the last in the set
*/
static inline int is_lastmsg(struct i2c_info *i2c)
{
	return i2c->msg_idx >= (i2c->msg_num - 1);
}

/* is_msglast
 * returns TRUE if we this is the last byte in the current message
*/
static inline int is_msglast(struct i2c_info *i2c)
{
	return i2c->msg_ptr == i2c->msg->len-1;
}

/* is_msgend
 * returns TRUE if we reached the end of the current message
*/
static inline int is_msgend(struct i2c_info *i2c)
{
	return i2c->msg_ptr >= i2c->msg->len;
}

static int i2c_nextbyte(struct i2c_info *i2c, unsigned int i2cstat)
{
	int ret = 0;
	unsigned int i2ccon;
	unsigned char byte;


	switch (i2c->state) {

	case STATE_IDLE:
		i2c_err("i2c %08x: %s: called in STATE_IDLE\n", i2c->base, __func__);
		goto out;

	case STATE_STOP:
		i2c_err("i2c %08x: %s: called in STATE_STOP\n", i2c->base, __func__);
		i2c_disable_irq(i2c);
		goto out_ack;

	case STATE_START:
		i2c_dbg("i2c %08x: %s: state - start\n", i2c->base, __func__);

		if (i2cstat & I2CSTAT_LASTBIT) {
			i2c_err("i2c %08x: %s: START: No ack\n", i2c->base, __func__);
			i2c_stop(i2c, -1);
			goto out_ack;
		}

		if (i2c->msg->flags & I2C_M_RD)
			i2c->state = STATE_READ;
		else
			i2c->state = STATE_WRITE;

		if (is_lastmsg(i2c) && i2c->msg->len == 0) {
			i2c_stop(i2c, 0);
			goto out_ack;
		}

		if (i2c->state == STATE_READ)
			goto prepare_read;

	case STATE_WRITE:
		i2c_dbg("i2c %08x: %s: state - write\n", i2c->base, __func__);

		if (i2cstat & I2CSTAT_LASTBIT) {
			i2c_err("i2c %08x: %s: WRITE: No ack\n", i2c->base, __func__);
			i2c_stop(i2c, -1);
			goto out_ack;
		}

		if (!is_msgend(i2c)) {
			byte = i2c->msg->buf[i2c->msg_ptr++];
			writeb(byte, i2c->base + I2CDS);

			i2c_dbg("i2c %08x: %s: write data 0x%x\n",
				i2c->base, __func__, byte);
			u_delay(1);
		} else if (!is_lastmsg(i2c)) {
			/* we need to go to the next i2c message */
			i2c_dbg("i2c %08x: write: next message\n", i2c->base);

			i2c->msg_ptr = 0;
			i2c->msg_idx++;
			i2c->msg++;

			/* send the new start */
			i2c_message_start(i2c, i2c->msg);
			i2c->state = STATE_START;
		} else {
			/* send stop */
			i2c_stop(i2c, 0);

			goto out;
		}
		break;

	case STATE_READ:
		i2c_dbg("i2c %08x: %s: state - read\n", i2c->base, __func__);

		/* we have a byte of data in the data register, do
		 * something with it, and then work out whether we are
		 * going to do any more read/write
		 */

		byte = readb(i2c->base + I2CDS);
		i2c->msg->buf[i2c->msg_ptr++] = byte;

		i2c_dbg("i2c %08x: %s: read data 0x%x\n", i2c->base, __func__, byte);
prepare_read:
		if (is_msglast(i2c)) {
			/* last byte of buffer */

			if (is_lastmsg(i2c))
				i2c_disable_ack(i2c);

		} else if (is_msgend(i2c)) {
			/* ok, we've read the entire buffer, see if there
			 * is anything else we need to do */

			if (is_lastmsg(i2c)) {
				/* last message, send stop and complete */
				i2c_dbg("i2c %08x: READ: Send Stop\n", i2c->base);

				i2c_stop(i2c, 0);

				goto out;
			} else {
				/* go to the next transfer */
				i2c_dbg("i2c %08x: READ: Next Transfer\n", i2c->base);

				i2c->msg_ptr = 0;
				i2c->msg_idx++;
				i2c->msg++;
			}
		}
		break;
	}

 out_ack:
	/* Clear pending irq */
	i2c_dbg("i2c %08x: clear pending irq\n", i2c->base);
	i2ccon = readl(i2c->base + I2CCON);
	i2ccon &= ~I2CCON_IRQPEND;
	writel(i2ccon, i2c->base + I2CCON);
 out:
	return ret;
}

static int i2c_set_master(struct i2c_info *i2c)
{
	unsigned int i2cstat;
	int timeout = 400;

	while (timeout-- > 0) {
		i2cstat = readl(i2c->base + I2CSTAT);

		if (!(i2cstat & I2CSTAT_BUSBUSY))
			return 0;

		u_delay(10);
	}
	return -1;
}

static int i2c_doxfer(struct i2c_info *i2c, struct i2c_msg *msgs, int msg_num)
{
	int ret;
	unsigned int i2cadd;

	i2cadd = (msgs->addr & 0x7F) << 1;
	writeb(i2cadd, i2c->base + I2CADD);

	ret = i2c_set_master(i2c);
	if (ret) {
		i2c_err("i2c %08x: %s: cannot get bus!\n"
			"I2C Stat Reg dump:\n"
			"I2CSTAT = 0x%08x\n"
			"I2CCON = 0x%08x\n"
			, i2c->base, __func__
			, readl(i2c->base + I2CSTAT)
			, readl(i2c->base + I2CCON));
		return -1;
	}

	i2c->msg = msgs;
	i2c->msg_num = msg_num;
	i2c->msg_ptr = 0;
	i2c->msg_idx = 0;
	i2c->state   = STATE_START;

	i2c_enable_irq(i2c);
	i2c_message_start(i2c, msgs);

	ret = i2c->msg_idx;
	if (ret != msg_num) {
		i2c_err("i2c %08x: %s: incomplete xfer (%d)\n"
			"I2C Stat Reg dump:\n"
			"I2CSTAT = 0x%08x\n"
			"I2CCON = 0x%08x\n"
			, i2c->base, __func__, ret
			, readl(i2c->base + I2CSTAT)
			, readl(i2c->base + I2CCON));

		goto out;
	}

 out:
	i2c->state = STATE_IDLE;

	return ret;
}

int i2c_xfer(struct i2c_info *i2c, struct i2c_msg *msgs, int msg_num)
{
	int ret, retry;

	if (!i2c->init_done) {
		i2c_err("i2c %08x: %s: channel(%d) not initialized\n",
			i2c->base, __func__, i2c->chan);
		return -1;
	}

	for (retry = 0; retry < 3; retry ++) {
		i2c_bus_init(i2c);
		ret = i2c_doxfer(i2c, msgs, msg_num);

		if (ret != -1)
			return ret;

		i2c_dbg("i2c %08x: %s: Retrying transmission (%d)\n",
			i2c->base, __func__, retry + 1);
	}

	/* Transfer error */
	if (ret) {
		i2c_err("i2c %08x: %s: Transfer error occured\n",
			i2c->base, __func__);
	}

	return -1;
}

void i2c_write(struct i2c_info *i2c, unsigned int addr,
		unsigned int reg, unsigned int len, unsigned char *data)
{
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

	i2c_dbg("i2c %08x: %s: msg[0]: chan=%d, addr=0x%x, flags=%u, len=%u, buf[0]=0x%x, buf[1]=0x%x\n",
		i2c->base, __func__, i2c->chan, msg[0].addr,
		msg[0].flags, msg[0].len, msg[0].buf[0], msg[0].buf[1]);

	i2c_xfer(i2c, msg, 1);
}

void i2c_read(struct i2c_info *i2c, unsigned int addr,
		unsigned int reg, unsigned int len, unsigned char *data)
{
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

	i2c_dbg("i2c %08x: %s: msg[0]: chan=%d, addr=0x%x, flags=%u, len=%u, buf=0x%x\n",
		i2c->base, __func__, i2c->chan, msg[0].addr,
		msg[0].flags, msg[0].len, msg[0].buf[0]);
	i2c_dbg("i2c %08x: %s: msg[1]: chan=%d, addr=0x%x, flags=%u, len=%u\n",
		i2c->base, __func__, i2c->chan, msg[1].addr, msg[1].flags, msg[1].len);

	i2c_xfer(i2c, msg, 2);
}

void i2c_init(struct i2c_info *i2c)
{
	unsigned int clkin;

	i2c_info("i2c: initilizing i2c chan%d\n", i2c->chan);

	if (i2c->chan < 0 || (MAX_I2C_CHAN && i2c->chan >= MAX_I2C_CHAN)) {
		i2c_err("i2c: wrong channel number(%d)\n", i2c->chan);
		return;
	}

	chan_info = match_i2c_chan_info();
	if (!chan_info)
		i2c_err("i2c: cannot get i2c register info\n");

	i2c->base = chan_info[i2c->chan].i2c_base;

	clkin = chan_info[i2c->chan].clk;

	i2c_gpio_set(i2c->chan);
	i2c_clockrate(i2c->base, i2c->clk, clkin);

	i2c->init_done = 1;
}
