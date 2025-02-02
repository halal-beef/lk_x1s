/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <lk/debug.h>
#include <string.h>
#include <stdlib.h>
#include <lk/reg.h>
#include <part.h>
#include <platform/sfr.h>
#include <platform/delay.h>
#include <platform/ab_update.h>
#include <platform/ab_slotinfo.h>
#include <platform/bootloader_message.h>
#include <platform/bl_sys_info.h>

#if defined(CONFIG_AB_UPDATE)
int ab_update_slot_info(void)
{
	void *buf;
	struct bootloader_message_ab *bm;
	ExynosSlotInfo *a, *b, *active, *inactive;
	int ret = 0;
	void *part;

	part = part_get(AB_SLOTINFO_PART_NAME);
	buf = memalign(0x1000, part_get_size_in_bytes(part));
	part_read(part, buf);

	bm = (struct bootloader_message_ab *)buf;
	a = (ExynosSlotInfo *)bm->slot_suffix;
	b = a + 1;

	printf("\n");
	printf("slot information update - start\n");
	printf("_a bootable: %d, is_active %d, boot_successful %d, tries_remaining %d\n",
			a->bootable, a->is_active, a->boot_successful, a->tries_remaining);
	printf("_b bootable: %d, is_active %d, boot_successful %d, tries_remaining %d\n",
			b->bootable, b->is_active, b->boot_successful, b->tries_remaining);
	printf("\n");

	if (memcmp(a->magic, "EXBC", 4) ||
		memcmp(b->magic, "EXBC", 4)) {
		printf("Invalid slot information magic code!\n");
		ret = AB_ERROR_INVALID_MAGIC;
	} else if (a->is_active == 1 && b->is_active == 0) {
		active = a;
		inactive = b;
		if (active->bootable == 1) {
			if (active->bootable == 1 && active->boot_successful == 0) {
				printf("A slot tries_remaining: %d\n", active->tries_remaining);
				if(active->tries_remaining == 0) {
					active->bootable = 0;
					active->is_active = 0;
					if(inactive->bootable == 1) {
						inactive->is_active = 1;
						part_write(part, buf);

						/* Delay for data write HW operation on AB_SLOTINFO_PART partition */
						mdelay(500);
						free(buf);
						/* reset */
						writel(readl(EXYNOS9630_SYSTEM_CONFIGURATION) | 0x2, EXYNOS9630_SYSTEM_CONFIGURATION);
						do {
							asm volatile("wfi");
						} while(1);
					} else {
						ret = AB_ERROR_NO_BOOTABLE_SLOT;
					}
				} else {
					active->tries_remaining--;
				}
			}
		} else {
			ret = AB_ERROR_UNBOOTABLE_SLOT;
		}
	} else if (a->is_active == 0 && b->is_active == 1) {
		active = b;
		inactive = a;
		if (active->bootable == 1) {
			if (active->bootable == 1 && active->boot_successful == 0) {
				printf("B slot tries_remaining: %d\n", active->tries_remaining);
				if(active->tries_remaining == 0) {
					active->bootable = 0;
					active->is_active = 0;
					if(inactive->bootable == 1) {
						inactive->is_active = 1;
						part_write(part, buf);

						/* Delay for data write HW operation on AB_SLOTINFO_PART partition */
						mdelay(500);
						free(buf);
						/* reset */
						writel(readl(EXYNOS9630_SYSTEM_CONFIGURATION) | 0x2, EXYNOS9630_SYSTEM_CONFIGURATION);
						do {
							asm volatile("wfi");
						} while(1);
					} else {
						ret = AB_ERROR_NO_BOOTABLE_SLOT;
					}
				} else {
					active->tries_remaining--;
				}
			}
		} else {
			ret = AB_ERROR_UNBOOTABLE_SLOT;
		}
	} else if (a->is_active == 1 && b->is_active == 1) {
		ret = AB_ERROR_SLOT_ALL_ACTIVE;
	} else {
		ret = AB_ERROR_SLOT_ALL_INACTIVE;
	}

	printf("\n");
	printf("_a bootable: %d, is_active %d, boot_successful %d, tries_remaining %d\n",
			a->bootable, a->is_active, a->boot_successful, a->tries_remaining);
	printf("_b bootable: %d, is_active %d, boot_successful %d, tries_remaining %d\n",
			b->bootable, b->is_active, b->boot_successful, b->tries_remaining);
	printf("slot information update - end\n");
	printf("\n");

	part_write(part, buf);

	free(buf);

	return ret;
}

int ab_set_active_bootloader(int slot, ExynosSlotInfo *si)
{
	int other_slot = 1 - slot;

	(si + slot)->bootable = 1;
	(si + slot)->is_active = 1;
	(si + slot)->boot_successful = 0;
	(si + slot)->tries_remaining = 7;
	memcpy((si + slot)->magic, "EXBC", 4);

	(si + other_slot)->bootable = 0;
	(si + other_slot)->is_active = 0;
	(si + other_slot)->boot_successful = 0;
	(si + other_slot)->tries_remaining = 0;
	memcpy((si + other_slot)->magic, "EXBC", 4);

	printf("\n");
	printf("Current slot(%d) booting failed in bootloader. Changing active slot to %d.\n", other_slot, slot);

	return 0;
}

int ab_update_slot_info_bootloader(void)
{
	void *buf;
	struct bootloader_message_ab *bm;
	ExynosSlotInfo *a, *b;
	struct bl_sys_info *bl_sys = (struct bl_sys_info *)BL_SYS_INFO;
	int ret = 0;
	void *part;

	part = part_get(AB_SLOTINFO_PART_NAME);
	buf = memalign(0x1000, part_get_size_in_bytes(part));
	part_read(part, buf);

	bm = (struct bootloader_message_ab *)buf;
	a = (ExynosSlotInfo *)bm->slot_suffix;
	b = a + 1;

	printf("\n");
	printf("Slot information update when bootloader booting is failed - start\n");
	printf("Before\n");
	printf("_a bootable: %d, is_active %d, boot_successful %d, tries_remaining %d\n",
			a->bootable, a->is_active, a->boot_successful, a->tries_remaining);
	printf("_b bootable: %d, is_active %d, boot_successful %d, tries_remaining %d\n",
			b->bootable, b->is_active, b->boot_successful, b->tries_remaining);
	printf("\n");

	if (memcmp(a->magic, "EXBC", 4) ||
		memcmp(b->magic, "EXBC", 4)) {
		printf("Invalid slot information magic code!\n");
		ret = AB_ERROR_INVALID_MAGIC;
	} else if (a->is_active == 1 && b->is_active == 0) {
		if (bl_sys->bl1_info.epbl_start !=
				part_get_start_in_blks(part_get("bootloader_a")))
			ab_set_active_bootloader(1, a);
	} else if (a->is_active == 0 && b->is_active == 1) {
		if (bl_sys->bl1_info.epbl_start !=
				part_get_start_in_blks(part_get("bootloader_b")))
			ab_set_active_bootloader(0, a);
	} else if (a->is_active == 1 && b->is_active == 1) {
		ret = AB_ERROR_SLOT_ALL_ACTIVE;
	} else {
		ret = AB_ERROR_SLOT_ALL_INACTIVE;
	}

	printf("\n");
	printf("After\n");
	printf("_a bootable: %d, is_active %d, boot_successful %d, tries_remaining %d\n",
			a->bootable, a->is_active, a->boot_successful, a->tries_remaining);
	printf("_b bootable: %d, is_active %d, boot_successful %d, tries_remaining %d\n",
			b->bootable, b->is_active, b->boot_successful, b->tries_remaining);
	printf("Slot information update when bootloader booting is failed - end\n");
	printf("\n");

	part_write(part, buf);

	free(buf);

	return ret;
}

int ab_set_active(int slot)
{
	int other_slot = 1 - slot;
	void *buf;
	struct bootloader_message_ab *bm;
	ExynosSlotInfo *si;
	void *part;

	part = part_get(AB_SLOTINFO_PART_NAME);
	buf = memalign(0x1000, part_get_size_in_bytes(part));
	part_read(part, buf);

	bm = (struct bootloader_message_ab *)buf;
	si = (ExynosSlotInfo *)bm->slot_suffix;

	(si + slot)->bootable = 1;
	(si + slot)->is_active = 1;
	(si + slot)->boot_successful = 0;
	(si + slot)->tries_remaining = 7;
	memcpy((si + slot)->magic, "EXBC", 4);

	(si + other_slot)->bootable = 1;
	(si + other_slot)->is_active = 0;
	(si + other_slot)->boot_successful = 0;
	(si + other_slot)->tries_remaining = 7;
	memcpy((si + other_slot)->magic, "EXBC", 4);

	printf("\n");
	printf("_a bootable: %d, is_active %d, boot_successful %d, tries_remaining %d\n",
			(si + 0)->bootable, (si + 0)->is_active, (si + 0)->boot_successful, (si + 0)->tries_remaining);
	printf("_b bootable: %d, is_active %d, boot_successful %d, tries_remaining %d\n",
			(si + 1)->bootable, (si + 1)->is_active, (si + 1)->boot_successful, (si + 1)->tries_remaining);

	part_write(part, buf);

	free(buf);

	return 0;
}

int ab_current_slot(void)
{
	void *buf;
	struct bootloader_message_ab *bm;
	ExynosSlotInfo *a;
	int ret;
	void *part;

	part = part_get(AB_SLOTINFO_PART_NAME);
	buf = memalign(0x1000, part_get_size_in_bytes(part));
	part_read(part, buf);

	bm = (struct bootloader_message_ab *)buf;
	a = (ExynosSlotInfo *)bm->slot_suffix;

	ret = a->is_active == 1 ? 0 : 1;

	free(buf);

	return ret;
}

int ab_slot_successful(int slot)
{
	void *buf;
	struct bootloader_message_ab *bm;
	ExynosSlotInfo *si;
	int ret;
	void *part;

	part = part_get(AB_SLOTINFO_PART_NAME);
	buf = memalign(0x1000, part_get_size_in_bytes(part));
	part_read(part, buf);

	bm = (struct bootloader_message_ab *)buf;
	si = (ExynosSlotInfo *)bm->slot_suffix;

	ret = (si + slot)->boot_successful;

	free(buf);

	return ret;
}

int ab_slot_unbootable(int slot)
{
	void *buf;
	struct bootloader_message_ab *bm;
	ExynosSlotInfo *si;
	int ret;
	void *part;

	part = part_get(AB_SLOTINFO_PART_NAME);
	buf = memalign(0x1000, part_get_size_in_bytes(part));
	part_read(part, buf);

	bm = (struct bootloader_message_ab *)buf;
	si = (ExynosSlotInfo *)bm->slot_suffix;

	ret = (si + slot)->bootable ? 0 : 1;

	free(buf);

	return ret;
}

int ab_slot_retry_count(int slot)
{
	void *buf;
	struct bootloader_message_ab *bm;
	ExynosSlotInfo *si;
	int ret;
	void *part;

	part = part_get(AB_SLOTINFO_PART_NAME);
	buf = memalign(0x1000, part_get_size_in_bytes(part));
	part_read(part, buf);

	bm = (struct bootloader_message_ab *)buf;
	si = (ExynosSlotInfo *)bm->slot_suffix;

	ret = (si + slot)->tries_remaining;

	free(buf);

	return ret;
}
int ab_update_support(void)
{
	return 1;
}
#else
int ab_update_slot_info(void)
{
	printf("%s: NO AB update support\n", __func__);
	return AB_ERROR_NOT_SUPPORT;
}

int ab_update_slot_info_bootloader(void)
{
	printf("%s: NO AB update support\n", __func__);
	return AB_ERROR_NOT_SUPPORT;
}

int ab_set_active(int slot)
{
	printf("%s: NO AB update support\n", __func__);
	return AB_ERROR_NOT_SUPPORT;
}

int ab_current_slot(void)
{
	printf("%s: NO AB update support\n", __func__);
	return AB_ERROR_NOT_SUPPORT;
}

int ab_slot_successful(int slot)
{
	printf("%s: NO AB update support\n", __func__);
	return AB_ERROR_NOT_SUPPORT;
}

int ab_slot_unbootable(int slot)
{
	printf("%s: NO AB update support\n", __func__);
	return AB_ERROR_NOT_SUPPORT;
}

int ab_slot_retry_count(int slot)
{
	printf("%s: NO AB update support\n", __func__);
	return AB_ERROR_NOT_SUPPORT;
}
int ab_update_support(void)
{
	return 0;
}
#endif
