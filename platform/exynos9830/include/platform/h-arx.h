/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 *
 * Alternatively, this program is free software in case of open source project
 * you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.

*/

#ifndef _HARX_H_
#define _HARX_H_

#include <lib/font_display.h>

/* Base address */
#define EXYNOS_HARX_BASE_ADDR			(0xC1000000)

/* Partition name */
#define EXYNOS_HARX_PART_NAME			"harx"

/*
 * Callers don't have to write line feed(\n) when calling
 * this macro because print_lcd_update can't handle this
 * character.
 * So this macro calls printf("\n") additionally.
 */
#define harx_print_with_lcd(...)				\
	do {							\
		printf(__VA_ARGS__);				\
		printf("\n");					\
		print_lcd_update(FONT_GREEN, FONT_GRAY,		\
				__VA_ARGS__);			\
	} while (0)

#ifndef __ASSEMBLY__
int load_and_init_harx(void);
#endif	/* __ASSEMBLY__ */
#endif	/* _HARX_H_ */
