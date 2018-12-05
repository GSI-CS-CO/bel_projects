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
#include <string.h>
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
//#if 0
   if( daqBusFindAndInitializeAll( &g_allDaq, find_device_adr(GSI, SCU_BUS_MASTER) ) <= 0 )
   {
      mprintf( "No usable DAQ found!\n" );
      return;
   }
   mprintf( "%d DAQ found\n", daqBusGetFoundDevices( &g_allDaq ) );
#if 1
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

   mprintf( "Total number of all used channels: %d\n", daqBusGetUsedChannels( &g_allDaq ) );

   //DAQ_CANNEL_T* pChannel = daqDeviceGetChannelObject( daqBusGetDeviceObject( &g_allDaq, DEVICE ), CHANNEL );
   DAQ_CANNEL_T* pChannel = daqBusGetChannelObjectByAbsoluteNumber( &g_allDaq, 0 );
   //pChannel->properties.notUsed = true;

   mprintf( "Total number of all used channels: %d\n", daqBusGetUsedChannels( &g_allDaq ) );

   daqBusSetAllTimeStampCounters( &g_allDaq, 0L );
   daqBusSetAllTimeStampCounterTags( &g_allDaq, 0 );

   daqChannelSetTriggerConditionLW( pChannel, 0xA );
   daqChannelSetTriggerConditionHW( pChannel, 0xB );
   daqChannelSetTriggerDelay( pChannel, 0xC );

   daqChannelEnablePostMortem( pChannel );
   while( daqChannelGetPmFifoWords( pChannel ) < 1023 );
   daqChannelDisablePostMortem( pChannel );
   daqChannelPrintInfo( pChannel );

   uint16_t volatile* ptr = daqChannelGetPmDatPtr( pChannel );
   DAQ_DESCRIPTOR_T descriptor;
   memset( &descriptor, 0, sizeof( descriptor ) );
   int j = 0;
   volatile uint16_t remaining;
   i = 0;
   do
   {
      remaining = daqChannelGetPmFifoWords( pChannel );
      volatile uint16_t data = *ptr;
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
#endif
   mprintf( "\nEnd...\n" );
}

/*================================== EOF ====================================*/
