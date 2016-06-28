/*
 * This work is part of the White Rabbit project
 *
 * Copyright (C) 2012 CERN (www.cern.ch)
 * Author: Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
 *
 * Released according to the GNU GPL, version 2 or any later version.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <time.h>
#include <wrc.h>

#include "util.h"

/* cut from libc sources */

#define 	YEAR0   1900
#define 	EPOCH_YR   1970
#define 	SECS_DAY   (24L * 60L * 60L)
#define 	LEAPYEAR(year)   (!((year) % 4) && (((year) % 100) || !((year) % 400)))
#define 	YEARSIZE(year)   (LEAPYEAR(year) ? 366 : 365)
#define 	FIRSTSUNDAY(timp)   (((timp)->tm_yday - (timp)->tm_wday + 420) % 7)
#define 	FIRSTDAYOF(timp)   (((timp)->tm_wday - (timp)->tm_yday + 420) % 7)
#define 	TIME_MAX   ULONG_MAX
#define 	ABB_LEN   3

static const char *_days[] = {
	"Sun", "Mon", "Tue", "Wed",
	"Thu", "Fri", "Sat"
};

static const char *_months[] = {
	"Jan", "Feb", "Mar",
	"Apr", "May", "Jun",
	"Jul", "Aug", "Sep",
	"Oct", "Nov", "Dec"
};

static const int _ytab[2][12] = {
	{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
	{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

char *format_time(uint64_t sec)
{
	struct tm t;
	static char buf[64];
	unsigned long dayclock, dayno;
	int year = EPOCH_YR;

	dayclock = (unsigned long)sec % SECS_DAY;
	dayno = (unsigned long)sec / SECS_DAY;

	t.tm_sec = dayclock % 60;
	t.tm_min = (dayclock % 3600) / 60;
	t.tm_hour = dayclock / 3600;
	t.tm_wday = (dayno + 4) % 7;	/* day 0 was a thursday */
	while (dayno >= YEARSIZE(year)) {
		dayno -= YEARSIZE(year);
		year++;
	}
	t.tm_year = year - YEAR0;
	t.tm_yday = dayno;
	t.tm_mon = 0;
	while (dayno >= _ytab[LEAPYEAR(year)][t.tm_mon]) {
		dayno -= _ytab[LEAPYEAR(year)][t.tm_mon];
		t.tm_mon++;
	}
	t.tm_mday = dayno + 1;
	t.tm_isdst = 0;

	//sprintf(buf, "%s, %s %d, %d, %02d:%02d:%02d", _days[t.tm_wday],_months[t.tm_mon], t.tm_mday, t.tm_year + YEAR0, t.tm_hour,t.tm_min, t.tm_sec);
	mprintf("%s, %s %d, %d, %02d:%02d:%02d", _days[t.tm_wday],_months[t.tm_mon], t.tm_mday, t.tm_year + YEAR0, t.tm_hour,t.tm_min, t.tm_sec);

	return buf;
}

void cprintf(int color, const char *fmt, ...)
{
	va_list ap;
	pp_printf("\e[0%d;3%dm", color & C_DIM ? 2 : 1, color & 0x7f);
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	pp_printf("\e[m");
}

void pcprintf(int row, int col, int color, const char *fmt, ...)
{
	va_list ap;
	pp_printf("\e[%d;%df", row, col);
	pp_printf("\e[0%d;3%dm", color & C_DIM ? 2 : 1, color & 0x7f);
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	pp_printf("\e[m");
}

void term_clear(void)
{
	pp_printf("\e[2J\e[1;1H");
}
