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
#include <part.h>
#include <lib/bio.h>
#include <dev/boot.h>
#include <lib/lock.h>
#include <platform/secure_boot.h>
#include <platform/smc.h>
#include <platform/cm_api.h>
#include <dev/rpmb.h>
#include <platform/sfr.h>
#include <platform/mmu/mmu_func.h>

#define CMD_STRING_MAX_SIZE 60

static uint32_t avbkey_is_trusted;
static struct AvbOps ops;
static KST_PUBKEY_ST ctx __attribute__((__aligned__(CACHE_WRITEBACK_GRANULE_64)));


uint32_t sb_get_avb_key(uint8_t *avb_pubkey, uint64_t pubkey_size,
		const char *keyname)
{
	uint32_t ret;
	uint32_t keyname_size = strlen(keyname);

	memset(&ctx, 0, sizeof(KST_PUBKEY_ST));
	ctx.ns_buf_addr = (uint64_t)avb_pubkey;
	ctx.ns_buf_size = pubkey_size;
	ctx.keyname_size = keyname_size;
	if (keyname_size > SB_MAX_PUBKEY_LEN)
		return AVB_ERROR_INVALID_KEYNAME_SIZE;
	memcpy(ctx.keyname, keyname, keyname_size);

	FLUSH_DCACHE_RANGE(&ctx, sizeof(KST_PUBKEY_ST));
	/* clean avb_pubkey */
	FLUSH_DCACHE_RANGE(avb_pubkey, pubkey_size);

	ret = exynos_smc((SMC_AARCH64_PREFIX | SMC_CM_SECURE_BOOT), SB_GET_AVB_KEY,
			(uint64_t)&ctx, 0);
	if (ret) {
		printf("[AVB 2.0 ERR] Fail to read AVB pubkey [ret: 0x%X]\n", ret);
	}

	INV_DCACHE_RANGE(avb_pubkey, pubkey_size);

	return ret;
}

static AvbIOResult exynos_read_is_device_unlocked(AvbOps *ops, bool *out_is_unlocked)
{
	AvbIOResult ret = AVB_IO_RESULT_OK;
	int lock_state;

	lock_state = get_lock_state();
	if (lock_state == -1)
		ret = -1;
	*out_is_unlocked = (bool)!lock_state;

	return ret;
}

static AvbIOResult exynos_get_size_of_partition(AvbOps *ops,
		const char *partition,
		uint64_t *out_size_num_bytes)
{
	AvbIOResult ret;
	void *part = part_get(partition);

	if (part) {
		*out_size_num_bytes = part_get_size_in_bytes(part);
		ret = AVB_IO_RESULT_OK;
	} else {
		ret = AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION;
	}

	return ret;
}

static AvbIOResult exynos_read_from_partition(AvbOps *ops,
		const char *partition,
		int64_t offset,
		size_t num_bytes,
		void *buffer,
		size_t *out_num_read)
{
	void *part = part_get(partition);
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
	uint32_t ret = 0;
	uint64_t partition_size = 0;

	if (offset < 0) {
		ret = exynos_get_size_of_partition(ops, partition, &partition_size);
		if (ret) {
			printf("There is no partition [%s]\n", partition);
			return ret;
		}
		offset = partition_size + offset;
	}

	boot_dev = get_boot_device();
	if (boot_dev == BOOT_UFS)
		name = "scsi0";
	else if (boot_dev == BOOT_EMMC) {
		name = "mmc0";
	} else {
		printf("Boot device: 0x%x. Unsupported boot device!\n", boot_dev);
		return AVB_IO_RESULT_ERROR_IO;
	}

	dev = bio_open(name);

	if (!part)
		return AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION;

	tmp_buff = (char *)memalign(0x1000, PART_SECTOR_SIZE);
	if (tmp_buff == NULL)
		return AVB_IO_RESULT_ERROR_OOM;

	printf("First block\n");
	/* First block */
	tmp_num_bytes = num_bytes;
	tmp_offset = offset;
	tmp_num_read = ((PART_SECTOR_SIZE - (tmp_offset % PART_SECTOR_SIZE)) < tmp_num_bytes)
					? (PART_SECTOR_SIZE - (tmp_offset % PART_SECTOR_SIZE)) : tmp_num_bytes;
	blkstart = part_get_start_in_secs(part) + (u32)(offset / PART_SECTOR_SIZE);
	dev->new_read(dev, tmp_buff, blkstart, 1);
	memcpy((void *)p, (void *)(tmp_buff + (tmp_offset % PART_SECTOR_SIZE)), tmp_num_read);
	p += tmp_num_read;
	tmp_offset += tmp_num_read;
	blkstart += 1;
	tmp_num_bytes -= tmp_num_read;

	/* Middle blocks */
	blknum = tmp_num_bytes / PART_SECTOR_SIZE;
	if (blknum) {
		printf("Middle blocks\n");
		for (i = 0; i < blknum; i++) {
			dev->new_read(dev, tmp_buff, blkstart + i, 1);

			memcpy((void *)p, (void *)tmp_buff, PART_SECTOR_SIZE);
			p += PART_SECTOR_SIZE;
		}
		tmp_offset += (PART_SECTOR_SIZE * i);
		tmp_num_bytes -= (PART_SECTOR_SIZE * i);
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

static uint32_t exynos_remove_unnecessary_region(
	AvbOps *ops,
	const char *partition,
	uint64_t partition_addr,
	size_t num_bytes)
{
	uint32_t ret = 0;
	uint64_t partition_size = 0;
	AvbFooter *footer_ptr = NULL;
	AvbFooter footer;
	AvbVBMetaImageHeader h;
	const uint8_t *header_block;
	AvbDescriptor *descriptor_ptr;
	AvbHashDescriptor hash_desc;
	const uint8_t* desc_partition_name = NULL;
	uint64_t r_address;
	uint32_t r_size;

	ret = exynos_get_size_of_partition(ops, partition, &partition_size);
	if (ret) {
		printf("%s: There is no partition [%s]\n", __func__, partition);
		goto out;
	}

	footer_ptr = (AvbFooter *)(partition_addr + partition_size - sizeof(AvbFooter));
	ret = avb_footer_validate_and_byteswap(footer_ptr, &footer);
	if (ret == false) {
		printf("%s: Footer parsing fail [%s]\n", __func__, partition);
		ret = -1;
		goto out;
	}

	header_block = (uint8_t *)(partition_addr + footer.vbmeta_offset);
	avb_vbmeta_image_header_to_host_byte_order(
			(const AvbVBMetaImageHeader*)header_block, &h);

	descriptor_ptr = (AvbDescriptor *)(header_block +
			sizeof(AvbVBMetaImageHeader) +
			h.authentication_data_block_size +
			h.descriptors_offset);
	ret = avb_hash_descriptor_validate_and_byteswap(
			(AvbHashDescriptor *)descriptor_ptr, &hash_desc);
	if (ret == false) {
		printf("%s: Descriptor parsing fail [%s]\n", __func__, partition);
		ret = -1;
		goto out;
	}

	desc_partition_name = ((const uint8_t*)descriptor_ptr) +
		sizeof(AvbHashDescriptor);
	ret = memcmp(partition, desc_partition_name, hash_desc.partition_name_len);
	if (ret) {
		printf("%s: Descriptor partition is different [%s, %s]\n",
				__func__, partition, desc_partition_name);
		goto out;
	}

	r_address = partition_addr + hash_desc.image_size;
	r_size = partition_size - hash_desc.image_size;
	memset((void *)r_address, 0, r_size);

out:
	if (ret)
		printf("%s: return = [0x%X]\n", __func__, ret);
	return ret;
}

static AvbIOResult exynos_get_preloaded_partition(AvbOps *ops,
		const char *partition,
		size_t num_bytes,
		uint8_t **out_pointer,
		size_t *out_num_bytes_preloaded)
{
	AvbIOResult ret = AVB_IO_RESULT_OK;
	void *part;
	bool unlock;

	if (!strcmp(partition, "boot")) {
		*out_pointer = (uint8_t *)BOOT_BASE;
	} else if (!strcmp(partition, "dtbo")) {
		*out_pointer = (uint8_t *)DTBO_BASE;
	} else {
		if (!(part = part_get(partition))) {
			ret = AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION;
			goto out;
		}

		if (part_read(part, (void *)AVB_PRELOAD_BASE)) {
			ret = AVB_IO_RESULT_ERROR_IO;
			goto out;
		}

		*out_pointer = (uint8_t *)AVB_PRELOAD_BASE;
	}
	ret = exynos_read_is_device_unlocked(ops, &unlock);
	if (ret)
		goto out;

	if (!unlock) {
		ret = exynos_remove_unnecessary_region(ops, partition,
				(uint64_t)*out_pointer, num_bytes);
		if (ret)
			goto out;
	}

	*out_num_bytes_preloaded = num_bytes;
	return ret;

out:
	*out_pointer = NULL;
	return ret;
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
	uint8_t avb_pubkey[SB_MAX_PUBKEY_LEN] __attribute__((__aligned__(CACHE_WRITEBACK_GRANULE_128)));

	ret = sb_get_avb_key(avb_pubkey, public_key_length, "vbmeta");
	if (ret) {
		*out_is_trusted = false;
	}

	*out_is_trusted = !memcmp(avb_pubkey, public_key_data, public_key_length);
	if (*out_is_trusted == false) {
		printf("[AVB 2.0 ERR] AVB pubkey is not matched with vbmeta\n");
#if defined(CONFIG_AVB_DEBUG)
		uint32_t i;
		printf("Keystorage key dump\n");
		for (i = 0; i < public_key_length; i++)
			printf("%02X ", avb_pubkey[i]);
		printf("\n");
		printf("vbmeta key dump\n");
		for (i = 0; i < public_key_length; i++)
			printf("%02X ", public_key_data[i]);
		printf("\n");
#endif
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

static AvbIOResult exynos_get_unique_guid_for_partition(AvbOps *ops,
		const char *partition,
		char *guid_buf,
		size_t guid_buf_size)
{
	AvbIOResult ret;
	void *part = part_get(partition);

	if (!part)
		return AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION;

	if (part_get_unique_guid(part, (char *)partition, guid_buf))
		ret = AVB_IO_RESULT_ERROR_IO;
	else
		ret = AVB_IO_RESULT_OK;

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
