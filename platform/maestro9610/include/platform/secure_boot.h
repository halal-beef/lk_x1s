#ifndef _SECURE_BOOT_H_
#define _SECURE_BOOT_H_

#include "../../../../lib/libavb/libavb.h"

/******************************************************************************/
/* Definition value */
/******************************************************************************/
/* secure boot crypto variable */
#define SHA1_DIGEST_LEN                 (20)
#define SHA1_BLOCK_LEN                  (64)
#define SHA256_DIGEST_LEN               (32)
#define SHA256_BLOCK_LEN                (64)
#define SHA512_DIGEST_LEN               (64)
#define SHA512_BLOCK_LEN                (128)

/* AVB variable */
#define AVB_CMD_MAX_SIZE		(1024)

/******************************************************************************/
/* Secure Boot context used by EL3 */
/******************************************************************************/
enum {
	ALG_SHA1,
	ALG_SHA256,
	ALG_SHA512,
};

struct ace_hash_ctx {
	int alg;
	size_t buflen;
	unsigned char buffer[SHA512_BLOCK_LEN];
	unsigned int state[SHA512_DIGEST_LEN/4];
	unsigned int prelen_high;
	unsigned int prelen_low;
};

uint32_t el3_sss_hash_digest(
	uint32_t addr,
	uint32_t size,
	uint32_t alg,
	uint8_t *hash);

uint32_t el3_sss_hash_init(
	uint32_t alg,
	struct ace_hash_ctx *ctx);

uint32_t el3_sss_hash_update(
	uint32_t addr,
	uint32_t remain_size,
	uint32_t unit_size,
	struct ace_hash_ctx *ctx,
	uint32_t done_flag);

uint32_t el3_sss_hash_final(
	struct ace_hash_ctx *ctx,
	uint8_t *hash);

/******************************************************************************/
/* Android verified boot */
/******************************************************************************/
void set_avbops(void);

uint32_t avb_main(const char *suffix, char *cmdline);

uint32_t get_ops_addr(struct AvbOps **ops_addr);

#endif /* _SECURE_BOOT_H_ */
