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

#ifndef __TD4150_PARAM_H__
#define __TD4150_PARAM_H__

static const unsigned char SEQ_TD4150_B0_OPEN[] = {
	0xB0, 0x04,
};

static const unsigned char SEQ_TD4150_B6[] = {
	0xB6,
	0x30, 0x61, 0x00, 0x06, 0xC3, 0x03,
};

static const unsigned char SEQ_TD4150_B7[] = {
	0xB7,
	0x11, 0x00, 0x00, 0x00,
};

static const unsigned char SEQ_TD4150_B8[] = {
	0xB8,
	0x00, 0x78, 0x64, 0x10, 0x64, 0xB4,
};

static const unsigned char SEQ_TD4150_B9[] = {
	0xB9,
	0x00, 0x78, 0x64, 0x10, 0x64, 0xB4,
};

static const unsigned char SEQ_TD4150_BA[] = {
	0xBA,
	0x00, 0x78, 0x64, 0x10, 0x64, 0xB4,
};

static const unsigned char SEQ_TD4150_BB[] = {
	0xBB,
	0x00, 0xB4, 0xA0,
};

static const unsigned char SEQ_TD4150_BC[] = {
	0xBC,
	0x00, 0xB4, 0xA0,
};

static const unsigned char SEQ_TD4150_BD[] = {
	0xBD,
	0x00, 0xB4, 0xA0,
};

static const unsigned char SEQ_TD4150_BE[] = {
	0xBE,
	0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
};

static const unsigned char SEQ_TD4150_C0[] = {
	0xC0,
	0x00, 0x7F, 0x10, 0x06, 0x40, 0x00, 0x0B, 0x06, 0xB3, 0x00,
	0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
};

static const unsigned char SEQ_TD4150_C1[] = {
	0xC1,
	0x30, 0x11, 0x50, 0xFA, 0x00, 0x00, 0x00, 0x22, 0x00, 0x00,
	0x00, 0x00, 0x40, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00,
};

static const unsigned char SEQ_TD4150_C2[] = {
	0xC2,
	0x00, 0x20, 0x4E, 0x22, 0x05, 0x00, 0x05, 0x12, 0x00, 0x04,
	0x20, 0x0E, 0x22, 0x06, 0x00, 0x05, 0x11, 0x00, 0x03, 0x20,
	0x7C, 0x01, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x4E,
	0x01, 0x05, 0x06, 0x54, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x20, 0x30, 0x08, 0x08, 0x04, 0x05, 0x05,
	0x00, 0x96, 0x50, 0x03, 0x00, 0x10, 0x08, 0x00, 0x00, 0x00,
	0x00, 0x11, 0x00, 0x96, 0x50, 0x08, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x04, 0x20, 0x70, 0x08, 0x08, 0x04,
	0x05, 0x05, 0x00, 0x96, 0x50, 0x03, 0x00, 0x10, 0x08, 0x00,
	0x00, 0x00, 0x00, 0x11, 0x00, 0x96, 0x50, 0x08, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00,
	0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
};

static const unsigned char SEQ_TD4150_C3[] = {
	0xC3,
	0x00, 0x40, 0x01, 0x01, 0x65, 0x90, 0x00, 0x00, 0x00, 0x00,
	0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x23, 0x00, 0x07, 0x10, 0x3B, 0x01, 0x65, 0x10,
	0x0A, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00,
	0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00,
};

static const unsigned char SEQ_TD4150_C4[] = {
	0xC4,
	0x00, 0x57, 0x0B, 0x00, 0x03, 0x04, 0x61, 0x55, 0x1A, 0x19,
	0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x02, 0x02, 0x02, 0x02,
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x55, 0x61,
	0x08, 0x07, 0x00, 0x0B, 0x57, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x80, 0x7F, 0x00, 0xE0, 0x1F, 0x00, 0x00, 0x00, 0x00,
	0xE0, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F,
	0x3F, 0x3F, 0x3F, 0x55, 0x55, 0x55, 0xD5, 0xFF, 0xFF, 0xFF,
	0x57, 0x55, 0x55, 0x55,
};

static const unsigned char SEQ_TD4150_C5[] = {
	0xC5,
	0x08, 0x00, 0x00, 0x00, 0x00,
};

static const unsigned char SEQ_TD4150_C6[] = {
	0xC6,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x04, 0x00,
	0x40, 0x3C, 0x00, 0x00, 0x04, 0x00, 0x40, 0x3C, 0x0F, 0x05,
	0x01, 0x19, 0x01, 0x35, 0xFF, 0x8F, 0x06, 0x05, 0x01, 0xC0,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x20, 0x20, 0x00, 0x00, 0x00, 0xC0, 0x11,
	0x1F, 0x00, 0x00, 0x10, 0x10, 0x00, 0x01, 0xF0, 0x01, 0x00,
	0x50, 0x00, 0x33, 0x03,
};

static const unsigned char SEQ_TD4150_C7[] = {
	0xC7,
	0x00, 0x00, 0x00, 0x52, 0x00, 0x60, 0x00, 0x65, 0x00, 0x79,
	0x00, 0x86, 0x00, 0x9C, 0x00, 0xA7, 0x00, 0xC8, 0x00, 0x9C,
	0x00, 0xFF, 0x00, 0xCE, 0x01, 0x20, 0x00, 0xE7, 0x01, 0x63,
	0x01, 0x83, 0x02, 0x17, 0x02, 0x96, 0x02, 0xA0, 0x00, 0x00,
	0x00, 0x52, 0x00, 0x60, 0x00, 0x65, 0x00, 0x79, 0x00, 0x86,
	0x00, 0x9C, 0x00, 0xA7, 0x00, 0xC8, 0x00, 0x9C, 0x00, 0xFF,
	0x00, 0xCE, 0x01, 0x20, 0x00, 0xE7, 0x01, 0x63, 0x01, 0x83,
	0x02, 0x17, 0x02, 0x96, 0x02, 0xA0,
};

static const unsigned char SEQ_TD4150_C8[] = {
	0xC8,
	0x40, 0x00, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00, 0xFC, 0x00,
	0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00, 0xFC, 0x00,
	0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00, 0xFC,
};

static const unsigned char SEQ_TD4150_C9[] = {
	0xC9,
	0x00, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x24, 0x8B,
	0x4D, 0x8B, 0x4D, 0x8B, 0x4D,
};

static const unsigned char SEQ_TD4150_CA[] = {
	0xCA,
	0x1C, 0xFC, 0xFC, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00,
};


static const unsigned char SEQ_TD4150_CB[] = {
	0xCB,
	0x12, 0xD0, 0x09, 0xC1, 0xB9, 0x65, 0x30, 0xA8, 0x74, 0x21,
	0xB8, 0x75, 0x21, 0xA9, 0x64, 0x30, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x40, 0x70, 0x00, 0x24, 0x00, 0x00, 0x00, 0xFF,
};

static const unsigned char SEQ_TD4150_CE[] = {
	0xCE,
	0x5D, 0x40, 0x43, 0x49, 0x55, 0x62, 0x71, 0x82, 0x94, 0xA8,
	0xB9, 0xCB, 0xDB, 0xE9, 0xF5, 0xFC, 0xFF, 0x04, 0x00, 0x04,
	0x04, 0x00, 0x00, 0x00,
};

static const unsigned char SEQ_TD4150_CF[] = {
	0xCF,
	0x00,
};

static const unsigned char SEQ_TD4150_D0[] = {
	0xD0,
	0xC2, 0x33, 0x81, 0x66, 0x09, 0x90, 0x00, 0xC0, 0x92, 0x80,
	0x12, 0x48, 0x06, 0x7E, 0x09, 0x08, 0xD0, 0x00,
};

static const unsigned char SEQ_TD4150_D1[] = {
	0xD1,
	0xD4, 0xD4, 0x1B, 0x33, 0x33, 0x17, 0x07, 0x3B, 0x55, 0x74,
	0x55, 0x74, 0x00, 0x33, 0x77, 0x07, 0x33, 0x30, 0x06, 0x72,
	0x13, 0x13, 0x00,
};

static const unsigned char SEQ_TD4150_D2[] = {
	0xD2,
	0x00, 0x00, 0x00,
};

static const unsigned char SEQ_TD4150_D3[] = {
	0xD3,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
	0xF7, 0xFF, 0xFF, 0xF7, 0xFF, 0xFF, 0xF7, 0xFF, 0xFF, 0xF7,
	0xFF, 0xFF, 0xF7, 0xFF, 0xFF, 0xF7, 0xFF, 0xFF, 0xF7, 0xFF,
	0xFF, 0xF7, 0xFF, 0xFF, 0xF7, 0xFF, 0xFF, 0xF7, 0xFF, 0xFF,
	0xF7, 0xFF, 0xFF, 0xF7, 0xFF, 0xFF, 0xF7, 0xFF, 0xFF, 0xF7,
	0xFF, 0xFF, 0xF7, 0xFF, 0xFF, 0xF7, 0xFF, 0xFF, 0xF7, 0xFF,
	0xFF, 0xF7, 0xFF, 0xFF, 0xF7, 0xFF, 0xFF, 0xF7, 0xFF, 0xFF,
	0xF7, 0xFF, 0xFF, 0xF7, 0xFF, 0xFF, 0xF7, 0xFF, 0xFF, 0xF7,
	0xFF, 0xFF, 0xF7, 0xFF, 0xFF, 0xF7, 0xFF, 0xFF, 0xF7, 0xFF,
	0xFF, 0xF7, 0xFF, 0xFF, 0xF7, 0xFF, 0xFF, 0xF7, 0xFF, 0xFF,
	0xF7, 0xFF, 0xFF, 0xF7, 0xFF, 0xFF, 0xF7, 0xFF, 0xFF, 0xF7,
	0xFF, 0xFF, 0xF7, 0xFF, 0xFF, 0xF7, 0xFF, 0xFF, 0xF7, 0xFF,
	0xFF, 0xF7, 0xFF, 0xFF, 0xF7, 0xFF, 0xFF, 0xF7, 0xFF, 0xFF,
	0xF7, 0xFF, 0xFF, 0xF7, 0xFF, 0xFF, 0xF7, 0xFF, 0xFF, 0xF7,
	0xFF, 0xFF, 0xF7, 0xFF, 0xFF, 0xF7, 0xFF, 0xFF, 0xF7, 0xFF,
	0xFF, 0xF7, 0xFF,
};

static const unsigned char SEQ_TD4150_E5[] = {
	0xE5,
	0x03, 0x00, 0x00, 0x00,
};

static const unsigned char SEQ_TD4150_D6[] = {
	0xD6,
	0xC0,
};

static const unsigned char SEQ_TD4150_D7[] = {
	0xD7,
	0x21, 0x00, 0x12, 0x12, 0x00, 0x70, 0x08, 0x58, 0x00, 0x70,
	0x08, 0x58, 0x00, 0x83, 0x80, 0x85, 0x85, 0x85, 0x87, 0x84,
	0x45, 0x86, 0x87, 0x80, 0x82, 0x80, 0x80, 0x83, 0x83, 0x88,
	0x84, 0x08, 0x0C, 0x0C, 0x0B, 0x0A, 0x0A, 0x0A, 0x07, 0x07,
	0x06, 0x06, 0x0C, 0x08,
};

static const unsigned char SEQ_TD4150_D9[] = {
	0xD9,
	0x00, 0x02, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
};

static const unsigned char SEQ_TD4150_DD[] = {
	0xDD,
	0x30, 0x06, 0x23, 0x65,
};

static const unsigned char SEQ_TD4150_DE[] = {
	0xDE,
	0x00, 0x00, 0x00, 0x0F, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x10,
	0x00, 0x00, 0x00, 0x00,
};

static const unsigned char SEQ_TD4150_E3[] = {
	0xE3,
	0xFF,
};

static const unsigned char SEQ_TD4150_E9[] = {
	0xE9,
	0x80, 0x11, 0x17, 0x00, 0x00, 0x00, 0x5C, 0x00, 0xFF, 0xBF,
	0x05, 0xFF, 0x9F, 0x04, 0xFF, 0x7F, 0x06, 0xFF, 0x3F, 0x05,
	0xFF, 0x9F, 0x04, 0xFF, 0xCF, 0x07, 0xFF, 0xE7, 0x07, 0xFF,
	0xF3, 0x07, 0xFF, 0xF9, 0x07, 0xCD, 0xFF, 0x07, 0xE4, 0xFF,
	0x07, 0xF3, 0xFF, 0x07, 0xE9, 0xFF, 0x07, 0xE4, 0xFC, 0x07,
	0x7F, 0xFE, 0x07, 0x3F, 0xFF, 0x07, 0x9F, 0xFF, 0x07, 0xDF,
	0xFF, 0x07, 0x0C,
};

static const unsigned char SEQ_TD4150_EA[] = {
	0xEA,
	0x01, 0x0A, 0x14, 0x08, 0xE1, 0x0A, 0x20, 0xA2, 0x00, 0x00,
	0x00, 0x0A, 0x23, 0x00, 0x03, 0x01, 0x16, 0x01, 0x16, 0x01,
	0x16, 0x01, 0x28, 0x01, 0x28, 0x00, 0x70, 0x00, 0x00, 0x00,
	0x7F, 0x00, 0x34, 0x00,
};

static const unsigned char SEQ_TD4150_EB[] = {
	0xEB,
	0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x11,
};

static const unsigned char SEQ_TD4150_EC[] = {
	0xEC,
	0x01, 0xC0, 0x00, 0x10, 0x9B, 0x0A, 0x20, 0xA2, 0x00, 0x00,
	0x00, 0x01, 0x2B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
};

static const unsigned char SEQ_TD4150_ED[] = {
	0xED,
	0x01, 0x02, 0x06, 0x07, 0x00, 0x00, 0x02, 0x01, 0x65, 0x80,
	0x01, 0x00, 0x08, 0x00, 0x80, 0x08, 0x00, 0x00, 0x00, 0x01,
	0x0F, 0x00, 0x01, 0x30, 0x01, 0x01, 0x0F, 0x00, 0x00, 0x10,
	0x81, 0x10, 0x01,
};

static const unsigned char SEQ_TD4150_EE[] = {
	0xEE,
	0x05, 0x40, 0x05, 0x00, 0xC0, 0x0F, 0x00, 0xC0, 0x0F, 0x00,
	0xC0, 0x0F, 0x00, 0xC0, 0x0F, 0x00, 0xC0, 0x0F, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xC0, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00,
	0x00, 0x00, 0x00, 0x00, 0xFF, 0x3F, 0x00, 0xC0, 0x0F, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03,
	0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00,
	0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x02,
};

static const unsigned char SEQ_TD4150_B0_CLOSE[] = {
	0xB0, 0x03,
};

static const unsigned char SEQ_TD4150_BRIGHTNESS_ON[] = {
	0x53, 0x25,
};

static const unsigned char SEQ_TD4150_BRIGHTNESS_DEF[] = {
	0x51, 0x80,
};

static const unsigned char SEQ_SLEEP_OUT[] = {
	0x11,
};

static const unsigned char SEQ_SLEEP_IN[] = {
	0x10,
};

static const unsigned char SEQ_DISPLAY_ON[] = {
	0x29,
};

static const unsigned char SEQ_DISPLAY_OFF[] = {
	0x28,
	0x00,  0x00
};

#endif /* __TD4150_PARAM_H__ */
