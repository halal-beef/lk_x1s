/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <string.h>
#include <lib/bio.h>
#include <dev/boot.h>
#include <platform/sfr.h>
#include <platform/decompress_ext4.h>

#define SECTOR_BITS		9	/* 512B */

#define ext4_printf(args, ...)

static int write_raw_chunk(char* data, unsigned int sector, unsigned int sector_size);


int check_compress_ext4(char *img_base, unsigned long long parti_size) {
	ext4_file_header *file_header;

	file_header = (ext4_file_header*)img_base;

	if (file_header->magic != EXT4_FILE_HEADER_MAGIC) {
		return -1;
	}

	if (file_header->major != EXT4_FILE_HEADER_MAJOR) {
		printf("Invalid Version Info! 0x%2x\n", file_header->major);
		return -1;
	}

	if (file_header->file_header_size != EXT4_FILE_HEADER_SIZE) {
		printf("Invalid File Header Size! 0x%8x\n",
								file_header->file_header_size);
		return -1;
	}

	if (file_header->chunk_header_size != EXT4_CHUNK_HEADER_SIZE) {
		printf("Invalid Chunk Header Size! 0x%8x\n",
								file_header->chunk_header_size);
		return -1;
	}

	if (file_header->block_size != EXT4_FILE_BLOCK_SIZE) {
		printf("Invalid Block Size! 0x%8x\n", file_header->block_size);
		return -1;
	}

	if ((parti_size/file_header->block_size)  < file_header->total_blocks) {
		printf("Invalid Volume Size! Image is bigger than partition size!\n");
		printf("partion size %lld , image size %d \n",
			(parti_size/file_header->block_size), file_header->total_blocks);
		while(1);
	}

	/* image is compressed ext4 */
	return 0;
}

int write_raw_chunk(char* data, unsigned int sector, unsigned int sector_size) {
	unsigned char *tmp_align;
	bdev_t *dev;
	unsigned int boot_dev;
	char *str;
	unsigned int blks;
	int ret;

	boot_dev = get_boot_device();
	if (boot_dev == BOOT_UFS)
		str = "scsi0";
	else
		;

	dev = bio_open(str);

	if (((unsigned long)data % 8) != 0) {
		tmp_align = (char *)CFG_FASTBOOT_MMC_BUFFER;
		memcpy((unsigned char *)tmp_align, (unsigned char *)data, sector_size * 512);
		data = tmp_align;
	}

	ext4_printf("write raw data in %d size %d \n", sector, sector_size);

	blks = dev->new_write(dev, data, sector, sector_size);

	bio_close(dev);

	if (blks != sector_size) {
		printf("Writing raw data on %d size %d failed.\n", sector, sector_size);
		ret = -1;
	} else
		ret = 0;

	return ret;
}

int write_compressed_ext4(char* img_base, unsigned int sector_base) {
	unsigned int sector_size;
	int total_chunks;
	ext4_chunk_header *chunk_header;
	ext4_file_header *file_header;

	file_header = (ext4_file_header*)img_base;
	total_chunks = file_header->total_chunks;

	ext4_printf("total chunk = %d \n", total_chunks);

	img_base += EXT4_FILE_HEADER_SIZE;

	while(total_chunks) {
		chunk_header = (ext4_chunk_header*)img_base;
		sector_size = (chunk_header->chunk_size * file_header->block_size) >> SECTOR_BITS;

		switch(chunk_header->type)
		{
		case EXT4_CHUNK_TYPE_RAW:
			ext4_printf("raw_chunk \n");
			write_raw_chunk(img_base + EXT4_CHUNK_HEADER_SIZE,
							sector_base, sector_size);
			sector_base += sector_size;
			break;

		case EXT4_CHUNK_TYPE_FILL:
			printf("*** fill_chunk ***\n");
			sector_base += sector_size;
			break;

		case EXT4_CHUNK_TYPE_NONE:
			ext4_printf("none chunk \n");
			sector_base += sector_size;
			break;

		default:
			printf("*** unknown chunk type ***\n");
			sector_base += sector_size;
			break;
		}
		total_chunks--;
		ext4_printf("remain chunks = %d \n", total_chunks);

		img_base += chunk_header->total_size;
	};

	ext4_printf("write done \n");
	return 0;
}
