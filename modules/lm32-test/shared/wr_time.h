
#ifndef _WR_TIME_H
#define _WR_TIME_H

#include <stdint.h>

typedef int64_t s64;
typedef uint64_t  u64;
typedef int32_t s32;
typedef int64_t SECONDS_T;

/*
 * Similar to the struct tm in userspace <time.h>, but it needs to be here so
 * that the kernel source is self contained.
 */
struct tm
{
   /*
    * the number of seconds after the minute, normally in the range
    * 0 to 59, but can be up to 60 to allow for leap seconds
    */
   int tm_sec;
   /* the number of minutes after the hour, in the range 0 to 59*/
   int tm_min;
   /* the number of hours past midnight, in the range 0 to 23 */
   int tm_hour;
   /* the day of the month, in the range 1 to 31 */
   int tm_mday;
   /* the number of months since January, in the range 0 to 11 */
   int tm_mon;
   /* the number of years since 1900 */
   long tm_year;
   /* the number of days since Sunday, in the range 0 to 6 */
   int tm_wday;
   /* the number of days since January 1, in the range 0 to 365 */
   int tm_yday;
};

void time64_to_tm( SECONDS_T totalsecs, int offset, struct tm* pResult);

#endif