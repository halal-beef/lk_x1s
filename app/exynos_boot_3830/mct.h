#ifndef __MCT_H__
#define __MCT_H__

#define MCT_BASE                (0x10040000)
#define	CONFIG_FIN_HZ           26000000
#define USEC_PER_SEC            1000000L

typedef unsigned long long cycle_t;

#define G_TCON		(0x240)
#define G_CNT_L		(0x100)
#define G_CNT_U		(0x104)
#define G_WSTAT		(0x24C)
#define WSTAT_G_TCON				(1 << 16)
#define G_TCON_START_FRC			(1 << 8)

#define mct_check_wstat(wstat, offset) \
do { \
	if (readl(MCT_BASE + wstat) & offset) { \
		readl(MCT_BASE + wstat) = offset; \
		break; \
	} \
} while (1)

typedef struct {
	void (*init)(void);
	void (*deinit)(void);
	void (*udelay)(cycle_t usec);
	cycle_t (*get_timer)(cycle_t start);
	cycle_t (*usec2ticks)(cycle_t usec);
	cycle_t (*ticks2usec)(cycle_t tick);
} mct_timer_t;

void mct_udelay(cycle_t usec);
extern mct_timer_t mct;

#endif
