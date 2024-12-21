#include <part.h>
#include <guid.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if GPT_PART
#include <platform/ab_update.h>
#endif
#include <dev/boot.h>
#include <lib/bio.h>
#include <lib/font_display.h>

#if INPUT_GPT_AS_PT
#else
int get_unique_guid(char *ptr_name, char *buf);
#endif

#if INPUT_GPT_AS_PT
#else
static const char *pit_fs_type_tokens[] = {
	"",
	"",
	"",
	"",
	"",
	"ext4",
	"f2fs",
};
#endif

static const char *part_dev_tokens[] = {
	"mmc",
	"scsi",
	"unknown",
};

static enum __boot_dev_id s_def_boot_dev_id = 0xFFFFFFFF;
static enum __boot_dev_id s_boot_dev_id;
static bdev_t *s_bdev;

void *part_get(const char *name)
{
	void *part;

#if INPUT_GPT_AS_PT
	part = (void *)gpt_get_entry(name);
#else
	part = (void *)pit_get_part_info(name);
#endif
	return part;
}

#if GPT_PART
void *part_get_ab(const char *name)
{
	void *part;
	char part_name[36 + 1];
	char postfix[3];
	int i;
	int len = strlen(name);
	int cur_slot;

	for (i = 0; i < len ; i++)
		part_name[i] = name[i];

	part_name[len] = '\0';

	/* only for A / B support case, add _a or _b */
	cur_slot = ab_current_slot();
	if (cur_slot == AB_SLOT_B) {
		sprintf(postfix, "_%1c", 'b');
		strcat(part_name, postfix);
	} else if (cur_slot == AB_SLOT_A) {
		sprintf(postfix, "_%1c", 'a');
		strcat(part_name, postfix);
	}
	printf("%s: Partition '%s' with %d > %s\n", __func__, name, len, part_name);
#if INPUT_GPT_AS_PT
	part = (void *)gpt_get_entry(part_name);
#else
	part = (void *)pit_get_part_info(part_name);
#endif

	return part;
}
#endif

u64 part_get_size_in_bytes(void *part) {
#if INPUT_GPT_AS_PT
	PART_ENTRY *entry = (PART_ENTRY *)part;

	return gpt_get_part_size_in_bytes(entry);
#else
	struct pit_entry *ptn = (struct pit_entry *)part;

	return pit_get_length(ptn);
#endif
}

u32 part_get_start_in_blks(void *part)
{
#if INPUT_GPT_AS_PT
	PART_ENTRY *entry = (PART_ENTRY *)part;

	return gpt_get_part_start_in_blks(entry);
#else
	struct pit_entry *ptn = (struct pit_entry *)part;

	return ptn->blkstart / (UFS_BSIZE / MMC_BSIZE);
#endif
}

u32 part_get_start_in_secs(void *part)
{
#if INPUT_GPT_AS_PT
	PART_ENTRY *entry = (PART_ENTRY *)part;

	return gpt_get_part_start_in_secs(entry);
#else
	struct pit_entry *ptn = (struct pit_entry *)part;

	return ptn->blkstart;
#endif
}

const char *part_get_fs_type(void *part)
{
	const char *str;
#if INPUT_GPT_AS_PT
	PART_ENTRY *entry = (PART_ENTRY *)part;

	str = gpt_get_fs_type(entry);
#else
	struct pit_entry *ptn = (struct pit_entry *)part;

	str = pit_fs_type_tokens[ptn->filesys];
#endif
	return str;
}

int part_get_unique_guid(void *part, char *name, char *buf)
{
#if INPUT_GPT_AS_PT
	return gpt_get_part_unique_guid(part, buf);
#else
	return get_unique_guid(name, buf);
#endif
}

static void part_print_raw(void *buf, u64 size)
{
#ifdef PART_DEBUG
	u8 *p = (u8 *)buf;
	u8 i;

	printf("%s: %llu\n", __func__, size);
	for (i = 0 ; i < 0x30; i++) {
		if (i % 0x10 == 0)
			printf("\n");
		printf("%02x ", p[i]);
	}
#endif
}

void part_write(void *part, void *buf)
{
#if INPUT_GPT_AS_PT
	PART_ENTRY *entry = (PART_ENTRY *)part;

	gpt_write_part(entry, buf, 0, 0);
#else
	struct pit_entry *ptn = (struct pit_entry *)part;

	pit_access(ptn, PIT_OP_FLASH, (u64)buf, 0);
#endif
}

int part_write_partial(void *part, void *buf, u64 offset, u64 size)
{
	int ret;

#if INPUT_GPT_AS_PT
	PART_ENTRY *entry = (PART_ENTRY *)part;

	ret = gpt_write_part(entry, buf, offset, size);
#else
	struct pit_entry *ptn = (struct pit_entry *)part;

	ret = pit_entry_write(ptn, buf, (u64)offset, size);
#endif
	return ret;
}

int part_read(void *part, void *buf)
{
	part_print_raw(buf, 0);
#if INPUT_GPT_AS_PT
	PART_ENTRY *entry = (PART_ENTRY *)part;

	return gpt_read_part(entry, buf, 0, 0);
#else
	struct pit_entry *ptn = (struct pit_entry *)part;

	return pit_access(ptn, PIT_OP_LOAD, (u64)buf, 0);
#endif
}

int part_read_partial(void *part, void *buf, u64 offset, u64 size)
{
	int ret;
#if INPUT_GPT_AS_PT
	PART_ENTRY *entry = (PART_ENTRY *)part;

	ret = gpt_read_part(entry, buf, offset, size);
#else
	struct pit_entry *ptn = (struct pit_entry *)part;

	ret = pit_entry_read(ptn, buf, (u64)offset, size);
#endif
	return ret;
}

int part_erase(void *part)
{
#if INPUT_GPT_AS_PT
	PART_ENTRY *entry = (PART_ENTRY *)part;

	return gpt_erase_part(entry, 0, 0);
#else
	struct pit_entry *ptn = (struct pit_entry *)part;

	return pit_access(ptn, PIT_OP_ERASE, 0, 0);
#endif
}

int part_get_pt_type(const char *name)
{
#if INPUT_GPT_AS_PT
	return !strcmp(name, "gpt");
#else
	return !strcmp(name, "pit");
#endif
}

void part_update(void *addr, u32 size)
{
#if INPUT_GPT_AS_PT
	gpt_update((u8 *)addr);
#else
	pit_update(addr, size);
#endif
}

void part_show(void)
{
#if INPUT_GPT_AS_PT
	gpt_show_info();
#else
	pit_show_info();
#endif
}

void part_set_def_dev(enum __boot_dev_id id)
{
	s_def_boot_dev_id = id;
}

enum __boot_dev_id part_get_dev(void)
{
	return ((s_def_boot_dev_id != 0xFFFFFFFF) ? s_boot_dev_id : 0xFFFFFFFF);
}

/* It's called once at boot time */
int part_open_dev_once(void)
{
	int res = -1;
	char str[10];
	unsigned int len;

	if (s_boot_dev_id == DEV_NONE) {
		printf("%s: block dev not set\n", __func__);
		print_lcd(FONT_RED, FONT_BLACK,
				"%s: block dev not set\n", __func__);
	} else {
		len = strlen(part_dev_tokens[s_boot_dev_id]);
		memcpy(str, part_dev_tokens[s_boot_dev_id], len);

		/* Current assumption is that BL1 exists in boot A */
		str[len] = '0';
		str[len + 1] = '\0';

		s_bdev = bio_open(str);
		if (!s_bdev) {
			printf("%s: fail to open %s\n", __func__, str);
			print_lcd(FONT_RED, FONT_BLACK,
					"%s: fail to open %s\n", __func__, str);
		} else {
			res = 0;
			bio_close(s_bdev);
		}
	}

	return res;
}

void part_init(void)
{
	unsigned int boot_dev;
	enum __boot_dev_id id;

	/*
	 * UFS by default because many projects have used it
	 * In MMC case, you need to call part_set_def_dev(DEV_MMC)
	 */
	if (s_def_boot_dev_id == 0xFFFFFFFF)
		s_def_boot_dev_id = DEV_UFS;

	boot_dev = get_boot_device();
	printf("got boot device\n");
	/*
	 * In USB 1st boot cases, part init needs to executed
	 * based on SW intention.
	 */
	id = DEV_UFS;
	s_boot_dev_id = id;

	/* Get block device information */
	printf("finished\n");
	part_open_dev_once();
#if INPUT_GPT_AS_PT
	gpt_init(id);
#else
	pit_init(id);
#endif
}

int part_read_raw(void *addr, u32 start_in_secs, u32 *size_in_secs)
{
	int res = 0;
#if INPUT_GPT_AS_PT
	res = gpt_read_raw(addr, start_in_secs, size_in_secs);
#else
	/* Not supported */
#endif
	return res;
}

void part_get_range_by_name(const char *name, u32 *start_in_secs, u32 *size_in_secs)
{
#if INPUT_GPT_AS_PT
	gpt_get_range_by_name(name, start_in_secs, size_in_secs);
#else
	/* Not supported */
#endif
}

void part_get_range_by_range(u32 *start_in_secs, u32 *size_in_secs)
{
#if INPUT_GPT_AS_PT
	gpt_get_range_by_range(start_in_secs, size_in_secs);
#else
	/* Not supported */
#endif
}

u32 part_get_lun(void *part)
{
#if INPUT_GPT_AS_PT
	return gpt_get_lun(part);
#else
	/* Not supported */
	return 0xFFFFFFFF;
#endif
}

int part_wipe_boot(void)
{
	char str[10];
	bdev_t *dev;
	int res = -1;
	unsigned int len;

	if (s_boot_dev_id == DEV_NONE) {
		printf("%s: block dev not set\n", __func__);
		print_lcd(FONT_RED, FONT_BLACK,
				"%s: block dev not set\n", __func__);
	} else {
		len = strlen(part_dev_tokens[s_boot_dev_id]);
		memcpy(str, part_dev_tokens[s_boot_dev_id], len);

		/* Current assumption is that BL1 exists in boot A */
		str[len] = '1';
		str[len + 1] = '\0';

		dev = bio_open(str);
		if (!dev) {
			printf("%s: fail to open %s\n", __func__, str);
			print_lcd(FONT_RED, FONT_BLACK,
					"%s: fail to open %s\n", __func__, str);
		} else {
			res = dev->new_erase_native(dev, 0, 1);
			if (res) {
				printf("%s: fail to wipe %s\n", __func__, str);
				print_lcd(FONT_RED, FONT_BLACK,
						"%s: fail to wipe %s\n", __func__, str);
			}
			bio_close(dev);
		}

		/* Current assumption is that partition table exists in User */
		str[len] = '0';
		str[len + 1] = '\0';

		dev = bio_open(str);
		if (!dev) {
			printf("%s: fail to open %s\n", __func__, str);
			print_lcd(FONT_RED, FONT_BLACK,
					"%s: fail to open %s\n", __func__, str);
		} else {
			res = dev->new_erase_native(dev, 0, 48 * PART_SECTOR_SIZE / dev->block_size);
			if (res) {
				printf("%s: fail to wipe %s\n", __func__, str);
				print_lcd(FONT_RED, FONT_BLACK,
						"%s: fail to wipe %s\n", __func__, str);
			}
			bio_close(dev);
		}
	}

	return res;
}

u32 part_get_block_size()
{
	return (u32)s_bdev->block_size;
}

u32 part_get_erase_size()
{
	return (u32)s_bdev->erase_size;
}
