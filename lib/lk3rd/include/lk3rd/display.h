#ifndef DISPLAY
#define DISPLAY

#include <lib/font_display.h>

#include "fastboot_menu.h"

#define FONT_X 16
#define MAX_NUM_CHAR_PER_LINE ((LCD_WIDTH / FONT_X) - 6)

void draw_menu(enum action current_action);

#endif /* DISPLAY */
