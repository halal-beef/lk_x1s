/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <lk/reg.h>
#include <stdio.h>
#include <part.h>
#include <string.h>
#include <sys/types.h>
#include <dev/boot.h>
#include <platform/sfr.h>
#include <platform/ldfw.h>
#include <platform/ab_update.h>
#include <platform/smc.h>
#include <platform/secure_boot.h>

int init_ldfw(u64 addr, u64 size);
int load_keystorage(u64 addr, u64 size);
int load_ssp(u64 addr, u64 size);

static u32 ldfw_loaded;
static u32 secure_os_loaded;

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

typedef struct {
	uint32_t magic;
	uint32_t version;
	uint32_t header_len;
	uint32_t key_count;
	uint64_t build_date;
	uint8_t author[8];
	uint32_t body_len;
	uint32_t reserved[3];
} SB_KEYSTORAGE_HEADER;

enum _bin_list_by_usb {
	SSP_DN_BY_USB,
	LDFW_DN_BY_USB,
	SP_DN_BY_USB
};

static u32 get_boot_device_info(void)
{
	u32 boot_device_info;

	if (*(unsigned int *)DRAM_BASE != 0xabcdef)
		/* Running on DRAM by TRACE32 */
		boot_device_info = 0x0;
	else
		boot_device_info = find_second_boot();

	return boot_device_info;
}

static int is_usb_boot(void)
{
	u32 order = 0;
	u32 boot_device_info = 0;

	boot_device_info = get_boot_device_info();
	if (!boot_device_info)
		/* boot from T32 */
		return 0;

	if ((boot_device_info & 0xFF000000) != 0xCB000000)
		/* abnormal boot */
		while (1)
			;

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
	int OmPin = readl(EXYNOS9630_POWER_INFORM3);
	int ret;
	void *part;
	char ch_name[][20] = { "ldfw", "keystorage", "ssp", "tzsw" };

	if (ch >= MAX_CH_NUM) {
		LDFW_ERR("Invalid ch\n");
		return -1;
	}

	if (ch == LDFW_PART) {
		part = part_get_ab("ldfw");
	} else if (ch == KEYSTORAGE_PART) {
		part = part_get_ab("keystorage");
	} else if (ch == TZSW_PART) {
		part = part_get("tzsw");
	} else if (ch == SSP_PART) {
		part = part_get("ssp");
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

	if (part)
		ret = !part_read(part, (void *)addr);
	else
		ret = 0;

	if (ret != 1) {
		LDFW_ERR("%s: there is no ldfw partition\n", ch_name[ch]);
	} else {
		*size = part_get_size_in_bytes(part);
		LDFW_INFO("%s: read whole partition from the storage\n", ch_name[ch]);
	}

	return ret == 1 ? 0 : -1;
}

int init_keystorage(void)
{
	s32 ret = 0;
	u64 addr = EXYNOS9630_KEYSTORAGE_NWD_ADDR;
	u64 size = EXYNOS9630_KEYSTORAGE_PARTITION_SIZE;
	SB_KEYSTORAGE_HEADER *header = NULL;

	if (is_usb_boot())
		/* boot from iROM USB booting */
		return 1;

	if (load_partition(addr, KEYSTORAGE_PART, &size)) {
		LDFW_ERR("keystorage: can not read keystorage from the storage\n");
		return -1;
	}

	if (!size)
		LDFW_ERR("keystorage: partition size is invalid\n");

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

int init_ssp(void)
{
	int ret = 0;
	u64 addr = EXYNOS9630_SSP_NWD_ADDR;
	u64 size = EXYNOS9630_SSP_PARTITION_SIZE;

	if (is_usb_boot())
		/* boot from iROM USB booting */
		return 1;

	if (load_partition(addr, SSP_PART, &size)) {
		LDFW_ERR("ssp: can not read ssp from the storage\n");
		return -1;
	}

	if (!size)
		LDFW_ERR("ssp: partition size is invalid\n");

	ret = load_ssp(addr, size);
	if (ret == 0)
		LDFW_INFO("ssp: It is successfully loaded.\n");
	else
		LDFW_INFO("ssp: loading fail, ret = [0x%X]\n", ret);

	return ret;
}

int init_ldfws(void)
{
	u64 addr = EXYNOS9630_LDFW_NWD_ADDR;
	u64 size = EXYNOS9630_LDFW_PARTITION_SIZE;
	struct fw_header *fwh;
	s64 ret;
	u32 try, try_fail, i;
	char name[17] = { 0, };

	if (is_usb_boot()) {
		/* boot from iROM USB booting */
		ret =  load_image_by_usb(LDFW_DN_BY_USB, addr, size);
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

	if (ret == -1 || ret == SDM_HW_RESET_STATUS) {
		LDFW_ERR("ldfw: It is dump_gpr state. It does not load ldfw.\n");
	} else if (ret == SDM_SW_RESET_STATUS) {
		LDFW_ERR("ldfw: It is kernel panic(SW RESET) state. It does not load ldfw.\n");
	} else if ((ret < 0) && ((ret & 0xffff0000) == SB_ERROR_PREFIX)) {
		LDFW_ERR("ldfw: signature of ldfw is corrupted! ret = [0x%X]\n", (uint32_t)ret);
	} else if (!ret) {
		LDFW_WARN("ldfw: No ldfw is inited\n");
	} else {
		try = ret & 0xffff;
		try_fail = (ret >> 16) & 0xffff;
		LDFW_INFO("ldfw: try to init %d ldfw(s). except %d ldfw " \
		          "%d ldfw(s) have been inited done.\n", \
		          try, try_fail, try - try_fail);
		ldfw_loaded = 1;
		ret = 0x0;
	}

	return ret;
}

int init_sp(void)
{
	s64 ret = 0;
	u64 addr = EXYNOS9630_LDFW_NWD_ADDR;
	u64 size = EXYNOS9630_SP_PARTITION_SIZE; /* default size 1MB */

	if (is_usb_boot()) {
		/* boot from iROM USB booting */
		ret =  load_image_by_usb(SP_DN_BY_USB, addr, size);
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
	if (ret == -1 || ret == SDM_HW_RESET_STATUS) {
		LDFW_INFO("spayload: It is dump_gpr state. It does not load spayload.\n");
	} else if (ret == SDM_SW_RESET_STATUS) {
		LDFW_ERR("spayload: It is kernel panic(SW RESET) state. It does not load spayload.\n");
	} else if (ret == 0) {
		LDFW_INFO("spayload: It is successfully loaded.\n");
		secure_os_loaded = 1;
	} else {
		LDFW_INFO("spayload: [ERR] ret = [0x%llX]\n", ret);
	}

	return ret;
}

u32 get_ldfw_load_flag(void)
{
	return ldfw_loaded;
}

u32 get_secure_os_load_flag(void)
{
	return secure_os_loaded;
}
