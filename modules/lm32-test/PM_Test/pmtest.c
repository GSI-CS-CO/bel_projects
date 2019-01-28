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


void _segfault( int sig )
{
   mprintf( ESC_FG_RED ESC_BOLD "Segmentation fault: %d\n" ESC_NORMAL, sig );
   while( 1 );
}


void main( void )
{
   discoverPeriphery();
   uart_init_hw();

   gotoxy( 0, 0 );
   clrscr();
   mprintf( "Post Mortem Fifo test, compiler: " COMPILER_VERSION_STRING "\n");
#if 1
   if( daqBusFindAndInitializeAll( &g_allDaq, find_device_adr(GSI, SCU_BUS_MASTER) ) <= 0 )
   {
      mprintf( "No usable DAQ found!\n" );
      return;
   }
   mprintf( "%d DAQ found\n", daqBusGetFoundDevices( &g_allDaq ) );
//#if 0
   int i;

   mprintf( "Total number of all used channels: %d\n", daqBusGetUsedChannels( &g_allDaq ) );

   //DAQ_CANNEL_T* pChannel = daqDeviceGetChannelObject( daqBusGetDeviceObject( &g_allDaq, DEVICE ), CHANNEL );
   DAQ_CANNEL_T* pChannel = daqBusGetChannelObjectByAbsoluteNumber( &g_allDaq, 0 );
   if( pChannel == NULL )
   {
      mprintf( ESC_FG_RED "ERROR: Channel number out of range!\n" ESC_NORMAL );
      return;
   }

   daqBusSetAllTimeStampCounters( &g_allDaq, 0L );
   daqBusSetAllTimeStampCounterTags( &g_allDaq, 0 );

   daqChannelSetTriggerConditionLW( pChannel, 0xA );
   daqChannelSetTriggerConditionHW( pChannel, 0xB );
   daqChannelSetTriggerDelay( pChannel, 0xC );


   mprintf( "HiResPending: 0x%04x\n", *daqChannelGetHiResIntPendingPtr( pChannel ) );

   daqChannelPrintInfo( pChannel );

   daqChannelTestAndClearHiResIntPending( pChannel );

   daqChannelEnablePostMortem( pChannel );
   //daqChannelEnableExternTriggerHighRes( pChannel );
   //daqChannelEnableHighResolution( pChannel );

   mprintf( "FiFo: %d\n", daqChannelGetPmFifoWords( pChannel ) );
   mprintf( "FiFo: %d\n", daqChannelGetPmFifoWords( pChannel ) );
   mprintf( "FiFo: %d\n", daqChannelGetPmFifoWords( pChannel ) );
   
   i = 0;
   while( daqChannelGetPmFifoWords( pChannel ) < (DAQ_FIFO_PM_HIRES_WORD_SIZE-1) )
      i++;
   mprintf( "Polling loops: %d\n", i );
   //while( !daqChannelTestAndClearHiResIntPending( pChannel ) );
   daqChannelPrintInfo( pChannel );

   daqChannelDisablePostMortem( pChannel );
   daqChannelDisableHighResolution( pChannel );

//   daqChannelReset( pChannel );

   mprintf( "HiResPending: 0x%04x\n", *daqChannelGetHiResIntPendingPtr( pChannel ) );

   //!!uint16_t volatile* ptr = daqChannelGetPmDatPtr( pChannel );
   DAQ_DESCRIPTOR_T descriptor;
   memset( &descriptor, 0, sizeof( descriptor ) );

   mprintf( "Reading FoFo...\n" );
   int j = 0;
//#define CONFIG_DAQ_SEPARAD_COUNTER
#ifdef CONFIG_DAQ_SEPARAD_COUNTER
   uint16_t remaining  = daqChannelGetPmFifoWords( pChannel ) + 1;
#else
   volatile uint16_t remaining;
#endif
   i = 0;
   do
   {
#ifdef CONFIG_DAQ_SEPARAD_COUNTER
      remaining--;
#else
      remaining = daqChannelGetPmFifoWords( pChannel );
#endif
      volatile uint16_t data = daqChannelPopPmFifo( pChannel ); //!!*ptr;
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
   DAQ_DESCRIPTOR_VERIFY_MY( &descriptor, pChannel );
   daqChannelPrintInfo( pChannel );
#endif
   mprintf( "\nEnd...\n" );
}

/*================================== EOF ====================================*/
