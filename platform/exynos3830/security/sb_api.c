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
#include <dev/rpmb.h>

/*
 *****************************************************************************
 * EL3 API*/

/******************************************************************************/
uint32_t read_secure_chip(void)
{
	return exynos_smc(SMC_CMD_READ_SB_PARAM, 0, 0, 0);
}

uint32_t el3_sss_hash_init(
	uint32_t alg,
	struct ace_hash_ctx *ctx)
{
	uint32_t ret;
	HASH_INFO info_hash __attribute__((__aligned__(CACHE_WRITEBACK_GRANULE_128)));

	info_hash.kindofhash = alg;
	memset(ctx, 0, sizeof(struct ace_hash_ctx));

	FLUSH_DCACHE_RANGE(ctx, sizeof(struct ace_hash_ctx));
	FLUSH_DCACHE_RANGE(&info_hash, sizeof(HASH_INFO));

	ret = exynos_smc(SMC_CMD_HASH, (uint64_t)&info_hash,
	                 (uint64_t)ctx, SHA_INIT);

	INV_DCACHE_RANGE(ctx, sizeof(struct ace_hash_ctx));

	return ret;
}

uint32_t el3_sss_hash_update(
	uint32_t addr,
	uint32_t remain_size,
	uint32_t unit_size,
	struct ace_hash_ctx *ctx,
	uint32_t done_flag)
{
	uint32_t ret;
	HASH_INFO info_hash __attribute__((__aligned__(CACHE_WRITEBACK_GRANULE_128)));

	info_hash.hash_addr = addr;
	info_hash.remain_size = remain_size;
	info_hash.unit_size = unit_size;

	FLUSH_DCACHE_RANGE(ctx, sizeof(struct ace_hash_ctx));
	FLUSH_DCACHE_RANGE(&info_hash, sizeof(HASH_INFO));

	if (done_flag) {
		FLUSH_DCACHE_RANGE(addr, remain_size);
		ret = exynos_smc(SMC_CMD_HASH, (uint64_t)&info_hash,
		                 (uint64_t)ctx, SHA_UPDATE_REMAIN_SIZE);
	} else {
		FLUSH_DCACHE_RANGE(addr, unit_size);
		ret = exynos_smc(SMC_CMD_HASH, (uint64_t)&info_hash,
		                 (uint64_t)ctx, SHA_UPDATE_UNIT_SIZE);
	}
	INV_DCACHE_RANGE(ctx, sizeof(struct ace_hash_ctx));

	return ret;
}

uint32_t el3_sss_hash_final(
	struct ace_hash_ctx *ctx,
	uint8_t *hash)
{
	uint32_t ret;
	HASH_INFO info_hash __attribute__((__aligned__(CACHE_WRITEBACK_GRANULE_128)));

	info_hash.result_hash = (uint32_t)(uint64_t)hash;

	FLUSH_DCACHE_RANGE(ctx, sizeof(struct ace_hash_ctx));
	FLUSH_DCACHE_RANGE(&info_hash, sizeof(HASH_INFO));
	FLUSH_DCACHE_RANGE(hash, SHA512_DIGEST_LEN);

	ret = exynos_smc(SMC_CMD_HASH, (uint64_t)&info_hash,
	                 (uint64_t)ctx, SHA_FINAL);

	INV_DCACHE_RANGE(hash, SHA512_DIGEST_LEN);

	return ret;
}

uint32_t el3_sss_hash_digest(
	uint32_t addr,
	uint32_t size,
	uint32_t alg,
	uint8_t *hash)
{
	uint32_t ret;
	HASH_INFO info_hash __attribute__((__aligned__(CACHE_WRITEBACK_GRANULE_128)));

	info_hash.hash_addr = addr;
	info_hash.result_hash = (uint32_t)(uint64_t)hash;
	info_hash.total_size = size;
	info_hash.kindofhash = alg;

	FLUSH_DCACHE_RANGE(&info_hash, sizeof(HASH_INFO));
	FLUSH_DCACHE_RANGE(addr, size);
	FLUSH_DCACHE_RANGE(hash, SHA512_DIGEST_LEN);

	ret = exynos_smc(SMC_CMD_HASH, (uint64_t)&info_hash, 0, SHA_DIGEST);

	INV_DCACHE_RANGE(hash, SHA512_DIGEST_LEN);

	return ret;
}

uint32_t el3_verify_signature_using_hash(
	uint64_t sign_field_ptr,
	uint8_t *hash)
{
	uint32_t ret;
	SB_V40_SMC_CTX ctx __attribute__((__aligned__(CACHE_WRITEBACK_GRANULE_128)));

	ctx.sign_field_ptr = sign_field_ptr;
	memcpy(ctx.hash, hash, sizeof(ctx.hash));

	FLUSH_DCACHE_RANGE(&ctx, sizeof(SB_V40_SMC_CTX));
	FLUSH_DCACHE_RANGE(sign_field_ptr, sizeof(SB_V40_SIGN_FIELD));
	FLUSH_DCACHE_RANGE(hash, SHA512_DIGEST_LEN);

	ret = exynos_smc(SMC_CMD_CHECK_SIGNATURE_WITH_HASH, 0, (uint64_t)&ctx, 0);

	return ret;
}

uint32_t el3_verify_signature_using_image(
	uint64_t signed_img_ptr,
	uint64_t signed_img_len,
	uint64_t sign_field_ptr,
	uint32_t ch)
{
	uint32_t ret;
	SB_V40_SMC_CTX ctx __attribute__((__aligned__(CACHE_WRITEBACK_GRANULE_128)));

	ctx.signed_img_ptr = signed_img_ptr;
	ctx.signed_img_len = signed_img_len;
	ctx.sign_field_ptr = sign_field_ptr;

	FLUSH_DCACHE_RANGE(&ctx, sizeof(SB_V40_SMC_CTX));
	FLUSH_DCACHE_RANGE(signed_img_ptr, signed_img_len - sizeof(SB_V40_SIGN_FIELD));
	FLUSH_DCACHE_RANGE(sign_field_ptr, sizeof(SB_V40_SIGN_FIELD));

	ret = exynos_smc(SMC_CMD_CHECK_SIGNATURE, ch, (uint64_t)&ctx, 0);

	return ret;
}

/*
 *****************************************************************************
 * LDFW API*/

/******************************************************************************/
uint32_t cm_verify_signature_using_image(
	uint64_t signed_img_ptr,
	uint64_t signed_img_len)
{
	uint32_t ret;
	SB_V40_SMC_CTX ctx __attribute__((__aligned__(CACHE_WRITEBACK_GRANULE_128)));

	ctx.signed_img_ptr = signed_img_ptr;
	ctx.signed_img_len = signed_img_len;

	FLUSH_DCACHE_RANGE(&ctx, sizeof(SB_V40_SMC_CTX));
	FLUSH_DCACHE_RANGE(signed_img_ptr, signed_img_len);

	ret = exynos_smc(SMC_AARCH64_PREFIX + SMC_CM_SECURE_BOOT,
	                 SB_CHECK_SIGN_NWD, (uint64_t)&ctx, 0);

	return ret;
}
