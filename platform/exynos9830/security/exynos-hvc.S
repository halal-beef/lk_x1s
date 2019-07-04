/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

/*
 * Function signature:
 *
 * unsigned long exynos_hvc(unsigned long hvc_fid,
 *			    unsigned long arg1,
 *			    unsigned long arg2,
 *			    unsigned long arg3,
 *			    unsigned long arg4)
 */
	.globl	exynos_hvc
exynos_hvc:
	dsb	sy
	hvc	#0
	ret
