/*
 * (C) Copyright 2019 SAMSUNG Electronics
 * Kiwoong Kim <kwmad.kim@samsung.com>
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 *
 */

#ifndef __PIT_FORMAT_H__
#define __PIT_FORMAT_H__

#define PIT_LBA_ALIGMENT		8		/* 4KB */
#define PIT_SECTOR_SIZE			512
#define PIT_MAGIC			0x12349876
#define PIT_PART_META			47		/* based on GPT */


/*
 * Reserved area
 *
 * |-----|-----...-----|--------------....
 * | GPT |  RESERVED   |  MANAGED BY PIT
 * | +++ INVISIBLE +++ |  ..
 *
 * User partition management is available in the area
 * where GPT and reserved area are excluded.
 */
#define PIT_DISK_SIZE_LIMIT		64					/* Up to 127 entries */
#define PIT_SIZE_LIMIT			(PIT_DISK_SIZE_LIMIT * PIT_SECTOR_SIZE)
#define PIT_MAX_PART_NUM		60					/* PIT_MAX_PART_NUM + 1 should be 16 (=4096/256) */
#define PIT_INVISIABLE_AREA		(32 * 1024 * 1024)			/* Total 32MB, GPT and reserved area */
#define PIT_RESERVED_AREA		(PIT_INVISIABLE_AREA - (PIT_PART_META + 1))
#define PIT_DISK_LOC			PIT_PART_META + 1

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

enum pit_filesys {
	FS_TYPE_NONE		= 0,
	FS_TYPE_BASIC,
	FS_TYPE_SPARSE_EXT4	= 5,
	FS_TYPE_SPARSE_F2FS,
};

#endif
