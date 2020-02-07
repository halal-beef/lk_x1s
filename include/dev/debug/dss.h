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

#ifndef __DSS_H__
#define __DSS_H__

enum {
	DEBUG_LEVEL_NONE = -1,
	DEBUG_LEVEL_LOW = 0,
	DEBUG_LEVEL_MID = 1,
};
#define DEBUG_LEVEL_PREFIX	(0xDB9 << 16)
extern int debug_level;

void set_debug_level(const char *buf);
void dss_fdt_init(void);
int dss_getvar_item(const char *name, char *response);
unsigned long dss_get_item_paddr(const char *name);
unsigned long dss_get_item_size(const char *name);
void dss_boot_cnt(void);
void dfd_display_reboot_reason(void);
void dfd_display_core_stat(void);
void dfd_run_post_processing(void);
void dfd_set_dump_en(int en);
void dfd_get_dbgc_version(void);
int dfd_get_revision(void);
bool is_cache_disable_mode(void);
unsigned int dfd_get_dump_en_before_reset(void);
#endif /* __DSS_H__ */
