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

#include <dev/exynos_adc.h>

/*
 * Set software reset bit
 * Set C_TIME bit
 * Set interrupt enable bit
 */
void exynos_adc_init_hw(void)
{
	u32 con1, con2;

	con1 = ADC_SOFT_RESET;
	writel(con1, ADC_BASE_ADDR + ADC_CON1);

	con1 = ADC_NON_SOFT_RESET;
	writel(con1, ADC_BASE_ADDR + ADC_CON1);

	con2 = ADC_CON2_C_TIME(6);
	writel(con2, ADC_BASE_ADDR + ADC_CON2);

	writel(EN_INT, ADC_BASE_ADDR + ADC_INT_EN);
}

void exynos_adc_exit_hw(void)
{
	u32 con2;

	con2 = readl(ADC_BASE_ADDR + ADC_CON2);
	con2 &= ~ADC_CON2_C_TIME(7);
	writel(con2, ADC_BASE_ADDR + ADC_CON2);

	writel(!EN_INT, ADC_BASE_ADDR + ADC_INT_EN);
}

void exynos_adc_start_conv(u32 chan)
{
	u32 con1, con2;

	con2 = readl(ADC_BASE_ADDR + ADC_CON2);
	con2 &= ~ADC_CON2_ACH_MASK;
	con2 |= ADC_CON2_ACH_SEL(chan);
	writel(con2, ADC_BASE_ADDR + ADC_CON2);

	con1 = readl(ADC_BASE_ADDR + ADC_CON1);
	writel(con1 | ADC_CON_EN_START, ADC_BASE_ADDR + ADC_CON1);
}

int exynos_adc_read_raw(u32 chan)
{
	int val = -1;

	exynos_adc_init_hw();
	exynos_adc_start_conv(chan);

	mdelay(100);

	val = readl(ADC_BASE_ADDR + ADC_DAT) & ADC_DAT_MASK;

	exynos_adc_exit_hw();
	return val;
}
