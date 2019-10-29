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
#include <platform/otp_v20.h>
#include <platform/ab_update.h>
#include <platform/cm_api.h>
#include <platform/bootimg.h>
#include <dev/rpmb.h>
#include <string.h>
#include <part.h>
#if defined(CONFIG_AVB_LCD_LOG)
#include <lib/font_display.h>
#endif

/* By convention, when a rollback index is not used the value remains zero. */
static const uint64_t kRollbackIndexNotUsed = 0;
static uint8_t avb_pubkey[SB_MAX_PUBKEY_LEN] __attribute__((__aligned__(CACHE_WRITEBACK_GRANULE_128)));

#if !defined(CONFIG_AVB_LCD_LOG)
void avb_print_lcd(const char *str, uint32_t boot_state) {};
#else
void avb_print_lcd(const char *str, uint32_t boot_state)
{
	uint32_t font_color;

	switch (boot_state) {
	case ORANGE:
		font_color = FONT_ORANGE;
		break;
	case YELLOW:
		font_color = FONT_YELLOW;
		break;
	case RED:
		font_color = FONT_RED;
		break;
	case GREEN:
		font_color = FONT_GREEN;
		break;
	default:
		print_lcd_update(FONT_WHITE, FONT_BLACK,
				"%s : color parsing fail\n", __func__);
		return;
	}
	print_lcd_update(font_color, FONT_BLACK, str);
}
#endif

uint32_t avb_set_root_of_trust(uint32_t device_state, uint32_t boot_state)
{
	uint32_t ret = 0;
	uint32_t boot_version = 0;
	uint32_t boot_patch_level = 0;
	uint32_t avb_pubkey_len = 0;
	uint8_t *vbmeta_buf = NULL;
	struct boot_img_hdr *b_hdr = (boot_img_hdr *)BOOT_BASE;
	struct AvbVBMetaImageHeader h;
	void *part = part_get_ab("vbmeta");
	uint8_t hash[SHA512_DIGEST_LEN];
	uint32_t hash_len = 0;

	if (!part) {
		printf("Partition 'vbmeta' does not exist\n");
		return -1;
	} else {
		vbmeta_buf = avb_malloc(part_get_size_in_bytes(part));
		if (vbmeta_buf == NULL) {
			printf("vbmeta allocation fail\n");
			return -1;
		}
		part_read(part, (void *)vbmeta_buf);
	}
	avb_vbmeta_image_header_to_host_byte_order((const AvbVBMetaImageHeader*)vbmeta_buf, &h);
	avb_pubkey_len = h.public_key_size;
	if (avb_pubkey_len == 0) {
		ret = AVB_ERROR_AVBKEY_LEN_ZERO;
		goto out;
	}
	memcpy(avb_pubkey, (void *)((uint64_t)vbmeta_buf +
				sizeof(AvbVBMetaImageHeader) +
				h.authentication_data_block_size +
				h.public_key_offset),
			avb_pubkey_len);
	ret = cm_secure_boot_set_pubkey(avb_pubkey, avb_pubkey_len);
	if (ret)
		goto out;
	boot_version = (b_hdr->os_version & 0xFFFFF800) >> 11;
	boot_patch_level = (b_hdr->os_version & 0x7FF) >> 0;
	ret = cm_secure_boot_set_os_version(boot_version, boot_patch_level);
	if (ret)
		goto out;
	ret = cm_secure_boot_set_vendor_boot_version(0, 0);
	if (ret)
		goto out;
	ret = cm_secure_boot_set_device_state(device_state);
	if (ret)
		goto out;
	ret = cm_secure_boot_set_boot_state(boot_state);
	if (ret)
		goto out;
	hash_len = SHA256_DIGEST_LEN;
	ret = el3_sss_hash_digest((uint32_t)(uint64_t)vbmeta_buf, 4 * 1024, ALG_SHA256, hash);
	if (ret) {
		printf("[AVB] hash digest fail [0x%X]\n", ret);
		goto out;
	}
	ret = cm_secure_boot_set_verified_boot_hash(hash, hash_len);
	if (ret)
		goto out;

out:
	avb_free(vbmeta_buf);
	cm_secure_boot_block_cmd();

	return ret;
}

uint32_t is_slot_marked_successful(void)
{
	uint32_t ret;

	/* No AB Support: return true case */
	if (!ab_update_support())
		return 1;

	ret = ab_slot_successful(ab_current_slot());

	return ret;
}

uint32_t update_rp_count_otp(const char *suffix)
{
	uint32_t ret = 0;
	uint32_t *rollback_index;
	char part_name[15] = "epbl";
	void *part;

	part = part_get("fwbl1");
	part_read(part, (void *)AVB_PRELOAD_BASE);

	rollback_index = (uint32_t *)(AVB_PRELOAD_BASE + part_get_size_in_bytes(part) -
			SB_SB_CONTEXT_LEN + SB_BL1_RP_COUNT_OFFSET);
	printf("[SB] BL1 RP addr: %p\n", rollback_index);
	printf("[SB] BL1 RP count: %d\n", *rollback_index);
	ret = cm_otp_update_antirbk_sec_ap(*rollback_index);
	if (ret)
		goto out;

	strcat(part_name, suffix);
	part = part_get(part_name);
	part_read(part, (void *)AVB_PRELOAD_BASE);

	rollback_index = (uint32_t *)(AVB_PRELOAD_BASE + part_get_size_in_bytes(part) -
			SB_MAX_RSA_SIGN_LEN - SB_SIGN_FIELD_HEADER_SIZE);
	printf("[SB] EPBL RP addr: %p\n", rollback_index);
	printf("[SB] EPBL RP count: %d\n", *rollback_index);
	ret = cm_otp_update_antirbk_non_sec_ap0(*rollback_index);
	if (ret)
		goto out;

out:
	return ret;
}

uint32_t update_rp_count_avb(AvbOps *ops, AvbSlotVerifyData *ctx_ptr)
{
	uint32_t ret = 0;
	uint32_t i = 0;
	uint64_t stored_rollback_index = 0;

	printf("[AVB] Check RP count for update\n");
	for (i = 0; i < AVB_MAX_NUMBER_OF_ROLLBACK_INDEX_LOCATIONS; i++) {
		if (ctx_ptr->rollback_indexes[i] != kRollbackIndexNotUsed) {
			printf("[AVB 2.0] Rollback index location: %d\n", i);
			printf("[AVB 2.0] Current Image RP count: %lld\n",
					ctx_ptr->rollback_indexes[i]);
			ret = ops->read_rollback_index(ops, i,
					&stored_rollback_index);
			if (ret) {
				printf("[AVB 2.0 ERR] Read RP count fail, ret: 0x%X\n",
						ret);
				goto out;
			}
			printf("[AVB 2.0] Current RPMB RP count: %lld\n",
					stored_rollback_index);

			if (ctx_ptr->rollback_indexes[i] > stored_rollback_index) {
				printf("[AVB 2.0] Update RP count start\n");
				ret = ops->write_rollback_index(ops, i,
						ctx_ptr->rollback_indexes[i]);
				if (ret) {
					printf("[AVB 2.0 ERR] Write RP count fail, ret: 0x%X\n",
							ret);
					goto out;
				}
				ret = ops->read_rollback_index(ops, i,
						&stored_rollback_index);
				if (ret) {
					printf("[AVB 2.0 ERR] Read RP count fail, ret: 0x%X\n",
							ret);
					goto out;
				}
				printf("[AVB 2.0] Updated Image RP count: %lld\n",
						ctx_ptr->rollback_indexes[i]);
				printf("[AVB 2.0] Updated RPMB RP count: %lld\n",
						stored_rollback_index);
				if (ctx_ptr->rollback_indexes[i] != stored_rollback_index)
					ret = AVB_ERROR_RP_UPDATE_FAIL;
			}
		}
	}

out:
	return ret;
}

#if defined(CONFIG_USE_AVB20)
uint32_t avb_main(const char *suffix, char *cmdline, char *verifiedbootstate, uint32_t recovery_mode)
{
	bool unlock;
	uint32_t ret = 0;
	uint32_t boot_state;
	struct AvbOps *ops;
	const char *partition_boot[3] = {"boot", "dtbo", NULL};
	const char *partition_recovery[2] = {"recovery", NULL};
	char buf[100];
	char color[AVB_COLOR_MAX_SIZE];
	AvbSlotVerifyData *ctx_ptr = NULL;

	set_avbops();
	get_ops_addr(&ops);
	ops->read_is_device_unlocked(ops, &unlock);

	/* slot verify */
	if ((recovery_mode == 1) && (!ab_update_support()))
		ret = avb_slot_verify(ops, partition_recovery, suffix,
				AVB_SLOT_VERIFY_FLAGS_ALLOW_VERIFICATION_ERROR,
				AVB_HASHTREE_ERROR_MODE_RESTART_AND_INVALIDATE,
				&ctx_ptr);
	else
		ret = avb_slot_verify(ops, partition_boot, suffix,
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
		if (unlock && ret == AVB_SLOT_VERIFY_RESULT_ERROR_VERIFICATION)
			snprintf(buf, 100, "[AVB 2.0 warning] authentication fail [ret: 0x%X] (%s) "
					"No effect on booting process\n", ret, color);
		else if (unlock && ret == AVB_SLOT_VERIFY_RESULT_ERROR_PUBLIC_KEY_REJECTED)
			snprintf(buf, 100, "[AVB 2.0 warning] authentication fail [ret: 0x%X] (%s) "
					"Invalid key is used\n", ret, color);
		else
			snprintf(buf, 100, "[AVB 2.0 ERR] authentication fail [ret: 0x%X] (%s)\n", ret, color);
	} else {
		snprintf(buf, 100, "[AVB 2.0] authentication success (%s)\n", color);
	}

	switch (color[0]) {
	case 'o':
		boot_state = ORANGE;
		break;
	case 'y':
		boot_state = YELLOW;
		break;
	case 'r':
		boot_state = RED;
		break;
	case 'g':
		boot_state = GREEN;
		break;
	default:
		return AVB_ERROR_INVALID_COLOR;
	}
	/* Print log */
	strcat(verifiedbootstate, color);
	printf(buf);
	avb_print_lcd(buf, boot_state);

#if defined(CONFIG_AVB_CMDLINE)
	/* set cmdline */
	uint32_t i = 0;
	if (ctx_ptr != NULL) {
		i = 0;
		while (ctx_ptr->cmdline[i++] != '\0');
		memcpy(cmdline, ctx_ptr->cmdline, i);
	}
#if defined(CONFIG_AVB_DEBUG)
	printf("i: %d\n", i);
	printf("cmdline: %s\n", cmdline);
#endif
#else
	strncpy(verifiedbootstate, "", AVB_VBS_MAX_SIZE);
	printf("[AVB] Command line is not set\n");
#endif

#if defined(CONFIG_AVB_ROT)
	uint32_t rot_ret = 0;

	/* Set root of trust */
	rot_ret = avb_set_root_of_trust(!unlock, boot_state);
	if (rot_ret)
		printf("[AVB] Root of trust error ret: 0x%X\n", rot_ret);
#else
	printf("[AVB] Root of trust is not set\n");
#endif

	/* Update RP count */
	if (!ret && is_slot_marked_successful()) {
#if defined(CONFIG_AVB_RP_UPDATE)
		ret = update_rp_count_avb(ops, ctx_ptr);
		if (ret)
			return ret;
#else
		printf("[AVB] RP count update is disabled\n");
#endif
#if defined(CONFIG_OTP_RP_UPDATE)
		ret = update_rp_count_otp(suffix);
		if (ret)
			return ret;
#else
		printf("[OTP] RP count update is disabled\n");
#endif
	}

#if defined(CONFIG_USE_RPMB)
	uint32_t rpmb_ret = 0;

	/* block RPMB */
	rpmb_ret = block_RPMB_hmac();
	if (rpmb_ret) {
		printf("[AVB 2.0 ERR] RPMB hmac ret: 0x%X\n", rpmb_ret);
	}
#endif

	return ret;
}
#endif
