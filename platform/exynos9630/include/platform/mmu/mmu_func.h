/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef MMU_FUNC_H_
#define MMU_FUNC_H_

void invalidate_dcache_all(void);
void invalidate_dcache_range(unsigned long long, unsigned long long);
void clean_invalidate_dcache_all(void);
void clean_dcache_range(unsigned long long, unsigned long long);
void clean_invalidate_dcache_range(unsigned long long, unsigned long long);
void disable_mmu_dcache(void);
void cpu_common_init(void);

#endif /* MMU_FUNC_H_ */
