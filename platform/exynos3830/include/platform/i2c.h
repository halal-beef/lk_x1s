#ifndef __I2C_H__
#define __I2C_H__

#if __has_include("i2c_3830.h")
#include "i2c_3830.h"
#endif

#ifndef __I2C_3830_H__
static struct i2c_chan_info *dummy_match_i2c_chan_info(void) { return NULL; }
#define match_i2c_chan_info()	dummy_match_i2c_chan_info()
#define MAX_I2C_CHAN (0)
#endif

#endif	/* __I2C_H__ */
