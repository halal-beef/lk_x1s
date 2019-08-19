/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __BL_SYS_INFO_H__

struct bl1_info {
	unsigned int version[2];
	unsigned int epbl_start;
};

struct el3_mon_info {
	unsigned int machine_id;
	unsigned int version;
	unsigned int flags;
	unsigned int reserved;
};

struct epbl_info {
	unsigned int version;
	unsigned int checksum_range;
	unsigned int reserved[2];
};

struct mem_info {
	unsigned long base;
	unsigned long size;
};

struct bl_sys_info {
	unsigned int version;
	unsigned int length;
	unsigned char *rng_seed;
	unsigned int rng_seed_len;
	struct bl1_info bl1_info;
	struct el3_mon_info el3_mon_info;
	struct epbl_info epbl_info;
	struct mem_info dram_total;
	struct mem_info dram_sec;
	unsigned int magic;
};

#endif /*__BL_SYS_INFO_H__*/

