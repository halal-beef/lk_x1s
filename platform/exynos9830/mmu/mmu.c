/*
 *  ==============================================================================
 *
 *       Filename:  mmu.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2014-05-24
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Kim, Young-san (ys0225.kim@samsung.com)
 *   Organization:  AP Development, System LSI Business
 *
 *  ==============================================================================
 */

#include <assert.h>
#include <platform/mmu/mmu.h>
#include <platform/mmu/cpu_a.h>
#include <platform/mmu/types.h>

void init_mair(CPU_EL_LEVEL cur_el)
{
	/* last index [64:57] is used for magic number : 0xB*/
	switch (cur_el) {
	case CPU_EL1:
		set_mair_el1(MAIR_EL1_ATTR);
		break;
	default:
		ASSERT(0);
		break;
	}
}

static void init_mair_clear(CPU_EL_LEVEL cur_el)
{

	/* last index [64:57] is used for magic number : 0xB*/
	switch (cur_el) {
	case CPU_EL1:
		set_mair_el1(MM_ATTR_ALL_INVALID);
		break;
	default:
		ASSERT(0);
		break;
	}
}

u32 get_mair_attr_index(CPU_EL_LEVEL cur_el, u32 attr)
{
	u64 ll_mair, t_ll_mair;
	u32 i;
	ll_mair = 0;
	t_ll_mair = 0;
	/* last index [64:57] is used for magic number : 0xB*/
	switch (cur_el) {
	case CPU_EL1:
		ll_mair = get_mair_el1();
		break;
	default:
		ASSERT(0);
		break;
	}
	/* check MAIR magic num */
	if ((t_ll_mair & 0xFF00000000000000) != MM_ATTR_MAGIC)
		/* need to initialize MAIR for current EL */
		init_mair_clear(cur_el);

	for (i = 0; i < 7; i++) {
		if (attr == (t_ll_mair & 0xFF))
			break;
		t_ll_mair >>= 8;
	}

	if (i > 6) { /* no attr in MAIR */
		t_ll_mair = ll_mair;
		for (i = 0; i < 7; i++) {
			if ((t_ll_mair & 0xFF) == MM_ATTR_INVALID) {
				/* add new */
				ll_mair = (ll_mair & ~(0xFF << (i * 8))) | attr << (i * 8);
				switch (cur_el) {
				case CPU_EL1:
					set_mair_el1(ll_mair);
					break;
				default:
					ASSERT(0);
					break;
				}
				break;
			}
			t_ll_mair >>=  8;
		}
	}

	ASSERT(i < 7);

	return i;
}

void set_mair_default(CPU_EL_LEVEL cur_el)
{
	u64 ll_mair;
	switch (cur_el) {
	case CPU_EL3:
		ll_mair = (MM_ATTR_NOR_NC << 0x00) /*  index 0 */
				  | (MM_ATTR_NOR_WBWARA << 0x08) /*  index 1 */
				  | (MM_ATTR_DEV_nGnRnE << 0x10) /*  index 2 */
				  | (MM_ATTR_DEV_nGnRE << 0x18) /*  index 3 */
				  | (MM_ATTR_DEV_nGnRnE << 0x20) /*  index 4 */
				  | (MM_ATTR_DEV_nGnRnE << 0x28) /*  index 5 */
				  | (MM_ATTR_DEV_nGnRnE << 0x30) /*  index 6 */
				  | (MM_ATTR_DEV_nGnRnE << 0x38);/*  index 7 */
		set_mair_el3(ll_mair);
		break;
	case CPU_EL2:
		ll_mair = (MM_ATTR_NOR_NC << 0x00) /*  index 0 */
				  | (MM_ATTR_NOR_WBWARA << 0x08) /*  index 1 */
				  | (MM_ATTR_DEV_nGnRnE << 0x10) /*  index 2 */
				  | (MM_ATTR_DEV_nGnRE << 0x18) /*  index 3 */
				  | (MM_ATTR_DEV_nGnRnE << 0x20) /*  index 4 */
				  | (MM_ATTR_DEV_nGnRnE << 0x28) /*  index 5 */
				  | (MM_ATTR_DEV_nGnRnE << 0x30) /*  index 6 */
				  | (MM_ATTR_DEV_nGnRnE << 0x38);/*  index 7 */
		set_mair_el2(ll_mair);
		break;
	case CPU_EL1:
		ll_mair = (MM_ATTR_NOR_NC << 0x00) /*  index 0 */
				  | (MM_ATTR_NOR_WBWARA << 0x08) /*  index 1 */
				  | (MM_ATTR_DEV_nGnRnE << 0x10) /*  index 2 */
				  | (MM_ATTR_DEV_nGnRE << 0x18) /*  index 3 */
				  | (MM_ATTR_DEV_nGnRnE << 0x20) /*  index 4 */
				  | (MM_ATTR_DEV_nGnRnE << 0x28) /*  index 5 */
				  | (MM_ATTR_DEV_nGnRnE << 0x30) /*  index 6 */
				  | (MM_ATTR_DEV_nGnRnE << 0x38);/*  index 7 */
		set_mair_el1(ll_mair);
		break;
	default:
		ASSERT(0);
		break;
	}
}

static void set_tcr_default(CPU_EL_LEVEL cur_el)
{
	u64 ll_temp_tcr = 0;
	switch (cur_el) {
	case CPU_EL1:
		ll_temp_tcr |= ((u64)0 << 38); /*  TIB1 - Top byte not ignored */
		ll_temp_tcr |= ((u64)0 << 37); /*  TIB0 - Top byte not ignored */
		ll_temp_tcr |= ((u64)1 << 36); /*  AS - ASID size 16bit */
		ll_temp_tcr |= ((u64)1 << 32); /*  IPS - IPA size - 36bit/64G */
		ll_temp_tcr |= ((u64)2 << 30); /*  TG1 - TTBR1_EL1 Granule size. 4KB */
		ll_temp_tcr |= ((u64)3 << 28); /*  SH1 - Inner shareable table walk */
		ll_temp_tcr |= ((u64)1 << 26); /*  OGRN1 - Outer WriteBack WriteAlloc Cacheable */
		ll_temp_tcr |= ((u64)1 << 24); /*  IGRN1 - Inner WriteBack WriteAlloc Cacheable */
		ll_temp_tcr |= ((u64)0 << 23); /*  EPD1 - table walk enable (disable) */
		ll_temp_tcr |= ((u64)0 << 22); /*  A1 - ASID selection from TTBR0 */
		ll_temp_tcr |= ((u64)28 << 16); /*  T1SZ - 2^(64-63) region size (1bit addressing) */
		ll_temp_tcr |= ((u64)0 << 14); /*  TG0 - TTBR0_EL1 Granule size. 4KB */
		ll_temp_tcr |= ((u64)3 << 12); /*  SH0 - Inner shareable table walk */
		ll_temp_tcr |= ((u64)1 << 10); /*  OGRN0 - Outer WriteBack WriteAlloc Cacheable */
		ll_temp_tcr |= ((u64)1 << 8); /*  IGRN0 - Inner WriteBack WriteAlloc Cacheable */
		ll_temp_tcr |= ((u64)0 << 4); /*  EPD0 - table walk enable (not disable) */
		ll_temp_tcr |= ((u64)28 << 0); /*  T0SZ - 2^(64-28) region size (36bit addressing) */
		set_tcr_el1(ll_temp_tcr);
		break;
	default:
		ASSERT(0);
		break;
	}
}

u64 get_tt_granule_size(CPU_EL_LEVEL cur_el)
{
	u64 size = 0;

	switch (cur_el) {
	case CPU_EL1:
		size = (get_tcr_el1() >> 14) & 0x3;
		break;
	default:
		ASSERT(0);
		break;
	}
	/* TG0[15:14]  =b00  granule size. b00 4KB, b01 64KB, b10 16KB */
	switch (size) {
	case 0:
		size = 0x1000;
		break;	/* 4KB */
	case 1:
		size = 0x10000;
		break;	/* 64KB */
	case 2:
		size = 0x4000;
		break;	/* 16KB */
	default:
		ASSERT(0);
		break;
	}

	return size;
}

u64 *get_tt_base(CPU_EL_LEVEL el, u32 table_level)
{
	u64 llp_ttbr = (u64)NULL;

	switch (el) {
	case CPU_EL1:
		switch (table_level) {
		case 0:
			llp_ttbr = (u64)TTBR0_EL1_L0;
			break;
		case 1:
			llp_ttbr = (u64)TTBR0_EL1_L1;
			break;
		case 2:
			llp_ttbr = (u64)TTBR0_EL1_L2;
			break;
		case 3:
			llp_ttbr = (u64)TTBR0_EL1_L3;
			break;
		}
		break;
	default:
		break;
	}
	return (u64 *)llp_ttbr;
}

static u64 get_tt_block_size(CPU_EL_LEVEL el, u32 table_level)
{
	u64 entry_num, granule_size, block_size = 0;

	granule_size = get_tt_granule_size(el);
	entry_num = (granule_size >> 3); /* divide by 8(64bit) */

	switch (table_level) {
	case 1:
		block_size = granule_size * entry_num * entry_num;
		break;
	case 2:
		block_size = granule_size * entry_num;
		break;
	case 3:
		block_size = granule_size;
		break;
	default:
		ASSERT(0);
		break;
	}
	return block_size;
}

static void set_ttbr0(CPU_EL_LEVEL el, u64 *ttbraddr)
{
	switch (el) {
	case CPU_EL1:
		set_ttbr0_el1(ttbraddr);
		/* MNGS EVT0 BUG 39040: bus error from TBW may cause WFI drop. */
		/*  not only TTBR1 but also TTBR0 can be accessed from speculatives, */
		/*  do not let TTBR0 or TTBR1 points garbage table - jyoul.kim-150409 */
		set_ttbr1_el1(ttbraddr);
		break;
	default:
		ASSERT(0);
		break;
	}
}


static void set_ttbr1(CPU_EL_LEVEL el, u64 *ttbraddr)
{
	switch (el) {
	case CPU_EL1:
		set_ttbr1_el1(ttbraddr);
		break;
	default:
		ASSERT(0);
		break;
	}
}


void invalidate_tlb_all(CPU_EL_LEVEL el)
{
	switch (el) {
	case CPU_EL1:
		//invalidate_tlb_alle1();
		break;
	case CPU_EL2:
		invalidate_tlb_alle2();
		break;
	case CPU_EL3:
		invalidate_tlb_alle3();
		break;
	default:
		ASSERT(0);
		break;
	}
}

void invalidate_tlb_is_all(CPU_EL_LEVEL el)
{
	switch (el) {
	case CPU_EL1:
		invalidate_tlb_vmalle1is();
		break;
	case CPU_EL2:
		invalidate_tlb_alle2is();
		break;
	case CPU_EL3:
		invalidate_tlb_alle3is();
		break;
	default:
		ASSERT(0);
		break;
	}
}

void set_tt_entry(u64 *va_start, u64 *va_end, u64 *pa_start, u64 attr)
{
	volatile u64 *tt;
	u64 ll_vs, ll_ve, ll_ps;
	u64 i, entry_num;
	CPU_EL_LEVEL cur_el;
	//u64 l2_block_size, granule_size;
	u64 l2_block_size;
	u64 attr_idx;

	//wfi();

	cur_el = get_current_el();
	l2_block_size = get_tt_block_size(cur_el, 2);
	//granule_size = get_tt_granule_size(cur_el);

	ll_vs = ((u64)va_start / l2_block_size) * l2_block_size;   /* align to block size */
	ll_ve = ((u64)va_end / l2_block_size) * l2_block_size;   /* align to block size */
	ll_ps = ((u64)pa_start / l2_block_size) * l2_block_size;   /* align to block size */

	entry_num = (ll_ve - ll_vs) / l2_block_size;
	if ((u64)va_end % l2_block_size) /* there is remainder */
		entry_num++;

	tt = get_tt_base(cur_el, 2) + (ll_vs / l2_block_size);

	attr_idx = attr;
	for (i = 0; i < entry_num; i++)
		*tt++ = (ll_ps + ((u64)l2_block_size * i)) | attr_idx;

}


void set_mmu_base_default(CPU_EL_LEVEL el)
{
	init_mair(el);

	set_ttbr0(el, get_tt_base(el, 1));

	set_ttbr1(el, get_tt_base(el, 1));

	set_tcr_default(el);
	dsb();
}


static void s5e9830_mmu_table_init(void)
{
	u32 i;
	//u64 *ll_ttb_l1, *ll_ttb_l2;
	u64 *ll_ttb_l1;
	CPU_EL_LEVEL cur_el;
	/* volatile u64 heap_base; */

	cur_el = get_current_el();

	/* init MAIR */
	/*	init_mair_Clr(cur_el); */
	init_mair(cur_el);

	/* get Table level 1,2 address */
	ll_ttb_l1 = get_tt_base(cur_el, 1);
	//ll_ttb_l2 = get_tt_base(cur_el, 2);

	//ASSERT(((u64)ll_ttb_l1 % (u64)get_tt_granule_size(cur_el)) == 0);
	//ASSERT(((u64)ll_ttb_l2 % (u64)get_tt_granule_size(cur_el)) == 0);

	/* Table Level 1 Init */

	for (i = 0; i < (get_tt_granule_size(cur_el) / sizeof(u64)); i++)
		ll_ttb_l1[i] = 0;

	for (i = 0; i < 48; i++)
		ll_ttb_l1[i] =
			((u64)get_tt_base(cur_el, 2) + (i * get_tt_granule_size(cur_el)))
			| TT_S1_TABLE;

	/* Generate L2 TT - Block */
	set_tt_entry((u64 *)0x00000000, (u64 *)0xBFFFFFFFF, (u64 *)0x00000000, TT_S1_FAULT);
	set_tt_entry((u64 *)0x02000000, (u64 *)0x021FFFFF, (u64 *)0x02000000, TT_DEVICE);
	set_tt_entry((u64 *)0x03000000, (u64 *)0x031FFFFF, (u64 *)0x03000000, TT_PERI);
	set_tt_entry((u64 *)0x04000000, (u64 *)0x05FFFFFF, (u64 *)0x04000000, TT_PERI);
	set_tt_entry((u64 *)0x06000000, (u64 *)0x0FFFFFFF, (u64 *)0x06000000, TT_PERI);	/* SIREX virtual iRAM */
	set_tt_entry((u64 *)0x10000000, (u64 *)0x1FFFFFFF, (u64 *)0x10000000, TT_PERI);
	set_tt_entry((u64 *)0x80000000, (u64 *)0xF97FFFFF, (u64 *)0x80000000, TT_RAM);
	set_tt_entry((u64 *)0xF9800000, (u64 *)0xFD3FFFFF, (u64 *)0xF9800000, TT_NONCACHEBLE);
	set_tt_entry((u64 *)0xFD400000, (u64 *)0xFD8FFFFF, (u64 *)0xFD400000, TT_RAM);
	set_tt_entry((u64 *)0xFD900000, (u64 *)0xFDAFFFFF, (u64 *)0xFD900000, TT_NONCACHEBLE);
	set_tt_entry((u64 *)0xFDB00000, (u64 *)0xFFFFFFFF, (u64 *)0xFDB00000, TT_RAM);
	set_tt_entry((u64 *)0x880000000, (u64 *)0xAFFFFFFFF, (u64 *)0x880000000, TT_RAM);
}

void cpu_common_init(void)
{
	CPU_EL_LEVEL cur_el = get_current_el();

	s5e9830_mmu_table_init();

	set_mmu_base_default(cur_el);
	invalidate_tlb_all(cur_el);
	invalidate_icache_all();

	enable_icache();
	enable_mmu();
	enable_dcache();
}
