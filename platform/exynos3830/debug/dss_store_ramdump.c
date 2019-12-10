/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <string.h>
#include <stdlib.h>
#include <part.h>
#include <lib/console.h>
#include <lib/font_display.h>
#include <platform/sizes.h>
#include <platform/dfd.h>
#include <platform/delay.h>
#include <platform/dss_store_ramdump.h>
#include <platform/fastboot.h>
#include <platform/gpio.h>
#include <target/pmic.h>
#include <platform/mmu/mmu_func.h>
#include "../fastboot/fastboot.h"

#ifdef CONFIG_OFFLINE_RAMDUMP
static int g_is_enabled = 1;
#else
static int g_is_enabled = 0;
#endif

union store_ramdump_metadata metadata;

static void wfi(void)
{
	asm volatile("wfi");
}

static int debug_store_is_skip(void)
{
	struct exynos_gpio_bank *bank = (struct exynos_gpio_bank *)EXYNOS3830_GPA1CON;
	int gpio = 0;	/* Volume Down */
	int ret = 0;
	int cnt = 100;

	/* Volume up set Input & Pull NONE */
	exynos_gpio_set_pull(bank, gpio, GPIO_PULL_NONE);
	exynos_gpio_cfg_pin(bank, gpio, GPIO_INPUT);

	print_lcd_update(FONT_GREEN, FONT_BLACK, "After 10 seconds, store ramdump");
	print_lcd_update(FONT_GREEN, FONT_BLACK, "press Vol-Down to skip store ramdump");

	do {
		u_delay(100000);
		if (!exynos_gpio_get_value(bank, gpio)) {
			ret = -1;
			break;
		}
	} while(--cnt);

	return ret;
}

int debug_store_ramdump(void)
{
	void *part;
	u64 dram_size;
	u64 dram_write_size;
	u64 dram_ptr;
	u32 reboot_reason;
	int ret = 0;

	if (!g_is_enabled) {
		printf("%s: Store Ramdump is off\n", __func__);
		goto store_out;
	}

	reboot_reason = readl(CONFIG_RAMDUMP_REASON);
	if (reboot_reason == RAMDUMP_SIGN_BL_REBOOT) {
		printf("%s: Bootloader Reboot, Skip saving remdump\n", __func__);

		/* reset reboot reason */
		writel(0, CONFIG_RAMDUMP_REASON);
		goto store_out;
	}

	if (debug_store_is_skip()) {
		print_lcd_update(FONT_GREEN, FONT_BLACK, "Skip Saving Ramdump");
		printf("%s: Volume down key is pressed!\n", __func__);
		goto store_out;
	}

	part = part_get("ramdump");
	if (!part) {
		printf("%s: no ramdump partition\n", __func__);
		ret = -1;
		goto store_out;
	}

	dram_size = *(u64 *)BL_SYS_INFO_DRAM_SIZE;
	printf("%s: dram size is [0x%llx]\n", __func__, dram_size);

	ret = part_read_partial(part, (void *)&metadata, (u64)METADATA_OFFSET, (u64)METADATA_SIZE);
	if (ret) {
		printf("%s: part read fail(line:%u)\n", __func__, __LINE__);
		goto store_out;
	}

	if (metadata.data.magic == RAMDUMP_STORE_MAGIC)
		printf("%s: ramdump data will be overwritten[%x]\n", __func__, metadata.data.magic);

	print_lcd_update(FONT_GREEN, FONT_BLACK, "WAIT for storing ramdump...");

	if (dram_size > DRAM_WRITE_SIZE_DEFAULT) {
		dram_write_size = DRAM_WRITE_SIZE_DEFAULT;
		dram_ptr = DRAM_BASE;
		ret = part_write_partial(part, (void *)dram_ptr, RAMDUMP_OFFSET, dram_write_size);
		if (ret) {
			printf("%s: part write fail(line:%u)\n", __func__, __LINE__);
			goto store_out;
		}
#ifdef DRAM_BASE2
		dram_write_size = dram_size - DRAM_WRITE_SIZE_DEFAULT;
		dram_ptr = DRAM_BASE2;
		ret = part_write_partial(part, (void *)dram_ptr,
				RAMDUMP_OFFSET + DRAM_WRITE_SIZE_DEFAULT, dram_write_size);
		if (ret) {
			printf("%s: part write fail(line:%u)\n", __func__, __LINE__);
			goto store_out;
		}
#endif
	} else {
		dram_write_size = dram_size;
		dram_ptr = DRAM_BASE;
		ret = part_write_partial(part, (void *)dram_ptr, RAMDUMP_OFFSET, dram_write_size);
		if (ret) {
			printf("%s: part write fail(line:%u)\n", __func__, __LINE__);
			goto store_out;
		}
	}

	metadata.data.magic = RAMDUMP_STORE_MAGIC;
	metadata.data.flag_data_to_storage = 0xffffffff;
	metadata.data.dram_size = dram_size;
	metadata.data.dram_start_addr = DRAM_BASE;
	get_pmic_rtc_time(metadata.data.file_name);
	printf("%s: get_pmic_rtc_time = [%s]\n", __func__,
					metadata.data.file_name);

	ret = part_write_partial(part, &metadata, METADATA_OFFSET, METADATA_SIZE);
	if (ret) {
		printf("%s: part write fail(line:%u)\n", __func__, __LINE__);
		goto store_out;
	}

	print_lcd_update(FONT_GREEN, FONT_BLACK, "Finish storing ramdump!");
	printf("%s: Finish storing ramdump!\n", __func__);
	printf("%s: Wait for 1 second for storing ramdump data\n", __func__);
	u_delay(1000000);

#ifdef DEBUG_STORE_RAMDUMP_TEST
	goto store_out;
#endif

	/* Reset device for normal booting */
	writel(0, CONFIG_RAMDUMP_SCRATCH);
	writel(readl(EXYNOS3830_SYSTEM_CONFIGURATION) | 0x2, EXYNOS3830_SYSTEM_CONFIGURATION);

	/* Do not run this code */
	do {
		wfi();
	} while(1);

store_out:
	return ret;
}

int debug_store_ramdump_redirection(void *ptr)
{
#ifdef DEBUG_STORE_RAMDUMP_TEST
	void *part;
	struct fastboot_ramdump_hdr *hdr = ptr;
	u64 storage_base;
	u64 dram_size;
	u64 possible_size = 0;
	u64 redirection_base = 0;
	int ret = 0;

	if (!g_is_enabled)
		goto redirection_out;

	part= part_get("ramdump");
	if (!part) {
		printf("%s: no ramdump partition\n", __func__);
		printf("%s: current ram data will be extracted\n", __func__);
		goto redirection_out;
	}

	printf("%s: do ramdump redirection base[0x%lx] size[%lx]\n",
					__func__,  hdr->base, hdr->size);

	/* Set possible redirection base and dump size */
	dram_size = *(u64 *)BL_SYS_INFO_DRAM_SIZE;
	if (dram_size > DRAM_WRITE_SIZE_DEFAULT) {
#ifdef DRAM_BASE2
		redirection_base = DRAM_BASE2;
		possible_size = dram_size - DRAM_WRITE_SIZE_DEFAULT;
#endif
	} else {
		possible_size = dram_size / 2;
		redirection_base = DRAM_BASE;
		if ((DRAM_BASE + possible_size) >= CONFIG_LK_TEXT_BASE)
			possible_size = CONFIG_LK_TEXT_BASE - DRAM_BASE;
	}

	/* Check ramdump size */
	if (possible_size < (hdr->size + 1)) {
		print_lcd_update(FONT_RED, FONT_BLACK, "Invalid ramdump size!");
		printf("%s: Invalid dump size[0x%lx]\n", __func__, hdr->size);
		ret = -1;
		goto redirection_out;
	}

	/* Check valid data in storage */
	ret = part_read_partial(part, (void *)&metadata, (u64)METADATA_OFFSET, (u64)METADATA_SIZE);
	if (ret) {
		printf("%s: part read fail(line:%u)\n", __func__, __LINE__);
		ret = -1;
		goto redirection_out;
	}
	if (metadata.data.magic != RAMDUMP_STORE_MAGIC) {
		printf("%s: ramdump data doesn't exist[%x]\n", __func__, metadata.data.magic);
		printf("%s: current ram data will be extracted\n", __func__);
		goto redirection_out;
	}

	/* Set storage data base */
	if (hdr->base > 0xFFFFFFFFUL)
		storage_base = RAMDUMP_OFFSET + hdr->base - 0x800000000UL;
	else
		storage_base = RAMDUMP_OFFSET + hdr->base - 0x80000000UL;

	hdr->base = redirection_base;
	ret = part_read_partial(part, (void *)hdr->base, (u64)storage_base, (u64)(hdr->size));
	if (ret)
		 printf("%s: part read fail(line:%u)\n", __func__, __LINE__);

redirection_out:
	return ret;
#else
	return 0;
#endif
}

int debug_store_ramdump_oem(const char *cmd)
{
	int ret = 0;

	if (!strcmp(cmd, "set")) {
		g_is_enabled = 1;
	} else if (!strcmp(cmd, "reset")) {
		g_is_enabled = 0;
	} else if (!strcmp(cmd, "clear")) {
		void *part;

		part= part_get("ramdump");
		if (!part) {
			printf("%s: no ramdump partition\n", __func__);
			ret = -1;
			goto oem_out;
		}

		ret = part_read_partial(part, (void *)&metadata, METADATA_OFFSET, METADATA_SIZE);
		if (ret) {
			printf("%s: read metadata block fail\n", __func__);
			goto oem_out;
		}

		if (metadata.data.magic == RAMDUMP_STORE_MAGIC) {
			printf("%s: Data will be erased!\n", __func__);
			metadata.data.magic = 0;
			ret = part_write_partial(part, (void *)&metadata, METADATA_OFFSET, METADATA_SIZE);
			if (ret)
				printf("%s: write metadata block fail\n", __func__);
		} else {
			printf("%s: Data does not exist!\n", __func__);
		}
	} else if (!strcmp(cmd, "save")) {
		ret = debug_store_ramdump();
	} else {
		ret = -1;
	}

oem_out:
	return ret;
}

void debug_store_ramdump_getvar(const char *cmd, char *response)
{
	void *part;
	int ret;

	if (!g_is_enabled) {
		sprintf(response, "STORE RAMDUMP IS DISABLED");
		goto getvar_out;
	}

	part = part_get("ramdump");
	if (!part) {
		printf("%s: no ramdump partition\n", __func__);
		sprintf(response, "PIT DOES NOT EXIST");
		goto getvar_out;
	}

	ret = part_read_partial(part, (void *)&metadata, METADATA_OFFSET, METADATA_SIZE);
	if (ret) {
		sprintf(response, "PIT READ FAIL");
		goto getvar_out;
	}

	if (metadata.data.magic != RAMDUMP_STORE_MAGIC) {
		sprintf(response, "%s\n%s", "RAMDUMP STORE IS ENABLED",
						"DATA DOES NOT EXIST");
	} else {
		sprintf(response, "%s\n%s", "RAMDUMP STORE IS ENABLED",
							"DATA EXIST");
	}

getvar_out:
	return;
}
