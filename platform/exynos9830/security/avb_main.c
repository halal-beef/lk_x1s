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

#include <lk/debug.h>
#include <platform/secure_boot.h>
#include <platform/sfr.h>
#include <dev/rpmb.h>
#include <string.h>
#include <lib/font_display.h>

#if defined(CONFIG_AVB_LCD_LOG)
void avb_print_lcd(const char *str, char *color);
#else
void avb_print_lcd(const char *str, char *color) {};
#endif

#if defined(CONFIG_USE_AVB20)

void troll(void)
{
#if defined(CONFIG_HAS_CURVED_DISPLAY)
	// Offset for curved displays
        print_lcd_update(FONT_ORANGE, FONT_BLACK, "");
        print_lcd_update(FONT_ORANGE, FONT_BLACK, "");
        print_lcd_update(FONT_ORANGE, FONT_BLACK, "");
#endif

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

uint32_t avb_main(const char *suffix, char *cmdline, char *verifiedbootstate)
{
	bool unlock;
	uint32_t ret = 0;
	uint32_t i = 0;
	struct AvbOps *ops;
	const char *partition_arr[] = { "boot", NULL };
	char buf[100];
	char color[AVB_COLOR_MAX_SIZE];
	AvbSlotVerifyData *ctx_ptr = NULL;

	set_avbops();
	get_ops_addr(&ops);
	ops->read_is_device_unlocked(ops, &unlock);

	/* slot verify */
	ret = avb_slot_verify(ops, partition_arr, suffix,
	                      AVB_SLOT_VERIFY_FLAGS_ALLOW_VERIFICATION_ERROR,
	                      AVB_HASHTREE_ERROR_MODE_RESTART_AND_INVALIDATE,
	                      &ctx_ptr);
	/* get color */
	if (unlock) {
		strncpy(color, "orange", AVB_COLOR_MAX_SIZE);
	} else {
		if (ret == AVB_SLOT_VERIFY_RESULT_ERROR_PUBLIC_KEY_REJECTED)
			strncpy(color, "yellow", AVB_COLOR_MAX_SIZE);
		else if (ret)
			strncpy(color, "red", AVB_COLOR_MAX_SIZE);
		else
			strncpy(color, "green", AVB_COLOR_MAX_SIZE);
	}
	if (ret) {
		if (unlock)
		{
			snprintf(buf, 100, "[AVB 2.0 warning] authentication fail [ret: 0x%X] (%s) "
					"No effect on booting process", ret, color);
			//clear_screen(0x000000);
			//troll();
		}
		else
			snprintf(buf, 100, "[AVB 2.0 ERR] authentication fail [ret: 0x%X] (%s)\n", ret, color);
	} else {
		snprintf(buf, 100, "[AVB 2.0] authentication success (%s)\n", color);
	}
	strcat(verifiedbootstate, color);
	printf(buf);
	//avb_print_lcd(buf, color);

	/* AVB color policy */
	if (!strncmp(color, "red", AVB_COLOR_MAX_SIZE))
		__asm__ volatile ("b\t.");

	/* block RPMB */
	ret = block_RPMB_hmac();
	if (ret)
		printf("[AVB 2.0 ERR] RPMB hmac ret: 0x%X\n", ret);

	/* set cmdline */
	if (ctx_ptr != NULL) {
		i = 0;
		while (ctx_ptr->cmdline[i++] != '\0')
			;
		memcpy(cmdline, ctx_ptr->cmdline, i);
	}
#if defined(CONFIG_AVB_DEBUG)
	printf("i: %d\n", i);
	printf("cmdline: %s\n", cmdline);
#endif

	return ret;
}

void avb_print_lcd(const char *str, char *color)
{
	uint32_t font_color;

	switch (color[0]) {
	case 'o':
		font_color = FONT_ORANGE;
		break;
	case 'y':
		font_color = FONT_YELLOW;
		break;
	case 'r':
		font_color = FONT_RED;
		break;
	case 'g':
		font_color = FONT_GREEN;
		break;
	default:
		print_lcd_update(FONT_WHITE, FONT_BLACK,
				"%s : color parsing fail\n", __func__);
		return;
	}
	print_lcd_update(font_color, FONT_BLACK, str);
}
#endif /* if defined(CONFIG_USE_AVB20) */
