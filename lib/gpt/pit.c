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

#include <stddef.h>
#include <string.h>
#include <err.h>
#include <part_gpt.h>
#include <lib/heap.h>
#include <lib/sysparam.h>
#include <dev/boot.h>
#include <platform/decompress_ext4.h>

// TODO:
/*
#include <decompress_ext4.h>
#include <asm/arch/display.h>
*/

/*
 * ---------------------------------------------------------------------------
 * Private macros
 * ---------------------------------------------------------------------------
 */
// TODO: need to be migrated..
#define FMP_USE_SIZE	(32 + 8)

#define PIT_FAT_MB_SIZE			200	/* 200MB */
#define PIT_FAT_SIZE			\
		(PIT_FAT_MB_SIZE * 1024 * 1024 / PIT_SECTOR_SIZE)

#define CMD_STRING_MAX_SIZE		60

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
static struct pit_info pit;
static void *pit_buf;
static u32 pit_blk_cnt;
static bdev_t *pit_dev;
static enum __pit_if pit_if;

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
struct pit_entry *pit_get_part_info(const char *name);

/*
 * ---------------------------------------------------------------------------
 * Private functions dependent on plaform
 * ---------------------------------------------------------------------------
 */
static inline u32 pit_get_last_lba(void)
{
	/*
	 * The unit is 512B.
	 */
	return (pit_dev) ? (pit_dev->block_count * (pit_dev->block_size / PIT_SECTOR_SIZE))
						- PIT_PART_META : 0;
}

static int pit_load_pit(void *buf)
{
	uint blks;

	if (!pit_dev)
		return ERR_IO;

	blks = pit_dev->new_read(pit_dev, buf, PIT_DISK_LOC, pit_blk_cnt);

	if (blks != pit_blk_cnt)
		return ERR_IO;

	return NO_ERROR;
}

static int pit_save_pit(void *buf)
{
	uint blks;

	if (!pit_dev)
		return ERR_IO;

	blks = pit_dev->new_write(pit_dev, buf, PIT_DISK_LOC, pit_blk_cnt);

	if (blks != pit_blk_cnt)
		return ERR_IO;

	return NO_ERROR;
}

static inline void pit_open_dev(void)
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
	} else {
		len = strlen(pit_if_tokens[pit_if]);
		memcpy(str, pit_if_tokens[pit_if], len);
		str[len] = '0';
		str[len + 1] = '\0';

		pit_dev = bio_open(str);
	}
}

static inline void pit_close_dev(void)		// TODO:
{
	if (!pit_dev)
		return;

	bio_close(pit_dev);
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

		if (ptn->filesys == FS_TYPE_EXT4) {
#if 0
			... same as UFS...
#endif
				return NO_ERROR;
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
		ret = pit_erase_emmc(dev, blkstart, blknum);
		break;
	case PIT_OP_LOAD:	/* load */
		printf("[PIT(%s)] load on eMMC..  \n", ptn->name);
		blks = dev->new_read(dev, (void *)addr, blkstart, blknum);
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

	dev = bio_open(str);

	switch (op) {
	case PIT_OP_FLASH:	/* flash */
		printf("[PIT(%s)] flash on UFS..  \n", ptn->name);

		if (ptn->filesys == FS_TYPE_EXT4) {
			if (!check_compress_ext4((char *)addr,
						pit_get_length(ptn)) != 0) {
				printf("Compressed ext4 image\n");
				ret = write_compressed_ext4((char *)addr, blkstart);
			} else {
				printf("[PIT] %s flash failed on UFS\n", ptn->name);
				return 1;
			}

			return NO_ERROR;
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
		printf("[PIT(%s)] erase on UFS..  \n", ptn->name);
		blks = dev->new_erase(dev, blkstart, blknum);
		break;
	case PIT_OP_LOAD:	/* load */
		printf("[PIT(%s)] load on UFS..  \n", ptn->name);
		blks = dev->new_read(dev, (void *)addr, blkstart, blknum);
		break;
	default:
		break;
	}

	bio_close(dev);

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

	if (ppit->magic != PIT_MAGIC) {
		printf("[PIT] magic(0x%x)is corrupted. Not 0x%x\n",
					ppit->magic, PIT_MAGIC);
		// TODO: print_lcd_update
		/*
		print_lcd_update(FONT_RED, FONT_BLACK,
			"[PIT] magic(0x%x)is corrupted. Not 0x%x",
				ppit->magic, PIT_MAGIC);
			*/
		goto err;
	}

	if (ppit->count > PIT_MAX_PART_NUM) {
		printf("[PIT] too many partitions. (%d < %d)\n",
					PIT_MAX_PART_NUM, ppit->count);
		// TODO: print_lcd_update
		/*
		print_lcd_update(FONT_RED, FONT_BLACK,
			"[PIT] too many partitions. (%d < %d)",
				PIT_MAX_PART_NUM, ppit->count);
			*/
		goto err;
	}

	ret = 0;
err:
	return ret;
}

static int pit_check_info(struct pit_info *ppit, int *idx,
						u32 lun,
						u32 startlba)
{
	u32 i;
	int ret = 1;
	struct pit_entry *ptn;
	u32 lba = startlba;

	for (i = *idx ; i < ppit->count; i++) {
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
		if (lun == 0 && (ptn->filesys == FS_TYPE_EXT4 ||
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
		if (!strlen(ptn->name)) {
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

		/* Rule for env size */
		if (!strcmp("env", ptn->name) &&
					pit_get_length(ptn) > PIT_ENV_SIZE) {
			printf("PIT(env): the size %llu is bigger than %d.\n",
					pit_get_length(ptn), PIT_ENV_SIZE);
		// TODO: print_lcd_update
		/*
			print_lcd_update(FONT_RED, FONT_BLACK,
				"PIT(env): the size %llu is bigger than %d.",
					pit_get_length(ptn), PIT_ENV_SIZE);
			*/
			goto err;
		}

		ptn->blkstart = lba;
		lba += ptn->blknum;
	}

	*idx = i;
	ret = 0;
err:
	return ret;
}

static int pit_check_info_gpt(struct pit_info *ppit, int *idx)
{
	u32 i;
	int ret = 1;
	struct pit_entry *ptn;
	u32 remained_idx = 0xFFFFFFFF;
	u32 last_idx;
	u32 total_size = 0;

	u32 blknum;

	u32 lun = 0;		/* here is only for part 0*/
	u32 startlba =			/* adding 1 for protective mbr */
			PIT_FAT_SIZE + PIT_PART_META + 1;
	u32 lastlba = pit_get_last_lba();
	u32 lba = startlba;

	for (i = *idx ; i < ppit->count; i++) {
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

		if (!strncmp(ptn->option, "remained", 8)) {
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
		if (ptn->filesys != FS_TYPE_EXT4 &&
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
		if (ptn->blknum == 0 && strncmp(ptn->option, "remained", 8)) {
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
				ptn->filesys == FS_TYPE_EXT4 &&
				strncmp(ptn->option, "remained", 8)) {
			printf("[PIT(%s)] smaller than 5MB\n", ptn->name);
		// TODO: print_lcd_update
		/*
			print_lcd_update(FONT_RED, FONT_BLACK,
				"[PIT(%s)] smaller than 5MB", ptn->name);
			*/
			goto err;
		}

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

	*idx = last_idx;
	ret = 0;
err:
	return ret;
}

static int pit_copy_one_string(char **t, const char *s, char *org)
{
	size_t len;

	len = strlen(s);
	if (*t + len > org + PIT_GPT_STRING_SIZE) {
		printf("[PIT] gpt string overflow.\n");
		// TODO: print_lcd_update
		/*
		print_lcd_update(FONT_RED, FONT_BLACK,
			"[PIT] gpt string overflow.");
			*/
		return 1;
	}

	strncpy(*t, s, len);
	*t += len;
	return 0;
}


/*
 * ---------------------------------------------------------------------------
 * Common public functions
 * ---------------------------------------------------------------------------
 */
void pit_init(void)
{
	int ret;

	printf("[PIT] pit init start...\n");

	pit_buf = malloc(PIT_SIZE_LIMIT);
	if (!pit_buf) {
		printf("[PIT] pit_buf not allocated !!\n");
		goto err;
	}

	/* Set a block device to be accessed */
	pit_open_dev();

	/*
	 * Set PIT block count as default, because here is the time
	 * before reading PIT and thus, we don't know what PIT block size
	 * is exactly.
	 */
	pit_blk_cnt = PIT_DISK_SIZE_LIMIT;

	/* Load pit data */
	pit_load_pit(pit_buf);
	LOAD_PIT(&pit, pit_buf);

	/* Clear buffer for partition writes */
	memset(nul_buf, 0, sizeof(nul_buf));

	pit_close_dev();

	/* Check if PIT is valid and set PIT block count */
	ret = pit_check_header(&pit);
	if (!ret) {
		struct pit_entry *ptn;

		ptn = pit_get_part_info("pit");
		if (!ptn)
			goto err;
		pit_blk_cnt = ptn->blknum;
		printf("... [PIT] pit init passes\n");
		/* Get Sysparam */
		ptn = pit_get_part_info("sysparam");
		if (ptn) {
			status_t err;

			printf("[PIT] load sysparam\n");
			err = sysparam_scan(pit_dev, ptn->blkstart, ptn->blknum);
			if (err)
				printf("... [PIT] fail to load sysparam, error code %d\n", err);
			else
				printf("... [PIT] pass to load sysparam\n");
		}
		return;
	}
err:
	pit_blk_cnt = 0xDEADBEAF;
	printf("... [PIT] pit init fails !!!\n");
	return;
}

void pit_show_info()
{
	struct pit_entry *ptn;
	u32 i;
	int is_filesys = 0;

	if (pit_check_header(&pit))
		return;

	printf("================= Partition Information Table =================\n");
	printf("%12s:\t%7s\t%15s\t%15s\t%7s\n",
					"NAME",
					"FILESYS",
					"BLKSTART(512B)",
					"BLKNUM(512B)",
					"PARTNUM");
	printf("---------------------------------------------------------------\n");

	for (i = 0; i < pit.count; i++) {
		ptn = &pit.pte[i];

		if (is_filesys == 0 && ptn->filesys) {
			printf("%12s:\t%7s\t%15u\t%15u\t%7s\n",
						"(FAT)",
						"-------",
						PIT_DISK_LOC,
						PIT_FAT_SIZE,
						"-------");
			is_filesys = ptn->filesys;
		}
		printf("%12s:\t%7u\t%15u\t%15u\t%7u\n",
						ptn->name,
						ptn->filesys,
						ptn->blkstart,
						ptn->blknum,
						ptn->lun);
	}
	printf("===============================================================\n");
}

int pit_update(void *buf, u32 size)
{
	int pit_index = 0;
	u32 lun;
	struct pit_entry *ptn;


	pit_open_dev();

	/*
	 * Rule for name, not check the case of movi
	 * because you can't know download size at the time
	 */
	if (size != 0xDEADBEAF && size > PIT_SIZE_LIMIT) {
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
	if (pit_check_info(&pit, (int *)&pit_index, 0, PIT_DISK_LOC))
		goto err;

	/* for gpt entries of part 0 */
	if (pit_check_info_gpt(&pit, (int *)&pit_index))
		goto err;


	/* for entries of others */
	for (lun = 1; ; lun++) {
		if (pit_check_info(&pit, (int *)&pit_index, lun, 0))
			goto err;
		if (pit.count == (u32)pit_index)
			break;
	}

	/* update gpt */
	pit_close_dev();

	/*
	 * GPT would open the same device as one opened here.
	 * So, we close here temporarily.
	 */
	if (gpt_create(&pit)) {
		/*
		// TODO: print_lcd_update
		print_lcd_update(FONT_RED, FONT_BLACK, "[PIT] GPT update failed !");
		 */
		printf("[PIT] GPT update failed !\n\n");
		goto err;
	}
	pit_open_dev();

	/* display all entries */
	pit_show_info();

	/* Check if PIT is valid and set PIT block count */
	ptn = pit_get_part_info("pit");
	if (!ptn)
		goto err;
	pit_blk_cnt = ptn->blknum;

	/* update pit */
	LOAD_PIT(pit_buf, &pit);
	pit_save_pit(pit_buf);

	printf("[PIT] pit updated\n\n");
	pit_close_dev();

	return 0;
err:
	pit_close_dev();
	return 1;
}

struct pit_entry *pit_get_part_info(const char *name)
{
	u32 i;
	struct pit_entry *ptn;

	if (pit_check_header(&pit))
		return 0;

	for (i = 0 ; i < pit.count; i++) {
		ptn = &pit.pte[i];

		if (strlen(name) == strlen(ptn->name)) {
			if (!strcmp(name, ptn->name))
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

u64 pit_get_start_addr(struct pit_entry *ptn)
{
	return (u64)ptn->blkstart * PIT_SECTOR_SIZE;
}

u64 pit_get_length(struct pit_entry *ptn)
{
	u64 blknum;

	/* Only the exception is for FMP */
	blknum = !strcmp("userdata", ptn->name) ?
					ptn->blknum - FMP_USE_SIZE :
					ptn->blknum;

	return blknum * PIT_SECTOR_SIZE;
}

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
