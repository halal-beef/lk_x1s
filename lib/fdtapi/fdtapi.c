/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
#include <string.h>
#include <stdlib.h>
#include <types.h>
#include <lib/console.h>
#include <lib/fdtapi.h>
#include <ufdt_overlay.h>
#include <lib/font_display.h>

#define BUFFER_SIZE	4096
#define MASK_4GB	(0xFFFFFFFF)

struct fdt_header *fdt_dtb;
struct dt_table_header *dtbo_table;
struct fdt_header *fdt_dpm;
int dtbo_idx = -EINVAL;

/*
 * function dependency : to get exact dtbo_idx value,
 * get_selected_dtbo_idx function must be referenced after merge_dto_to_main_dtb function
 */
int get_selected_dtbo_idx(void)
{
	if (dtbo_idx == -EINVAL)
		printf("%s: dtbo_idx is not initialized (%d)", __func__, dtbo_idx);

	return dtbo_idx;
}

int merge_dto_to_main_dtb(unsigned int board_id, unsigned int board_rev)
{
	void *fdto, *merged_fdt;
	struct dt_table_entry *dt_entry;
	u32 i;
	int ret;

	ret = fdt_check_header(fdt_dtb);
	if (ret < 0) {
		printf("DTBO: fdt_check_header(): %s\n", fdt_strerror(ret));
		return -EINVAL;
	}

	if (fdt32_to_cpu(dtbo_table->magic) != DT_TABLE_MAGIC) {
		printf("DTBO: dtbo.img: %s\n", fdt_strerror(-FDT_ERR_BADMAGIC));
		return -EINVAL;
	}
	dt_entry = (struct dt_table_entry *)((unsigned long)dtbo_table
	                                     + fdt32_to_cpu(dtbo_table->header_size));

	for (i = 0; i < fdt32_to_cpu(dtbo_table->dt_entry_count); i++, dt_entry++) {
		u32 id = fdt32_to_cpu(dt_entry->id);
		u32 rev = fdt32_to_cpu(dt_entry->rev);

		if((fdt32_to_cpu(dt_entry->custom[0]) <= board_rev && fdt32_to_cpu(dt_entry->custom[1]) >= board_rev) ||
		   (dt_entry->custom[0] <= board_rev && dt_entry->custom[1] >= board_rev)) {
			dtbo_idx = i;
			printf("DTBO: id: 0x%x, rev: 0x%x, dtbo_idx: %d\n", id, rev, dtbo_idx);
			//print_lcd_update(FONT_YELLOW, FONT_BLACK,
			//       "DTBO: id: 0x%x, rev: 0x%x, dtbo_idx: %d", id, rev, dtbo_idx);
			break;
		}
	}

	if (i == fdt32_to_cpu(dtbo_table->dt_entry_count)) {
		printf("DTBO: Not found dtbo of board_rev 0x%x.\n", board_rev);
		print_lcd_update(FONT_RED, FONT_BLACK, "DTBO: Not found dtbo of board_rev 0x%x.\n", board_rev);
		return -EINVAL;
	}

	fdto = malloc(fdt32_to_cpu(dt_entry->dt_size));
	memcpy((void *)fdto, (void *)((unsigned long)dtbo_table + fdt32_to_cpu(dt_entry->dt_offset)),
	       fdt32_to_cpu(dt_entry->dt_size));

	ret = fdt_check_header(fdto);
	if (ret < 0) {
		printf("DTBO: overlay dtbo: %s", fdt_strerror(ret));
		goto fdto_magic_err;
	}

	merged_fdt = ufdt_apply_overlay(fdt_dtb, fdt_totalsize(fdt_dtb),
	                                fdto, fdt_totalsize(fdto));
	if (!merged_fdt)
		goto fdto_magic_err;

	memcpy(fdt_dtb, merged_fdt, fdt_totalsize(merged_fdt));
	printf("DTBO: Merge Complete (size:%d)!\n", fdt_totalsize(fdt_dtb));

	free(merged_fdt);

	/* merge dpm dt */
	ret = fdt_check_header(fdt_dpm);
	if (ret >= 0) {
		merged_fdt = ufdt_apply_overlay(fdt_dtb, fdt_totalsize(fdt_dtb),
				fdt_dpm, fdt_totalsize(fdt_dpm));
		if (!merged_fdt)
			goto fdto_magic_err;

		memcpy(fdt_dtb, merged_fdt, fdt_totalsize(merged_fdt));
		printf("DPM: Merge Complete (size%d)!\n", fdt_totalsize(fdt_dtb));
		free(merged_fdt);
	} else {
		printf("DPM: dpm dtb: does not exist.(%s)\n", fdt_strerror(ret));
	}
fdto_magic_err:
	free(fdto);
	return -EINVAL;
}

int resize_dt(unsigned int sz)
{
	uint64_t size;
	int ret = 0;

	ret = fdt_check_header(fdt_dtb);
	if (ret) {
		printf("libfdt fdt_check_header(): %s\n", fdt_strerror(ret));
		return ret;
	}

	size = fdt_totalsize(fdt_dtb) + sz;
	/* Align for 4byte. */
	size = ALIGN(size, 0x4);
	fdt_set_totalsize(fdt_dtb, size);

	return size;
}

int make_fdt_node(const char *path, char *node)
{
	int offset;
	int ret;

	ret = fdt_check_header(fdt_dtb);
	if (ret) {
		printf("libfdt fdt_check_header(): %s\n", fdt_strerror(ret));
		return ret;
	}

	offset = fdt_path_offset(fdt_dtb, path);
	if (offset < 0) {
		printf("libfdt fdt_path_offset(): %s\n", fdt_strerror(offset));
		return 1;
	}

	ret = fdt_add_subnode(fdt_dtb, offset, node);
	if (ret < 0) {
		printf("libfdt fdt_add_subnode(): %s\n", fdt_strerror(ret));
		return 1;
	}

	return 0;
}

int get_fdt_val(const char *path, const char *property, char *retval)
{
	const char *np;
	int offset;
	int len, ret = 0;

	ret = fdt_check_header(fdt_dtb);
	if (ret) {
		printf("libfdt fdt_check_header(): %s\n", fdt_strerror(ret));
		return 1;
	}

	offset = fdt_path_offset(fdt_dtb, path);
	if (offset < 0) {
		printf("libfdt fdt_path_offset(): %s\n", fdt_strerror(offset));
		return 1;
	}
	np = fdt_getprop(fdt_dtb, offset, property, &len);
	if (len <= 0) {
		printf("libfdt fdt_getprop(): %s\n", fdt_strerror(len));
		return 1;
	}
	memcpy(retval, np, len);
	return 0;
}

int set_fdt_val(const char *path, const char *property, const char *value)
{
	char parsed_value[BUFFER_SIZE];
	int offset;
	int len = 0;
	int ret;
	const char *tp;
	const char *np;
	unsigned long tmp;
	char *dp = parsed_value;

	np = value;

	if (*np == '<') {
		np++;
		while (*np != '>') {
			tp = np;
			tmp = strtoul(tp, (char **)&np, 0);
			*(uint32_t *)dp = cpu_to_be32(tmp);
			dp  += 4;
			len += 4;

			while (*np == ' ')
				np++;
		}
	} else {
		strcpy(parsed_value, value);
		len += strlen(value) + 1;
	}

	ret = fdt_check_header(fdt_dtb);
	if (ret) {
		printf("libfdt fdt_check_header(): %s\n", fdt_strerror(ret));
		return ret;
	}

	offset = fdt_path_offset(fdt_dtb, path);
	if (offset < 0) {
		printf("libfdt fdt_path_offset(): %s\n", fdt_strerror(offset));
		return 1;
	}

	ret = fdt_setprop(fdt_dtb, offset, property, parsed_value, len);
	if (ret < 0) {
		printf("libfdt fdt_setprop(): %s\n", fdt_strerror(ret));
		return 1;
	}

	return 0;
}

void add_dt_memory_node(unsigned long base, unsigned int size)
{
	char str[BUFFER_SIZE];
	char str2[BUFFER_SIZE];
	unsigned int ubase = 0;
	unsigned int lbase = 0;

	ubase = base >> 32;
	lbase = base & MASK_4GB;

	sprintf(str, "memory@%lx", base);
	make_fdt_node("/", str);

	sprintf(str, "/memory@%lx", base);
	sprintf(str2, "<0x%x 0x%x 0x%x>", ubase, lbase, size);
	set_fdt_val(str, "reg", str2);

	sprintf(str, "/memory@%lx", base);
	set_fdt_val(str, "device_type", "memory");
}

int get_fdt_dpm_val(const char *path, const char *property, char *retval)
{
	const char *np;
	int  offset;
	int  len, ret = 0;

	ret = fdt_check_header(fdt_dpm);
	if (ret) {
		printf("libfdt fdt_check_header(): %s\n", fdt_strerror(ret));
		return 1;
	}

	offset = fdt_path_offset(fdt_dpm, path);
	if (offset < 0) {
		printf("libfdt fdt_path_offset(): %s\n", fdt_strerror(offset));
		return 1;
	}
	np = fdt_getprop(fdt_dpm, offset, property, &len);
	if (len <= 0) {
		printf("libfdt fdt_getprop(): %s\n", fdt_strerror(len));
		return 1;
	}
	memcpy(retval, np, len);
	return 0;
}
