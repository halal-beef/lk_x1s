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

#ifndef __PART_H__
#define __PART_H__

#include <part_dev.h>
#if INPUT_GPT_AS_PT
#include <gpt.h>
#define PART_SECTOR_SIZE	SECTOR_SIZE
#else
#include <pit.h>
#include <part_gpt.h>
#define PART_SECTOR_SIZE	PIT_SECTOR_SIZE
#endif

#undef PART_DEBUG
//#define PART_DEBUG

void *part_get(const char *name);
void *part_get_ab(const char *name);
u64 part_get_size_in_bytes(void *part);
u32 part_get_start_in_blks(void *part);
u32 part_get_start_in_secs(void *part);
const char *part_get_fs_type(void *part);
int part_get_unique_guid(void *part, char *name, char *buf);
void part_write(void *part, void *buf);
int part_write_partial(void *part, void *buf, u64 offset, u64 size);
int part_read(void *part, void *buf);
int part_read_partial(void *part, void *buf, u64 offset, u64 size);
int part_erase(void *part);
int part_get_pt_type(const char *name);
void part_update(void *addr, u32 size);
void part_show(void);
void part_set_def_dev(enum __boot_dev_id id);
enum __boot_dev_id part_get_dev(void);
void part_init(void);
int part_read_raw(void *addr, u32 start_in_secs, u32 *size_in_secs);
void part_get_range_by_name(const char *name, u32 *start_in_secs, u32 *size_in_secs);
void part_get_range_by_range(u32 *start_in_secs, u32 *size_in_secs);
u32 part_get_lun(void *part);
int part_wipe_boot(void);
u32 part_get_block_size(void);
u32 part_get_erase_size(void);

#endif /* __PART_H__ */
