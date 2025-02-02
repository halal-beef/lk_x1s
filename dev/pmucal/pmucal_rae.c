#include <lk/debug.h>
#include <lk/reg.h>
#include <string.h>
#include <stdlib.h>
#include <lk/err.h>
#include <lib/console.h>
#include <lib/font_display.h>
#include <pit.h>
#include <platform/sfr.h>
#include <platform/environment.h>
#include <dev/boot.h>
#include <dev/pmucal_common.h>
#include <dev/pmucal_rae.h>

/**
 * A global index for pmucal_rae_handle_seq.
 * it should be helpful in ramdump.
 */
static unsigned int pmucal_rae_seq_idx;

static inline bool pmucal_rae_check_value(struct pmucal_seq *seq)
{
	u32 reg;

	if (seq->access_type == PMUCAL_WRITE_WAIT)
		reg = readl(seq->base_pa + 0x4);
	else
		reg = readl(seq->base_pa);
	reg &= seq->mask;
	if (reg == seq->value)
		return true;
	else
		return false;
}

static int pmucal_rae_wait(struct pmucal_seq *seq)
{
	u32 timeout = 0;

	while (1) {
		if (pmucal_rae_check_value(seq))
			break;
		timeout++;
		mdelay(1);
		if (timeout > 10) {
			u32 reg;
			reg = readl(seq->base_pa);
			printf("%s %s:timed out during wait. (value:0x%x, seq_idx = %d)\n",
						PMUCAL_PREFIX, __func__, reg, pmucal_rae_seq_idx);
			return -2;
		}
	}

	return 0;
}

static inline void pmucal_rae_write(struct pmucal_seq *seq)
{
	if (seq->mask == 0xFFFFFFFF)
		writel(seq->value, seq->base_pa);
	else {
		u32 reg;
		reg = readl(seq->base_pa);
		reg = (reg & ~seq->mask) | (seq->value & seq->mask);
		writel(reg, seq->base_pa);
	}
}
/**
 *  pmucal_rae_handle_seq - handles a sequence array based on each element's access_type.
 *			    exposed to PMUCAL common logics.(CPU/System/Local)
 *
 *  @seq: Sequence array to be handled.
 *  @seq_size: Array size of seq.
 *
 *  Returns 0 on success. Otherwise, negative error code.
 */
int pmucal_rae_handle_seq(struct pmucal_seq *seq, unsigned int seq_size)
{
	int ret;
	unsigned int i;

	for (i = 0; i < seq_size; i++) {
		pmucal_rae_seq_idx = i;

		switch (seq[i].access_type) {
		case PMUCAL_WRITE:
			pmucal_rae_write(&seq[i]);
			break;
		case PMUCAL_WAIT:
		case PMUCAL_WAIT_TWO:
			ret = pmucal_rae_wait(&seq[i]);
			if (ret)
				return ret;
			break;
		case PMUCAL_WRITE_WAIT:
			pmucal_rae_write(&seq[i]);
			ret = pmucal_rae_wait(&seq[i]);
			if (ret)
				return ret;
			break;
		case PMUCAL_WRITE_RETURN:
			pmucal_rae_write(&seq[i]);
			return 0;
		case PMUCAL_DELAY:
			mdelay(seq[i].value);
			break;
		default:
			printf("%s %s:invalid PMUCAL access type\n", PMUCAL_PREFIX, __func__);
			return -1;
		}
	}

	return 0;
}
