/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
 *
 * DFD for Exynos
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#define __ASSEMBLY__
#include <lk/asm.h>
#include <arch/ops.h>
#include <arch/defines.h>
#include <platform/dfd.h>
#include <platform/sfr.h>

FUNCTION(dfd_entry_point)
	/* booring core couldn't be here */
	ldr	x0, =CONFIG_IRAM_STACK
	mov	sp, x0
	bl	read_mpidr
	bl	get_core_pos
	bl	dfd_secondary_cpu_cache_flush
	dsb     sy
	isb
	wfi

FUNCTION(read_mpidr)
	mrs	x0, mpidr_el1
	tst	x0, #(1 << MT_BIT_OFFSET)
	beq	1f
	lsr	x0, x0, #MPIDR_AFFINITY_BITS
	and	x0, x0, #((1 << MPIDR_AFF2_SHIFT) - 1)
1:
	ret

FUNCTION(get_core_pos)
        and     x1, x0, #MPIDR_CPU_MASK
        ands    x0, x0, #MPIDR_CLUSTER_MASK
        add     x2, x1, #2                      /* Cluster 1 */
        csel    x1, x1, x2, eq
        add     x0, x1, x0, LSR #6
        ret
