/*
 * Copyright (c) 2013, Google Inc. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __DEV_EXYNOS_ADC_H
#define __DEV_EXYNOS_ADC_H

#include <arch/ops.h>
#include <lk/init.h>
#include <lk/reg.h>
#include <kernel/thread.h>
#include <platform.h>
#include <platform/timer.h>
#include <sys/types.h>
#include <platform/debug.h>
#include <platform/delay.h>
#include <platform/sfr.h>

#define MAX_CHANNEL		11
#define ADC_CON1		(0x0)
#define ADC_CON2		(0x4)
#define ADC_DAT			(0x8)
#define ADC_SUM_DAT		(0xC)
#define ADC_INT_EN		(0x10)
#define ADC_INT_STATUS		(0x14)
#define ADC_DEBUG_DATA		(0x1C)
#define ADC_BASE_ADDR		EXYNOS_ADC_BASE

#define ADC_SOFT_RESET		(1u << 2)
#define ADC_NON_SOFT_RESET	(1u << 1)
#define ADC_CON2_C_TIME(x)	(((x) & 7) << 4)
#define ADC_CON2_ACH_MASK	(0xf)
#define ADC_CON2_ACH_SEL(x)	(((x) & 0xF) << 0)
#define ADC_CON_EN_START	(1u << 0)
#define ADC_DAT_MASK		(0xFFF)
#define EN_INT			(1)

void exynos_adc_init_hw(void);
void exynos_adc_exit_hw(void);
void exynos_adc_start_conv(u32 chan);
int exynos_adc_read_raw(u32 chan);

#endif
