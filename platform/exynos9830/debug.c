/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 *
 * No part of this software, either material or conceptual may be copied or
 * distributed, transmitted, transcribed, stored in a retrieval system or
 * translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed to third parties
 * without the express written permission of Samsung Electronics.
 *
 */
#include <platform/debug.h>
#include <platform/uart.h>

/* Temporary debug shit. TODO */

#define FB_BASE ((u32 *)0xf1000000)
#define FB_WIDTH 1080
#define FB_HEIGHT 2400

typedef unsigned int uint32_t;
typedef unsigned char uint8_t;

#include <stdint.h>
#include "../../lib/font/exynos_font.h"

volatile u32 *fb = FB_BASE;

// Helper function to draw a character.
void putc_fb(u32 x, u32 y, char c, u32 color)
{
	// Address of framebuffer (assuming RGB format and 32-bit pixels).
	volatile u32 *fb = FB_BASE;

	// Get the font data for the character.
	const u8 *char_data = &font[(c - ' ') * FONT_Y * 2];

	// Iterate through each pixel of the character.
	for (u32 row = 0; row < FONT_Y; ++row)
	{
		// Each row in the font is 2 bytes (16 bits).
		u16 row_data = (char_data[row * 2] << 8) | char_data[row * 2 + 1];

		for (u32 col = 0; col < FONT_X; ++col)
		{
			// Check if the bit at position `col` is set.
			if (row_data & (1 << (15 - col)))
			{
				// Draw the pixel at the corresponding framebuffer location.
				fb[(y + row) * 1024 + (x + col)] = color;
			}
		}
	}
}

// Helper function to draw a string to the framebuffer.
void puts_fb(u32 x, u32 y, const char *str, u32 color)
{
	while (*str)
	{
		putc_fb(x, y, *str, color);
		x += FONT_X; // Move to the next character position.
		++str;
	}
}

void platform_dputc(char c)
{
	// putc(c);
}

/* End TODO */

int platform_dgetc(char *c, bool wait)
{
	return 0;
}
