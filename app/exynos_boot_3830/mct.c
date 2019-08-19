#include "mct.h"
#include <platform/mmu/mmu.h>
#include <reg.h>

static cycle_t divided_clk_rate = CONFIG_FIN_HZ;
void mct_global_start_frc(void)
{
	unsigned int reg;

	reg = readl(MCT_BASE + G_TCON);
	reg |= (1 << 8);
	readl(MCT_BASE + G_TCON) = reg;
}

void mct_global_stop_frc(void)
{
	unsigned int reg;

	reg = readl(MCT_BASE + G_TCON);
	if (reg & G_TCON_START_FRC) {
		reg &= ~(G_TCON_START_FRC);
		readl(MCT_BASE + G_TCON) = reg;
	}
}

cycle_t mct_global_get_frc_cnt(void)
{
	cycle_t count;
	unsigned int low, high, high1;

	high = readl(MCT_BASE + G_CNT_U);
	low = readl(MCT_BASE + G_CNT_L);
	dmb();
	dsb();
	high1 = readl(MCT_BASE + G_CNT_U);
	if (high != high1)
		low = readl(MCT_BASE + G_CNT_L);
	dmb();
	dsb();

	count = (cycle_t)low;

	if (sizeof(cycle_t) > sizeof(unsigned int))
		count |= ((cycle_t)high1 << 32);

	return count;
}

void mct_global_set_frc_cnt(cycle_t count)
{
	uint32_t low;
	uint32_t high;

	if (count == mct_global_get_frc_cnt())
		return;

	high = (u32)((count >> 32) & 0xFFFFFFFF);
	low = (uint32_t)(count & 0xFFFFFFFF);

	readl(MCT_BASE + G_CNT_U) = high;

	readl(MCT_BASE + G_CNT_L) = low;
}

/* Returns how many ticks elapsed since start */
cycle_t mct_get_timer(cycle_t start)
{
	return mct_global_get_frc_cnt() - start;
}

u64 ticks2usec(cycle_t tick)
{
	return (tick * USEC_PER_SEC) / divided_clk_rate;
}

cycle_t usec2ticks(cycle_t usec)
{
	return (usec * divided_clk_rate) / USEC_PER_SEC;
}

void mct_udelay(cycle_t usec)
{
	cycle_t end = mct_get_timer(0) + usec2ticks(usec);
	while (mct_get_timer(0) < end);
}

void mct_init(void)
{
	mct_global_set_frc_cnt(0);
	mct_global_start_frc();
}

void mct_deinit(void)
{
	mct_global_stop_frc();
	mct_global_set_frc_cnt(0);
}

mct_timer_t mct = {
	.init = mct_init,
	.deinit = mct_deinit,
	.udelay = mct_udelay,
	.get_timer = mct_get_timer,
	.usec2ticks = usec2ticks,
	.ticks2usec = ticks2usec,
};
