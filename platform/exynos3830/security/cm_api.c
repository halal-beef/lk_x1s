/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <debug.h>
#include <stdlib.h>
#include <string.h>
#include <platform/cm_api.h>
#include <platform/secure_boot.h>
#include <platform/mmu/mmu_func.h>

static inline uint64_t exynos_cm_smc(uint64_t *cmd, uint64_t *arg1,
				     uint64_t *arg2, uint64_t *arg3)
{
	register uint64_t reg0 __asm__("x0") = *cmd;
	register uint64_t reg1 __asm__("x1") = *arg1;
	register uint64_t reg2 __asm__("x2") = *arg2;
	register uint64_t reg3 __asm__("x3") = *arg3;

	__asm__ volatile (
		"dsb	sy\n"
		"smc	0\n"
		: "+r"(reg0), "+r"(reg1), "+r"(reg2), "+r"(reg3)
	);

	*cmd = reg0;
	*arg1 = reg1;
	*arg2 = reg2;
	*arg3 = reg3;

	if (reg0 == RV_SB_LDFW_NOT_LOADED)
		printf("[CM] LDFW hasn't been loaded\n");

	return reg0;
}

uint64_t cm_secure_boot_set_pubkey(
	uint8_t *pub_key_ptr, uint32_t pub_key_len)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_SECURE_BOOT;
	r1 = SB_SET_PUBLIC_KEY;
	r2 = VIRT_TO_PHYS(pub_key_ptr);
	r3 = pub_key_len;

	printf("[CM] set publick_key: start\n");

	FLUSH_DCACHE_RANGE(pub_key_ptr, pub_key_len);

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[CM] set publick_key: fail: "
		"r0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	printf("[CM] set public_key: success\n");

	return ret;
}

uint64_t cm_secure_boot_erase_pubkey(void)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_SECURE_BOOT;
	r1 = SB_ERASE_PUBLIC_KEY;
	r2 = 0;
	r3 = 0;

	printf("[CM] erase public_key: start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[CM] erase public_key: fail: "
		"r0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	printf("[CM] erase public_key: success\n");

	return ret;
}

uint64_t cm_secure_boot_set_os_version(
	uint32_t os_version, uint32_t os_patch_level)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_SECURE_BOOT;
	r1 = SB_SET_OS_VERSION;
	r2 = os_version;
	r3 = os_patch_level;

	printf("[CM] set os_version & os_patch_level: start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[CM] set os_version & os_patch_level: fail: "
		"r0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	printf("[CM] set os_version & os_patch_level: success\n");

	return ret;
}

uint64_t cm_secure_boot_set_vendor_boot_version(
	uint32_t vendor_patch_level, uint32_t boot_patch_level)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_SECURE_BOOT;
	r1 = SB_SET_VENDOR_BOOT_VERSION;
	r2 = vendor_patch_level;
	r3 = boot_patch_level;

	printf("[CM] set vendor & boot patch level: start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[CM] set vendor & boot patch level: fail: "
		"r0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	printf("[CM] set vendor & boot patch level: success\n");

	return ret;
}

uint64_t cm_secure_boot_set_device_state(
	uint32_t state)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_SECURE_BOOT;
	r1 = SB_SET_DEVICE_STATE;
	r2 = state;
	r3 = 0;

	printf("[CM] set device_state: start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[CM] set device_state: fail: "
		"r0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	printf("[CM] set device_state: success\n");

	return ret;
}

uint64_t cm_secure_boot_set_boot_state(
	uint32_t state)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_SECURE_BOOT;
	r1 = SB_SET_BOOT_STATE;
	r2 = state;
	r3 = 0;

	printf("[CM] set boot_state: start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[CM] set boot_state: fail: "
		"r0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	printf("[CM] set boot_state: success\n");

	return ret;
}

uint64_t cm_secure_boot_block_cmd(void)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_SECURE_BOOT;
	r1 = SB_BLOCK_CMD;
	r2 = 0;
	r3 = 0;

	printf("[CM] block verified boot commands: start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[CM] block verified boot commands: fail: "
		"r0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	printf("[CM] block verified boot commands: success\n");

	return ret;
}

uint64_t cm_get_random(
	uint8_t *random, uint32_t random_len)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;
	uint64_t byte_buf[2];
	uint32_t byte16_cnt;
	uint32_t byte_cnt;
	uint32_t i;

	printf("[CM] get random number: start\n");

	byte16_cnt = random_len / 16;
	byte_cnt = random_len & 0xF;

	for (i = 0; i < byte16_cnt; i++) {
		r0 = SMC_AARCH64_PREFIX | SMC_CM_RANDOM;
		r1 = GET_RANDOM_WORD;
		r2 = 0;
		r3 = 0;

		ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
		if (ret != RV_SUCCESS) {
			printf("[CM] get random number: fail: "
			"r0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
			return ret;
		}
		memcpy(&random[i * 16], (uint8_t *)&r1, 8);
		memcpy(&random[(i * 16) + 8], (uint8_t *)&r2, 8);
	}

	if (byte_cnt) {
		r0 = SMC_AARCH64_PREFIX | SMC_CM_RANDOM;
		r1 = GET_RANDOM_WORD;
		r2 = 0;
		r3 = 0;

		ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
		if (ret != RV_SUCCESS) {
		    printf("[CM] get random number: fail: "
			    "r0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		    return ret;
		}
		byte_buf[0] = r1;
		byte_buf[1] = r2;
		memcpy(&random[i * 16], (uint8_t *)byte_buf, byte_cnt);
	}

	printf("[CM] get random number: success\n");

	return ret;
}

uint64_t cm_km_wrap_key_with_kek(
	CM_KW_CTX_T *ctx)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	printf("[CM] wrap/unwrap key: start\n");

	r0 = SMC_AARCH64_PREFIX | SMC_CM_KEY_MANAGER;
	r1 = CMD_KEY_WRAP_WITH_KEK;
	r2 = VIRT_TO_PHYS(ctx);
	r3 = 0;

	FLUSH_DCACHE_RANGE(ctx, sizeof(CM_KW_CTX_T));

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[CM] wrap/unwrap key: fail: "
		"r0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	INV_DCACHE_RANGE(ctx, sizeof(CM_KW_CTX_T));

	printf("[CM] wrap/unwrap key: success\n");

	return ret;
}

/*****************************************************************************/
/* Test functions for Secure Boot & RootOfTrust                              */
/*****************************************************************************/
static uint8_t rsa_sb_pubkey[] = {
	0x00, 0x01, 0x00, 0x00, 0x17, 0x31, 0xba, 0x3e, 0x65, 0x9e,
	0x8a, 0xfa, 0xa9, 0x47, 0x2e, 0x26, 0x5f, 0x4a, 0x50, 0x4b,
	0x5d, 0xd8, 0x03, 0x4e, 0x00, 0x61, 0x72, 0xc8, 0x7a, 0xfb,
	0x7d, 0x29, 0x33, 0x29, 0x41, 0x13, 0x39, 0x64, 0x82, 0xda,
	0xb8, 0xcb, 0x06, 0x27, 0x1c, 0x50, 0x54, 0x3f, 0x30, 0x44,
	0x21, 0x05, 0x6b, 0x07, 0x29, 0xfd, 0x8e, 0x7f, 0xdd, 0xec,
	0x86, 0xca, 0x83, 0x0e, 0x1d, 0xa4, 0x6f, 0x99, 0x4e, 0x7c,
	0xc0, 0x5b, 0x2f, 0x72, 0xe6, 0xc6, 0xb4, 0xba, 0x78, 0x01,
	0x53, 0x4c, 0xbc, 0x47, 0xff, 0x37, 0x61, 0xd4, 0x88, 0x4c,
	0x1b, 0xbf, 0x31, 0x6c, 0xa7, 0xa8, 0xfb, 0xab, 0x24, 0x6a,
	0xde, 0x1e, 0x36, 0x89, 0x67, 0xec, 0x15, 0x55, 0x08, 0xfc,
	0xd5, 0xea, 0xa3, 0x36, 0x50, 0x1b, 0x28, 0xaf, 0x69, 0xf7,
	0x65, 0xa6, 0x0b, 0x10, 0x30, 0x25, 0x5a, 0x48, 0x72, 0x59,
	0xe7, 0x9d, 0xd6, 0x4b, 0x96, 0xb3, 0x28, 0x50, 0xb6, 0xce,
	0xe0, 0xca, 0x8e, 0xc6, 0x87, 0x9c, 0xa6, 0x83, 0xc7, 0x99,
	0x23, 0xf0, 0x6f, 0x4f, 0x45, 0x85, 0x35, 0xef, 0x49, 0x89,
	0x82, 0xce, 0x34, 0x8d, 0x72, 0xd2, 0x94, 0x03, 0xe4, 0x39,
	0x19, 0x63, 0x5e, 0x6b, 0xf2, 0xde, 0x98, 0x44, 0x14, 0xa4,
	0xc9, 0x6a, 0xb0, 0xac, 0xe8, 0xb1, 0x63, 0xf1, 0xbd, 0xd1,
	0xf4, 0x99, 0x0c, 0xea, 0xe4, 0x48, 0x66, 0xaa, 0x2b, 0x53,
	0x27, 0xc7, 0xdd, 0x67, 0x31, 0x81, 0x99, 0x5b, 0xf2, 0x16,
	0xd4, 0x7e, 0xa7, 0x74, 0xc2, 0xdc, 0x7b, 0xbe, 0x3a, 0x30,
	0x11, 0x37, 0x0e, 0x49, 0x77, 0xff, 0x10, 0x55, 0x1f, 0x62,
	0xbd, 0x15, 0xfc, 0x37, 0x1e, 0x85, 0x9f, 0xa0, 0x52, 0x6b,
	0x43, 0x9c, 0x24, 0x7d, 0x01, 0x93, 0x10, 0x62, 0xa3, 0x29,
	0x8d, 0x86, 0x9f, 0x6c, 0x11, 0x72, 0x9e, 0x74, 0x50, 0x86,
	0x04, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00
};

uint64_t cm_secure_boot_self_test(void)
{
	uint32_t ret = RV_SUCCESS;
	uint32_t sb_os_version = 0x00101010;
	uint32_t sb_patch_version = 0x00201611;
	uint32_t sb_device_state = LOCKED;
	uint32_t sb_boot_state = GREEN;

	SB_PARAM_ROOT_OF_TRUST_ST root_of_trust;
	memset((uint8_t *)&root_of_trust, 0, sizeof(SB_PARAM_ROOT_OF_TRUST_ST));

	printf("[CM] TEST 01/11: call cm_secure_boot_set_os_version()\n");
	ret = cm_secure_boot_set_os_version(sb_os_version, sb_patch_version);
	if (ret != RV_SUCCESS) {
		printf("[CM] TEST_FAIL: cm_secure_boot_set_os_version() "
			"is failed: 0x%x\n", ret);
	}

	printf("[CM] TEST 02/11: call cm_secure_boot_set_device_state()\n");
	ret = cm_secure_boot_set_device_state(sb_device_state);
	if (ret != RV_SUCCESS) {
		printf("[CM] TEST_FAIL: cm_secure_boot_set_device_state() "
			"is failed: 0x%x\n", ret);
	}

	printf("[CM] TEST 03/11: call cm_secure_boot_set_boot_state()\n");
	ret = cm_secure_boot_set_boot_state(sb_boot_state);
	if (ret != RV_SUCCESS) {
		printf("[CM] TEST_FAIL: cm_secure_boot_set_boot_state() "
			"is failed: 0x%x\n", ret);
	}

	printf("[CM] TEST 04/11: call cm_secure_boot_set_pubkey()\n");
	ret = cm_secure_boot_set_pubkey(rsa_sb_pubkey, sizeof(rsa_sb_pubkey));
	if (ret != RV_SUCCESS) {
		printf("[CM] TEST_FAIL: cm_secure_boot_set_pubkey() "
			"is failed: 0x%x\n", ret);
	}

	printf("[CM] TEST 05/11: call cm_secure_boot_erase_pubkey()\n");
	ret = cm_secure_boot_erase_pubkey();
	if (ret != RV_SUCCESS) {
		printf("[CM] TEST_FAIL: cm_secure_boot_erase_pubkey() "
			"is failed: 0x%x\n", ret);
	}

	/* test block command */
	printf("[CM] TEST 06/11: call cm_secure_boot_block_cmd()\n");
	ret = cm_secure_boot_block_cmd();
	if (ret != RV_SUCCESS) {
		printf("[CM] TEST FAIL: cm_secure_boot_block_cmd() "
			"is failed: 0x%x\n", ret);
	}

	printf("[CM] TEST 07/11: call cm_secure_boot_set_os_version()\n");
	ret = cm_secure_boot_set_os_version(sb_os_version, sb_patch_version);
	if (ret != RV_SB_CMD_BLOCKED) {
		printf("[CM] TEST_FAIL: cm_secure_boot_set_os_version() "
			"is not blocked: 0x%x\n", ret);
	} else {
		printf("[CM] TEST_PASS: cm_secure_boot_set_os_version() "
			"is blocked\n");
	}

	printf("[CM] TEST 08/11: call cm_secure_boot_set_device_state()\n");
	ret = cm_secure_boot_set_device_state(sb_device_state);
	if (ret != RV_SB_CMD_BLOCKED) {
		printf("[CM] TEST_FAIL: cm_secure_boot_set_device_state() "
			"is not blocked: 0x%x\n", ret);
	} else {
		printf("[CM] TEST_PASS: cm_secure_boot_set_device_state() "
			"is blocked\n");
	}

	printf("[CM] TEST 09/11: call cm_secure_boot_set_boot_state()\n");
	ret = cm_secure_boot_set_boot_state(sb_boot_state);
	if (ret != RV_SB_CMD_BLOCKED) {
		printf("[CM] TEST_FAIL: cm_secure_boot_set_boot_state() "
			"is not blocked: 0x%x\n", ret);
	} else {
		printf("[CM] TEST_PASS: cm_secure_boot_set_boot_state() "
			"is blocked\n");
	}

	printf("[CM] TEST 10/11: call cm_secure_boot_set_pubkey()\n");
	ret = cm_secure_boot_set_pubkey(rsa_sb_pubkey, sizeof(rsa_sb_pubkey));
	if (ret != RV_SB_CMD_BLOCKED) {
		printf("[CM] TEST_FAIL: cm_secure_boot_set_pubkey() "
			"is not blocked: 0x%x\n", ret);
	} else {
		printf("[CM] TEST_PASS: cm_secure_boot_set_pubkey() "
			"is blocked\n");
	}

	printf("[CM] TEST 11/11: call cm_secure_boot_erase_pubkey()\n");
	ret = cm_secure_boot_erase_pubkey();
	if (ret != RV_SB_CMD_BLOCKED) {
		printf("[CM] TEST FAIL: cm_secure_boot_erase_pubkey() "
			"is not blocked: 0x%x\n", ret);
	} else {
		printf("[CM] TEST_PASS: cm_secure_boot_erase_pubkey() "
			"is blocked\n");
	}

	return ret;
}

/*****************************************************************************/
/* Test functions for Random                                                 */
/*****************************************************************************/
#define DRBG_TEST_CNT				8

/* Health Test Cutoff for DRBG
 * (assume the min-entropy H is 0.5, false positive probability W is 2^-30) */
#define REPETITION_TEST_CUTOFF			9	/* ceil(1 + (-logW)/H) */
#define ADAPTIVE_TEST_WSIZE_64_CUTOFF		20	/* CRITBINOM(64, 2^(-H), 1-W) */
#define ADAPTIVE_TEST_WSIZE_4096_CUTOFF		354	/* CRITBINOM(4096, 2^(-H), 1-W) */

/* Health Test Context */
struct continuous_test {
	uint32_t	lastvalue;
};

struct repetition_test {
	uint8_t		lastvalue;
	uint32_t	cutoff;
	uint32_t	rnd_ctr;
};

struct adaptive_test {
	uint32_t	wsize;
	uint32_t	cutoff;
	uint32_t	curr_size;
	uint8_t		curr_rnd;
	uint32_t	rnd_ctr;
};

struct test_ctx {
	struct continuous_test	ctest;
	struct repetition_test	rtest;
	struct adaptive_test    atest[2];
};

uint32_t out_len[DRBG_TEST_CNT] = {16, 32, 50, 64, 65, 100, 127, 128};

static void health_test_init(struct test_ctx *tctx)
{
	tctx->rtest.cutoff = REPETITION_TEST_CUTOFF;
	tctx->rtest.rnd_ctr = 0;
	tctx->atest[0].wsize = 64;
	tctx->atest[0].cutoff = ADAPTIVE_TEST_WSIZE_64_CUTOFF;
	tctx->atest[0].curr_size = 64;
	tctx->atest[0].rnd_ctr = 0;
	tctx->atest[1].wsize = 4096;
	tctx->atest[1].cutoff = ADAPTIVE_TEST_WSIZE_4096_CUTOFF;
	tctx->atest[1].curr_size = 4096;
	tctx->atest[1].rnd_ctr = 0;
}

static uint32_t health_test(uint8_t *buf, uint32_t len)
{
	uint32_t *temp;
	uint32_t i, j;
	struct test_ctx tctx;

	health_test_init(&tctx);

	/* Continuous Test
	 * (equal to Repetition Count Test if min-entropy is more than 0.9375) */
	temp = (uint32_t *)buf;

	tctx.ctest.lastvalue = temp[0];
	for (i = 1; i < len/4; i--) {
		if (tctx.ctest.lastvalue == temp[i])
			goto fail;
		tctx.ctest.lastvalue = temp[i];
	}

	/* Repetition Test should be performed if min-entropy is less than 0.9375 */
	if (tctx.rtest.rnd_ctr == 0) {
		tctx.rtest.lastvalue = buf[0];
		tctx.rtest.rnd_ctr = 1;
	}

	for (i = 1; i < len; i++) {
		if (tctx.rtest.lastvalue == buf[i]) {
			if (tctx.rtest.cutoff == ++tctx.rtest.rnd_ctr)
				goto fail;
		} else {
			tctx.rtest.lastvalue = buf[i];
			tctx.rtest.rnd_ctr = 1;
		}
	}

	/* Adaptive Proportion Test (W = 64, 4096) */
	for (i = 0; i < 2; i++) {
		for (j = 0; j < len; j++) {
			if (tctx.atest[i].curr_size == tctx.atest[i].wsize) {
				tctx.atest[i].curr_rnd = buf[j];
				tctx.atest[i].rnd_ctr = 0;
				tctx.atest[i].curr_size = 0;
			} else {
				tctx.atest[i].curr_size++;
				if (tctx.atest[i].curr_rnd == buf[j]) {
					if (++tctx.atest[i].rnd_ctr > tctx.atest[i].cutoff)
						goto fail;
				}
			}
		}
	}

	return 0;

fail:
	/* Initialize tests contexts for considering it'll try again */
	tctx.rtest.rnd_ctr = 0;
	tctx.atest[0].curr_size = 64;
	tctx.atest[0].rnd_ctr = 0;
	tctx.atest[1].curr_size = 4096;
	tctx.atest[1].rnd_ctr = 0;

	return (uint32_t)-1;
}

uint64_t cm_random_self_test(void)
{
	uint32_t ret = RV_SUCCESS;
	uint32_t drbg_total_cnt, drbg_fail_cnt;
	uint32_t i;
	uint8_t out[256];

	drbg_total_cnt = 0;
	drbg_fail_cnt = 0;

	do {
		ret = cm_get_random(out, out_len[drbg_total_cnt]);
		if (ret != RV_SUCCESS) {
			printf("[CM] TEST FAIL - DRBG %d bit generation with ret: %x\n",
				out_len[drbg_total_cnt], ret);
			drbg_fail_cnt++;
			continue;
		} else {
			ret = health_test(out, out_len[drbg_total_cnt]);
			if (ret != RV_SUCCESS) {
				printf("[CM] TEST FAIL - DRBG %d "
				       "bit generation to perform health test\n",
					out_len[drbg_total_cnt]);

				printf("Random Bits: ");
				for (i = 0; i < out_len[drbg_total_cnt]; i++)
					printf("0x%x, ", out[i]);
				printf("\n");

				drbg_fail_cnt++;
				continue;
			} else {
				printf("[CM] TEST PASS - DRBG %d bit generation\n",
					out_len[drbg_total_cnt]);
			}
		}
	} while (++drbg_total_cnt < DRBG_TEST_CNT);

	if (drbg_fail_cnt)
		printf("[CM] TEST FAIL: %d/%d\n", drbg_fail_cnt, drbg_total_cnt);

	return RV_SUCCESS;
}

/*****************************************************************************/
/* Test functions for Keymanager                                             */
/*****************************************************************************/
#define KM_KW_MAX_TESTS		4

static uint32_t kw_salt_len[KM_KW_MAX_TESTS] = { 32, 40, 20, 32 };
static uint8_t kw_salt[KM_KW_MAX_TESTS][64] = {
	{
	0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA,
	0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA,
	0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA,
	0x11, 0x22
	},
	{
	0xFF, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA,
	0xFF, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA,
	0xFF, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA,
	0xFF, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA,
	},
	{
	0xFF, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA,
	0xFF, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA,
	},
	{
	0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA,
	0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA,
	0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA,
	0x11, 0x22
	},
};

static uint32_t iv_len[KM_KW_MAX_TESTS] = { 12, 12, 12 , 12 };
static uint8_t iv[KM_KW_MAX_TESTS][12] = {
	{
	0x9f, 0xf1, 0x85, 0x63, 0xb9, 0x78, 0xec, 0x28, 0x1b, 0x3f,
	0x27, 0x94
	},
	{
	0xc0, 0x94, 0x66, 0x23, 0x6f, 0xc4, 0xb2, 0x06, 0x7a, 0xde,
	0xcd, 0xec
	},
	{
	0x0d, 0x18, 0xe0, 0x6c, 0x7c, 0x72, 0x5a, 0xc9, 0xe3, 0x62,
	0xe1, 0xce
	},
	{
	0x3c, 0x81, 0x9d, 0x9a, 0x9b, 0xed, 0x08, 0x76, 0x15, 0x03,
	0x0b, 0x65
	},
};

static uint32_t aad_len[KM_KW_MAX_TESTS] = { 20, 16, 0 , 0 };
static uint8_t aad[KM_KW_MAX_TESTS][64] = {
	{
	0xad, 0xb5, 0xec, 0x72, 0x0c, 0xcf, 0x98, 0x98, 0x50, 0x00,
	0x28, 0xbf, 0x34, 0xaf, 0xcc, 0xbc, 0xac, 0xa1, 0x26, 0xef
	},
	{
	0x02, 0xf1, 0xd1, 0x8b, 0x34, 0x37, 0x15, 0x0d, 0xf9, 0x25,
	0xa9, 0x2e, 0xa5, 0x93, 0x79, 0xfe
	},
	{
	0x0,
	},
	{
	0x0,
	},
};

static uint32_t kek_len[KM_KW_MAX_TESTS] = { 32, 32, 32, 16 };
static uint8_t kek[KM_KW_MAX_TESTS][32] = {
	{
	0x24, 0x50, 0x1a, 0xd3, 0x84, 0xe4, 0x73, 0x96, 0x3d, 0x47,
	0x6e, 0xdc, 0xfe, 0x08, 0x20, 0x52, 0x37, 0xac, 0xfd, 0x49,
	0xb5, 0xb8, 0xf3, 0x38, 0x57, 0xf8, 0x11, 0x4e, 0x86, 0x3f,
	0xec, 0x7f
	},
	{
	0x59, 0xbc, 0x04, 0x1d, 0x2d, 0x9b, 0xc5, 0x9d, 0x8e, 0xb2,
	0x8a, 0x0b, 0x43, 0x82, 0x8f, 0xb0, 0x97, 0x64, 0x37, 0xfd,
	0x38, 0x78, 0x5f, 0xad, 0x3e, 0xaa, 0x88, 0xa3, 0xf8, 0xd8,
	0x4a, 0x14
	},
	{
	0x31, 0xbd, 0xad, 0xd9, 0x66, 0x98, 0xc2, 0x04, 0xaa, 0x9c,
	0xe1, 0x44, 0x8e, 0xa9, 0x4a, 0xe1, 0xfb, 0x4a, 0x9a, 0x0b,
	0x3c, 0x9d, 0x77, 0x3b, 0x51, 0xbb, 0x18, 0x22, 0x66, 0x6b,
	0x8f, 0x22
	},
	{
	0x11, 0x75, 0x4c, 0xd7, 0x2a, 0xec, 0x30, 0x9b, 0xf5, 0x2f,
	0x76, 0x87, 0x21, 0x2e, 0x89, 0x57
	},
};

static uint32_t plaintext_key_len[KM_KW_MAX_TESTS] = { 51, 0, 16 , 0 };
static uint8_t plaintext_key[KM_KW_MAX_TESTS][128] = {
	{
	0x27, 0xf3, 0x48, 0xf9, 0xcd, 0xc0, 0xc5, 0xbd, 0x5e, 0x66,
	0xb1, 0xcc, 0xb6, 0x3a, 0xd9, 0x20, 0xff, 0x22, 0x19, 0xd1,
	0x4e, 0x8d, 0x63, 0x1b, 0x38, 0x72, 0x26, 0x5c, 0xf1, 0x17,
	0xee, 0x86, 0x75, 0x7a, 0xcc, 0xb1, 0x58, 0xbd, 0x9a, 0xbb,
	0x38, 0x68, 0xfd, 0xc0, 0xd0, 0xb0, 0x74, 0xb5, 0xf0, 0x1b,
	0x2c
	},
	{
	0x0,
	},
	{
	0x2d, 0xb5, 0x16, 0x8e, 0x93, 0x25, 0x56, 0xf8, 0x08, 0x9a,
	0x06, 0x22, 0x98, 0x1d, 0x01, 0x7d
	},
	{
	0x0,
	},
};

static uint32_t encrypted_key_len[KM_KW_MAX_TESTS] = { 51, 0, 16, 0 };
static uint8_t encrypted_key[KM_KW_MAX_TESTS][128] = {
	{
	0xeb, 0x7c, 0xb7, 0x54, 0xc8, 0x24, 0xe8, 0xd9, 0x6f, 0x7c,
	0x6d, 0x9b, 0x76, 0xc7, 0xd2, 0x6f, 0xb8, 0x74, 0xff, 0xbf,
	0x1d, 0x65, 0xc6, 0xf6, 0x4a, 0x69, 0x8d, 0x83, 0x9b, 0x0b,
	0x06, 0x14, 0x5d, 0xae, 0x82, 0x05, 0x7a, 0xd5, 0x59, 0x94,
	0xcf, 0x59, 0xad, 0x7f, 0x67, 0xc0, 0xfa, 0x5e, 0x85, 0xfa,
	0xb8
	},
	{
	0x0,
	},
	{
	0xfa, 0x43, 0x62, 0x18, 0x96, 0x61, 0xd1, 0x63, 0xfc, 0xd6,
	0xa5, 0x6d, 0x8b, 0xf0, 0x40, 0x5a
	},
	{
	0x0,
	},
};

static uint32_t auth_tag_len[KM_KW_MAX_TESTS] = { 16, 15, 16, 16 };
static uint8_t auth_tag[KM_KW_MAX_TESTS][16] = {
	{
	0xbc, 0x95, 0xc5, 0x32, 0xfe, 0xcc, 0x59, 0x4c, 0x36, 0xd1,
	0x55, 0x02, 0x86, 0xa7, 0xa3, 0xf0
	},
	{
	0x03, 0x00, 0xcb, 0x98, 0x7c, 0x65, 0xf8, 0x99, 0x9e, 0x32,
	0xd7, 0x60, 0x0b, 0x72, 0x50
	},
	{
	0xd6, 0x36, 0xac, 0x1b, 0xbe, 0xdd, 0x5c, 0xc3, 0xee, 0x72,
	0x7d, 0xc2, 0xab, 0x4a, 0x94, 0x89
	},
	{
	0x25, 0x03, 0x27, 0xc6, 0x74, 0xaa, 0xf4, 0x77, 0xae, 0xf2,
	0x67, 0x57, 0x48, 0xcf, 0x69, 0x71
	},
};

uint64_t cm_km_self_test(void)
{
	uint32_t ret = RV_SUCCESS;
	uint32_t i, j;
	uint32_t test_cnt;
	uint32_t fail_cnt;
	uint8_t out_buff[128];
	uint8_t tag_buff[KM_KW_MAX_TAG_LEN];

	CM_KW_CTX_T keyman_kw;

	test_cnt = 0;
	fail_cnt = 0;

	/* Key Wrap/Unwrap with SW KEK */
	for (i = 0; i < KM_KW_MAX_TESTS; i++) {
		test_cnt++;

		printf("[CM] Key wrap/unwrap with SWKEK [%d]\n", i);

		memset(&keyman_kw, 0, sizeof(CM_KW_CTX_T));
		memcpy(keyman_kw.salt, (uint8_t *)kw_salt[i], kw_salt_len[i]);
		keyman_kw.salt_len = kw_salt_len[i];
		memcpy(keyman_kw.iv, (uint8_t *)iv[i], iv_len[i]);
		keyman_kw.iv_len = iv_len[i];
		memcpy(keyman_kw.aad, (uint8_t *)aad[i], aad_len[i]);
		keyman_kw.aad_len = aad_len[i];
		memcpy(keyman_kw.kek, (uint8_t *)kek[i], kek_len[i]);
		keyman_kw.kek_len = kek_len[i];
		memcpy(keyman_kw.plaintext_key, (uint8_t *)plaintext_key[i], plaintext_key_len[i]);
		keyman_kw.plaintext_key_len = plaintext_key_len[i];
		keyman_kw.encrypted_key_len = encrypted_key_len[i];
		keyman_kw.auth_tag_len = auth_tag_len[i];
		keyman_kw.kw_mode = WRAP;
		keyman_kw.kek_mode = SWKEK;

		ret = cm_km_wrap_key_with_kek(&keyman_kw);
		if (ret != RV_SUCCESS) {
			printf("[CM] Error from cm_km_wrap_key_with_kek(): 0x%x\n", ret);
			fail_cnt++;
		} else if ((encrypted_key_len[i] && memcmp(keyman_kw.encrypted_key, encrypted_key[i], encrypted_key_len[i])) ||
			   memcmp(keyman_kw.auth_tag, auth_tag[i], auth_tag_len[i])) {
			printf("[CM] TEST FAIL: Key wrap with SWKEK [%d]\n", i);
			printf("[CM] encrypted_key:\n");
			for(j = 0; j < encrypted_key_len[i]; j++)
				printf("0x%X ", keyman_kw.encrypted_key[j]);
			printf("\n");
			printf("[CM] auth_tag:");
			for(j = 0; j < auth_tag_len[i]; j++)
				printf("0x%X ", keyman_kw.auth_tag[j]);
			printf("\n");

			fail_cnt++;
		} else {
			printf("[CM] TEST PASS: Key wrap with SWKEK [%d]\n", i);
		}

		test_cnt++;

		memset(&keyman_kw, 0, sizeof(CM_KW_CTX_T));
		memcpy(keyman_kw.salt, (uint8_t *)kw_salt[i], kw_salt_len[i]);
		keyman_kw.salt_len = kw_salt_len[i];
		memcpy(keyman_kw.iv, (uint8_t *)iv[i], iv_len[i]);
		keyman_kw.iv_len = iv_len[i];
		memcpy(keyman_kw.aad, (uint8_t *)aad[i], aad_len[i]);
		keyman_kw.aad_len = aad_len[i];
		memcpy(keyman_kw.kek, (uint8_t *)kek[i], kek_len[i]);
		keyman_kw.kek_len = kek_len[i];
		memcpy(keyman_kw.encrypted_key, (uint8_t *)encrypted_key[i], encrypted_key_len[i]);
		keyman_kw.encrypted_key_len = encrypted_key_len[i];
		keyman_kw.plaintext_key_len = plaintext_key_len[i];

		memcpy(keyman_kw.auth_tag, (uint8_t *)auth_tag[i], auth_tag_len[i]);
		keyman_kw.auth_tag_len = auth_tag_len[i];
		keyman_kw.kw_mode = UNWRAP;
		keyman_kw.kek_mode = SWKEK;

		ret = cm_km_wrap_key_with_kek(&keyman_kw);
		if (ret != RV_SUCCESS) {
			printf("[CM] Error from cm_km_wrap_key_with_kek(): 0x%x\n", ret);
			fail_cnt++;
		} else if (plaintext_key_len[i] && memcmp(keyman_kw.plaintext_key, plaintext_key[i], plaintext_key_len[i])) {
			printf("[CM] TEST FAIL: Key unwrap with SWKEK [%d]\n", i);
			printf("[CM] plaintext_key:\n");
			for(j = 0; j < plaintext_key_len[i]; j++)
				printf("0x%X ", keyman_kw.plaintext_key[j]);
			printf("\n");

			fail_cnt++;
		} else {
			printf("[CM] TEST PASS: Key unwrap with SWKEK [%d]\n", i);
		}
	}

	/* Key Wrap/Unwrap with HW KEK */
	for (i = 0; i < KM_KW_MAX_TESTS; i++) {
		test_cnt++;

		printf("[CM] TEST START: Key wrap/unwrap with HWKEK [%d]\n", i);
		memset(out_buff, 0, sizeof(out_buff));
		memset(tag_buff, 0, sizeof(tag_buff));
		memset(&keyman_kw, 0, sizeof(CM_KW_CTX_T));
		memcpy(keyman_kw.salt, (uint8_t *)kw_salt[i], kw_salt_len[i]);
		keyman_kw.salt_len = kw_salt_len[i];
		memcpy(keyman_kw.iv, (uint8_t *)iv[i], iv_len[i]);
		keyman_kw.iv_len = iv_len[i];
		memcpy(keyman_kw.aad, (uint8_t *)aad[i], aad_len[i]);
		keyman_kw.aad_len = aad_len[i];
		memcpy(keyman_kw.plaintext_key, (uint8_t *)plaintext_key[i], plaintext_key_len[i]);
		keyman_kw.plaintext_key_len = plaintext_key_len[i];
		keyman_kw.encrypted_key_len = encrypted_key_len[i];
		keyman_kw.auth_tag_len = auth_tag_len[i];
		keyman_kw.kek_len = 0;
		keyman_kw.kw_mode = WRAP;
		keyman_kw.kek_mode = HWKEK;

		ret = cm_km_wrap_key_with_kek(&keyman_kw);
		if (ret != RV_SUCCESS) {
			printf("[CM] Error from cm_km_wrap_key_with_kek(): 0x%x\n", ret);
			fail_cnt++;
			continue;
		} else {
			memcpy(out_buff, (uint8_t *)keyman_kw.encrypted_key, encrypted_key_len[i]);
			memcpy(tag_buff, (uint8_t *)keyman_kw.auth_tag, auth_tag_len[i]);
			printf("[CM] TEST PASS: Key wrap with HWKEK [%d]\n", i);
		}

		memset(&keyman_kw, 0, sizeof(CM_KW_CTX_T));
		memcpy(keyman_kw.salt, (uint8_t *)kw_salt[i], kw_salt_len[i]);
		keyman_kw.salt_len = kw_salt_len[i];
		memcpy(keyman_kw.iv, (uint8_t *)iv[i], iv_len[i]);
		keyman_kw.iv_len = iv_len[i];
		memcpy(keyman_kw.aad, (uint8_t *)aad[i], aad_len[i]);
		keyman_kw.aad_len = aad_len[i];

		memcpy(keyman_kw.encrypted_key, (uint8_t *)out_buff, encrypted_key_len[i]);
		keyman_kw.encrypted_key_len = encrypted_key_len[i];
		keyman_kw.plaintext_key_len = plaintext_key_len[i];
		memcpy(keyman_kw.auth_tag, (uint8_t *)tag_buff, auth_tag_len[i]);
		keyman_kw.auth_tag_len = auth_tag_len[i];
		keyman_kw.kek_len = 0;
		keyman_kw.kw_mode = UNWRAP;
		keyman_kw.kek_mode = HWKEK;

		ret = cm_km_wrap_key_with_kek(&keyman_kw);
		if (ret != RV_SUCCESS) {
			printf("[CM] Error from cm_km_wrap_key_with_kek(): 0x%x\n", ret);
			fail_cnt++;
		} else if (plaintext_key_len[i] && memcmp(keyman_kw.plaintext_key, plaintext_key[i], plaintext_key_len[i])) {
			printf("[CM] TEST FAIL: Key unwrap with HWKEK [%d]\n", i);
			printf("[CM] plaintext_key:\n");
			for(j = 0; j < plaintext_key_len[i]; j++)
				printf("0x%X ", keyman_kw.plaintext_key[j]);
			printf("\n");

			fail_cnt++;
		} else {
			printf("[CM] TEST PASS: Key unwrap with HWKEK [%d]\n", i);
		}
	}

	if (fail_cnt > 0)
		printf("[CM] TEST FAIL occur: %d/%d\n", fail_cnt, test_cnt);

	return ret;
}
