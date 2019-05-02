/******************************************************************
*-
*- Filename		: cpu_aarch64.s
*-
*- Description	: cpu related asm PROCs for aarch64
*-
*- Author		: Jaeyoul Kim <jyoul.kim@samsung.com>
*- Created at	: 13/11/09
*-
*- Modified by	:
*- Modified at	:
*-
*- Copyright(C) : Samsung Electronics, 2010, All rights reserved.
*-
******************************************************************/

/******************************************************************
*- Area definition
******************************************************************/

#define MPIDR_MT_MASK	(0x1 << 24)

	.align 3

	.global  get_cpu_id
	.global  get_cluster_id
	.global  get_cpu_num
	.global  get_cpu_part_num
	.global  get_cpu_implementer
	.global  get_current_el
	.global  get_sp_sel

/******************************************************************
*- CPU ID related procedures
******************************************************************/

	/**
	 * Returns the Aff0 field from the MPIDR_EL1
	 * unsigned int get_cpu_id(void)
	 */
get_cpu_id:
	mrs     x0, mpidr_el1
#if defined(CONFIG_SOC_S5E9810) || defined(CONFIG_SOC_S5E9820) ||	\
    defined(CONFIG_SOC_S5E9830) || defined(CONFIG_SOC_S5E9809)
	and	x1, x0, #MPIDR_MT_MASK
	mov	x2, #8
	tst	x1, #MPIDR_MT_MASK
	csel	x1, x2, xzr, ne
	lsr	x0, x0, x1
	and     x0, x0,#0x3
	mov     w0, w0		// Caller expects a 32-bit value, not a 64-bit
#else
	and     x0, x0,#0x3
	mov     w0, w0		// Caller expects a 32-bit value, not a 64-bit
#endif
	ret

	/**
	 * Returns the Aff0 field from the MPIDR_EL1
	 * unsigned int get_cluster_id(void)
	 */
get_cluster_id:
	mrs     x0, mpidr_el1
#if defined(CONFIG_SOC_S5E9810) || defined(CONFIG_SOC_S5E9820) ||	\
    defined(CONFIG_SOC_S5E9830) || defined(CONFIG_SOC_S5E9809)
	and	x1, x0, #MPIDR_MT_MASK
	mov	x2, #8
	tst	x1, #MPIDR_MT_MASK
	csel	x1, x2, xzr, ne
	lsr	x0, x0, x1
#endif
	lsr	x0, x0, #8
	and     x0, x0, #0xff
	mov     w0, w0		// Caller expects a 32-bit value, not a 64-bit
	ret

// ------------------------------------------------------------
// Returns the Aff0 field from the MIDR_EL1
// unsigned int get_cpu_part_num(void)
get_cpu_part_num:
	mrs     x0, midr_el1
	lsr     x0, x0, #4
	and     x0, x0, #0xfff
	mov     w0, w0		// caller expects a 32-bit value, not a 64-bit
	ret

get_cpu_implementer:
	mrs     x0, midr_el1
	lsr     x0, x0, #24
	and     x0, x0, #0xff
	mov     w0, w0		// caller expects a 32-bit value, not a 64-bit
	ret

// ------------------------------------------------------------
// returns the aff0 field from the midr_el1
// unsigned int get_cpu_num(void)

#if defined(CONFIG_SOC_S5E7872_K2) || defined(CONFIG_SOC_S5E7885)	/* three-clusters */

get_cpu_num:
	mrs     x0, MPIDR_EL1		// Read Multiprocessor Affinity Register
	and	x1, x0, #3		// Get CPU ID
	lsr     x0, x0, #0x8
	and	x0, x0, #0xff		// Get Cluster ID
	cmp     x0, #0
	beq	cpucl0_cpu_num
	cmp     x0, #1
	beq	cpucl1_cpu_num
	cmp     x0, #2
	beq	cpucl2_cpu_num
	ret
cpucl2_cpu_num:
	add	x0, x1, #6		// Cluster 2 -> core4,5 (logical)
	mov	w0, w0			// caller expects a 32-bit value, not a 64-bit
	ret
cpucl1_cpu_num:
	add	x0, x1, #0		// Cluster 1 -> core0,1,2,3 (logical)
	mov	w0, w0			// caller expects a 32-bit value, not a 64-bit
	ret
cpucl0_cpu_num:
	add	x0, x1, #4		// Cluster 0 -> core6,7 (logical)
	mov	w0, w0			// caller expects a 32-bit value, not a 64-bit
	ret

#elif defined(CONFIG_SOC_S5E9810) || defined(CONFIG_SOC_S5E9820) ||	\
      defined(CONFIG_SOC_S5AHR80) || defined(CONFIG_SOC_S5E9830) ||	\
      defined(CONFIG_SOC_S5E9809)

	// S5E9810, S5E9809:
	//	core name	 MPIDR		core_num
	//	Ananke     core0 0x8100_0000	0
	//	Ananke     core1 0x8100_0100	1
	//	Ananke     core2 0x8100_0200	2
	//	Ananke     core3 0x8100_0300	3
	//	Meerkat    core0 0x8000_0100	4
	//	Meerkat    core1 0x8000_0101	5
	//	Meerkat    core2 0x8000_0102	6
	//	Meerkat    core3 0x8000_0103	7
	// S5E9820:
	//	core name	 MPIDR		core_num
	//	Ananke     core0 0x8100_0000	0
	//	Ananke     core1 0x8100_0100	1
	//	Ananke     core2 0x8100_0200	2
	//	Ananke     core3 0x8100_0300	3
	//	Prometheus core0 0x8100_0400	4
	//	Prometheus core1 0x8100_0500	5
	//	Cheetah    core0 0x8000_0100	6
	//	Cheetah    core1 0x8000_0101	7
	// S5AHR80:
	//	core name	 MPIDR		core_num
	//	Enyo       core0 0x8100_0000	0
	//	Enyo       core1 0x8100_0100	1
	//	Enyo       core2 0x8100_0200	2
	//	Enyo       core3 0x8100_0300	3
	//	Enyo       core0 0x8101_0000	4
	//	Enyo       core1 0x8101_0100	5
	//	Enyo       core2 0x8101_0200	6
	//	Enyo       core3 0x8101_0300	7

get_cpu_num:
	mrs	x0, mpidr_el1
	and	x1, x0, #MPIDR_MT_MASK	// extract MT bit
	mov	x2, #8			// shift value for ananke (armv8.2)
	tst	x1, #MPIDR_MT_MASK	// check MT bit
	csel	x1, x2, xzr, ne
	lsr	x0, x0, x1
	and	x1, x0, #0xff		// x1: cpu_id in cluster
	and	x2, x0, #(0xff << 8)	// x0: cluster_id

    #if defined(CONFIG_SOC_S5E9820) || defined(CONFIG_SOC_S5E9830)	/* three-clusters */
	mov	x0, x1
	cmp	x2, #0
	b.eq	1f
	add	x0, x0, #6
1:
    #else
	add	x0, x1, x2, lsr #6	// x0: cpu_num
    #endif

	ret

#else

get_cpu_num:
	mrs     x0, mpidr_el1		// read multiprocessor affinity register
	and	x1, x0, #0x3		// get cpu id
	lsr     x0, x0, #0x8
	and	x0, x0, #0xff		// get cluster id
#if !defined(CONFIG_SOC_S5E7870) && !defined(CONFIG_SOC_S5E7880) && \
    !defined(CONFIG_SOC_S5E7570) && !defined(CONFIG_SOC_S5E7872) && \
    !defined(CONFIG_SOC_S5E9110)
	cmp     x0, #0
	cset    x0, eq
#endif
	lsl     x0, x0, #0x2
	orr     x0, x0, x1
	mov     w0, w0			// caller expects a 32-bit value, not a 64-bit
	ret

#endif /* CONFIG_SOC_S5E7872_K2 */

get_current_el:
	mrs     x0, CurrentEL
	lsr     x0, x0, #2
	mov     w0, w0			// caller expects a 32-bit value, not a 64-bit
	ret

get_sp_sel:
	mrs     x0, spsel
	mov     w0, w0			// caller expects a 32-bit value, not a 64-bit
	ret

//------------------------------------------------------------------------------
//- mmu,cache control related spr access
//------------------------------------------------------------------------------

	.global get_sctlr_el1
	.global get_sctlr_el2
	.global get_sctlr_el3
	.global set_sctlr_el1
	.global set_sctlr_el2
	.global set_sctlr_el3

	.global get_mair_el1
	.global get_mair_el2
	.global get_mair_el3
	.global set_mair_el1
	.global set_mair_el2
	.global set_mair_el3

	.global get_tcr_el1
	.global get_tcr_el2
	.global get_tcr_el3
	.global get_vtcr_el2
	.global set_tcr_el1
	.global set_tcr_el2
	.global set_tcr_el3
	.global set_vtcr_el2

	.global get_ttbr0_el1
	.global get_ttbr0_el2
	.global get_ttbr0_el3
	.global get_ttbr1_el1
	.global get_vttbr_el2

	.global set_ttbr0_el1
	.global set_ttbr0_el2
	.global set_ttbr0_el3
	.global set_ttbr1_el1
	.global set_vttbr_el2

	.global set_daif
	.global get_daif

	.global is_dcache_enable_d

get_sctlr_el1:
	mrs     x0, sctlr_el1
	ret

get_sctlr_el2:
	mrs     x0, sctlr_el2
	ret

get_sctlr_el3:
	mrs     x0, sctlr_el3
	ret

set_sctlr_el1:
	msr     sctlr_el1, x0
	ret

set_sctlr_el2:
	msr     sctlr_el2, x0
	ret

set_sctlr_el3:
	msr     sctlr_el3, x0
	ret

set_mair_el1:
	msr     mair_el1, x0
	ret

set_mair_el2:
	msr     mair_el2, x0
	ret

set_mair_el3:
	msr     mair_el3, x0
	ret

get_mair_el1:
	mrs     x0, mair_el1
	ret

get_mair_el2:
	mrs     x0, mair_el2
	ret

get_mair_el3:
	mrs     x0, mair_el3
	ret

get_tcr_el1:
	mrs     x0, tcr_el1
	ret

get_tcr_el2:
	mrs     x0, tcr_el2
	ret

get_tcr_el3:
	mrs     x0, tcr_el3
	ret

get_vtcr_el2:
	mrs     x0, vtcr_el2
	ret

set_tcr_el1:
	msr     tcr_el1, x0
	ret

set_tcr_el2:
	msr     tcr_el2, x0
	ret

set_tcr_el3:
	msr     tcr_el3, x0
	ret

set_vtcr_el2:
	msr     vtcr_el2, x0
	ret

get_ttbr0_el1:
	mrs     x0, ttbr0_el1
	ret

get_ttbr0_el2:
	mrs     x0, ttbr0_el2
	ret

get_ttbr0_el3:
	mrs     x0, ttbr0_el3
	ret

get_ttbr1_el1:
	mrs     x0, ttbr1_el1
	ret

get_vttbr_el2:
	mrs     x0, vttbr_el2
	ret

set_ttbr0_el1:
	msr     ttbr0_el1, x0
	ret

set_ttbr0_el2:
	msr     ttbr0_el2, x0
	ret

set_ttbr0_el3:
	msr     ttbr0_el3, x0
	ret

set_ttbr1_el1:
	msr     ttbr1_el1, x0
	ret

set_ttbr_el2:
	msr     vttbr_el2, x0
	ret

get_daif:
	mrs	x0, daif
	ret

set_daif:
	msr	daif, x0
	ret


// ------------------------------------------------------------
// get c-bit from current els sctlr register
// unsigned u32 is_dcache_enable_d(void)
is_dcache_enable_d:
	mrs     x0, CurrentEL
	lsr     x0, x0, #2
	cmp     x0, #1
	b.eq    1f
	cmp     x0, #2
	b.eq    2f
	mrs     x0, sctlr_el3
	b       9f
2:
	mrs     x0, sctlr_el2
	b       9f
1:
	mrs     x0, sctlr_el1
	b       9f
9:
	and     w0, w0, #0x04                   // sctlr_elx.c bit
	ret


//------------------------------------------------------------------------------
//- spr access
//------------------------------------------------------------------------------
	.global get_scr_el3
	.global set_scr_el3
	.global get_hcr_el2
	.global set_hcr_el2

	.global get_esr_el1
	.global get_esr_el2
	.global get_esr_el3
	.global get_elr_el1
	.global get_elr_el2
	.global get_elr_el3
	.global set_elr_el1
	.global set_elr_el2
	.global set_elr_el3
	.global get_spsr_el1
	.global get_spsr_el2
	.global get_spsr_el3
	.global set_spsr_el1
	.global set_spsr_el2
	.global set_spsr_el3

	.global get_vbar_el1
	.global get_vbar_el2
	.global get_vbar_el3
	.global set_vbar_el1
	.global set_vbar_el2
	.global set_vbar_el3

	.global get_sp_el0
	.global get_sp_el1
	.global get_sp_el2
	.global set_sp_el0
	.global set_sp_el1
	.global set_sp_el2
	.global get_sp_CurrentEL
	.global set_sp_CurrentEL


get_scr_el3:
	mrs     x0, scr_el3
	ret

set_scr_el3:
	msr     scr_el3, x0
	ret

get_hcr_el2:
	mrs     x0, hcr_el2
	ret

set_hcr_el2:
	msr     hcr_el2, x0
	ret

get_esr_el1:
	mrs     x0, esr_el1
	ret

get_esr_el2:
	mrs     x0, esr_el2
	ret

get_esr_el3:
	mrs     x0, esr_el3
	ret

get_elr_el1:
	mrs     x0, elr_el1
	ret

get_elr_el2:
	mrs     x0, elr_el2
	ret

get_elr_el3:
	mrs     x0, elr_el3
	ret

set_elr_el1:
	msr     elr_el1, x0
	ret

set_elr_el2:
	msr     elr_el2, x0
	ret

set_elr_el3:
	msr     elr_el3, x0
	ret

get_spsr_el1:
	mrs     x0, spsr_el1
	ret

get_spsr_el2:
	mrs     x0, spsr_el2
	ret

get_spsr_el3:
	mrs     x0, spsr_el3
	ret

set_spsr_el1:
	msr     spsr_el1, x0
	ret

set_spsr_el2:
	msr     spsr_el2, x0
	ret

set_spsr_el3:
	msr     spsr_el3, x0
	ret


get_sp_el0:
	mrs     x0, sp_el0
	ret

get_sp_el1:
	mrs     x0, sp_el1
	ret

get_sp_el2:
	mrs     x0, sp_el2
	ret

set_sp_el0:
	msr     sp_el0, x0
	ret

set_sp_el1:
	msr     sp_el1, x0
	ret

set_sp_el2:
	msr     sp_el2, x0
	ret

get_sp_CurrentEL:
	mov     x0, sp
	ret

set_sp_CurrentEL:
	mov     sp, x0
	ret

#if 0
get_vbar_el1:
	mrs     x0, vbar_el1
	ret

get_vbar_el2:
	mrs     x0, vbar_el2
	ret

get_vbar_el3:
	mrs     x0, vbar_el3
	ret

set_vbar_el1:
	msr     vbar_el1, x0
	ret

set_vbar_el2:
	msr     vbar_el2, x0
	ret

set_vbar_el3:
	msr     vbar_el3, x0
	ret
#endif

/*Keep it here! Used for ASP tests */
	.global vbar_el3_swap
vbar_el3_swap:
	MRS	x1, vbar_el3
	MSR	vbar_el3, x0
	MOV	x0, x1
	RET

	.global vbar_el1_swap
vbar_el1_swap:
	MRS	x1, vbar_el1
	MSR	vbar_el1, x0
	MOV	x0, x1
	RET

//------------------------------------------------------------------------------
//- implementation defined sprs for aarch64
//------------------------------------------------------------------------------

	.global get_l2ctlr_el1
	.global set_l2ctlr_el1
	.global get_l2ectlr_el1
	.global set_l2ectlr_el1
	.global get_l2actlr_el1
	.global set_l2actlr_el1
	.global get_cpuactlr_el1
	.global set_cpuactlr_el1
	.global get_cpuectlr_el1
	.global set_cpuectlr_el1

get_l2ctlr_el1:
	mrs     x0,s3_1_c11_c0_2
	ret

set_l2ctlr_el1:
	msr     s3_1_c11_c0_2,x0
	ret

get_l2ectlr_el1:
	mrs     x0,s3_1_c11_c0_3
	ret

set_l2ectlr_el1:
	msr     s3_1_c11_c0_3,x0
	ret

get_l2actlr_el1:
	mrs     x0,s3_1_c15_c0_0
	ret

set_l2actlr_el1:
	msr     s3_1_c15_c0_0,x0
	ret

get_cpuactlr_el1:
	mrs     x0, s3_1_c15_c2_0
	ret

set_cpuactlr_el1:
	msr     s3_1_c15_c2_0,x0
	ret

get_cpuectlr_el1:
	mrs     x0, s3_1_c15_c2_1
	ret

set_cpuectlr_el1:
	msr     s3_1_c15_c2_1, x0
	ret


//------------------------------------------------------------------------------
//- aarch64 architectural feature trap registers
//------------------------------------------------------------------------------

	.global get_cptr_el3
	.global get_cptr_el2
	.global get_cpacr_el1
	.global set_cptr_el3
	.global set_cptr_el2
	.global set_cpacr_el1
	.global get_actlr_el3
	.global set_actlr_el3
	.global get_actlr_el2
	.global set_actlr_el2

get_cptr_el3:
	mrs     x0, cptr_el3
	ret

get_cptr_el2:
	mrs     x0, cptr_el2
	ret

get_cpacr_el1:
	mrs     x0, cpacr_el1
	ret

set_cptr_el3:
	msr     cptr_el3, x0
	ret

set_cptr_el2:
	msr     cptr_el2, x0
	ret

set_cpacr_el1:
	msr     cpacr_el1, x0
	ret


get_actlr_el3:
	mrs     x0, actlr_el3
	ret

set_actlr_el3:
	msr     actlr_el3, x0
	ret

get_actlr_el2:
	mrs     x0, actlr_el2
	ret

set_actlr_el2:
	msr     actlr_el2, x0
	ret


//------------------------------------------------------------------------------
//- tlb operations
//------------------------------------------------------------------------------

	.global invalidate_tlb_alle1
	.global invalidate_tlb_alle2
	.global invalidate_tlb_alle3
	.global invalidate_tlb_alle1is
	.global invalidate_tlb_alle2is
	.global invalidate_tlb_alle3is
	.global invalidate_tlb_vmalle1
	.global invalidate_tlb_vmalle1is

invalidate_tlb_alle1:
	dsb		ishst
	tlbi    alle1                           // all stage 1 tlb at el1
	dsb		ish
	ret				                        // can be executed in el2, el3 only

invalidate_tlb_alle2:
	dsb		ishst
	tlbi    alle2                           // all stage 1 tlb at el1
	dsb		ish
	ret

invalidate_tlb_alle3:
	dsb		ishst
	tlbi    alle3                           // all stage 1 tlb at el1
	mov     x0, #0
	tlbi    vae3, x0
	tlbi    vae3is, x0
	dsb		ish
	ret

invalidate_tlb_alle1is:
	dsb		ishst
	tlbi    alle1is                         // all stage 1 in-shared tlb at el1
	dsb		ish
	ret                                     // can be executed in el2, el3 only

invalidate_tlb_alle2is:
	dsb		ishst
	tlbi    alle2is                         // all stage 1 in-shared tlb at el2
	dsb		ish
	ret

invalidate_tlb_alle3is:
	dsb		ishst
	tlbi    alle3is                         // all stage 1 in-shared tlb at el3
	dsb		ish
	ret

invalidate_tlb_vmalle1:
	dsb		ishst
	tlbi    vmalle1                         // all stage 1 tlb at el1 (cur vmid)
	dsb		ish
	ret

invalidate_tlb_vmalle1is:
	dsb		ishst
	tlbi    vmalle1is                       // all stage 1 tlb at el1 (cur vmid)
	dsb		ish
	ret


//------------------------------------------------------------------------------
//- Cache maintenance operations
//------------------------------------------------------------------------------

	.global invalidate_icache_all_is
	.global invalidate_icache_all
	.global invalidate_icache_va

	.global clean_dcache_level
	.global clean_dcache_all
	.global clean_dcache_range

	.global invalidate_dcache_level
	.global invalidate_dcache_all
	.global invalidate_dcache_range

	.global clean_invalidate_dcache_level
	.global clean_invalidate_dcache_all
	.global clean_invalidate_dcache_range

	.global get_dcache_size_level
	.global get_dcache_linesize_level
	.global get_dcache_waynum_level
	.global get_dcache_setnum_level

// ------------------------------------------------------------
// void invalidate_icache_all_is(void)
// invalidate i cache all, inner shareable.
invalidate_icache_all_is:
	ic      ialluis
	isb     sy
	ret

// ------------------------------------------------------------
// void invalidate_icache_all(void)
// invalidate i cache all.
invalidate_icache_all:
	ic      iallu
	isb     sy
	ret

// ------------------------------------------------------------
// void invalidate_icache_va(void *)
// invalidate i cache by va.
// x0: virtural address
invalidate_icache_va:
	ic      ivau, x0
	isb     sy
	ret

// ------------------------------------------------------------
// void clean_Dcache_Level(level)
// clean one level cache.
// x0: cache level
// x1~x9: clobbered
clean_dcache_level:
	lsl	    x1, x0, #1
	msr     csselr_el1, x1                  // select cache level
	isb                                     // isb to sych the new cssr&csidr

	mrs     x4, ccsidr_el1                  // w4 = csidr

	ubfx    w3, w4, #0, #3
	add     w3, w3, #4                      // w3 = log2(line size)
	ubfx    w5, w4, #13, #15
	ubfx    w4, w4, #3, #10                 // w4 = way number
	clz     w6, w4                          // w6 = 32 - log2(number of ways)
1:	//invalidatecaches_flush_set
	mov     w8, w4                          // w8 = way number
2:	//invalidatecaches_flush_way
	mov     w7, w1                          // fill level field
	lsl     w9, w5, w3
	orr     w7, w7, w9                      // fill index field
	lsl     w9, w8, w6
	orr     w7, w7, w9                      // fill way field
	dc      csw, x7                         // invalidate by set/way to point of coherency
	subs    w8, w8, #1                      // decrement way
	b.ge    2b
	subs    w5, w5, #1                      // descrement set
	b.ge    1b

	ret

// ------------------------------------------------------------
// void invalidate_dcache_level(level)
// invalidate one level cache.
// x0: cache level
// x1~x9: clobbered
invalidate_dcache_level:
	lsl	    x1, x0, #1
	msr     csselr_el1, x1                  // select cache level
	isb                                     // isb to sych the new cssr&csidr

	mrs     x4, ccsidr_el1                  // w4 = csidr

	ubfx    w3, w4, #0, #3
	add     w3, w3, #4                      // w3 = log2(line size)
	ubfx    w5, w4, #13, #15
	ubfx    w4, w4, #3, #10                 // w4 = way number
	clz     w6, w4                          // w6 = 32 - log2(number of ways)
1:	//invalidatecaches_flush_set
	mov     w8, w4                          // w8 = way number
2:	//invalidatecaches_flush_way
	mov     w7, w1                          // fill level field
	lsl     w9, w5, w3
	orr     w7, w7, w9                      // fill index field
	lsl     w9, w8, w6
	orr     w7, w7, w9                      // fill way field
	dc      isw, x7                         // invalidate by set/way to point of coherency
	subs    w8, w8, #1                      // decrement way
	b.ge    2b
	subs    w5, w5, #1                      // descrement set
	b.ge    1b

	ret

// ------------------------------------------------------------
// void clean_invalidate_dcache_level(level)
// clean and invalidate one level cache.
// x0: cache level
// x1~x9: clobbered

clean_invalidate_dcache_level:
	lsl	    x1, x0, #1
	msr     csselr_el1, x1                  // select cache level
	isb                                     // isb to sych the new cssr&csidr

	mrs     x4, ccsidr_el1                  // w4 = csidr

	ubfx    w3, w4, #0, #3
	add     w3, w3, #4                      // w3 = log2(line size)
	ubfx    w5, w4, #13, #15
	ubfx    w4, w4, #3, #10                 // w4 = way number
	clz     w6, w4                          // w6 = 32 - log2(number of ways)
1:	//invalidatecaches_flush_set
	mov     w8, w4                          // w8 = way number
2:	//invalidatecaches_flush_way
	mov     w7, w1                          // fill level field
	lsl     w9, w5, w3
	orr     w7, w7, w9                      // fill index field
	lsl     w9, w8, w6
	orr     w7, w7, w9                      // fill way field
	dc      cisw, x7                        // invalidate by set/way to point of coherency
	subs    w8, w8, #1                      // decrement way
	b.ge    2b
	subs    w5, w5, #1                      // descrement set
	b.ge    1b
	ret

// ------------------------------------------------------------
// void clean_dcache_all(void)
// clean all data cache by set/way. (l1, l2)
clean_dcache_all:
	dsb     sy
	mov     x15, x30
	mrs     x10, clidr_el1                  // read clidr
	lsr     x11, x10, #24
	and     x11, x11, #0x7                  // x11 = loc
	cbz     x11, 3f                         // if loc = 0, no need to clean
	mov     x0, #0                          // start flush at cache level 0
	// x0 = level
	// x10 = clidr_el1
	// x11 = loc
1:
	lsl     x1, x0, #1
	add     x1, x1, x0                      // x0 = 3x cache level
	lsr     x1, x10, x1
	and     x1, x1, #7                      // x1 = cache type
	cmp     x1, #2
	b.lt    2f                              // skip if no cache or icache
	bl      clean_dcache_level              // clean dcache of level
2:
	add     x0, x0, #1                      // increment cache level
	cmp     x11, x0
	b.gt    1b
3:
	mov     x0, #0
	msr     csselr_el1, x0                  // swith back to cache level 0
	dsb     sy
	isb
	ret     x15

// ------------------------------------------------------------
// void invalidate_dcache_all(void)
// invalidate all data cache by set/way. (l1, l2)
invalidate_dcache_all:
	dsb     sy
	mov     x15, x30
	mrs     x10, clidr_el1                  // read clidr
	lsr     x11, x10, #24
	and     x11, x11, #0x7                  // x11 = loc
	cbz     x11, 3f                         // if loc = 0, no need to clean
	mov     x0, #0                          // start flush at cache level 0
	// x0 = level
	// x10 = clidr_el1
	// x11 = loc
1:
	lsl     x1, x0, #1
	add     x1, x1, x0                      // x0 = 3x cache level
	lsr     x1, x10, x1
	and     x1, x1, #7                      // x1 = cache type
	cmp     x1, #2
	b.lt    2f                              // skip if no cache or icache
	bl      invalidate_dcache_level         // ivalidate dcache of level
2:
	add     x0, x0, #1                      // increment cache level
	cmp     x11, x0
	b.gt    1b
3:
	mov     x0, #0
	msr     csselr_el1, x0                  // swith back to cache level 0
	dsb     sy
	isb
	ret     x15

// ------------------------------------------------------------
// void clean_invalidate_dcache_all(void)
// invalidate all data cache by set/way. (l1, l2)
clean_invalidate_dcache_all:
	dsb     sy
	mov     x15, x30
	mrs     x10, clidr_el1                  // read clidr
	lsr     x11, x10, #24
	and     x11, x11, #0x7                  // x11 = loc
	cbz     x11, 3f                         // if loc = 0, no need to clean
	mov     x0, #0                          // start flush at cache level 0
	// x0 = level
	// x10 = clidr_el1
	// x11 = loc
1:
	lsl     x1, x0, #1
	add     x1, x1, x0                      // x0 = 3x cache level
	lsr     x1, x10, x1
	and     x1, x1, #7                      // x1 = cache type
	cmp     x1, #2
	b.lt    2f                              // skip if no cache or icache
	bl      clean_invalidate_dcache_level   // flush dcache of level
2:
	add     x0, x0, #1                      // increment cache level
	cmp     x11, x0
	b.gt    1b
3:
	mov     x0, #0
	msr     csselr_el1, x0                  // swith back to cache level 0
	dsb     sy
	isb
	ret     x15

// ------------------------------------------------------------
// void clean_dcache_range(start, end)
// clean data cache in the range
// x0: start address
// x1: end address
clean_dcache_range:
	mrs     x3, ctr_el0                     // read ctr
	lsr     x3, x3, #16
	and     x3, x3, #0xf                    // cache line size encoding
	mov     x2, #4                          // bytes per word
	lsl     x2, x2, x3                      // actual cache line size
	// x2 = minimal cache line size in cache system
	sub     x3, x2, #1
	bic     x0, x0, x3
1:
	dc      cvac, x0                       // clean d line / unified line
	add     x0, x0, x2
	cmp     x0, x1
	b.lo    1b
	dsb     sy
	ret

// ------------------------------------------------------------
// void invalidate_dcache_range(start, end)
// invalidate data cache in the range
// x0: start address
// x1: end address
invalidate_dcache_range:
	mrs     x3, ctr_el0                     // read ctr
	lsr     x3, x3, #16
	and     x3, x3, #0xf                    // cache line size encoding
	mov     x2, #4                          // bytes per word
	lsl     x2, x2, x3                      // actual cache line size
	// x2 = minimal cache line size in cache system
	sub     x3, x2, #1
	bic     x0, x0, x3
1:
	dc      ivac, x0                        // invalidate d line / unified line
	add     x0, x0, x2
	cmp     x0, x1
	b.lo    1b
	dsb     sy
	ret

// ------------------------------------------------------------
// void clean_invalidate_dcache_range(start, end)
// clean and invalidate data cache in the range
// x0: start address
// x1: end address
clean_invalidate_dcache_range:
	mrs     x3, ctr_el0                     // read ctr
	lsr     x3, x3, #16
	and     x3, x3, #0xf                    // cache line size encoding
	mov     x2, #4                          // bytes per word
	lsl     x2, x2, x3                      // actual cache line size
	// x2 = minimal cache line size in cache system
	sub     x3, x2, #1
	bic     x0, x0, x3
1:
	dc      civac, x0                       // clean&inv d line / unified line
	add     x0, x0, x2
	cmp     x0, x1
	b.lo    1b
	dsb     sy
	ret

get_dcache_size_level:
	lsl	    x1, x0, #1
	msr     csselr_el1, x1                  // select cache level
	isb                                     // isb to sych the new cssr&csidr
	mrs     x6, ccsidr_el1                  // x6 = read the new ccsidr

	and     x2, x6, #7                      // linesize field
	add     x2, x2, #4                      // linesize field + 4
	mov     x1, #0x1
	lsl     x2, x1, x2						// x2 = line size=2^(linesizefield+4)

	mov     x3, #0x3ff
	and     x3, x3, x6, lsr #3
	add     x3, x3, #0x1                    // x3 = way number

	mov     x4, #0x7fff
	and     x4, x4, x6, lsr #13
	add     x4, x4, #0x1                    // x4 = set number

	mul     x0, x2, x3
	mul     x0, x0, x4                      // total size = line*way*set
	ret

get_dcache_linesize_level:
	lsl	    x1, x0, #1
	msr     csselr_el1, x1                  // select cache level
	isb                                     // isb to sych the new cssr&csidr
	mrs     x6, ccsidr_el1                  // x6 = read the new ccsidr

	and     x2, x6, #7                      // linesize field
	add     x2, x2, #4                      // linesize field + 4
	mov     x1, #0x1
	lsl     x0, x1, x2						// line size=2^(linesizefield+4)

	ret

get_dcache_waynum_level:
	lsl	    x1, x0, #1
	msr     csselr_el1, x1                  // select cache level
	isb                                     // isb to sych the new cssr&csidr
	mrs     x6, ccsidr_el1                  // x6 = read the new ccsidr

	mov     x3, #0x3ff
	and     x3, x3, x6, lsr #3
	add     x0, x3, #0x1                    // way number
	ret

get_dcache_setnum_level:
	lsl	    x1, x0, #1
	msr     csselr_el1, x1                  // select cache level
	isb                                     // isb to sych the new cssr&csidr
	mrs     x6, ccsidr_el1                  // x6 = read the new ccsidr

	mov     x4, #0x7fff
	and     x4, x4, x6, lsr #13
	add     x0, x4, #0x1                    // set number
	ret

	.global smc
smc:
	dsb	sy
	smc #0x0
	ret

	.global hvc
hvc:
	dsb	sy
	hvc #0x0
	ret

	.global enable_icache
enable_icache:
	mrs     x0, CurrentEL
	lsr     x0, x0, #2
3:
	cmp     x0, #0x3
	b.ne    2f
	mrs     x0, sctlr_el3
	orr     x0, x0, #(1<<12)
	msr     sctlr_el3, x0
	b       9f
2:
	cmp     x0, #0x2
	b.ne    1f
	mrs     x0, sctlr_el2
	orr     x0, x0, #(1<<12)
	msr     sctlr_el2, x0
	b       9f
1:
	mrs     x0, sctlr_el1
	orr     x0, x0, #(1<<12)
	msr     sctlr_el1, x0
9:
	ret

	.global disable_icache
disable_icache:
	mrs     x0, CurrentEL
	lsr     x0, x0, #2
3:
	cmp     x0, #0x3
	b.ne    2f
	mrs     x0, sctlr_el3
	and     x0, x0, #~(1<<12)
	msr     sctlr_el3, x0
	b       9f
2:
	cmp     x0, #0x2
	b.ne    1f
	mrs     x0, sctlr_el2
	and     x0, x0, #~(1<<12)
	msr     sctlr_el2, x0
	b       9f
1:
	mrs     x0, sctlr_el1
	and     x0, x0, #~(1<<12)
	msr     sctlr_el1, x0
9:
	ret

	.global enable_dcache
enable_dcache:
	mrs     x0, CurrentEL
	lsr     x0, x0, #2
3:
	cmp     x0, #0x3
	b.ne    2f
	mrs     x0, sctlr_el3
	orr     x0, x0, #(1<<2)                // c bit
	msr     sctlr_el3, x0
	b       9f
2:
	cmp     x0, #0x2
	b.ne    1f
	mrs     x0, sctlr_el2
	orr     x0, x0, #(1<<2)                // c bit
	msr     sctlr_el2, x0
	b       9f
1:
	mrs     x0, sctlr_el1
	orr     x0, x0, #(1<<2)                // c bit
	msr     sctlr_el1, x0
9:
	isb
	ret

	.global disable_dcache
disable_dcache:
	mrs     x0, CurrentEL
	lsr     x0, x0, #2
3:
	cmp     x0, #0x3
	b.ne    2f
	mrs     x0, sctlr_el3
	and     x0, x0, #~(1<<2)                // c bit
	msr     sctlr_el3, x0
	b       9f
2:
	cmp     x0, #0x2
	b.ne    1f
	mrs     x0, sctlr_el2
	and     x0, x0, #~(1<<2)                // c bit
	msr     sctlr_el2, x0
	b       9f
1:
	mrs     x0, sctlr_el1
	and     x0, x0, #~(1<<2)                // c bit
	msr     sctlr_el1, x0
9:
	isb
	ret

	.global enable_mmu
enable_mmu:
	mrs     x0, CurrentEL
	lsr     x0, x0, #2
3:
	cmp     x0, #0x3
	b.ne    2f
	mrs     x0, sctlr_el3
	orr     x0, x0, #(1<<0)
	msr     sctlr_el3, x0
	b       9f
2:
	cmp     x0, #0x2
	b.ne    1f
	mrs     x0, sctlr_el2
	orr     x0, x0, #(1<<0)
	msr     sctlr_el2, x0
	b       9f
1:
	mrs     x0, sctlr_el1
	orr     x0, x0, #(1<<0)
	msr     sctlr_el1, x0
9:
	isb
	ret

	.global disable_mmu
disable_mmu:
	mrs     x0, CurrentEL
	lsr     x0, x0, #2
3:
	cmp     x0, #0x3
	b.ne    2f
	mrs     x0, sctlr_el3
	and     x0, x0, #~(1<<0)
	msr     sctlr_el3, x0
	b       9f
2:
	cmp     x0, #0x2
	b.ne    1f
	mrs     x0, sctlr_el2
	and     x0, x0, #~(1<<0)
	msr     sctlr_el2, x0
	b       9f
1:
	mrs     x0, sctlr_el1
	and     x0, x0, #~(1<<0)
	msr     sctlr_el1, x0
9:
	isb
	ret

	.global enable_mmu_dcache
enable_mmu_dcache:
	mrs     x0, CurrentEL
	lsr     x0, x0, #2
3:
	cmp     x0, #0x3
	b.ne    2f
	mrs     x0, sctlr_el3
	orr     x0, x0, #(1<<0)
	orr     x0, x0, #(1<<2)
	msr     sctlr_el3, x0
	b       9f
2:
	cmp     x0, #0x2
	b.ne    1f
	mrs     x0, sctlr_el2
	orr     x0, x0, #(1<<0)
	orr     x0, x0, #(1<<2)
	msr     sctlr_el2, x0
	b       9f
1:
	mrs     x0, sctlr_el1
	orr     x0, x0, #(1<<0)
	orr     x0, x0, #(1<<2)
	msr     sctlr_el1, x0
9:
	isb
	ret

	.global disable_mmu_dcache
disable_mmu_dcache:
	mrs     x0, CurrentEL
	lsr     x0, x0, #2
3:
	cmp     x0, #0x3
	b.ne    2f
	mrs     x0, sctlr_el3
	and     x0, x0, #~(1<<0)
	and     x0, x0, #~(1<<2)
	msr     sctlr_el3, x0
	b       9f
2:
	cmp     x0, #0x2
	b.ne    1f
	mrs     x0, sctlr_el2
	and     x0, x0, #~(1<<0)
	and     x0, x0, #~(1<<2)
	msr     sctlr_el2, x0
	b       9f
1:
	mrs     x0, sctlr_el1
	and     x0, x0, #~(1<<0)
	and     x0, x0, #~(1<<2)
	msr     sctlr_el1, x0
9:
	isb
	ret

#if 0
	.global  Calc64bitCheckSum
Calc64bitCheckSum:
#if 0
	mov		x3,	#0                          // x3 = accumulator, init 0
checksum64_loop:
	ldr     x2, [x0], #8                    // x0 = start address
	add     x3, x3, x2                      // accumulate datas
	subs    x1, x1, #1                      // decrease count
	b.ne    checksum64_loop                 // loop
	mov		x0, x3
	ret
#else
	mov	x2, #0
	cmp	w1, #7
	b.ls	Calc64bitCheckSum_last
checksum64_loop:
;	prfm    pldl1keep, [x0,#0]
;	prfm    pldl1keep, [x0,#64]
;	prfm    pldl1keep, [x0,#128]
;	prfm    pldl1keep, [x0,#192]
	prfm    pldl1keep, [x0,#256]
;	prfm    pldl1keep, [x0,#320]
	ldp	x3, x4, [x0, #0]
	ldp	x5, x6, [x0, #16]
	ldp	x7, x8, [x0, #32]
	ldp	x9, x10, [x0, #48]

	add     x2, x2, x3
	add     x2, x2, x4
	add     x2, x2, x5
	add     x2, x2, x6
	add     x2, x2, x7
	add     x2, x2, x8
	add     x2, x2, x9
	add     x2, x2, x10

	add	x0, x0, #64
	subs    x1, x1, #8
	cmp	x1, #7
	b.hi	checksum64_loop
Calc64bitCheckSum_last:
        cmp	x1,#0
        b.eq	Calc64bitChecksum_end
	ldr     x3, [x0]

	add     x2, x2, x3
	add	x0, x0, #4
	subs    x1, x1, #1
	b.ne	Calc64bitCheckSum_last
Calc64bitChecksum_end:
	mov	x0, x2
	ret
#endif

;	.align 4
	.global Copy8dw
Copy8dw:
	prfm    pldl1keep, [x0,#64]
	prfm    pldl1keep, [x0,#128]
	ldp	x3, x4, [x0, #0]
	ldp	x5, x6, [x0, #16]
	ldp	x7, x8, [x0, #32]
	ldp	x9, x10, [x0, #48]
	add	x0, x0, #64

	stp	x3, x4, [x1, #0]
	stp	x5, x6, [x1, #16]
	stp	x7, x8, [x1, #32]
	subs    w2, w2, #1
	stp	x9, x10, [x1, #48]
	add	x1, x1, #64

	cmp	w2, #0
	b.ne	Copy8dw
	ret

	.align 4
	.global MemClear
MemClear:
	mov	x7, 0
MemClear_sub:
	stp	x7, x7, [x0]
	stp	x7, x7, [x0, #16]
	stp	x7, x7, [x0, #32]
	sub     w1, w1, #1
	stp	x7, x7, [x0, #48]
	add	x0, x0, #0x40

	cmp	w1, #0
	b.ne	MemClear_sub
	ret

	.align 4
	.global Calc32bitCheckSum
Calc32bitCheckSum:
	mov	x2, #0
	cmp	w1, #7
	b.ls	Calc32bitCheckSum_last
checksum32_loop:
	prfm    pldl1keep, [x0,#256]
	ldp	w3, w4, [x0, #0]
	ldp	w5, w6, [x0, #8]
	ldp	w7, w8, [x0, #16]
	ldp	w9, w10, [x0, #24]

	add     x2, x2, x3
	add     x2, x2, x4
	add     x2, x2, x5
	add     x2, x2, x6
	add     x2, x2, x7
	add     x2, x2, x8
	add     x2, x2, x9
	add     x2, x2, x10

	add	x0, x0, #32
	subs    w1, w1, #8
	cmp	w1, #7
	b.hi	checksum32_loop
Calc32bitCheckSum_last:
        cmp	w1,#0
        b.eq	Calc32bitChecksum_end
	ldr     w3, [x0]

	add     x2, x2, x3
	add	x0, x0, #4
	subs    w1, w1, #1
	b.ne	Calc32bitCheckSum_last
Calc32bitChecksum_end:
	mov	x0, x2
	ret

	.global SetPhysicalUser
SetPhysicalUser:
	MRS	x0, CNTKCTL_EL1
	AND	x0, x0, #~(3<<8)
	AND x0, x0, #~(1<<0)
	ORR	x0, x0, #(1<<1)
	RET

	.global SetPPhysicalTimerCompVal
SetPPhysicalTimerCompVal:
	msr		CNTP_CVAL_EL0, x0
	ret

	.global	GetPPhysicalTimerCompVal
GetPPhysicalTimerCompVal:
	mrs     x0, CNTP_CVAL_EL0
	ret

	.global SetPPhysicalTimerVal
SetPPhysicalTimerVal:
	msr		CNTP_TVAL_EL0, x0
	ret

	.global EnablePPhysicalTimer
EnablePPhysicalTimer:
	mrs		x0, CNTP_CTL_EL0
	and		x0, x0, #~(1 << 1)
	orr		x0, x0, #(1 << 0)
	msr		CNTP_CTL_EL0, x0
	ret

	.global DisablePPhysicalTimer
DisablePPhysicalTimer:
	mrs		x0, CNTP_CTL_EL0
	and		x0, x0, #~(1 << 0)
	msr		CNTP_CTL_EL0, x0
	ret

	.global GetPhysicalCountTime
GetPhysicalCountTime:
	isb
	mrs		x0, CNTPCT_EL0
	ret

	.global GetPhysicalTimerConReg
GetPhysicalTimerConReg:
	mrs		x0, CNTP_CTL_EL0
	ret

	.global SetSPhysicalTimerCompVal
SetSPhysicalTimerCompVal:
	msr		CNTPS_CVAL_EL1, x0
	ret

	.global	GetSPhysicalTimerCompVal
GetSPhysicalTimerCompVal:
	mrs     x0, CNTPS_CVAL_EL1
	ret

	.global SetSPhysicalTimerVal
SetSPhysicalTimerVal:
	msr		CNTPS_TVAL_EL1, x0
	ret

	.global EnableSPhysicalTimer
EnableSPhysicalTimer:
	mrs		x0, CNTPS_CTL_EL1
	and		x0, x0, #~(1 << 1)
	orr		x0, x0, #(1 << 0)
	msr		CNTPS_CTL_EL1, x0
	ret

	.global DisableSPhysicalTimer
DisableSPhysicalTimer:
	mrs		x0, CNTPS_CTL_EL1
	and		x0, x0, #~(1 << 0)
	msr		CNTPS_CTL_EL1, x0
	ret


	.global SecondaryLinuxCall
SecondaryLinuxCall:
	MOV	x8, x0
	MOV	x0, #0
	MOV	x1, #0
	MOV	x2, #0
	MOV	x3, #0
	MOV	x4, #0
	MOV	x5, #0
	MOV	x6, #0
	MOV	x7, #0

	BLR	x8

	RET


	.global PrimaryLinuxCall
PrimaryLinuxCall:
	MOV	x8, x0
	MOV	x0, x1
	MOV	x1, #0
	MOV	x2, #0
	MOV	x3, #0
	MOV	x4, #0
	MOV	x5, #0
	MOV	x6, #0
	MOV	x7, #0

	BLR	x8

	RET

	.global co_get_daif
co_get_daif:
	mrs	x0, ISR_EL1
	RET
#endif

#if 0
	.global sarc_dc_cvac_line
sarc_dc_cvac_line:
	dc	cvac, x0
	dsb	sy
	ret

	.global sarc_ic_ivau_line
sarc_ic_ivau_line:
	ic	ivau, x0
	dsb	ish
	isb
	ret
#endif
