#include <string.h>
#include <stdarg.h>
#include <syscon.h> /* usleep from wrpc-sw */
#include <assert.h>
#include <pp-printf.h>

unsigned int assertWait(int us)
{
  volatile unsigned int i;

  for (i = 0; i < 6 * us ; i++)
    ;

  return i;
} // wait 

void panic(const char *fmt, ...)
{
	va_list args;
	char message[128];

	strcpy(message, "Panic: ");
	va_start(args, fmt);
	pp_vsprintf(message + 7, fmt, args);
	va_end(args);

	while (1) {
		pp_printf(message);
		assertWait(1000 * 1000);
	}
}

void __assert(const char *func, int line, int forever,
		     const char *fmt, ...)
{
	va_list args;
	char message[200];
	int more = fmt && fmt[0];

	pp_sprintf(message, "Assertion failed (%s:%i)%s", func, line,
		   more ? ": " : "\n");
	if (more) {
		va_start(args, fmt);
		pp_vsprintf(message + strlen(message), fmt, args);
		va_end(args);
	}

	while (1) {
		pp_printf(message);
		if (!forever)
                  break;

                assertWait(1000 * 1000);
	}
}
