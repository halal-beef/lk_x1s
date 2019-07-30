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

#define EXYNOS9830_KEYSTORAGE_NWD_ADDR		(0x89000000)
#define EXYNOS9830_KEYSTORAGE_PARTITION_SIZE	(16 * 512)
#define EXYNOS_AP_MAGIC				(0x49534c53)

#define EXYNOS9830_LDFW_NWD_ADDR		(0x88000000)
#define EXYNOS9830_LDFW_PARTITION_SIZE		(1024 * 1024 * 5)       /* 5 MB */
#define EXYNOS_LDFW_MAGIC			(0x10adab1e)

#define EXYNOS9830_SSP_NWD_ADDR			(EXYNOS9830_LDFW_NWD_ADDR)
#define EXYNOS9830_SSP_PARTITION_SIZE		(1024 * 512)

#define EXYNOS9830_SP_NWD_ADDR			(EXYNOS9830_LDFW_NWD_ADDR)
#define EXYNOS9830_SP_PARTITION_SIZE		(1024 * 1024)           /* 1 MB */

#define LDFW_DEBUG

#define CHECK_SIGNATURE_FAIL			(0x2)
#define CHECK_ROLL_BACK_COUNT_FAIL		(0x20)

#ifdef LDFW_DEBUG
#define LDFW_INFO(format, arg ...)	printf(format, ## arg)
#define LDFW_WARN(format, arg ...)	printf(format, ## arg)
#define LDFW_ERR(format, arg ...)	printf(format, ## arg)
#else
#define LDFW_INFO(msg)
#define LDFW_WARN(msg)
#define LDFW_ERR(format, arg ...)	printf(format, ## arg)
#endif


#ifndef __ASSEMBLY__
enum {
	LDFW_PART,
	KEYSTORAGE_PART,
	SSP_PART,
	TZSW_PART,
	MAX_CH_NUM,
};

int init_ldfws(void);
int init_keystorage(void);
int init_sp(void);
int init_ssp(void);

int is_usb_boot(void);
#endif	/* __ASSEMBLY__ */
#endif	/* __LDFW_H_ */
