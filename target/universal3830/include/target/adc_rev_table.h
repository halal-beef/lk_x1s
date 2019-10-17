/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __ADC_REV_TABLE_H__
#define __ADC_REV_TABLE_H__

#define REV_ADC_COUNT	1
#define SHIFT_BIT	4

#define TABLE_SHEET			\
{				\
	{0, 137},		\
	{137, 410},		\
	{410, 683},		\
	{683, 956},		\
	{956, 1229},		\
	{1229, 1502},		\
	{1502, 1775},		\
	{1775, 2048},		\
	{2048, 2321},		\
	{2321, 2594},		\
	{2594, 2867},		\
	{2867, 3140},		\
	{3140, 3413},		\
	{3413, 3686},		\
	{3686, 3959},		\
	{3959, 4095},		\
}

struct b_rev_adc_info b_rev_adc[REV_ADC_COUNT] = {
	{
		.ch = 3,
		.bits = SHIFT_BIT,
		.table = TABLE_SHEET,
		.level = 16,
	},
};
#endif
