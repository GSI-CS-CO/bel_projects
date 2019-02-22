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
   EB_MEMBER_INFO_T info[6];

   EB_INIT_INFO_ITEM_STATIC( info, 0, pIo->a );
   EB_INIT_INFO_ITEM_STATIC( info, 1, pIo->b );
   EB_INIT_INFO_ITEM_STATIC( info, 2, pIo->c );
   EB_INIT_INFO_ITEM_STATIC( info, 3, pIo->bf );
   EB_INIT_INFO_ITEM_STATIC( info, 4, pIo->sb.a );
   EB_INIT_INFO_ITEM_STATIC( info, 5, pIo->sb.b );

   EB_MAKE_CB_OR_ARG( cArg, info );

   if( ebObjectReadCycleOpen( pThis, &cArg ) != EB_OK )
   {
      fprintf( stderr, "%s: failed to create cycle: %s\n",
             g_pProgramName, ebGetStatusString( pThis ));
      return pThis->status;
   }

   EB_OJECT_MEMBER_READ( pThis, IO_T, a );
   EB_OJECT_MEMBER_READ( pThis, IO_T, b );
   EB_OJECT_MEMBER_READ( pThis, IO_T, c );
   EB_OJECT_MEMBER_READ( pThis, IO_T, bf );
   EB_OJECT_MEMBER_READ( pThis, IO_T, sb.a );
   EB_OJECT_MEMBER_READ( pThis, IO_T, sb.b );

   ebCycleClose( pThis );

   while( !cArg.exit )
      ebSocketRun( pThis );

   pThis->status = cArg.status;
   return pThis->status;
}

eb_status_t ebWriteIoStruct( EB_HANDLE_T* pThis, IO_T* pIo )
{
   EB_MAKE_CB_OW_ARG( cArg );

   if( ebObjectWriteCycleOpen( pThis, &cArg ) != EB_OK )
   {
      fprintf( stderr, "%s: failed to create cycle: %s\n",
               g_pProgramName, ebGetStatusString( pThis ));
      return pThis->status;
   }

   EB_OJECT_MEMBER_WRITE( pThis, pIo, a );
   EB_OJECT_MEMBER_WRITE( pThis, pIo, b );
   EB_OJECT_MEMBER_WRITE( pThis, pIo, c );
   EB_OJECT_MEMBER_WRITE( pThis, pIo, bf );
   EB_OJECT_MEMBER_WRITE( pThis, pIo, sb.a );
   EB_OJECT_MEMBER_WRITE( pThis, pIo, sb.b );
   ebCycleClose( pThis );

   while( !cArg.exit )
      ebSocketRun( pThis );

   pThis->status = cArg.status;
   return pThis->status;
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

   io.a = 2;
   io.b++;
   io.bf.a = 4;
   io.bf.b = 5;
   io.bf.c = 6;
   ebWriteIoStruct( &ebh, &io );


   ebClose( &ebh );
   printf( ESC_FG_CYAN"End...\n"ESC_NORMAL );
   return 0;
}

/*================================== EOF ====================================*/

