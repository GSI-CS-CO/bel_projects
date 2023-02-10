#include <stdarg.h>
#include <pp-printf.h>
/*
 * empty vsprintf: only the format string. Public domain
 */
int pp_vsprintf(char *buf, const char *fmt, va_list args)
{
	char *str = buf;

	for (; *fmt ; ++fmt)
		*str++ = *fmt;
	*str++ = '\0';
	return str - buf;
}
