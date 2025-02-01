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
	------------------------------------------------------------
	HW	Divided	Target	Voltage(V)	ADC	Range
	Rev.	Vol(V)	Vol(V)	Range		Value	Min	Max
	------------------------------------------------------------
	0	0	0	0.000~0.094	0	0	136
	1	0.117	0.12	0.095~0.150	273	137	409
	2	0.247	0.24	0.200~0.292	546	410	682
	3	0.363	0.36	0.310~0.419	819	683	955
	4	0.48	0.48	0.422~0.538	1092	956	1228
	5	0.6	0.6	0.550~0.650	1365	1229	1501
	6	0.72	0.72	0.670~0.770	1638	1502	1774
	7	0.861	0.84	0.780~0.910	1911	1775	2047
	8	0.982	0.96	0.911~1.030	2185	2048	2321
	9	1.08	1.08	1.031~1.120	2458	2322	2594
	10	1.174	1.2	1.121~1.250	2731	2595	2867
	11	1.32	1.32	1.270~1.370	3004	2868	3140
	12	1.437	1.44	1.380~1.500	3277	3141	3413
	13	1.553	1.56	1.501~1.620	3550	3414	3686
	14	1.683	1.68	1.621~1.750	3823	3687	3959
	------------------------------------------------------------
*/

struct b_rev_adc_info b_rev_adc[B_REV_ADC_LINES] = {
	{	// HW_REV0
		.ch = 6,
		.bits = 4,
		.table = {    0,  273,  546,  819, 1092,
			   1365, 1638, 1911, 2185, 2458,
			   2731, 3004, 3277, 3550, 3823
			 },
		.levels = 15,
		.dt = 136,
	},

};

#endif /*__B_REV_ADC_H__*/
