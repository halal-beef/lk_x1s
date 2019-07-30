
#ifndef __PLATFORM_DELAY_H
#define __PLATFORM_DELAY_H

#define u_delay(x) DMC_Delay(0, x)
#define udelay(x) u_delay(x)
#define mdelay(x) u_delay((x) * (1000))

void DMC_Delay(u32 uTsel, u32 uSec);

#endif
