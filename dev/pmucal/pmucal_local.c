#include <debug.h>
#include <reg.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>
#include <lib/console.h>
#include <lib/font_display.h>
#include <pit.h>
#include <platform/sfr.h>
#include <platform/environment.h>
#include <platform/dfd.h>
#include <dev/boot.h>
#include <dev/pmucal_local.h>
#include <platform/flexpmu_cal_local_exynos9830.h>

/**
 *  pmucal_local_disable - disables a power domain.
 *		        exposed to PWRCAL interface.
 *
 *  @pd_id: power domain index.
 *
 *  Returns 0 on success. Otherwise, negative error code.
 */
int pmucal_local_disable(unsigned int pd_id)
{
	int ret = 0;

	if (pd_id >= pmucal_pd_list_size) {
		printf("%s pd index(%d) is out of supported range (0~%d).\n",
				PMUCAL_PREFIX, pd_id, pmucal_pd_list_size);
		ret = -EINVAL;
		goto err_out;
	}

	if (!pmucal_pd_list[pd_id].off) {
		printf("%s there is no sequence element for pd(%d) power-off.\n",
				PMUCAL_PREFIX, pd_id);
		ret = -ENOENT;
		goto err_out;
	}

	ret = pmucal_rae_handle_seq(pmucal_pd_list[pd_id].off,
				pmucal_pd_list[pd_id].num_off);
	if (ret) {
		printf("%s %s: error on handling disable sequence. (pd_id : %d, %s)\n",
				PMUCAL_PREFIX, __func__, pd_id, pmucal_pd_list[pd_id].name);

		goto err_out;
	}

	return 0;

err_out:
	return ret;
}

void exynos_local_power_off(void)
{
	unsigned int i;
	int ret;

	for (i = 0; i < pmucal_pd_list_size; i ++) {
		printf("%s%s power off... ", PMUCAL_PREFIX, pmucal_pd_list[i].name);
		ret = pmucal_local_disable(i);
		if (ret)
			printf("failed\n");
		else
			printf("done\n");
	}
}
