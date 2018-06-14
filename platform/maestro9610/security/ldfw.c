/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <reg.h>
#include <pit.h>
#include <string.h>
#include <sys/types.h>
#include <dev/boot.h>
#include <platform/sfr.h>
#include <platform/ldfw.h>

int init_ldfw(u64 addr, u64 size);
int load_keystorage(u64 addr, u64 size);

struct fw_header {
	unsigned int magic;  /* Should be ‘0x10adab1e’ */
	unsigned int size;
	unsigned int init_entry;
	unsigned int entry_point;
	unsigned int suspend_entry;
	unsigned int resume_entry;
	unsigned int start_smc_id;
	unsigned int version;
	unsigned int set_runtime_entry;
	unsigned int reserved1;
	unsigned int reserved2;
	unsigned int reserved3;
	char fw_name[16];
};

typedef struct
{
	uint32_t magic;
	uint32_t version;
	uint32_t header_len;
	uint32_t key_count;
	uint64_t build_date;
	uint8_t author[8];
	uint32_t body_len;
	uint32_t reserved[3];
} SB_KEYSTORAGE_HEADER;

int load_parition(u64 addr, u64 ch)
{
	int OmPin = readl(EXYNOS9610_POWER_INFORM3);
	int ret;
	struct pit_entry *ptn = pit_get_part_info("ldfw");

	if (ch >= MAX_CH_NUM) {
		printf("invalid channel loading\n");
		return -1;
	}

	if (OmPin == BOOT_UFS) {
		if (ch == LDFW_PART)
			LDFW_INFO("ldfw: booting device is UFS.\n");

	} else if (OmPin == BOOT_MMCSD || OmPin == BOOT_EMMC || \
			OmPin == BOOT_EMMC) {
		if (ch == LDFW_PART)
			LDFW_INFO("ldfw: booting device is eMMC.\n");

	} else {
		/* LDFW should be on eMMC or UFS */
		if (ch == LDFW_PART)
			LDFW_INFO("ldfw: This booting device is not supported.\n");
		else if (ch == KEYSTORAGE_PART)
			printf("keystorage: This booting device is not supported.\n");
		return -1;
	}

	if (ch == LDFW_PART)
		LDFW_INFO("ldfw: read whole ldfw partition from the storage\n");
	else if (ch == KEYSTORAGE_PART)
		printf("read whole keystorage partition from the storage\n");

	if (ptn)
		ret = !pit_access(ptn, PIT_OP_LOAD, addr, 0);
	else
		ret = 0;

	if (ret != 1) {
		if (ch == LDFW_PART)
			LDFW_ERR("ldfw: there is no ldfw partition\n");
		else if (ch == KEYSTORAGE_PART)
			printf("keystorage: there is no keystorage partition\n");
	}

	return ret == 1 ? 0:-1;
}

int init_keystorage(void)
{
	u32 ret = 0;
	u64 addr = EXYNOS9610_KEYSTORAGE_NWD_ADDR;
	u64 size = EXYNOS9610_KEYSTORAGE_PARTITION_SIZE;
	SB_KEYSTORAGE_HEADER *header = NULL;

	if (load_parition(addr, KEYSTORAGE_PART)) {
		printf("keystorage: can not read keystorage from the storage\n");
		return -1;
	}

	header = (SB_KEYSTORAGE_HEADER *)addr;
	if (header->magic != EXYNOS_AP_MAGIC) {
		printf("keystorage: Invalid binary magic\n");
		return -1;
	}

	ret = load_keystorage(addr, size);
	if (ret == -1)
		printf("keystorage: It is dump_gpr state. It does not load keystorage.\n");
	else if (ret == 0)
		printf("keystorage: It is successfully loaded.\n");
	else
		printf("keystorage: [SB_ERR] ret = [0x%X]\n", ret);

	return ret;
}

int init_ldfws(void)
{
	u64 addr = EXYNOS9610_LDFW_NWD_ADDR;
	u64 size = 0;
	struct fw_header *fwh;
	s64 ret;
	u32 try, try_fail, i;
	char name[17] = {0,};

	if (load_parition(addr, LDFW_PART)) {
		LDFW_ERR("ldfw: can not read ldfw from the storage\n");
		return -1;
	}

	fwh = (struct fw_header *)addr;
	if (fwh->magic != EXYNOS_LDFW_MAGIC) {
		/* there are no ldfw */
		LDFW_ERR("ldfw: there is no ldfw at ldfw partition\n");
		return -1;
	} else {
		for(i=0;;i++) {
			if (fwh->magic != EXYNOS_LDFW_MAGIC)
				break;
			strncpy(name, fwh->fw_name, 16);
			LDFW_INFO("ldfw: %dth ldfw's version 0x%x name : %s\n",
				i, fwh->version, name);
			size += (u64)fwh->size;
			fwh = (struct fw_header *)((u64)fwh + (u64)fwh->size);
		}
	}

	LDFW_INFO("ldfw: init ldfw(s). whole ldfws size 0x%llx\n", size);
	ret = init_ldfw(addr, size);

	if (ret == -1)
		LDFW_ERR("ldfw: It is dump_gpr state. It does not load ldfw.\n");
	else if ((ret < 0) && (ret & CHECK_SIGNATURE_FAIL))
		LDFW_ERR("ldfw: signature of ldfw is corrupted.!\n");
	else if ((ret < 0) && (ret & CHECK_ROLL_BACK_COUNT_FAIL))
		LDFW_ERR("ldfw: roll back count of ldfw is corrupted.!\n");
	else if (!ret)
		LDFW_WARN("ldfw: No ldfw is inited\n");
	else {
		try = ret & 0xffff;
		try_fail = (ret >> 16) & 0xffff;
		LDFW_INFO("ldfw: try to init %d ldfw(s). except %d ldfw " \
			"%d ldfw(s) have been inited done.\n", \
				try, try_fail, try - try_fail);
	}

	return 0;
}
