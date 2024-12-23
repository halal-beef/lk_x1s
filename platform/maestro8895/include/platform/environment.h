/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __ENVIRONMENT_H__
#define __ENVIRONMENT_H__

typedef enum {
	ENV_ID_RAMDISK_SIZE = 0x0,
	ENV_ID_LOCKED = 0x1,		/* 0:unlocked, 1:locked */
	ENV_ID_LOCKED_CRITICAL = 0x2,	/* 0:unlocked, 1:locked */
} env_id_t;

#endif	/* __ENVIRONMENT_H__ */
