/*!
 *  @file continue_test.c
 *  @brief Testprogram reads the DAQ Fifo of channel 1
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
#include <string.h>
#include "mini_sdb.h"
#include "../../top/gsi_scu/daq.h"
#include "eb_console_helper.h"
#include "helper_macros.h"

DAQ_BUS_T g_allDaq;



void main( void )
{
   discoverPeriphery();
   uart_init_hw();

   gotoxy( 0, 0 );
   clrscr();
   mprintf( "DAQ Fifo test\n");
#if 1
   if( daqBusFindAndInitializeAll( &g_allDaq, find_device_adr(GSI, SCU_BUS_MASTER) ) <= 0 )
   {
      mprintf( ESC_FG_RED "ERROR: No usable DAQ found!\n" ESC_NORMAL );
      return;
   }
   mprintf( "%d DAQ found\n", daqBusGetFoundDevices( &g_allDaq ) );
//#if 0
   int i;

   mprintf( "Total number of all used channels: %d\n", daqBusGetUsedChannels( &g_allDaq ) );

   DAQ_CANNEL_T* pChannel = daqBusGetChannelObjectByAbsoluteNumber( &g_allDaq, 0 );
   if( pChannel == NULL )
   {
      mprintf( ESC_FG_RED "ERROR: Channel number out of range!\n" ESC_NORMAL );
      return;
   }
   daqChannelPrintInfo( pChannel );
   DAQ_DESCRIPTOR_T descriptor;
   memset( &descriptor, 0, sizeof( descriptor ) );

   daqChannelSample1msOn( pChannel );
   //daqChannelSample100usOn( pChannel );
   //daqChannelSample10usOn( pChannel );

   mprintf( "FiFo: %d\n", daqChannelGetPmFifoWords( pChannel ) );
   mprintf( "FiFo: %d\n", daqChannelGetPmFifoWords( pChannel ) );
   mprintf( "FiFo: %d\n", daqChannelGetPmFifoWords( pChannel ) );

   i = 0;
   while( daqChannelGetDaqFifoWords( pChannel ) < (DAQ_FIFO_DAQ_WORD_SIZE-1) )
      i++;
   mprintf( "Polling loops: %d\n", i );
   daqChannelPrintInfo( pChannel );

   daqChannelSample1msOff( pChannel );
   daqChannelSample100usOff( pChannel );
   daqChannelSample10usOff( pChannel );

   mprintf( "Reading FoFo...\n" );
   int j = 0;
#ifdef CONFIG_DAQ_SEPARAD_COUNTER
   uint16_t remaining  = daqChannelGetDaqFifoWords( pChannel ) + 1;
#else
   volatile uint16_t remaining;
#endif
   i = 0;
   do
   {
#ifdef CONFIG_DAQ_SEPARAD_COUNTER
      remaining--;
#else
      remaining = daqChannelGetDaqFifoWords( pChannel );
#endif
      volatile uint16_t data = daqChannelPopDaqFifo( pChannel ); //!!*ptr;
#if 0
      mprintf( "%d: 0x%04x, %d\n", i, data, remaining );
#endif
      if( remaining < ARRAY_SIZE( descriptor.index ) )
      {
         LM32_ASSERT( j < ARRAY_SIZE( descriptor.index ) );
         descriptor.index[j++] = data;
      }
      i++;
   }
   while( remaining != 0 );

   for( j = 0; j < ARRAY_SIZE( descriptor.index ); j++ )
      mprintf( "Descriptor %d: 0x%04x\n", j, descriptor.index[j] );

   daqDescriptorPrintInfo( &descriptor );

   LM32_ASSERT( daqDescriptorGetSlot( &descriptor ) == daqChannelGetSlot( pChannel ) );
   LM32_ASSERT( daqDescriptorGetChannel( &descriptor ) == daqChannelGetNumber( pChannel ) );
   LM32_ASSERT( daqDescriptorGetTriggerConditionLW( &descriptor ) == daqChannelGetTriggerConditionLW( pChannel ) );
   LM32_ASSERT( daqDescriptorGetTriggerConditionHW( &descriptor ) == daqChannelGetTriggerConditionHW( pChannel ) );
   LM32_ASSERT( daqDescriptorGetTriggerDelay( &descriptor ) == daqChannelGetTriggerDelay( pChannel ) );

  // daqChannelTestAndClearHiResIntPending( pChannel );
   daqChannelTestAndClearDaqIntPending( pChannel );
   daqChannelPrintInfo( pChannel );


#endif
   mprintf( "\nEnd...\n" );
}

/*================================== EOF ====================================*/
