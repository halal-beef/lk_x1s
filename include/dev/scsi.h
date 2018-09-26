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

#ifndef _SCSI_H
#define _SCSI_H

#include <sys/types.h>
#include <lib/bio.h>
#include <err.h>
#include <string.h>

typedef struct scsi_command_meta scm;
typedef struct scsi_device_s scsi_device_t;

typedef status_t (exec_t)(scm *);
typedef scsi_device_t *(get_sdev_t)(void);

struct scsi_device_s {
	bdev_t dev;
	u32 lun;
	// TODO:
	char vendor[40+1+3];
	char product[20+1+3];
	char revision[8+1+3];
	exec_t* exec;
	get_sdev_t* get_ssu_sdev;
};

/*
 * @cdb: command descriptor block
 * @lun: logical unit number
 * @datalen: data length in byte for CDB and COMMAND UPIU(UFS)
 */

struct scsi_command_meta {
	u8 cdb[16];
	u32 datalen;
	u8 *buf;
	u8 sense_buf[64];
	u8 status;

	scsi_device_t *sdev;
};

enum scsi_opcode {
	SCSI_OP_TEST_UNIT_READY		= 0x00,
	SCSI_OP_READ_CAPACITY_10	= 0x25,
	SCSI_OP_READ_10			= 0x28,
	SCSI_OP_WRITE_10		= 0x2A,
	SCSI_OP_SECU_PROT_IN		= 0xA2,
	SCSI_OP_SECU_PROT_OUT		= 0xB5,
	SCSI_OP_INQUIRY			= 0x12,
	SCSI_OP_WRITE_BUFFER		= 0x3B,
	SCSI_OP_REQUEST_SENSE		= 0x03,
	SCSI_OP_FORMAT_UNIT		= 0x04,
	SCSI_OP_START_STOP_UNIT		= 0x1B,
	SCSI_OP_UNMAP			= 0x42,
};

/* External Functions */
status_t scsi_scan(scsi_device_t *sdev, u32 wlun, u32 dev_num, exec_t *func,
				const char *name_s, bnum_t max_seg);
status_t scsi_scan_ssu(scsi_device_t *sdev, u32 wlun,
					exec_t *func, get_sdev_t *func1);
status_t scsi_do_ssu(void);
#endif				/* _SCSI_H */
