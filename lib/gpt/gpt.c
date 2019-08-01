/*
 * (C) Copyright 2019 SAMSUNG Electronics
 * Kiwoong Kim <kwmad.kim@samsung.com>
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <lib/bio.h>
#include <lib/heap.h>
#include <dev/boot.h>
#include <part_dev.h>
#include <gpt.h>
#include "gpt_format.h"
#include <platform/decompress_ext4.h>
#include <platform/secure_boot.h>
#include <lib/sysparam.h>
#include <lib/cksum.h>
#include <lib/font_display.h>
#include <lib/sysparam.h>
#include <trace.h>

#define LOCAL_TRACE 0
#undef GPT_DEBUG
//#define GPT_DEBUG

#define CMD_STRING_MAX_SIZE		60

/*
 * ---------------------------------------------------------------------------
 * Private macros and definitions
 * ---------------------------------------------------------------------------
 */
enum __part_op {
	GPT_READ_OP	= 0,
	GPT_WRITE_OP,
	GPT_ERASE_OP,
	GPT_NUM_OF_OP,
};

/*
 * We assume some things.
 *
 * 1. Actual partitions for LU #0 are populated starting at this.
 * 2. There are no partition table (a.k.a. PT) for more than LU #8
 * 3. No matter which device it's, 1st usable area starts at 512KB.
 * 4. One GPT has 20KB space named 'chunk'
 * header (512B or 4KB) + entries (128B * 128)
 *
 * 5. One more chunk for LU #0 because of 2nd GPT verification
 * 6. The last entry in LU #0 expand with remained size of LU #0
 *
 * Partition table layout is like below.
 * +------------------+     0				(LBA 0)
 * +  Protective MBR  +
 * +  (512B or 4KB)   +
 * +------------------+     UFS: 4KB, MMC: 512B		(LBA 1)
 * +                  +     OS actually reads this.
 * +     GPT #0       +
 * +     (20KB)       +
 * +                  +
 * +------------------+
 * +                  +
 * +   GPT #0 backup  +
 * +     (20KB)       +
 * +                  +
 * +------------------+
 * +                  +
 * +     GPT #1       +
 * +     (20KB)       +
 * +                  +
 * +------------------+
 * +                  +
 * +     GPT #2       +
 * +     (20KB)       +
 * +                  +
 * +------------------+
 * +  Verification    +
 * +      Code        +
 * +     (4KB)        +
 * +------------------+
 * +                  +
 * +                  +
 * +   (Reserved)     +
 * +                  +
 * +                  +
 * +------------------+    512KB			(UFS: LBA 128, MMC: LBA 1K)
 * +//////////////////+
 * +/// Usable area //+
 * +///   ......    //+
 */
#define START_IN_BYTES_LU_ZERO	(512 * 1024)
#define CHUNK_SIZE_IN_BYTES	(20U * 1024U)
#define CHUNK_BASE_INDEX	1
#define SIGN_SIZE_IN_BYTES	(4 * 1024)
#define FMP_USE_SIZE		((32 + 8) * SECTOR_SIZE)

/* Protective MBR */
#define MSDOS_MBR_SIGNATURE 0xAA55
#define EFI_PMBR_OSTYPE_EFI_GPT 0xEE

/* We assume all storage has less than 4TB */
#define INV_LBA_IN_SECS		0xFFFFFFFF

/* Part entry magic */
#define PART_ENTRY_MAGIC	0x54524150

typedef struct _GPT_MANAGER {
	/* raw data but not used in public directley */
	u8 bdata[(CHUNK_BASE_INDEX + DEF_NUM_OF_LU) * CHUNK_SIZE_IN_BYTES + SIGN_SIZE_IN_BYTES];
	u8 *pgpt[CHUNK_BASE_INDEX + DEF_NUM_OF_LU];

	u32 cnt_e;
	PART_ENTRY part_entry[GPT_ENTRY_NUMBERS * DEF_NUM_OF_LU];
} GPT_MANAGER;

struct p_mbr_head {
	u8 boot_code[446];
	u8 active;
	u8 s_head;		/* starting head */
	u8 s_sector;		/* starting sector */
	u8 s_cylinder;		/* starting cylinder */
	u8 partition_type;
	u8 e_head;		/* end head */
	u8 e_sector;		/* end sector */
	u8 e_cylinder;		/* end cylinder */
	u32 start_sect_cnt;	/* starting sector counting form 0 */
	u32 nr_sect;
	u32 reserved[12];
	u16 signature;
} __attribute__ ((__packed__));

#define gpt_info(fmt, args...)	printf("[GPT] " fmt, ## args)
#define gpt_err(fmt, args...)							\
	do {									\
		printf("[GPT] %s:%u: " fmt, __func__, __LINE__, ## args);	\
		print_lcd_update(FONT_RED, FONT_BLACK, "[GPT]" fmt, ## args);		\
	} while (0)
/*
 * ---------------------------------------------------------------------------
 * Private data used by external functions
 * ---------------------------------------------------------------------------
 */
static GPT_MANAGER gpt_mgr;

/*
 * ---------------------------------------------------------------------------
 * Private data used like constants, decided at boot time or earlier
 * ---------------------------------------------------------------------------
 */
static const char *gpt_dev_tokens[] = {
	"mmc",
	"scsi",
	"unknown",
};

static const char *gpt_fs_type_tokens[] = {
	"",
	"",
	"ext4",
	"f2fs",
};

static enum __boot_dev_id s_gpt_dev_id;
static bdev_t *s_gpt_dev;
static u32 s_chunk_size;
static u32 s_block_in_bytes;
static u32 s_block_in_secs;
static u32 s_last_in_blks;


/*
 * ---------------------------------------------------------------------------
 * Private functions dependent on plaform
 * ---------------------------------------------------------------------------
 */
static inline int gpt_is_mgr_valid(GPT_MANAGER *mgr)
{
	return (mgr->cnt_e) ? 1 : 0;
}

static inline int gpt_is_valid(struct gpt_header *gpt_h)
{
	return (gpt_h->signature == GPT_HEADER_SIGNATURE) ? 1 : 0;
}

static inline PART_ENTRY *__gpt_get_entry(GPT_MANAGER *mgr, const char *name)
{
	int found = 0;
	PART_ENTRY *part_e = mgr->part_entry;

	while (part_e->gpt_entry) {
		if (!strcmp((const char *)part_e->name, name)) {
			found = 1;
			break;
		}
		part_e++;
	}

	if (!found)
		part_e = NULL;

	return part_e;
}

static int gpt_load_gpt(bdev_t *dev, void *buf, u64 size_in_bytes)
{
	int res = 0;
	u32 cnt;

	cnt = (u32)(size_in_bytes / s_block_in_bytes);
	res = dev->new_read_native(dev, buf, PMBR_LBA, cnt);

	if (res)
		gpt_err("fail to read GPTs as much as %u secs: %d\n", cnt, res);

	return res;
}

static int gpt_save_gpt(bdev_t *dev, void *buf, u32 start_in_blks, u64 size_in_bytes)
{
	int res = 0;
	u32 cnt;

	cnt = (u32)(size_in_bytes / s_block_in_bytes);
	res = dev->new_write_native(dev, buf, start_in_blks, cnt);

	if (res)
		gpt_err("fail to write GPTs as much as %u secs: %d\n", cnt, res);

	return res;
}

static int gpt_open_dev(bdev_t **dev, u8 lun)
{
	char str[CMD_STRING_MAX_SIZE];
	unsigned int len;

	if (s_gpt_dev_id == DEV_NONE)
		return -1;

	len = strlen(gpt_dev_tokens[s_gpt_dev_id]);
	memcpy(str, gpt_dev_tokens[s_gpt_dev_id], len);
	str[len] = '0' + lun;
	str[len + 1] = '\0';

	*dev = bio_open(str);
	if (!(*dev))
		return -2;

	return 0;
}

static void gpt_close_dev(bdev_t *dev)		// TODO:
{
	bio_close(dev);
}

/* This would be used to update last entry */
static struct gpt_entry *gpt_get_last_gpt_entry(GPT_MANAGER *mgr, bdev_t *dev, u8 lun)
{
	u8 *base = mgr->pgpt[CHUNK_BASE_INDEX + lun];

	struct gpt_header *gpt_h =
		(struct gpt_header *)base;
	struct gpt_entry *gpt_e =
		(struct gpt_entry *)(base + s_block_in_bytes) + gpt_h->part_num_entry - 1;

	return gpt_e;
}

static int gpt_complete_undecisable_info(GPT_MANAGER *mgr, bdev_t *dev, u8 lun, u32 chunk_in_bytes)
{
	int res = 0;
	u64 start_in_blks;
	struct gpt_entry *gpt_e;
	struct gpt_header *gpt_h;
	char name[UID_STR_LEN];
	u32 i;
	u32 part_in_blks;
	u32 rsvd_in_blks;
	u64 gpt_in_bytes;


	/*
	 * Calculate part_end_lba of last partition with remained size
	 * except the amount for GPT backup and reserved area to store FMP keys
	 */
	gpt_e = gpt_get_last_gpt_entry(mgr, dev, lun);
	for (i = 0 ; i < UID_STR_LEN ; i++)
		name[i] = (u8)gpt_e->part_name[i];
	start_in_blks = gpt_e->part_start_lba;

	chunk_in_bytes = ROUNDUP(chunk_in_bytes, s_block_in_bytes);

	rsvd_in_blks = ROUNDUP(chunk_in_bytes, s_block_in_bytes) / s_block_in_bytes;
	if (!strcmp(name, "userdata"))
		rsvd_in_blks += ROUNDUP(FMP_USE_SIZE, s_block_in_bytes) / s_block_in_bytes;

	part_in_blks = (s_last_in_blks + 1) - start_in_blks;

	if (part_in_blks < rsvd_in_blks) {

		gpt_err("Remained size %u of LU #%u is equal or smaller than GPT backup size %u\n",
				part_in_blks, lun, rsvd_in_blks);
		gpt_err("You might assign too much capacity to other partitions, except with last entry of LU #%u\n", lun);
		res = -1;
		goto end;
	} else {
		part_in_blks -= rsvd_in_blks;
		gpt_e->part_end_lba = start_in_blks + part_in_blks - 1;
	}

	/* Calculate the rest of undecisable things */
	gpt_h = (struct gpt_header *)(mgr->pgpt[CHUNK_BASE_INDEX + lun]);
	gpt_h->gpt_back_header = s_last_in_blks;

	gpt_in_bytes = s_block_in_bytes + (u64)GPT_ENTRY_NUMBERS * gpt_h->part_size_entry;
	gpt_in_bytes = ROUNDUP(gpt_in_bytes, s_block_in_bytes);
	gpt_h->end_lba = s_last_in_blks - gpt_in_bytes / s_block_in_bytes;

	gpt_e = (struct gpt_entry *)(((u8 *)gpt_h) + s_block_in_bytes);
	gpt_h->part_table_crc = crc32(0, (const unsigned char *)gpt_e,
		gpt_h->part_num_entry * gpt_h->part_size_entry);
	gpt_h->head_crc = 0;
	gpt_h->head_crc = crc32(0, (const unsigned char *)gpt_h, gpt_h->head_sz);

end:
	return res;
}

/* It's called once at boot time */
static int gpt_verify(GPT_MANAGER *mgr)
{
	u32 chunk_in_bytes = s_chunk_size;
	int res;

	/* 1st GPT verification */
#if FIRST_GPT_VERIFY
	u8 *p = mgr->pgpt[CHUNK_BASE_INDEX];

	res = el3_verify_signature_using_image((uint64_t)&p, chunk_in_bytes, 0);
	if (res) {
		gpt_err("1st GPT verification fails: 0x%X\n", res);
		goto end;
	} else {
		gpt_info("1st GPT verification passes\n");
		print_lcd_update(FONT_GREEN, FONT_BLACK,
				"1st GPT verification passes");
	}
#endif

	/*
	 * With assumption #6, the last entry of LU #0 expands and
	 * its related information is updated
	 */
	res = gpt_complete_undecisable_info(mgr, s_gpt_dev, 0, chunk_in_bytes);
	if (res)
		goto end;

	/* 2nd GPT verification for LU #0 */
	if (memcmp(mgr->pgpt[0], mgr->pgpt[1], chunk_in_bytes)) {
		gpt_err("2nd GPT for LU #0 verification fails\n");
		res = -1;
		goto end;
	} else {
		gpt_info("2nd GPT for LU #0 verification passes\n");
		print_lcd_update(FONT_GREEN, FONT_BLACK,
				"2nd GPT verification passes");
	}

end:
	return res;
}

static int gpt_build_entry_table_cache(GPT_MANAGER *mgr, bdev_t *dev)
{
	int res = 0;
	u8 i, chunk_idx;
	u32 j, k;
	PART_ENTRY *entry;
	struct gpt_header *gpt_h;
	struct gpt_entry *gpt_e;

	/* Init cache */
	mgr->cnt_e = 0;
	memset(mgr->part_entry, 0, sizeof(PART_ENTRY) * GPT_ENTRY_NUMBERS * DEF_NUM_OF_LU);

	/*
	 * GPT #0 backup is not used to build entry table cache.
	 * It's just mapped to its pointer to be used
	 * to copy from fastboot in the future.
	 */
	mgr->pgpt[CHUNK_BASE_INDEX] = mgr->bdata + CHUNK_SIZE_IN_BYTES;
	for (i = 0; i < DEF_NUM_OF_LU; i++) {
		/*
		 * Chunk #1 is a GPT #0 backup to verify GPT #0,
		 * so, it might not provide the final information,
		 * especially after gpt_update because the chunk
		 * is set to default data to store
		 *
		 * Thus, when entry table case is built,
		 * the chunk is not referred.
		 */
		chunk_idx = (i == 0) ? i : i + CHUNK_BASE_INDEX;

		/* Set GPT pointers */
		gpt_h = (struct gpt_header *)(mgr->bdata + CHUNK_SIZE_IN_BYTES * chunk_idx);
		mgr->pgpt[chunk_idx] = (u8 *)gpt_h;
		if (!gpt_is_valid(gpt_h)) {
			gpt_err("GPT #%u header invalid\n", i);
			res = -1;
			continue;
		}

		/* Build entry table cache */
		gpt_e = (struct gpt_entry *)(mgr->bdata + CHUNK_SIZE_IN_BYTES * chunk_idx + s_block_in_bytes);
		entry = &mgr->part_entry[mgr->cnt_e];
		for (k = 0 ; k < gpt_h->part_num_entry ; k++) {

			/*
			 * GPT has entry name as unicode,
			 * so it will be translated into normal string
			 */
			for (j = 0 ; j < UID_STR_LEN ; j++)
				entry->name[j] = (u8)gpt_e->part_name[j];
			entry->gpt_entry = (void *)gpt_e;
			entry->lun = i;
			entry->magic = 0x54524150;

			/* Count all entries */
			mgr->cnt_e++;
			entry++;
			gpt_e++;
		}

		if (mgr->cnt_e == 0) {
			gpt_info("No entries for LU %u\n", i);
			res = -1;
		}
	}
end:
	return res;
}

static void gpt_dump_header(GPT_MANAGER *mgr, u32 len)
{
#ifdef GPT_DEBUG
	u32 i;
	u8 *p = mgr->bdata;
	printf("------------------\n");
	//for (i = 0 ; i < len; i++) {
	for (i = 0 ; i < len; i++) {
		if (i % 0x10 == 0)
			printf("\n");
		printf("%02x ", *(p + i));
	}
	printf("\n");
#endif
}

/* It's called once at boot time */
static int gpt_set_dev_and_build_manager(GPT_MANAGER *mgr, u32 chunk_in_bytes)
{
	bdev_t *dev;
	int res = 0;
	u64 total_in_bytes;

	/* Open block device to load or save GPTs that are located on LU #0 */
	res = gpt_open_dev(&dev, 0);
	if (res) {
		gpt_err("GPT dev not opened: %d\n", res);
		goto end;
	} else if (GPT_MAX_HEADER_SIZE < dev->block_size) {
		gpt_err("GPT block size %lu is smaller than %u\n", dev->block_size, GPT_MAX_HEADER_SIZE);
		res = -1;
		goto end;
	} else if (CHUNK_SIZE_IN_BYTES % dev->block_size) {
		gpt_err("Chunk size %u is not aligned with %lu\n", CHUNK_SIZE_IN_BYTES, dev->block_size);
		res = -1;
		goto end;
	} else if (SIGN_SIZE_IN_BYTES % dev->block_size) {
		gpt_err("Sign size %u is not aligned with %lu\n", SIGN_SIZE_IN_BYTES, dev->block_size);
		res = -1;
		goto end;
	}

	/* Check more */
	if (CHUNK_SIZE_IN_BYTES	< chunk_in_bytes) {
		gpt_err("CHUNK_SIZE_IN_BYTES %u is bigger than %u\n", CHUNK_SIZE_IN_BYTES, chunk_in_bytes);
		res = -1;
		goto end;
	}
	total_in_bytes = (u64)dev->block_size + chunk_in_bytes * (CHUNK_BASE_INDEX + DEF_NUM_OF_LU) + SIGN_SIZE_IN_BYTES;
	if (START_IN_BYTES_LU_ZERO < total_in_bytes) {
		gpt_err("Start byte offset %u of LU #0 is less than %llu\n", START_IN_BYTES_LU_ZERO, total_in_bytes);
		res = -1;
		goto end;
	}

	/* Set private data */
	s_gpt_dev = dev;
	s_block_in_bytes = dev->block_size;
	s_block_in_secs = dev->block_size / SECTOR_SIZE;
	s_last_in_blks = dev->block_count - 1;

	/* Load chunks for necessary information and set pointers */
	total_in_bytes -= dev->block_size;
	res = gpt_load_gpt(dev, (void *)mgr->bdata, total_in_bytes);
	if (res)
		goto err_close;

	gpt_dump_header(mgr, 0x60);

	res = gpt_build_entry_table_cache(mgr, dev);

err_close:
	/* Close block dev */
	gpt_close_dev(dev);
end:
	return res;
}

static int gpt_set_protective_mbr(bdev_t *dev)
{
	struct p_mbr_head *p_mbr = NULL;
	u32 block_in_bytes = s_block_in_bytes;
	int res;

	p_mbr = malloc(block_in_bytes);
	if (!p_mbr) {
		printf("p_mbr alloc failed\n");
		return -1;
	}

	memset(p_mbr, 0, block_in_bytes);
	p_mbr->signature = MSDOS_MBR_SIGNATURE;
	p_mbr->partition_type = EFI_PMBR_OSTYPE_EFI_GPT;
	p_mbr->start_sect_cnt = PMBR_LBA;
	p_mbr->nr_sect = (s_last_in_blks - 1) * s_block_in_secs;

	/* Write P MBR BLOCK */
	res = dev->new_write_native(dev, p_mbr, 0, 1);
	if (res) {
		gpt_err("fail to write PMBR\n");
		printf("\n");
		goto end;
	}

	gpt_info("P_MBR write done\n");
	free((void *)p_mbr);
end:
	return 0;
}

static struct gpt_entry *__gpt_get_gpt_entry(PART_ENTRY *part_e, const char *caller)
{
	struct gpt_entry *gpt_e = NULL;

	if (!part_e)
		gpt_err("Part entry is null in caller %s\n", caller);
	else if (part_e->magic != PART_ENTRY_MAGIC)
		gpt_err("Part entry is null in caller %s: 0x%08X\n", caller, part_e->magic);
	else
		gpt_e = (struct gpt_entry *)part_e->gpt_entry;

	return gpt_e;
}

static u32 gpt_get_offset_in_secs(PART_ENTRY *part_e, u64 _offset_in_bytes, u32 block_in_secs, u32 block_in_bytes)
{
	struct gpt_entry *gpt_e;
	u32 last_in_secs;
	u64 start_in_bytes;
	u32 offset_in_secs;

	/*
	 * Check and calculate start in secs,
	 * so _offset_in_bytes could be permitted from 0 to last sector in partition
	 */
	gpt_e = (struct gpt_entry *)part_e->gpt_entry;
	last_in_secs = (gpt_e->part_end_lba + 1) * block_in_secs - 1;
	start_in_bytes = gpt_e->part_start_lba * block_in_bytes;

	if (_offset_in_bytes) {
		if (_offset_in_bytes % SECTOR_SIZE) {
			gpt_err("offset %llu is not aligned with %u for '%s'\n", _offset_in_bytes, SECTOR_SIZE, part_e->name);
			offset_in_secs = INV_LBA_IN_SECS;
		} else if ((start_in_bytes + _offset_in_bytes) / SECTOR_SIZE > last_in_secs) {
			gpt_err("offset %llu is too far from start for '%s'\n", _offset_in_bytes, part_e->name);
			offset_in_secs = INV_LBA_IN_SECS;
		} else
			offset_in_secs = (u32)(_offset_in_bytes / (u64)SECTOR_SIZE);
	} else
		offset_in_secs = 0;

end:
	return offset_in_secs;
}

static u32 gpt_get_size_in_secs(PART_ENTRY *part_e, u64 offset_in_bytes, u64 _size_in_bytes, u32 block_in_secs)
{
	u32 size_in_secs;
	struct gpt_entry *gpt_e;
	u64 size_in_bytes;

	/*
	 * Check and calculate size in secs,
	 * so _size_in_bytes could be permitted from 1 to total sectors in partition
	 */
	gpt_e = (struct gpt_entry *)part_e->gpt_entry;
	size_in_bytes = (gpt_e->part_end_lba - gpt_e->part_start_lba + 1) * s_block_in_bytes;

	if (_size_in_bytes) {
		if (offset_in_bytes + _size_in_bytes > size_in_bytes) {
			gpt_err("size %llu is bigger than %llu for '%s', %llu, %llu\n",
					_size_in_bytes, size_in_bytes - offset_in_bytes, part_e->name, offset_in_bytes, size_in_bytes);
			size_in_bytes = 0;
		} else
			size_in_bytes = _size_in_bytes;
	} else
		size_in_bytes -= offset_in_bytes;

	/* Need to align with block size to prevent losing non-aligned data */
	size_in_bytes = ROUNDUP(size_in_bytes, (u64)SECTOR_SIZE);
	size_in_secs = size_in_bytes / SECTOR_SIZE;
end:
	return size_in_secs;
}

static u32 gpt_access_disk(void* buf, enum __part_op op, char *name, u8 lun, u32 offset_in_secs, u32 size_in_secs)
{
	bdev_t *dev;
	const char *op_tokens[] = {
		"read",
		"write",
		"erase",
	};
	u32 secs;
	int res;

	res = gpt_open_dev(&dev, lun);
	if (res == 0) {
		switch (op) {
		case GPT_READ_OP:
			secs = dev->new_read(dev, buf, offset_in_secs, size_in_secs);
			break;
		case GPT_WRITE_OP:
			secs = dev->new_write(dev, buf, offset_in_secs, size_in_secs);
			break;
		case GPT_ERASE_OP:
			secs = dev->new_erase(dev, offset_in_secs, size_in_secs);
			break;
		default:
			gpt_close_dev(dev);
			gpt_err("invalid opcode %u\n", op);
			secs = 0;
			break;
		}

		if (secs != size_in_secs) {
			gpt_close_dev(dev);
			gpt_err("fail to %s %u sectors in '%s': %u\n",
					op_tokens[op], size_in_secs, name, secs);
			secs = 0;
			goto end;
		}
	} else {
		gpt_err("GPT dev not opened: %d\n", res);
		secs = 0;
		goto end;
	}

	/* Close block dev */
	gpt_close_dev(dev);
end:
	return secs;
}

static int gpt_access_part(GPT_MANAGER *mgr, enum __part_op op,
		PART_ENTRY *part_e, void* buf, u64 _offset_in_bytes, u64 _size_in_bytes)
{
	int res = 0;
	u64 part_in_bytes;
	u32 block_in_secs = s_block_in_bytes / SECTOR_SIZE;
	struct gpt_entry *gpt_e;
	u32 offset_in_secs;
	u32 size_in_secs;
	u32 secs;

	if (!part_e) {
		gpt_err("Part entry is null\n");
		goto end;
	} else if (part_e->magic != PART_ENTRY_MAGIC) {
		gpt_err("Part entry is null: 0x%08X\n", part_e->magic);
		goto end;
	}
	gpt_e = (struct gpt_entry *)part_e->gpt_entry;
#ifdef GPT_DEBUG
	gpt_info("%s: %s 0x%08X\n", __func__, part_e->name, (((u32)gpt_e->part_start_lba) * block_in_secs));
#endif

	/* sparse case, input offset and size are ignored */
	if (op == GPT_WRITE_OP && gpt_e->attributes.type >= FS_TYPE_SPARSE_EXT4 &&
					gpt_e->attributes.type < FS_TYPE_NUM) {
		part_in_bytes = (gpt_e->part_end_lba - gpt_e->part_start_lba + 1) * s_block_in_bytes;
		if (check_compress_ext4((char *)buf, part_in_bytes) == 0) {
			printf("Compressed %s image\n", gpt_fs_type_tokens[gpt_e->attributes.type]);
			res = write_compressed_ext4((char *)buf,
				gpt_e->part_start_lba * block_in_secs);
		} else {
			gpt_err("fail to write '%s'\n", part_e->name);
			res = -1;
		}
		goto end;
	}

	/*
	 * Check and calculate start in secs,
	 * so _offset_in_bytes could be permitted from 0 to last sector in partition.
	 * And we assume that 2TB storage device doesn't exist.
	 */
	offset_in_secs = gpt_get_offset_in_secs(part_e, _offset_in_bytes, block_in_secs, s_block_in_bytes);
	if (INV_LBA_IN_SECS == offset_in_secs) {
		res = -1;
		goto end;
	}

	/*
	 * Check and calculate size in secs,
	 * so _size_in_byte could be permitted from 1 to total sectors in partition
	 * The input offset_in_secs must be non-zero.
	 */
	size_in_secs = gpt_get_size_in_secs(part_e, offset_in_secs * SECTOR_SIZE, _size_in_bytes, block_in_secs);
	if (0 == size_in_secs) {
		res = -1;
		goto end;
	}

	/* Raw access case */
	secs = gpt_access_disk(buf, op, part_e->name, part_e->lun, gpt_e->part_start_lba * s_block_in_secs + offset_in_secs, size_in_secs);
	if (secs == 0)
		res = -1;
end:
	return res;
}

static void __gpt_show_uuid(void)
{
	u8 *p;
	struct gpt_header *gpt_h;
	struct gpt_entry *gpt_e;
	PART_ENTRY *part_e;
	u32 i;
	u8 lun, chunk_idx;
	unsigned char *uuid_bin;
	char buf[37];

	if (!gpt_is_mgr_valid(&gpt_mgr)) {
		gpt_err("GPT wasn't actived properly\n");
		return;
	}

	printf("\n");
	part_e = gpt_mgr.part_entry;
	for (lun = 0; lun < DEF_NUM_OF_LU; lun++) {
		chunk_idx = (lun == 0) ? lun : lun + CHUNK_BASE_INDEX;
		p = (u8 *)gpt_mgr.pgpt[chunk_idx];

		/* Check if there is any entry in here */
		gpt_h = (struct gpt_header *)p;
		if (!gpt_h->part_num_entry)
			break;

		printf("================ UUID Information ===================\n\n");

		/* Print entries */
		gpt_e = (struct gpt_entry *)(p + s_block_in_bytes);
		for (i = 0 ; i < gpt_h->part_num_entry; i++) {
			uuid_bin = (unsigned char *)gpt_e->part_guid.b;
			uid_bin_to_str(uuid_bin, buf, UID_STR_GUID);
			printf("%-15s\t%-45s\n",
					part_e->name,
					buf);
			gpt_e++;
			part_e++;
		}
		printf("\n\n");
	}
}

static void __gpt_show_info(void)
{
	u8 *p;
	struct gpt_header *gpt_h;
	struct gpt_entry *gpt_e;
	PART_ENTRY *part_e;
	u32 i;
	u8 lun, chunk_idx;

	if (!gpt_is_mgr_valid(&gpt_mgr)) {
		gpt_err("GPT wasn't actived properly\n");
		return;
	}

	printf("\n");
	part_e = gpt_mgr.part_entry;
	for (lun = 0; lun < DEF_NUM_OF_LU; lun++) {
		chunk_idx = (lun == 0) ? lun : lun + CHUNK_BASE_INDEX;
		p = (u8 *)gpt_mgr.pgpt[chunk_idx];

		/* Check if there is any entry in here */
		gpt_h = (struct gpt_header *)p;
		if (!gpt_h->part_num_entry)
			break;

		printf("======= GUID PARTITION TABLE (LU #%u, Total: %u entries) =======\n\n", lun, gpt_h->part_num_entry);

		/* Print header */
		printf("%-29s\t%-15llu\n", "MyLBA", gpt_h->gpt_header);
		printf("%-29s\t%-15llu\n", "AlternateLBA", gpt_h->gpt_back_header);
		printf("%-29s\t%-15llu\n", "FirstUsableLBA", gpt_h->start_lba);
		printf("%-29s\t%-15llu\n", "LastUsableLBA", gpt_h->end_lba);
		printf("%-29s\t%-15u\n", "NumberOfPartitionEntries", gpt_h->part_num_entry);

		printf("\n");

		/* Print entries */
		gpt_e = (struct gpt_entry *)(p + s_block_in_bytes);
		printf("%-15s\t%-15s\t%-15s\t%-15s\n\n",
				"FS type", "PartitionName", "StartingLBA", "EndingLBA");
		for (i = 0 ; i < gpt_h->part_num_entry; i++) {
			printf("%-15s\t%-15s\t%-15llu\t%-llu\n",
					part_e->name,
					gpt_fs_type_tokens[gpt_e->attributes.type],
					gpt_e->part_start_lba,
					gpt_e->part_end_lba);
			gpt_e++;
			part_e++;
		}
		printf("\n\n");
	}
}

static void gpt_sysparam_scan(GPT_MANAGER *mgr)
{
	PART_ENTRY *part_e = mgr->part_entry;
	status_t sts;
	bdev_t *dev;
	u32 start_in_secs;
	u32 size_in_secs;
	struct gpt_entry *gpt_e;
	int res = 0;

	part_e = __gpt_get_entry(mgr, "env");
	if (!part_e)
		return;

	res = gpt_open_dev(&dev, part_e->lun);
	if (res) {
		gpt_err("GPT dev not opened: %d\n", res);
		return;
	}

	gpt_e = (struct gpt_entry *)part_e->gpt_entry;
	start_in_secs = gpt_e->part_start_lba * s_block_in_secs;
	size_in_secs = (gpt_e->part_end_lba + 1) * s_block_in_secs;

	LTRACEF("GPT Block Start:0x%x, blknum:0x%x\n", start_in_secs, size_in_secs);
	sts = sysparam_scan(dev, start_in_secs * SECTOR_SIZE, size_in_secs * SECTOR_SIZE);
	if (sts < 0)
		LTRACEF("sysparam scan fail, error code:%d\n", sts);
	else
		LTRACEF("sysparam size:%d\n", sts);

	gpt_close_dev(dev);
}

/*
 * ---------------------------------------------------------------------------
 * Common public functions
 * ---------------------------------------------------------------------------
 */
void gpt_init(enum __boot_dev_id id)
{
	int res;

	/* Init private data */
	s_gpt_dev_id = id;
	s_gpt_dev = NULL;
	s_chunk_size = 0;
	s_block_in_bytes = 0;
	s_block_in_secs = 0;
	s_last_in_blks = 0;

	printf("\n\n");
	printf("-------------------------------------------------------------\n");
	printf("\n[GPT] gpt init start...\n");

	/* Init memory */
	memset((void *)&gpt_mgr, 0, sizeof(GPT_MANAGER));

	/* Set private data that doesn't depends on boot device */
	s_chunk_size = GPT_MAX_HEADER_SIZE + GPT_ENTRY_NUMBERS * sizeof(struct gpt_entry);

	/*
	 * Set device for GPT access and build GPT manager,
	 * which provide GPT raw data and related infomation
	 * In here, gpt_dev and its related stuffs are set
	 */
	res = gpt_set_dev_and_build_manager(&gpt_mgr, s_chunk_size);
	if (res)
		goto end;

	/* Verify GPTs and APT */
	res = gpt_verify(&gpt_mgr);
	if (res)
		goto end;

	/* Show GPT */
	__gpt_show_info();

end:
	if (res) {
		gpt_mgr.cnt_e = 0;
		gpt_err("... gpt init fails !!!\n");
	}
	printf("-------------------------------------------------------------\n\n");

	return;
}

int gpt_update(u8 *buf)
{
	int res = 0;
	u32 i;
	u64 size_in_bytes;
	u32 start_in_blks;
	struct gpt_header *gpt_h =
		(struct gpt_header *)(gpt_mgr.bdata);

	/* Check incoming GPT header */
	for (i = 0; i < DEF_NUM_OF_LU; i++) {
		if (!gpt_is_valid((struct gpt_header *)(buf + CHUNK_SIZE_IN_BYTES * i))) {
			res = -1;
			goto end;
		}
	}

	/* Copy to GPT manager */
	memcpy(gpt_mgr.pgpt[CHUNK_BASE_INDEX], buf, s_chunk_size * DEF_NUM_OF_LU + SIGN_SIZE_IN_BYTES);

	/* Complete the rest for LU #0 */
	res = gpt_complete_undecisable_info(&gpt_mgr, s_gpt_dev, 0, s_chunk_size);
	if (res)
		goto end;

	/*
	 * Copy to GPT cache for area that OS access
	 * Copy to GPT backup cache with original data
	 * that is used to use for verification
	 */
	memcpy(gpt_mgr.pgpt[0], gpt_mgr.pgpt[1], s_chunk_size);
	memcpy(gpt_mgr.pgpt[1], buf, s_chunk_size);

	gpt_dump_header(&gpt_mgr, 0x60);

	/* Build entry table cache */
	res = gpt_build_entry_table_cache(&gpt_mgr, s_gpt_dev);
	if (res) {
		res = -1;
		goto end;
	}

	/* Write GPTs including GPT #0 backup */
	size_in_bytes = ((u64)s_chunk_size * (CHUNK_BASE_INDEX + DEF_NUM_OF_LU)) + SIGN_SIZE_IN_BYTES;
	start_in_blks = (u32)gpt_h->gpt_header;
	res = gpt_save_gpt(s_gpt_dev, (void *)gpt_mgr.bdata, start_in_blks, size_in_bytes);
	if (res) {
		res = -1;
		goto end;
	}

	/* Write alternative GPT header for LU #0 */
	size_in_bytes = (u64)(s_block_in_bytes);
	start_in_blks = (u32)gpt_h->gpt_back_header;
	res = gpt_save_gpt(s_gpt_dev, (void *)gpt_mgr.bdata, start_in_blks, size_in_bytes);
	if (res) {
		res = -1;
		goto end;
	}

	/* Write alternative GPT entries for LU #0 */
	size_in_bytes = (u64)(gpt_h->part_num_entry * gpt_h->part_size_entry);
	size_in_bytes = ROUNDUP(size_in_bytes, s_block_in_bytes);
	start_in_blks -= size_in_bytes / s_block_in_bytes;
	res = gpt_save_gpt(s_gpt_dev, (void *)(gpt_mgr.bdata + s_block_in_bytes), start_in_blks, size_in_bytes);
	if (res) {
		res = -1;
		goto end;
	}

	/* Write PMBR */
	res = gpt_set_protective_mbr(s_gpt_dev);
	if (res) {
		res = -1;
		goto end;
	}

	/* Show result */
	__gpt_show_info();
	__gpt_show_uuid();
end:
	return res;
}

void gpt_show_info(void)
{
	__gpt_show_info();
}

PART_ENTRY *gpt_get_entry(const char *name)
{
	if (!gpt_is_mgr_valid(&gpt_mgr)) {
		gpt_err("GPT wasn't actived properly\n");
		return NULL;
	}

	return __gpt_get_entry(&gpt_mgr, name);
}

char *gpt_get_fs_type(PART_ENTRY *part_e)
{
	struct gpt_entry *gpt_e = __gpt_get_gpt_entry(part_e, __func__);
	const char *str = NULL;
	u8 type;

	if (!gpt_e) {
		gpt_err("GPT entry is null\n");
		goto end;
	}

	gpt_e = (struct gpt_entry *)part_e->gpt_entry;
	type = gpt_e->attributes.type & 0xF;
	if (type < FS_TYPE_NUM)
		str = gpt_fs_type_tokens[type];
	else
		gpt_err("This entry's fs type %u is invalid\n", type);

end:
	return (char *)str;
}

u32 gpt_get_lun(PART_ENTRY *part_e)
{
	u32 lun = INV_LBA_IN_SECS;

	if (!part_e)
		gpt_err("Part entry is null\n");
	else if (part_e->magic != PART_ENTRY_MAGIC)
		gpt_err("Part entry is null: 0x%08X\n", part_e->magic);
	else
		lun = (u32)part_e->lun;

	return lun;
}

int gpt_get_part_unique_guid(PART_ENTRY *part_e, char *buf)
{
	struct gpt_entry *gpt_e = __gpt_get_gpt_entry(part_e, __func__);

	uid_bin_to_str((unsigned char *)gpt_e->part_guid.b, buf, UID_STR_GUID);

	return 0;
}

u32 gpt_get_part_start_in_blks(PART_ENTRY *part_e)
{
	struct gpt_entry *gpt_e = __gpt_get_gpt_entry(part_e, __func__);

	return (gpt_e) ? (u32)(gpt_e->part_start_lba) : INV_LBA_IN_SECS;
}

u32 gpt_get_part_start_in_secs(PART_ENTRY *part_e)
{
	struct gpt_entry *gpt_e = __gpt_get_gpt_entry(part_e, __func__);
	u64 size = 0;

	if (gpt_e)
		size = ((u64)gpt_e->part_start_lba) * s_block_in_bytes / SECTOR_SIZE;

	return (u32)size;
}

u64 gpt_get_part_size_in_bytes(PART_ENTRY *part_e)
{
	struct gpt_entry *gpt_e = __gpt_get_gpt_entry(part_e, __func__);
	u64 size = 0;

	if (gpt_e)
		size = ((gpt_e->part_end_lba - gpt_e->part_start_lba + 1) * s_block_in_bytes);

	return size;
}

int gpt_read_part(PART_ENTRY *part_e, void *buf, u64 offset_in_bytes, u64 size_in_bytes)
{
	return gpt_access_part(&gpt_mgr, GPT_READ_OP, part_e, buf, offset_in_bytes, size_in_bytes);
}

int gpt_write_part(PART_ENTRY *part_e, void *buf, u64 offset_in_bytes, u64 size_in_bytes)
{
	return gpt_access_part(&gpt_mgr, GPT_WRITE_OP, part_e, buf, offset_in_bytes, size_in_bytes);
}

int gpt_erase_part(PART_ENTRY *part_e, u64 offset_in_bytes, u64 size_in_bytes)
{
	return gpt_access_part(&gpt_mgr, GPT_ERASE_OP, part_e, NULL, offset_in_bytes, size_in_bytes);
}

int gpt_read_raw(void *buf, u32 start_in_secs, u32 *_size_in_secs)
{
	u32 size_in_secs = *_size_in_secs;
	u32 done;
	u32 last = (s_last_in_blks + 1) * s_block_in_secs - 1;

	/* Only support LU #0 */
	*_size_in_secs = size_in_secs =
		MIN(size_in_secs, last - start_in_secs + 1);
	done = gpt_access_disk(buf, GPT_READ_OP, NULL, 0, start_in_secs, size_in_secs);
	if (done == size_in_secs)
		return 0;
	else
		return -1;
}

void gpt_get_range_by_name(const char *name, u32 *start_in_secs, u32 *size_in_secs)
{
	PART_ENTRY *part_e = __gpt_get_entry(&gpt_mgr, name);
	struct gpt_entry *gpt_e;

	if (!part_e || !part_e->gpt_entry) {
		*start_in_secs = 0;
		*size_in_secs = 0;
	} else {
		gpt_e = part_e->gpt_entry;
		*start_in_secs = gpt_e->part_start_lba * s_block_in_secs;
		*size_in_secs = gpt_e->part_end_lba * s_block_in_secs;
	}
}

void gpt_get_range_by_range(u32 *start_in_secs, u32 *size_in_secs)
{
	u32 remained;
	u32 last = (s_last_in_blks + 1) * s_block_in_secs - 1;

	if (*start_in_secs <= last) {
		remained = last - *start_in_secs + 1;
		*size_in_secs = MIN(remained, *size_in_secs);
	} else {
		*start_in_secs = 0;
		*size_in_secs = 0;
	}
}
