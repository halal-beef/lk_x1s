/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <lk/debug.h>
#include <stdlib.h>
#include <pit.h>
#include <lib/ab_update.h>
#include <lib/bootloader_message.h>

int ab_set_active(int slot)
{
	int other_slot = 1 - slot;
	void *buf;
	struct bootloader_message_ab *bm;
	struct pit_entry *ptn;
	ExynosBootInfo *bi;
	ExynosSlotInfo *si;

	ptn = pit_get_part_info("misc");
	buf = memalign(0x1000, pit_get_length(ptn));
	pit_access(ptn, PIT_OP_LOAD, (u64)buf, 0);

	bm = (struct bootloader_message_ab *)buf;
	bi = (ExynosBootInfo *)bm->slot_suffix;
	si = (ExynosSlotInfo *)bi->slot_info;

	if ((si + other_slot)->priority == 15)
		(si + other_slot)->priority = 14;
	(si + slot)->bootable = 1;
	(si + slot)->priority = 15;
	(si + slot)->tries_remaining = 7;
	(si + slot)->boot_successful = 0;

	printf("\n");
	printf("_a bootable: %d, priority %d, tries_remaining %d, boot_successful %d\n",
	       (si + 0)->bootable, (si + 0)->priority, (si + 0)->tries_remaining, (si + 0)->boot_successful);
	printf("_b bootable: %d, priority %d, tries_remaining %d, boot_successful %d\n",
	       (si + 1)->bootable, (si + 1)->priority, (si + 1)->tries_remaining, (si + 1)->boot_successful);

	pit_access(ptn, PIT_OP_FLASH, (u64)buf, 0);

	free(buf);

	return 0;
}

int ab_current_slot(void)
{
	void *buf;
	struct bootloader_message_ab *bm;
	struct pit_entry *ptn;
	ExynosBootInfo *bi;
	ExynosSlotInfo *si;
	int ret;

	ptn = pit_get_part_info("misc");
	buf = memalign(0x1000, pit_get_length(ptn));
	pit_access(ptn, PIT_OP_LOAD, (u64)buf, 0);

	bm = (struct bootloader_message_ab *)buf;
	bi = (ExynosBootInfo *)bm->slot_suffix;
	si = (ExynosSlotInfo *)bi->slot_info;

	ret = ((si + 0)->priority > (si + 1)->priority) ? 0 : 1;

	free(buf);

	return ret;
}

int ab_slot_successful(int slot)
{
	void *buf;
	struct bootloader_message_ab *bm;
	struct pit_entry *ptn;
	ExynosBootInfo *bi;
	ExynosSlotInfo *si;
	int ret;

	ptn = pit_get_part_info("misc");
	buf = memalign(0x1000, pit_get_length(ptn));
	pit_access(ptn, PIT_OP_LOAD, (u64)buf, 0);

	bm = (struct bootloader_message_ab *)buf;
	bi = (ExynosBootInfo *)bm->slot_suffix;
	si = (ExynosSlotInfo *)bi->slot_info;

	ret = (si + slot)->boot_successful;

	free(buf);

	return ret;
}

int ab_slot_unbootable(int slot)
{
	void *buf;
	struct bootloader_message_ab *bm;
	struct pit_entry *ptn;
	ExynosBootInfo *bi;
	ExynosSlotInfo *si;
	int ret;

	ptn = pit_get_part_info("misc");
	buf = memalign(0x1000, pit_get_length(ptn));
	pit_access(ptn, PIT_OP_LOAD, (u64)buf, 0);

	bm = (struct bootloader_message_ab *)buf;
	bi = (ExynosBootInfo *)bm->slot_suffix;
	si = (ExynosSlotInfo *)bi->slot_info;

	ret = (si + slot)->bootable ? 0 : 1;

	free(buf);

	return ret;
}

int ab_slot_retry_count(int slot)
{
	void *buf;
	struct bootloader_message_ab *bm;
	struct pit_entry *ptn;
	ExynosBootInfo *bi;
	ExynosSlotInfo *si;
	int ret;

	ptn = pit_get_part_info("misc");
	buf = memalign(0x1000, pit_get_length(ptn));
	pit_access(ptn, PIT_OP_LOAD, (u64)buf, 0);

	bm = (struct bootloader_message_ab *)buf;
	bi = (ExynosBootInfo *)bm->slot_suffix;
	si = (ExynosSlotInfo *)bi->slot_info;

	ret = (si + slot)->tries_remaining;

	free(buf);

	return ret;
}
