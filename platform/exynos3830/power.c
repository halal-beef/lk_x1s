/*
 * power.c
 *
 *  Created on: 2019. 5. 7.
 *      Author: sunghyun.na
 */
#include <lk/debug.h>
#include <lk/err.h>
#include <lk/compiler.h>
#include <platform.h>
#include <platform/debug.h>
#include <kernel/thread.h>
#include <stdio.h>
#include <lib/console.h>

#include "platform/sfr.h"

extern int start_usb_gadget(void);
extern void gadgeg_dev_polling_handle(void);

void platform_halt(platform_halt_action suggested_action,
                          platform_halt_reason reason)
{
#if ENABLE_PANIC_SHELL

    if (reason == HALT_REASON_SW_PANIC) {
        dprintf(ALWAYS, "CRASH: starting debug shell... (reason = %d)\n", reason);
        arch_disable_ints();
        panic_shell_start();
    }

#endif  // ENABLE_PANIC_SHELL
	if (suggested_action == HALT_ACTION_REBOOT) {
		writel(0, CONFIG_RAMDUMP_SCRATCH);

		writel(readl(EXYNOS3830_SYSTEM_CONFIGURATION) | 0x2, EXYNOS3830_SYSTEM_CONFIGURATION);
	}
    dprintf(ALWAYS, "HALT: spinning forever... (reason = %d)\n", reason);
    arch_disable_ints();
    start_usb_gadget();
    for (;;) {
	    gadgeg_dev_polling_handle();
    }
}
