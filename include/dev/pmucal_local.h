#ifndef __PMUCAL_LOCAL_H__
#define __PMUCAL_LOCAL_H__
#include "pmucal_common.h"
#include "pmucal_rae.h"

/* will be a member of struct exynos_pm_domain */
struct pmucal_pd {
	u32 id;
	const char *name;
	struct pmucal_seq *on;
	struct pmucal_seq *save;
	struct pmucal_seq *off;
	struct pmucal_seq *status;
	u32 num_on;
	u32 num_save;
	u32 num_off;
	u32 num_status;
};

#define PMUCAL_PD_DESC(_pd_id, _name, _on, _save, _off, _status)	\
	[_pd_id] = {							\
		.id = _pd_id,						\
		.name = _name,						\
		.on = _on,						\
		.save = _save,						\
		.off = _off,						\
		.status = _status,					\
		.num_on = ARRAY_SIZE(_on),				\
		.num_save = ARRAY_SIZE(_save),				\
		.num_off = ARRAY_SIZE(_off),				\
		.num_status = ARRAY_SIZE(_status),			\
	}

void exynos_local_power_off(void);
#endif
