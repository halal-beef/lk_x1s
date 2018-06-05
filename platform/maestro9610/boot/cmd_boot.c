/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <lib/console.h>

int cmd_boot(int argc, const cmd_args *argv)
{
	void (*kernel_entry)(int r0, int r1, int r2, int r3);

	kernel_entry = (void (*)(int, int, int, int))0x80080000;
	kernel_entry(0x8a000000, 0, 0, 0);

	return 0;
}

STATIC_COMMAND_START
STATIC_COMMAND("boot", "start kernel booting", &cmd_boot)
STATIC_COMMAND_END(boot);
