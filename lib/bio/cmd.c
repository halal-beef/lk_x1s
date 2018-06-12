/*
 * (C) Copyright 2018 SAMSUNG Electronics
 * Kiwoong Kim <kwmad.kim@samsung.com>
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 *
 */

#include <lib/bio.h>
#include <string.h>

#if defined(WITH_LIB_CONSOLE)

#include <lib/console.h>

static int cmd_blk(int argc, const cmd_args *argv);

STATIC_COMMAND_START
STATIC_COMMAND("blk", "block io commands", &cmd_blk)
STATIC_COMMAND_END(blk);

/*
 * Now, we would consider all the IO context's unit as 512 bytes.
 */
static int cmd_blk(int argc, const cmd_args *argv)
{
	int rc = 0;
	addr_t addr;
	bnum_t lba;
	uint cnt;
	uint cnt_ret;

	if (argc < 2) {
notenoughargs:
		printf("not enough arguments:\n");
usage:
		printf("%s read <dev> <buf> <lba> <blks>\n", argv[0].str);
		printf("%s write <dev> <buf> <lba> <blks>\n", argv[0].str);
		printf("%s erase <dev> <buf> <lba> <blks>\n", argv[0].str);
		printf("\n");
		printf("LBA and count here assumes an 512 bytes of unit.\n");
		printf("\n");
		printf("Example: read 4KB data to address 0x80000000 at LBA 0x100 of user partition of a UFS device\n");
		printf("%s read scsi0 0x80000000 0x100 0x8\n", argv[0].str);
		printf("Example: write 512B data from address 0x80000000 at LBA 0x200 of the first boot partition of a eMMC device\n");
		printf("%s write mmc1 0x80000000 0x200 0x1\n", argv[0].str);
		printf("Example: erase 1MB data at LBA 0x8000 of the user partition of a UFS device\n");
		printf("%s erase scsi0 0x8000 0x800\n", argv[0].str);
		return -1;
	}

	addr = argv[3].u;
	lba = argv[4].u;
	cnt = argv[5].u;

	if (!strcmp(argv[1].str, "read")) {
		if (argc < 6) goto notenoughargs;

		bdev_t *dev = bio_open(argv[2].str);
		if (!dev) {
			printf("error opening block device\n");
			return -1;
		}

		cnt_ret = dev->new_read(dev, (void *)addr, lba, cnt);
		printf("read done  %u!!\n", cnt_ret);

		bio_close(dev);

		rc = (cnt == cnt_ret) ? 0 : -1;
	} else if (!strcmp(argv[1].str, "write")) {
		if (argc < 6) goto notenoughargs;

		bdev_t *dev = bio_open(argv[2].str);
		if (!dev) {
			printf("error opening block device\n");
			return -1;
		}

		cnt_ret = dev->new_write(dev, (void *)addr, lba, cnt);
		printf("write done %u!!\n", cnt_ret);

		bio_close(dev);

		rc = (cnt == cnt_ret) ? 0 : -1;
	} else if (!strcmp(argv[1].str, "erase")) {
		if (argc < 5) goto notenoughargs;

		bdev_t *dev = bio_open(argv[2].str);
		if (!dev) {
			printf("error opening block device\n");
			return -1;
		}

		cnt_ret = dev->new_erase(dev, lba, cnt);
		printf("erase done %u!!\n", cnt_ret);

		bio_close(dev);

		rc = (cnt == cnt_ret) ? 0 : -1;
	} else {
		printf("unrecognized subcommand\n");
		goto usage;
	}

	return rc;
}

#endif
