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
#include "daq.h"
#include "eb_console_helper.h"
#include "helper_macros.h"

DAQ_BUS_T g_allDaq;

#define CHANNEL 0
#define DEVICE  0

void _segfault(int sig)
{
   mprintf( ESC_FG_RED ESC_BOLD "Segmentation fault: %d\n" ESC_NORMAL, sig );
   while( 1 );
}

void readFiFo( DAQ_CANNEL_T* pThis )
{
   int j = 0;
   DAQ_DESCRIPTOR_T descriptor;
   memset( &descriptor, 0, sizeof( descriptor ) );

   volatile unsigned int remaining;
   int i = 0;
   do
   {
      remaining = daqChannelGetPmFifoWords( pThis );
      volatile uint16_t data = daqChannelPopPmFifo( pThis ); //!!*ptr;
#if 0
      mprintf( "%d: 0x%04x, %d\n", i, data, remaining );
#endif
      if( remaining < ARRAY_SIZE( descriptor.index ) )
      {
         SCU_ASSERT( j < ARRAY_SIZE( descriptor.index ) );
         descriptor.index[j++] = data;
      }
      i++;
   }
   while( remaining != 0 );

#if 0
   for( j = 0; j < ARRAY_SIZE( descriptor.index ); j++ )
      mprintf( "Descriptor %d: 0x%04x\n", j, descriptor.index[j] );
#endif
   daqDescriptorPrintInfo( &descriptor );
   DAQ_DESCRIPTOR_VERIFY_MY( &descriptor, pThis );

}

void printBin( uint16_t a )
{
   for( uint16_t i = 1 << (BIT_SIZEOF(uint16_t) - 1); i != 0; i >>= 1 )
      mprintf( "%c", (a & i)? '1' : '0' );
}

void printIntRegs( DAQ_DEVICE_T* pDevice )
{
   volatile uint16_t* volatile pIntr_In      = &((uint16_t*)daqDeviceGetScuBusSlaveBaseAddress( pDevice ))[Intr_In];
   volatile uint16_t* volatile pIntr_Ena     = &((uint16_t*)daqDeviceGetScuBusSlaveBaseAddress( pDevice ))[Intr_Ena];
   volatile uint16_t* volatile pIntr_pending = &((uint16_t*)daqDeviceGetScuBusSlaveBaseAddress( pDevice ))[Intr_pending];
   volatile uint16_t* volatile pIntr_Active  = &((uint16_t*)daqDeviceGetScuBusSlaveBaseAddress( pDevice ))[Intr_Active];

   mprintf( "Intr_In:      0x%08x -> ", pIntr_In );
   printBin( *pIntr_In );

   mprintf( "\nIntr_Ena:     0x%08x -> ", pIntr_Ena );
   printBin( *pIntr_Ena );

   mprintf( "\nIntr_pending: 0x%08x -> ", pIntr_pending );
   printBin( *pIntr_pending );

   mprintf( "\nIntr_Active:  0x%08x -> ", pIntr_Active );
   printBin( *pIntr_Active );

 //  (*pIntr_Ena) |= ~0;
   if( (*pIntr_Active) & 0x01 )
      (*pIntr_Active) |= 0x01;
   mprintf( "\n" );
}


void main( void )
{
   discoverPeriphery();
   uart_init_hw();

   gotoxy( 0, 0 );
   clrscr();
   mprintf( ESC_FG_MAGENTA ESC_BOLD "DAQ High Resolution test, compiler: " COMPILER_VERSION_STRING ESC_NORMAL "\n");
#if 1
   if( daqBusFindAndInitializeAll( &g_allDaq, find_device_adr(GSI, SCU_BUS_MASTER) ) <= 0 )
   {
      mprintf( "No usable DAQ found!\n" );
      return;
   }
   mprintf( "%d DAQ found %d channels\n",
            daqBusGetFoundDevices( &g_allDaq ),
            daqBusGetNumberOfAllFoundChannels( &g_allDaq ) );

#define CHANNEL 4
   DAQ_CANNEL_T* pChannel = daqBusGetChannelObjectByAbsoluteNumber( &g_allDaq, CHANNEL );
   if( pChannel == NULL )
   {
      mprintf( ESC_FG_RED "ERROR: Channel " TO_STRING( CHANNEL ) " not present!\n" ESC_NORMAL );
      return;
   }
   mprintf( "Using channel: " TO_STRING( CHANNEL ) "\n" );

   printIntRegs( DAQ_CHANNEL_GET_PARENT_OF( pChannel ) );
 //  daqChannelEnableExtrenTrigger( pChannel );
  daqChannelEnableTriggerMode( pChannel );
   daqChannelEnableExternTriggerHighRes( pChannel );

 daqChannelEnableHighResolution( pChannel );
 //  daqChannelEnablePostMortem( pChannel );
   daqChannelPrintInfo( pChannel );

//daqDeviceDisableScuSlaveInterrupt( DAQ_CHANNEL_GET_PARENT_OF( pChannel ) );
   printIntRegs( DAQ_CHANNEL_GET_PARENT_OF( pChannel ) );
   while( true )
   {
      daqChannelEnableHighResolution( pChannel );
      unsigned int i = 0;
      while( !daqDeviceTestAndClearHiResInt( DAQ_CHANNEL_GET_PARENT_OF( pChannel ) ) )
      while( !daqChannelTestAndClearHiResIntPending( pChannel ) )
     // while( daqChannelGetPmFifoWords( pChannel ) < DAQ_FIFO_PM_HIRES_WORD_SIZE )
         i++;

      daqChannelDisableHighResolution( pChannel );
      daqChannelDisablePostMortem( pChannel );
      daqChannelPrintInfo( pChannel );
      mprintf( "Polling: %d\n", i );
      readFiFo( pChannel );
       printIntRegs( DAQ_CHANNEL_GET_PARENT_OF( pChannel ) );
   }
#endif
   mprintf( "End...\n" );
}

/*================================== EOF ====================================*/
