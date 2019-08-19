#include <platform/debug.h>
#include <platform/environment.h>
#include "uart_simple.h"

void platform_dputc(char c)
{
#if defined(CONFIG_UART_LOG_MODE)
	if (uart_log_mode == 1)
#endif
		uart_simple_char_out(c);
}

int platform_dgetc(char *c, bool wait)
{
	uart_simple_char_in(c);

	return 0;
}
