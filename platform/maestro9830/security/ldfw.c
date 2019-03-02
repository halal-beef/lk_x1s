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
#include <platform/smc.h>

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

static u32 get_boot_device_info(void)
{
	u32 boot_device_info;

	if (*(unsigned int *)DRAM_BASE != 0xabcdef) {
		/* Running on DRAM by TRACE32 */
		boot_device_info = 0x0;
	} else {
		boot_device_info = find_second_boot();
	}

	return boot_device_info;
}

static int is_usb_boot(void)
{
	u32 order = 0;
	u32 boot_device_info = 0;

	boot_device_info = get_boot_device_info();
	if (!boot_device_info) {
		/* boot from T32 */
		return 0;
	}

	if ((boot_device_info & 0xFF000000) != 0xCB000000) {
		/* abnormal boot */
		while (1) ;
	}

	order = boot_device_info & 0xF;
	switch ((boot_device_info >> (4 * order)) & 0xF) {
	case BD_USB:
		return 1;
		break;
	default:
		break;
	}
	return 0;
}


static int load_partition(u64 addr, u64 ch, u64 *size)
{
	int OmPin = readl(EXYNOS9830_POWER_INFORM3);
	int ret;
	struct pit_entry *ptn;
	char ch_name[][20] = { "ldfw", "keystorage", "ssp", "tzsw" };

	if (ch >= MAX_CH_NUM) {
		LDFW_ERR("Invalid ch\n");
		return -1;
	}

	if (ch == LDFW_PART) {
		ptn = pit_get_part_info("ldfw");
	} else if (ch == KEYSTORAGE_PART) {
		ptn = pit_get_part_info("keystorage");
	} else if (ch == TZSW_PART) {
		ptn = pit_get_part_info("tzsw");
	} else {
		printf("Invalid ch\n");
		return -1;
	}

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
		LDFW_INFO("%s: This booting device is not supported.\n", ch_name[ch]);
		return -1;
	}

	if (ptn)
		ret = !pit_access(ptn, PIT_OP_LOAD, addr, 0);
	else
		ret = 0;

	if (ret != 1) {
		LDFW_ERR("%s: there is no ldfw partition\n", ch_name[ch]);

	} else {
		*size = pit_get_length(ptn);
		LDFW_INFO("%s: read whole partition from the storage\n", ch_name[ch]);
	}

	return ret == 1 ? 0:-1;
}

int init_keystorage(void)
{
	int ret = 0;
	u64 addr = EXYNOS9610_KEYSTORAGE_NWD_ADDR;
	u64 size = EXYNOS9610_KEYSTORAGE_PARTITION_SIZE;
	SB_KEYSTORAGE_HEADER *header = NULL;

	if (is_usb_boot()) {
		/* boot from iROM USB booting */
		return 1;
	}

	if (load_partition(addr, KEYSTORAGE_PART, &size)) {
		LDFW_ERR("keystorage: can not read keystorage from the storage\n");
		return -1;
	}

	if (!size) {
		LDFW_ERR("keystorage: partition size is invalid\n");
	}

	header = (SB_KEYSTORAGE_HEADER *)addr;
	if (header->magic != EXYNOS_AP_MAGIC) {
		LDFW_ERR("keystorage: Invalid binary magic\n");
		return -1;
	}

	ret = load_keystorage(addr, size);

	if (ret == -1)
		LDFW_INFO("keystorage: It is dump_gpr state. It does not load keystorage.\n");
	else if (ret == 0)
		LDFW_INFO("keystorage: It is successfully loaded.\n");
	else
		LDFW_INFO("keystorage: [SB_ERR] ret = [0x%X]\n", ret);

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

	if (is_usb_boot()) {
		/* boot from iROM USB booting */
		ret =  init_ldfw_by_usb(addr, size);
		if (ret) {
			LDFW_INFO("spayload: read Spayload from USB with error # 0x%llx\n", (u64)ret);
			return ret;
		}
	} else if (load_partition(addr, LDFW_PART, &size)) {
		LDFW_ERR("ldfw: can not read ldfw from the storage\n");
		return -1;
	}

	fwh = (struct fw_header *)addr;
	if (fwh->magic != EXYNOS_LDFW_MAGIC) {
		/* There are no LDFWs */
		LDFW_ERR("ldfw: there is no ldfw at ldfw partition\n");
		return -1;
	} else {
		for (i = 0, size = 0;; i++) {
			if (fwh->magic != EXYNOS_LDFW_MAGIC)
				break;
			strncpy(name, fwh->fw_name, 16);
			LDFW_INFO("ldfw: %dth ldfw's version 0x%x name : %s\n",
				i, fwh->version, name);

			/* to calculate real ldfw size */
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

int init_sp(void)
{
	s64 ret = 0;
	u64 addr = EXYNOS9610_KEYSTORAGE_NWD_ADDR;
	u64 size = 0x100000; /* default size 1MB */

	if (is_usb_boot()) {
		/* boot from iROM USB booting */
		ret =  load_sp_by_usb(addr, size);
		if (ret) {
			LDFW_INFO("spayload: read Spayload from USB with error # 0x%llx\n", ret);
			return ret;
		}
	} else if (load_partition(addr, TZSW_PART, &size)) {
		LDFW_ERR("spayload: can not read spayload from the storage\n");
		return -1;
	}

	if (!size) {
		LDFW_ERR("spayload: secure payload partition size is not valid.\n");
		return -1;
	}

	ret = (s64)load_sp(addr, size);
	if (ret == -1)
		LDFW_INFO("spayload: It is dump_gpr state. It does not load spayload.\n");
	else if (ret == 0)
		LDFW_INFO("spayload: It is successfully loaded.\n");
	else
		LDFW_INFO("spayload: [ERR] ret = [0x%llX]\n", ret);

	return ret;
}
