/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <err.h>
#include <platform/sfr.h>
#include <platform/bl_sys_info.h>
#include <platform/dram_training.h>
#include <dev/boot.h>
#include <lib/bio.h>

static unsigned int crc(char *buffer, int length)
{
	unsigned int crc = 0;
	int j, lower, upper, temp_value;

	for (j = 0; j < length; ++j) {
		lower = j % 8;
		upper = 8 - lower;
		temp_value = buffer[j];

		crc ^= ((temp_value << upper) | (temp_value >> lower));
	}

	return crc;
}

void write_dram_training_data(void)
{
	printf("\nChecking DRAM training data\n");
	printf("ACPM magic code: 0x%X\n", *(unsigned int *)DRAM_TRAINING_ACPM_MAGIC_CODE_BASE);
	printf("Write magic code: 0x%X\n", *(unsigned int *)DRAM_TRAINING_WRITE_MAGIC_CODE_BASE);
	if ((*(unsigned int *)DRAM_TRAINING_ACPM_MAGIC_CODE_BASE == DRAM_TRAINING_ACPM_MAGIC_CODE)
		&& (*(unsigned int *)DRAM_TRAINING_WRITE_MAGIC_CODE_BASE != DRAM_TRAINING_WRITE_MAGIC_CODE)) {
		unsigned int boot_dev;
		unsigned int blks;
		bdev_t *dev;

		boot_dev = get_boot_device();
		if (boot_dev == BOOT_UFS) {
			dev = bio_open("scsi0");
		} else if (boot_dev == BOOT_EMMC) {
			dev = bio_open("mmc1");
		} else {
			printf("write_dram_training_data: 'block dev' is not setted.\n");
			return;
		}

		*(unsigned int *)DRAM_TRAINING_WRITE_MAGIC_CODE_BASE = DRAM_TRAINING_WRITE_MAGIC_CODE;
		*(unsigned int *)DRAM_TRAINING_CRC_CODE_BASE = crc((char *)DRAM_TRAINING_AREA_BASE, DRAM_TRAINING_CRC_AREA_SIZE);

		/* write DRAM training data to first booting device */
		if (dev) {
			printf("Writing DRAM training data on %d\n", CONFIG_DRAM_TRAINING_AREA_BLOCK_OFFSET);

			blks = dev->new_write(dev, (void *)DRAM_TRAINING_AREA_BASE, CONFIG_DRAM_TRAINING_AREA_BLOCK_OFFSET,
							DRAM_TRAINING_AREA_SIZE / USER_BLOCK_SIZE);

			if (blks != DRAM_TRAINING_AREA_SIZE / USER_BLOCK_SIZE)
				printf("DRAM training data writing failed!\n");

			bio_close(dev);
		}
	}
	printf("\n");
}
