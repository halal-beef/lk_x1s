/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <stdio.h>
#include <libfdt.h>
#include <platform/fdt.h>

#define BUFFER_SIZE 10
extern u32 ddi_id;
void configure_ddi_id(void)
{
	char str[BUFFER_SIZE];

	printf("Attached DDI id is [%#x]\n", ddi_id);

	if (ddi_id) {
		memset(str, 0, sizeof(str));
		sprintf(str, "<0x%x>", ddi_id);
		set_fdt_val("/panel_0", "ddi_id", str);
	}
#if 0
	/* the path(node) name, /dsim is same with /dsim@0x148E0000 */
	get_fdt_val("/dsim", "ddi_id", (char *)str);
	printf("ddi_id (%s)\n", str);
#endif
}

