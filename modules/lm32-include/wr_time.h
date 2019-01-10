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

#ifndef CONFIG_WR_NODE
  #error Compiler switch CONFIG_WR_NODE has to be defined in your Makefile!
#endif

#include <stdint.h>
#include <helper_macros.h>
#include <wb_slaves.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef WR_PPS_GEN_ADJ_NSEC
  #define WR_PPS_GEN_ADJ_NSEC   0x10
#endif
#ifndef WR_PPS_GEN_ADJ_UTCLO
  #define WR_PPS_GEN_ADJ_UTCLO  0x14
#endif
#ifndef WR_PPS_GEN_ADJ_UTCHI
  #define WR_PPS_GEN_ADJ_UTCHI  0x18
#endif

#define SECS_PER_HOUR   (60 * 60)
#define SECS_PER_DAY    (SECS_PER_HOUR * 24)

/*!
 * @brief Counter registers
 */
typedef struct PACKED_SIZE
{
   uint32_t tv_nsec;   /*!< @brief [0x4]: REG Nanosecond counter register */
   uint32_t tv_secLo;  /*!< @brief [0x8]: REG UTC Counter register (least-significant part)*/
   uint32_t tv_secHi;  /*!< @brief [0xc]: REG UTC Counter register (most-significant part) */
} volatile  TIMESPEC_T;
STATIC_ASSERT( sizeof( TIMESPEC_T ) == 3 * sizeof(uint32_t) );
STATIC_ASSERT( offsetof( TIMESPEC_T, tv_secLo ) == (WR_PPS_GEN_CNTR_UTCLO-WR_PPS_GEN_CNTR_NSEC) );
STATIC_ASSERT( offsetof( TIMESPEC_T, tv_secHi ) == (WR_PPS_GEN_CNTR_UTCHI-WR_PPS_GEN_CNTR_NSEC) );

/*!
 * @brief Adjustment registers
 */
typedef struct PACKED_SIZE
{
   uint32_t nsec;   /*!< @brief [0x10]: REG Nanosecond adjustment register */
   uint32_t utcLo;  /*!< @brief [0x14]: REG UTC Adjustment register (least-significant part) */
   uint32_t utcHi;  /*!< @brief [0x18]: REG UTC Adjustment register (most-significant part) */
} volatile WR_ADJ_T;
STATIC_ASSERT( sizeof( WR_ADJ_T ) == 3 * sizeof(uint32_t) );
STATIC_ASSERT( offsetof( WR_ADJ_T, utcLo ) == (WR_PPS_GEN_ADJ_UTCLO-WR_PPS_GEN_ADJ_NSEC) );
STATIC_ASSERT( offsetof( WR_ADJ_T, utcHi ) == (WR_PPS_GEN_ADJ_UTCHI-WR_PPS_GEN_ADJ_NSEC) );

/*!
 * @brief White Rabbit time structure
 * @see wb_slaves.h
 */
typedef struct PACKED_SIZE
{
   uint32_t   cr;   /*!< @brief [0x0]: REG Control Register */
   TIMESPEC_T tsv;  /*!< @brief Registers for separated access of seconds and nanoseconds */
   WR_ADJ_T   adj;  /*!< @brief Adjustment registers */
   uint32_t   escr; /*!< @brief [0x1c]: REG External sync control register */
} volatile WR_PPS_T;
STATIC_ASSERT( offsetof( WR_PPS_T, cr ) == 0 );
STATIC_ASSERT( offsetof( WR_PPS_T, tsv ) == WR_PPS_GEN_CNTR_NSEC );
STATIC_ASSERT( offsetof( WR_PPS_T, adj ) == WR_PPS_GEN_ADJ_NSEC );
STATIC_ASSERT( offsetof( WR_PPS_T, escr ) == WR_PPS_GEN_ESCR );

/*!
 * @brief the time structure defined in <time.h>
 * @see time.h
 */
typedef struct tm TM_T;

/*! ---------------------------------------------------------------------------
 * @brief Returns a pointer to the value of the White Rabbit timer.
 * @see WR_PPS_T
 * @retval ==NULL Function was not successful
 * @retval !=NULL Pointer to the White Rabbit time structure
 */
WR_PPS_T* wrGetPtr( void );

/*! ---------------------------------------------------------------------------
 * @brief Converts the seconds part of the White Rabbit time into
 *        the ANSI time format.
 * @see WR_PPS_T
 * @param totalsecs the number of seconds elapsed since 00:00:00 on
 *                  January 1, 1970, Coordinated Universal Time (UTC).
 * @param offset  offset seconds adding to totalsecs.
 * @result pointer to struct TM_T variable to receive broken-down time
 */
void wrTime2tm( WR_PPS_T* pWr, int offset, TM_T* pResult );

#ifdef __cplusplus
}
#endif

#endif /* ifndef _WR_TIME_H */
/*================================== EOF ====================================*/

