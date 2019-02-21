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
#include <eb_object_transfer.h>
#include <shared.h>

char* g_pProgramName;

STATIC_ASSERT( sizeof(eb_data_t) == sizeof(uint64_t) );


eb_status_t ebReadIOstruct( EB_HANDLE_T* pThis, IO_T* pIo )
{
   EB_MEMBER_INFO_T info[4];
   EB_CYCLE_CB_ARG_T cArg;

   EB_INIT_INFO_ITEM( info, pIo->a, 0 );
   EB_INIT_INFO_ITEM( info, pIo->b, 1 );
   EB_INIT_INFO_ITEM( info, pIo->c, 2 );
   EB_INIT_INFO_ITEM( info, pIo->bf, 3 );

   EB_INIT_CB_ARG( cArg, info );

   eb_status_t status;

   if( (status = ebObjectCycleOpen( pThis, &cArg )) != EB_OK )
   {
      fprintf(stderr, "%s: failed to create cycle: %s\n",
             g_pProgramName, eb_status(status));
      return status;
   }
   EB_OJECT_MEMBER_READ( pThis, IO_T, a );
   EB_OJECT_MEMBER_READ( pThis, IO_T, b );
   EB_OJECT_MEMBER_READ( pThis, IO_T, c );
   EB_OJECT_MEMBER_READ( pThis, IO_T, bf );

   ebCycleClose( pThis );

   while( !cArg.exit )
      ebSocketRun( pThis );

   return cArg.status;
}


int main( int argc, char** ppArgv )
{
   g_pProgramName = ppArgv[0];
   printf( ESC_FG_CYAN"Program: %s, arg: %s\n"ESC_NORMAL, g_pProgramName, ppArgv[1] );

   EB_HANDLE_T ebh;
   if( ebOpen( &ebh, ppArgv[1] ) != EB_OK )
      return 1;

   IO_T io;
   if( ebReadIOstruct( &ebh, &io ) == EB_OK )
      printIO( &io );

   ebClose( &ebh );
   printf( ESC_FG_CYAN"End...\n"ESC_NORMAL );
   return 0;
}

/*================================== EOF ====================================*/

