/*
 * Copyright (c) 2024 Igor Belwon <igor.belwon@mentallysanemainliners.org>
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 *
 * Samsung PIT format library. Pretty dangerous.
 * Reimplements Samsung's functions. Possibly in a saner manner.
 * Currently un-implemented are UFS LUNs.
 * So, you can only read/write to LUN0 partitions.
 */

#include <ctype.h>
#include <err.h>
#include <part_dev.h>
#include <types.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <lib/font_display.h>

#include "pit.h"

static struct pit_info pit;

static int pit_blk_count = 64;
static void *pit_buf;
static bdev_t *pit_dev;

// If we put this in the header it may
// collide with the actual PIT structure info.
static int pit_available = 0;

// You shouldn't mess with this. Ever.
static int read_raw_boot_partition = 0;

/* -------------------- Private API -------------------- */

/*
 * pit_set_final_block_count - Read PIT partition and set its final block count
 * @pit: PIT data
 *
 * Returns NO_ERROR if the operation succeeded
 * Otherwise, ERR_NOT_VALID.
*/
static int pit_set_final_block_count(struct pit_info *pit)
{
	struct pit_entry *ptn;
	int ret;

	ptn = pit_get_part_info("PIT");
	if (!ptn)
		return ERR_NOT_VALID;

	printf("New block count: %d\n", ptn->blknum);

	pit_blk_count = ptn->blknum;

	return NO_ERROR;
}

/*
 * pit_print_header - print PIT header information
 * @pit: PIT data
 *
 * Prints PIT header data in a pretty format to the console.
 * Does not show up on LCD, unless if you're using a debug build.
 *
 * Returns nothing.
*/
static void pit_print_header(struct pit_info *pit)
{
	printf("------ PIT Header debug ------\n");
	printf("Magic: 0x%x\n", pit->magic);
	printf("Count: %d\n", pit->count);
	printf("Reserved: %d %d %d %d %d\n",
		pit->reserved[0], pit->reserved[1], pit->reserved[2],
		pit->reserved[3], pit->reserved[4]);
	printf("------------------------------\n");
}

/*
 * pit_check_header - check PIT header integrity
 * @pit: PIT data
 *
 * Checks if PIT header is valid. Also, does some
 * sanity checks for the partition count.
 *
 * Returns NO_ERROR if PIT header is valid.
 * Otherwise, ERR_NOT_VALID or ERR_TOO_BIG.
*/
static int pit_check_header(struct pit_info *pit)
{
	if (pit->magic != PIT_MAGIC)
		return ERR_NOT_VALID;

	if (pit->count > PIT_MAX_PART_NUM)
		return ERR_TOO_BIG;

	/*
	 * If the header is correct, the PIT
	 * should be too. So, make it available.
	*/
	pit_available = 1;

	return NO_ERROR;
}

/*
 * pit_load_blocks - load PIT blocks from the device
 * @buf: buffer to load PIT blocks into
 *
 * Loads PIT blocks from the device into the buffer.
 *
 * Returns NO_ERROR if PIT blocks are loaded successfully.
 * Otherwise, ERR_IO.
*/
static int pit_load_blocks(void *buf)
{
	uint blks;

	blks = pit_dev->new_read(pit_dev, buf, PIT_DISK_LOC, pit_blk_count);

	if (blks != pit_blk_count)
		return ERR_IO;

	return NO_ERROR;
}

/*
 * pit_get_virtual_partition_entry - get a virtual partition entry from PIT
 * @int: either 1 for boot, or 2 for lk3rd
 *
 * Do not use this as a public API, pit_get_part_info handles everything.
 *
 * Returns struct pit_entry.
*/
struct pit_entry *pit_get_virtual_partition_entry(int partition)
{
	struct pit_entry *pet;
	struct pit_entry *new;

	// Read the whole boot partition
	read_raw_boot_partition = 1;
	pet = pit_get_part_info("boot");
	read_raw_boot_partition = 0;

	new = malloc(sizeof(struct pit_entry));
	memcpy(new, pet, sizeof(struct pit_entry));

	if(partition == 1)
	{
		// Need to read the remainder of the partition
		new->blkstart += 0x200000 / PIT_UFS_BLK_SIZE;
		new->blknum -= 0x200000 / PIT_UFS_BLK_SIZE;
		strcpy(new->name, "BOOT");
	}
	else if(partition == 2)
	{
		// Need to read only the first 0x200000 bytes
		new->blknum = 0x200000 / PIT_UFS_BLK_SIZE;
		strcpy(new->name, "LK3RD");
	}
	else
	{
		printf("%s: invalid partition type", __func__);
	}
end:
	return new;
}

/* -------------------- Public API -------------------- */

/*
 * pit_init - do PIT initialization.
 * @id: boot device ID.
 *
 * Initializes PIT block device, checks its header and gets PIT block count
 * from the "PIT" partition on the device.
 *
 * Returns to code execution if PIT init succeeds. Otherwise, panics.
 */
void pit_init(enum __boot_dev_id id)
{
	int ret;

	// Only tested on UFS devices. Reject all others.
	if (id != DEV_UFS) {
		printf("Device doesn't boot with UFS. \
			Sorry, we don't support non-UFS devices.\n");
		goto error;
	}

	pit_buf = malloc(PIT_SIZE_LIMIT);
	if (!pit_buf) {
		printf("Couldn't allocate PIT memory buffer. Check your memory map.\n");
		goto error;
	}

	pit_dev = bio_open("scsi0");
	if(!pit_dev) {
		printf("Couldn't open scsi0 block device.\n");
		goto error;
	}

	ret = pit_load_blocks(pit_buf);
	if (ret != NO_ERROR) {
		printf("Failed to load PIT blocks. This is (most probably) a bug.\n");
		goto error;
	}
	bio_close(pit_dev);

	// Buffer is loaded. Populate the PIT struct in memory.
	memcpy(&pit, pit_buf, sizeof(struct pit_info));

	ret = pit_check_header(&pit);
	if (ret != NO_ERROR) {
		printf("PIT header check failed.\n");
		goto error;
	}

	ret = pit_set_final_block_count(&pit);
	if (ret != NO_ERROR) {
		printf("Failed to set final block count.\n");
		goto error;
	}

	return;
error:
	/*
	 * LK code will use PIT regardless of the return value.
	 * Or, it will just flat out crash. Either is bad.
	 * Assume it's too dangerous to continue. Don't.
	*/
	panic("Failed to initialize PIT. \
	       If this device doesn't have storage support, \
	       or is eMMC, please set LK3RD_NO_PIT.");
}

/*
 * pit_get_part_info - get PIT entry for partition
 * @name: partition name
 *
 * Gets the entry for a partition from the PIT.
 * Also, handles the lk3rd virtual partition.
 *
 * Returns a pit_entry if the partition is found.
 * Otherwise, 0.
*/
struct pit_entry *pit_get_part_info(const char *name)
{
	int need_to_lie = 0;

	if (!pit_available)
		return 0;

	// Check for virtual partitions
	if (strcicmp(name, "boot") == 0 && read_raw_boot_partition != 1)
		need_to_lie = 1;
	else if (strcicmp(name, "lk3rd") == 0)
		need_to_lie = 2;

	for (u32 i = 0; i < pit.count; i++) {
		struct pit_entry *ptn = &pit.pte[i];

		if(need_to_lie != 0)
			return pit_get_virtual_partition_entry(need_to_lie);

		if (strcicmp(name, ptn->name) == 0)
		{
			return ptn;
		}
	}

	printf("Partition %s does not exist in PIT.\n", name);
	return 0;
}

/*
 * pit_access - do an operation on a block device
 * @ptn: PIT entry
 * @op: operation type to do
 * @addr: memory address to read/write to
 * @size: unused. kept for parity
 *
 * Does an operation on a block device. This can be
 * flash, erase, or load (read).
 *
 * Returns NO_ERROR if the operation completed successfully.
 * Otherwise, ERR_IO, ERR_NOT_VALID, ERR_NOT_CONFIGURED.
*/
int pit_access(struct pit_entry *ptn, int op, u64 addr, u32 size)
{
	int ret;
	unsigned int len;
	char str[PIT_MAX_PART_NUM];
	bdev_t *dev;
	u64 start, num;

	if(!pit_available)
		return ERR_NOT_CONFIGURED;

	/* Open device, do operation */
	dev = bio_open("scsi0");
	if (!dev)
		return ERR_IO;

	/* Calculate partiton geometry for UFS. */
	start = ptn->blkstart << 3;
	num = ptn->blknum << 3;

	switch (op) {
		case PIT_OP_FLASH:
			ret = dev->new_write(dev, (void *)addr, start, num);
			break;
		case PIT_OP_ERASE:
			ret = dev->new_erase(dev, start, num);
			break;
		case PIT_OP_LOAD:
			ret = dev->new_read(dev, (void *)addr, start, num);
			break;
		default:
			printf("Unknown PIT operation?\n");
			return ERR_NOT_VALID;
	}

	/* Sanity checks, cleanup */
	bio_close(dev);

	if(ret != num)
		ret = ERR_IO;
	else
		ret = NO_ERROR;

	return ret;
}

/*
 * pit_get_length - get a partition length from PIT
 * @ptn: PIT entry
 *
 * Returns the partition length if successful.
 * Otherwise, 0.
*/
u64 pit_get_length(struct pit_entry *ptn)
{
	// Fastboot will reject writing if partition size is 0.
	if(!pit_available)
		return 0;

	/* UFS uses FMP for the userdata partition */
	if(!strcmp("userdata", ptn->name))
		return (ptn->blknum - PIT_UFS_FMP_USE_SIZE) * PIT_UFS_BLK_SIZE;

	return ptn->blknum * PIT_UFS_BLK_SIZE;
}

/*
 * pit_entry_write - write to a PIT entry partially
 * @ptn: PIT entry
 * @buf: buffer to write from
 * @offset: offset to write to
 * @size: unused, kept for parity
 *
 * Returns NO_ERROR if successful.
 * Otherwise, ERR_IO.
*/
int pit_entry_write(struct pit_entry *ptn, void *buf, u64 offset, u64 size)
{
	int ret = 0;
	bdev_t *dev;
	u64 start, num;

	if(!pit_available)
		return ERR_NOT_CONFIGURED;

	dev = bio_open("scsi0");
	if (!dev)
		return ERR_IO;

	/* Calculate partition geometry for UFS. */
	start = (ptn->blkstart + offset) << 3;
	num = ptn->blknum << 3;

	ret = dev->new_write(dev, buf, start, num);

	if(ret != num)
		return ERR_IO;

	bio_close(dev);

	return NO_ERROR;
}

/*
 * pit_entry_read - read from a PIT entry partially
 * @ptn: PIT entry
 * @buf: buffer to read to
 * @offset: offset to read from
 * @size: unused, kept for parity
 *
 * Returns NO_ERROR if successful.
 * Otherwise, ERR_IO.
*/
int pit_entry_read(struct pit_entry *ptn, void *buf, u64 offset, u64 size)
{
	int ret = 0;
	bdev_t *dev;
	u64 start, num;

	if(!pit_available)
		return ERR_NOT_CONFIGURED;

	dev = bio_open("scsi0");
	if (!dev)
		return ERR_IO;

	/* Calculate partition geometry for UFS. */
	start = (ptn->blkstart + offset) << 3;
	num = ptn->blknum << 3;

	ret = dev->new_read(dev, buf, start, num);

	if(ret != ptn->blknum << 3)
		return ERR_IO;

	bio_close(dev);

	return ret;
}

/*
 * pit_entry_erase - erase a PIT entry partially
 * @ptn: PIT entry
 * @offset: offset to read from
 * @size: unused, kept for parity
 *
 * Returns NO_ERROR if successful.
 * Otherwise, ERR_IO.
*/
int pit_entry_erase(struct pit_entry *ptn, u64 offset, u64 size)
{
	int ret = 0;
	bdev_t *dev;
	u64 start, num;

	if(!pit_available)
		return ERR_NOT_CONFIGURED;

	dev = bio_open("scsi0");
	if (!dev)
		return ERR_IO;

	/* Calculate partition geometry for UFS. */
	start = (ptn->blkstart + offset) << 3;
	num = ptn->blknum << 3;

	ret = dev->new_erase(dev, start, num);

	if(ret != ptn->blknum << 3)
		return ERR_IO;

	bio_close(dev);

	return NO_ERROR;
}

/* -------------------- Unimplemented. Public API. --------------------*/

int pit_update(void *buf, unsigned int size)
{
	printf("PIT update is not implemented in pitlib. Sorry!\n");
	return 0;
}
