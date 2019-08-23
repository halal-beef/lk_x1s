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

#ifndef __GPT_H__
#define __GPT_H__

#include <types.h>
#include <sys/types.h>
#include <guid.h>

/*
 * Basic unit to be used to describe partition layout.
 * Binaries exported by external tools would inform size information in this unit.
 */
#define SECTOR_SIZE			512U

#define GPT_MAX_HEADER_SIZE		4096U	/* Up to UFS block size */

typedef struct _PART_ENTRY {
	u32 magic;
	char name[UID_STR_LEN];
	u8 lun;
	void *gpt_entry;
} PART_ENTRY;

void gpt_init(enum __boot_dev_id id);
int gpt_update(u8 *buf);
void gpt_show_info(void);
PART_ENTRY *gpt_get_entry(const char *name);
char *gpt_get_fs_type(PART_ENTRY *part_e);
u32 gpt_get_lun(PART_ENTRY *part_e);
int gpt_get_part_unique_guid(PART_ENTRY *part_e, char *buf);
u32 gpt_get_part_start_in_blks(PART_ENTRY *part_e);
u32 gpt_get_part_start_in_secs(PART_ENTRY *part_e);
u64 gpt_get_part_size_in_bytes(PART_ENTRY *part_e);
int gpt_read_part(PART_ENTRY *part_e, void *buf, u64 offset_in_bytes, u64 size_in_bytes);
int gpt_write_part(PART_ENTRY *part_e, void *buf, u64 offset_in_bytes, u64 size_in_bytes);
int gpt_erase_part(PART_ENTRY *part_e, u64 offset_in_bytes, u64 size_in_bytes);
int gpt_read_raw(void *addr, u32 start_in_secs, u32 *size_in_secs);
void gpt_get_range_by_name(const char *name, u32 *start_in_secs, u32 *size_in_secs);
void gpt_get_range_by_range(u32 *start_in_secs, u32 *size_in_secs);

#endif /* __GPT_H__*/

