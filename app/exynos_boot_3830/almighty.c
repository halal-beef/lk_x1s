/*
 * @file    test/iptest/dmc/almighty/almighty.c
 * @author  Byungwook So <bw80.so@samsung.com>
 * @version 1.0
 *
 * @section LICENSE
 *
 * Copyright 2017 by Samsung Electronics, Inc.
 * All rights reserved.
 *
 * This software is only used to validate EXYNOS silicons.
 * Nobody can use this software without our permission.
 *
 * @section DESCRIPTION
 *
 * Almighty: dram test tool for customer
 */

#include <platform/mmu/mmu.h>
#include <debug.h>
#include <reg.h>
#include <platform/delay.h>
#include "almighty.h"
#include "drex_v3_3.h"
#include "mct.h"

almighty_system_info_t almighty_system_info;
int almighty_system_info_init_done = -1;
almighty_core_test_info_t almighty_core_test_info[CONFIG_MAXCPU];
int almighty_test_result = 0xff;

static tc_dram_address_type_t tc_dram_address_type[] = {
	/*	name				type(row/column)	address_bit(system view. 4ch, 1rank case)*/
	{	.name = "common",	.type = NONE,		.address_bit = 36 },
	{	.name = "row0",		.type = ROW,		.address_bit = 16 },
	{	.name = "row3",		.type = ROW,		.address_bit = 19 },
	{	.name = "burst16",	.type = COLUMN,		.address_bit = 5 },
};

tc_almighty_test_pattern_t tc_almighty_test_pattern[] = {
	/* even_pattern			odd_pattern */
	{ 0x5555555555555555,	0xaaaaaaaaaaaaaaaa },
	{ 0xaaaaaaaaaaaaaaaa,	0x5555555555555555 },
	{ 0x5555aaaa5555aaaa,	0xaaaa5555aaaa5555 },
	{ 0xaaaa5555aaaa5555,	0x5555aaaa5555aaaa },
	{ 0x55aaaa5555aaaa55,	0xaa5555aaaa5555aa },
	{ 0xaa5555aaaa5555aa,	0x55aaaa5555aaaa55 },
	{ 0xffffffffffffffff,	0x0 },
	{ 0x0000000000000000,	0xffffffffffffffff },
};

static almighty_skip_test_address_info_t skip_test_address[] = {
	{	.start_address = 0x80008000,  .end_address = 0x80040000,	},	/* 0x8000_8000 ~ 0x8004_0000 is mmu table info area */
	{	.start_address = 0x90000000,  .end_address = 0x90100000,	},	/* 0x9000_0000 ~ 0x9010_0000 is ECT area in DRAM */
	{	.start_address = 0xBC800000,  .end_address = 0xC0000000,	},	/* 0xBC80_0000 ~ 0xC000_0000 is secure RO area */
	{	.start_address = 0xE0000000,  .end_address = 0xE0D00000,	},	/* 0xE000_0000 ~ 0xE0D0_0000 secure page table area */
	{	.start_address = 0xEC000000,  .end_address = 0xF0000000,	},	/* #define CONFIG_DISPLAY_LOGO_BASE_ADDRESS    0xec000000, #define CONFIG_DISPLAY_TEMP_BASE_ADDRESS    0xf0000000 */
	{	.start_address = 0xF9000000,  .end_address = 0xF9210000,	},	/* 0xF900_0000 ~ 0xF921_0000 is LK log buffer area */
	{	.start_address = 0xFF000000,  .end_address = 0xFFFFFFFF,	},	/* 0xFF00_0000 ~ 0xFFFF_FFFF is LK area */
};

static void almighty_init(void)
{
	printf("\nInitialize DRAM almighty test\n");

	// get ECT info including number of rank
	almighty_system_info.ect_key.data = __REG64(ECT_LINK_KEY_LOC);

	if (almighty_system_info.ect_key.bitfield.rank_num == 1)
		almighty_system_info.num_of_rank = 1;
	else if (almighty_system_info.ect_key.bitfield.rank_num == 3)
		almighty_system_info.num_of_rank = 2;

	// address bit for 2 rank system
	if (almighty_system_info.num_of_rank == 2 && tc_dram_address_type[0].type == ROW)
		tc_dram_address_type[0].address_bit += 1;

	// address bit for 2 channel system
	if (PHY_CH_ALL == 2 && tc_dram_address_type[0].type == ROW)
		tc_dram_address_type[0].address_bit -= 1;

	float size;

	size = (*((volatile float *)((unsigned int)DEBUG_FLOAT_DRAM_TOTAL_SIZE)));

	if (size == 1)
		set_tt_entry((u64 *)0xC0000000, (u64 *)0xC0001000, (u64 *)0xC0000000, TT_S1_FAULT);
	else if (size == 2)
		set_tt_entry((u64 *)0x100000000, (u64 *)0x100001000, (u64 *)0x100000000, TT_S1_FAULT);
	else if (size == 4)
		set_tt_entry((u64 *)0x900000000, (u64 *)0x900001000, (u64 *)0x900000000, TT_S1_FAULT);
	else if (size == 6)
		set_tt_entry((u64 *)0x980000000, (u64 *)0x980001000, (u64 *)0x980000000, TT_S1_FAULT);
	else if (size == 8)
		set_tt_entry((u64 *)0xA00000000, (u64 *)0xA00001000, (u64 *)0xA00000000, TT_S1_FAULT);
	else if (size == 10)
		set_tt_entry((u64 *)0xA80000000, (u64 *)0xA80001000, (u64 *)0xA80000000, TT_S1_FAULT);
	else if (size == 12)
		set_tt_entry((u64 *)0xB00000000, (u64 *)0xB00001000, (u64 *)0xB00000000, TT_S1_FAULT);

	printf("DRAM size = %.2fGB\n", size);

	almighty_system_info_init_done = 1;
}

static void almighty_write_pattern(void *args)
{
	unsigned int j;
	almighty_core_test_info_t *cpu_info = (almighty_core_test_info_t *)args;
	u64 *p1 = (u64 *)cpu_info->start_address;
	int test_address_bit = tc_dram_address_type[cpu_info->address_type_num].address_bit;

	if (cpu_info->pattern_num > sizeof(tc_almighty_test_pattern)/sizeof(tc_almighty_test_pattern[0])) {
		printf("Pattern number is out of range.\n");
		return;
	}

	for (j = 0; j < (cpu_info->end_address - cpu_info->start_address) / sizeof(u64); j++) {

		if (((u64)p1 & ((u64)0x1 << test_address_bit)) != 0)
			*p1++ = tc_almighty_test_pattern[cpu_info->pattern_num].odd_pattern;
		else
			*p1++ = tc_almighty_test_pattern[cpu_info->pattern_num].even_pattern;

	}

	printf("write pattern%d finished - core%d\n", cpu_info->pattern_num, 0);

}

static void almighty_read_pattern(void *args)
{
	unsigned int j;
	almighty_core_test_info_t *cpu_info = (almighty_core_test_info_t *)args;
	u64 *p1 = (u64 *)cpu_info->start_address;
	int test_address_bit = tc_dram_address_type[cpu_info->address_type_num].address_bit;
	int ret = 0;
	//int cpu_id = smp_processor_id();
	int cpu_id = 0;

	if (cpu_info->pattern_num > sizeof(tc_almighty_test_pattern)/sizeof(tc_almighty_test_pattern[0])) {
		printf("Pattern number is out of range.\n");
		return;
	}

	//CleanInvalidateDCacheAll();

	for (j = 0; j < (cpu_info->end_address - cpu_info->start_address) / sizeof(u64); j++) {

		if (((u64)p1 & ((u64)0x1 << test_address_bit)) != 0) {
			if (*p1 != tc_almighty_test_pattern[cpu_info->pattern_num].odd_pattern) {
				printf("value of address 0x%09lx should be 0x%016lx, but current value is 0x%016lx\n", \
					(long unsigned int)p1, (long unsigned int)tc_almighty_test_pattern[cpu_info->pattern_num].odd_pattern, (long unsigned int)*p1);
				ret = 1;
			}
		} else {
			if (*p1 != tc_almighty_test_pattern[cpu_info->pattern_num].even_pattern) {
				printf("value of address 0x%09lx should be 0x%016lx, but current value is 0x%016lx\n", \
					(long unsigned int)p1, (long unsigned int)tc_almighty_test_pattern[cpu_info->pattern_num].even_pattern, (long unsigned int)*p1);
				ret = 1;
			}
		}
		p1++;
	}

	if (ret == 1) {
		printf("[core%d]fail\n", cpu_id);
		almighty_test_result |= (0x1 << cpu_id);
	}

	printf("read pattern%d finished - core%d\n", cpu_info->pattern_num, 0);

}

int almighty_pattern_test(int pattern_num)
{
	unsigned int i, j, k, l;
	int num_pattern;
	unsigned int min_pattern_num, max_pattern_num;
	u64 test_size, total_size = 0;
	float percent;
	float dram_size;
	cycle_t test_time, total_test_time;

	if (almighty_system_info_init_done != 1)
		almighty_init();

	num_pattern = sizeof(tc_almighty_test_pattern)/sizeof(tc_almighty_test_pattern[0]);

	if (pattern_num < 0 || pattern_num > num_pattern) {
		min_pattern_num = 0;
		max_pattern_num = num_pattern - 1;
	} else {
		min_pattern_num = pattern_num;
		max_pattern_num = pattern_num;
	}

	almighty_test_result = 0;

	total_test_time = mct.get_timer(0);
	printf("\nStart pattern test\n");

	for (l = 0; l < sizeof(skip_test_address)/sizeof(skip_test_address[0]); l++) {
		if (l != sizeof(skip_test_address)/sizeof(skip_test_address[0]) - 1) {
			test_size = skip_test_address[l+1].start_address - skip_test_address[l].end_address;

			almighty_core_test_info[0].pattern_num = 0;
			almighty_core_test_info[0].address_type_num = 0;

			almighty_core_test_info[0].start_address = skip_test_address[l].end_address;
			almighty_core_test_info[0].end_address = almighty_core_test_info[0].start_address + test_size;
		} else {
			float size;
			/* arm dram hole 0x1_0000_0000 ~ 0x8_8000_0000 */
			/* high dram area starts to 0x8_8000_0000 ~ */

			size = (*((volatile float *)((unsigned int)DEBUG_FLOAT_DRAM_TOTAL_SIZE)));

			test_size = (size * 1024 * 1024 * 1024) - 0x80000000;

			almighty_core_test_info[0].pattern_num = 0;
			almighty_core_test_info[0].address_type_num = 0;

			almighty_core_test_info[0].start_address = 0x880000000;
			almighty_core_test_info[0].end_address = almighty_core_test_info[0].start_address + test_size;
		}

		printf("\n %d. core #%d start address = 0x%-9lx, end address = 0x%-9lx, low0_size = %-4luMB\n", l, 0, almighty_core_test_info[0].start_address, \
				almighty_core_test_info[0].end_address, (almighty_core_test_info[0].end_address - almighty_core_test_info[0].start_address) / (1024 * 1024));

		for (k = 0; k < sizeof(tc_dram_address_type)/sizeof(tc_dram_address_type[0]); k++) {

			printf("\nAddress type %d : %s\n", k, tc_dram_address_type[k].name);
			for (i = min_pattern_num; i <= max_pattern_num; i++) {
				test_time = mct.get_timer(0);

				for (j = 0; j < CONFIG_MAXCPU; j++) {
					almighty_core_test_info[j].pattern_num = i;
					almighty_core_test_info[j].address_type_num = k;
				}

				printf("Pattern test %d(0x%016lx) started\n", almighty_core_test_info->pattern_num, tc_almighty_test_pattern[almighty_core_test_info->pattern_num].even_pattern);

				// write pattern
				almighty_write_pattern((void *)&almighty_core_test_info[0]);

				// read pattern
				almighty_read_pattern((void *)&almighty_core_test_info[0]);

				printf(" - finished, test time is %llu(us)\n", mct.ticks2usec(mct.get_timer(test_time)));

				if (almighty_test_result) {
					printf("almighty test result: %x\n", almighty_test_result);
					goto TEST_END;
				}
			}
		}

		total_size += test_size;
	}

	dram_size = (*((volatile float *)((unsigned int)DEBUG_FLOAT_DRAM_TOTAL_SIZE)));

	total_size = total_size / (1024 * 1024);
	percent = (float)((total_size / (dram_size * 1024)) * 100);
	printf("The Tested Dram area size is %-4lluMB, so %.2f%% of DRAM area is covered\n", total_size, percent);
	printf("\nEnd pattern test\n");
	printf("Total test time is %llu(us)\n\n", mct.ticks2usec(mct.get_timer(total_test_time)));

TEST_END:
	for (j = 0; j < CONFIG_MAXCPU; j++) {
		almighty_core_test_info[j].pattern_num = 0;
		almighty_core_test_info[j].address_type_num = 0;
	}

	return (almighty_test_result)? 1 : 0; //pass = 0, fail = 1;
}

int almighty_get_dram_freq(void)
{
	int dvfs_mode = 0;

	dvfs_mode = (readl(PHY0_BASE + 0xB8) >> 30) & 0x3;

	if (dvfs_mode == 0)
		return (readl(PHY0_BASE + 0xB8) >> 0) & 0xFFF;
	else if (dvfs_mode == 1)
		return (readl(PHY0_BASE + 0xBC) >> 20) & 0xFFF;
	else if (dvfs_mode == 2)
		return (readl(PHY0_BASE + 0xC0) >> 20) & 0xFFF;
	else
		return 0;
}
