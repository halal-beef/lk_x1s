/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __FDTAPI_H__
#define __FDTAPI_H__
int merge_dto_to_main_dtb(unsigned int board_id, unsigned int board_rev);
int resize_dt(unsigned int sz);
int make_fdt_node(const char *path, char *node);
int get_fdt_val(const char *path, const char *property, char *retval);
int set_fdt_val(const char *path, const char *property, const char *value);
void add_dt_memory_node(unsigned long base, unsigned int size);
int get_fdt_dpm_val(const char *path, const char *property, char *retval);

extern struct fdt_header *fdt_dtb;
extern struct dt_table_header *dtbo_table;
extern struct fdt_header *fdt_dpm;
#endif
