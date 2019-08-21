/* Copyright (c) 2018 Samsung Electronics Co, Ltd.

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.

 * Copyright@ Samsung Electronics Co. LTD
 * Manseok Kim <manseoks.kim@samsung.com>

 * Alternatively, Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <malloc.h>
#include <dpu/decon.h>
#include <target/dpu_config.h>

//#ifdef CONFIG_PIT
//#include <pit.h>
//#endif
#include <part.h>

#define BMP_HEADER_SIZE 54

int decon_resize_align(unsigned int xsize, unsigned int ysize);
int show_boot_logo(void)
{
#ifdef CONFIG_PIT
	unsigned char *file = (unsigned char *)(CONFIG_DISPLAY_TEMP_BASE_ADDRESS);
	unsigned int *fb = (unsigned int *)(CONFIG_DISPLAY_LOGO_BASE_ADDRESS);
	void *part = part_get("logo");
	u64 aligned_read_size = 0;

	unsigned int i;

	/* bmp header */
	unsigned short type;
	unsigned int file_size;
	unsigned int img_width;
	unsigned int img_height;
	unsigned short bpp;

	if (!part)
		goto error;
	part_read_partial(part, (void *)file, 0, (u64)512);

	type = ((unsigned short)*(file + 1) << 8) | (unsigned short)(*file);
	printf("type : 0x%04x\n", type);

	if (type == 0x4d42) {	// BMP file

		unsigned int aligned_read_size = 0;

		file += 2;

		file_size = ((unsigned int)*(file + 3) << 24) | ((unsigned int)*(file + 2) << 16) | ((unsigned int)*(file + 1) << 8) | (unsigned int)(*file);
		file += 4;
		file += 12;

		img_width = ((unsigned int)*(file + 3) << 24) | ((unsigned int)*(file + 2) << 16) | ((unsigned int)*(file + 1) << 8) | (unsigned int)(*file);
		file += 4;

		img_height = ((unsigned int)*(file + 3) << 24) | ((unsigned int)*(file + 2) << 16) | ((unsigned int)*(file + 1) << 8) | (unsigned int)(*file);
		file += 4;
		file += 2;

		bpp = ((unsigned short)*(file + 1) << 8) | (unsigned short)(*file);

		printf("file size : %d (%d)\n", file_size, img_width * img_height * 3 + BMP_HEADER_SIZE);
		printf("image width : %d\n", img_width);
		printf("image height : %d\n", img_height);
		printf("bpp : %d\n", (unsigned int)bpp);

		if (bpp != 24) {
			printf(" bmp file should be 24 bpp. (%d)\n", bpp);
			goto error;
		}
		aligned_read_size = (((img_width * img_height * 3 + BMP_HEADER_SIZE) / 1024) + 1) * 1024;

		file = (unsigned char *)(CONFIG_DISPLAY_TEMP_BASE_ADDRESS);
		aligned_read_size = ((aligned_read_size + PART_SECTOR_SIZE - 1) / PART_SECTOR_SIZE) * PART_SECTOR_SIZE;
		part_read_partial(part, (void *)file, 0, (u64)aligned_read_size);

		file+=BMP_HEADER_SIZE;

		for (i = 0; i < img_width * img_height * 3; i += 24) {
			*fb++ = (0xff << 24)|(*(file + i +  2) << 16)|(*(file + i +  1) << 8)|*(file + i);
			*fb++ = (0xff << 24)|(*(file + i +  5) << 16)|(*(file + i +  4) << 8)|*(file + i +  3);
			*fb++ = (0xff << 24)|(*(file + i +  8) << 16)|(*(file + i +  7) << 8)|*(file + i +  6);
			*fb++ = (0xff << 24)|(*(file + i + 11) << 16)|(*(file + i + 10) << 8)|*(file + i +  9);

			*fb++ = (0xff << 24)|(*(file + i + 14) << 16)|(*(file + i + 13) << 8)|*(file + i + 12);
			*fb++ = (0xff << 24)|(*(file + i + 17) << 16)|(*(file + i + 16) << 8)|*(file + i + 15);
			*fb++ = (0xff << 24)|(*(file + i + 20) << 16)|(*(file + i + 19) << 8)|*(file + i + 18);
			*fb++ = (0xff << 24)|(*(file + i + 23) << 16)|(*(file + i + 22) << 8)|*(file + i + 21);
		}

		decon_resize_align(img_width, img_height);

	} else {
		// RGB raw file
		aligned_read_size = (u64)(LCD_WIDTH * LCD_HEIGHT * 4);
		aligned_read_size = ((aligned_read_size + PART_SECTOR_SIZE - 1) / PART_SECTOR_SIZE) * PART_SECTOR_SIZE;
		part_read_partial(part, (void *)fb, 0, aligned_read_size);
	}

	decon_string_update();

	return 0;

error:
	return -1;
#else
	return 0;
#endif
}
