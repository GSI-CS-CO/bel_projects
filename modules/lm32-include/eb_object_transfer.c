/*!
 * @brief     Some helper functions for simplifying the
 *            data transfer of flat objects of types struct, union or class
 *            via wishbone/etherbone bus.
 *
 * @note
 * Flat objects means: the object doesn't contain members of type pointer or
 * reverence.
 *
 * Based on etherbone.h
 *
 * @file      eb_object_transfer.c
 * @see       eb_object_transfer.h
 * @see       etherbone.h
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      21.02.2019
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
#include <eb_object_transfer.h>
#include <dbg.h>
#include <sdb_ids.h>

#define ATTEMPTS 3

uint32_t g_lm32Base;

/*! ---------------------------------------------------------------------------
 */
eb_status_t ebOpen( EB_HANDLE_T* pThis, const char* name )
{
   SCU_ASSERT( name != NULL );
   SCU_ASSERT( strlen( name ) > 0 );

   pThis->status = eb_socket_open( EB_ABI_CODE, 0, EB_DATAX | EB_ADDRX,
                            &pThis->socket );
   if( pThis->status != EB_OK )
   {
      fprintf( stderr, ESC_FG_RED ESC_BOLD
               "Error: eb_socket_open \"%s\" returns %s\n"ESC_NORMAL,
               name, ebGetStatusString( pThis ) );
      return pThis->status;
   }

   pThis->status = eb_device_open( pThis->socket, name,
                                   EB_DATAX | EB_ADDRX, ATTEMPTS,
                                   &pThis->device );
   if( pThis->status != EB_OK )
   {
      fprintf( stderr, ESC_FG_RED ESC_BOLD
              "Error: eb_device_open \"%s\" returns %s\n"ESC_NORMAL,
               name, ebGetStatusString( pThis ));
      eb_socket_close( pThis->socket );
   }
   if( pThis->status != EB_OK )
      return pThis->status;

   pThis->status = ebFindFirstDeviceAddrById( pThis, GSI, LM32_RAM_USER,
                                              &g_lm32Base );
   return pThis->status;
}


/*! ---------------------------------------------------------------------------
 */
eb_status_t ebClose( EB_HANDLE_T* pThis )
{
  if( (pThis->status = eb_device_close(pThis->device)) != EB_OK)
  {
     fprintf( stderr, ESC_FG_RED ESC_BOLD
                      "Error: eb_device_close returns %s\n"ESC_NORMAL,
                      ebGetStatusString( pThis ));
     return pThis->status;
  }

  if( (pThis->status = eb_socket_close(pThis->socket)) != EB_OK)
  {
     fprintf( stderr,
              ESC_FG_RED ESC_BOLD"Error: eb_socket_close returns %s\n"
              ESC_NORMAL, ebGetStatusString( pThis ));
  }
  return pThis->status;
}

/*! ---------------------------------------------------------------------------
 */
eb_status_t ebFindFirstDeviceAddrById( EB_HANDLE_T* pThis,
                                       uint64_t vendorId, uint32_t deviceId,
                                       uint32_t* pDevAddr )
{
   struct sdb_device devices[1];
   int numDevices = ARRAY_SIZE( devices );

   if( eb_sdb_find_by_identity( pThis->device, vendorId, deviceId, devices,
                               &numDevices ) != EB_OK )
   {
      fprintf( stderr, ESC_FG_RED ESC_BOLD"Error: eb_sdb_find_by_identity"
                       " returns %s\n"ESC_NORMAL,
                       ebGetStatusString( pThis ) );
      return pThis->status;
   }
   if( numDevices == 0 )
   {
      fprintf( stderr,
               ESC_FG_RED ESC_BOLD"Error: no matching device found:\n"
                       "Vendor ID: 0x%08X\n"
                       "Device ID: 0x%08X\n"ESC_NORMAL,
                       vendorId, deviceId );
      pThis->status = EB_SEGFAULT;
      return pThis->status;
   }

   *pDevAddr = devices[0].sdb_component.addr_first;

   DBPRINT1( "DBG: INFO: Found device at addr: 0x%08X\n", *pDevAddr );

   return pThis->status;
}

/*! ---------------------------------------------------------------------------
 */
eb_status_t ebReadData32( EB_HANDLE_T* pThis, uint32_t addr, uint32_t* pData,
                          size_t len )
{
   if( len == 0 )
   {
      pThis->status = EB_OK;
      return pThis->status;
   }

   EB_MEMBER_INFO_T info[len];
   for( size_t i = 0; i < len; i++ )
   {
      info[i].pData = (uint8_t*)&pData[i];
      info[i].size = sizeof( uint32_t );
   }

   EB_CYCLE_OR_CB_ARG_T arg;
   arg.aInfo   = info;
   arg.infoLen = len;
   arg.exit    = false;

   if( ebObjectReadCycleOpen( pThis, &arg ) != EB_OK )
   {
      fprintf( stderr, ESC_FG_RED ESC_BOLD
                       "Error: failed to create cycle for read: %s\n"
                       ESC_NORMAL,
                       ebGetStatusString( pThis ));
      return pThis->status;
   }

   for( size_t i = 0; i < len; i++, addr += sizeof(uint32_t) )
   {
      eb_cycle_read( pThis->cycle, addr, EB_DATA32 | EB_LITTLE_ENDIAN, NULL );
   }

   ebCycleClose( pThis );

   while( !arg.exit )
      ebSocketRun( pThis );

   pThis->status = arg.status;

   return pThis->status;
}

/*! ---------------------------------------------------------------------------
 */
eb_status_t ebWriteData32( EB_HANDLE_T* pThis, uint32_t addr, uint32_t* pData,
                           size_t len )
{
   if( len == 0 )
   {
      pThis->status = EB_OK;
      return pThis->status;
   }

   EB_MAKE_CB_OW_ARG( cArg );

   if( ebObjectWriteCycleOpen( pThis, &cArg ) != EB_OK )
   {
      fprintf( stderr, ESC_FG_RED ESC_BOLD
                       "Error: Failed to create cycle for write: %s\n"
                       ESC_NORMAL,
               ebGetStatusString( pThis ));
      return pThis->status;
   }

   for( size_t i = 0; i < len; i++ )
   {
      eb_cycle_write( pThis->cycle, addr, EB_DATA32 | EB_LITTLE_ENDIAN,
                      pData[i] );
   }

   ebCycleClose( pThis );

   while( !cArg.exit )
      ebSocketRun( pThis );

   pThis->status = cArg.status;
   return pThis->status;
}


/*! --------------------------------------------------------------------------
 */
void __ebCycleReadIoObjectCb( eb_user_data_t user, eb_device_t dev,
                              eb_operation_t op, eb_status_t status )
{
   ((EB_CYCLE_OR_CB_ARG_T*)user)->exit   = true;
   ((EB_CYCLE_OR_CB_ARG_T*)user)->status = status;

   if( status != EB_OK )
   {
      fprintf( stderr, ESC_FG_RED ESC_BOLD
               "ERROR: Callback function %s called by status: %s\n"ESC_NORMAL,
              __func__, eb_status(status));
      return;
   }

   size_t i = 0;
   while( (op != EB_NULL) && (i < ((EB_CYCLE_OR_CB_ARG_T*)user)->infoLen) )
   {
      size_t size = eb_operation_format(op) & EB_DATAX;
      SCU_ASSERT( ((EB_CYCLE_OR_CB_ARG_T*)user)->aInfo[i].size == size );
      eb_data_t data = eb_operation_data( op );
      memcpy( ((EB_CYCLE_OR_CB_ARG_T*)user)->aInfo[i++].pData, &data, size );
      op = eb_operation_next( op );
   }
}

/*! ---------------------------------------------------------------------------
 */
void __ebCycleWriteIoObjectCb( eb_user_data_t user, eb_device_t dev,
                              eb_operation_t op, eb_status_t status )
{
   ((EB_CYCLE_OW_CB_ARG_T*)user)->exit   = true;
   ((EB_CYCLE_OW_CB_ARG_T*)user)->status = status;

   if( status != EB_OK )
   {
      fprintf( stderr, ESC_FG_RED ESC_BOLD
               "ERROR: Callback function %s called by sratus : %s\n"ESC_NORMAL,
               __func__, eb_status( status ));
      return;
   }

   while( op != EB_NULL )
   {
      if( eb_operation_had_error( op ) )
      {
         fprintf( stderr, ESC_FG_RED ESC_BOLD
                  "ERROR: Wishbone segfault %s %s %s bits to address 0x%"
                  EB_ADDR_FMT"\n"ESC_NORMAL,
                  eb_operation_is_read(op)?"reading":"writing",
                  eb_width_data(eb_operation_format(op)),
                  eb_format_endian(eb_operation_format(op)),
                  eb_operation_address(op));
         ((EB_CYCLE_OW_CB_ARG_T*)user)->status = EB_SEGFAULT;
      }
      op = eb_operation_next( op );
   }

}

/*================================== EOF ====================================*/
