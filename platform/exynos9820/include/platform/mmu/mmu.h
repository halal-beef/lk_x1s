/*
 * ==============================================================================
 *
 *       Filename:  mmu.h
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
 * ==============================================================================
 */

#ifndef MMU_H_
#define MMU_H_

#include "cpu_a.h"
#define	CONFIG_TTBR_EL3_L1			0x80008000ULL
#define	CONFIG_TTBR_EL3_L2			0x80010000ULL

#define BLK_1G  1
#define BLK_2M  2
#define PAGE_4K 4
#define CONFIG_TT_FORMAT (BLK_2M)

#define TTBR0_EL1_L0	0x0
#define TTBR0_EL1_L1	((unsigned long int)CONFIG_TTBR_EL3_L1)	/* need 0x1000 */
#define TTBR0_EL1_L2	((unsigned long int)CONFIG_TTBR_EL3_L2)	/* need 0x4000 */
#define TTBR0_EL1_L3	0x0

#define MM_ATTR_DEV_nGnRnE	   0x00UL
#define MM_ATTR_DEV_nGnRE	   0x04UL
#define MM_ATTR_NOR_NC        0x44UL
#define MM_ATTR_NOR_WB        0xCCUL
#define MM_ATTR_NOR_WT        0x88UL
#define MM_ATTR_NOR_WBRA      0xEEUL
#define MM_ATTR_NOR_WBWA      0xDDUL
#define MM_ATTR_NOR_WBWARA    0xFFUL
#define MM_ATTR_NOR_WBWARAT   0x77UL
#define MM_ATTR_NOR_WTWARA    0x33UL

/* user defined attribute */
#define MM_ATTR_INVALID       (u8)0x0F
	/* last MAIR index [64:57] is used for magic number : 0xB*/
#define MM_ATTR_MAGIC           0x0B00000000000000
#define MM_ATTR_ALL_INVALID    (MM_ATTR_MAGIC|0x000F0F0F0F0F0F0F)
#define MM_ATTR_FAULT		 (u8)0x03

#define MAIR_EL1_ATTR	0x000000000000FF44
#define MAIR_EL2_ATTR	0x000000000000FF44
#define MAIR_EL3_ATTR	0x000000000000FF44

#define TT_S1_FAULT		0x0
#define TT_S1_NORMAL_NO_CACHE	0x00000000000000401UL	// Index[4:2] = 0, AF[10]=1
#define TT_S1_NORMAL_WBWA	0x00000000000000405UL	// Index[4:2] = 1, AF[10]=1
#define TT_S1_DEVICE_nGnRnE	0x00000000000000409UL	// Index[4:2] = 2, AF[10]=1
#define TT_S1_DEVICE_nGnRE	0x0000000000000040dUL	// Index[4:2] = 3, AF[10]=1

#define TT_S1_UXN	(1UL<<54)	// UXN:Unprivileged Execute-never(stage 1 of EL1&0), XN:Execlute-never
#define TT_S1_PXN	(1UL<<53)	// privileged Execute-never (used only for stage 1 of EL1&0)
#define TT_S1_nG	(1UL<<11)	// the not global bit. Determines whether the TLB entry applies to all ASID values or only to the current ASID. (valid only for EL1&0)
#define TT_S1_NS	(1UL<<5)	// Non-secure bit

#define TT_S1_NON_SHARED	(0 << 8)               // Non-shareable
#define TT_S1_INNER_SHARED      (3 << 8)               // Inner-shareable
#define TT_S1_OUTER_SHARED      (2 << 8)               // Outer-shareable

#define TT_S1_PRIV_RW		(0x0)
#define TT_S1_PRIV_RO		(0x2 << 6)
#define TT_S1_USER_RW		(0x1 << 6)
#define TT_S1_USER_RO		(0x3 << 6)

#define TT_FLASH	(u64)TT_S1_NORMAL_WBWA|TT_S1_INNER_SHARED|TT_S1_PRIV_RO
#define TT_RAM		(u64)TT_S1_NORMAL_WBWA|TT_S1_INNER_SHARED|TT_S1_PRIV_RW
#define TT_PERI		(u64)TT_S1_DEVICE_nGnRnE|TT_S1_PXN|TT_S1_UXN|TT_S1_PRIV_RW
#define TT_DEVICE	(u64)TT_S1_DEVICE_nGnRE|TT_S1_PXN|TT_S1_UXN|TT_S1_PRIV_RW
#define TT_NONCACHEBLE	(u64)TT_S1_NORMAL_NO_CACHE|TT_S1_INNER_SHARED|TT_S1_PRIV_RW
#define TT_RAM_PXN	(u64)TT_S1_NORMAL_WBWA|TT_S1_INNER_SHARED|TT_S1_USER_RW|TT_S1_PXN

//level 1, 2 descriptor attr
#define TT_S1_TABLE	0x00000000000000003    // NSTable=0, PXNTable=0, UXNTable=0, APTable=0
#define TT_BLOCK	0x00000000000000001

void init_mair(CPU_EL_LEVEL cur_el);
u64 *get_tt_base(CPU_EL_LEVEL el, u32 table_level);
u64 *get_ttbr1_base(CPU_EL_LEVEL el, u32 table_level);
u64 get_tt_granule_size(CPU_EL_LEVEL cur_el);

void invalidate_tlb_all(CPU_EL_LEVEL cur_el);
void set_tt_entry(u64 *va_start, u64 *va_end, u64 *pa_start, u64 attr);
void get_highmem_mmu(unsigned long long *base, unsigned int *size);


void set_tt_direct(void);
void set_mmu_base_default(CPU_EL_LEVEL el);

extern void enable_mmu(void);
extern void disable_mmu(void);
extern void enable_mmu_dcache(void);
extern void disable_mmu_dcache(void);
void cpu_common_init(void);
#endif /* MMU_H_ */
