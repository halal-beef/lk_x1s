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
#include <types.h>
#include <stdint.h>

#define FB_BASE ((u32 *)0xca000000)
#define FB_WIDTH 1440
#define FB_HEIGHT 3040

volatile u32 *fb = FB_BASE;

/* Clears the framebuffer by filling it with a specified color */
void clear_screen(uint32_t color);

typedef unsigned int uint32_t;
typedef unsigned char uint8_t;

#include <stdint.h>
#include <kernel/thread.h>
#include "exynos_font.h"

#define LCD_WIDTH 1440
#define FB_HEIGHT 3040

// Global cursor position and color.
static u32 cursor_x = 10;
static u32 cursor_y = 0;
static u32 text_color = 0xFFFFFF; // Default white color.

// Helper function to draw a character.
void putc_fb(char c)
{
    // Address of framebuffer (assuming RGB format and 32-bit pixels).
    volatile u32 *fb = FB_BASE;

    // Handle newline.
    if (c == '\n')
    {
        cursor_x = 10;
        cursor_y += FONT_Y;
        if (cursor_y + 3*FONT_Y > LCD_HEIGHT)
        {
            clear_screen(0); // Clear the screen.
            cursor_y = 0; // Reset to the top if we overflow.
        }
        return;
    }

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
                u32 pixel_x = cursor_x + col;
                u32 pixel_y = cursor_y + row;
                if (pixel_x < LCD_WIDTH && pixel_y < LCD_HEIGHT)
                {
                    fb[pixel_y * LCD_WIDTH + pixel_x] = text_color;
                }
            }
        }
    }
    // Move cursor to the next character position.
    cursor_x += FONT_X;
    if (cursor_x + FONT_X > LCD_WIDTH)
    { // Handle line wrap.
        cursor_x = 0;
        cursor_y += FONT_Y;

        if (cursor_y + FONT_Y > LCD_HEIGHT)
        {
            cursor_y = 10; // Reset to the top if we overflow.
        }
    }
}

void platform_dputc(char c)
{
    putc_fb(c);
}

int platform_dgetc(char *c, bool wait)
{
    return 0;
}
