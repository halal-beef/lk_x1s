/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __B_REV_ADC_H__

#define B_REV_ADC_LINES	1
/*
	HW REV	ADC	Taget(V)
	0	0	0.00
	1	273	0.12
	2	546	0.24
	3	819	0.36
	4	1092	0.48
	5	1365	0.60
	6	1638	0.72
	7	1911	0.84
	8	2184	0.96
	9	2457	1.08
	10	2730	1.20
	11	3003	1.32
	12	3276	1.44
	13	3549	1.56
	14	3822	1.68
	15	4095	1.80
*/

struct b_rev_adc_info b_rev_adc[B_REV_ADC_LINES] = {
	{
		.ch = 4,
		.bits = 4,
		.table = {    0,  273,  546,  819, 1092, 1365, 1638, 1911,
			   2184, 2457, 2730, 3003, 3276, 3549, 3822, 4095},
		.levels = 16,
		.dt = 100,
	},
};

#endif /*__B_REV_ADC_H__*/
