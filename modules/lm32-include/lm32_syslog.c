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
#include <scu_mmu_tag.h>
#include <lm32Interrupts.h>

STATIC_ASSERT( sizeof(char*) == sizeof(uint32_t) );

MMU_OBJ_T g_mmuObj;

/*! ---------------------------------------------------------------------------
 */
STATIC inline void syslogWriteRam( unsigned int index, const RAM_PAYLOAD_T* pData )
{
   criticalSectionEnter();
   ddr3write64( &g_mmuObj, index, pData );
   criticalSectionExit();
}

/*! ---------------------------------------------------------------------------
 */
STATIC inline void syslogReadRam( unsigned int index, RAM_PAYLOAD_T* pData )
{
   criticalSectionEnter();
   ddr3read64( &g_mmuObj, pData, index );
   criticalSectionExit();
}

/*! ---------------------------------------------------------------------------
 */
STATIC void syslogWriteFifoAdmin( const SYSLOG_FIFO_ADMIN_T* pAdmin )
{
   unsigned int index = pAdmin->admin.indexes.offset - SYSLOG_FIFO_ADMIN_SIZE;
   for( size_t i = 0; i < SYSLOG_FIFO_ADMIN_SIZE; i++, index++ )
   {
      syslogWriteRam( index, &((RAM_PAYLOAD_T*)pAdmin)[i] );
   }
}

/*! ---------------------------------------------------------------------------
 */
STATIC void syslogReadFifoAdmin( SYSLOG_FIFO_ADMIN_T* pAdmin )
{
   unsigned int index = pAdmin->admin.indexes.offset - SYSLOG_FIFO_ADMIN_SIZE;
   for( size_t i = 0; i < SYSLOG_FIFO_ADMIN_SIZE; i++, index++ )
   {
      syslogReadRam( index, &((RAM_PAYLOAD_T*)pAdmin)[i] );
   }
}

/*! ---------------------------------------------------------------------------
 */
STATIC void syslogPushItem( const SYSLOG_FIFO_ITEM_T* pItem )
{
   SYSLOG_FIFO_ADMIN_T admin;

   syslogReadFifoAdmin( &admin );

   /*
    * Removing the items which has been probably read by the Linux daemon.
    */
   sysLogFifoSynchonizeReadIndex( &admin );

   /*
    * Is enough space for the new item?
    */
   if( sysLogFifoGetRemainingItemCapacity( &admin ) == 0 )
   { /*
      * No, deleting the oldest item to make space.
      */
      sysLogFifoAddToReadIndex( &admin, SYSLOG_FIFO_ITEM_SIZE );
   }

   for( size_t i = 0; i < SYSLOG_FIFO_ITEM_SIZE; i++ )
   {
      syslogWriteRam( sysLogFifoGetWriteIndex( &admin ), &((RAM_PAYLOAD_T*)pItem)[i] );
      sysLogFifoIncWriteIndex( &admin );
   }

   syslogWriteFifoAdmin( &admin );
}

/*! ---------------------------------------------------------------------------
 * @see lm32_syslog.h
 */
MMU_STATUS_T syslogInit( unsigned int numOfItems )
{
   MMU_STATUS_T status;

   if( (status = mmuInit( &g_mmuObj )) != OK )
      return status;

   numOfItems *= SYSLOG_FIFO_ITEM_SIZE;
   numOfItems += SYSLOG_FIFO_ADMIN_SIZE;

   MMU_ADDR_T startAddr;
   status = mmuAlloc( TAG_LM32_LOG, &startAddr, &numOfItems, true );
   if( status != OK )
      return status;

   SYSLOG_FIFO_ADMIN_T fifoAdmin =
   {
      .admin.indexes.offset   = startAddr  + SYSLOG_FIFO_ADMIN_SIZE,
      .admin.indexes.capacity = numOfItems - SYSLOG_FIFO_ADMIN_SIZE,
      .admin.indexes.start    = 0,
      .admin.indexes.end      = 0,
      .admin.wasRead          = 0
   };

   syslogWriteFifoAdmin( &fifoAdmin );

   return status;
}

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

   syslogPushItem( &item );
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