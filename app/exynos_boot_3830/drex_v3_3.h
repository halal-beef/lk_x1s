/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.


 * Alternatively, this program is free software in case of open source project
 * you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.

*/

#ifndef __MEMORY_CONTROLLER_H__
#define __MEMORY_CONTROLLER_H__

#include "drex_v3_3_reg.h"

#define DMC0_BASE                   (0x10440000)
#define DMC1_BASE                   (0x10540000)
#define DMC_ALL_BASE                (0x12110000)

typedef enum {
	MC_CH_0,
	MC_CH_1,
//	MC_CH_2,
//	MC_CH_3,
	MC_CH_ALL,
} dmc_ch_t;

// spes rank concept check. DMC & SMC
typedef enum {
	MC_RANK_0 = 1,
	MC_RANK_1 = 2,
	MC_RANK_ALL = 3,
} dmc_rank_t;


typedef enum {
	MC_BYTE_0,
	MC_BYTE_1,
	MC_BYTE_ALL,
} dmc_byte_t;

typedef enum {
	MC_COMMAND_ACTIVE                  = 0,
	MC_COMMAND_PER_BANK_PRECHARGE      = 1,
	MC_COMMAND_ALL_BANK_PRECHARGE      = 2,
	MC_COMMAND_RESERVED_0x3            = 3,
	MC_COMMAND_MRW                     = 5,
	MC_COMMAND_MRR                     = 6,
	MC_COMMAND_SRE                     = 7,
	MC_COMMAND_SRX                     = 8,
	MC_COMMAND_PWR_DOWN_ENTRY          = 9,
	MC_COMMAND_PWR_DOWN_EXIT           = 0xA,
	MC_COMMAND_NOP                     = 0xB,
	MC_COMMAND_MPC_ZQCAL_START         = 0xC,
	MC_COMMAND_MPC_ZQCAL_LATCH         = 0xD,
	MC_COMMAND_MPC_ZQ_SHORT_CALIBRAION = 0xE,
	MC_COMMAND_RESERVED_0xF            = 0xF,
} dmc_direct_command_type_t;

struct dmc_command_func_t {
	int (*mode_read)(const dmc_ch_t ch, dmc_rank_t rank, unsigned char mr);
};

struct dmc_driver_t {
	struct dmc_command_func_t command;
};

extern struct dmc_driver_t mc_driver;
extern volatile drex_ge_regs_t *__DMC[MC_CH_ALL];

#endif /* __MEMORY_CONTROLLER_H__ */
