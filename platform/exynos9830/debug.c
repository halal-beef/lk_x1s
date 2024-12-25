/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 *
 * No part of this software, either material or conceptual may be copied or
 * distributed, transmitted, transcribed, stored in a retrieval system or
 * translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed to third parties
 * without the express written permission of Samsung Electronics.
 *
 */
#include <platform/debug.h>
#include <platform/uart.h>

void platform_dputc(char c)
{
	// putc(c);
}

/* End TODO */

int platform_dgetc(char *c, bool wait)
{
	return 0;
}
