/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __boot_H
#define __boot_H

typedef enum {
	BD_NO_DEVICE = 0x0,
	BD_UFS = 0x1,
	BD_EMMC_BOOT = 0x2,
	BD_ERROR = 0x3,
	BD_USB = 0x4,
	BD_SDMMC = 0x5,
	BD_UFS_CARD = 0x6,
	BD_SPI = 0x7,
} boot_device_info_t;

/*
 * Boot device
 */

#define BOOT_MMCSD		0x1
#define BOOT_EMMC		0x2
#define BOOT_USB		0x3
#define BOOT_UFS		0x4

#define BOOTDEVICE_ORDER_ADDR (IRAM_BASE + 0x64)

int is_first_boot(void);
void set_first_boot_device_info(void);
int get_boot_device(void);

#endif

