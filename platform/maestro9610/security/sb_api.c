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

#include <string.h>
#include <platform/smc.h>
#include <platform/secure_boot.h>

/******************************************************************************/
/* EL3 API */
/******************************************************************************/
uint32_t el3_sss_hash_init(
	uint32_t alg,
	struct ace_hash_ctx *ctx)
{
	HASH_INFO info_hash;

	info_hash.kindofhash = alg;
	memset(ctx, 0, sizeof(struct ace_hash_ctx));

	return exynos_smc(SMC_CMD_HASH, (uint64_t)&info_hash,
			(uint64_t)ctx, SHA_INIT);
}

uint32_t el3_sss_hash_update(
	uint32_t addr,
	uint32_t remain_size,
	uint32_t unit_size,
	struct ace_hash_ctx *ctx,
	uint32_t done_flag)
{
	HASH_INFO info_hash;

	info_hash.hash_addr = addr;
	info_hash.remain_size = remain_size;
	info_hash.unit_size = unit_size;

	if (done_flag)
		return exynos_smc(SMC_CMD_HASH, (uint64_t)&info_hash,
				(uint64_t)ctx, SHA_UPDATE_REMAIN_SIZE);
	else
		return exynos_smc(SMC_CMD_HASH, (uint64_t)&info_hash,
				(uint64_t)ctx, SHA_UPDATE_UNIT_SIZE);
}

uint32_t el3_sss_hash_final(
	struct ace_hash_ctx *ctx,
	uint8_t *hash)
{
	HASH_INFO info_hash;

	info_hash.result_hash = (uint32_t)(uint64_t)hash;

	return exynos_smc(SMC_CMD_HASH, (uint64_t)&info_hash,
			(uint64_t)ctx, SHA_FINAL);
}

uint32_t el3_sss_hash_digest(
	uint32_t addr,
	uint32_t size,
	uint32_t alg,
	uint8_t *hash)
{
	HASH_INFO info_hash;

	info_hash.hash_addr = addr;
	info_hash.result_hash = (uint32_t)(uint64_t)hash;
	info_hash.total_size = size;
	info_hash.kindofhash = alg;

	return exynos_smc(SMC_CMD_HASH, (uint64_t)&info_hash, 0, SHA_DIGEST);
}

uint32_t el3_verify_signature_using_image(
	uint64_t signed_img_ptr,
	uint64_t signed_img_len)
{
	CHECK_IMAGE_INFO check_info_image;

	check_info_image.context = 0x0;
	check_info_image.data = signed_img_ptr;
	check_info_image.dataLen = signed_img_len - SB_MAX_SIGN_LEN;
	check_info_image.signature = signed_img_ptr + signed_img_len -
		SB_MAX_SIGN_LEN;
	check_info_image.signatureLen = SB_MAX_SIGN_LEN;

	return exynos_smc(SMC_CMD_CHECK_SIGNATURE, 0,
			(uint64_t)&check_info_image, 0);
}
