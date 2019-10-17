/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __BOARD_REV_H__
#define __BOARD_REV_H__

#define REV_TABLE_SIZE	256

/*
 * ch: ADC channel
 * bits: Shift bits
 * table: ADC range according to revision level
 * level: Number of revision level
 */

struct b_rev_adc_info {
	int ch;
	int bits;
	int table[REV_TABLE_SIZE][2];
	int level;
};

#ifdef CONFIG_GET_B_REV_FROM_ADC
int get_board_rev_adc(int *sh);
#else
#define get_board_rev_adc(a)	(0)
#endif

#ifdef CONFIG_GET_B_REV_FROM_GPIO
int get_board_rev_gpio(void);
#else
#define get_board_rev_gpio()	(0)
#endif

int get_board_rev(void);
#endif
