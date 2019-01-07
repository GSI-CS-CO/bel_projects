
#include "lm32_assert.h"
#include "wr_time.h"


/**
 * div_s64_rem - signed 64bit divide with 32bit divisor with remainder
 * @dividend: signed 64bit dividend
 * @divisor: signed 32bit divisor
 * @pRemainder: pointer to signed 32bit remainder
 *
 * Return: sets ``*remainder``, then returns dividend / divisor
 */
static inline int64_t div_s64_rem( int64_t dividend, int32_t divisor, int *pRemainder )
{
   LM32_ASSERT( divisor != 0 );
   *pRemainder = dividend % divisor;
   return dividend / divisor;
}

/*
 * Nonzero if YEAR is a leap year (every 4 years,
 * except every 100th isn't, and every 400th is).
 */
static int __isleap( long year )
{
   return (year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0);
}

/* do a mathdiv for long type */
static long math_div(long a, long b)
{
   LM32_ASSERT( b != 0 );
   return a / b - (a % b < 0);
}

/* How many leap years between y1 and y2, y1 must less or equal to y2 */
static long leaps_between( long y1, long y2 )
{
   long leaps1 = math_div(y1 - 1, 4) - math_div(y1 - 1, 100)
           + math_div(y1 - 1, 400);
   long leaps2 = math_div(y2 - 1, 4) - math_div(y2 - 1, 100)
           + math_div(y2 - 1, 400);
   return leaps2 - leaps1;
}

/* How many days come before each month (0-12). */
static const unsigned short __mon_yday[2][13] =
{
   /* Normal years. */
   { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
   /* Leap years. */
   { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
};

#define SECS_PER_HOUR   (60 * 60)
#define SECS_PER_DAY    (SECS_PER_HOUR * 24)

/**
 * time64_to_tm - converts the calendar time to local broken-down time
 *
 * @totalsecs   the number of seconds elapsed since 00:00:00 on January 1, 1970,
 *              Coordinated Universal Time (UTC).
 * @offset      offset seconds adding to totalsecs.
 * @result      pointer to struct tm variable to receive broken-down time
 */
void time64_to_tm( SECONDS_T totalsecs, int offset, struct tm* pResult )
{
   long days, rem, y;
   int remainder;
   const unsigned short *ip;

   days = div_s64_rem( totalsecs, SECS_PER_DAY, &remainder );
   rem = remainder;
   rem += offset;
   while (rem < 0)
   {
      rem += SECS_PER_DAY;
      --days;
   }
   while (rem >= SECS_PER_DAY)
   {
      rem -= SECS_PER_DAY;
      ++days;
   }

   pResult->tm_hour = rem / SECS_PER_HOUR;
   rem %= SECS_PER_HOUR;
   pResult->tm_min = rem / 60;
   pResult->tm_sec = rem % 60;

   /* January 1, 1970 was a Thursday. */
   pResult->tm_wday = (4 + days) % 7;
   if (pResult->tm_wday < 0)
      pResult->tm_wday += 7;

   y = 1970;

   while (days < 0 || days >= (__isleap(y) ? 366 : 365))
   {
      /* Guess a corrected year, assuming 365 days per year. */
      long yg = y + math_div(days, 365);

      /* Adjust DAYS and Y to match the guessed year. */
      days -= (yg - y) * 365 + leaps_between(y, yg);
      y = yg;
   }

   pResult->tm_year = y - 1900;

   pResult->tm_yday = days;

   ip = __mon_yday[__isleap(y)];
   for( y = 11; days < ip[y]; y-- )
      continue;
   days -= ip[y];

   pResult->tm_mon = y;
   pResult->tm_mday = days + 1;
}