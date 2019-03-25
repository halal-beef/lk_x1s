#ifndef __PMUCAL_LOCAL_H__
#define __PMUCAL_LOCAL_H__
#include "pmucal_common.h"
#include "pmucal_rae.h"

/* will be a member of struct exynos_pm_domain */
struct pmucal_pd {
	u32 id;
	const char *name;
	struct pmucal_seq *off;
	u32 num_off;
};

#define PMUCAL_PD_DESC(_pd_id, _name, _off)	\
	[_pd_id] = {				\
		.id = _pd_id,			\
		.name = _name,			\
		.off = _off,			\
		.num_off = ARRAY_SIZE(_off),	\
	}

void exynos_local_power_off(void);
#endif
