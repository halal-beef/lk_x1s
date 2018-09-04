/**
 * @file  rpmb.h
 * @brief CryptoManager wrapper functions for RPMB security
 *
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied
 * or distributed, transmitted, transcribed, stored in a retrieval system
 * or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed to third parties
 * without the express written permission of Samsung Electronics.
 */

#ifndef __RPMB_H__
#define __RPMB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <arch/arm64.h>

/* For debugging */
//#define RPMB_DBG      1

/* SMC id for CryptoManager function call */
#define SMC_AARCH64_PREFIX			0xC2000000
#define SMC_CM_RPMB				0x1018
#define SMC_CM_DRBG				0x1019
#define SMC_SRPMB_PROVISIONED			0x3818

/* key and hmac length definition */
#define RPMB_KEY_LEN				32
#define RPMB_HMAC_LEN				32

/* define maximum retry count */
#define MAX_SMC_RETRY_CNT			0x5000

/* error codes from CryptoManager F/W */
#define RV_SUCCESS				0
#define RV_SYNC_AES_BUSY			0x20000
#define RV_RPMB_ERROR_CODE_BASE			0x40200
#define RV_RPMB_INVALID_CMD			0x40201
#define RV_RPMB_INVALID_PARAM			0x40202
#define RV_RPMB_PARAM_NULL_POINTER		0x40203
#define RV_RPMB_INVALID_KEY_LEN			0x40204
#define RV_RPMB_KEY_BLOCKED			0x40205
#define RV_RPMB_HMAC_BLOCKED			0x40206
#define RV_RPMB_RI_TABLE_NOT_INITIALIZED        0x40300
#define RV_RPMB_INVALID_ROLLBACK_INDEX          0x40301
#define RV_RPMB_PERSIST_NAME_NOT_FOUND          0x40302
#define RV_RPMB_INVALID_PERSIST_DATA_SIZE       0x40303


/* error code for bootloader */
#define RV_BOOT_RPMB_EXCEED_SMC_RETRY_CNT	0x80100

/*
 * if the caller of this fucntion is executed on the cache enabled memory area
 * below definitions and macros should be defined
 * and also mercos should be defined
 */
//#define CACHE_ENABLED
#define CACHE_CLEAN(addr, len)
#define CACHE_CLEAN_INVALIDATE(addr, len)

/* RPMB function number */
	enum {
		RPMB_GET_KEY = 1,
		RPMB_BLOCK_KEY,
		RPMB_GET_HMAC,
		RPMB_BLOCK_HMAC,
		RPMB_GET_HMAC_SWD,
	};

/* data definition for RPMB hmac */
	typedef struct rpmb_param {
		uint64_t input_data;
		uint64_t input_len;
		uint64_t output_data;
	} rpmb_param;

	uint32_t get_RPMB_key(size_t key_len, uint8_t * rpmb_key);
	uint32_t block_RPMB_key(void);
	uint32_t get_RPMB_hmac(const uint8_t * input_data, size_t input_len, uint8_t * output_data);
	uint32_t block_RPMB_hmac(void);

#ifdef __cplusplus
}
#endif
struct rpmb_packet {
	u16 request;
	u16 result;
	u16 count;
	u16 address;
	u32 write_counter;
	u8 nonce[16];
	u8 data[256];
	u8 Key_MAC[32];
	u8 stuff[196];
};

typedef struct random_ctx {
        u64 output;
        u64 len;
        u64 reserved[2];
} RANDOM_CTX;

/* Functions */
void rpmb_key_programming(void);
uint32_t block_RPMB_hmac(void);
int rpmb_load_boot_table(void);
int rpmb_get_rollback_index(size_t loc, uint64_t *rollback_index);
int rpmb_set_rollback_index(size_t loc, uint64_t rollback_index);
int rpmb_read_persistent_value(const char *name,
		size_t buffer_size,
		uint8_t *out_buffer,
		size_t *out_num_bytes_read);
int rpmb_write_persistent_value(const char *name,
		size_t value_size,
		const uint8_t *value);
int rpmb_get_lock_state(uint32_t *state);
int rpmb_set_lock_state(uint32_t state);

/* Parameter */
#define HMAC_SIZE	32
#define NONCE_SIZE      16
#define DATA_SIZE	256
#define HMAC_CALC_SIZE	284
#define HMAC_START_BYTE	196
#define DATA_START_BYTE	228
#define RPMB_SIZE	512
#define RPMB_BLOCK_PER_PARTITION 512

#define SSS_BASE			0x10830000

#define FEED_REG_BASE			(SSS_BASE + 0x0000)
#define FCINTSTAT			(FEED_REG_BASE + 0x0000)
#define FCINTENSET			(FEED_REG_BASE + 0x0004)
#define FCINTENCLR			(FEED_REG_BASE + 0x0008)
#define FCINTPEND			(FEED_REG_BASE + 0x000c)
#define FCFIFOSTAT			(FEED_REG_BASE + 0x0010)
#define FCFIFOCTRL			(FEED_REG_BASE + 0x0014)
#define FCGLOBAL			(FEED_REG_BASE + 0x0018)
#define FCBRDMAS			(FEED_REG_BASE + 0x0020)
#define FCBRDMAL			(FEED_REG_BASE + 0x0024)
#define FCBRDMAC			(FEED_REG_BASE + 0x0028)
#define FCBTDMAS			(FEED_REG_BASE + 0x0030)
#define FCBTDMAL			(FEED_REG_BASE + 0x0034)
#define FCBTDMAC			(FEED_REG_BASE + 0x0038)
#define FCHRDMAS			(FEED_REG_BASE + 0x0040)
#define FCHRDMAL			(FEED_REG_BASE + 0x0044)
#define FCHRDMAC			(FEED_REG_BASE + 0x0048)

#define HASH_REG_BASE			(SSS_BASE + 0x0400)
#define HASH_CONTROL_1			(HASH_REG_BASE + 0x0000)
#define HASH_CONTROL_2			(HASH_REG_BASE + 0x0004)
#define HASH_FIFO_MODE_EN		(HASH_REG_BASE + 0x0008)
#define HASH_BYTE_SWAP		(HASH_REG_BASE + 0x000c)
#define HASH_STATUS			(HASH_REG_BASE + 0x0010)
#define HASH_SEED_CONF			(HASH_REG_BASE + 0x0014)
#define HASH_MSG_SIZE_LOW		(HASH_REG_BASE + 0x0020)
#define HASH_MSG_SIZE_HIGH		(HASH_REG_BASE + 0x0024)
#define HASH_PRE_MSG_LENG_LOW	(HASH_REG_BASE + 0x0028)
#define HASH_PRE_MSG_LENG_HIGH	(HASH_REG_BASE + 0x002c)
#define HASH_DATA_IN			(HASH_REG_BASE + 0x0030)
#define HASH_HMAC_KEY_IN		(HASH_REG_BASE + 0x0070)
#define HASH_USER_IV_IN		(HASH_REG_BASE + 0x00b0)
#define HASH_RESULT			(HASH_REG_BASE + 0x0100)
#define HASH_SEED_IN			(HASH_REG_BASE + 0x0140)
#define HASH_PRNG			(HASH_REG_BASE + 0x0160)

#define UFS_RPMB 1

#endif				/* __RPMB_H__ */
