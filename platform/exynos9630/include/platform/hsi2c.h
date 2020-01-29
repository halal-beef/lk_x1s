#ifndef __HSI2C_H__
#define __HSI2C_H__

#if __has_include("hsi2c_9630.h")
#include "hsi2c_9630.h"
#endif

#ifndef __HSI2C_9630_H__
static struct hsi2c_chan_info *dummy_match_hsi2c_chan_info(void) { return NULL; }
#define match_hsi2c_chan_info()	dummy_match_hsi2c_chan_info()
#define MAX_HSI2C_CHAN (0)
#endif

#endif	/* __HSI2C_H__ */
