/*
 * (C) Copyright 2017 SAMSUNG Electronics
 * Kiwoong Kim <kwmad.kim@samsung.com>
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 *
 */

#include <err.h>
#include <string.h>
#include <part_gpt.h>
#include <lib/heap.h>
#include <dev/boot.h>
#include <platform/decompress_ext4.h>
#include <platform/secure_boot.h>


/*
 * ---------------------------------------------------------------------------
 * Private macros
 * ---------------------------------------------------------------------------
 */
// TODO: need to be migrated..
#define FMP_USE_SIZE	(32 + 8)
#define PIT_SIGNITURE_SIZE		1024
#define CMD_STRING_MAX_SIZE		60
#define PIT_EMMC_ERASE_SIZE		(1024)

#define LOAD_PIT(t, s)							\
		memcpy((void *)(t), (void *)(s), sizeof(struct pit_info))\

enum __pit_if {
	PIT_NONE    = 0,
	PIT_MMC,
	PIT_UFS,
};

static const char *pit_if_tokens[] = {
	"unknown",
	"mmc",
	"scsi",
};

/*
 * ---------------------------------------------------------------------------
 * Private data
 * ---------------------------------------------------------------------------
 */
static struct pit_info pit;	/* pit cached data */
static void *pit_buf;		/* buffer for disk access with default block count */
static u32 pit_blk_cnt;		/* block count to access pit backed data */
static bdev_t *pit_dev;
static enum __pit_if pit_if;	/* block device identifier */
static struct gpt_info gpt_if;	/* GPT LBA range to give GPT */

/*
 * When you erase somewhere on eMMC supporting high capacity and
 * extended CSD, an erase unit is decided with HC_ERASE_GRP_SIZE,
 * that is the 224th member in extended CSD and thus, might be different
 * from its block size. However, it's almost 0x1, that 512KB.
 * So now we assume our erase unit is 512KB and this value would works
 * even with SD, because an erase unit of SD is the same as its block size
 * and it's always been 512B.
 */
static char nul_buf[PIT_EMMC_ERASE_SIZE * PIT_SECTOR_SIZE];

/*
 * ---------------------------------------------------------------------------
 * Function declaration
 * ---------------------------------------------------------------------------
 */

/*
 * ---------------------------------------------------------------------------
 * Private functions dependent on plaform
 * ---------------------------------------------------------------------------
 */
static inline u32 pit_get_last_lba(void)
{
	/*
	 * The unit is 512B.
	 * decreasing 1 is because it is last lba, not count.
	 */
	return (pit_dev) ? (pit_dev->block_count * (pit_dev->block_size / PIT_SECTOR_SIZE))
						- PIT_PART_META - 1 : 0;
}

static int pit_load_pit(void *buf)
{
	uint blks;

	blks = pit_dev->new_read(pit_dev, buf, PIT_DISK_LOC, pit_blk_cnt);

	if (blks != pit_blk_cnt)
		return ERR_IO;

	return NO_ERROR;
}

static int pit_save_pit(void *buf)
{
	uint blks;

	blks = pit_dev->new_write(pit_dev, buf, PIT_DISK_LOC, pit_blk_cnt);

	if (blks != pit_blk_cnt)
		return ERR_IO;

	return NO_ERROR;
}

static status_t pit_open_dev(void)
{
	unsigned int boot_dev;
	char str[CMD_STRING_MAX_SIZE];
	unsigned int len;

	boot_dev = get_boot_device();
	if (boot_dev == BOOT_UFS)
		pit_if = PIT_UFS;
	else if (boot_dev == BOOT_EMMC)
		pit_if = PIT_MMC;
	else
		pit_if = PIT_NONE;

	if (pit_if == PIT_NONE) {
		printf("[PIT] block dev not set\n");
	/*
		print_lcd_update(FONT_RED, FONT_BLACK,
			"[PIT] block dev not set");
	*/
		return ERR_NOT_VALID;
	} else {
		len = strlen(pit_if_tokens[pit_if]);
		memcpy(str, pit_if_tokens[pit_if], len);
		str[len] = '0';
		str[len + 1] = '\0';

		pit_dev = bio_open(str);

		return (pit_dev == 0) ? ERR_NOT_FOUND : NO_ERROR;
	}
}

static inline void pit_close_dev(void)		// TODO:
{
	bio_close(pit_dev);
}

u64 __pit_get_length(struct pit_entry *ptn)
{
	u64 blknum;

	/* Only the exception is for FMP */
	blknum = !strcmp("userdata", (const char *)ptn->name) ?
					ptn->blknum - FMP_USE_SIZE :
					ptn->blknum;

	return blknum * PIT_SECTOR_SIZE;
}

static int pit_erase_emmc(bdev_t *dev, u32 blkstart, u32 blknum)
{
	u32 lba = blkstart;
	u32 blknum_t;

	/* Partial write */
	if (lba % PIT_EMMC_ERASE_SIZE) {
		blknum_t = ((lba + PIT_EMMC_ERASE_SIZE - 1) / PIT_EMMC_ERASE_SIZE)
						* PIT_EMMC_ERASE_SIZE - lba;
		dev->new_write(dev, nul_buf, lba, blknum_t);
		lba += blknum_t;
		blknum -= blknum_t;
	}

	/* Erase with 512K unit */
	blknum_t = (blknum / PIT_EMMC_ERASE_SIZE) * PIT_EMMC_ERASE_SIZE;
	dev->new_erase(dev, lba, blknum_t);

	lba += blknum_t;
	blknum -= blknum_t;

	/* Partial write */
	blknum_t = blknum;
	if (blknum_t)
		dev->new_write(dev, nul_buf, lba, blknum_t);

	return blknum;
}

static int pit_access_emmc(struct pit_entry *ptn, int op, u64 addr, u32 size)
{
	int ret = 1;
	u64 bytestart = (u64)ptn->blkstart * PIT_SECTOR_SIZE;
	u32 blkstart = (u32)(bytestart / PIT_SECTOR_SIZE);
	u32 blknum = (u32)ptn->blknum;
	char str[CMD_STRING_MAX_SIZE];
	unsigned int len;

	bdev_t *dev;
	uint blks = 0;

	len = strlen(pit_if_tokens[pit_if]);
	memcpy(str, pit_if_tokens[pit_if], len);
	str[len] = '0' + ptn->lun;
	str[len + 1] = '\0';

	//printf("[PIT] bio_open %s %d\n", str, strlen(pit_if_tokens[pit_if]));
	dev = bio_open(str);

	/* only for boot partition of emmc */
#if 0
	if (ptn->lun == 1 || ptn->lun == 2) {
		sprintf(run_cmd, "emmc open %u", ptn->lun - 1);
		// TODO: run_command
		//run_command(run_cmd, 0);
	}
#endif


	switch (op) {
	case PIT_OP_FLASH:	/* flash */
		printf("\nPIT(%s): flash on eMMC..  \n", ptn->name);

		if (ptn->filesys == FS_TYPE_SPARSE_EXT4 || ptn->filesys == FS_TYPE_SPARSE_F2FS) {
#if 0
			... same as UFS...
#endif
				return NO_ERROR;
		} else {
			dev = bio_open(str);
			blks = dev->new_write(dev, (void *)addr, blkstart, blknum);
			bio_close(dev);
		}

#if 0
		if (!strcmp("ramdisk", ptn->name)) {
			blknum = (size + PIT_SECTOR_SIZE - 1) / PIT_SECTOR_SIZE;
			sprintf(ramdisk_size, "0x%x", size);
		// TODO: setenv
			/*
			setenv("rootfslen", ramdisk_size);
			saveenv();
			*/
		}
#endif
		blks = dev->new_write(dev, (void *)addr, blkstart, blknum);
		break;
	case PIT_OP_ERASE:	/* erase */
		printf("[PIT(%s)] erase on eMMC..  \n", ptn->name);
		/*
		 * There is possible not to erase eMMC with an unit of block size.
		 * In these casee, we need to do partial write.
		 */
		dev = bio_open(str);
		ret = pit_erase_emmc(dev, blkstart, blknum);
		bio_close(dev);
		break;
	case PIT_OP_LOAD:	/* load */
		dev = bio_open(str);
		printf("[PIT(%s)] load on eMMC..  \n", ptn->name);
		blks = dev->new_read(dev, (void *)addr, blkstart, blknum);
		bio_close(dev);
		break;
	default:
		printf("[PIT(%s)] Not supported op mode 0x%08x\n",
						ptn->name, op);
		// TODO: print_lcd_update
		/*
		print_lcd_update(FONT_RED, FONT_BLACK,
			"[PIT(%s)] Not supported op mode 0x%08x", ptn->name, op);
			*/
		break;
	}

	/* only for boot partition of emmc */
#if 0
	if (ptn->lun == 1 || ptn->lun == 2) {
		sprintf(run_cmd, "emmc close 0");
		// TODO: run_command
		//run_command(run_cmd, 0);
	}
#endif

	bio_close(dev);

	if (blks != blknum)
		ret = ERR_IO;
	else
		ret = NO_ERROR;


	return ret;
}

static int pit_access_ufs(struct pit_entry *ptn, int op, u64 addr, u32 size)
{
	int ret = 1;
	u64 bytestart = (u64)ptn->blkstart * PIT_SECTOR_SIZE;
	u32 blkstart = (u32)(bytestart / PIT_SECTOR_SIZE);
	u32 blknum = (u32)ptn->blknum;
	char str[CMD_STRING_MAX_SIZE];
	unsigned int len;

	bdev_t *dev;
	uint blks = 0;

	len = strlen(pit_if_tokens[pit_if]);
	memcpy(str, pit_if_tokens[pit_if], len);
	str[len] = '0' + ptn->lun;
	str[len + 1] = '\0';


	switch (op) {
	case PIT_OP_FLASH:	/* flash */
		printf("[PIT(%s)] flash on UFS..  \n", ptn->name);

		if (ptn->filesys == FS_TYPE_SPARSE_EXT4 || ptn->filesys == FS_TYPE_SPARSE_F2FS) {
			/* In this case, bio_open will be called in ext apis */
			if (!check_compress_ext4((char *)addr,
						__pit_get_length(ptn)) != 0) {
				printf("Compressed ext4 image\n");
				ret = write_compressed_ext4((char *)addr, blkstart);
			} else {
				printf("[PIT] %s flash failed on UFS\n", ptn->name);
				ret = ERR_IO;
			}

			if (ret != NO_ERROR)
				return ret;
			else
				blks = blknum;
		} else {
			dev = bio_open(str);
			blks = dev->new_write(dev, (void *)addr, blkstart, blknum);
			bio_close(dev);
		}
#if 0
		if (!strcmp("ramdisk", ptn->name)) {
			blknum = (size + PIT_SECTOR_SIZE - 1) / PIT_SECTOR_SIZE;
			sprintf(ramdisk_size, "0x%x", size);
		// TODO: setenv
			/*
			setenv("rootfslen", ramdisk_size);
			saveenv();
			*/
		}
#endif
		break;
	case PIT_OP_ERASE:	/* erase */
		printf("[PIT(%s)] erase on UFS..  \n", ptn->name);
		dev = bio_open(str);
		blks = dev->new_erase(dev, blkstart, blknum);
		bio_close(dev);
		break;
	case PIT_OP_LOAD:	/* load */
		printf("[PIT(%s)] load on UFS..  \n", ptn->name);
		dev = bio_open(str);
		blks = dev->new_read(dev, (void *)addr, blkstart, blknum);
		bio_close(dev);
		break;
	default:
		break;
	}

	if (blks != blknum)
		ret = ERR_IO;
	else
		ret = NO_ERROR;

	return ret;
}


/*
 * ---------------------------------------------------------------------------
 * CAUTION !!!
 *
 * Following functions should be not dependent on specific software platform
 * or tool chain. So I recommended that you use generic or pit-specific
 * data type and library functions here.
 * ---------------------------------------------------------------------------
 */

/*
 * ---------------------------------------------------------------------------
 * Common private functions
 * ---------------------------------------------------------------------------
 */
static int pit_check_header(struct pit_info *ppit)
{
	int ret = 1;

	if (ppit->hdr.magic != PIT_MAGIC) {
		printf("[PIT] magic(0x%x)is corrupted. Not 0x%x\n",
					ppit->hdr.magic, PIT_MAGIC);
		// TODO: print_lcd_update
		/*
		print_lcd_update(FONT_RED, FONT_BLACK,
			"[PIT] magic(0x%x)is corrupted. Not 0x%x",
				ppit->hdr.magic, PIT_MAGIC);
			*/
		goto err;
	}

	if (ppit->hdr.count > PIT_MAX_PART_NUM) {
		printf("[PIT] too many partitions. (%d < %d)\n",
					PIT_MAX_PART_NUM, ppit->hdr.count);
		// TODO: print_lcd_update
		/*
		print_lcd_update(FONT_RED, FONT_BLACK,
			"[PIT] too many partitions. (%d < %d)",
				PIT_MAX_PART_NUM, ppit->hdr.count);
			*/
		goto err;
	}

	ret = 0;
err:
	return ret;
}

static int pit_check_info(struct pit_info *ppit, int *idx,
						u32 lun,
						u32 *non_gpt_end)
{
	u32 i;
	int ret = 1;
	struct pit_entry *ptn;
	u32 lba = *non_gpt_end;

	for (i = *idx ; i < ppit->hdr.count; i++) {
		ptn = &ppit->pte[i];

		/* Rule for order */
		if (lun > ptn->lun) {
			printf("[PIT(%s)] PIT entries are out of order: curr=%u, new=%u\n",
					ptn->name, lun, ptn->lun);
		// TODO: print_lcd_update
		/*
			print_lcd_update(FONT_RED, FONT_BLACK,
				"[PIT(%s)] PIT entries are out of order: curr=%u, new=%u",
					ptn->name, lun, ptn->lun);
			*/
			goto err;
		}

		/* Do not something here for user, so terminate */
		if (lun != ptn->lun)
			break;
		if (lun == 0 && (ptn->filesys == FS_TYPE_SPARSE_EXT4 ||
					ptn->filesys == FS_TYPE_SPARSE_F2FS ||
					ptn->filesys == FS_TYPE_BASIC))
			break;

		/* Rule for concatenation and size in the same lun */
		if (ptn->blknum == 0) {
			printf("[PIT(%s)] size 0\n", ptn->name);
		// TODO: print_lcd_update
		/*
			print_lcd_update(FONT_RED, FONT_BLACK,
				"[PIT(%s)] size 0", ptn->name);
			*/
			goto err;
		}

		/* Rule for name */
		if (!strlen((const char *)ptn->name)) {
			printf("[PIT] %dth entry of lu %d has no name.\n",
							i, lun);
		// TODO: print_lcd_update
		/*
			print_lcd_update(FONT_RED, FONT_BLACK,
				"[PIT] %dth entry of lu %d has no name.",
							i, lun);
			*/
			goto err;
		}

		ptn->blkstart = lba;
		lba += ptn->blknum;
	}

	*idx = i;
	*non_gpt_end = lba;
	ret = 0;
err:
	return ret;
}

static int pit_check_info_gpt(struct pit_info *ppit, int *idx, struct gpt_info *gpt_if)
{
	u32 i;
	int ret = 1;
	struct pit_entry *ptn;
	u32 remained_idx = 0xFFFFFFFF;
	u32 last_idx;
	u32 total_size = 0;

	u32 blknum;

	u32 lun = 0;		/* here is only for part 0*/
	u32 startlba = gpt_if->gpt_start_lba;
	u32 lastlba = gpt_if->gpt_last_lba;
	u32 lba = startlba;
	bool fixed_started = false;

	for (i = *idx ; i < ppit->hdr.count; i++) {
		ptn = &ppit->pte[i];
#ifdef PIT_DEBUG
		printf("---------------------------------\n");
		printf("filesys: %d\n", ptn->filesys);
		printf("blkstart: %u\n", ptn->blkstart);
		printf("blknum: %u\n", ptn->blknum);
		printf("lun: %u\n", ptn->lun);
		printf("name: %s\n", ptn->name);
		printf("option: %s\n\n\n", ptn->option);
#endif

		/* Do not something here for user, so terminate */
		if (lun != ptn->lun)
			break;

		if (!strncmp((const char *)ptn->option, "remained", 8)) {
			if (remained_idx == 0xFFFFFFFF) {
				remained_idx = i;
			} else {
				printf("[PIT(%s)] muliple remaineds\n", ptn->name);
		// TODO: print_lcd_update
		/*
				print_lcd_update(FONT_RED, FONT_BLACK,
					"[PIT(%s)] muliple remaineds", ptn->name);
			*/
				goto err;
			}
		}

		/* Rule for filesys */
		if (ptn->filesys != FS_TYPE_SPARSE_EXT4 &&
					ptn->filesys != FS_TYPE_SPARSE_F2FS &&
					ptn->filesys != FS_TYPE_BASIC) {
			printf("[PIT(%s)] unknown filesys in user: filesys=%u.\n",
					ptn->name, ptn->filesys);
		// TODO: print_lcd_update
		/*
			print_lcd_update(FONT_RED, FONT_BLACK,
				"[PIT(%s)] unknown filesys in user: filesys=%u.",
					ptn->name, ptn->filesys);
			*/
			goto err;
		}

		/* Rule for size in lun0 except for 'remained' case*/
		if (ptn->blknum == 0 && strncmp((const char *)ptn->option, "remained", 8)) {
			printf("[PIT(%s)] size 0, option=%s.\n", ptn->name, ptn->option);
		// TODO: print_lcd_update
		/*
			print_lcd_update(FONT_RED, FONT_BLACK,
				"[PIT(%s)] size 0, option=%s.", ptn->name, ptn->option);
			*/
			goto err;
		}

		/* Rule for alignement */
		if (ptn->blknum % PIT_LBA_ALIGMENT) {
			printf("[PIT(%s)] 4KB non-aligned.\n", ptn->name);
		// TODO: print_lcd_update
		/*
			print_lcd_update(FONT_RED, FONT_BLACK,
				"[PIT(%s)] 4KB non-aligned.", ptn->name);
			*/
			goto err;
		}

		/* Rule for size */
		if (ptn->blknum < (5 * 1024 * 2) &&
				(ptn->filesys == FS_TYPE_SPARSE_EXT4 || ptn->filesys == FS_TYPE_SPARSE_F2FS) &&
				strncmp((const char *)ptn->option, "remained", 8)) {
			printf("[PIT(%s)] smaller than 5MB\n", ptn->name);
		// TODO: print_lcd_update
		/*
			print_lcd_update(FONT_RED, FONT_BLACK,
				"[PIT(%s)] smaller than 5MB", ptn->name);
			*/
			goto err;
		}

		/*
		 * Rule for fixed location
		 *
		 * All the fixed location partitions should be adjacent
		 * at the end of user physical partition to guarentee
		 * keeping data in there even if you update PIT again.
		 *
		 * ID	FILESYS	BLKNUM	LUN	NAME	OPTION
		 * ...
		 * 260	..	..	0	..	(blank)
		 * 261	..	..	0	..	fixed	<-- Legal
		 * 262	..	..	0	..	fixed	<-- Legal
		 * 263	..	..	0	..	(blank)	<-- Illegal !!
		 * 264	..	..	0	..	fixed	<-- Legal
		 * 265	..	..	0	..	fixed	<-- Legal
		 * 266	..	..	0	..	(blank)	<-- Illegal
		 * 512	..	..	1	..	(blank)
		 * (end)
		 */
		if (fixed_started && strncmp((const char *)ptn->option, "fixed", 8)) {
			printf("[PIT(%s)] un-fixed partition mixed at the end of this PIT\n", ptn->name);
			// TODO: print_lcd_update
			/*
			   print_lcd_update(FONT_RED, FONT_BLACK,
			   "[PIT(%s)] un-fixed partition mixed at the end of this PIT", ptn->name);
			 */
			goto err;
		}

		/*
		 * A partition with fixed location is enumerated for the first time.
		 */
		if (!strncmp((const char *)ptn->option, "fixed", 8))
			fixed_started = true;

		total_size += ptn->blknum;
	}

	last_idx = i;

	if (remained_idx != 0xFFFFFFFF) {
		blknum = (lastlba - startlba + 1 - total_size) /
						PIT_LBA_ALIGMENT;
		ppit->pte[remained_idx].blknum = blknum * PIT_LBA_ALIGMENT;
	}

	/*
	 * Override blkstart referring to each size
	 */
	for (i = *idx ; i < last_idx ; i++) {
		ptn = &ppit->pte[i];

		ptn->blkstart = lba;
		lba += ptn->blknum;
	}

	gpt_if->gpt_entry_cnt = last_idx - *idx;
	*idx = last_idx;
	ret = 0;
err:
	return ret;
}

static int pit_lba_cumulation(int only_check)
{
	int pit_index = 0;
	u32 lun;
	u32 lun_start_lba = 0;

	/*
	 * check pit entries and build input string to build gpt
	 *
	 * PIT entries are supposed to be described as follows and
	 * they should not be mixed to another group
	 * e.g.) An entry in LUN1 should not exist in the middle of
	 * entry array of LUN0
	 *
	 * LUN0 > LUN1 > LUN2
	 */

	/* for non gpt entries of part 0 */
	lun_start_lba = PIT_DISK_LOC;
	if (pit_check_info(&pit, (int *)&pit_index, 0, (u32 *)&lun_start_lba))
		goto err;

	/* set 4K align */
	if(lun_start_lba & (PIT_LBA_ALIGMENT-1)) {
		lun_start_lba = lun_start_lba + PIT_LBA_ALIGMENT;
		lun_start_lba = (lun_start_lba / PIT_LBA_ALIGMENT) * PIT_LBA_ALIGMENT;
	}

	gpt_if.gpt_start_lba = lun_start_lba;
	gpt_if.gpt_last_lba = pit_get_last_lba();

	/* for gpt entries of part 0 */
	if (pit_check_info_gpt(&pit, (int *)&pit_index, &gpt_if))
		goto err;

	/* GPT check */
	if (only_check && gpt_compare_chk(&pit, &gpt_if))
		goto err;

	/* for entries of others */
	for (lun = 1; ; lun++) {
		lun_start_lba = 0;
		if (pit_check_info(&pit, (int *)&pit_index, lun, (u32 *)&lun_start_lba))
			goto err;
		if (pit.hdr.count == (u32)pit_index)
			break;
	}

	return 0;
err:
	return 1;
}

static void pit_show_info(void)
{
	struct pit_entry *ptn;
	u32 i;

	printf("================= Partition Information Table =================\n");
	printf("%12s:\t%7s\t%15s\t%15s\t%7s\n",
					"NAME",
					"FILESYS",
					"BLKSTART(512B)",
					"BLKNUM(512B)",
					"PARTNUM");
	printf("---------------------------------------------------------------\n");

	for (i = 0; i < pit.hdr.count; i++) {
		ptn = &pit.pte[i];

		printf("%12s:\t%7u\t%15u\t%15u\t%7u\n",
						ptn->name,
						ptn->filesys,
						ptn->blkstart,
						ptn->blknum,
						ptn->lun);
	}
	printf("===============================================================\n");
}

static struct pit_entry *__pit_get_part_info(const char *name)
{
	u32 i;
	struct pit_entry *ptn;

	for (i = 0 ; i < pit.hdr.count; i++) {
		ptn = &pit.pte[i];

		if (strlen(name) == strlen((const char *)ptn->name)) {
			if (!strcmp(name, (const char *)ptn->name))
				return ptn;
		}
	}

	printf("[PIT(%s)] it doesn't exist in pit\n", name);
	// TODO: print_lcd_update
	/*
	   print_lcd_update(FONT_RED, FONT_BLACK,
	   "[PIT(%s)] it doesn't exist in pit", name);
	 */

	return 0;
}

static int pit_check_format(void)
{
	int res = 0;

	if (sizeof(struct pit_entry) != (PIT_SECTOR_SIZE / 2)) {
		printf("PIT: PIT entry size %lu is not 256 bytes !\n",
				sizeof(struct pit_entry));
		res = -1;
	} else if (sizeof(struct pit_header) != (PIT_SECTOR_SIZE / 2)) {
		printf("PIT: PIT header size %lu is not 256 bytes !\n",
				sizeof(struct pit_header));
		res = -1;
	} else if ((sizeof(struct pit_info) % (PIT_SECTOR_SIZE * 8)) != 0) {
		printf("PIT: PIT info size %lu is not 4096 bytes aligned !\n",
				sizeof(struct pit_info));
		res = -1;
	} else if (sizeof(struct pit_info) > PIT_SIZE_LIMIT) {
		printf("PIT: PIT info size %lu is bigger than %u !\n",
				sizeof(struct pit_info), PIT_SIZE_LIMIT);
		res = -1;
	}

	return res;
}

/*
 * ---------------------------------------------------------------------------
 * Common public functions
 * ---------------------------------------------------------------------------
 */

/**
 * Initialize PIT for PIT integrity, signiture verification and GPT verification
 *
 * This returns nothing, but if something wrong, that would show you something.
 */
void pit_init(void)
{
	int ret;

	printf("[PIT] pit init start (Max entries: %u, Max blk count: %u)\n",
			PIT_MAX_PART_NUM, PIT_DISK_SIZE_LIMIT);

	pit_buf = malloc(PIT_SIZE_LIMIT + PIT_SIGNITURE_SIZE);
	if (!pit_buf) {
		printf("[PIT] pit_buf not allocated !!\n");
		goto err;
	}

	/*
	 * Alignment rules
	 *
	 * Sizes of both of PIT header and entry should be 256 bytes.
	 * Whenever you change some fields' names, the sizes of header and an entry
	 * should be the same as before.
	 * And the total size of PIT should be 4096 aligned because it makes
	 * its debugging easier.
	 */
	if (pit_check_format())
		goto err;

	/*
	 * Set PIT block count as default, because here is the time
	 * before reading PIT and thus, we don't know what PIT block size
	 * is exactly.
	 */
	pit_blk_cnt = PIT_DISK_SIZE_LIMIT;

	/* Load pit data */
	if (NO_ERROR != pit_open_dev())
		goto err;
	pit_load_pit(pit_buf);
	pit_close_dev();
	LOAD_PIT(&pit, pit_buf);

	/*
	ret = el3_verify_signature_using_image((uint64_t)pit_buf,
			sizeof(struct pit_info) + PIT_SIGNITURE_SIZE);
	if (ret) {
		printf("[SB ERR] pit signature check fail [ret: 0x%X]\n", ret);
	} else {
		printf("pit signature check success\n");
	}
	*/

	/* Calculation Start LBA */
	ret = pit_lba_cumulation(1);
	if (ret != 0)
		goto err;

	/* Clear buffer for partition writes */
	memset(nul_buf, 0, sizeof(nul_buf));

	/* Check if PIT is valid and set PIT block count */
	ret = pit_check_header(&pit);
	if (!ret) {
		struct pit_entry *ptn;

		ptn = __pit_get_part_info("pit");
		if (!ptn)
			goto err;
		pit_blk_cnt = ptn->blknum;

		/* Calculation userdata lba */
		printf("... [PIT] pit init passes\n");
		pit_show_info();

		return;
	}

err:
	/* Disable PIT, so you can't access partitions */
	pit_blk_cnt = 0xDEADBEAF;
	pit.hdr.magic = 0xDEADBEAF;

	printf("... [PIT] pit init fails !!!\n");
	return;
}

/**
 * Update PIT and GPT
 *
 * @buf: start memory address
 * @size: downloaded PIT binary size
 *
 * Returns 0 if pit is valid and GPT is updated properly, otherwise 1.
 */
int pit_update(void *buf, u32 size)
{
	struct pit_entry *ptn;
	int ret;


	/*
	 * When pit backed data is loaded in pit_init(), bootloader always uses
	 * PIT_DISK_SIZE_LIMIT as block count because it doesn't know the value
	 * at the time. If pit partition size is bigger than PIT_DISK_SIZE_LIMIT,
	 * bootloader might not load partition table properly. So I limited
	 * available block count of pit partition in here.
	 */
	if (size > PIT_SIZE_LIMIT) {
		printf("[PIT] %d size is bigger than %d.\n", size, PIT_SIZE_LIMIT);
		// TODO: print_lcd_update
		/*
		print_lcd_update(FONT_RED, FONT_BLACK,
			"[PIT] %d size is bigger than %d.", size, PIT_SIZE_LIMIT);
			*/
		goto err;
	}

	LOAD_PIT(&pit, buf);

	/*
	 * Check pit header's integrity
	 */
	if (pit_check_header(&pit))
		goto err;

	/* Check if PIT is valid and set PIT block count */
	ptn = __pit_get_part_info("pit");
	if (!ptn)
		goto err;
	pit_blk_cnt = ptn->blknum;

	LOAD_PIT(pit_buf, &pit);

	/* Save pure pit binary */
	if (NO_ERROR != pit_open_dev())
		goto err;
	pit_save_pit(pit_buf);
	pit_close_dev();

	/* Calculation Start LBA */
	ret = pit_lba_cumulation(0);
	if (ret != 0)
		goto err;

	/* update gpt */
	if (gpt_create(&pit, &gpt_if)) {

		/*
		// TODO: print_lcd_update
		print_lcd_update(FONT_RED, FONT_BLACK, "[PIT] GPT update failed !");
		 */
		printf("[PIT] GPT update failed !\n\n");
		goto err;
	}
	printf("[PIT] pit updated\n\n");
	/* display all entries */
	pit_show_info();

	return 0;
err:
	return 1;
}

/**
 * Return data structure of target partition name
 *
 * @name: target partition name
 *
 * Returns 0 if pit header is invalid, otherwise its data structure.
 */
struct pit_entry *pit_get_part_info(const char *name)
{
	if (pit_check_header(&pit))
		return 0;

	return __pit_get_part_info(name);
}

/**
 * Return available block count for target partition
 * In userdata partition, OS keeps encryption data at the end of the partition,
 * so we should exclude this amount.
 *
 * @ptn: target partition
 *
 * Returns 0 if pit header is invalid, otherwise available block count.
 */
u64 pit_get_length(struct pit_entry *ptn)
{
	if (pit_check_header(&pit))
		return 0;

	return __pit_get_length(ptn);
}

/**
 * Return if the reqeust is processed properly.
 *
 * @ptn: target partition
 * @op:
 *	PIT_OP_FLASH - write
 *	PIT_OP_ERASE - erase
 *	PIT_OP_LOAD - read
 * @addr: start memory address
 * @size: (not used)
 *
 * Returns
 * NO_ERROR(0) if the reqeusted size is processed,
 * 1 if pit header is invalid,
 * otherwise other values.
 */
int pit_access(struct pit_entry *ptn, int op, u64 addr, u32 size)
{
	if (pit_check_header(&pit))
		return 1;

	if (pit_if == PIT_MMC)
		return pit_access_emmc(ptn, op, addr, size);
	else if (pit_if == PIT_UFS)
		return pit_access_ufs(ptn, op, addr, size);
	else
		return 1;
}
