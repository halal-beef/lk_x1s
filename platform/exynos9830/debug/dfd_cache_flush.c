/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.


 * Alternatively, this program is free software in case of open source project
 * you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <lk/reg.h>
#include <stdio.h>
#include <string.h>
#include <arch/ops.h>
#include <platform/delay.h>
#include <platform/sizes.h>
#include <platform/sfr.h>
#include <platform/smc.h>
#include <platform/dfd.h>
#include <dev/debug/dss.h>

#define MOESI_VALID			(1 << 2)
#define MOESI_MODIFIED			(1 << 0)

#define MESI_VALID			(1 << 2)
#define MESI_MODIFIED			(1 << 1)

#define L2_MESI_VALID			(1 << 2)
#define L2_MESI_MODIFIED		(1 << 0)

#define L2TAG0_PA_39_17_MASK		(u64)0x01FFFFFC
#define DL1TAG0_PA_39_12_MASK		(u64)0x0FFFFFFF

#define IL1TAG_RAMID			0x00
#define IL1TAG_INDEX_END		0x80
#define IL1TAG_WAY_END			0x04
#define IL1TAG_DATA_END			0x01

#define IL1DATA_RAMID			0x01
#define IL1DATA_INDEX_END		0x800
#define IL1DATA_WAY_END			0x04
#define IL1DATA_DATA_END		0x02

#define FE_BIG_TLB_PTE_RAMID		0x05
#define FE_BIG_TLB_PTE_INDEX_END	0x100
#define FE_BIG_TLB_PTE_WAY_END		0x01
#define FE_BIG_TLB_PTE_DATA_END		0x02

#define TBW_L2TLB_RAMID			0x00
#define TBW_L2TLB_INDEX_END		0x400
#define TBW_L2TLB_WAY_END		0x04
#define TBW_L2TLB_DATA_END		0x04

#define L3_BANK_NUM			0x02
#define L3_SUBBANK_NUM			0x04
#define L3_BANK_WIDTH			0x02
#define L3_TAG_SECTORS_CONCATENATE	0x02
#define L3_TAG_WAYS_CONCATENATE		0x02
#define L3_TAG_SIZE			12
#define L3_DATA_SIZE			64
#define L3_SET_INDEX_ADDR_LSB		8
#define SET_END_L3			0x100
#define WAY_END_L3			0x10
#define SECTOR_END_L3			0x2
#define OFFSET_END_L3			0x40
#define OFFSET_WIDTH_L3			0x06
#define OFFSET_STEP_NUM_L3		0x10
#define OFFSET_STEP_WIDTH_L3		0x04

#define BANK_END_L2			0x4
#define SET_END_L2			0x100
#define WAY_END_L2			0x8
#define SECTOR_END_L2			0x2
#define L2_CHUNK			0x48
#define OFFSET_END_L2			0x40
#define OFFSET_WIDTH_L2			0x7
#define OFFSET_STEP_NUM_L2		0x10
#define OFFSET_STEP_WIDTH_L2		0x04

#define SET_END_D			0x100
#define WAY_END_D			0x4
#define OFFSET_END_D			0x40
#define OFFSET_WIDTH_D			0x06
#define OFFSET_STEP_NUM_D		0x08
#define OFFSET_STEP_WIDTH_D		0x03

#define L3CACHE_DUMP_ADDR		(u32)dss_get_item_paddr("log_arrdumpreset")
#define L3CACHE_DUMP_SIZE		(u64)0x00460000
#define L3CACHE_TAG_SIZE		(u64)(L3_BANK_NUM * L3_SUBBANK_NUM * SET_END_L3 * \
					WAY_END_L3 / L3_TAG_WAYS_CONCATENATE * L3_TAG_SIZE)
#define L3CACHE_DATA_DUMP_ADDR		(u64)(L3CACHE_DUMP_ADDR + L3CACHE_TAG_SIZE)

#define L2CACHE_DUMP_ADDR		(u64)(L3CACHE_DUMP_ADDR + L3CACHE_DUMP_SIZE)
#define L2_TAG_SIZE			8
#define L2_DATA_SIZE			64
#define L2CACHE_TAG_SIZE		(u64)(BANK_END_L2 * SET_END_L2 * WAY_END_L2) * L2_TAG_SIZE
#define L2CACHE_DATA_SIZE		(u64)(BANK_END_L2 * SET_END_L2 * WAY_END_L2 * SECTOR_END_L2) * L2_DATA_SIZE
#define L2CACHE_DATA_DUMP_ADDR		(u64)(L2CACHE_DUMP_ADDR + L2CACHE_TAG_SIZE)

#define CORE_DUMP_ADDR			(u64)(L2CACHE_DUMP_ADDR + L2CACHE_TAG_SIZE + L2CACHE_DATA_SIZE)
#define BTLB_SIZE			(u64)(512 * 1 * 4 * 4)
#define L2TLB_SIZE			(u64)(TBW_L2TLB_INDEX_END * TBW_L2TLB_WAY_END * TBW_L2TLB_DATA_END * 4)
#define IL1TAG_SIZE			(u64)(IL1TAG_INDEX_END * IL1TAG_WAY_END * IL1TAG_DATA_END * 4)
#define IL1DATA_SIZE			(u64)(IL1DATA_INDEX_END * IL1DATA_WAY_END * IL1DATA_DATA_END * 4)

#define CORE_DUMP_SIZE			(u64)(L1DATA_RAW_SIZE + L2TLB_SIZE + IL1TAG_SIZE + \
						IL1DATA_SIZE + 257 * 1024 + BTLB_SIZE)

#define DL1_TAG_DATA_SIZE		8
#define DL1_LINE_RAW_DATA_SIZE		72

#define L1DATA_RAW_SIZE			(u64)(SET_END_D * WAY_END_D * DL1_LINE_RAW_DATA_SIZE)

#define AN_DUMP_ADDR			(L3CACHE_DUMP_ADDR + (8 * 1024 * 1024))
#define AN_CORE_DUMP_SIZE		(128 * 1024)

#define ENYO_DUMP_ADDR			(AN_DUMP_ADDR + AN_CORE_DUMP_SIZE * LITTLE_NR_CPUS)
//#define ENYO_CORE_DUMP_SIZE		(1734656)
#define ENYO_CORE_DUMP_SIZE		(1552384) // wo l1btb, l1ghb, bpiq, l2victim
//#define DEBUG_PRINT

#define SCI_BASE			0x1A000000
#define PM_SCI_CTL			(SCI_BASE + 0x2A8)
#define PM_SCI_ST			(SCI_BASE + 0x2AC)
#define LLCInv				(SCI_BASE + 0x548)
#define CCMControl1			(SCI_BASE + 0x410)
#define SB_LLC_STATUS			(SCI_BASE + 0xA0C)
#define LLC_LOOP_TIMEOUT		(3000)

//#define DEBUG_PRINT
extern u64 sec_area_base;
extern u64 sec_area_end;

struct DL1RAMINDEX_LO_BIT_INFO {
	unsigned int INDEX:19;		//bit 18:0
	unsigned int WAY:4;			//bit 22:19
	unsigned int WORD_IDENTIFIER:2;	//bit 24:23
	unsigned int DUMMY1:2;		//bit 25:26
	unsigned int RAMID:5;		//bit 31:27
};

struct DL1RAMINDEX_HI_BIT_INFO {
	unsigned int DUMMY0:31;		//bit 30:0
	unsigned int COMPLETE:1;	//bit 31
};

struct DL1TAG0_BIT_INFO {
	unsigned int PHY_ADD_39_12:28;	//bit 27:0
	unsigned int NS:1;			//bit 28
	unsigned int DUMMY0:3;		//bit 31:29
};

struct DL1TAG1_BIT_INFO {
	unsigned int MESI:3;		//bit 2:0
	unsigned int DUMMY0:29;		//bit 31:3
};

struct L3RAMINDEX_LO_BIT_INFO {
	unsigned int DUMMY0:1;		//bit 0
	unsigned int SECTOR:1;		//bit 1
	unsigned int SUBBANK:2;		//bit 3:2
	unsigned int QWORD:2;		//bit 5:4
	unsigned int BANK:2;		//bit 7:6
	unsigned int L3INDEX:8;		//bit 15:8
	unsigned int DUMMY1:3;		//bit 18:16
	unsigned int WAY:4;			//bit 22:19
	unsigned int DUMMY2:2;		//bit 24:23
	unsigned int WRITE:1;		//bit 25
	unsigned int CONTROL:1;		//bit 26
	unsigned int RAMID:5;		//bit 31:27
};

struct L3RAMINDEX_HI_BIT_INFO {
	unsigned int DUMMY0:31;		//bit 30:0
	unsigned int COMPLETE:1;	//bit 31
};

struct L3TAG_BIT_INFO {
	u64 MOESI0:3;			//bit 2:0
	u64 MOESI1:3;			//bit 5:3
	u64 SL2ID0:2;			//bit 7:6
	u64 SL2ID1:2;			//bit 9:8
	u64 NS:1;				//bit 10
	u64 PA_7:1;				//bit 11
	u64 PA_39_16:24;		//bit 35:12
	u64 REREF:1;			//bit 36
	u64 PREFETCH:1;			//bit 37
	u64 ECC:7;				//bit 44:38
};

struct L2RAMINDEX_LO_BIT_INFO {
	unsigned int DUMMY0:4;		//bit 3:0
	unsigned int QWORD:2;		//bit 5:4
	unsigned int BANK:2;		//bit 7:6
	unsigned int L2INDEX:9;		//bit 16:8
	unsigned int DUMMY1:2;		//bit 18:17
	unsigned int WAY:4;			//bit 22:19
	unsigned int SECTOR:1;		//bit 23
	unsigned int DUMMY2:3;		//bit 26:24
	unsigned int RAMID:5;		//bit 31:27
};

struct L2RAMINDEX_HI_BIT_INFO {
	unsigned int DUMMY0:31;		//bit 30:0
	unsigned int COMPLETE:1;	//bit 31
};

struct L2TAG0_BIT_INFO {
	unsigned int MESI0:3;		//bit 2:0
	unsigned int CPUID0:2;		//bit 4:3
	unsigned int LOCK0:1;		//bit 5
	unsigned int DUMMY0:4;		//bit 9:6
	unsigned int MESI1:3;		//bit 12:10
	unsigned int CPUID1:2;		//bit 14:13
	unsigned int LOCK1:1;		//bit 15
	unsigned int DUMMY1:4;		//bit 19:16
	unsigned int NS:1;			//bit 20
	unsigned int PA_9_7:3;		//bit 23:21
	unsigned int PA_23_16:8;	//bit 31:24
};

struct L2TAG1_BIT_INFO {
	unsigned int PA_39_24:16;	//bit 15:0
	unsigned int res:16;
};

union DL1RAMINDEX_LO_REG {
	u32 word;
	struct DL1RAMINDEX_LO_BIT_INFO bits;
};

union DL1RAMINDEX_HI_REG {
	u32 word;
	struct DL1RAMINDEX_HI_BIT_INFO bits;
};

union DL1TAG0 {
	u32 word;
	struct DL1TAG0_BIT_INFO bits;
};

union DL1TAG1 {
	u32 word;
	struct DL1TAG1_BIT_INFO bits;
};

union L2RAMINDEX_LO_REG {
	u32 word;
	struct L2RAMINDEX_LO_BIT_INFO bits;
};

union L2RAMINDEX_HI_REG {
	u32 word;
	struct L2RAMINDEX_HI_BIT_INFO bits;
};

union L2TAG0 {
	u32 word;
	struct L2TAG0_BIT_INFO bits;
};

union L2TAG1 {
	u32 word;
	struct L2TAG1_BIT_INFO bits;
};

union L3RAMINDEX_LO_REG {
	u32 word;
	struct L3RAMINDEX_LO_BIT_INFO bits;
};

union L3RAMINDEX_HI_REG {
	u32 word;
	struct L3RAMINDEX_HI_BIT_INFO bits;
};

union L3TAG {
	u64 ll_word;
	struct L3TAG_BIT_INFO bits;
};

static int is_sec_area_bound(u64 addr)
{
	return ((sec_area_base <= addr) && (addr <= sec_area_end));
}

static int is_dram_bound(u64 addr)
{
	u64 dram_size = *(u64 *)BL_SYS_INFO_DRAM_SIZE;
	u64 dram_base = DRAM_BASE;

	/* Compare with low 2GB DRAM. */
	if (dram_size < SZ_2G) {
		if (dram_base <= addr && addr <= dram_base + dram_size)
			return 1;
		else
			return 0;
	}

	if (dram_base <= addr && addr <= dram_base + SZ_2G)
		return 1;

	dram_base = DRAM_BASE2;
	/* Compare with high DRAM. */
	if (dram_base <= addr && addr <= dram_base + dram_size - SZ_2G)
		return 1;

	return 0;
}

static void dump_l3_operation(u64 bank, u64 subbank, u64 set, u64 way, u64 sector)
{
	union L3TAG l3tag_reg;
	u64 l3addr_full;
	u64 l3_tag_raw_data0, l3_tag_raw_data1, l3_tag_raw_data2;
	u64 dump_base;
	u64 MOESI;

	dump_base = L3CACHE_DUMP_ADDR+bank * L3_SUBBANK_NUM * SET_END_L3 *
					WAY_END_L3 / L3_TAG_WAYS_CONCATENATE * L3_TAG_SIZE;
	dump_base += subbank * SET_END_L3 * WAY_END_L3 / L3_TAG_WAYS_CONCATENATE * L3_TAG_SIZE;
	dump_base += set * WAY_END_L3 / L3_TAG_WAYS_CONCATENATE * L3_TAG_SIZE;
	dump_base += way / L3_TAG_WAYS_CONCATENATE * L3_TAG_SIZE;

	l3_tag_raw_data0 = readl(dump_base + 0x0);
	l3_tag_raw_data1 = readl(dump_base + 0x4);
	l3_tag_raw_data2 = readl(dump_base + 0x8);

	l3tag_reg.ll_word = 0;
	if ((way % 2) == 0)
	    l3tag_reg.ll_word = ((l3_tag_raw_data1 & 0x1fff) << 32) | l3_tag_raw_data0;
	else
	    l3tag_reg.ll_word = ((l3_tag_raw_data2 & 0x3ffffff) << 19)| (l3_tag_raw_data1 >> 13);

	if (sector == 0)
		MOESI = l3tag_reg.bits.MOESI0;
	else
		MOESI = l3tag_reg.bits.MOESI1;

	/* determine writeback */
	if (!(((MOESI & (MOESI_VALID | MOESI_MODIFIED)) == (MOESI_VALID | MOESI_MODIFIED))
			&& ((l3tag_reg.bits.NS) == 0x1)))
		return;

	l3addr_full = (((u64) l3tag_reg.bits.PA_39_16) << 16) |
		((u64) set << L3_SET_INDEX_ADDR_LSB) |
		((u64) l3tag_reg.bits.PA_7 << 7) |
		((u64) sector << 6);

	dump_base = L3CACHE_DATA_DUMP_ADDR + bank * L3_SUBBANK_NUM * SET_END_L3 *
				WAY_END_L3 * SECTOR_END_L3 * L3_DATA_SIZE;
	dump_base += subbank * SET_END_L3 * WAY_END_L3 * SECTOR_END_L3 * L3_DATA_SIZE;
	dump_base += set * WAY_END_L3 * SECTOR_END_L3 * L3_DATA_SIZE;
	dump_base += way * SECTOR_END_L3 * L3_DATA_SIZE;
	dump_base += sector * L3_DATA_SIZE;

	for (u32 offset = 0; offset < OFFSET_END_L3; offset += OFFSET_STEP_NUM_L3) {
		u32 l3data0 = readl(dump_base + offset + 0x0);
		u32 l3data1 = readl(dump_base + offset + 0x4);
		u32 l3data2 = readl(dump_base + offset + 0x8);
		u32 l3data3 = readl(dump_base + offset + 0xc);
		u64 l3addr = l3addr_full + offset;
#ifdef DEBUG_PRINT
		printf("L3] addr:0x%016llx, data:0x%08x 0x%08x 0x%08x 0x%08x"
				"(bank:0x%x, subbank:0x%x, set:0x%x, way:0x%x)\n",
				l3addr, l3data0, l3data1, l3data2, l3data3, bank, subbank, set, way);
#endif
		if (is_sec_area_bound(l3addr))
			continue;

		if (!is_dram_bound(l3addr))
			continue;

		writel(l3data0, l3addr + 0x0);
		writel(l3data1, l3addr + 0x4);
		writel(l3data2, l3addr + 0x8);
		writel(l3data3, l3addr + 0xc);
	}
}

static void write_back_l3_cache(int core)
{
	u64 bank, subbank, set, way, sector;

	if (core < 0 || core > BIG_NR_CPUS - 1)
		return;

	for (bank = 0; bank < L3_BANK_NUM; bank++)
		for (subbank = 0; subbank < L3_SUBBANK_NUM; subbank++)
			for (set = 0; set < SET_END_L3; set++)
				for (way = 0; way < WAY_END_L3; way++)
					for (sector = 0; sector < SECTOR_END_L3; sector++)
						dump_l3_operation(bank, subbank, set, way ,sector);

}

static void dump_l2_operation(u64 core, u64 bank, u64 set, u64 way, u64 sector)
{
	union L2TAG0 l2tag0_reg;
	union L2TAG1 l2tag1_reg;
	u64 l2addr_full;
	u64 dump_base;
	u64 MESI;

	dump_base = L2CACHE_DUMP_ADDR +
			(bank * SET_END_L2 * WAY_END_L2 + set * WAY_END_L2 + way) * L2_TAG_SIZE;

	l2tag0_reg.word = readl(dump_base + 0x0);
	l2tag1_reg.word = readl(dump_base + 0x4);

	if (sector == 0)
		MESI=l2tag0_reg.bits.MESI0;
	else
		MESI=l2tag0_reg.bits.MESI1;

	/* determine writeback. modified or owned */
	if (!(((MESI & (L2_MESI_VALID | L2_MESI_MODIFIED)) == (L2_MESI_VALID | L2_MESI_MODIFIED))
			&& ((l2tag0_reg.bits.NS) == 0x1)))
		return;

	l2addr_full = (((u64) l2tag1_reg.bits.PA_39_24) << 24) |
		(((u64) l2tag0_reg.bits.PA_23_16) << 16) |
		(((u64) (set & 0x3f) << 10)) |
		((u64) l2tag0_reg.bits.PA_9_7 << OFFSET_WIDTH_L2) |
		((u64) sector << 6);

	dump_base = L2CACHE_DATA_DUMP_ADDR + (bank * SET_END_L2 * WAY_END_L2 * SECTOR_END_L2 +
			set * WAY_END_L2 * SECTOR_END_L2 + way * SECTOR_END_L2 + sector) * L2_DATA_SIZE;

	for (u32 offset = 0; offset < OFFSET_END_L2; offset += OFFSET_STEP_NUM_L2) {
		u32 l2data0 = readl(dump_base + offset + 0x0);
		u32 l2data1 = readl(dump_base + offset + 0x4);
		u32 l2data2 = readl(dump_base + offset + 0x8);
		u32 l2data3 = readl(dump_base + offset + 0xc);
		u64 l2addr = l2addr_full + offset;
#ifdef DEBUG_PRINT
		printf("[L2] PA:0x%016llx: 0x%08x 0x%08x 0x%08x 0x%08x (set:0x%x, way:0x%x)\n",
				l2addr, l2data0, l2data1, l2data2, l2data3, set, way);
#endif
		if (is_sec_area_bound(l2addr))
			continue;

		if (!is_dram_bound(l2addr))
			continue;

		writel(l2data0, l2addr + 0x0);
		writel(l2data1, l2addr + 0x4);
		writel(l2data2, l2addr + 0x8);
		writel(l2data3, l2addr + 0xc);
	}
}

static void write_back_l2_cache(int core)
{
	u64 bank, set, way, sector;

	if (core < 0 || core > BIG_NR_CPUS - 1)
		return;

	for (bank = 0; bank < BANK_END_L2; bank++)
		for (set = 0; set < SET_END_L2; set++)
			for (way = 0; way < WAY_END_L2; way++)
				for (sector = 0; sector < SECTOR_END_L2; sector++)
					dump_l2_operation(core, bank, set, way, sector);

	dumpgpr_flush_secdram(1, core);
}

static void write_back_d_cache_operation(u32 core, u64 set, u64 way)
{
	union DL1TAG0 dl1tag0_reg;
	union DL1TAG1 dl1tag1_reg;
	u64 daddr_full;
	u64 dump_base;

	dump_base = CORE_DUMP_ADDR + CORE_DUMP_SIZE * core +
		(set * WAY_END_D + way) * DL1_LINE_RAW_DATA_SIZE;

	dl1tag0_reg.word = readl(dump_base + 0x0);
	dl1tag1_reg.word = readl(dump_base + 0x4);

	/* determine writeback. modified or owned */
	if (!(((dl1tag1_reg.bits.MESI & (MESI_VALID | MESI_MODIFIED)) ==
		(MESI_VALID | MESI_MODIFIED)) && (dl1tag0_reg.bits.NS != 0)))
		return;

	daddr_full = (((u64)dl1tag0_reg.word & DL1TAG0_PA_39_12_MASK) << 12) |
		(((set & 0x3f) << OFFSET_WIDTH_D) & 0x0fff);

	for (u32 offset = 0; offset < OFFSET_END_D; offset += OFFSET_STEP_NUM_D) {
		u32 data0 = readl(dump_base + DL1_TAG_DATA_SIZE + offset + 0x0);
		u32 data1 = readl(dump_base + DL1_TAG_DATA_SIZE + offset + 0x4);
		u64 daddr = daddr_full + offset;
#ifdef DEBUG_PRINT
		printf("[DL1-%d] PA:0x%016llx: 0x%08x 0x%08x (set:0x%x, way:0x%x)\n",
				core, daddr, data0, data1, set, way);
#endif
		if (is_sec_area_bound(daddr))
			continue;

		if (!is_dram_bound(daddr))
			continue;

		writel(data0, daddr + 0x0);
		writel(data1, daddr + 0x4);
	}

	dumpgpr_flush_secdram(0, core);
}

static void write_back_d_cache(int core)
{
	u64 set, way;

	if (core < 0 || core > BIG_NR_CPUS - 1)
		return;

	for (set = 0; set < SET_END_D; set++)
		for (way = 0; way < WAY_END_D; way++)
			write_back_d_cache_operation(core, set, way);
}

static bool common_cache_flush = false;
void write_back_cache(int cpu)
{
	u32 stat = 0;
	u32 cpu_offset = cpu - BIG_CORE_START;

	stat = readl(CONFIG_RAMDUMP_GPR_POWER_STAT + (cpu * REG_OFFSET));
	if (stat == FLUSH_SKIP)
		goto finish;

	if (cpu == BIG_CORE_START) {
		write_back_l3_cache(cpu_offset);
		write_back_l2_cache(cpu_offset);
		common_cache_flush = true;
	} else {
		if (!common_cache_flush) {
			write_back_l3_cache(cpu_offset);
			write_back_l2_cache(cpu_offset);
			common_cache_flush = true;
		}
	}
	write_back_d_cache(cpu_offset);
finish:
	stat = readl(CONFIG_RAMDUMP_DUMP_GPR_WAIT);
	writel((stat | (1 << cpu)), CONFIG_RAMDUMP_DUMP_GPR_WAIT);
}

void llc_flush(u32 invway)
{
	unsigned int w1;
	unsigned int loop_cnt = 0;
	unsigned int rst_seq = readl(POWER_RST_STAT);

	if (!(rst_seq & LLC_INIT)) {
		printf("LLC is not enabled!! skip LLC flush (0x%08x, %d)\n",
				rst_seq, rst_seq & LLC_INIT);
		goto out;
	}

	/* clear invalidate bits,
	 * .LlcWrtBkInvReq[1] = 0x0,
	 * .LlcInvReq[0] = 0x0
	 */
	w1 = readl(LLCInv);
	w1 = w1 & ~((0x1 << 1) | (0x1 << 0));
	writel(w1, LLCInv);

	/* llc invalidate with write-back */

	/* .LlcInvCache[2] = 0x1 */
	w1 = readl(LLCInv);
	w1 = w1 | (0x1 << 2);
	writel(w1, LLCInv);

	/* .LlcInvWay[31:16] */
	w1 = readl(LLCInv);
	w1 &= ~(0xFFFF << 16);
	w1 |= (invway << 16);
	writel(w1, LLCInv);

	/* .LlcWrtBkInvReq[1] = 1
	 * .LlcInvReq[0] = 0x1 */
	w1 = readl(LLCInv);
	w1 = w1 | (0x1 << 1);
	w1 = w1 | (0x1 << 0);
	writel(w1, LLCInv);

	do {
		udelay(1);
		loop_cnt++;
		if (loop_cnt > LLC_LOOP_TIMEOUT) {
			printf("timeout LLC invalidate with write-back\n");
			return;
		}
		w1 = readl(SB_LLC_STATUS);
	} while (w1 & (0x1 << 0)); /* LlcInvInProgress[5] = 0 */

	printf("Flushed LLC\n");

out:
	rst_seq = clear_llc_init_state();
	printf("LLC init state clear!! (0x%08x)\n", rst_seq);
}

void llc_flush_disable(void)
{
	unsigned int w1;
	unsigned int loop_cnt = 0;

	/* clock gating enable */
	w1 = readl(PM_SCI_CTL);
	w1 = w1 | (0x1 << 25);	/* .LLC_En[25] = 0x1 */
	writel(w1, PM_SCI_CTL);

	do {
		udelay(1);
		loop_cnt++;
		if (loop_cnt > LLC_LOOP_TIMEOUT) {
			printf("timeout LLC clock gating enable\n");
			return;
		}
		w1 = readl(PM_SCI_ST);
	} while (w1 & (0x1 << 11));	/* .LLC_CLK_EN[11] = 0x0 */

	/* LLC power up */
	w1 = readl(PM_SCI_CTL);
	w1 = w1 & ~(0x1 << 15);		/* .LlcArrayPDE[15] = 0x0, Power Down enable clear */
	writel(w1, PM_SCI_CTL);

	loop_cnt = 0;

	/* .Llc0ArrayPGTSTOUTO[7] = 0x0, Llc1ArrayPGTSTOUTO[8] = 0x0) */
	do {
		udelay(1);
		loop_cnt++;
		if (loop_cnt > LLC_LOOP_TIMEOUT) {
			printf("timeout LLC power up\n");
			return;
		}
		w1 = readl(PM_SCI_ST);
	} while ((w1 & (0x1 << 7)) || (w1 & (0x1 << 8)));

	/* llc flush */
	llc_flush(0xFF00);
	llc_flush(0x00FF);

	/* llc disable */
	w1 = readl(CCMControl1);
	w1 = w1 | (0x1 << 9);		/* .DisableLlc[9] = 0x1 */
	writel(w1, CCMControl1);

	loop_cnt = 0;

	/* llc power down */
	do {
		udelay(1);
		loop_cnt++;
		if (loop_cnt > LLC_LOOP_TIMEOUT) {
			printf("timeout LLC power down ready\n");
			return;
		}
		w1 = readl(PM_SCI_ST);
	} while (!(w1 & (0x3 << 9)));	/* .PMLlcArrDisRdy[10:9] = 0x3 */

	w1 = readl(PM_SCI_CTL);
	w1 = w1 | (0x1 << 15);		/* .LlcArrayPDE[15] = 0x1, Power Down enable set */
	writel(w1, PM_SCI_CTL);

	loop_cnt = 0;

	/* .Llc0ArrayPGTSTOUTA[5] = 0x1, Llc1ArrayPGTSTOUTA[6] = 0x1) */
	do {
		udelay(1);
		loop_cnt++;
		if (loop_cnt > LLC_LOOP_TIMEOUT) {
			printf("timeout LLC power down\n");
			return;
		}
		w1 = readl(PM_SCI_ST);
	} while (!(w1 & (0x1 << 5)) || !(w1 & (0x1 << 6)));

	/* .PMLlcArrDisRdy[10:9] = 0x3, write to clear */
	w1 = w1 | (0x3 << 9);
	writel(w1, PM_SCI_ST);
}
