/*!
 *
 * @brief Module to convert WhiteRabbit to a readable date and time format
 *
 * This module has been adopt from the Linux kernel source and customized
 * for LM32.
 *
 * @file      wr_time.h
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
#ifndef _WR_TIME_H
#define _WR_TIME_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int32_t SECONDS_T;

#define SECS_PER_HOUR   (60 * 60)
#define SECS_PER_DAY    (SECS_PER_HOUR * 24)

/*!
 * Similar to the struct tm in userspace <time.h>, but it needs to be here so
 * that the kernel source is self contained.
 */
struct TM_T
{
   /*!
    * @brief the number of seconds after the minute, normally in the range
    *        0 to 59, but can be up to 60 to allow for leap seconds
    */
   int tm_sec;
   /*! @brief the number of minutes after the hour, in the range 0 to 59*/
   int tm_min;
   /*! @brief the number of hours past midnight, in the range 0 to 23 */
   int tm_hour;
   /*! @brief the day of the month, in the range 1 to 31 */
   int tm_mday;
   /*! @brief the number of months since January, in the range 0 to 11 */
   int tm_mon;
   /*! @brief the number of years since 1900 */
   long tm_year;
   /*! @brief the number of days since Sunday, in the range 0 to 6 */
   int tm_wday;
   /*! @brief the number of days since January 1, in the range 0 to 365 */
   int tm_yday;
};

/**
 * @brief converts the calendar time to local broken-down time
 *
 * @param totalsecs the number of seconds elapsed since 00:00:00 on
 *                  January 1, 1970, Coordinated Universal Time (UTC).
 * @param offset  offset seconds adding to totalsecs.
 * @result pointer to struct TM_T variable to receive broken-down time
 */
void wrTime2tm( SECONDS_T totalsecs, int offset, struct TM_T* pResult );

#ifdef __cplusplus
}
#endif

#endif /* ifndef _WR_TIME_H */
/*================================== EOF ====================================*/

