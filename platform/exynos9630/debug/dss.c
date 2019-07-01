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
#include <stdlib.h>
#include <libfdt.h>
#include <stdio.h>
#include <part.h>
#include <lib/console.h>
#include <platform/exynos9630.h>
#include <dev/boot.h>
#include <platform/sfr.h>
#include <platform/sizes.h>
#include <platform/dfd.h>
#include <lib/fdtapi.h>

#define DSS_RESERVE_PATH	"/reserved-memory/debug_snapshot"

extern int load_boot_images(void);

struct reserve_mem {
	unsigned int paddr;
	unsigned int size;
};

struct dss_item {
	char name[16];
	struct reserve_mem rmem;
	int enabled;
};

struct dbg_snapshot_bl {
	unsigned int magic1;
	unsigned int magic2;
	unsigned int item_count;
	unsigned int reserved;
	struct dss_item item[16];
};

struct dbg_snapshot_bl static_dss_bl = {
	.item[0] = {"header",		{0, 0}, 0},
	.item[1] = {"log_kernel",	{0, 0}, 0},
	.item[2] = {"log_platform",	{0, 0}, 0},
	.item[3] = {"log_sfr",		{0, 0}, 0},
	.item[4] = {"log_s2d",		{0, 0}, 0},
	.item[5] = {"log_arrdumpreset",	{0, 0}, 0},
	.item[6] = {"log_arrdumppanic",	{0, 0}, 0},
	.item[8] = {"log_bcm",		{0, 0}, 0},
	.item[9] = {"log_llc",		{0, 0}, 0},
	.item[10] = {"log_dbgc",	{0, 0}, 0},
	.item[11] = {"log_pstore",	{0, 0}, 0},
	.item[12] = {"log_kevents",	{0, 0}, 0},
	.item[13] = {"log_fatal",	{0, 0}, 0},
};

struct dbg_snapshot_bl *dss_bl_p = &static_dss_bl;

void dbg_snapshot_boot_cnt(void)
{
	unsigned int reg;

	reg = readl(CONFIG_RAMDUMP_BL_BOOT_CNT_MAGIC);
	if (reg == RAMDUMP_BOOT_CNT_MAGIC) {
		reg = readl(CONFIG_RAMDUMP_BL_BOOT_CNT);
		reg += 1;
		writel(reg , CONFIG_RAMDUMP_BL_BOOT_CNT);
	} else {
		reg = 1;
		writel(reg, CONFIG_RAMDUMP_BL_BOOT_CNT);
		writel(RAMDUMP_BOOT_CNT_MAGIC, CONFIG_RAMDUMP_BL_BOOT_CNT_MAGIC);
	}

	printf("Bootloader Booting SEQ #%u\n", reg);
}

void debug_snapshot_boot_cnt(void)
{
	unsigned int reg;

	reg = readl(CONFIG_RAMDUMP_BL_BOOT_CNT_MAGIC);
	if (reg == RAMDUMP_BOOT_CNT_MAGIC) {
		reg = readl(CONFIG_RAMDUMP_BL_BOOT_CNT);
		reg += 1;
		writel(reg , CONFIG_RAMDUMP_BL_BOOT_CNT);
	} else {
		reg = 1;
		writel(reg, CONFIG_RAMDUMP_BL_BOOT_CNT);
		writel(RAMDUMP_BOOT_CNT_MAGIC, CONFIG_RAMDUMP_BL_BOOT_CNT_MAGIC);
	}

	printf("Bootloader Booting SEQ #%u\n", reg);
}

static int dbg_snapshot_get_items(void)
{
	char path[64];
	u32 ret[8];
	u32 i;

	if (readl(CONFIG_RAMDUMP_DSS_ITEM_INFO) == 0x01234567
	    && readl(CONFIG_RAMDUMP_DSS_ITEM_INFO + 0x4) == 0x89ABCDEF) {
		dss_bl_p = (struct dbg_snapshot_bl *)CONFIG_RAMDUMP_DSS_ITEM_INFO;
	} else {
		load_boot_images();
		fdt_dtb = (struct fdt_header *)DT_BASE;

		for (i = 0; i < ARRAY_SIZE(static_dss_bl.item); i++) {
			if (!strlen(static_dss_bl.item[i].name))
				continue;

			memset(path, 0, 64);
			memset(ret, 0, 8 * sizeof(u32));
			strncpy(path, DSS_RESERVE_PATH, sizeof(DSS_RESERVE_PATH));
			strncat(path, "/", 1);
			strncat(path, static_dss_bl.item[i].name,
			        strlen(static_dss_bl.item[i].name));

			if (!get_fdt_val(path, "reg", (char *)ret)) {
				static_dss_bl.item[i].rmem.paddr |= be32_to_cpu(ret[1]);
				static_dss_bl.item[i].rmem.size = be32_to_cpu(ret[2]);
				static_dss_bl.item[i].enabled = 1;
			} else {
				static_dss_bl.item[i].enabled = 0;
			}
		}

		dss_bl_p = &static_dss_bl;
	}

	printf("debug-snapshot kernel physical memory layout:(MAGIC:0x%lx)\n",
	       (unsigned long)readl(CONFIG_RAMDUMP_DSS_ITEM_INFO) << 32L |
	       (unsigned long)readl(CONFIG_RAMDUMP_DSS_ITEM_INFO + 0x4));

	for (i = 0; i < ARRAY_SIZE(dss_bl_p->item); i++) {
		if (dss_bl_p->item[i].enabled) {
			printf("%-15s: phys:0x%x / size:0x%x\n",
			       dss_bl_p->item[i].name,
			       dss_bl_p->item[i].rmem.paddr,
			       dss_bl_p->item[i].rmem.size);
		}
	}

	return 0;
}

unsigned long dbg_snapshot_get_item_count(void)
{
	return ARRAY_SIZE(dss_bl_p->item);
}

struct dss_item *dbg_snapshot_get_item(const char *name)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(dss_bl_p->item); i++) {
		if (!strncmp(dss_bl_p->item[i].name, name, strlen(dss_bl_p->item[i].name))
		    && dss_bl_p->item[i].enabled)
			return &dss_bl_p->item[i];
	}

	return NULL;
}

unsigned long dbg_snapshot_get_item_paddr(const char *name)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(dss_bl_p->item); i++) {
		if (!strncmp(dss_bl_p->item[i].name, name, strlen(name)))
			return dss_bl_p->item[i].rmem.paddr;
	}

	return 0;
}

unsigned long dbg_snapshot_get_item_size(const char *name)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(dss_bl_p->item); i++) {
		if (!strncmp(dss_bl_p->item[i].name, name, strlen(name)))
			return dss_bl_p->item[i].rmem.size;
	}

	return 0;
}

void dbg_snapshot_fdt_init(void)
{
	if (dbg_snapshot_get_items() < 0)
		return;
}

int dbg_snapshot_getvar_item(const char *name, char *response)
{
	char log_name[32] = { 0, };
	struct dss_item *item;

	if (!strcmp(name, "dramsize")) {
		u64 dram_size = *(u64 *)BL_SYS_INFO_DRAM_SIZE;

		dram_size /= SZ_1M;
		sprintf(response, "%lluMB", dram_size);
		return 0;
	}

	if (!strcmp(name, "header")) {
		item = dbg_snapshot_get_item("header");
		if (!item)
			return -1;

		sprintf(response, "%X, %X, %X", item->rmem.paddr, item->rmem.size,
		        item->rmem.paddr + item->rmem.size - 1);

		return 0;
	}

	snprintf(log_name, sizeof(log_name) - 1, "log_%s", name);
	item = dbg_snapshot_get_item(log_name);
	if (!item)
		return -1;

	sprintf(response, "%X, %X, %X", item->rmem.paddr, item->rmem.size,
	        item->rmem.paddr + item->rmem.size - 1);
	return 0;
}
