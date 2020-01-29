#ifndef __EXYNOS_HSI2C_H__
#define __EXYNOS_HSI2C_H__

#include <stdio.h>
#include <dev/i2c_common.h>

struct hsi2c_chan_info {
	unsigned int hsi2c_addr;
	unsigned int sysreg_addr;
	unsigned int *gpio_scl;
	unsigned int *gpio_sda;
	unsigned int scl_pin;
	unsigned int sda_pin;
	unsigned int con_val;
	unsigned int sysreg_val;
	unsigned int clk;
};

#define I2C_SW_CONF			(1 << 2)

/* I2C REGISTER OFFSET */
#define CTL			0x0000
#define FIFO_CTL		0x0004
#define TRAILING_CTL		0x0008
#define INT_EN			0x0020
#define INT_STAT		0x0024
#define FIFO_STAT		0x0030
#define TXDATA			0x0034
#define RXDATA			0x0038
#define I2C_CONF		0x0040
#define I2C_AUTO_CONF		0x0044
#define I2C_TIMEOUT		0x0048
#define I2C_MANUAL_CMD		0x004C
#define I2C_TRANS_STATUS	0x0050
#define I2C_TIMING_HS1		0x0054
#define I2C_TIMING_HS2		0x0058
#define I2C_TIMING_HS3		0x005C
#define I2C_TIMING_FS1		0x0060
#define I2C_TIMING_FS2		0x0064
#define I2C_TIMING_FS3		0x0068
#define I2C_TIMING_SLA		0x006C
#define I2C_ADDR		0x0070

/* I2C_CTL Register bits */
#define HSI2C_FUNC_MODE_I2C			(1 << 0)
#define HSI2C_MASTER				(1 << 3)
#define HSI2C_RXCHON				(1 << 6)
#define HSI2C_TXCHON				(1 << 7)
#define HSI2C_EXT_MSB				(1 << 29)
#define HSI2C_EXT_ADDR				(1 << 30)
#define HSI2C_SW_RST				(1 << 31)

/* I2C_FIFO_CTL Register bits */
#define HSI2C_RXFIFO_EN				(1 << 0)
#define HSI2C_TXFIFO_EN				(1 << 1)
#define HSI2C_FIFO_MAX				(0x40)
#define HSI2C_RXFIFO_TRIGGER_LEVEL(x)	(x << 4)
#define HSI2C_TXFIFO_TRIGGER_LEVEL(x)	(x << 16)
/* I2C_TRAILING_CTL Register bits */
#define HSI2C_TRAILING_COUNT			(0xffffff)

/* I2C_INT_EN Register bits */
#define HSI2C_INT_TX_ALMOSTEMPTY_EN		(1 << 0)
#define HSI2C_INT_RX_ALMOSTFULL_EN		(1 << 1)
#define HSI2C_INT_TRAILING_EN			(1 << 6)
#define HSI2C_INT_TRANSFER_DONE			(1 << 7)
#define HSI2C_INT_I2C_EN			(1 << 9)
#define HSI2C_INT_CHK_TRANS_STATE		(0xf << 8)

/* I2C_INT_STAT Register bits */
#define HSI2C_INT_TX_ALMOSTEMPTY		(1 << 0)
#define HSI2C_INT_RX_ALMOSTFULL			(1 << 1)
#define HSI2C_INT_TX_UNDERRUN			(1 << 2)
#define HSI2C_INT_TX_OVERRUN			(1 << 3)
#define HSI2C_INT_RX_UNDERRUN			(1 << 4)
#define HSI2C_INT_RX_OVERRUN			(1 << 5)
#define HSI2C_INT_TRAILING			(1 << 6)
#define HSI2C_INT_I2C				(1 << 9)
#define HSI2C_INT_NODEV				(1 << 10)
#define HSI2C_RX_INT				(HSI2C_INT_RX_ALMOSTFULL | \
						 HSI2C_INT_RX_UNDERRUN | \
						 HSI2C_INT_RX_OVERRUN | \
						 HSI2C_INT_TRAILING)

/* I2C_FIFO_STAT Register bits */
#define HSI2C_RX_FIFO_EMPTY			(1 << 24)
#define HSI2C_RX_FIFO_FULL			(1 << 23)
#define HSI2C_RX_FIFO_LVL(x)			((x >> 16) & 0x7f)
#define HSI2C_RX_FIFO_LVL_MASK			(0x7F << 16)
#define HSI2C_TX_FIFO_EMPTY			(1 << 8)
#define HSI2C_TX_FIFO_FULL			(1 << 7)
#define HSI2C_TX_FIFO_LVL(x)			((x >> 0) & 0x7f)
#define HSI2C_TX_FIFO_LVL_MASK			(0x7F << 0)
#define HSI2C_FIFO_EMPTY			(HSI2C_RX_FIFO_EMPTY |	\
						HSI2C_TX_FIFO_EMPTY)

/* I2C_CONF Register bits */
#define HSI2C_AUTO_MODE				(1 << 31)
#define HSI2C_10BIT_ADDR_MODE			(1 << 30)
#define HSI2C_HS_MODE				(1 << 29)
#define HSI2C_FILTER_EN_SCL			(1 << 28)
#define HSI2C_FILTER_EN_SDA			(1 << 27)
#define HSI2C_FTL_CYCLE_SCL_MASK		(0x7 << 16)
#define HSI2C_FTL_CYCLE_SDA_MASK		(0x7 << 13)

/* I2C_AUTO_CONF Register bits */
#define HSI2C_READ_WRITE			(1 << 16)
#define HSI2C_STOP_AFTER_TRANS			(1 << 17)
#define HSI2C_MASTER_RUN			(1 << 31)

/* I2C_TIMEOUT Register bits */
#define HSI2C_TIMEOUT_EN			(1 << 31)

/* I2C_TRANS_STATUS register bits */
#define HSI2C_MASTER_BUSY			(1 << 17)
#define HSI2C_SLAVE_BUSY			(1 << 16)

/* I2C_ADDR register bits */
#define HSI2C_SLV_ADDR_SLV(x)			((x & 0x3ff) << 0)
#define HSI2C_SLV_ADDR_MAS(x)			((x & 0x3ff) << 10)
#define HSI2C_MASTER_ID(x)			((x & 0xff) << 24)
#define MASTER_ID(x)				((x & 0x7) + 0x08)

#define EXYNOS_FIFO_SIZE	(16)
#define FIFO_TRIG_CRITERIA	(8)

#define USI_CON			(0x00C4)
#define USI_RESET		(0 << 0)

#define MSEC_PER_SEC		(1000)

extern int hsi2c_log_level;

#define hsi2c_err(fmt, ...)							\
	do {									\
		if (hsi2c_log_level >= 3) {					\
			printf(fmt, ##__VA_ARGS__);				\
		}								\
	} while (0)

#define hsi2c_warn(fmt, ...)							\
	do {									\
		if (hsi2c_log_level >= 4) {					\
			printf(fmt, ##__VA_ARGS__);				\
		}								\
	} while (0)

#define hsi2c_info(fmt, ...)							\
	do {									\
		if (hsi2c_log_level >= 6)					\
			printf(fmt, ##__VA_ARGS__);				\
	} while (0)

#define hsi2c_dbg(fmt, ...)							\
	do {									\
		if (hsi2c_log_level >= 7)					\
			printf(fmt, ##__VA_ARGS__);				\
	} while (0)

extern void hsi2c_init(struct i2c_info *i2c);
extern int hsi2c_xfer(struct i2c_info *i2c,
			struct i2c_msg *msgs, int num);
extern int hsi2c_write(struct i2c_info *i2c, unsigned int addr,
			unsigned int reg, unsigned int len,
			unsigned char *data);
extern int hsi2c_read(struct i2c_info *i2c, unsigned int addr,
			unsigned int reg, unsigned int len,
			unsigned char *data);

#endif // __EXYNOS_HSI2C_H__
