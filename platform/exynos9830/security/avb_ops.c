/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or
 * distributed, transmitted, transcribed, stored in a retrieval system or
 * translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed to third parties
 * without the express written permission of Samsung Electronics.
 */

#include <debug.h>
#include <stdlib.h>
#include <string.h>
#include <pit.h>
#include <lib/bio.h>
#include <dev/boot.h>
#include <platform/lock.h>
#include <platform/secure_boot.h>
#include <platform/smc.h>
#include <dev/rpmb.h>
#include <platform/sfr.h>

#define CMD_STRING_MAX_SIZE	60
#define AVB_PRELOAD_BASE	0xA0000000

static uint32_t avbkey_is_trusted;
static struct AvbOps ops;

int get_unique_guid(char *ptr_name, char *buf);

static AvbIOResult exynos_read_from_partition(AvbOps *ops,
                                              const char *partition,
                                              int64_t offset,
                                              size_t num_bytes,
                                              void *buffer,
                                              size_t *out_num_read)
{
	struct pit_entry *ptn;
	bdev_t *dev;
	const char *name;
	unsigned int boot_dev;
	u32 blkstart;
	u32 blknum;
	char *tmp_buff;
	char *p = (char *)buffer;
	u32 tmp_num_read;
	uint64_t tmp_offset;
	size_t tmp_num_bytes;
	u32 i;

	boot_dev = get_boot_device();
	if (boot_dev == BOOT_UFS) {
		name = "scsi0";
	} else {
		printf("Boot device: 0x%x. Unsupported boot device!\n", boot_dev);
		return AVB_IO_RESULT_ERROR_IO;
	}

	dev = bio_open(name);

	ptn = pit_get_part_info(partition);
	if (ptn == 0)
		return AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION;

	tmp_buff = (char *)memalign(0x1000, PIT_SECTOR_SIZE);
	if (tmp_buff == NULL)
		return AVB_IO_RESULT_ERROR_OOM;

	printf("First block\n");
	/* First block */
	tmp_num_bytes = num_bytes;
	tmp_offset = offset;
	tmp_num_read = ((PIT_SECTOR_SIZE - (tmp_offset % PIT_SECTOR_SIZE)) < tmp_num_bytes)
	               ? (PIT_SECTOR_SIZE - (tmp_offset % PIT_SECTOR_SIZE)) : tmp_num_bytes;
	blkstart = ptn->blkstart + (u32)(offset / PIT_SECTOR_SIZE);
	dev->new_read(dev, tmp_buff, blkstart, 1);
	memcpy((void *)p, (void *)(tmp_buff + (tmp_offset % PIT_SECTOR_SIZE)), tmp_num_read);
	p += tmp_num_read;
	tmp_offset += tmp_num_read;
	blkstart += 1;
	tmp_num_bytes -= tmp_num_read;

	/* Middle blocks */
	blknum = tmp_num_bytes / PIT_SECTOR_SIZE;
	if (blknum) {
		printf("Middle blocks\n");
		for (i = 0; i < blknum; i++) {
			dev->new_read(dev, tmp_buff, blkstart + i, 1);

			memcpy((void *)p, (void *)tmp_buff, PIT_SECTOR_SIZE);
			p += PIT_SECTOR_SIZE;
		}
		tmp_offset += (PIT_SECTOR_SIZE * i);
		tmp_num_bytes -= (PIT_SECTOR_SIZE * i);
		tmp_num_read = tmp_num_bytes;
		blkstart += i;
	}

	/* Last block */
	if (tmp_num_bytes) {
		printf("Last block\n");
		dev->new_read(dev, tmp_buff, blkstart, 1);
		memcpy((void *)p, (void *)tmp_buff, tmp_num_read);
	}

	printf("free(tmp_buff)\n");
	free(tmp_buff);

	printf("*out_num_read = num_bytes\n");
	*out_num_read = num_bytes;

	bio_close(dev);

	return AVB_IO_RESULT_OK;
}

static AvbIOResult exynos_get_preloaded_partition(AvbOps *ops,
                                                  const char *partition,
                                                  size_t num_bytes,
                                                  uint8_t **out_pointer,
                                                  size_t *out_num_bytes_preloaded)
{
	struct pit_entry *ptn;
	bdev_t *dev;
	const char *name;
	unsigned int boot_dev;

	boot_dev = get_boot_device();
	if (boot_dev == BOOT_UFS) {
		name = "scsi0";
	} else {
		printf("Boot device: 0x%x. Unsupported boot device!\n", boot_dev);
		return AVB_IO_RESULT_ERROR_IO;
	}

	dev = bio_open(name);

	ptn = pit_get_part_info(partition);
	if (ptn == 0)
		return AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION;

	dev->new_read(dev, (void *)AVB_PRELOAD_BASE, ptn->blkstart, ptn->blknum);

	*out_pointer = (uint8_t *)AVB_PRELOAD_BASE;
	*out_num_bytes_preloaded = num_bytes;

	bio_close(dev);

	return AVB_IO_RESULT_OK;
}

static AvbIOResult exynos_write_to_partition(AvbOps *ops,
                                             const char *partition,
                                             int64_t offset,
                                             size_t num_bytes,
                                             const void *buffer)
{
	AvbIOResult ret = AVB_IO_RESULT_OK;

	//TODO

	return ret;
}

static AvbIOResult exynos_validate_vbmeta_public_key(AvbOps *ops,
                                                     const uint8_t *public_key_data,
                                                     size_t public_key_length,
                                                     const uint8_t *public_key_metadata,
                                                     size_t public_key_metadata_length,
                                                     bool *out_is_trusted)
{
	AvbIOResult ret = AVB_IO_RESULT_OK;
	uint8_t buf[SB_MAX_PUBKEY_LEN] __attribute__((__aligned__(CACHE_WRITEBACK_GRANULE_128)));

	ret = exynos_smc((SMC_AARCH64_PREFIX | SMC_CM_SECURE_BOOT), SB_GET_AVB_KEY,
	                 (uint64_t)buf, public_key_length);
	if (ret) {
		*out_is_trusted = false;
		printf("[AVB 2.0 ERR] Fail to read AVB pubkey [ret: 0x%X]\n", ret);
		goto out;
	}

	INV_DCACHE_RANGE(buf, public_key_length)
	* out_is_trusted = !memcmp(buf, public_key_data, public_key_length);
	if (*out_is_trusted == false) {
		printf("[AVB 2.0 ERR] AVB pubkey is not matched with vbmeta\n");
		goto out;
	}

out:
	avbkey_is_trusted = *out_is_trusted;
	return ret;
}

static AvbIOResult exynos_read_rollback_index(AvbOps *ops,
                                              size_t rollback_index_location,
                                              uint64_t *out_rollback_index)
{
	AvbIOResult ret = AVB_IO_RESULT_OK;

#if defined(CONFIG_USE_RPMB)
	ret = rpmb_get_rollback_index(rollback_index_location, out_rollback_index);
#else
	*out_rollback_index = 0;
	printf("RP index is read as 0. RPMB was disabled.\n");
#endif

	return ret;
}

static AvbIOResult exynos_write_rollback_index(AvbOps *ops,
                                               size_t rollback_index_location,
                                               uint64_t rollback_index)
{
	AvbIOResult ret = AVB_IO_RESULT_OK;

#if defined(CONFIG_USE_RPMB)
	ret = rpmb_set_rollback_index(rollback_index_location, rollback_index);
#else
	printf("RP index is not written. RPMB was disabled.\n");
#endif

	return ret;
}

static AvbIOResult exynos_read_is_device_unlocked(AvbOps *ops, bool *out_is_unlocked)
{
	AvbIOResult ret = AVB_IO_RESULT_OK;

	*out_is_unlocked = (bool)!get_lock_state();

	return ret;
}

static AvbIOResult exynos_get_unique_guid_for_partition(AvbOps *ops,
                                                        const char *partition,
                                                        char *guid_buf,
                                                        size_t guid_buf_size)
{
	AvbIOResult ret;
	struct pit_entry *ptn;

	ptn = pit_get_part_info(partition);
	if (ptn == 0)
		return AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION;

	if (get_unique_guid((char *)partition, guid_buf))
		ret = AVB_IO_RESULT_ERROR_IO;
	else
		ret = AVB_IO_RESULT_OK;

	return ret;
}

static AvbIOResult exynos_get_size_of_partition(AvbOps *ops,
                                                const char *partition,
                                                uint64_t *out_size_num_bytes)
{
	AvbIOResult ret;
	struct pit_entry *ptn;

	ptn = pit_get_part_info(partition);
	if (ptn) {
		*out_size_num_bytes = pit_get_length(ptn);
		ret = AVB_IO_RESULT_OK;
	} else {
		ret = AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION;
	}

	return ret;
}

static AvbIOResult exynos_read_persistent_value(AvbOps *ops,
                                                const char *name,
                                                size_t buffer_size,
                                                uint8_t *out_buffer,
                                                size_t *out_num_bytes_read)
{
	int ret;

	if (!name) {
		*out_num_bytes_read = 0;
		return AVB_IO_RESULT_ERROR_NO_SUCH_VALUE;
	}

	if (buffer_size == 0) {
		*out_num_bytes_read = 0;
		return AVB_IO_RESULT_OK;
	}

	if (!out_buffer) {
		*out_num_bytes_read = 0;
		return AVB_IO_RESULT_ERROR_IO;
	}

	ret = rpmb_read_persistent_value(name, buffer_size, out_buffer, out_num_bytes_read);

	if (!ret)
		return AVB_IO_RESULT_OK;

	switch (ret) {
	case RV_RPMB_PERSIST_NAME_NOT_FOUND:
		ret = AVB_IO_RESULT_ERROR_NO_SUCH_VALUE;
		break;
	case RV_RPMB_INVALID_PERSIST_DATA_SIZE:
		ret = AVB_IO_RESULT_ERROR_INSUFFICIENT_SPACE;
		break;
	default:
		ret = AVB_IO_RESULT_ERROR_IO;
		break;
	}

	return ret;
}

static AvbIOResult exynos_write_persistent_value(AvbOps *ops,
                                                 const char *name,
                                                 size_t value_size,
                                                 const uint8_t *value)
{
	int ret;

	if (!name)
		return AVB_IO_RESULT_ERROR_NO_SUCH_VALUE;

	if (value_size == 0)
		return AVB_IO_RESULT_OK;

	ret = rpmb_write_persistent_value(name, value_size, value);

	if (!ret)
		return AVB_IO_RESULT_OK;

	switch (ret) {
	case RV_RPMB_PERSIST_NAME_NOT_FOUND:
	case RV_RPMB_INVALID_KEY_LEN:
		ret = AVB_IO_RESULT_ERROR_NO_SUCH_VALUE;
		break;
	case RV_RPMB_INVALID_PERSIST_DATA_SIZE:
		ret = AVB_IO_RESULT_ERROR_INSUFFICIENT_SPACE;
		break;
	default:
		ret = AVB_IO_RESULT_ERROR_IO;
		break;
	}

	return ret;
}

void set_avbops(void)
{
	ops.read_from_partition = &exynos_read_from_partition;
	ops.get_preloaded_partition = &exynos_get_preloaded_partition;
	ops.write_to_partition = &exynos_write_to_partition;
	ops.validate_vbmeta_public_key = &exynos_validate_vbmeta_public_key;
	ops.read_rollback_index = &exynos_read_rollback_index;
	ops.write_rollback_index = &exynos_write_rollback_index;
	ops.read_is_device_unlocked = &exynos_read_is_device_unlocked;
	ops.get_unique_guid_for_partition = &exynos_get_unique_guid_for_partition;
	ops.get_size_of_partition = &exynos_get_size_of_partition;
	ops.read_persistent_value = &exynos_read_persistent_value;
	ops.write_persistent_value = &exynos_write_persistent_value;
	printf("avbops functions are set successfully\n");
}

uint32_t get_ops_addr(struct AvbOps **ops_addr)
{
	if (ops_addr == NULL)
		return -1;

	*ops_addr = &ops;

	return 0;
}

uint32_t get_avbkey_trust(void)
{
	return avbkey_is_trusted;
}
