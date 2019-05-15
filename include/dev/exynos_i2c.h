#ifndef __EXYNOS_I2C_H__
#define __EXYNOS_I2C_H__

#include <stdio.h>
#include <dev/i2c_common.h>

struct i2c_chan_info {
	unsigned int i2c_base;
	unsigned int *gpio_scl;
	unsigned int *gpio_sda;
	unsigned int scl_pin;
	unsigned int sda_pin;
	unsigned int clk;
};

#define I2C_IDLE_TIMEOUT	(5000)
#define I2C_SCL_SDA		(0x02)

/* I2C REGISTER OFFSET */
#define I2CCON			0x00
#define I2CSTAT			0x04
#define I2CADD			0x08
#define I2CDS			0x0C
#define I2CLC			0x10

/* I2CCON Register bits */
#define I2CCON_ACKEN		(1 << 7)
#define I2CCON_TXDIV_16		(0 << 6)
#define I2CCON_TXDIV_512	(1 << 6)
#define I2CCON_IRQEN		(1 << 5)
#define I2CCON_IRQPEND		(1 << 4)
#define I2CCON_BUS_RELEASE	(1 << 4)
#define I2CCON_SCALE(x)		((x) & 0xf)
#define I2CCON_SCALEMASK	(0xf)

/* I2CSTAT Register bits */
#define I2CSTAT_MASTER_RX	(2 << 6)
#define I2CSTAT_MASTER_TX	(3 << 6)
#define I2CSTAT_SLAVE_RX	(0 << 6)
#define I2CSTAT_SLAVE_TX	(1 << 6)
#define I2CSTAT_MODEMASK	(3 << 6)

#define I2CSTAT_START		(1 << 5)
#define I2CSTAT_BUSBUSY		(1 << 5)
#define I2CSTAT_TXRXEN		(1 << 4)
#define I2CSTAT_ARBITR		(1 << 3)
#define I2CSTAT_ASSLAVE		(1 << 2)
#define I2CSTAT_ADDR0		(1 << 1)
#define I2CSTAT_LASTBIT		(1 << 0)

/* I2CCLC Register bits */
#define I2CLC_SDA_DELAY0	(0 << 0)
#define I2CLC_SDA_DELAY5	(1 << 0)
#define I2CLC_SDA_DELAY10	(2 << 0)
#define I2CLC_SDA_DELAY15	(3 << 0)
#define I2CLC_SDA_DELAY_MASK	(3 << 0)

#define I2CLC_FILTER_ON		(1 << 2)

extern int i2c_log_level;

#define i2c_err(fmt, ...)							\
	do {									\
		if (i2c_log_level >= 3) {					\
			printf(fmt, ##__VA_ARGS__);			\
		}								\
	} while (0)

#define i2c_warn(fmt, ...)							\
	do {									\
		if (i2c_log_level >= 4) {					\
			printf(fmt, ##__VA_ARGS__);			\
		}								\
	} while (0)

#define i2c_info(fmt, ...)							\
	do {									\
		if (i2c_log_level >= 6)					\
			printf(fmt, ##__VA_ARGS__);			\
	} while (0)

#define i2c_dbg(fmt, ...)							\
	do {									\
		if (i2c_log_level >= 7)					\
			printf(fmt, ##__VA_ARGS__);			\
	} while (0)

enum i2c_state {
	STATE_IDLE,
	STATE_STOP,
	STATE_START,
	STATE_READ,
	STATE_WRITE,
};

extern void i2c_init(struct i2c_info *i2c);
extern int i2c_xfer(struct i2c_info *i2c,
		struct i2c_msg *msgs, int num);
extern void i2c_write(struct i2c_info *i2c, unsigned int addr,
		unsigned int reg, unsigned int len, unsigned char *data);
extern void i2c_read(struct i2c_info *i2c, unsigned int addr,
		unsigned int reg, unsigned int len, unsigned char *data);

#endif  // __EXYNOS_I2C_H__
