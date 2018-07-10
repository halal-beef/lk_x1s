/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or
 * distributed, transmitted, transcribed, stored in a retrieval system or
 * translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed to third parties
 * without the express written permission of Samsung Electronics.
 */

#include <debug.h>
#include <platform/secure_boot.h>
#include <platform/sfr.h>
#include <dev/rpmb.h>

#if defined(CONFIG_USE_AVB20)
uint32_t avb_main(char *suffix)
{
	uint32_t ret;
	struct AvbOps *ops;
	const char *partition_arr[] = {"boot", NULL};
	AvbSlotVerifyData* ctx_ptr;

	set_avbops();
	get_ops_addr(&ops);
	ret = avb_slot_verify(ops, partition_arr, suffix,
			AVB_SLOT_VERIFY_FLAGS_NONE,
			AVB_HASHTREE_ERROR_MODE_RESTART_AND_INVALIDATE,
			&ctx_ptr);
	if (ret) {
		printf("[AVB 2.0 ERR] authentication fail\n");
	} else {
		printf("[AVB 2.0] authentication success\n");
	}

	ret = block_RPMB_hmac();
	if (ret) {
		printf("[AVB 2.0 ERR] RPMB hmac ret: 0x%X\n", ret);
	}

	return ret;
}
#endif
