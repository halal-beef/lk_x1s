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
#include <string.h>

#if defined(CONFIG_USE_AVB20)
uint32_t avb_main(const char *suffix, char *cmdline)
{
	bool unlock;
	uint32_t ret = 0;
	uint32_t i = 0;
	struct AvbOps *ops;
	const char *partition_arr[] = {"boot", NULL};
	AvbSlotVerifyData* ctx_ptr;
	AvbSlotVerifyFlags asv_flag;

	set_avbops();
	get_ops_addr(&ops);
	ops->read_is_device_unlocked(ops, &unlock);
	if (unlock == 1) {
		asv_flag = AVB_SLOT_VERIFY_FLAGS_ALLOW_VERIFICATION_ERROR;
	} else {
		asv_flag = AVB_SLOT_VERIFY_FLAGS_NONE;
	}

	ret = avb_slot_verify(ops, partition_arr, suffix,
			asv_flag,
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

	i = 0;
	while (ctx_ptr->cmdline[i++] != '\0');
	memcpy(cmdline, ctx_ptr->cmdline, i);
#if defined(CONFIG_AVB_DEBUG)
	printf("i: %d\n", i);
	printf("cmdline: %s\n", cmdline);
#endif

	return ret;
}
#endif
