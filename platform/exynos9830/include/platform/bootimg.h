/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef _BOOT_IMAGE_H_
#define _BOOT_IMAGE_H_

typedef struct boot_img_hdr boot_img_hdr;

#define BOOT_MAGIC		"ANDROID!"
#define BOOT_MAGIC_SIZE		8
#define BOOT_NAME_SIZE		16
#define BOOT_ARGS_SIZE		512
#define BOOT_EXTRA_ARGS_SIZE	1024

struct boot_img_hdr {
	uint8_t magic[BOOT_MAGIC_SIZE];

	uint32_t kernel_size;           /* size in bytes */
	uint32_t kernel_addr;           /* physical load addr */

	uint32_t ramdisk_size;          /* size in bytes */
	uint32_t ramdisk_addr;          /* physical load addr */

	uint32_t second_size;           /* size in bytes */
	uint32_t second_addr;           /* physical load addr */

	uint32_t tags_addr;             /* physical addr for kernel tags */
	uint32_t page_size;             /* flash page size we assume */
	uint32_t header_version;        /* reserved for future expansion: MUST be 0 */

	/* operating system version and security patch level; for
	 * version "A.B.C" and patch level "Y-M-D":
	 * ver = A << 14 | B << 7 | C         (7 bits for each of A, B, C)
	 * lvl = ((Y - 2000) & 127) << 4 | M  (7 bits for Y, 4 bits for M)
	 * os_version = ver << 11 | lvl */
	uint32_t os_version;

	uint8_t name[BOOT_NAME_SIZE]; /* asciiz product name */

	uint8_t cmdline[BOOT_ARGS_SIZE];

	uint32_t id[8]; /* timestamp / checksum / sha1 / etc */

	/* Supplemental command line data; kept here to maintain
	 * binary compatibility with older versions of mkbootimg */
	uint8_t extra_cmdline[BOOT_EXTRA_ARGS_SIZE];

	unsigned recovery_dtbo_size;
	unsigned long long recovery_dtbo_offset;
	unsigned header_size;
} __attribute__((packed));
#endif /* _BOOT_IMAGE_H_ */
