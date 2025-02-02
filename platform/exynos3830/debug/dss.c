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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <lk/list.h>
#include <lib/io.h>
#include <lib/fdtapi.h>
#include <lib/font_display.h>
#include <lib/console.h>
#include <dev/boot.h>
#include <dev/debug/dss.h>
#include <platform/sfr.h>
#include <platform/sizes.h>
#include <platform/mmu/barrier.h>
#include <platform/dfd.h>

struct bl_log_header {
	unsigned int magic;
	unsigned int start;
	unsigned int last;
	unsigned int size;
	unsigned int count;
	unsigned int eob;
} *bl_log;

static void dbg_snapshot_kernel_print(print_callback_t *cb, const char *str, size_t len)
{
	u64 last_buf = readl(CONFIG_RAMDUMP_LASTBUF);

	if ((last_buf <= CONFIG_RAMDUMP_LOGBUF) ||
			(last_buf >= (CONFIG_RAMDUMP_LOGBUF + CONFIG_RAMDUMP_LOGSZ)) ||
			((CONFIG_RAMDUMP_LOGBUF + CONFIG_RAMDUMP_LOGSZ) < (last_buf + len))) {

		if (likely(last_buf >= CONFIG_RAMDUMP_LOGBUF &&
			last_buf < (CONFIG_RAMDUMP_LOGBUF + CONFIG_RAMDUMP_LOGSZ))) {
			u32 size;
			size = CONFIG_RAMDUMP_LOGBUF + CONFIG_RAMDUMP_LOGSZ - last_buf;
			memset((void *)last_buf, 0, size);
		}
		last_buf = CONFIG_RAMDUMP_LOGBUF;
	}

	memcpy((char *)last_buf, str, len);
	last_buf += len;
	writel(last_buf, CONFIG_RAMDUMP_LASTBUF);
}

static struct __print_callback dss_kernel_print_cb = {
	.entry = LIST_INITIAL_VALUE(dss_kernel_print_cb.entry),
	.print = dbg_snapshot_kernel_print,
};

void dbg_snapshot_copy_bl_log(void)
{
	u64 size = 0;

	/* first stage */
	/* copy sram log to dram buffer */
	bl_log = (struct bl_log_header *)CONFIG_RAMDUMP_SRAM_LOG_BASE;
	if (bl_log->magic != CONFIG_RAMDUMP_BL_LOG_MAGIC)
		return;

	if (bl_log->eob) {
		/* copy old log */
		size = bl_log->start + bl_log->size - bl_log->last;
		dbg_snapshot_kernel_print(NULL, (char *)(u64)bl_log->last, size);
	}
	size = bl_log->last - bl_log->start;
	dbg_snapshot_kernel_print(NULL, (char *)(u64)bl_log->start, size);

	memset((void *)CONFIG_RAMDUMP_SRAM_LOG_BASE, 0, bl_log->size + sizeof(struct bl_log_header));
}

void dbg_snapshot_early_init(void)
{
	unsigned int rst_stat = readl(EXYNOS3830_POWER_RST_STAT);

	if (rst_stat & PIN_RESET)
		memset((void *)CONFIG_RAMDUMP_BASE, 0, SZ_4K);
	register_print_callback(&dss_kernel_print_cb);
	dbg_snapshot_copy_bl_log();
}
