/*
 * ==============================================================================
 *
 *       Filename:  cpu_a.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2014-07-01
 *       Revision:  none
 *       Compiler:  aarch64-arm-none-gcc
 *
 *         Author:  Kim, Young-san (ys0225.kim@samsung.com)
 *   Organization:  AP Development, System LSI Business
 *
 * ==============================================================================
 */
#ifndef _CPU_A_H_
#define _CPU_A_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "barrier.h"
#include "cache.h"

struct cpu_stack_ctx {
	phys_addr_t	stack_ptr[32];
};

/******************************************************************
*- CPU ID related procedures
******************************************************************/
u32 get_cpu_id(void);
u32 get_cluster_id(void);
u32 get_cpu_num(void);
u32 get_cpu_part_num(void);
u32 get_cpu_implementer(void);
u32 get_current_el(void);
u32 get_sp_sel(void);

/******************************************************************
*- mmu,cache control related spr access
******************************************************************/
u64 get_sctlr_el1(void);
u64 get_sctlr_el2(void);
u64 get_sctlr_el3(void);
void set_sctlr_el1(u64 val);
void set_sctlr_el2(u64 val);
void set_sctlr_el3(u64 val);

u64 get_mair_el1(void);
u64 get_mair_el2(void);
u64 get_mair_el3(void);
void set_mair_el1(u64 val);
void set_mair_el2(u64 val);
void set_mair_el3(u64 val);

u64 get_tcr_el1(void);
u64 get_tcr_el2(void);
u64 get_tcr_el3(void);
u64 get_vtcr_el2(void);
void set_tcr_el1(u64 val);
void set_tcr_el2(u64 val);
void set_tcr_el3(u64 val);
void set_vtcr_el2(u64 val);

u64 *get_ttbr0_el1(void);
u64 *get_ttbr0_el2(void);
u64 *get_ttbr0_el3(void);
u64 *get_ttbr1_el1(void);
u64 *get_vttbr_el2(void);

void set_ttbr0_el1(u64 *addr);
void set_ttbr0_el2(u64 *addr);
void set_ttbr0_el3(u64 *addr);
void set_ttbr1_el1(u64 *addr);
void set_vttbr_el2(u64 *addr);

#define D_DAIF	(0x1<<9)
#define A_DAIF	(0x1<<8)
#define I_DAIF	(0x1<<7)
#define F_DAIF	(0x1<<6)

u64 get_daif(void);
void set_daif(u64 arg);

u32 is_dcache_enable_d(void);

//------------------------------------------------------------------------------
//- spr access
//------------------------------------------------------------------------------

u64 get_scr_el3(void);
void set_scr_el3(u64 val);
u64 get_hcr_el2(void);
void set_hcr_el2(u64 val);

u64 get_esr_el1(void);
u64 get_esr_el2(void);
u64 get_esr_el3(void);
u64 get_elr_el1(void);
u64 get_elr_el2(void);
u64 get_elr_el3(void);
void set_elr_el1(u64 val);
void set_elr_el2(u64 val);
void set_elr_el3(u64 val);
u64 get_spsr_el1(void);
u64 get_spsr_el2(void);
u64 get_spsr_el3(void);
void set_spsr_el1(u64 val);
void set_spsr_el2(u64 val);
void set_spsr_el3(u64 val);

u64 *get_vbar_el1(void);
u64 *get_vbar_el2(void);
u64 *get_vbar_el3(void);
void set_vbar_el1(u64 *addr);
void set_vbar_el2(u64 *addr);
void set_vbar_el3(u64 *addr);

u64 get_sp_el0(void);
u64 get_sp_el1(void);
u64 get_sp_el2(void);
void set_sp_el0(u64 val);
void set_sp_el1(u64 val);
void set_sp_el2(u64 val);
u64 get_sp_CurrentEL(void);
void set_sp_CurrentEL(u64 val);

extern void *vbar_el3_swap(void *new_vbar_el3);
extern void *vbar_el1_swap(void *new_vbar_el1);



//------------------------------------------------------------------------------
//- implementation defined sprs for aarch64
//------------------------------------------------------------------------------
u64 get_l2ctlr_el1(void);
void set_l2ctlr_el1(u64 val);
u64 get_l2ectlr_el1(void);
void set_l2ectlr_el1(u64 val);
u64 get_l2actlr_el1(void);
void set_l2actlr_el1(u64 val);
u64 get_cpuactlr_el1(void);
void set_cpuactlr_el1(u64 val);
u64 get_cpuectlr_el1(void);
void set_cpuectlr_el1(u64 val);

//------------------------------------------------------------------------------
//- aarch64 architectural feature trap registers
//------------------------------------------------------------------------------

u64 get_cptr_el3(void);
u64 get_cptr_el2(void);
u64 get_cpacr_el1(void);
void set_cptr_el3(u64 val);
void set_cptr_el2(u64 val);
void set_cpacr_el1(u64 val);
u64 get_actlr_el3(void);
void set_actlr_el3(u64 val);
u64 get_actlr_el2(void);
void set_actlr_el2(u64 val);

//------------------------------------------------------------------------------
//- tlb operations
//------------------------------------------------------------------------------

void invalidate_tlb_alle1(void);
void invalidate_tlb_alle2(void);
void invalidate_tlb_alle3(void);
void invalidate_tlb_alle1is(void);
void invalidate_tlb_alle2is(void);
void invalidate_tlb_alle3is(void);
void invalidate_tlb_vmalle1(void);
void invalidate_tlb_vmalle1is(void);

//------------------------------------------------------------------------------
//- Cache maintenance operations
//------------------------------------------------------------------------------
u64 get_dcache_size_level(u64 level);
u64 get_dcache_linesize_level(u64 level);
u64 get_dcache_waynum_level(u64 level);
u64 get_dcache_setnum_level(u64 level);

//------------------------------------------------------------------------------
//- primitives
//------------------------------------------------------------------------------
#if 0 // old diag style
void smc(void);
#else
extern u64 smc(u64 cmd, u64 arg1, u64 arg2, u64 arg3);
#endif
extern u64 hvc(u64 cmd, u64 arg1, u64 arg2, u64 arg3);

static inline void CleanAndInvalidateDCache(u64 uStAddr, u64 uSize)
{
	u64 *start, *end;

	start = (u64 *)uStAddr;
	end = (u64 *)(uStAddr + uSize);

	clean_invalidate_dcache_range(start, end);
}

static inline void InvalidateDCache(u64 uStAddr, u64 uSize)
{
	u64 *start, *end;

	start = (u64 *)uStAddr;
	end = (u64 *)(uStAddr + uSize);

	invalidate_dcache_range(start, end);
}

static inline void CoCleanDCache(u64 uStAddr, u64 uSize)
{
	u64 *start, *end;

	start = (u64 *)uStAddr;
	end = (u64 *)(uStAddr + uSize);

	clean_dcache_range(start, end);
}

#define CleanDCache CoCleanDCache
#define CleanInvalidateDCacheAll clean_invalidate_dcache_all
#define CPU_GetCPUNum get_cpu_num


#define IMPLEMENTER_ARM_LIMITED	0x41
#define PART_A53	0xd03
#define PART_A57	0xd07

#define IMPLEMENTER_SAMSUNG_CORPORATION	0x53
#define PART_MNGS	0x1
#define PART_CHEETAH	0x3


#ifdef __cplusplus
}
#endif

#endif
