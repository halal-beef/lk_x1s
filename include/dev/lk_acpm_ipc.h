
/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __EXYNOS_ACPM_IPC_H_
#define __EXYNOS_ACPM_IPC_H_

#include <platform/sfr.h>

#define INTGR0                          (0x0008)
#define INTCR1                          (0x0020)
#define INTSR1                          (0x0028)


//#define ACPM_IPC_PROTOCOL_TEST (23)
//#define ACPM_IPC_PROTOCOL_ID   (26)

#define IPC_AP_I3C              (10)

#define INTR_FLAG_OFFSET       (16)

#define EXYNOS_MESSAGE_APM	(0x203c200)
#define SR0                     (0x0)
#define SR1                     (0x4)
#define SR2                     (0x8)
#define SR3                     (0xC)

void i3c_read(unsigned char channel, unsigned char type, unsigned char reg, unsigned char *dest);
void i3c_write(unsigned char channel, unsigned char type, unsigned char reg, unsigned char value);
#endif /* __EXYNOS_ACPM_IPC_H__ */
