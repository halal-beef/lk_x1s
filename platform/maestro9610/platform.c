#include "uart_simple.h"
#include <dev/ufs.h>

void platform_early_init(void)
{
	uart_test_function();
}

void platform_init(void)
{
	ufs_init(2);
	ufs_set_configuration_descriptor();
}
