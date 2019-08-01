/*
 * (C) Copyright 2019 SAMSUNG Electronics
 * Kiwoong Kim <kwmad.kim@samsung.com>
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __GPT_FORMAT_H__
#define __GPT_FORMAT_H__

#include <guid.h>
/*
 * User defined
 */
#define MAX_NUM_OF_LU			8
#define DEF_NUM_OF_LU			3	/* Up to 3, assuming total three LUs */

/* GPT HEADER  */
#define MSDOS_MBR_SIGNATURE		0xAA55
#define EFI_PMBR_OSTYPE_EFI		0xEF
#define EFI_PMBR_OSTYPE_EFI_GPT		0xEE

#define GPT_HEADER_SIGNATURE		0x5452415020494645ULL
#define GPT_HEADER_REVISION_V1		0x00010000
#define GPT_PRIMARY_PARTITION_TABLE_LBA	1ULL

#define GPT_ENTRY_NAME			"gpt"
#define GPT_ENTRY_NUMBERS               128
#define GPT_ENTRY_SIZE                  128

#define GPT_TABLE_BLOCK			32

#define PMBR_LBA			1
#define GPT_HEAD_LBA			1
#define GPT_TABLE_LBA			2

#define NULL_GUID			"00000000-0000-0000-0000-000000000000"
#define UUID_DISK			"4558594e-4f53-4449-534B-00000000000"
#define EXYNOS_TYPE_GUID		"4558594e-4f53-5041-5254-4954494f4e00"
#define EXYNOS_UNIQUE_GUID		"4558594e-4f53-554e-4951-554549443"

#define HEAD_SIZE			0x5C
#define TABLE_SIZE			0x80

typedef struct {
	__u8 b[16];
} efi_guid_t;

/* Aligned to 512 bytes */
struct gpt_header {
	__u64 signature;
	__u32 revision;
	__u32 head_sz;
	__u32 head_crc;			/* undecisable */
	__u32 reserved;
	__u64 gpt_header;
	__u64 gpt_back_header;		/* undecisable */
	__u64 start_lba;
	__u64 end_lba;			/* undecisable */
	__u8 guid[16];
	__u64 part_table_lba;
	__u32 part_num_entry;
	__u32 part_size_entry;
	__u32 part_table_crc;		/* undecisable */
	__u8 reserved1[420];
#ifndef PACKED_NO_EXSITED
} __attribute__ ((__packed__));
#else
};
#endif

typedef union _gpt_table_attributes{
	__u64 fields;
	struct {
		__u64 reserved:48;	/* reserved by UEFI */
		__u64 type:4;		/* Partition type, such as EXT4, F2FS */
		__u64 reserved1:12;
	};
#ifndef PACKED_NO_EXSITED
} __attribute__ ((__packed__)) gpt_table_attributes;
#else
} gpt_table_attributes;
#endif

/* Aligned to 128 bytes */
struct gpt_entry {
	efi_guid_t part_type;
	efi_guid_t part_guid;
	__u64 part_start_lba;
	__u64 part_end_lba;
	gpt_table_attributes attributes;
	__u16 part_name[UID_STR_LEN];
#ifndef PACKED_NO_EXSITED
} __attribute__ ((__packed__));
#else
};
#endif

enum fstype {
	FS_TYPE_HIDDEN	= 0,
	FS_TYPE_NO_FS,
	FS_TYPE_SPARSE_EXT4,
	FS_TYPE_SPARSE_F2FS,
	FS_TYPE_NUM,
};

#define ROUNDUP(a, b)			(((a) + ((b)-1)) & ~((b)-1))

#endif /* __GPT_FORMAT_H__*/
