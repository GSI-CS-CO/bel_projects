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

#define ATTEMPTS 3

/*! ---------------------------------------------------------------------------
 */
eb_status_t ebOpen( EB_HANDLE_T* pThis, char* name )
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

   pThis->status = eb_device_open( pThis->socket, name, EB_DATAX | EB_ADDRX, ATTEMPTS,
                            &pThis->device );
   if( pThis->status != EB_OK )
   {
      fprintf( stderr, ESC_FG_RED ESC_BOLD
              "Error: eb_device_open \"%s\" returns %s\n"ESC_NORMAL,
               name, ebGetStatusString( pThis ));
      eb_socket_close( pThis->socket );
   }

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
     fprintf( stderr, ESC_FG_RED ESC_BOLD"Error: eb_socket_close returns %s\n"
             ESC_NORMAL, ebGetStatusString( pThis ));
  }
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
