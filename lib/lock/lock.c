/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <lk/debug.h>
#include <stdlib.h>
#include <dev/rpmb.h>
#include <platform/sfr.h>

int set_lock_state(int state)
{
	int ret;

#if defined(CONFIG_USE_RPMB)
	int rpmb_ret;
	rpmb_ret = rpmb_set_lock_state((uint32_t)(state));
	if (rpmb_ret == RV_SUCCESS) {
		ret = 0;
	} else {
		ret = -1;
	}
#else
	ret = -1;
#endif

	return ret;
}

int get_lock_state(void)
{
	int ret;

#if defined(CONFIG_USE_RPMB)
	int rpmb_ret;
	uint32_t lock_state = 0;
	rpmb_ret = rpmb_get_lock_state(&lock_state);
	if (rpmb_ret == RV_SUCCESS) {
		ret = lock_state;
	} else {
		ret = -1;
	}
#else
	ret = 0;
#endif

	return ret;
}

void lock_critical(int state)
{
}

int get_unlock_ability(void)
{
	/* If get_unlock_ability is "0" the user needs to boot
	  to the home screen, go into the Settings > System > Developer
	  options menu and enable the OEM unlocking option to set
	  unlock_ability to: "1" */
	int unlock_ability = 1;

	return unlock_ability;
}
