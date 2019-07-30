/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __WDT_RECOVERY_H__
#define __WDT_RECOVERY_H__

int clear_wdt_recovery_settings(void);
int force_wdt_recovery(void);

void wdt_stop(void);
void wdt_start(unsigned int timeout);
void wdt_keepalive(void);

#endif	/* __WDT_RECOVERY_H__ */
