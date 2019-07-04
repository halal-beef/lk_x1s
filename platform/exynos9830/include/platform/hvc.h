/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef _HVC_H_
#define _HVC_H_

/* HVC FID */
#define HVC_CMD_GET_HARX_INFO				(0xC6000020)

#ifndef __ASSEMBLY__
#include <sys/types.h>

uint64_t exynos_hvc(uint64_t hvc_fid,
		    uint64_t arg1,
		    uint64_t arg2,
		    uint64_t arg3,
		    uint64_t arg4);
#endif /* __ASSEMBLY__ */
#endif	/* _HVC_H_ */
