/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __GPT_H__
#define __GPT_H__

#include <pit.h>
#include <lib/bio.h>

/* GPT HEADER  */
#define MSDOS_MBR_SIGNATURE 0xAA55
#define EFI_PMBR_OSTYPE_EFI 0xEF
#define EFI_PMBR_OSTYPE_EFI_GPT 0xEE

#define GPT_HEADER_SIGNATURE 0x5452415020494645ULL
#define GPT_HEADER_REVISION_V1 0x00010000
#define GPT_PRIMARY_PARTITION_TABLE_LBA 1ULL

#define GPT_ENTRY_NAME			"gpt"
#define GPT_ENTRY_NUMBERS               128
#define GPT_ENTRY_SIZE                  128

#define GPT_TABLE_BLOCK			32

#define BLOCK_SIZE_4KB                  8
#define UFS_BSIZE                       4096
#define MMC_BSIZE                       512

#define PMBR_LBA		1
#define GPT_HEAD_LBA		1
#define GPT_TABLE_LBA		2

#define FAT_PART_SIZE		409600

#define UUID_DISK "12345678-1234-1234-1234-123456789012"
#define FILE_SYSTEM_DATA "0fc63daf-8483-4772-8e79-3d69d8477de4"
#define BASIC_DATA "ebd0a0a2-b9e5-4433-87c0-68b6b72699c7"
#define EXYNOS_UNIQUE_GUID "4558594e-4f53-534c-7369-504152543d"

#define HEAD_SIZE 0x5C
#define TABLE_SIZE 0x80

#define be16_to_cpu(x) \
		((((x) & 0xff00) >> 8) | \
		 (((x) & 0x00ff) << 8))
#define cpu_to_be16(x) be16_to_cpu(x)

#define be32_to_cpu(x) \
		((((x) & 0xff000000) >> 24) | \
		 (((x) & 0x00ff0000) >>  8) | \
		 (((x) & 0x0000ff00) <<  8) | \
		 (((x) & 0x000000ff) << 24))
#define cpu_to_be32(x) be32_to_cpu(x)

#define be64_to_cpu(x) \
		((((x) & 0x00000000000000ff) << 56) | \
		 (((x) & 0x000000000000ff00) << 40) | \
		 (((x) & 0x0000000000ff0000) << 24) | \
		 (((x) & 0x00000000ff000000) << 8) | \
		 (((x) & 0x000000ff00000000) >> 8) | \
		 (((x) & 0x0000ff0000000000) >> 24) | \
		 (((x) & 0x00ff000000000000) >> 40) | \
		 (((x) & 0xff00000000000000) >> 56))
#define cpu_to_be64(x) be64_to_cpu(x)

#define EFI_GUID(a,b,c,d0,d1,d2,d3,d4,d5,d6,d7) \
		((efi_guid_t) \
		 {{ (a) & 0xff, ((a) >> 8) & 0xff, ((a) >> 16) & 0xff, ((a) >> 24) & 0xff, \
		 (b) & 0xff, ((b) >> 8) & 0xff, \
		 (c) & 0xff, ((c) >> 8) & 0xff, \
		 (d0), (d1), (d2), (d3), (d4), (d5), (d6), (d7) }})

#define PARTITION_SYSTEM_GUID \
		EFI_GUID( 0xC12A7328, 0xF81F, 0x11d2, \
				0xBA, 0x4B, 0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B)

#define LEGACY_MBR_PARTITION_GUID \
		EFI_GUID( 0x024DEE41, 0x33E7, 0x11d3, \
				0x9D, 0x69, 0x00, 0x08, 0xC7, 0x81, 0xF3, 0x9F)

#define PARTITION_MSFT_RESERVED_GUID \
		EFI_GUID( 0xE3C9E316, 0x0B5C, 0x4DB8, \
				0x81, 0x7D, 0xF9, 0x2D, 0xF0, 0x02, 0x15, 0xAE)

#define PARTITION_BASIC_DATA_GUID \
		EFI_GUID( 0xEBD0A0A2, 0xB9E5, 0x4433, \
				0x87, 0xC0, 0x68, 0xB6, 0xB7, 0x26, 0x99, 0xC7)

#define PARTITION_LINUX_RAID_GUID \
		EFI_GUID( 0xa19d880f, 0x05fc, 0x4d3b, \
				0xa0, 0x06, 0x74, 0x3f, 0x0f, 0x84, 0x91, 0x1e)

#define PARTITION_LINUX_SWAP_GUID \
		EFI_GUID( 0x0657fd6d, 0xa4ab, 0x43c4, \
				0x84, 0xe5, 0x09, 0x33, 0xc8, 0x4b, 0x4f, 0x4f)

#define PARTITION_LINUX_LVM_GUID \
		EFI_GUID( 0xe6d6d379, 0xf507, 0x44c2, \
				0xa2, 0x3c, 0x23, 0x8f, 0x2a, 0x3d, 0xf9, 0x28)

typedef struct {
	u8 b[16];
} efi_guid_t;

struct p_mbr_head {
	u8 boot_code[446];
	u8 active;
	u8 s_head;		/* starting head */
	u8 s_sector;		/* starting sector */
	u8 s_cylinder;		/* starting cylinder */
	u8 partition_type;
	u8 e_head;		/* end head */
	u8 e_sector;		/* end sector */
	u8 e_cylinder;		/* end cylinder */
	bnum_t start_sect_cnt;	/* starting sector counting form 0 */
	bnum_t nr_sect;
	u32 reserved[12];
	u16 signature;
} __attribute__ ((__packed__));

struct gpt_header {
	u64 signature;
	u32 revision;
	u32 head_sz;
	u32 head_crc;
	u32 reserved;
	u64 gpt_header;
	u64 gpt_back_header;
	u64 start_lba;
	u64 end_lba;
	u8 guid[16];
	u64 part_table_lba;
	u32 part_num_entry;
	u32 part_size_entry;
	u32 part_table_crc;
	u8 reserved1[430];
} __attribute__ ((__packed__));

typedef union _gpt_table_attributes{
	struct {
		u64 reserved;
	} fields;
} __attribute__ ((__packed__)) gpt_table_attributes;

#define PT_NAME_SZ	(72 / sizeof(u16))

struct gpt_part_table {
	efi_guid_t part_type;
	efi_guid_t part_guid;
	u64 part_start_lba;
	u64 part_end_lba;
	gpt_table_attributes attributes;
	u16 part_name[72 / sizeof(u16)];
} __attribute__ ((__packed__));

struct gpt_backup_header {
	u64 signature;
	u32 revision;
	u32 head_sz;
	u32 head_crc;
	u32 reserved;
	u64 gpt_header;
	u64 gpt_back_header;
	u64 start_lba;
	u64 end_lba;
	u8 guid[16];
	u64 part_table;
	u32 part_num_entry;
	u32 part_size_entry;
	u32 part_table_crc;
} __attribute__ ((__packed__));

/*
 * Public Function
 */
int gpt_create(struct pit_info *pit, u32 pit_last_lba);
extern int get_unique_guid(char *ptr_name, char *buf);
#endif
