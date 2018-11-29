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

#define CHANNEL 2
#define DEVICE  0

void main( void )
{
   discoverPeriphery();
   uart_init_hw();

   gotoxy( 0, 0 );
   clrscr();
   mprintf( "Post Mortem Fifo test\n");
#if 1
   if( daqBusFindAndInitializeAll( &g_allDaq, find_device_adr(GSI, SCU_BUS_MASTER) ) <= 0 )
   {
      mprintf( "No usable DAQ found!\n" );
      return;
   }
   mprintf( "%d DAQ found\n", daqBusGetFoundDevices( &g_allDaq ) );

   int i;
   for( i = 1; i <= MAX_SCU_SLAVES; i++ )
   {
      if( daqBusGetDeviceBySlotNumber( &g_allDaq, i ) != NULL )
         mprintf( "DAQ found in slot %d\n", i );
   }

   i = 0;
   while( true )
   {
      DAQ_CANNEL_T* pChannel = daqBusGetChannelObjectByAbsoluteNumber( &g_allDaq, i );
      if( pChannel == NULL )
         break;
      mprintf( "Channel %d, address 0x%08x, slot %d\n", i, pChannel, daqChannelGetSlot( pChannel ) );
      i++;
   }

   DAQ_CANNEL_T* pChannel = daqDeviceGetChannelObject( daqBusGetDeviceObject( &g_allDaq, DEVICE ), CHANNEL );

   daqChannelPrintInfo( pChannel );
   uint16_t volatile* ptr = daqChannelGetPmDatPtr( pChannel );
   for( int i = 0; i < 10; i++ )
   {
      mprintf( "%d: 0x%04x, %d\n", i,
                                  *ptr,
                                  //daqChannelPopPmFifo( pChannel ),
                                  daqChannelGetPmFifoWords( pChannel ) );
   }
#endif
}

/*================================== EOF ====================================*/
