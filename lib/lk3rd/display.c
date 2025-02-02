/*
 * Copyright (c) 2024 Igor Belwon <igor.belwon@mentallysanemainliners.org>
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 *
 */
#include <lib/font_display.h>
#include <lib/version.h>
#include <stdlib.h>
#include <string.h>

#include "include/lk3rd/display.h"
#include "include/lk3rd/fastboot_menu.h"

void draw_line_lcd(int color_fg, int color_bg)
{
	char *str = malloc(MAX_NUM_CHAR_PER_LINE + 1);
	memset(str, '-', MAX_NUM_CHAR_PER_LINE);
	str[MAX_NUM_CHAR_PER_LINE] = '\0';
	print_lcd_update(color_fg, color_bg, str);
}

void draw_current_action(enum action current_action)
{
	switch(current_action)
	{
		case ACTION_START:
			draw_line_lcd(FONT_GREEN, FONT_BLACK);
			print_lcd_update(FONT_GREEN, FONT_BLACK, "START");
			draw_line_lcd(FONT_GREEN, FONT_BLACK);
			break;

		case ACTION_REBOOT_RECOVERY:
			draw_line_lcd(FONT_YELLOW, FONT_BLACK);
			print_lcd_update(FONT_YELLOW, FONT_BLACK, "Reboot recovery");
			draw_line_lcd(FONT_YELLOW, FONT_BLACK);
			break;

		case ACTION_REBOOT_BOOTLOADER:
			draw_line_lcd(FONT_RED, FONT_BLACK);
			print_lcd_update(FONT_RED, FONT_BLACK, "Reboot bootloader");
			draw_line_lcd(FONT_RED, FONT_BLACK);
			break;

		case ACTION_REBOOT_FASTBOOTD:
			draw_line_lcd(FONT_ORANGE, FONT_BLACK);
			print_lcd_update(FONT_ORANGE, FONT_BLACK, "Reboot FastbootD");
			draw_line_lcd(FONT_ORANGE, FONT_BLACK);
			break;

		case ACTION_REBOOT_DOWNLOAD:
			draw_line_lcd(FONT_BLUE, FONT_BLACK);
			print_lcd_update(FONT_BLUE, FONT_BLACK, "Reboot Download");
			draw_line_lcd(FONT_BLUE, FONT_BLACK);
			break;

		case ACTION_POWEROFF:
			draw_line_lcd(FONT_RED, FONT_BLACK);
			print_lcd_update(FONT_RED, FONT_BLACK, "Power off");
			draw_line_lcd(FONT_RED, FONT_BLACK);
			break;

		default:
			break;
	}
}

void draw_menu(enum action current_action)
{
	clear_screen(FONT_BLACK);

#if defined(CONFIG_HAS_CURVED_DISPLAY)
	// Offset for curved displays
	print_lcd_update(FONT_WHITE, FONT_BLACK, "");
	print_lcd_update(FONT_WHITE, FONT_BLACK, "");
	print_lcd_update(FONT_WHITE, FONT_BLACK, "");
#endif

	draw_current_action(current_action);
	print_lcd_update(FONT_WHITE, FONT_BLACK, "");
	print_lcd_update(FONT_WHITE, FONT_BLACK, "Press volume key to select, and press power key to select");
	print_lcd_update(FONT_WHITE, FONT_BLACK, "");
	print_lcd_update(FONT_RED,   FONT_BLACK, "lk3rd FastBoot Mode", MAX_NUM_CHAR_PER_LINE);
	print_lcd_update(FONT_WHITE, FONT_BLACK, "PRODUCT_NAME - %s", version.platform);
	print_lcd_update(FONT_WHITE, FONT_BLACK, "BOOTLOADER VERSION - DEV (%s)", version.buildid);
	print_lcd_update(FONT_RED,   FONT_BLACK, "DEVICE STATE - unlocked");
}
