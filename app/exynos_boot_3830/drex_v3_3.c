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
#include "drex_v3_3.h"

volatile drex_ge_regs_t *__DMC[2] = {
	(volatile drex_ge_regs_t *)DMC0_BASE,
	(volatile drex_ge_regs_t *)DMC1_BASE,
};

static int dmc_command_direct_command(const dmc_ch_t ch, dmc_rank_t rank, dmc_direct_command_type_t cmd, unsigned char ma, unsigned short op)
{
	drex_ge_direct_command_t DIRECT_COMMAND;

	int loop, start_ch, end_ch;

	if (ch == MC_CH_ALL) {
		start_ch = 0;
		end_ch = 1;
	} else {
		start_ch = ch;
		end_ch = ch;
	}

	for (loop = start_ch; loop <= end_ch; loop++) {
		DIRECT_COMMAND.data = 0;

		switch (rank) {
		case 1:
			DIRECT_COMMAND.bitfield.dcmd_rank = 0;
			break;
		case 2:
			DIRECT_COMMAND.bitfield.dcmd_rank = 1;
			break;
		case 3:
			DIRECT_COMMAND.bitfield.dcmd_rank_all = 1;
			break;
		default:
			__asm__ __volatile__("wfi" : : : "memory");
		}

		DIRECT_COMMAND.bitfield.dcmd_type = cmd;

		if (cmd == MC_COMMAND_MRW)
			DIRECT_COMMAND.bitfield.dcmd_op = op;

		DIRECT_COMMAND.bitfield.dcmd_ma = ma;

		__DMC[loop]->DIRECT_COMMAND.data = DIRECT_COMMAND.data;
	}

	return 0;
}

static int dmc_command_mode_read(const dmc_ch_t ch, dmc_rank_t rank, unsigned char ma)
{
	dmc_command_direct_command(ch, rank, MC_COMMAND_MRR, ma, 0);

	return (__DMC[ch]->MR_STATUS.data & 0xff);
}

struct dmc_driver_t mc_driver = {
	.command = {
		.mode_read           = dmc_command_mode_read,
	}
};
