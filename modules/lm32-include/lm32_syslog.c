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
#ifndef __DOCFSM__
 #include <scu_wr_time.h>
 #include <lm32_syslog.h>
 #include <scu_mmu_tag.h>
 #include <lm32Interrupts.h>
 #include <dbg.h>
#include <mprintf.h>
#endif

MMU_OBJ_T g_mmuObj;
STATIC MMU_ADDR_T mg_adminOffset = 0;

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
   MMU_ADDR_T index = mg_adminOffset;
   for( size_t i = 0; i < SYSLOG_FIFO_ADMIN_SIZE; i++, index++ )
   {
      syslogWriteRam( index, &((RAM_PAYLOAD_T*)pAdmin)[i] );
   }
}

/*! ---------------------------------------------------------------------------
 */
STATIC void syslogReadFifoAdmin( SYSLOG_FIFO_ADMIN_T* pAdmin )
{
   MMU_ADDR_T index = mg_adminOffset;
   for( size_t i = 0; i < SYSLOG_FIFO_ADMIN_SIZE; i++, index++ )
   {
      syslogReadRam( index, &((RAM_PAYLOAD_T*)pAdmin)[i] );
   }
   ramRingDbgPrintIndexes( &pAdmin->admin.indexes, "read" );
}

/*! ---------------------------------------------------------------------------
 * @see lm32_syslog.h
 */
MMU_STATUS_T syslogInit( unsigned int numOfItems )
{
   MMU_STATUS_T status;

   if( (status = mmuInit( &g_mmuObj )) != OK )
      return status;

#ifdef CONFIG_LOG_TEST
   mmuDelete();
#endif
   numOfItems *= SYSLOG_FIFO_ITEM_SIZE;
   numOfItems += SYSLOG_FIFO_ADMIN_SIZE;

   status = mmuAlloc( TAG_LM32_LOG, &mg_adminOffset, &numOfItems, true );
   if( status != OK )
      return status;

   SYSLOG_FIFO_ADMIN_T fifoAdmin =
   {
      .admin.indexes.offset   = mg_adminOffset + SYSLOG_FIFO_ADMIN_SIZE,
      .admin.indexes.capacity = numOfItems     - SYSLOG_FIFO_ADMIN_SIZE,
      .admin.indexes.start    = 0,
      .admin.indexes.end      = 0,
      .admin.wasRead          = 0
   };

   DBPRINT1( "offset: %u\ncapacity: %u\n",
             fifoAdmin.admin.indexes.offset,
             fifoAdmin.admin.indexes.capacity );

   syslogWriteFifoAdmin( &fifoAdmin );

   return status;
}


/*! ---------------------------------------------------------------------------
 */
STATIC inline void syslogPushItem( const SYSLOG_FIFO_ITEM_T* pItem )
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
 * @brief Returns true if the character is a decimal digit.
 */
STATIC inline bool isDecDigit( const char c )
{
   return (c >= '0') && (c <= '9');
}

STATIC inline bool isPaddingChar( const char c )
{
   switch( c )
   {
      case '0': /* No break here! */
      case ' ': /* No break here! */
      case '.': /* No break here! */
      case '_':
      {
         return true;
      }
   }
   return false;
}


#define FSM_DECLARE_STATE( newState, attr... ) newState
#define FSM_INIT_FSM( initState, attr... ) STATE_T state = initState
#define FSM_TRANSITION( target, attr... ) { state = target; break; }
#define FSM_TRANSITION_NEXT( target, attr... ) { state = target; next = true; break; }
#define FSM_TRANSITION_SELF( attr...) break

STATIC_ASSERT( sizeof(char*) == sizeof(uint32_t) );

typedef enum
{
   FSM_DECLARE_STATE( NORMAL, color=blue ),
   FSM_DECLARE_STATE( PADDING_CHAR, color=green ),
   FSM_DECLARE_STATE( PADDING_SIZE, color=cyan ),
   FSM_DECLARE_STATE( PARAM, color=magenta )
} STATE_T;



#if 1
void printStates( STATE_T state )
{
   char* str = "unknown";
   #define _CASE_STATE( s ) s: str = #s; break;
   #define CASE_STATE( s ) _CASE_STATE( s )
   switch( state )
   {
      //_CASE_STATE( NORMAL )
      case NORMAL:       str = "NORMAL";       break;
      case PADDING_CHAR: str = "PADDING_CHAR"; break;
      case PADDING_SIZE: str = "PADDING_SIZE"; break;
      case PARAM:        str = "PARAM";        break;
   }
   mprintf( "State: %s\n", str );
}
#endif

/*! ---------------------------------------------------------------------------
 * @see lm32_syslog.h
 */
STATIC void inline vsyslog( uint32_t filter, const char* format, va_list ap )
{
   mprintf( "'%s' %s\n", format, __func__ );

   const uint64_t timestamp = getWrSysTimeSafe();

   SYSLOG_FIFO_ITEM_T item =
   {
   #if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
      .timestamp = MERGE_HIGH_LOW( GET_LOWER_HALF( timestamp ),
                         (uint32_t)GET_UPPER_HALF( timestamp ) ),
   #else
      .timestamp = timestamp,
   #endif
      .filter = filter,
      .format = (uint32_t)format
   };

   FSM_INIT_FSM( NORMAL, color=blue );
   unsigned int ai = 0;
   while( (*format != '\0') && (ai < ARRAY_SIZE(item.param)) )
   {
      mprintf( "'%c'", *format );
      bool next;
      do
      {
         next = false;
         printStates( state );
         switch( state )
         {
            case NORMAL:
            {
               if( *format == '%' )
                  FSM_TRANSITION( PADDING_CHAR );
               FSM_TRANSITION_SELF();
            }

            case PADDING_CHAR:
            {
               if( *format == '%' )
                  FSM_TRANSITION( NORMAL );

               if( isPaddingChar( *format ) )
                  FSM_TRANSITION( PADDING_SIZE );

               if( isDecDigit( *format ) )
                  FSM_TRANSITION_NEXT( PADDING_SIZE );

               FSM_TRANSITION_NEXT( PARAM );
            }

            case PADDING_SIZE:
            {
               if( isDecDigit( *format ) )
                  FSM_TRANSITION_SELF();
               FSM_TRANSITION_NEXT( PARAM );
            }

            case PARAM:
            {
               mprintf( "Type: %c\n", *format );
               switch( *format )
               {
                  case 'S': /* No break here! */
                  case 's': /* No break here! */
                  case 'c': /* No break here! */
                  case 'X': /* No break here! */
                  case 'x': /* No break here! */
                  case 'p': /* No break here! */
                  case 'i': /* No break here! */
                  case 'd': /* No break here! */
                  case 'u': /* No break here! */
                  case 'o': /* No break here! */
               #ifndef CONFIG_NO_BINARY_PRINTF_FORMAT
                  case 'b':
               #endif
                  {

                     item.param[ai] = va_arg( ap, typeof(item.param[0]) );
                     mprintf( "Param[%u] = 0x%08X, %d\n", ai, item.param[ai], item.param[ai] );
                     ai++;
                     break;
                  }
               }
               FSM_TRANSITION( NORMAL );
            }
         }
      }
      while( next );
      format++;
   }

   syslogPushItem( &item );
}

/*! ---------------------------------------------------------------------------
 * @see lm32_syslog.h
 */
void syslog( uint32_t filter, const char* format, ... )
{
   //mprintf( "'%s' %s\n", format, __func__ );
   va_list ap;
   va_start( ap, format );
   vsyslog( filter, format, ap );
   va_end( ap );
}

/*================================== EOF ====================================*/