/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef _CM_API_H_
#define _CM_API_H_

/*
 ****************************************************************************
 * defines for general purpose
 *****************************************************************************/
#define VIRT_TO_PHYS(_virt_addr_) ((uint64_t)(_virt_addr_))

#define CM_FLUSH_DCACHE_RANGE(addr, length) \
	clean_dcache_range((unsigned long long)addr, (unsigned long long)(addr + length))
#define CM_INV_DCACHE_RANGE(addr, length) \
	invalidate_dcache_range((unsigned long long)addr, (unsigned long long)(addr + length))

#define CACHE_WRITEBACK_SHIFT	6
#define CACHE_WRITEBACK_GRANULE	(1 << CACHE_WRITEBACK_SHIFT)

#define MASK_LSB8		0x00000000000000FFULL
#define MASK_LSB16		0x000000000000FFFFULL
#define MASK_LSB32		0x00000000FFFFFFFFULL

/* SMC ID for CM APIs */
#define SMC_AARCH64_PREFIX	(0xC2000000)
#define SMC_CM_KEY_MANAGER	(0x1013)
#define SMC_CM_RANDOM		(0x101C)
#define SMC_CM_SECURE_BOOT	(0x101D)

#define SHA256_DIGEST_LEN	(32)

/*
 ****************************************************************************
 * ERROR Codes
 *****************************************************************************/
#define RV_SUCCESS		0
#define RV_SB_LDFW_NOT_LOADED	0xFFFFFFFF
#define RV_SB_CMD_BLOCKED	0x51007

/*
 ****************************************************************************
 * CM API functions for RootOfTrust
 ****************************************************************************
 * define device state*/
enum device_state {
	UNLOCKED,
	LOCKED,
};

/* define boot state */
enum boot_state {
	GREEN,
	YELLOW,
	ORANGE,
	RED,
};

/* define secure boot commands */
enum secure_boot_cmd {
	SB_CHECK_SIGN_AP,
	SB_CHECK_SIGN_CP,
	SB_SELF_TEST,
	SB_SET_PUBLIC_KEY,
	SB_SET_DEVICE_STATE,
	SB_ERASE_PUBLIC_KEY,
	SB_GET_BOOT_INFO,
	SB_CHECK_SIGN_NWD,
	SB_GET_ROOT_OF_TRUST_INFO,
	SB_SET_BOOT_STATE,
	SB_SET_OS_VERSION,
	SB_BLOCK_CMD,
	SB_CHECK_SIGN_NWD_WITH_PUBKEY,
	SB_CHECK_SIGN_NWD_AND_RP,               /* Automotive */
	SB_CHECK_SIGN_NWD_WITH_HASH_AND_RP,     /* Automotive */
	SB_CHECK_SIGN_CP_WITH_HASH,
	SB_GET_AVB_KEY,
	SB_SET_VENDOR_BOOT_VERSION,
};

typedef struct {
	uint32_t verified_boot_state;
	uint32_t device_locked;
	uint32_t os_version;
	uint32_t os_patch_level;
	uint8_t	verified_boot_key[SHA256_DIGEST_LEN];
	uint32_t vendor_patch_level;
	uint32_t boot_patch_level;
	uint64_t reserved[3];
} SB_PARAM_ROOT_OF_TRUST_ST __attribute__((__aligned__(CACHE_WRITEBACK_GRANULE)));

uint64_t cm_secure_boot_set_pubkey(uint8_t *pub_key_ptr, uint32_t pub_key_len);
uint64_t cm_secure_boot_erase_pubkey(void);
uint64_t cm_secure_boot_set_os_version(uint32_t os_version,
                                       uint32_t patch_month_year);
uint64_t cm_secure_boot_set_vendor_boot_version(uint32_t vendor_patch_level,
                                                uint32_t boot_patch_level);
uint64_t cm_secure_boot_set_device_state(uint32_t state);
uint64_t cm_secure_boot_set_boot_state(uint32_t state);
uint64_t cm_secure_boot_get_root_of_trust(
	SB_PARAM_ROOT_OF_TRUST_ST *root_of_trust);
uint64_t cm_secure_boot_block_cmd(void);
uint64_t cm_secure_boot_self_test(void);

/*
 ****************************************************************************
 * CM API functions for Random
 *****************************************************************************/
#define GET_RANDOM_WORD	(0)

uint64_t cm_get_random(uint8_t *random, uint32_t random_len);
uint64_t cm_random_self_test(void);

/*
 ****************************************************************************
 * CM API functions for Keymanager
 *****************************************************************************/
#define KM_KW_MAX_SALT_LEN	(60)
#define KM_KW_MAX_IV_LEN	(12)
#define KM_KW_MAX_AAD_LEN	(32)
#define KM_KW_MAX_KEY_LEN	(32)
#define KM_KW_MAX_INPUT_LEN	(4096)
#define KM_KW_MAX_TAG_LEN	(16)

/* define function number */
enum {
	CMD_REK_BASED_KDF = 0,
	CMD_KEY_WRAP_WITH_KEK,
};

enum kw_mode {
	WRAP,
	UNWRAP,
};

enum kek_mode {
	SWKEK,
	HWKEK,
};

typedef struct cm_kw_ctx {
	uint8_t salt[KM_KW_MAX_SALT_LEN];
	uint32_t salt_len;
	uint8_t iv[KM_KW_MAX_IV_LEN];
	uint32_t iv_len;
	uint8_t aad[KM_KW_MAX_AAD_LEN];
	uint32_t aad_len;
	uint8_t kek[KM_KW_MAX_KEY_LEN];
	uint32_t kek_len;
	uint8_t plaintext_key[KM_KW_MAX_INPUT_LEN];
	uint32_t plaintext_key_len;
	uint8_t encrypted_key[KM_KW_MAX_INPUT_LEN];
	uint32_t encrypted_key_len;
	uint8_t auth_tag[KM_KW_MAX_TAG_LEN];
	uint32_t auth_tag_len;
	uint32_t kw_mode;
	uint32_t kek_mode;
	uint32_t reserved[17];
} CM_KW_CTX_T __attribute__((__aligned__(CACHE_WRITEBACK_GRANULE)));

uint64_t cm_km_wrap_key_with_kek(CM_KW_CTX_T *ctx);
uint64_t cm_km_self_test(void);

#endif /* _CM_API_H_ */
