/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __CHIP_REV_H__
#define __CHIP_REV_H__

struct chip_rev_info {
	unsigned int main;
	unsigned int sub;
};

extern struct chip_rev_info s5p_chip_rev;
#endif	/* __CHIP_REV_H__ */
