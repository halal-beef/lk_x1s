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

#include <asm.h>
#include <arch/ops.h>
#include <arch/defines.h>
#include <platform/dfd.h>
#include <platform/sfr.h>

FUNCTION(dfd_entry_dump_gpr)
	/* booting core couldn't be here */
        ldr     x0, =CONFIG_IRAM_STACK
        mov     sp, x0
        bl      dfd_get_core_num
        bl      dfd_secondary_dump_gpr
        dsb     sy
        isb
        wfi

FUNCTION(__dfd_dump_gpr)
        /* Store dump gpr register to dram,
         * x1: target address
         * x2: source register
         * The offset is following as kernel's struct pt_regs */
        mov     x5, x2
        add     x2, x2, #DUMP_GPR_OFFSET
        ldp     x3, x4, [x2], #16 /* x0, x1 */
        stp     x3, x4, [x1], #16
        ldp     x3, x4, [x2], #16 /* x2, x3 */
        stp     x3, x4, [x1], #16
        ldp     x3, x4, [x2], #16 /* x4, x5 */
        stp     x3, x4, [x1], #16
        ldp     x3, x4, [x2], #16 /* x6, x7 */
        stp     x3, x4, [x1], #16
        ldp     x3, x4, [x2], #16 /* x8, x19 */
        str     x3, [x1], #88
        str     x4, [x1], #8
        ldp     x3, x4, [x2], #16 /* x20, x21 */
        stp     x3, x4, [x1], #16
        ldp     x3, x4, [x2], #16 /* x22, x23 */
        stp     x3, x4, [x1], #16
        ldp     x3, x4, [x2], #16 /* x24, x25 */
        stp     x3, x4, [x1], #16
        ldp     x3, x4, [x2], #16 /* x26, x27 */
        stp     x3, x4, [x1], #16
        ldp     x3, x4, [x2], #16 /* x28, x29 */
        stp     x3, x4, [x1], #16
        ldp     x3, x4, [x2], #16 /* x30, sp */
        stp     x3, x4, [x1], #16

        mov     x2, x5

        /* x0: target address, x1: core number */
        ldr     x3, [x2, x0, lsl #3]
        str     x3, [x1]
        ret

FUNCTION(dfd_get_core_num)
	mrs     x1, mpidr_el1
	and     x0, x1, #0xf            /* x0 = cpu id */
	lsr     x1, x1, #8
	and     x1, x1, #0xf            /* x1 = cluster id */
	cmp	x1, #1
	b.ne	1f
	add	x0, x0, #4		/* reorder cpu id */
1:
	ret				/* r0(cpu id) is return */
