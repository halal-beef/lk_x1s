/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __BOARD_INFO_H__
#define __BOARD_INFO_H__

#define CONFIG_BOARD_NAME	"universal9830"
#define CONFIG_BOARD_UNIVERSAL9830

#define CONFIG_BOARD_ID		0x10
#define CONFIG_SUB_PMIC_S2DOS05
#define CONFIG_UFS_BOARD_TYPE	1      /* smdk : 0, universal : 1 */

#define CONFIG_USE_RPMB
#define CONFIG_USE_F2FS

#define BOOT_IMG_HDR_V2
#define CONFIG_DTB_IN_BOOT
#define CONFIG_RAMDISK_IN_BOOT

#define CONFIG_GET_B_REV_FROM_ADC
#define CONFIG_GET_B_REV_FROM_GPIO

#define VOLDOWN_GPIOCON EXYNOS9830_GPA0CON
#define VOLDOWN_BIT	4
#endif /*__BOARD_INFO_H__ */
