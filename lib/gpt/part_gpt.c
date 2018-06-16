#include <reg.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <lib/console.h>
#include <lib/cksum.h>
#include <part_gpt.h>
#include <uuid.h>

static int set_protective_mbr(bdev_t *dev)
{
	struct p_mbr_head *p_mbr = NULL;
	int err;

	p_mbr = malloc(sizeof(struct p_mbr_head) * 8);
	if (!p_mbr){
		printf("p_mbr alloc failed\n");
		return -1;
	}
	memset(p_mbr, 0, sizeof(*p_mbr) * 8);

	if (p_mbr == NULL) {
		printf("p_mbr_calloc failed\n");
		return -1;
	}

	p_mbr->signature = MSDOS_MBR_SIGNATURE;
	p_mbr->partition_type = EFI_PMBR_OSTYPE_EFI_GPT;
	p_mbr->start_sect_cnt = 1;
	p_mbr->nr_sect = (dev->block_count + 1) * 8 - 1; /* block count */

	/* Write P MBR BLOCK */
	err = dev->new_write(dev, p_mbr, 0, 8);
	if(err != 8) {
		printf("write failed\n");
		return -1;
	}
	printf("P_MBR write done\n");
	return 0;
}

static int set_gpt_header(bdev_t *dev, struct gpt_header *gpt_h,
		struct gpt_part_table *gpt_e)
{
	char *str_disk_guid;

	gpt_h->signature = GPT_HEADER_SIGNATURE;
	gpt_h->revision = GPT_HEADER_REVISION_V1;
	gpt_h->head_sz = HEAD_SIZE;
	gpt_h->gpt_header = 1;
	gpt_h->gpt_back_header = dev->block_count;
	gpt_h->start_lba = 6;    		//UFS 1LBA is 4KB
	gpt_h->end_lba = dev->block_count - 4;
	gpt_h->part_table_lba = 2;
	gpt_h->part_num_entry = 90;
	gpt_h->part_size_entry = TABLE_SIZE;
	gpt_h->part_size_entry = sizeof(struct gpt_part_table);
	gpt_h->head_crc = 0;
	gpt_h->part_table_crc = 0;

	str_disk_guid = strdup(UUID_DISK);
	if (uuid_str_to_bin(str_disk_guid, gpt_h->guid, UUID_STR_FORMAT_GUID))
		return -1;

	return 0;
}
#if USE_PIT
static int set_partition_table(bdev_t *dev,
		struct gpt_header *gpt_h,
		struct gpt_part_table *gpt_e,
		struct pit_info *pit)
#else
static int set_partition_table(bdev_t *dev,
		struct gpt_header *gpt_h,
		struct gpt_part_table *gpt_e)
#endif
{
	uint32_t offset = gpt_h->start_lba;
	uint32_t start;
	uint32_t end_use_lba = gpt_h->end_lba;
	int i, k;
	int j;
	size_t efiname_len, dosname_len;
	char fat[72] = "fat";
	char fat1[72] = "system";
	char fat2[72] = "userdata";
	char fat3[72] = "cache";
	char fat4[72] = "modem";
	char fat5[72] = "efs";
	char fat6[72] = "persist";
	char fat7[72] = "vendor";
	char *str_disk_guid;

	printf("Set up start for Partition Table\n");
	printf("The default size is 512Byte\n");
	printf("==============================================================\n");
	start = offset;
	offset = start + 51200; /* 200MB */
	printf("Start LBA : %d\t\t Size : %d\t\t partition name : FAT\n", start * 8, (offset - start) * 8);

	/* FAT Partition */
	gpt_e[0].part_start_lba = start;
	gpt_e[0].part_end_lba = offset - 1;

	str_disk_guid = strdup(BASIC_DATA);
	if (uuid_str_to_bin(str_disk_guid, gpt_e[0].part_guid.b, UUID_STR_FORMAT_STD))
		return -1;
	memcpy(gpt_e[0].part_type.b,&PARTITION_BASIC_DATA_GUID, 16);
	efiname_len = sizeof(gpt_e[i].part_name)
		/ sizeof(u16);
	dosname_len = sizeof(fat);

	memset(&gpt_e[0].part_name, 0, sizeof(gpt_e[0].part_name));

	for (k = 0; k < MIN(dosname_len, efiname_len); k++)
		gpt_e[0].part_name[k] = (u16)fat[k];

#if USE_PIT
	for(i = 0, j = 0; i < pit->count; i++) {	/* PIT NUM COUNT */

		/* partition start lba */
		if (pit->pte[i].filesys != 0) {
			start = offset;
			if (start && (start < offset)) {
				printf("parttiotn overlap!\n");
				return -1;
			}
			gpt_e[j+1].part_start_lba = start;
			offset = start + (pit->pte[i].blknum / 8);
			if(offset >= end_use_lba) {
				printf("partition layout over disk size\n");
				return -1;
			}
			gpt_e[j+1].part_end_lba = offset - 1;

			printf("Start LBA : %d\t Size : %d\t\t partition name : ", start * 8, (offset - start) * 8);

			str_disk_guid = strdup(FILE_SYSTEM_DATA);
			/* UUID */
			if (uuid_str_to_bin(str_disk_guid, gpt_e[j+1].part_guid.b, UUID_STR_FORMAT_STD))
				return -1;

			/* GUID */
			memcpy(gpt_e[j+1].part_type.b, &PARTITION_BASIC_DATA_GUID, 16);

			efiname_len = sizeof(gpt_e[j].part_name)
				/ sizeof(u16);
			dosname_len = sizeof(pit->pte[i].name);

			memset(gpt_e[j+1].part_name, 0,
					sizeof(gpt_e[j+1].part_name));
			for (k = 0; k < MIN(dosname_len, efiname_len); k++) {
				gpt_e[j+1].part_name[k] =
					(u16)(pit->pte[i].name[k]);
				printf("%c", gpt_e[j+1].part_name[k]);
			}
			printf("\n");
			j++;
		}
	}
#else
	/* TEST SYSTEM */
	start = offset;
	offset = start + 768000;
	printf("Start LBA : %d\t Size : %d\t\t partition name : System\n", start * 8, (offset - start) * 8);

	gpt_e[1].part_start_lba = start;
	gpt_e[1].part_end_lba = offset - 1;

	str_disk_guid = strdup(FILE_SYSTEM_DATA);

	if (uuid_str_to_bin(str_disk_guid, gpt_e[1].part_guid.b, UUID_STR_FORMAT_STD))
		return -1;

	memcpy(gpt_e[1].part_type.b, &PARTITION_BASIC_DATA_GUID, 16);
	efiname_len = sizeof(gpt_e[1].part_name)
		/ sizeof(u16);
	dosname_len = sizeof(fat1);

	memset(&gpt_e[1].part_name, 0, sizeof(gpt_e[1].part_name));

	for (k = 0; k < MIN(dosname_len, efiname_len); k++)
		gpt_e[1].part_name[k] = (u16)fat1[k];

	/* TEST USER */
	start = offset;
	offset = start + 14532083;

	printf("Start LBA : %d\t Size : %d\t partition name : Userdata\n", start * 8, (offset - start) * 8);
	gpt_e[2].part_start_lba = start;
	gpt_e[2].part_end_lba = offset - 1;

	str_disk_guid = strdup(FILE_SYSTEM_DATA);
	if (uuid_str_to_bin(str_disk_guid, gpt_e[2].part_guid.b, UUID_STR_FORMAT_STD))
		return -1;

	memcpy(gpt_e[2].part_type.b, &PARTITION_BASIC_DATA_GUID, 16);
	efiname_len = sizeof(gpt_e[2].part_name)
		/ sizeof(u16);
	dosname_len = sizeof(fat2);

	memset(&gpt_e[2].part_name, 0, sizeof(gpt_e[2].part_name));

	for (k = 0; k < MIN(dosname_len, efiname_len); k++)
		gpt_e[2].part_name[k] = (u16)fat2[k];

	/* TEST CACHE */
	start = offset;
	offset = start + 76800;
	printf("Start LBA : %d\t Size : %d\t\t partition name : Cache\n", start * 8, (offset - start) * 8);
	gpt_e[3].part_start_lba = start;
	gpt_e[3].part_end_lba = offset - 1;

	str_disk_guid = strdup(FILE_SYSTEM_DATA);
	if (uuid_str_to_bin(str_disk_guid, gpt_e[3].part_guid.b, UUID_STR_FORMAT_STD))
		return -1;

	memcpy(gpt_e[3].part_type.b, &PARTITION_BASIC_DATA_GUID, 16);
	efiname_len = sizeof(gpt_e[3].part_name)
		/ sizeof(u16);
	dosname_len = sizeof(fat3);

	memset(&gpt_e[3].part_name, 0, sizeof(gpt_e[3].part_name));

	for (k = 0; k < MIN(dosname_len, efiname_len); k++)
		gpt_e[3].part_name[k] = (u16)fat3[k];

	/* Modem TEST  */
	start = offset;
	offset = start + 25600;

	printf("Start LBA : %d\t Size : %d\t\t partition name : Modem\n", start * 8, (offset - start) * 8);
	gpt_e[4].part_start_lba = start;
	gpt_e[4].part_end_lba = offset - 1;

	str_disk_guid = strdup(FILE_SYSTEM_DATA);
	if (uuid_str_to_bin(str_disk_guid, gpt_e[4].part_guid.b, UUID_STR_FORMAT_STD))
		return -1;

	memcpy(gpt_e[4].part_type.b, &PARTITION_BASIC_DATA_GUID, 16);
	efiname_len = sizeof(gpt_e[4].part_name)
		/ sizeof(u16);
	dosname_len = sizeof(fat4);

	memset(&gpt_e[4].part_name, 0, sizeof(gpt_e[4].part_name));

	for (k = 0; k < MIN(dosname_len, efiname_len); k++)
		gpt_e[4].part_name[k] = (u16)fat4[k];

	/* TEST EFS */
	start = offset;
	offset = start + 5120;

	printf("Start LBA : %d\t Size : %d\t\t partition name : efs\n", start * 8, (offset - start) * 8);
	gpt_e[5].part_start_lba = start;
	gpt_e[5].part_end_lba = offset - 1;

	str_disk_guid = strdup(FILE_SYSTEM_DATA);
	if (uuid_str_to_bin(str_disk_guid, gpt_e[5].part_guid.b, UUID_STR_FORMAT_STD))
		return -1;

	memcpy(gpt_e[5].part_type.b, &PARTITION_BASIC_DATA_GUID, 16);
	efiname_len = sizeof(gpt_e[5].part_name)
		/ sizeof(u16);
	dosname_len = sizeof(fat5);

	memset(&gpt_e[5].part_name, 0, sizeof(gpt_e[5].part_name));

	for (k = 0; k < MIN(dosname_len, efiname_len); k++)
		gpt_e[5].part_name[k] = (u16)fat5[k];

	/* TEST PERSIST */
	start = offset;
	offset = start + 7680;

	printf("Start LBA : %d\t Size : %d\t\t partition name : Persist\n", start * 8, (offset - start) * 8);
	gpt_e[6].part_start_lba = start;
	gpt_e[6].part_end_lba = offset - 1;

	str_disk_guid = strdup(FILE_SYSTEM_DATA);
	if (uuid_str_to_bin(str_disk_guid, gpt_e[6].part_guid.b, UUID_STR_FORMAT_STD))
		return -1;

	memcpy(gpt_e[6].part_type.b, &PARTITION_BASIC_DATA_GUID, 16);
	efiname_len = sizeof(gpt_e[6].part_name)
		/ sizeof(u16);
	dosname_len = sizeof(fat6);

	memset(&gpt_e[6].part_name, 0, sizeof(gpt_e[6].part_name));

	for (k = 0; k < MIN(dosname_len, efiname_len); k++)
		gpt_e[6].part_name[k] = (u16)fat6[k];

	/* TEST VENDOR */
	start = offset;
	offset = start + 153600;

	printf("Start LBA : %d\t Size : %d\t\t partition name : Vendor\n", start * 8, (offset - start) * 8);

	gpt_e[7].part_start_lba = start;
	gpt_e[7].part_end_lba = offset - 1;

	str_disk_guid = strdup(FILE_SYSTEM_DATA);
	if (uuid_str_to_bin(str_disk_guid, gpt_e[7].part_guid.b, UUID_STR_FORMAT_STD))
		return -1;

	memcpy(gpt_e[7].part_type.b, &PARTITION_BASIC_DATA_GUID, 16);
	efiname_len = sizeof(gpt_e[7].part_name)
		/ sizeof(u16);
	dosname_len = sizeof(fat7);

	memset(&gpt_e[7].part_name, 0, sizeof(gpt_e[7].part_name));

	for (k = 0; k < MIN(dosname_len, efiname_len); k++)
		gpt_e[7].part_name[k] = (u16)fat7[k];

	printf("==============================================================\n");
	printf("Part table complete\n");
#endif
	return 0;
}

static int write_gpt_table(bdev_t *dev, struct gpt_header *gpt_h,
		struct gpt_part_table *gpt_e)
{
	int pte_blk_cnt;
	u32 calc_crc32;
	u64 val;
	int err;

	/* Setup the Protective MBR */
	err = set_protective_mbr(dev);
	if (err)
		goto err;
	pte_blk_cnt = ((GPT_ENTRY_NUMBERS * sizeof(struct gpt_part_table) -1 ) /
			(dev->block_size + 1)) * 8;

	/* Generate CRC for the Primary GPT Header */
	calc_crc32 = crc32(0, (const unsigned char *)gpt_e,
		gpt_h->part_num_entry * gpt_h->part_size_entry);

	gpt_h->part_table_crc = calc_crc32;

	calc_crc32 = crc32(0, (const unsigned char *)gpt_h, gpt_h->head_sz);
	gpt_h->head_crc = calc_crc32;

	/* Write the First GPT to the block right after the Legacy MBR */

	err = dev->new_write(dev, gpt_h , 8, 8);
	if (err != 8) {
		printf("GPT HEAD write fail\n");
		goto err;
	}

	err = dev->new_write(dev, gpt_e, 16, pte_blk_cnt);
	if (err != pte_blk_cnt) {
		printf("Partition Table write Fail\n");
		goto err;
	}

	/* recalculate the values for the Backup GPT Header */
	val = 1;
	gpt_h->gpt_header = gpt_h->gpt_back_header;
	gpt_h->gpt_back_header = val;
	gpt_h->head_crc = 0;

	calc_crc32 = crc32(0, (const unsigned char *)gpt_h, gpt_h->head_sz);
	gpt_h->head_crc = calc_crc32;

	err = dev->new_write(dev, gpt_e, (gpt_h->end_lba + 1) * 8, pte_blk_cnt);
	if (err != pte_blk_cnt) {
		printf("Back up partition table write Fail\n");
		goto err;
	}

	err = dev->new_write(dev, gpt_h, gpt_h->gpt_header * 8, 8);

	if (err != 8) {
		printf("Back up Gpt Head write fail\n");
		goto err;
	}

	printf("GPT successfully written to block device!\n");
	return 0;
err:
	printf("** Can't write to device **\n");
	return -1;
}
#if USE_PIT
int gpt_create(struct pit_info *pit)
#else
int gpt_create(void)
#endif
{
	bdev_t *dev;
	dev = bio_open("scsi0");
	struct gpt_header *gpt_h = NULL;
	struct gpt_part_table *gpt_e = NULL;
	int err = 0;

	gpt_h = malloc(sizeof(struct gpt_header) * 8);

	if (gpt_h == NULL) {
		printf("%s: malloc failed!\n", __func__);
		return -1;
	}
	memset(gpt_h, 0, sizeof(*gpt_h) * 8);

	gpt_e = malloc(sizeof(struct gpt_part_table) * 128);

	if (gpt_e == NULL) {
		printf("%s: malloc failed!\n", __func__);
		free(gpt_h);
		return -1;
	}
	memset(gpt_e, 0, 4096 * 3);
	err = set_gpt_header(dev, gpt_h, gpt_e);
#if USE_PIT
	err = set_partition_table(dev, gpt_h, gpt_e, pit);
#else
	err = set_partition_table(dev, gpt_h, gpt_e);
#endif
	err = write_gpt_table(dev, gpt_h, gpt_e);

	free(gpt_h);
	free(gpt_e);

	bio_close(dev);
	return err;
}

static int gpt_test(int argc, const cmd_args *argv)
{
	int err;
#if USE_PIT
	printf("Use PIT plz!, not create manual gpt.\n");
#else
	printf("COMMAND GPT TEST \n");
	err = gpt_create();
	if(err) {
		printf("Create GPT Fail\n");
		return -1;
	}
#endif
	return 0;
}

static int gpt_dump(int argc, const cmd_args *argv)
{
	bdev_t *dev;
	int rv;
	unsigned char buf[4096] = {0,};
	int i, block;
	unsigned int offset = 0;

	printf("partition Read\n");

	dev = bio_open("scsi0");
	block = argv[1].u;

	printf("start block %d \n", block);

	rv = dev->new_read(dev, buf, block * 8, 8);

	if(rv != 8)
	{
		printf("scsi device read fail.\n");
		return -1;
	}
	for (i = 1; i <= 4096; i++) {
		printf("%02x ", buf[i-1]);
		if (i % 16 == 0) {
			printf("\tOffset %x \n",offset);
			offset += 0x10;
		}
		if (i % 512 == 0)
			printf("\n");
	}
	printf("\n");

	bio_close(dev);
	return 0;
}

STATIC_COMMAND_START
STATIC_COMMAND("gpt_test", "gpt_test", &gpt_test)
STATIC_COMMAND("gpt_dump", "start block", &gpt_dump)
STATIC_COMMAND_END(gpt_test);
