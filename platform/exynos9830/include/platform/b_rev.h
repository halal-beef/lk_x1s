/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */


#ifndef __B_REV_H
#define __B_REV_H

struct b_rev_adc_info {
	int ch;
	int bits;
	int table[256];
	int levels;
	int dt;
};

struct b_rev_gpio_info {
	struct exynos_gpio_bank *bank;
	int bit;
};

#endif // __B_REV_H
