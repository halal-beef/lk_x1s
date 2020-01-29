#ifndef __I2C_COMMON_H__
#define __I2C_COMMON_H__

#define I2C_M_WR		(0x0000)
#define I2C_M_RD		(0x0001)

/*
 * Controller operating frequency, timing values for operation
 * are calculated against this frequency
 */
#define I2C_HS_TX_CLOCK		(2500000)
#define I2C_FAST_PLUS_TX_CLOCK	(1000000)
#define I2C_FS_TX_CLOCK		(400000)
#define I2C_STAND_TX_CLOCK	(100000)

#define I2C_STAND_SPD		(3)
#define I2C_FAST_PLUS_SPD	(2)
#define I2C_HIGH_SPD		(1)
#define I2C_FAST_SPD		(0)

struct i2c_msg {
	unsigned int addr;
	unsigned int flags;
	unsigned int len;
	unsigned char *buf;
};

struct i2c_info {
	int chan;
	unsigned int clk;
	unsigned int base;
	unsigned int init_done;

	/* for exynos hsi2c */
	int mode;

	/* for exynos i2c */
	int state;
	struct i2c_msg *msg;
	unsigned int msg_num;
	unsigned int msg_idx;
	unsigned int msg_ptr;
};

#endif  // __I2C_COMMON_H__
