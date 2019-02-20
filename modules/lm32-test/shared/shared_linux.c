/*!
 *
 * @brief     Testprogram for using shared between LM32 and Linux.
 *
 * @file      shared_linux.c
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      19.02.2019
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
#include <eb_console_helper.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <etherbone.h>
#include <shared.h>
#include <dbg.h>
#include <stdbool.h>
#include <scu_assert.h>

#include <generated/shared_mmap.h>

#define ATTEMPTS 3

char* g_pProgramName;


#define SHARED_BASE_ADDRESS (0x100A0000 + INT_BASE_ADR + SHARED_OFFS)

#define GET_ADDR_OF_MEMBER( type, member ) \
   (SHARED_BASE_ADDRESS + offsetof( type, member ))

#define GET_SIZE_OF_MEMBER( type, member ) \
({\
   type __c; \
   sizeof( __c.member ); \
})

typedef struct
{
   eb_socket_t socket;
   eb_device_t device;
   eb_cycle_t  cycle;
   bool        exit;
} EB_HANDLE_T;

IMPLEMENT_CONVERT_BYTE_ENDIAN( uint32_t )

/*! ---------------------------------------------------------------------------
 */
eb_status_t ebOpen( EB_HANDLE_T* pThis, char* name )
{
   eb_status_t status;

   SCU_ASSERT( name != NULL );

   pThis->exit = false;
   status = eb_socket_open( EB_ABI_CODE, 0, EB_DATAX | EB_ADDRX,
                            &pThis->socket );
   if( status != EB_OK )
   {
      fprintf( stderr, ESC_FG_RED ESC_BOLD
               "Error: eb_socket_open \"%s\" returns %s\n"ESC_NORMAL,
               name, eb_status( status ) );
      return status;
   }

   status = eb_device_open( pThis->socket, name, EB_DATAX | EB_ADDRX, ATTEMPTS,
                            &pThis->device );
   if( status != EB_OK )
   {
      fprintf( stderr, ESC_FG_RED ESC_BOLD
              "Error: eb_device_open \"%s\" returns %s\n"ESC_NORMAL,
               name, eb_status( status ));
      eb_socket_close( pThis->socket );
      return status;
   }
   return EB_OK;
}


/*! ---------------------------------------------------------------------------
 */
eb_status_t ebClose( EB_HANDLE_T* pThis )
{
  eb_status_t status;

  if( (status = eb_device_close(pThis->device)) != EB_OK)
  {
     fprintf( stderr, ESC_FG_RED ESC_BOLD
                      "Error: eb_device_close returns %s\n"ESC_NORMAL,
                      eb_status(status));
     return status;
  }

  if( (status = eb_socket_close(pThis->socket)) != EB_OK)
  {
    fprintf( stderr, ESC_FG_RED ESC_BOLD"Error: eb_socket_close returns %s\n"
             ESC_NORMAL, eb_status(status));
    return status;
  }
  return EB_OK;
}

/*! ---------------------------------------------------------------------------
 */
int fdcb(eb_user_data_t pUser, eb_descriptor_t des, uint8_t mode)
{
   printf( "Descriptor: %d, mode 0x%02x\n", des, mode );
   return 0;
}

/*! ---------------------------------------------------------------------------
 */
void cycleCb( eb_user_data_t user, eb_device_t dev, eb_operation_t op, eb_status_t status )
{
   ((EB_HANDLE_T*)user)->exit = true;
   if( status != EB_OK )
   {
      printf( ESC_FG_RED ESC_BOLD"Callback function called by status: %s\n"ESC_NORMAL,
              eb_status(status));
      return;
   }
   printf( "data = 0x%04x\n", eb_operation_data( op ) );
}


int main( int argc, char** ppArgv )
{
   g_pProgramName = ppArgv[0];
   printf( ESC_FG_CYAN"Program: %s, arg: %s\n"ESC_NORMAL, g_pProgramName, ppArgv[1] );
   printf( "Sizeof eb_data_t: %d\n", sizeof(eb_data_t) );

   EB_HANDLE_T ebh;
   if( ebOpen( &ebh, ppArgv[1] ) != EB_OK )
      return 1;
   eb_socket_descriptors( ebh.socket, &ebh, fdcb );

   eb_status_t status;
   if ((status = eb_cycle_open( ebh.device, &ebh, cycleCb, &ebh.cycle)) != EB_OK)
   {
     fprintf(stderr, "%s: failed to create cycle: %s\n",
             g_pProgramName, eb_status(status));
     return 1;
   }

   eb_cycle_read( ebh.cycle, GET_ADDR_OF_MEMBER( IO_T, c ), GET_SIZE_OF_MEMBER( IO_T, c ) | EB_BIG_ENDIAN, NULL );

   eb_cycle_close(ebh.cycle);

   while( !ebh.exit )
      eb_socket_run( ebh.socket, 10000 );

   ebClose( &ebh );
   printf( ESC_FG_CYAN"End...\n"ESC_NORMAL );
   return 0;
}

/*================================== EOF ====================================*/

