/*
 * opyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
#include <config.h>
#include <dev/ufs.h>
#include <dev/boot.h>
#include <string.h>
#include <lib/block.h>
#include <lib/bio.h>
#include <dev/scsi.h>
#include <dev/mmc.h>

#define CMD_STRING_MAX_SIZE     60
#define BLKCNT_UFS_TO_MMC(x) (x << 3)

void blk_init(int bootdevice)
{
	if (get_boot_device() == BOOT_EMMC) {
		printf("MMC will be here\n");
		return;
	} else if (get_boot_device() == BOOT_UFS) {
		printf("UFS init\n");
		ufs_alloc_memory();
		ufs_init(2);

		return;
	}

	printf("Not supported boot device\n");
}

bdev_t *blk_get_dev(unsigned int lun)
{
	char str[CMD_STRING_MAX_SIZE];
	bdev_t *dev;
	int bMode = get_boot_device();

	if (bMode == BOOT_UFS) {
		memcpy(str, "scsi", 4);
		str[4] = '0' + lun;
		str[5] = '\0';
	} else if (bMode == BOOT_EMMC) {
		if (lun == MMC_PARTITION_MMC_RPMB) {
			memcpy(str, "mmcrpmb", 7);
			str[7] = '\0';
		} else {
			memcpy(str, "mmc", 3);
			str[3] = '0' + lun;
			str[4] = '\0';
		}
	}

	dev = bio_open(str);
	if (!dev) {
		printf("error opening block device\n");
	}

	return dev;
}

u32 blk_card_total_sector(unsigned int lun)
{
	int ret = 0;
	bdev_t *dev;
	int bMode = get_boot_device();

	dev = blk_get_dev(lun);
	ret = dev->block_count;

	if (bMode == BOOT_UFS)
		ret = BLKCNT_UFS_TO_MMC(ret);

	printf("%s: LU:%d total sector is 0x%x\n", __func__, lun, ret);
	return ret;
}

int blk_bread(u8 *p, u32 lun, u32 from, u32 size)
{
	ssize_t cnt;
	bdev_t *dev;

	dev = blk_get_dev(lun);
	cnt = dev->new_byte_read(dev, (void *)p, from, size);

	return cnt;
}

int blk_bwrite(u8 *p, u32 lun, u32 from, u32 size)
{
	ssize_t cnt;
	bdev_t *dev;

	dev = blk_get_dev(lun);
	cnt = dev->new_byte_write(dev, (void *)p, from, size);

	return cnt;
}

int blk_berase(u32 lun, u32 from, u32 size)
{
	ssize_t cnt;
	bdev_t *dev;

	dev = blk_get_dev(lun);
	cnt = dev->new_byte_erase(dev, from, size);

	return cnt;
}

int blk_set_boot_wp(int enable)
{
	int ret = 0;
	int bMode = get_boot_device();
	bdev_t *dev;

	if (bMode == BOOT_UFS) {
#if defined(CONFIG_BOOT_PARTITION_WP_SW)
		do_swp_lock();
#endif
	} else {
		printf("%s: MMC mode\n", __func__);
		dev = blk_get_dev(0);
		dev->new_set_wp(dev, 1, enable, 0, 0);
	}

	return ret;
}

int blk_set_user_wp(u32 start, u32 size)
{
	int ret = 0;
	int bMode = get_boot_device();
	bdev_t *dev;

	if (bMode == BOOT_UFS) {
		printf("%s:NONE\n", __func__);
	} else {
		printf("%s: MMC mode\n", __func__);
		dev = blk_get_dev(0);
		dev->new_set_wp(dev, 0, 1, start, size);
	}

	return ret;
}

int blk_close(void)
{
	int ret = 0;
	bdev_t *dev;

	dev = blk_get_dev(0);

	if (dev)
		bio_close(dev);

	return ret;
}
