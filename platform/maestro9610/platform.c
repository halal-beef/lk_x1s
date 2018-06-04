#include "uart_simple.h"
#include <dev/ufs.h>
#include <dev/boot.h>
#include <pit.h>

void platform_early_init(void)
{
	set_first_boot_device_info();
	uart_test_function();
}

void platform_init(void)
{
	ufs_init(2);
	ufs_set_configuration_descriptor();
	pit_init();
}
