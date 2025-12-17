/* syscalls_unity.c - "Unity-only" Newlib syscall glue
 *
 * Routes libc write-data into your pp_printf so Unity output uses
 * your existing UART-backed printing implementation.
 *
 * Assumes linker script provides:
 *   extern char _ebss;    // heap start
 *   extern char _endram;  // heap end (stack start)
 *
 * Also assumes you have a pp_printf implementation already linked:
 *   int pp_printf(const char *fmt, ...);
 *
 * Drop this in your build and link the object into the final ELF.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stddef.h>
#include <stdarg.h>

/* Linker-provided heap bounds */
extern char _ebss;
extern char _endram;

/* Existing printf implementation provided by your linker script:
 * PROVIDE(printf = pp_printf);
 * We declare pp_printf explicitly so we can call it from _write.
 */
extern int pp_printf(const char *fmt, ...);

/* Minimal heap pointer */
static char *heap_ptr = &_ebss;

/* sbrk: used by malloc()/free() */
void * _sbrk(ptrdiff_t incr)
{
    char *prev = heap_ptr;
    char *next = heap_ptr + incr;

    if (next >= &_endram) {
        errno = ENOMEM;
        return (void *) -1;
    }
    heap_ptr = next;
    return (void *) prev;
}

/* _write: route libc output into pp_printf.
 * The buffer passed may contain arbitrary bytes (not necessarily NUL-terminated),
 * so we use a precision-limited format specifier.
 */
int _write(int fd, const void *buf, size_t count)
{
    (void)fd; /* ignore fd - treat all as console output */

    /* Safety: if pp_printf is heavy or uses malloc, be cautious.
       We assume pp_printf writes directly to UART. */
    if (count == 0) return 0;

    /* Print as a string with length limit. Use %.*s to avoid needing a
       temporary buffer. Cast count to int — if count might exceed INT_MAX,
       clamp it (very unlikely on embedded). */
    int len = (count > 0x7fffffff) ? 0x7fffffff : (int)count;
    pp_printf("%.*s", len, (const char *)buf);

    return (int)count;
}

/* Minimal read: no input available for tests by default */
int _read(int fd, void *buf, size_t count)
{
    (void)fd; (void)buf; (void)count;
    return 0; /* no input */
}

/* Other benign stubs */
int _close(int fd) { (void)fd; return 0; }

int _fstat(int fd, struct stat *st)
{
    (void)fd;
    if (st) st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int fd) { (void)fd; return 1; }

/* lseek returns 0 (offset unchanged) */
off_t _lseek(int fd, off_t offset, int whence)
{
    (void)fd; (void)offset; (void)whence;
    return 0;
}
