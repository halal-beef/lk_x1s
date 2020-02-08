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
#include <stdio.h>
#include <string.h>
#include <list.h>
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

static void dbg_snapshot_kernel_print(print_callback_t *cb, const char *str, size_t len)
{
	u64 last_buf = readl(CONFIG_RAMDUMP_LASTBUF);

	if ((last_buf <= CONFIG_RAMDUMP_BASE) ||
			(last_buf >= (CONFIG_RAMDUMP_LOGBUF + CONFIG_RAMDUMP_LOGSZ)) ||
			((CONFIG_RAMDUMP_LOGBUF + CONFIG_RAMDUMP_LOGSZ) < (last_buf + len)))
		last_buf = CONFIG_RAMDUMP_LOGBUF;
	strncpy((char *)last_buf, str, len);
	last_buf += len;
	writel(last_buf, CONFIG_RAMDUMP_LASTBUF);
}

static struct __print_callback dss_kernel_print_cb = {
	.entry = LIST_INITIAL_VALUE(dss_kernel_print_cb.entry),
	.print = dbg_snapshot_kernel_print,
};

void dbg_snapshot_early_init(void)
{
	register_print_callback(&dss_kernel_print_cb);
}
