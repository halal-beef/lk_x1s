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

#ifndef _DPU_IO_CTRL_H_
#define _DPU_IO_CTRL_H_

/**********************EXYNOS_BOOTLOADER_DISPLAY*******************************/
#define EXYNOS_MIPI_PHY_ENABLE          (1 << 0)
#define EXYNOS_MIPI_PHY_SRESETN         (1 << 1)
#define EXYNOS_MIPI_PHY_MRESETN         (1 << 2)

/* GPIO are depenent upon board schematic */
enum board_gpio_type {
	BOARD_ESPRESSO = 0,
	BOARD_BTYPE,
	BOARD_BTYPE_REV01,
};

struct exynos_display_config {
	void (*set_mipi_phy)(unsigned int enable);
	void (*set_gpio_hw_te)(void);
	void (*set_gpio_lcd_reset)(enum board_gpio_type board_type);
	void (*set_gpio_lcd_power)(enum board_gpio_type board_type);
};

extern enum board_gpio_type get_exynos_board_type(void);
extern struct exynos_display_config *get_exynos_display_config(void);

#endif	/* _DPU_IO_CTRL_H_ */
