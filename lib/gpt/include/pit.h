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

#include <types.h>
#include <sys/types.h>
#include <lib/bio.h>
#include <pit_format.h>
#include <part_dev.h>

#undef	PIT_DEBUG
#define PIT_DEBUG

#define PIT_GPT_STRING_SIZE		0x2000
#define PIT_EMMC_ERASE_SIZE		(1024)

#define PIT_ENV_SIZE			(16 << 10)

enum pit_op {
	PIT_OP_FLASH = 0,
	PIT_OP_ERASE,
	PIT_OP_LOAD,
};

/* Struct for gpt information */
struct gpt_info {
	__le32 gpt_start_lba; /* The unit is 512B */
	__le32 gpt_last_lba; /* The unit is 512B, usable lba limitation */
	__le32 gpt_entry_cnt;
};

/*
 * Public functions
 */
void pit_init(enum __boot_dev_id id);
void pit_show_info(void);
int pit_update(void *buf, unsigned int size);
struct pit_entry *pit_get_part_info(const char *name);
int pit_access(struct pit_entry *ptn, int op, u64 addr, u32 size);
u64 pit_get_length(struct pit_entry *ptn);
int pit_entry_write(struct pit_entry *ptn, void *buf, u64 offset, u64 size);
int pit_entry_read(struct pit_entry *ptn, void *buf, u64 offset, u64 size);
int pit_entry_erase(struct pit_entry *ptn, u64 offset, u64 size);


/*
 * Exported functions
 */
// TODO:
bdev_t *pit_get_boot_device(char *);

#endif
