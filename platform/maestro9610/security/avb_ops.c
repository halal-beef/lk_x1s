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
#include <platform/secure_boot.h>

static struct AvbOps ops;

static AvbIOResult exynos_read_from_partition(AvbOps *ops,
		const char *partition,
		int64_t offset,
		size_t num_bytes,
		void *buffer,
		size_t *out_num_read)
{
	AvbIOResult ret = AVB_IO_RESULT_OK;
	//TODO

	return ret;
}

static AvbIOResult exynos_get_preloaded_partition(AvbOps *ops,
		const char *partition,
		size_t num_bytes,
		uint8_t **out_pointer,
		size_t *out_num_bytes_preloaded)
{
	AvbIOResult ret = AVB_IO_RESULT_OK;
	//TODO

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
	//TODO

	return ret;
}

static AvbIOResult exynos_read_rollback_index(AvbOps *ops,
		size_t rollback_index_location,
		uint64_t *out_rollback_index)
{
	AvbIOResult ret = AVB_IO_RESULT_OK;
	//TODO

	return ret;
}

static AvbIOResult exynos_write_rollback_index(AvbOps *ops,
		size_t rollback_index_location,
		uint64_t rollback_index)
{
	AvbIOResult ret = AVB_IO_RESULT_OK;
	//TODO

	return ret;
}

static AvbIOResult exynos_read_is_device_unlocked(AvbOps *ops, bool *out_is_unlocked)
{
	AvbIOResult ret = AVB_IO_RESULT_OK;
	//TODO

	return ret;
}

static AvbIOResult exynos_get_unique_guid_for_partition(AvbOps *ops,
		const char *partition,
		char *guid_buf,
		size_t guid_buf_size)
{
	AvbIOResult ret = AVB_IO_RESULT_OK;
	//TODO

	return ret;
}

static AvbIOResult exynos_get_size_of_partition(AvbOps *ops,
		const char *partition,
		uint64_t *out_size_num_bytes)
{
	AvbIOResult ret = AVB_IO_RESULT_OK;
	//TODO

	return ret;
}

static AvbIOResult exynos_read_persistent_value(AvbOps *ops,
		const char *name,
		size_t buffer_size,
		uint8_t *out_buffer,
		size_t *out_num_bytes_read)
{
	AvbIOResult ret = AVB_IO_RESULT_OK;
	//TODO

	return ret;
}

static AvbIOResult exynos_write_persistent_value(AvbOps *ops,
		const char *name,
		size_t value_size,
		const uint8_t *value)
{
	AvbIOResult ret = AVB_IO_RESULT_OK;
	//TODO

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
