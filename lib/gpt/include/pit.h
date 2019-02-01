/*
 * (C) Copyright 2017 SAMSUNG Electronics
 * Kiwoong Kim <kwmad.kim@samsung.com>
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 *
 */

#ifndef __PIT_H__
#define __PIT_H__

#include <sys/types.h>
#include <lib/bio.h>
// TODO:
/*
#include <common.h>
#include <compiler.h>
#include <part.h>
*/

#undef	PIT_DEBUG
//#define PIT_DEBUG

#define PIT_LBA_ALIGMENT		8		/* 4KB */
#define PIT_SECTOR_SIZE			512
#define PIT_MAGIC			0x12349876

/*
 * This is how PIT location was originated.
 *
 * A size of GPT entry is total 128 bytes and I assume
 * supporting '128' entries at maximum.
 * With these values, we need space as follow:
 * - 4.x blocks (UFS), 32.X blocks (eMMC)
 * And considering 4KB aligment, we need to reserve '48' units in 512 bytes
 * because we need to consider sizes of PMBR and a GPT header, and those sizes
 * depends on its block size, i.e. 8 units (UFS), 2 units (eMMC)
 */
#define PIT_PART_META			48

#define PIT_GPT_STRING_SIZE		0x2000

#define PIT_EMMC_ERASE_SIZE		(1024)

#define PIT_DISK_SIZE_LIMIT		40 /* increase for pit 128 entry */
#define PIT_SIZE_LIMIT			(PIT_DISK_SIZE_LIMIT * PIT_SECTOR_SIZE)


/*
 * Setting this definition to smaller than maximum value is recommended.
 */
#define PIT_MAX_PART_NUM		60


enum pit_filesys {
	FS_TYPE_NONE	= 0,
	FS_TYPE_BASIC,
	FS_TYPE_SPARSE_EXT4	= 5,
	FS_TYPE_SPARSE_F2FS	= 6,
};

/*
 * PIT assumes that a unit size of block is 512 bytes.
 * Therefore a sort of translation is necessary when passing it to
 * block device drivers.
 */
struct pit_entry {
	u32 reserved[4];
	u32 filesys;		/* if this is gpt entry */
	u32 blkstart;	/* overrided, start lba */
	u32 blknum;	/* size as block */
	u32 lun;	/* partition # */
	u32 reserved1;
	char name[32];		/* partition name */
	u32 reserved2[8];
	char option[32];	/* only for indication of 'remained' */
} __attribute__((packed));

struct pit_info {
	u32 magic;	/* to check integrity */
	u32 count;	/* a number of partitions */
	u32 reserved[5];

	struct pit_entry pte[PIT_MAX_PART_NUM];
} __attribute__((packed));


/* Struct for gpt information */
struct gpt_info {
	u32 gpt_start_lba; /* The unit is 512B */
	u32 gpt_last_lba; /* The unit is 512B, usable lba limitation */
	u32 gpt_entry_cnt;
};

enum pit_op {
	PIT_OP_FLASH = 0,
	PIT_OP_ERASE,
	PIT_OP_LOAD,
};

/*
 * Public functions
 */
void pit_init(void);
int pit_update(void *buf, unsigned int size);
struct pit_entry *pit_get_part_info(const char *name);
int pit_access(struct pit_entry *ptn, int op, u64 addr, u32 size);
u64 pit_get_length(struct pit_entry *ptn);

/*
 * ---------------------------------------------------------------------------
 * Exported functions
 * ---------------------------------------------------------------------------
 */
// TODO:
bdev_t *pit_get_boot_device(char *);

#endif
