/* linux/drivers/video/decon_display/nt36672a_param.h
 *
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
 *
 * Hwangjae Lee <hj-yo.lee@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __NT36672A_PARAM_H__
#define __NT36672A_PARAM_H__

/* MIPI commands list */
static const unsigned char SEQ_SLEEP_OUT[] = {
	0x11,
};

static const unsigned char SEQ_DISPLAY_ON[] = {
	0x29,
};

static const unsigned char SEQ_DISPLAY_OFF[] = {
	0x28,
};

static const unsigned char SEQ_SLEEP_IN[] = {
	0x10,
};

static const unsigned char SEQ_CMD_0[] = {
	0x51, 0xFF
};

static const unsigned char SEQ_CMD_1[] = {
	0x53, 0x24
};

static const unsigned char SEQ_CMD_2[] = {
	0x55, 0x02
};

#endif /* __NT36672A_GAMMA_H__ */
