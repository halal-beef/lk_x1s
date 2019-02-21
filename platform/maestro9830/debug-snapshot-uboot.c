/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <stdlib.h>
#include <libfdt.h>
#include <pit.h>
#include <part_gpt.h>
#include <lib/console.h>
#include <platform/exynos9830.h>
#include <platform/sfr.h>
#include <platform/sizes.h>
#include <platform/ab_update.h>
#include <platform/dfd.h>
#include <platform/fdt.h>

#define ARRAY_SIZE(x)		(int)(sizeof(x) / sizeof((x)[0]))
#define DSS_RESERVE_PATH	"/reserved-memory/debug_snapshot"
#define CP_RESERVE_PATH		"/reserved-memory/modem_if"

struct reserve_mem {
	unsigned long paddr;
	unsigned long size;
};

struct dss_item {
	char name[16];
	struct reserve_mem rmem;
	int enabled;
};

struct reserve_mem dss_rmem;
struct reserve_mem cp_rmem;
struct dss_item dss_items[] = {
	{"header",		{0, 0}, 0},
	{"log_kernel",		{0, 0}, 0},
	{"log_platform",	{0, 0}, 0},
	{"log_sfr",		{0, 0}, 0},
	{"log_pstore",		{0, 0}, 0},
	{"log_kevents",		{0, 0}, 0},
};

static struct reserve_mem *debug_snapshot_get_reserve_mem(void)
{
	return &dss_rmem;
}

static int debug_snapshot_load_dt(void)
{
	struct pit_entry *ptn;

	if (ab_current_slot())
		ptn = pit_get_part_info("dtb_b");
	else
		ptn = pit_get_part_info("dtb_a");

	if (ptn == 0) {
		printf("Partition 'dtb' does not exist\n");
		return -1;
	} else {
		pit_access(ptn, PIT_OP_LOAD, (u64)DT_BASE, 0);
	}

	fdt_dtb = (struct fdt_header *)DT_BASE;


	return 0;
}

static int debug_snapshot_get_items_from_dt(void)
{
	char path[64];
	u32 ret[8];
	int i;

	for (i = 0; i < ARRAY_SIZE(dss_items); i++) {
		memset(path, 0, 64);
		memset(ret, 0, 8 * sizeof(u32));
		strncpy(path, DSS_RESERVE_PATH, sizeof(DSS_RESERVE_PATH));
		strncat(path, "/", 1);
		strncat(path, dss_items[i].name, strlen(dss_items[i].name));

		if (!get_fdt_val(path, "reg", (char *)ret)) {
			dss_items[i].rmem.paddr = be32_to_cpu(ret[0]);
			dss_items[i].rmem.paddr <<= 32UL;
			dss_items[i].rmem.paddr |= be32_to_cpu(ret[1]);
			dss_items[i].rmem.size = be32_to_cpu(ret[2]);
			dss_items[i].enabled = 1;
		} else {
			dss_items[i].enabled = 0;
		}
	}

	printf("debug-snapshot kernel physical memory layout:\n");
	for (i = 0; i < ARRAY_SIZE(dss_items); i++) {
		if (dss_items[i].enabled)
			printf("%-15s: phys:0x%lx / size:0x%lx\n",
					dss_items[i].name,
					dss_items[i].rmem.paddr,
					dss_items[i].rmem.size);
	}

	memset(ret, 0, 8 * sizeof(u32));
	if (!get_fdt_val(CP_RESERVE_PATH, "reg", (char *)ret)) {
		cp_rmem.paddr = be32_to_cpu(ret[0]);
		cp_rmem.paddr <<= 32UL;
		cp_rmem.paddr |= be32_to_cpu(ret[1]);
		cp_rmem.size = be32_to_cpu(ret[2]);
	}

	return 0;
}

unsigned long debug_snapshot_get_item_count(void)
{
	return ARRAY_SIZE(dss_items);
}

struct dss_item *debug_snapshot_get_item(const char *name)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(dss_items); i++) {
		if (!strncmp(dss_items[i].name, name, strlen(dss_items[i].name))
				&& dss_items[i].enabled)
			return &dss_items[i];
	}

	return NULL;
}

unsigned long debug_snapshot_get_item_paddr(const char *name)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(dss_items); i++) {
		if (!strncmp(dss_items[i].name, name, strlen(name)))
			return dss_items[i].rmem.paddr;
	}

	return 0;
}

void debug_snapshot_fdt_init(void)
{
	if (debug_snapshot_load_dt())
		return;

	if (debug_snapshot_get_items_from_dt() < 0)
		return;
}

int debug_snapshot_getvar_item(const char *name, char *response)
{
	char log_name[16] = {0, };
	struct dss_item *item;

	if (!strcmp(name, "dramsize")) {
		u64 dram_size = *(u64 *)BL_SYS_INFO_DRAM_SIZE;

		dram_size /= SZ_1M;
		sprintf(response, "%lluMB", dram_size);
		return 0;
	}

	if (!strcmp(name, "cpmem")) {
		if (cp_rmem.paddr == 0 || cp_rmem.size == 0)
			return -1;
		sprintf(response, "%lX, %lX, %lX", cp_rmem.paddr, cp_rmem.size - 1,
				cp_rmem.paddr + cp_rmem.size - 1);
		return 0;
	}

	snprintf(log_name, 16, "log_%s", name);
	item = debug_snapshot_get_item(log_name);
	if (!item)
		return -1;

	sprintf(response, "%lX, %lX, %lX", item->rmem.paddr, item->rmem.size - 1,
			item->rmem.paddr + item->rmem.size - 1);
	return 0;
}
