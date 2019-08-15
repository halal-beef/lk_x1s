/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __LOCK_H__
#define __LOCK_H__

#define LOCK_SYSPARAM_NAME		"locked"
#define LOCK_CRITICAL_SYSPARAM_NAME	"lock_critical"

void lock(int state);
int set_lock_state(int state);
int get_lock_state(void);
void lock_critical(int state);
int get_unlock_ability(void);

#endif	/* __LOCK_H__ */
