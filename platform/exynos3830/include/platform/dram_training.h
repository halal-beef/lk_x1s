/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __DRAM_TRAINING_DATA_H__
#define __DRAM_TRAINING_DATA_H__

#include <platform/sfr.h>

#define DRAM_TRAINING_AREA_BASE			CONFIG_DRAM_TRAINING_AREA_BASE
#define DRAM_TRAINING_AREA_SIZE			CONFIG_DRAM_TRAINING_AREA_SIZE
#define DRAM_TRAINING_DATA_SIZE			(DRAM_TRAINING_AREA_SIZE - 16)
#define DRAM_TRAINING_CRC_AREA_SIZE			(DRAM_TRAINING_AREA_SIZE - 8)
#define DRAM_TRAINING_INTEGRITY_HASH_AREA_SIZE		(DRAM_TRAINING_AREA_SIZE - 4)
#define DRAM_TRAINING_ACPM_MAGIC_CODE_BASE		(DRAM_TRAINING_AREA_BASE + DRAM_TRAINING_DATA_SIZE)
#define DRAM_TRAINING_WRITE_MAGIC_CODE_BASE		(DRAM_TRAINING_AREA_BASE + DRAM_TRAINING_DATA_SIZE + 4)
#define DRAM_TRAINING_CRC_CODE_BASE			(DRAM_TRAINING_AREA_BASE + DRAM_TRAINING_CRC_AREA_SIZE)
#define DRAM_TRAINING_INTEGRITY_HASH_VALUE_BASE	(DRAM_TRAINING_AREA_BASE + DRAM_TRAINING_INTEGRITY_HASH_AREA_SIZE)
#define DRAM_TRAINING_ACPM_MAGIC_CODE			(0xAC8338A9)
#define DRAM_TRAINING_WRITE_MAGIC_CODE			(0xF67038A9)

extern void write_dram_training_data(void);

#endif /*__DRAM_TRAINING_DATA_H__*/

