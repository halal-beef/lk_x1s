#include <platform/debug.h>
#include "uart_simple.h"

void platform_dputc(char c)
{
	uart_simple_char_out(c);
}

int platform_dgetc(char *c, bool wait)
{
	uart_simple_char_in(c);

	return 0;
}
