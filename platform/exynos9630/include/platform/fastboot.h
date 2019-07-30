/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __FASTBOOT_H__
#define __FASTBOOT_H__

int do_fastboot(int argc, const cmd_args *argv);
int connect_usb(void);
int init_fastboot_variables(void);

#define CFG_FASTBOOT_PAGESIZE		  (2048)
#define CFG_FASTBOOT_TRANSFER_BUFFER	  (0X8A000000)
#define CFG_FASTBOOT_TRANSFER_BUFFER_SIZE (0x30000000)

#define FASTBOOT_VERSION_BOOTLOADER	"EXYNOS9630-LK-20190509-1"

#endif	/* __FASTBOOT_H__ */
