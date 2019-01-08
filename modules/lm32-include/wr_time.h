/*!
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
#include <helper_macros.h>
#include <wb_slaves.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SECS_PER_HOUR   (60 * 60)
#define SECS_PER_DAY    (SECS_PER_HOUR * 24)

typedef struct
{
   uint32_t tv_nsec; /*!< @brief nanoseconds */
   uint32_t tv_sec;  /*!< @brief seconds     */
} volatile TIMESPEC_T;
STATIC_ASSERT( sizeof( TIMESPEC_T ) == sizeof(uint64_t) );
STATIC_ASSERT( offsetof( TIMESPEC_T, tv_sec ) == (WR_PPS_GEN_CNTR_UTCLO-WR_PPS_GEN_CNTR_NSEC) );

/*!
 * @brief White Rabbit time structure
 */
typedef union
{
   uint64_t   wrv; /*!< @brief 64 bit White Rabbit value */
   TIMESPEC_T tsv; /*!< @brief Value for separated access of seconds and nanoseconds */
} volatile WR_TIME_T;
STATIC_ASSERT( sizeof( WR_TIME_T ) == sizeof(uint64_t) );

/*!
 * @brief the time structure defined in <time.h>
 * @see time.h
 */
typedef struct tm TM_T;

/*! ---------------------------------------------------------------------------
 * @brief Returns a pointer to the value of the White Rabbit timer.
 * @see WR_TIME_T
 * @retval ==NULL Function was not successful
 * @retval !=NULL Pointer to the White Rabbit time structure
 */
WR_TIME_T* wrGetPtr( void );

/*! ---------------------------------------------------------------------------
 * @brief Converts the seconds part of the White Rabbit time into
 *
 * @see WR_TIME_T
 * @param totalsecs the number of seconds elapsed since 00:00:00 on
 *                  January 1, 1970, Coordinated Universal Time (UTC).
 * @param offset  offset seconds adding to totalsecs.
 * @result pointer to struct TM_T variable to receive broken-down time
 */
void wrTime2tm( WR_TIME_T* pWr, int offset, TM_T* pResult );

#ifdef __cplusplus
}
#endif

#endif /* ifndef _WR_TIME_H */
/*================================== EOF ====================================*/

