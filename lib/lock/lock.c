/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <debug.h>
#include <stdlib.h>
#include <part_gpt.h>
#include <pit.h>
#include <part.h>
#include <platform/environment.h>

void lock(int state)
{
	unsigned int *env_val;
	struct pit_entry *ptn;

	ptn = pit_get_part_info("env");
	env_val = memalign(0x1000, part_get_size_in_bytes(ptn));
	pit_access(ptn, PIT_OP_LOAD, (u64)env_val, 0);

	/* The device should prompt users to warn them that
	 * they may encounter problems with unofficial images.
	 * After acknowledging, a factory data reset should be
	 * done to prevent unauthorized data access. */
	if ((env_val[ENV_ID_LOCKED] == 1) && (state == 0)) {
	}

	env_val[ENV_ID_LOCKED] = state;
	pit_access(ptn, PIT_OP_FLASH, (u64)env_val, 0);

	free(env_val);
}

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
	unsigned int *env_val;
	struct pit_entry *ptn;
	int lock_state;

	ptn = part_get("env");
	env_val = memalign(0x1000, part_get_size_in_bytes(ptn));
	part_read(ptn, env_val);

	lock_state = env_val[ENV_ID_LOCKED];

	free(env_val);

	return lock_state;
}

void lock_critical(int state)
{
	unsigned int *env_val;
	struct pit_entry *ptn;

	ptn = part_get("env");
	env_val = memalign(0x1000, part_get_size_in_bytes(ptn));
	part_read(ptn, env_val);

	/* Transitioning from locked to unlocked state should
	 * require a physical interaction with the device. */
	if ((env_val[ENV_ID_LOCKED_CRITICAL] == 1) && (state == 0)) {
	}

	env_val[ENV_ID_LOCKED_CRITICAL] = state;
	part_write(ptn, env_val);

	free(env_val);
}

int get_unlock_ability(void)
{
	/* If get_unlock_ability is "0" the user needs to boot
	 * to the home screen, go into the Settings > System > Developer
	 * options menu and enable the OEM unlocking option to set
	 * unlock_ability to: "1" */
	int unlock_ability = 1;

	return unlock_ability;
}
