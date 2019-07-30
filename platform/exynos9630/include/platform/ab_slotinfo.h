/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __AB_SLOTINFO_H__
#define __AB_SLOTINFO_H__

typedef struct ExynosSlotInfo {
	uint8_t magic[4];

	uint8_t bootable;
	uint8_t is_active;
	uint8_t boot_successful;
	uint8_t tries_remaining;

	uint8_t reserved[8];
} ExynosSlotInfo;

#if (__STDC_VERSION__ >= 201112L) || defined(__cplusplus)
//static_assert(sizeof(struct ExynosSlotInfo) == 16);
#endif

#define AB_SLOTINFO_PART_NAME "slotinfo"

#endif	/* __AB_SLOTINFO_H__ */
