/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __LDFW_H_
#define __LDFW_H_

#define EXYNOS9610_KEYSTORAGE_NWD_ADDR		(0x89000000)
#define EXYNOS9610_KEYSTORAGE_PARTITION_SIZE	(16 * 512)
#define EXYNOS_AP_MAGIC				(0x49534c53)

#define EXYNOS9610_LDFW_NWD_ADDR		(0x88000000)
#define EXYNOS9610_LDFW_PARTITION_SIZE		(8192 * 512)
#define EXYNOS_LDFW_MAGIC			(0x10adab1e)

#define LDFW_DEBUG

#define CHECK_SIGNATURE_FAIL			(0x2)
#define CHECK_ROLL_BACK_COUNT_FAIL		(0x20)

#ifdef LDFW_DEBUG
#define LDFW_INFO(format, arg...)		printf(format, ##arg)
#define LDFW_WARN(format, arg...)		printf(format, ##arg)
#define LDFW_ERR(format, arg...)		printf(format, ##arg)
#else
#define LDFW_INFO(msg)
#define LDFW_WARN(msg)
#define LDFW_ERR(format, arg...)		printf(format, ##arg)
#endif



#ifndef __ASSEMBLY__
enum {
	LDFW_PART,
	KEYSTORAGE_PART,
	MAX_CH_NUM,
};

int init_ldfws(void);
int init_keystorage(void);
#endif	/* __ASSEMBLY__ */
#endif	/* __LDFW_H_ */
