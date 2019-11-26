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
#ifndef __PLATFORM_DELAY_H
#define __PLATFORM_DELAY_H

#define u_delay(x)	platform_delay(x)
#define udelay(x)	u_delay(x)
#define mdelay(x)	u_delay((x) * (1000))

void DMC_Delay(u32 uTsel, u32 uSec);
void platform_delay(u32 u_sec);

#endif
