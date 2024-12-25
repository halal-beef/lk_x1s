/* Copyright (c) 2018 Samsung Electronics Co, Ltd.

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.

 *

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

#include <reg.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h> /* TODO : divide print_lcd function */

#include "exynos_font.h"
/*
#include <target/dpu_config.h>
#include <target/lcd_module.h>
*/

typedef unsigned char u8;
typedef unsigned int u32;

static u32 y_pos = 0;

#define MAX_NUM_CHAR_PER_LINE		(LCD_WIDTH / (FONT_X + 1))
#define ALPHANUMERIC_OFFSET		32
#define LENGTH_OF_A_CHAR_ARRAY		((FONT_Y) * 2)
#define FONT_PTR_BIT			(((FONT_X) / 2) - 1)

/* Panel specific. If this bootloader would be useful this would be dynamic. */
#define CONFIG_DISPLAY_FONT_BASE_ADDRESS 0xcc000000
#define LCD_WIDTH 1440
#define LCD_HEIGHT 2960

#define LK3RD_USER
#ifdef LK3RD_USER

/* Fill the frame buffer one character at a time */
static int fill_fb_one_char(u32 *fb_buf, u32 x_pos, u32 fb_width, char ascii,
		u32 y_pos, u32 font_color, u32 bg_color)
{
	int i, j;
	u32 offset; /* Offset of font array, exynos_font.h */
	u32 *fb_ptr;

	/* From Null(0x00) to '~'(0x7E) */
	if (ascii < 32 || ascii > 126)
		return -1;

	offset = LENGTH_OF_A_CHAR_ARRAY * (ascii - ALPHANUMERIC_OFFSET);

	for (i = 0; i < FONT_Y; i++) {
		/* Move to fill next or start pixel of fb */
		fb_ptr = fb_buf + ((i + y_pos) * fb_width) + x_pos;

		/* Fill a first half part of a font width, 8bit */
		for (j = 0; j < (FONT_X / 2); j++) {
			if (font[offset] & (1 << j)) {
				/* Filled area in font */
				fb_ptr[(FONT_X / 2 - 1) - j] = font_color;
			} else {
				/* Unfilled area in font */
				fb_ptr[(FONT_X / 2 - 1) - j] = bg_color;
			}
		}
		/* Move to next (FONT / 2) pixel pointer of fb */
		fb_ptr = fb_ptr + (FONT_X / 2);
		/* Move to next (FONT / 2) pixel pointer of font */
		offset++;
		/* Fill the other half part of a font width, 8bit */
		for (j = 0; j < (FONT_X / 2); j++) {
			if (font[offset] & (1 << j)) {
				/* Filled area in font */
				fb_ptr[(FONT_X / 2 - 1) - j] = font_color;
			} else {
				/* Unfilled area in font */
				fb_ptr[(FONT_X / 2 - 1) - j] = bg_color;
			}
		}

		/* Move to next (FONT / 2) pixel pointer of font */
		offset++;
	}

	return 0;
}

#else
static int fill_fb_one_char(u32 *fb_buf, u32 x_pos, u32 fb_width, char ascii,
		u32 y_pos, u32 font_color, u32 bg_color)
{
	return 0;
}
#endif


static void initialize_font_fb(void)
{
	memset((void *)CONFIG_DISPLAY_FONT_BASE_ADDRESS, 0, LCD_WIDTH * LCD_HEIGHT * 4);
}

/* Fill one line of the frame buffer with characters */
static int _fill_fb_string(u32 *fb_buf, u32 x_pos, u8 *str,
		u32 font_color, u32 bg_color, int lgth)
{
	int i = 0;
	int cnt = 0;
	char ch = 0;

	if (lgth > MAX_NUM_CHAR_PER_LINE)
		cnt = MAX_NUM_CHAR_PER_LINE;
	else
		cnt = lgth;

	if (y_pos > LCD_HEIGHT) {
		/* Rolling fb, y_pos and fb address reinit */
		y_pos = 0;
		fb_buf = (u32 *)CONFIG_DISPLAY_FONT_BASE_ADDRESS;
		initialize_font_fb();
	}

	for (i = 0; i < cnt; i++) {
		ch = *(str++);
		if (fill_fb_one_char(fb_buf, x_pos + (i * FONT_X), LCD_WIDTH,
			ch, y_pos, font_color, bg_color)) {
			printf("This(%c) character is not supported\n", ch);
		}
	}

	y_pos += FONT_Y;
	lgth = lgth - MAX_NUM_CHAR_PER_LINE;

	return lgth;
}

/* Fill a frame buffer with characters */
int fill_fb_string(u32 *fb_buf, u32 x_pos, u8 *str, u32 font_color, u32 bg_color)
{
	int lgth = 0;

	if (!str)
		return 1;
	else
		lgth = strlen((char *)str);

	do {
		lgth = _fill_fb_string(fb_buf, x_pos, str,
					font_color, bg_color, lgth);
		if (lgth) {
			/* for updating the position of the remaining
			 * string points after filling one line of the LCD.
			 */
			str = str + MAX_NUM_CHAR_PER_LINE;
		}
	} while (lgth > 0);

	return 0;
}

#define PRINT_BUF_SIZE 384
#define TOP_MARGIN	40
extern u32 _win_fb0 = CONFIG_DISPLAY_FONT_BASE_ADDRESS;
extern void decon_string_update(void);

int print_lcd(u32 font_color, u32 bg_color, const char *fmt, ...)
{
	va_list args;
	char printbuffer[PRINT_BUF_SIZE];
	u64 ptr = _win_fb0;
	va_start(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	vsnprintf(printbuffer, sizeof(printbuffer), fmt, args);
	va_end(args);

	if (fill_fb_string((u32 *)ptr, TOP_MARGIN,
				(u8 *)printbuffer, font_color, bg_color)) {
		printf("failed to print on lcd\n");
		return -1;
	}

	return 0;
}

int print_lcd_update(u32 font_color, u32 bg_color, const char *fmt, ...)
{
	va_list args;
	char printbuffer[PRINT_BUF_SIZE];
	u64 ptr = _win_fb0;
	va_start(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	vsnprintf(printbuffer, sizeof(printbuffer), fmt, args);
	va_end(args);

	if (fill_fb_string((u32 *)ptr, TOP_MARGIN,
				(u8 *)printbuffer, font_color, bg_color)) {
		printf("failed to print on lcd\n");
		return -1;
	}

	// decon_string_update();

	return 0;
}
