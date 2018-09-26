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

#include <dev/scsi.h>
#include <lib/font_display.h>

#define	SCSI_UNMAP_DESC_LEN	16

/* Only one block descriptor is used for simple software control. */
#define	SCSI_UNMAP_DESC_NUM	1
#define	SCSI_UNMAP_BLOCK_DESC_DATA_LEN	(SCSI_UNMAP_DESC_LEN * \
						SCSI_UNMAP_DESC_NUM)
#define	SCSI_UNMAP_DATA_LEN	(SCSI_UNMAP_BLOCK_DESC_DATA_LEN	+ 6)

/*
 * RPMB Message Data Frame size
 *
 * Now we assumed that its case is only UFS, so the size is 512 bytes
 */
#define RPMB_MSG_DATA_SIZE	512

/*
 * Argument 'p' should be char pointer
 */
#define	set_dword_le(p, v)	\
	do {						\
		*(p) = (u8)((v) >> 24) & 0xff;		\
		*(p+1) = (u8)((v) >> 16) & 0xff;	\
		*(p+2) = (u8)((v) >> 8) & 0xff;		\
		*(p+3) = (u8)(v) & 0xff;		\
	} while (0)

#define	get_dword_le(p)		((*(p) << 24) |		\
				(*(p+1) << 16) |	\
				(*(p+2) << 8) |		\
				(*(p+3)))

#define	set_word_le(p, v)	\
	do {						\
		*(p) = (u8)((v) >> 8) & 0xff;		\
		*(p+1) = (u8)(v) & 0xff;		\
	} while (0)

/* Command meta, only one when not using multi-tasking */
scm g_scm;

/* Command meta, only one when not using multi-tasking */
u8 g_buf[4096];

/* Function declaration */
static status_t scsi_format_unit(struct bdev *dev);
status_t scsi_start_stop_unit(struct bdev *dev);

/* UFS user command definition */
#if defined(WITH_LIB_CONSOLE)

#include <lib/console.h>

static int cmd_scsi(int argc, const cmd_args *argv)
{
	int rc = 0;
	bdev_t *dev;
	status_t ret;

	if (argc < 2) {
notenoughargs:
		printf("not enough arguments:\n");
usage:
		return -1;
	}

	if (!strcmp(argv[1].str, "format")) {
		if (argc > 3) goto notenoughargs;

		dev = bio_open(argv[2].str);
		if (!dev) {
			printf("error opening block device\n");
			return -1;
		}

		ret = scsi_format_unit(dev);
		rc = (ret == NO_ERROR) ? 0 : -1;

		bio_close(dev);
	} else if (!strcmp(argv[1].str, "ssu")) {
		if (argc > 3) goto notenoughargs;

		ret = scsi_do_ssu();
		rc = (ret == NO_ERROR) ? 0 : -1;
	} else {
		printf("unrecognized subcommand\n");
		goto usage;
	}

	return rc;
}

STATIC_COMMAND_START
STATIC_COMMAND("scsi", "SCSI commands", &cmd_scsi)
STATIC_COMMAND_END(scsi);

#endif

static status_t scsi_parse_status(u8 status)
{
	status_t ret = ERR_ACCESS_DENIED;

	switch (status) {
	case 0:
		ret = NO_ERROR;
		break;
	case 2:
		printf("CHECK CONDITION\n");
		break;
	case 4:
		printf("CONDITION MET\n");
		break;
	case 8:
		printf("BUSY\n");
		break;
	case 0x18:
		printf("RESERVATION CONFLICT\n");
		break;
	case 0x28:
		printf("TASK SET FULL\n");
		break;
	case 0x30:
		printf("ACA ACTIVE\n");
		break;
	case 0x40:
		printf("TASK ABORTED\n");
		break;
	default:
		printf("reserved\n");
		break;
	}

	return ret;
}

static status_t scsi_read_10(struct bdev *dev, void *buf, bnum_t block, uint count)
{
	scsi_device_t *sdev = (scsi_device_t *)dev->private;
	status_t ret = NO_ERROR;

	g_scm.sdev = sdev;
	g_scm.buf = (u8 *)buf;
	g_scm.datalen = (u32)count * dev->block_size;

	/*
	 * Prepare CDB
	 *
	 * RDPROTECT is always zero here for UFS
	 */
	memset((void *)g_scm.cdb, 0, sizeof(g_scm.cdb));
	g_scm.cdb[0] = SCSI_OP_READ_10;
	set_dword_le(&g_scm.cdb[2], (u32)block);
	set_word_le(&g_scm.cdb[7], (u16)count);

	/* Actual issue */
	ret = sdev->exec(&g_scm);
	if (!ret)
		ret = scsi_parse_status(g_scm.status);

	return ret;
}

static status_t scsi_write_10(struct bdev *dev, const void *buf,
					bnum_t block, uint count)
{
	scsi_device_t *sdev = (scsi_device_t *)dev->private;
	status_t ret = NO_ERROR;

	g_scm.sdev = sdev;
	g_scm.buf = (u8 *)buf;
	g_scm.datalen = (u32)count * dev->block_size;

	/*
	 * Prepare CDB
	 *
	 * RDPROTECT is always zero here for UFS
	 */
	memset((void *)g_scm.cdb, 0, sizeof(g_scm.cdb));
	g_scm.cdb[0] = SCSI_OP_WRITE_10;
	set_dword_le(&g_scm.cdb[2], (u32)block);
	set_word_le(&g_scm.cdb[7], (u16)count);

	/* Actual issue */
	ret = sdev->exec(&g_scm);
	if (!ret)
		ret = scsi_parse_status(g_scm.status);

	return ret;
}

static status_t scsi_unmap(struct bdev *dev,
					bnum_t block, uint count)
{
	scsi_device_t *sdev = (scsi_device_t *)dev->private;
	status_t ret = NO_ERROR;

	g_scm.sdev = sdev;
	g_scm.buf = (u8 *)g_buf;
	g_scm.datalen = SCSI_UNMAP_DATA_LEN + 2;

	/*
	 * Prepare CDB
	 *
	 * Only one block descriptor is used for simple software control,
	 * thus only the first UNMAP block descriptor is configured.
	 */
	memset((void *)g_scm.cdb, 0, sizeof(g_scm.cdb));

	g_scm.datalen = SCSI_UNMAP_DATA_LEN + 8;

	g_scm.cdb[0] = SCSI_OP_UNMAP;
	set_word_le(&g_scm.cdb[7], g_scm.datalen);

	/* Prepare data */
	memset((void *)g_scm.buf, 0, SCSI_UNMAP_DATA_LEN + 2);

	set_word_le(&g_scm.buf[0], SCSI_UNMAP_DATA_LEN);
	set_word_le(&g_scm.buf[2], SCSI_UNMAP_BLOCK_DESC_DATA_LEN);

	set_dword_le(&g_scm.buf[12], block);
	set_dword_le(&g_scm.buf[16], count);

	/* Actual issue */
	ret = sdev->exec(&g_scm);
	if (!ret)
		ret = scsi_parse_status(g_scm.status);

	return ret;

}

int scsi_start_stop_unit(struct bdev *dev)
{
	scsi_device_t *sdev = (scsi_device_t *)dev->private;
	status_t ret = NO_ERROR;

	g_scm.sdev = sdev;

	/*
	 * Prepare CDB
	 *
	 * This is only to do power down now, thus offset 4 is always 3.
	 */
	memset((void *)g_scm.cdb, 0, sizeof(g_scm.cdb));
	g_scm.cdb[0] = SCSI_OP_START_STOP_UNIT;
	g_scm.cdb[4] = 3 << 4;

	/* Actual issue */
	ret = sdev->exec(&g_scm);
	if (!ret)
		ret = scsi_parse_status(g_scm.status);

	return ret;
}

static status_t scsi_inquiry(struct bdev *dev, void *buf)
{
	scsi_device_t *sdev = (scsi_device_t *)dev->private;
	status_t ret = NO_ERROR;

	g_scm.sdev = sdev;
	g_scm.buf = (u8 *)buf;

	/*
	 * Prepare CDB
	 *
	 * EVPD is always zero for standard inquiry data
	 */
	memset((void *)g_scm.cdb, 0, sizeof(g_scm.cdb));
	g_scm.datalen = 255;

	g_scm.cdb[0] = SCSI_OP_INQUIRY;
	set_word_le(&g_scm.cdb[3], g_scm.datalen);

	/* Actual issue */
	ret = sdev->exec(&g_scm);
	if (!ret)
		ret = scsi_parse_status(g_scm.status);

	return ret;
}

static int scsi_mode_sense(struct bdev *dev)
{
	scsi_device_t *sdev = (scsi_device_t *)dev->private;
	status_t ret = NO_ERROR;

	g_scm.sdev = sdev;

	/*
	 * Prepare CDB
	 */
	memset((void *)g_scm.cdb, 0, sizeof(g_scm.cdb));

	g_scm.cdb[0] = SCSI_OP_REQUEST_SENSE;
	g_scm.cdb[4] = 18;	/* ??? */

	/* Actual issue */
	ret = sdev->exec(&g_scm);
	if (!ret)
		ret = scsi_parse_status(g_scm.status);

	return ret;
}

static int scsi_read_capacity_10(struct bdev *dev, void *buf)
{
	scsi_device_t *sdev = (scsi_device_t *)dev->private;
	status_t ret = NO_ERROR;

	g_scm.sdev = sdev;
	g_scm.buf = (u8 *)buf;

	/*
	 * Prepare CDB
	 */
	memset((void *)g_scm.cdb, 0, sizeof(g_scm.cdb));
	g_scm.datalen = 8;

	g_scm.cdb[0] = SCSI_OP_READ_CAPACITY_10;

	/* Actual issue */
	ret = sdev->exec(&g_scm);
	if (!ret)
		ret = scsi_parse_status(g_scm.status);

	return ret;
}

static status_t scsi_format_unit(struct bdev *dev)
{
	scsi_device_t *sdev = (scsi_device_t *)dev->private;
	status_t ret = NO_ERROR;

	g_scm.sdev = sdev;

	/*
	 * Prepare CDB
	 */
	memset((void *)g_scm.cdb, 0, sizeof(g_scm.cdb));

	g_scm.cdb[0] = SCSI_OP_FORMAT_UNIT;

	/* Actual issue */
	ret = sdev->exec(&g_scm);
	if (!ret)
		ret = scsi_parse_status(g_scm.status);

	return ret;
}

static status_t scsi_secu_prot_in(struct bdev *dev, void *buf, bnum_t block, uint count)
{
	scsi_device_t *sdev = (scsi_device_t *)dev->private;
	status_t ret = NO_ERROR;

	g_scm.sdev = sdev;
	g_scm.buf = (u8 *)buf;
	g_scm.datalen = (u32)count * dev->block_size;

	/*
	 * Prepare CDB
	 *
	 * SECURITY PROTOCOL 0xEC means UFS.
	 */
	memset((void *)g_scm.cdb, 0, sizeof(g_scm.cdb));
	g_scm.cdb[0] = SCSI_OP_SECU_PROT_IN;
	g_scm.cdb[1] = 0xEC;
	set_word_le(&g_scm.cdb[2], 0x1);
	set_dword_le(&g_scm.cdb[6], (u32)count);

	/* Actual issue */
	ret = sdev->exec(&g_scm);
	if (!ret)
		ret = scsi_parse_status(g_scm.status);

	return ret;
}

static status_t scsi_secu_prot_out(struct bdev *dev, const void *buf, bnum_t block, uint count)
{
	scsi_device_t *sdev = (scsi_device_t *)dev->private;
	status_t ret = NO_ERROR;

	g_scm.sdev = sdev;
	g_scm.buf = (u8 *)buf;
	g_scm.datalen = (u32)count * dev->block_size;

	/*
	 * Prepare CDB
	 *
	 * SECURITY PROTOCOL 0xEC means UFS.
	 * SECURITY PROTOCOL SPECIFIC 0x1 means device specific.
	 */
	memset((void *)g_scm.cdb, 0, sizeof(g_scm.cdb));
	g_scm.cdb[0] = SCSI_OP_SECU_PROT_OUT;
	g_scm.cdb[1] = 0xEC;
	set_word_le(&g_scm.cdb[2], 0x1);
	set_dword_le(&g_scm.cdb[6], (u32)count);

	/* Actual issue */
	ret = sdev->exec(&g_scm);
	if (!ret)
		ret = scsi_parse_status(g_scm.status);

	return ret;
}

static status_t scsi_scan_common(scsi_device_t *sdev, u32 i)
{
	status_t ret = NO_ERROR;

	/*
	 * Check if a device exists and if true,
	 * get device infomations
	 */
	ret = scsi_inquiry(&sdev->dev, (void *)g_buf);
	if (ret < 0) {
		ret = ERR_NOT_FOUND;
		goto err;
	}

	memcpy(&sdev->vendor[0], &g_buf[8], 40);
	sdev->vendor[43] = '\0';
	memcpy(&sdev->product[0], &g_buf[16], 20);
	sdev->product[23] = '\0';
	memcpy(&sdev->revision[0], &g_buf[32], 8);
	sdev->revision[11] = '\0';

	printf("[SCSI] LU%u\t%s\t%s\t%s\n", sdev->lun, sdev->vendor,
			sdev->product, sdev->revision);

	/* Clear Unit Attention Condition per device */
	ret = scsi_mode_sense(&sdev->dev);
	if (ret < 0) {
		printf("[SCSI] MODE SENSE failed: %d\n", ret);
		goto err;
	}
err:
	return ret;
}
/*
 * The relations between initiators and targets should not seem like web here.
 * That is, any target need to be connected to only one initiator,
 * even if SCSI Architecture Model also allows the contrary and
 * you can't see any 'target' literally.
 */
status_t scsi_scan(scsi_device_t *sdev, u32 wlun, u32 dev_num, exec_t *func,
				const char *name_s, bnum_t max_seg)
{
	u32 i, j;
	char name[16];
	size_t block_size;
	bnum_t block_count;
	status_t ret = NO_ERROR;

	/* Enumeration */
	if (wlun) {
		i = wlun;
		j = wlun + 1;
	} else {
		i = 0;
		j = dev_num;
	}
	for (; i < j; i++) {
		if (wlun) {
			sdev->lun = wlun;
			snprintf(name, sizeof(name), "scsi%s", name_s);
		} else {
			sdev->lun = i;
			snprintf(name, sizeof(name), "scsi%u", i);
		}

		/* for lower driver */
		sdev->dev.private = sdev;
		sdev->exec = func;

		ret = scsi_scan_common(sdev, i);
		if (ret == ERR_NOT_FOUND)
			continue;
		else if (ret < 0)
			break;

		/* Get max LBA and block size */
		if (wlun == 0) {
#ifdef CONFIG_EXYNOS_BOOTLOADER_DISPLAY
			u32 capacity = 0;
#endif
			ret = scsi_read_capacity_10(&sdev->dev, g_buf);
			if (ret < 0) {
				printf("[SCSI] READ CAPACITY 10 failed: %d\n",
							ret);
				break;
			}

			block_size = get_dword_le(&g_buf[4]);
			block_count = get_dword_le(&g_buf[0]) + 1;

#ifdef CONFIG_EXYNOS_BOOTLOADER_DISPLAY
			capacity = ((block_size * (block_count + 1)) / 1024 / 1024);

			if (capacity > 1024) {
				capacity /= 1024;
				print_lcd(FONT_WHITE, FONT_BLACK, "[UFS] LU%u\t%s\t%s\t%s\t%s\t%3d GB",
						sdev->lun, name, sdev->vendor, sdev->product, sdev->revision, capacity);
			} else {
				print_lcd(FONT_WHITE, FONT_BLACK, "[UFS] LU%u\t%s\t%s\t%s\t%s\t%3d MB",
						sdev->lun, name, sdev->vendor, sdev->product, sdev->revision, capacity);
			}
#endif
		} else {
			/*
			 * This is for RPMB W-LUN. Origially it's block size
			 * and capacity should be got in another way, but
			 * but now we pre-define them. If you access more than
			 * its capacity, the acccess would fail.
			 */
			block_size = !strcmp(name_s, "rpmb") ?
						RPMB_MSG_DATA_SIZE: 4096;
			block_count = 0xFFFFFFFF;
		}

		bio_initialize_bdev(&sdev->dev,
				name,
				block_size,
				block_count,
				0,
				NULL,
				BIO_FLAGS_NONE);

		/* Override operations */
		if (wlun == 0) {
			sdev->dev.new_read_native = scsi_read_10;
			sdev->dev.new_write_native = scsi_write_10;
			sdev->dev.new_erase_native = scsi_unmap;
		} else {
			sdev->dev.new_read_native = scsi_secu_prot_in;
			sdev->dev.new_write_native = scsi_secu_prot_out;
		}
		sdev->dev.max_blkcnt_per_cmd = max_seg * block_size / USER_BLOCK_SIZE;

		bio_register_device(&sdev->dev);

		sdev++;
	}

	return ret;
}

status_t scsi_scan_ssu(scsi_device_t *sdev, u32 wlun,
					exec_t *func, get_sdev_t *func1)
{
	status_t ret = NO_ERROR;
	char name[16];

	/* Enumeration */
	sdev->dev.private = sdev;
	sdev->lun = wlun;
	snprintf(name, sizeof(name), "scsissu");
	sdev->exec = func;
	sdev->get_ssu_sdev = func1;

	ret = scsi_scan_common(sdev, wlun);

	if (!ret) {
		bio_initialize_bdev(&sdev->dev,
				name,
				4096,
				1,
				0,
				NULL,
				BIO_FLAGS_NONE);
		bio_register_device(&sdev->dev);
	}

	return ret;
}

status_t scsi_do_ssu()
{
	bdev_t *dev;
	status_t ret;

	dev = bio_open("scsissu");
	if (!dev) {
		printf("error opening block device\n");
		return ERR_GENERIC;
	}

	ret = scsi_start_stop_unit(dev);

	bio_close(dev);

	return ret;
}
