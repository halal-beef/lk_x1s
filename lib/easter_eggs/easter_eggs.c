/* SPDX-License-Identifier: MIT */
/*
 * Copyright (c) 2024, Umer Uddin <umer.uddin@mentallysanemainliners.org>
 * Copyright (c) 2024, Igor Belwon <igor.belwon@mentallysanemainliners.org>
 */

#include <lib/font_display.h>

/* Show a fake unlock warning */
void show_dummy_unlock_warning(void)
{
	print_lcd_update(FONT_ORANGE, FONT_BLACK, "");
	print_lcd_update(FONT_ORANGE, FONT_BLACK, "");
	print_lcd_update(FONT_ORANGE, FONT_BLACK, "");
	print_lcd_update(FONT_ORANGE, FONT_BLACK, "");
	print_lcd_update(FONT_ORANGE, FONT_BLACK, "<!>");
	print_lcd_update(FONT_ORANGE, FONT_BLACK, "");
	print_lcd_update(FONT_WHITE, FONT_BLACK, "The boot loader is unlocked and software");
	print_lcd_update(FONT_WHITE, FONT_BLACK, "integrity cannot be guaranteed. Any data");
	print_lcd_update(FONT_WHITE, FONT_BLACK, "stored on this device may be available to");
	print_lcd_update(FONT_WHITE, FONT_BLACK, "attackers. Do not store any sensitive data");
	print_lcd_update(FONT_WHITE, FONT_BLACK, "on this device.");
	print_lcd_update(FONT_WHITE, FONT_BLACK, "");
	print_lcd_update(FONT_WHITE, FONT_BLACK, "Visit this link on another device:");
	print_lcd_update(FONT_ORANGE, FONT_BLACK, "youtu.be/xvFZjo5PgG0");
	print_lcd_update(FONT_WHITE, FONT_BLACK, "");
	print_lcd_update(FONT_WHITE, FONT_BLACK, "Loading lk3rd. Be patient.");
}