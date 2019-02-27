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

#ifndef __LCD_CTRL_H__
#define __LCD_CTRL_H__

#include <dev/dpu/decon_lcd.h>

void s6e3fa0_lcd_init(unsigned int id, struct decon_lcd *lcd);
void s6e3fa0_lcd_enable_exynos(unsigned int id);
void s6e3fa0_lcd_disable_exynos(unsigned int id);
int s6e3fa0_lcd_gamma_ctrl(unsigned int id, unsigned int backlightlevel);
int s6e3fa0_lcd_gamma_update(int id);

void nt36672a_lcd_init(unsigned int id, struct decon_lcd *lcd);
void nt36672a_lcd_enable_exynos(unsigned int id);
void nt36672a_lcd_disable_exynos(unsigned int id);
int nt36672a_lcd_gamma_ctrl(unsigned int id, unsigned int backlightlevel);
int nt36672a_lcd_gamma_update(int id);

struct decon_lcd *decon_get_lcd_info(void);
struct dsim_lcd_driver *decon_get_panel_info(void);

#endif /* __LCD_CTRL_H__ */
