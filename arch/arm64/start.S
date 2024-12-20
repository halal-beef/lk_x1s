#include <asm.h>
#include <arch/arm64/mmu.h>
#include <arch/asm_macros.h>
#include <kernel/vm.h>

/*
 * Register use:
 *  x0-x3   Arguments
 *  x9-x15  Scratch
 *  x19-x28 Globals
 */
tmp                     .req x9
tmp2                    .req x10
wtmp2                   .req w10
index                   .req x11
index_shift             .req x12
page_table              .req x13
new_page_table          .req x14
phys_offset             .req x15

cpuid                   .req x19
page_table0             .req x20
page_table1             .req x21
mmu_initial_mapping     .req x22
vaddr                   .req x23
paddr                   .req x24
mapping_size            .req x25
size                    .req x26
attr                    .req x27

.section .text.boot
FUNCTION(_start)

.globl arm_reset
arm_reset:
    bl      arm64_elX_to_el1

    ldr tmp, =__stack_end
    mov sp, tmp

    /* clear bss */
.L__do_bss:
    /* clear out the bss excluding the stack and kernel translation table  */
    /* NOTE: relies on __post_prebss_bss_start and __bss_end being 8 byte aligned */
    ldr     tmp, =__post_prebss_bss_start
    ldr     tmp2, =__bss_end
    sub     tmp2, tmp2, tmp
    cbz     tmp2, .L__bss_loop_done
.L__bss_loop:
    sub     tmp2, tmp2, #8
    str     xzr, [tmp], #8
    cbnz    tmp2, .L__bss_loop
.L__bss_loop_done:

    bl  lk_main
    b   .

#if WITH_SMP
.Lsecondary_boot:
    and     tmp, cpuid, #0xff
    cmp     tmp, #(1 << SMP_CPU_CLUSTER_SHIFT)
    bge     .Lunsupported_cpu_trap
    bic     cpuid, cpuid, #0xff
    orr     cpuid, tmp, cpuid, LSR #(8 - SMP_CPU_CLUSTER_SHIFT)

    cmp     cpuid, #SMP_MAX_CPUS
    bge     .Lunsupported_cpu_trap

    /* Set up the stack */
    ldr     tmp, =__stack_end
    mov     tmp2, #ARCH_DEFAULT_STACK_SIZE
    mul     tmp2, tmp2, cpuid
    sub     sp, tmp, tmp2

    mov     x0, cpuid
    bl      arm64_secondary_entry

.Lunsupported_cpu_trap:
    wfe
    b       .Lunsupported_cpu_trap
#endif

.ltorg

#if WITH_SMP
.data
DATA(page_tables_not_ready)
    .long       1
#endif

.section .bss.prebss.stack
    .align 4
DATA(__stack)
    .skip ARCH_DEFAULT_STACK_SIZE * SMP_MAX_CPUS
DATA(__stack_end)

#if WITH_KERNEL_VM
.section ".bss.prebss.translation_table"
.align 3 + MMU_PAGE_TABLE_ENTRIES_IDENT_SHIFT
DATA(tt_trampoline)
    .skip 8 * MMU_PAGE_TABLE_ENTRIES_IDENT
#endif
