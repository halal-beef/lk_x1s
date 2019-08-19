#ifndef __ALMIGHTY_H__
#define __ALMIGHTY_H__

#ifdef AARCH_64
typedef unsigned long long  dmc_addr_t;
#else
typedef unsigned int        dmc_addr_t;
#endif

#define CONFIG_MAXCPU 1

#define PHY0_BASE	(0x10460000)
#define PHY1_BASE	(0x10560000)
#define PHY_CH_ALL 2

#define APM_SRAM_BASE                          (0x02039000)
#define DRAM_PARAM_RESTORE_LOC                 (APM_SRAM_BASE + (76 * 1024))
#define ECT_LINK_KEY_LOC                       (DRAM_PARAM_RESTORE_LOC)
#define DEBUG_FLOAT_DRAM_TOTAL_SIZE            (DRAM_PARAM_RESTORE_LOC + 0x30)



typedef unsigned long long u64;
typedef volatile unsigned long long			u64v;		//- 32-bit unsigned integer
#define __REG64(x)							(*((u64v *)((u64)x)))

typedef union {
	volatile unsigned long long data;
	struct {
		volatile unsigned long long type           :(3 - 0 + 1);
		volatile unsigned long long rank_num	   :(7 - 4 + 1);
		volatile unsigned long long manufacturer   :(15 - 8 + 1);
		volatile unsigned long long io_width1      :(19 - 16 + 1);
		volatile unsigned long long density1       :(23 - 20 + 1);
		volatile unsigned long long io_width0      :(27 - 24 + 1);
		volatile unsigned long long density0       :(31 - 28 + 1);
		volatile unsigned long long rev_id1        :(39 - 32 + 1);
		volatile unsigned long long rev_id2        :(47 - 40 + 1);
		volatile unsigned long long reserved_60_48   :(62 - 48 + 1);
		volatile unsigned long long flag_32        :(63 - 63 + 1);
	} bitfield;
} ect_key_t;

typedef struct tc_almighty_test_pattern {
	unsigned long	even_pattern;
	unsigned long	odd_pattern;
} tc_almighty_test_pattern_t;

typedef enum {
	ROW = 0,
	BANK,
	COLUMN,
	NONE,
} dram_address_type_t;

typedef struct tc_dram_address_type {
	const char				*name;
	dram_address_type_t		type;
	int						address_bit;
} tc_dram_address_type_t;

typedef struct almighty_core_test_info {

	unsigned int	pattern_num;
	int				address_type_num;
	unsigned long	start_address;
	unsigned long	end_address;

} almighty_core_test_info_t;

typedef struct almighty_skip_test_address_info {

	unsigned long	start_address;
	unsigned long	end_address;

} almighty_skip_test_address_info_t;

typedef struct almighty_system_info {

	// MIF DVFS info
	int				max_dvfs_lv;	// highest freq(boot level)
	int				min_dvfs_lv;
	int				freq_table[15];

	ect_key_t		ect_key;
	unsigned int	num_of_rank;

} almighty_system_info_t;

int almighty_pattern_test(int pattern_num);
int almighty_get_dram_freq(void);
void cpu_common_init(void);
extern void clean_invalidate_dcache_all(void);
extern void disable_mmu_dcache(void);
#endif
