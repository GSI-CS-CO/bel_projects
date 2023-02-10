#include <stdarg.h>
/*
 * minimal vsprintf: only %s and hex values
 * Alessandro Rubini 2010, based on code in u-boot (from older Linux)
 * GNU GPL version 2.
 */
int pp_vsprintf(char *buf, const char *fmt, va_list args)
{
	int i, j;
	static char hex[] = "0123456789abcdef";
	char *s;
	char *str = buf;

	for (; *fmt ; ++fmt) {
		if (*fmt != '%') {
			*str++ = *fmt;
			continue;
		}

	repeat:
		fmt++;		/* Skip '%' initially, other stuff later */

		/* Skip the complete format string */
		switch(*fmt) {
		case '\0':
			goto ret;
		case '*':
			/* should be precision, just eat it */
			i = va_arg(args, int);
			/* fall through: discard unknown stuff */
		default:
			goto repeat;

			/* Special cases for conversions */

		case 'c': /* char: supported */
			*str++ = (unsigned char) va_arg(args, int);
			break;
		case 's': /* string: supported */
			s = va_arg(args, char *);
			while (*s)
				*str++ = *s++;
			break;
		case 'n': /* number-thus-far: not supported */
			break;
		case '%': /* supported */
			*str++ = '%';
			break;

			/* all integer (and pointer) are printed as <%08x> */
		case 'o':
		case 'x':
		case 'X':
		case 'd':
		case 'i':
		case 'u':
		case 'p':
			i = va_arg(args, int);
			*str++ = '<';
			for (j = 28; j >= 0; j -= 4)
				*str++ = hex[(i>>j)&0xf];
			*str++ = '>';
			break;
		}
	}
 ret:
	*str = '\0';
	return str - buf;
}
