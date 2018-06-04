/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <reg.h>
#include <platform/sfr.h>
#include <dev/boot.h>

void set_first_boot_device_info(void)
{
	unsigned int boot_device = 0;
	unsigned int boot_device_info = 0;

	boot_device_info = 0xCB000010; /* get_boot_device_info(); */
	if ((boot_device_info & 0xFF000000) != 0xCB000000) {
		/* abnormal boot */
		while (1) ;
	}

	switch ((boot_device_info >> 4) & 0xF) {
	case BD_UFS:
		boot_device = BOOT_UFS;
		break;
	case BD_EMMC_BOOT:
		boot_device = BOOT_EMMC;
		break;
	case BD_USB:
		boot_device = BOOT_USB;
		break;
	case BD_SDMMC:
		boot_device = BOOT_MMCSD;
		break;
	case BD_UFS_CARD:
		break;
	case BD_SPI:
		break;
	default:
		break;
	}

	writel(boot_device, BOOT_DEV_INFO);
}

int get_boot_device(void)
{
	return BOOT_DEV;
}