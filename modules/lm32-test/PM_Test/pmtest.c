/*!
 *  @file pmtest.c
 *  @brief Testprogram reads the PostMortem Fifo of channel 1
 *         form the first found DAQ device on the SCU- bus
 *  @date 28.11.2018
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
#include "mini_sdb.h"
#include "../../top/gsi_scu/daq.h"
#include "eb_console_helper.h"
#include "helper_macros.h"

DAQ_BUS_T g_allDaq;

void getDaqChannelInfo( DAQ_CANNEL_T* pThis )
{
   mprintf( "Slot: %d, Channel %d, Address: 0x%08x\n",
            daqChannelGetSlot( pThis ),
            daqChannelGetNumber( pThis ),
            daqChannelGetRegPtr( pThis ) );
}

void main( void )
{
   discoverPeriphery();
   uart_init_hw();

   gotoxy( 0, 0 );
   clrscr();
   mprintf( "Post Mortem Fifo test\n");
   if( daqBusFindAndInitializeAll( &g_allDaq, find_device_adr(GSI, SCU_BUS_MASTER) ) <= 0 )
   {
      mprintf( "No usable DAQ found!\n" );
      return;
   }
   mprintf( "%d DAQ found\n", daqBusGetFoundDevices( &g_allDaq ) );

   DAQ_CANNEL_T* pChannel = daqDeviceGetChannelObject( daqBusGetDeviceObject( &g_allDaq, 0 ), 0 );

   getDaqChannelInfo( pChannel );
   uint16_t volatile* ptr = daqChannelGetPmDatPtr( pChannel );
   for( int i = 0; i < 10; i++ )
   {
      mprintf( "%d: 0x%04x, %d\n", i,
                                  *ptr,
                                  //daqChannelPopPmFifo( pChannel ),
                                  daqChannelGetPmFifoWords( pChannel ) );
   }

}

/*================================== EOF ====================================*/
