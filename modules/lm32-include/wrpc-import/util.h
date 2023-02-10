/*
 * This work is part of the White Rabbit project
 *
 * Released according to the GNU GPL, version 2 or any later version.
 */
#ifndef __UTIL_H
#define __UTIL_H

#include <stdint.h>

/* Color codes for cprintf()/pcprintf() */
#define C_DIM 0x80

#define C_RED 1
#define C_GREEN 2
#define C_BLUE 4
#define C_MAGENTA 5
#define C_CYAN 6
#define C_GREY 7
/* Default foreground color, White or Black depends on User's terminal */
#define C_WHITE 9

/* Return TAI date/time in human-readable form. Non-reentrant. */
char *format_time(uint64_t sec, int format);
#define TIME_FORMAT_LEGACY 0
#define TIME_FORMAT_SYSLOG 1
#define TIME_FORMAT_SORTED 2

typedef struct
{
    uint32_t start_tics;
    uint32_t timeout;
} timeout_t;

/* Color printf() variant. Does not restore color */
void cprintf(int color, const char *fmt, ...);

/* Color printf() variant, sets curspor position to (row, col) too.
 * Does not restore color */
void pcprintf(int row, int col, int color, const char *fmt, ...);

/* Printf, sets curspor position to (row, col) */
void pprintf(int row, int col, const char *fmt, ...);

void __debug_printf(const char *fmt, ...);

/* Clears the terminal screen */
void term_clear(void);

/* Clears the terminal screen from cursor to the end */
void term_clear_to_end(void);

int tmo_init(timeout_t *tmo, uint32_t milliseconds);
int tmo_restart(timeout_t *tmo);
int tmo_expired(timeout_t *tmo);

int atoi(const char *s);

const char *fromhex(const char *hex, int *v);
const char *fromhex64(const char *hex, int64_t *v);
const char *fromdec(const char *dec, int *v);

char *format_mac(char *s, const unsigned char *mac);
char *format_hex8(char *s, const unsigned char *mac);

void decode_mac(const char *str, unsigned char *mac);
void decode_port(const char *str, int *port);

/* div64.c, lifted from the linux kernel through pp_printf or ppsi */
extern uint32_t __div64_32(uint64_t *n, uint32_t base);

static inline int within_range(int x, int minval, int maxval, int wrap)
{
    int rv;

    //printf("min %d max %d x %d ", minval, maxval, x);

    while (maxval >= wrap)
        maxval -= wrap;

    while (maxval < 0)
        maxval += wrap;

    while (minval >= wrap)
        minval -= wrap;

    while (minval < 0)
        minval += wrap;

    while (x < 0)
        x += wrap;

    while (x >= wrap)
        x -= wrap;

    if (maxval > minval)
        rv = (x >= minval && x <= maxval) ? 1 : 0;
    else
        rv = (x >= minval || x <= maxval) ? 1 : 0;

    return rv;
}


#endif
