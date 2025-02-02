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

#include <lk/debug.h>
#include <platform/secure_boot.h>
#include <platform/sfr.h>
#include <dev/rpmb.h>
#include <string.h>

void avb_print_lcd(const char *str);

#if defined(CONFIG_USE_AVB20)
uint32_t avb_main(const char *suffix, char *cmdline, char *verifiedbootstate)
{
	bool unlock;
	uint32_t ret = 0;
	uint32_t i = 0;
	struct AvbOps *ops;
	const char *partition_arr[] = {"boot", "dtbo", NULL};
	char buf[100];
	char color[AVB_COLOR_MAX_SIZE];
	AvbSlotVerifyData *ctx_ptr = NULL;

	set_avbops();
	get_ops_addr(&ops);
	ops->read_is_device_unlocked(ops, &unlock);

	/* slot verify */
	ret = avb_slot_verify(ops, partition_arr, suffix,
			AVB_SLOT_VERIFY_FLAGS_ALLOW_VERIFICATION_ERROR,
			AVB_HASHTREE_ERROR_MODE_RESTART_AND_INVALIDATE,
			&ctx_ptr);
	/* get color */
	if (unlock) {
		strncpy(color, "orange", AVB_COLOR_MAX_SIZE);
	} else {
		if (ret == AVB_SLOT_VERIFY_RESULT_ERROR_PUBLIC_KEY_REJECTED) {
			strncpy(color, "yellow", AVB_COLOR_MAX_SIZE);
		} else if (ret) {
			strncpy(color, "red", AVB_COLOR_MAX_SIZE);
		} else {
			strncpy(color, "green", AVB_COLOR_MAX_SIZE);
		}
	}
	if (ret) {
		snprintf(buf, 100, "[AVB 2.0 ERR] authentication fail [ret: 0x%X] (%s)\n", ret, color);
	} else {
		snprintf(buf, 100, "[AVB 2.0] authentication success (%s)\n", color);
	}
	strcat(verifiedbootstate, color);
	printf(buf);
	avb_print_lcd(buf);

	/* AVB color policy */
	if (!strncmp(color, "red", AVB_COLOR_MAX_SIZE))
		__asm__ volatile("b	.");

	/* block RPMB */
	ret = block_RPMB_hmac();
	if (ret) {
		printf("[AVB 2.0 ERR] RPMB hmac ret: 0x%X\n", ret);
	}

	/* set cmdline */
	if (ctx_ptr != NULL) {
		i = 0;
		while (ctx_ptr->cmdline[i++] != '\0');
		memcpy(cmdline, ctx_ptr->cmdline, i);
	}
#if defined(CONFIG_AVB_DEBUG)
	printf("i: %d\n", i);
	printf("cmdline: %s\n", cmdline);
#endif

	return ret;
}
#endif
