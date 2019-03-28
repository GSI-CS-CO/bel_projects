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
#include "daq.h"
#include "eb_console_helper.h"
#include "helper_macros.h"
#include "scu_assert.h"
#include "irq.h"

DAQ_BUS_T g_allDaq;

void _segfault(int sig)
{
   mprintf( ESC_FG_RED ESC_BOLD "Segmentation fault: %d\n" ESC_NORMAL, sig );
   while( 1 );
}

#ifdef CONFIG_STACK_PROTECTOR_CODE
void __stack_chk_fail( void )
{
   mprintf( ESC_FG_RED ESC_BOLD"PANIC: Stack overflow!!!\n"ESC_NORMAL );
}
#endif

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


void readFiFo( DAQ_CANNEL_T* pThis )
{
   int j = 0;
   DAQ_DESCRIPTOR_T descriptor;
   memset( &descriptor, 0, sizeof( descriptor ) );

#ifdef CONFIG_DAQ_SEPARAD_COUNTER
   uint16_t remaining  = daqChannelGetDaqFifoWords( pThis ) + 1;
#else
   volatile uint16_t remaining;
#endif
   int i = 0;
   do
   {
#ifdef CONFIG_DAQ_SEPARAD_COUNTER
      remaining--;
#else
      remaining = daqChannelGetDaqFifoWords( pThis );
#endif
      volatile uint16_t data = daqChannelPopDaqFifo( pThis ); //!!*ptr;
#if 1
      if( i < 4 )
       mprintf( ESC_FG_MAGNETA"Index: %d, Data: 0x%04x, Remaining: %d\n"ESC_NORMAL, i, data, remaining );
#endif
      if( remaining < ARRAY_SIZE( descriptor.index ) )
      {
         SCU_ASSERT( j < ARRAY_SIZE( descriptor.index ) );
         descriptor.index[j++] = data;
      }
      i++;
   }
   while( remaining != 0 );
   mprintf( ESC_FG_BLUE ESC_BOLD"Received: %d 16-bit words\n"ESC_NORMAL, i );
#if 0
   for( j = 0; j < ARRAY_SIZE( descriptor.index ); j++ )
      mprintf( "Descriptor %d: 0x%04x\n", j, descriptor.index[j] );
#endif
   daqDescriptorPrintInfo( &descriptor );
  // DAQ_DESCRIPTOR_VERIFY_MY( &descriptor, pThis );

}



void printScuBusSlaveInfo( DAQ_CANNEL_T* pThis )
{
   daqDevicePrintInterruptStatus( DAQ_CHANNEL_GET_PARENT_OF( pThis ));
}


volatile unsigned int g_daqIrqCount   = 0;
volatile unsigned int g_hiResIrqCount = 0;

void onIrqDaq( void )
{
   irq_disable();
   g_daqIrqCount++;
   irq_enable();
}

unsigned int getDaqIrqCount( void )
{
   volatile unsigned int ret;
   irq_disable();
   ret = g_daqIrqCount;
   irq_enable();
   return ret;
}

void onIrqHiRes( void )
{
   irq_disable();
   g_hiResIrqCount++;
   irq_enable();
}

unsigned int getHiResIrqCount( void )
{
   volatile unsigned int ret;
   irq_disable();
   ret = g_hiResIrqCount;
   irq_enable();
   return ret;
}

void initIrq( void )
{
   g_daqIrqCount = 0;
   g_hiResIrqCount = 0;
   isr_table_clr();
   //isr_ptr_table[DAQ_IRQ_DAQ_FIFO_FULL]  = onIrqDaq;
   for( int i = 0; i < ARRAY_SIZE( isr_ptr_table ); i++ )
      isr_ptr_table[i] = onIrqDaq;
   isr_ptr_table[DAQ_IRQ_HIRES_FINISHED] = onIrqHiRes;
  // irq_set_mask( (1 << DAQ_IRQ_DAQ_FIFO_FULL) | (1 << DAQ_IRQ_HIRES_FINISHED) );
   irq_set_mask( 0xFFFFFFFF );
   irq_enable();
}

void printLine( const char c )
{
   for( int i = 0; i < 80; i++ )
      mprintf( "%c", c );
   mprintf( "\n" );
}

//=============================================================================
void main( void )
{
   discoverPeriphery();
   uart_init_hw();

   gotoxy( 0, 0 );
   clrscr();
   mprintf( ESC_FG_MAGNETA ESC_BOLD "DAQ Fifo test, compiler: " COMPILER_VERSION_STRING ESC_NORMAL "\n");
#if 1
   if( daqBusFindAndInitializeAll( &g_allDaq, find_device_adr(GSI, SCU_BUS_MASTER) ) <= 0 )
   {
      mprintf( ESC_FG_RED "ERROR: No usable DAQ found!\n" ESC_NORMAL );
      return;
   }
   mprintf( "%d DAQ found\n", daqBusGetFoundDevices( &g_allDaq ) );

//#if 0

   int i, j;
   mprintf( "Total number of all used channels: %d\n", daqBusGetUsedChannels( &g_allDaq ) );

   DAQ_CANNEL_T* pChannel = daqBusGetChannelObjectByAbsoluteNumber( &g_allDaq, 0 );
   if( pChannel == NULL )
   {
      mprintf( ESC_FG_RED "ERROR: Channel number out of range!\n" ESC_NORMAL );
      return;
   }
   uint32_t* pBusSlave = daqDeviceGetScuBusSlaveBaseAddress( DAQ_CHANNEL_GET_PARENT_OF( pChannel ) );
   uint16_t flags = scuBusGetSlaveValue16( pBusSlave, Intr_Active );
   mprintf( "SCU-Bus IRQ-flags: 0x%04x, Address : 0x%08x\n", flags, &((uint16_t*)pBusSlave)[Intr_Active] );

//#if 1
   daqChannelPrintInfo( pChannel );
   printScuBusSlaveInfo( pChannel );
//#if 0
   daqDeviceEnableScuSlaveInterrupt( DAQ_CHANNEL_GET_PARENT_OF( pChannel ) );
   printIntRegs( DAQ_CHANNEL_GET_PARENT_OF( pChannel ) );
   initIrq();
   daqChannelTestAndClearHiResIntPending( pChannel );
  // daqChannelSample1msOn( pChannel );
  // daqChannelSample100usOn( pChannel );
  // daqChannelSample10usOn( pChannel );
   printLine( '-' );
   for( int k = 0; k < 3; k++ )
   {
      printLine( '+' );
      daqChannelSetTriggerConditionLW( pChannel, (k == 0)? 0x4710 : 0x4711 );
      //daqChannelSample10usOn( pChannel );
      daqChannelSample1msOn( pChannel );
      //daqChannelSample100usOn( pChannel );
      i = 0;
    //  while( daqChannelGetDaqFifoWords( pChannel ) < DAQ_FIFO_DAQ_WORD_SIZE )
    //  while( !daqDeviceTestAndClearDaqInt( DAQ_CHANNEL_GET_PARENT_OF( pChannel ) ) );
      while( !daqChannelTestAndClearDaqIntPending( pChannel ) )
    //    while( !daqChannelTestAndClearHiResIntPending( pChannel ) ) //!!!!!!!!
         i++;

      mprintf( "Polling loops: %d\n", i );
      printIntRegs( DAQ_CHANNEL_GET_PARENT_OF( pChannel ) );
//#define BUG_FIX
#ifdef BUG_FIX
      if( daqChannelIsSample10usActive( pChannel ) )
      {
         daqChannelSample10usOff( pChannel );
      }
      else if( daqChannelIsSample100usActive( pChannel ) )
      {
         daqChannelSample100usOff( pChannel );
      }
      else if( daqChannelIsSample1msActive( pChannel ) )
      {
         daqChannelSample1msOff( pChannel );
      }
#endif
   //   daqChannelPrintInfo( pChannel );
   //   printScuBusSlaveInfo( pChannel );
   //   mprintf( "Reading FoFo for the " ESC_FG_YELLOW ESC_BOLD"%dth"ESC_NORMAL" time\n", k+1 );
      readFiFo( pChannel );
   }
   printLine( '=' );
//   daqChannelSample1msOff( pChannel );
//   daqChannelSample100usOff( pChannel );
//   daqChannelSample10usOff( pChannel );


  // daqChannelTestAndClearHiResIntPending( pChannel );
  // daqChannelTestAndClearDaqIntPending( pChannel );
   daqChannelPrintInfo( pChannel );
   printScuBusSlaveInfo( pChannel );
   mprintf( "IRQ DAQ:   %d\n", getDaqIrqCount() );
   mprintf( "IRQ HIRES: %d\n", getHiResIrqCount() );
   mprintf( "DAQ devices: parent %d\n", daqBusGetFoundDevices( DAQ_CHANNEL_GET_GRANDPARENT_OF( pChannel )) );
#endif
   mprintf( ESC_FG_MAGNETA ESC_BOLD "\nEnd...\n"ESC_NORMAL );
}

/*================================== EOF ====================================*/
