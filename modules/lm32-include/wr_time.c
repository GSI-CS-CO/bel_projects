/*!
 *
 * @brief Module to convert WhiteRabbit to a readable date and time format
 *
 * This module has been adopt from the Linux kernel source and customized
 * for LM32.
 *
 * @file      wr_time.c
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      08.01.2019
 *******************************************************************************
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************
 */
#include <mini_sdb.h>
#include <scu_assert.h>
#include <wr_time.h>

/**
 * div_rem - signed 64bit divide with 32bit divisor with remainder
 * @param dividend: signed 64bit dividend
 * @param divisor: signed 32bit divisor
 * @param pRemainder: pointer to signed 32bit remainder
 *
 * @return: sets ``*remainder``, then returns dividend / divisor
 */
static inline int32_t div_rem( int32_t dividend, int32_t divisor, int *pRemainder )
{
   SCU_ASSERT( divisor != 0 );
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
static long math_div( long a, long b )
{
   SCU_ASSERT( b != 0 );
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

/*! ---------------------------------------------------------------------------
 * @see wr_time.h
 */
WR_PPS_T* wrGetPtr( void )
{
   WR_PPS_T* pPPSGen;   // WB address of PPS_GEN

   pPPSGen = (WR_PPS_T*)find_device_adr( WR_PPS_GEN_VENDOR, WR_PPS_GEN_PRODUCT );
   if( pPPSGen == (WR_PPS_T*)ERROR_NOT_FOUND )
      return NULL;

   return pPPSGen;
}


/*! ---------------------------------------------------------------------------
 * @see wr_time.h
 */
void wrTime2tm( WR_PPS_T* pWr, int offset, TM_T* pResult )
{
   long days, rem, y;
   int remainder;
   const unsigned short *ip;

   SCU_ASSERT( pWr != NULL );
   days = div_rem( pWr->tsv.tv_secLo, SECS_PER_DAY, &remainder );
   rem = remainder;
   rem += offset;
   while( rem < 0 )
   {
      rem += SECS_PER_DAY;
      --days;
   }
   while( rem >= SECS_PER_DAY )
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

   while( days < 0 || days >= (__isleap(y) ? 366 : 365) )
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

   /*
    * No information in day light saving flag.
    */
   pResult->tm_isdst = -1;
}

/*================================== EOF ====================================*/

