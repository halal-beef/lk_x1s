/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <reg.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <part_gpt.h>
#include <guid.h>
#include <ctype.h>
#include <lib/console.h>
#include <lib/cksum.h>
#include <dev/boot.h>

u32 num_blk_size;

static int set_protective_mbr(bdev_t *dev)
{
	struct p_mbr_head *p_mbr = NULL;
	u32 size;
	u32 write_size;

	p_mbr = malloc(sizeof(struct p_mbr_head) * num_blk_size);
	if (!p_mbr) {
		printf("p_mbr alloc failed\n");
		return -1;
	}

	memset(p_mbr, 0, sizeof(struct p_mbr_head) * num_blk_size);
	p_mbr->signature = MSDOS_MBR_SIGNATURE;
	p_mbr->partition_type = EFI_PMBR_OSTYPE_EFI_GPT;
	p_mbr->start_sect_cnt = PMBR_LBA;
	p_mbr->nr_sect = ((dev->block_count + 1) * num_blk_size) - 1;

	/* Write P MBR BLOCK */
	write_size = PMBR_LBA * num_blk_size;
	size = dev->new_write(dev, p_mbr, 0, write_size);
	if (size != write_size) {
		printf("write failed\n");
		return -1;
	}

	printf("P_MBR write done\n");
	return 0;
}

static int set_gpt_header(bdev_t *dev, struct gpt_header *gpt_h,
		struct gpt_part_table *gpt_e, struct gpt_info *gpt_if)
{
	char *str_disk_guid;

	gpt_h->signature = GPT_HEADER_SIGNATURE;
	gpt_h->revision = GPT_HEADER_REVISION_V1;
	gpt_h->head_sz = HEAD_SIZE;
	gpt_h->gpt_header = GPT_HEAD_LBA;
	gpt_h->gpt_back_header = dev->block_count - 1;

	/* gpt unit is 4096 */
	gpt_h->start_lba = gpt_if->gpt_start_lba  / num_blk_size;

	/* use last lba from pit */
	gpt_h->end_lba = gpt_if->gpt_last_lba / num_blk_size;

	gpt_h->part_table_lba = GPT_TABLE_LBA;
	gpt_h->part_num_entry = gpt_if->gpt_entry_cnt;
	gpt_h->part_size_entry = sizeof(struct gpt_part_table);
	gpt_h->head_crc = 0;
	gpt_h->part_table_crc = 0;

	str_disk_guid = strdup(UUID_DISK);
	if (uid_str_to_bin(str_disk_guid, gpt_h->guid, UID_STR_GUID)) {
		printf("disk guid set fail\n");
		return -1;
	}

	return 0;
}

static int set_partition_table(bdev_t *dev, struct gpt_header *gpt_h,
		struct gpt_part_table *gpt_e, struct pit_info *pit, bool print)
{
	uint32_t offset = gpt_h->start_lba;
	uint32_t end_use_lba = gpt_h->end_lba;
	uint32_t start;
	int part_cnt;
	int ret = 0;
	char *unique_guid;
	char part_number[2];
	u32 i, j, name_len;

	if (print) {
		printf("Set up start for Partition Table\n");
		printf("The default size is 512Byte\n");
		printf("==============================================================\n");
	}

	for (i = 0, part_cnt = 0; i < pit->count; i++) {
		/* partition start lba */
		if (pit->pte[i].filesys == 0)
			continue;

		start = offset;
		gpt_e[part_cnt].part_start_lba = start;

		offset = start + (pit->pte[i].blknum / num_blk_size);
		if (offset > end_use_lba) {
			printf("partition layout over disk size %u > %u\n", offset, end_use_lba);
			//goto out;
		}
		gpt_e[part_cnt].part_end_lba = offset - 1;
		if (print) {
			printf("Start LBA : %d\t Size : %d\t\t", start * num_blk_size,
					(offset - start) * num_blk_size);
		}

		/* Unique guid */
		unique_guid = strdup(EXYNOS_UNIQUE_GUID);
		if (!unique_guid) {
			printf("Unique GUID strdup fail\n");
			goto out;
		}
		sprintf(part_number, "%02x", part_cnt + 1);
		strcat(unique_guid, part_number);
		ret = uid_str_to_bin(unique_guid, gpt_e[part_cnt].part_guid.b, UID_STR_STD);
		if (ret) {
			printf("Unique guid set fail : %s\n", pit->pte[i].name);
			goto free;
		}
		if (print)
			printf("Unique GUID : %s\t name : ", unique_guid);
		free(unique_guid);

		/* partition type */
		memcpy(gpt_e[part_cnt].part_type.b, &PARTITION_BASIC_DATA_GUID, 16);
		name_len = sizeof(pit->pte[i].name);
		memset(gpt_e[part_cnt].part_name, 0, sizeof(gpt_e[part_cnt].part_name));
		for (j = 0; j < name_len; j++) {
			gpt_e[part_cnt].part_name[j] = (u16)(pit->pte[i].name[j]);
			if (print)
				printf("%c", gpt_e[part_cnt].part_name[j]);
		}
		if (print)
			printf("\n");
		part_cnt++;
	}

	if (print)
		printf("==============================================================\n");

	return 0;
free:
	free(unique_guid);
out:
	return -1;
}

static int write_gpt_table(bdev_t *dev, struct gpt_header *gpt_h,
		struct gpt_part_table *gpt_e)
{
	int pte_blk_cnt;
	int ret;
	u32 calc_crc32;
	u32 start_blk;
	u32 write_size;
	u32 size;

	/* Setup the Protective MBR */
	ret = set_protective_mbr(dev);
	if (ret) {
		printf("set_protective_mbr fail\n");
		return -1;
	}

	pte_blk_cnt = ((GPT_ENTRY_NUMBERS * sizeof(struct gpt_part_table) - 1) /
			(dev->block_size + 1)) * num_blk_size;

	/* Generate CRC for the primary GPT header */
	calc_crc32 = crc32(0, (const unsigned char *)gpt_e,
		gpt_h->part_num_entry * gpt_h->part_size_entry);
	gpt_h->part_table_crc = calc_crc32;
	calc_crc32 = crc32(0, (const unsigned char *)gpt_h, gpt_h->head_sz);
	gpt_h->head_crc = calc_crc32;

	/* GPT header write */
	start_blk = GPT_HEAD_LBA * num_blk_size;
	write_size = num_blk_size * 1;
	size = dev->new_write(dev, gpt_h, start_blk, write_size);
	if (size != write_size) {
		printf("GPT HEAD write fail\n");
		return -1;
	}

	/* GPT primary entry table write */
	start_blk = GPT_TABLE_LBA * num_blk_size;
	write_size = pte_blk_cnt;
	size = dev->new_write(dev, gpt_e, start_blk, write_size);
	if (size != write_size) {
		printf("Partition Table write Fail\n");
		return -1;
	}

	/* Recalculate the values for the backup GPT header */
	gpt_h->gpt_header = gpt_h->gpt_back_header;
	gpt_h->gpt_back_header = GPT_HEAD_LBA;
	calc_crc32 = crc32(0, (const unsigned char *)gpt_h, gpt_h->head_sz);
	gpt_h->head_crc = calc_crc32;

	/* Backup GPT primary entry table write */
	start_blk = (gpt_h->end_lba + 1) * num_blk_size;
	write_size = pte_blk_cnt;
	size = dev->new_write(dev, gpt_e, start_blk, write_size);
	if (size != write_size) {
		printf("Back up partition table write Fail\n");
		return -1;
	}

	/* Backup GPT header write */
	start_blk = gpt_h->gpt_header * num_blk_size;
	write_size = num_blk_size * 1;
	size = dev->new_write(dev, gpt_h, start_blk, write_size);
	if (size != write_size) {
		printf("Back up Gpt Head write fail\n");
		return -1;
	}

	printf("GPT successfully written to block device!\n");

	return 0;
}

int gpt_create(struct pit_info *pit, struct gpt_info *gpt_if)
{
	struct gpt_header *gpt_h = NULL;
	struct gpt_part_table *gpt_e = NULL;
	unsigned int boot_dev;
	int ret = -1;
	bdev_t *dev;

	boot_dev = get_boot_device();
	if (boot_dev == BOOT_UFS) {
		dev = bio_open("scsi0");
		num_blk_size = 8;
	} else {
		dev = bio_open("mmc0");
		num_blk_size = 1;
	}
	if (!dev) {
		printf("%s: fail to bio open\n", __func__);
		return ret;
	}

	gpt_h = malloc(sizeof(struct gpt_header) * num_blk_size);
	if (!gpt_h) {
		printf("%s: gpt_h malloc failed!\n", __func__);
		goto out;
	}

	memset(gpt_h, 0, sizeof(struct gpt_header) * num_blk_size);

	gpt_e = malloc(sizeof(struct gpt_part_table) * GPT_ENTRY_NUMBERS);
	if (!gpt_e) {
		printf("%s: gpt_e malloc failed!\n", __func__);
		goto entry_free;
	}

	memset(gpt_e, 0, sizeof(struct gpt_part_table) * GPT_ENTRY_NUMBERS);

	ret = set_gpt_header(dev, gpt_h, gpt_e, gpt_if);
	if (ret) {
		printf("Set gpt header fail\n");
		goto entry_free;
	}

	ret = set_partition_table(dev, gpt_h, gpt_e, pit, true);
	if (ret) {
		printf("Set partition table fail\n");
		goto entry_free;
	}

	ret = write_gpt_table(dev, gpt_h, gpt_e);
	if (ret) {
		printf("gpt write fail\n");
		goto entry_free;
	}

entry_free:
	free(gpt_e);
head_free:
	free(gpt_h);
out:
	bio_close(dev);
	return ret;
}

static int gpt_dump(int argc, const cmd_args *argv)
{
	unsigned char *buf;
	unsigned int boot_dev, offset = 0;
	int block, err = -1;
	bdev_t *dev;
	u32 i, size;
	u32 start_blk;
	u32 read_size;

	boot_dev = get_boot_device();
	if (boot_dev == BOOT_UFS) {
		dev = bio_open("scsi0");
		num_blk_size = 8;
	} else {
		dev = bio_open("mmc0");
		num_blk_size = 1;
	}
	if (!dev) {
		printf("%s: fail to bio open\n", __func__);
		return err;
	}

	buf = malloc(dev->block_size);
	if (!buf) {
		printf("%s: alloc fail\n", __func__);
		goto out;
	}

	memset(buf, 0, dev->block_size);

	printf("partition Read\n");
	block = argv[1].u;
	printf("start block %d\n", block);
	num_blk_size = dev->block_size / MMC_BSIZE;
	start_blk = block * num_blk_size;
	read_size = num_blk_size * 1;
	size = dev->new_read(dev, buf, start_blk, read_size);
	if (size != read_size) {
		printf("device read fail.\n");
		goto free;
	}

	for (i = 1; i <= dev->block_size; i++) {
		printf("%02x ", buf[i-1]);
		if (i % 16 == 0) {
			printf("\tOffset %x\n", offset);
			offset += 0x10;
		}
		if (i % 512 == 0)
			printf("\n");
	}
	printf("\n");
	err = 0;

free:
	free(buf);
out:
	bio_close(dev);

	return err;
}

static void print_efiname(struct gpt_part_table *gpt_e, char *buf)
{
	u32 i;
	u8 c;

	for (i = 0; i < PT_NAME_SZ; i++) {
		c = gpt_e->part_name[i] & 0xff;
		c = isprint(c) ? c : '.';
		buf[i] = c;
	}
	buf[PT_NAME_SZ] = 0;
}

int get_unique_guid(char *pt_name, char *buf)
{
	struct gpt_header *gpt_h = NULL;
	struct gpt_part_table *gpt_e = NULL;
	unsigned char *uuid_bin;
	unsigned int boot_dev;
	char name_buf[PT_NAME_SZ + 1];
	int ret = -1;
	int pte_blk_cnt;
	bdev_t *dev;
	u32 num_blk_size;
	u32 start_blk;
	u32 read_size;
	u32 size, i;

	boot_dev = get_boot_device();
	if (boot_dev == BOOT_UFS)
		dev = bio_open("scsi0");
	else
		dev = bio_open("mmc0");
	if (!dev) {
		printf("%s: fail to bio open\n", __func__);
		return -1;
	}

	if (strlen(pt_name) > PT_NAME_SZ) {
		printf("Partition name is too long\n");
		goto out;
	}

	num_blk_size = dev->block_size / MMC_BSIZE;
	gpt_h = malloc(sizeof(struct gpt_header) * num_blk_size);
	if (!gpt_h) {
		printf("%s: malloc failed!\n", __func__);
		goto out;
	}

	gpt_e = malloc(sizeof(struct gpt_part_table) * GPT_ENTRY_NUMBERS);
	if (!gpt_e) {
		printf("%s: malloc failed!\n", __func__);
		goto head_free;
	}

	memset(gpt_h, 0, sizeof(struct gpt_header) * num_blk_size);
	memset(gpt_e, 0, sizeof(struct gpt_part_table) * GPT_ENTRY_NUMBERS);

	start_blk = GPT_HEAD_LBA * num_blk_size;
	read_size = num_blk_size * 1;
	size = dev->new_read(dev, gpt_h, start_blk, read_size);
	if (size != read_size) {
		printf("ERROR: Can't read GPT header\n");
		goto entry_free;
	}

	pte_blk_cnt = ((GPT_ENTRY_NUMBERS * sizeof(struct gpt_part_table) - 1) /
			(dev->block_size + 1)) * num_blk_size;
	start_blk = GPT_TABLE_LBA * num_blk_size;
	read_size = pte_blk_cnt;
	size = dev->new_read(dev, gpt_e, start_blk, read_size);
	if (size != read_size)	{
		printf("ERROR: Can't read GPT primary partition table\n");
		goto entry_free;
	}

	for (i = 0; i < gpt_h->part_num_entry; i++) {
		memset(name_buf, 0, (PT_NAME_SZ + 1));

		print_efiname(&gpt_e[i], name_buf);
		if (!strncmp(name_buf, pt_name, strlen(pt_name))) {
			uuid_bin = (unsigned char *)gpt_e[i].part_guid.b;
			uid_bin_to_str(uuid_bin, buf, UID_STR_GUID);
			ret = 0;
			goto entry_free;
		}
	}

	printf("Not find partition name %s\n", pt_name);

entry_free:
	free(gpt_e);
head_free:
	free(gpt_h);
out:
	bio_close(dev);
	return ret;
}

int gpt_compare_chk(struct pit_info *pit, struct gpt_info *gpt_if)
{
	struct gpt_header *gpt_h = NULL;
	struct gpt_part_table *gpt_e = NULL;

	struct gpt_header *gpts_h = NULL;
	struct gpt_part_table *gpts_e = NULL;

	unsigned int boot_dev;
	unsigned int pte_blk_cnt;
	int ret = 0;
	size_t head_size;
	size_t entry_size;
	u32 size, calc_crc32, start_blk, read_size;
	bdev_t *dev;

	boot_dev = get_boot_device();

	if (boot_dev == BOOT_UFS) {
		dev = bio_open("scsi0");
		num_blk_size = 8;
	} else {
		dev = bio_open("mmc0");
		num_blk_size = 1;
	}

	if (!dev) {
		printf("%s: fail to bio open\n", __func__);
		return -1;
	}

	gpt_h = malloc(sizeof(struct gpt_header) * num_blk_size);
	if (!gpt_h) {
		printf("%s: gpt_h malloc failed!\n", __func__);
		return -1;
	}
	memset(gpt_h, 0, sizeof(struct gpt_header) * num_blk_size);

	gpt_e = malloc(sizeof(struct gpt_part_table) * GPT_ENTRY_NUMBERS);
	if (!gpt_e) {
		printf("%s: gpt_e malloc failed!\n", __func__);
		ret = -1;
		goto gpt_header_free;
	}
	memset(gpt_e, 0, sizeof(struct gpt_part_table) * GPT_ENTRY_NUMBERS);

	ret = set_gpt_header(dev, gpt_h, gpt_e, gpt_if);
	if (ret) {
		printf("Set gpt header fail\n");
		goto gpt_table_free;
	}

	ret = set_partition_table(dev, gpt_h, gpt_e, pit, false);
	if (ret) {
		printf("Set partition table fail\n");
		goto gpt_table_free;
	}

	gpts_h = malloc(sizeof(struct gpt_header) * num_blk_size);
	if (!gpt_h) {
		printf("%s: gpts_h malloc failed!\n", __func__);
		ret = -1;
		goto gpt_table_free;
	}
	memset(gpts_h, 0, sizeof(struct gpt_header) * num_blk_size);

	gpts_e = malloc(sizeof(struct gpt_part_table) * GPT_ENTRY_NUMBERS);
	if (!gpts_e) {
		printf("%s: gpts_e malloc failed!\n", __func__);
		ret = -1;
		goto gpts_header_free;
	}
	memset(gpts_e, 0, sizeof(struct gpt_part_table) * GPT_ENTRY_NUMBERS);

	/* Generate CRC for the primary GPT header */
	calc_crc32 = crc32(0, (const unsigned char *)gpt_e,
		gpt_h->part_num_entry * gpt_h->part_size_entry);
	gpt_h->part_table_crc = calc_crc32;
	calc_crc32 = crc32(0, (const unsigned char *)gpt_h, gpt_h->head_sz);
	gpt_h->head_crc = calc_crc32;

	start_blk = GPT_HEAD_LBA * num_blk_size;
	read_size = num_blk_size * 1;
	size = dev->new_read(dev, gpts_h, start_blk, read_size);
	if (size != read_size) {
		printf("ERROR: Can't read GPT header\n");
		ret = -1;
		goto gpts_table_free;
	}

	pte_blk_cnt = ((GPT_ENTRY_NUMBERS * sizeof(struct gpt_part_table) - 1) /
			(dev->block_size + 1)) * num_blk_size;
	start_blk = GPT_TABLE_LBA * num_blk_size;
	read_size = pte_blk_cnt;
	size = dev->new_read(dev, gpts_e, start_blk, read_size);
	if (size != read_size)	{
		printf("ERROR: Can't read GPT primary partition table\n");
		ret = -1;
		goto gpts_table_free;
	}

	head_size = sizeof(struct gpt_header);
	entry_size = sizeof(struct gpt_part_table) * GPT_ENTRY_NUMBERS;

	if (memcmp(gpt_h, gpts_h, head_size)) {
		printf("ERROR: Does not match gpt header!!\n");
		ret = -1;
		goto gpts_table_free;
	}

	if (memcmp(gpt_e, gpts_e, entry_size)) {
		printf("ERROR: Does not match gpt entry_table!!\n");
		ret = -1;
		goto gpts_table_free;
	}

	/* Recalculate the values for the backup GPT header */
	gpt_h->gpt_header = gpt_h->gpt_back_header;
	gpt_h->gpt_back_header = GPT_HEAD_LBA;
	calc_crc32 = crc32(0, (const unsigned char *)gpt_h, gpt_h->head_sz);
	gpt_h->head_crc = calc_crc32;

	start_blk = gpt_h->gpt_header * num_blk_size;
	read_size = num_blk_size * 1;
	size = dev->new_read(dev, gpts_h, start_blk, read_size);
	if (size != read_size) {
		printf("ERROR: Can't read BACKUP GPT header\n");
		ret = -1;
		goto gpts_table_free;
	}

	pte_blk_cnt = ((GPT_ENTRY_NUMBERS * sizeof(struct gpt_part_table) - 1) /
			(dev->block_size + 1)) * num_blk_size;
	start_blk = (gpt_h->end_lba + 1) * num_blk_size;;
	read_size = pte_blk_cnt;
	size = dev->new_read(dev, gpts_e, start_blk, read_size);
	if (size != read_size)	{
		printf("ERROR: Can't read GPT primary partition table\n");
		ret = -1;
		goto gpts_table_free;
	}

	if (memcmp(gpt_h, gpts_h, head_size)) {
		printf("ERROR: Does not match backup gpt header!!\n");
		ret = -1;
		goto gpts_table_free;
	}

	if (memcmp(gpt_e, gpts_e, entry_size)) {
		printf("ERROR: Does not match backup gpt entry_table!!\n");
		ret = -1;
		goto gpts_table_free;
	}

	printf("gpt compare check complete!!\n");

gpts_table_free:
	free(gpts_e);
gpts_header_free:
	free(gpts_h);
gpt_table_free:
	free(gpt_e);
gpt_header_free:
	free(gpt_h);

	return ret;
}

STATIC_COMMAND_START
STATIC_COMMAND("gpt_dump", "start block", &gpt_dump)
STATIC_COMMAND_END(gpt_test);
