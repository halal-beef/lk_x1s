/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __CMD_BOOT_H__
#define __CMD_BOOT_H__
int cmd_scatter_load_boot(int argc, const cmd_args *argv);
void configure_dtb(void);
void configure_ddi_id(void);

void arm_generic_timer_disable(void);
#endif // __CMD_BOOT_H__
