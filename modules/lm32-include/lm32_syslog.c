/*!
 * @file lm32_syslog.c
 * @brief LM32 version of syslog.
 *
 * @see       lm32_syslog.h
 * @see       lm32_syslog_common.h
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      22.04.2022
 ******************************************************************************
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */
#include <scu_wr_time.h>
#include <lm32_syslog.h>

STATIC_ASSERT( sizeof(char*) == sizeof(uint32_t) );

/*! ---------------------------------------------------------------------------
 * @see lm32_syslog.h
 */
void vsyslog( uint32_t priority, const char* format, va_list ap )
{
   uint64_t timestamp = getWrSysTimeSafe();

   SYSLOG_FIFO_ITEM_T item =
   {
   #if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
      .timestamp = MERGE_HIGH_LOW( GET_LOWER_HALF( timestamp ),
                                   (uint32_t)GET_UPPER_HALF( timestamp ) ),
   #else
      .timestamp = timestamp,
   #endif
      .priority = priority,
      .format = (uint32_t)format
   };
}

/*! ---------------------------------------------------------------------------
 * @see lm32_syslog.h
 */
void syslog( uint32_t priority, const char* format, ... )
{
   va_list ap;

   va_start( ap, format );
   vsyslog( priority, format, ap );
   va_end( ap );
}

/*================================== EOF ====================================*/