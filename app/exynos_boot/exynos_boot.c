#include <app.h>

static void exynos_boot_task(const struct app_descriptor *app, void *args)
{
	return;
}

APP_START(exynos_boot)
	.entry = exynos_boot_task,
	.flags = 0,
APP_END
