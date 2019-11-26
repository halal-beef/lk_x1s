/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <sys/types.h>
#include <platform.h>

void platform_delay(u32 u_sec)
{
	lk_bigtime_t destination_time, curr_time;

	destination_time = current_time_hires() + (lk_bigtime_t)u_sec;

	while(1) {
		curr_time = current_time_hires();

		if (curr_time > destination_time)
			break;
	}
}
