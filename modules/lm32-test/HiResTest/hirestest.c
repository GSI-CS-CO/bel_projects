/*!
 *  @file hirestest.c
 *  @brief Testprogram for testing the DAQ high resolution mode.
 *  @date 05.12.2018
 *  @copyright (C) 2018 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *
 *
 *******************************************************************************
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT AqNY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************
 */
#include <string.h>
#include "mini_sdb.h"
#include "../../top/gsi_scu/daq.h"
#include "eb_console_helper.h"
#include "helper_macros.h"

DAQ_BUS_T g_allDaq;

#define CHANNEL 0
#define DEVICE  0

void main( void )
{
   discoverPeriphery();
   uart_init_hw();

   gotoxy( 0, 0 );
   clrscr();
   mprintf( "DAQ High Resolution test\n");
#if 1
   if( daqBusFindAndInitializeAll( &g_allDaq, find_device_adr(GSI, SCU_BUS_MASTER) ) <= 0 )
   {
      mprintf( "No usable DAQ found!\n" );
      return;
   }
   mprintf( "%d DAQ found %d channels\n",
            daqBusGetFoundDevices( &g_allDaq ),
            daqBusGetNumberOfAllFoundChannels( &g_allDaq ) );
   DAQ_CANNEL_T* pChannel = daqBusGetChannelObjectByAbsoluteNumber( &g_allDaq, CHANNEL );
   if( pChannel == NULL )
   {
      mprintf( ESC_FG_RED "ERROR: Channel " TO_STRING( CHANNEL ) " not present!\n" ESC_NORMAL );
      return;
   }
   mprintf( "Using channel: " TO_STRING( CHANNEL ) "\n" );

  // daqChannelEnableHighResolution( pChannel );
   daqChannelPrintInfo( pChannel );
   unsigned int i = 0;
   while( !daqChannelTestAndClearHiResIntPending( pChannel ) )
      i++;
   mprintf( "i = %d\n", i );



#endif
   mprintf( "End...\n" );
}

/*================================== EOF ====================================*/
