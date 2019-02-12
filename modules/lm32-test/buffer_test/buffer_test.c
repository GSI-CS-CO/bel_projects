/*!
 *  @file buffer_test.h
 *  @brief Testprogram for data-buffer within the SCU3 between LM32 and
 *         linux host.
 *
 *  @see scu_ddr3.h
 *  @see scu_ddr3.c
 *  @see
 *  <a href="https://www-acc.gsi.de/wiki/Hardware/Intern/MacroF%C3%BCr1GbitDDR3MT41J64M16LADesSCUCarrierboards">
 *     DDR3 VHDL Macro der SCU3 </a>
 *  @date 08.02..2019
 *  @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *
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

#include <mini_sdb.h>
#include <scu_ramBuffer.h>
#include <eb_console_helper.h>
#include <string.h>

#if 1

RAM_RING_INDEXES_T g_FifoAdmin = RAM_RING_INDEXES_INITIALIZER;

typedef struct PACKED_SIZE
{
   DAQ_DESCRIPTOR_T daq;
   uint16_t data[sizeof(DAQ_DESCRIPTOR_T)%sizeof(DDR3_PAYLOAD_T)/sizeof(uint16_t)];
} DDR3_DAQ_DESCRIPTOR_T;

STATIC_ASSERT( (sizeof(DDR3_DAQ_DESCRIPTOR_T) % sizeof(DDR3_PAYLOAD_T)) == 0 );
#endif

void printPayload16( DDR3_PAYLOAD_T* pPl )
{
   for( int i = 0; i < ARRAY_SIZE( pPl->ad16 ); i++ )
      mprintf( "   %d: 0x%04x\n", i, pPl->ad16[i] );
}

void printPayloadArray( DDR3_PAYLOAD_T* aPl, size_t size )
{
   for( size_t i = 0; i < size; i++ )
   {
      mprintf( "Index: %d\n", i );
      printPayload16( &aPl[i] );
   }
}

void ddrPrint16( DDR3_T* pthis, unsigned int index )
{
   DDR3_PAYLOAD_T toRead;

   ddr3read64( pthis, &toRead, index );
   mprintf( "DDR-Index: %d\n", index );
   printPayload16( &toRead );
}

#ifndef CONFIG_DDR3_NO_BURST_FUNCTIONS
int ddr3Poll(  const DDR3_T* pThis UNUSED, unsigned int count )
{
   if( count >= 10 )
   {
      mprintf( ESC_FG_RED"Poll-function called for 10 times!\n"ESC_NORMAL );
      return -1;
   }
   return 0;
}
#endif


void readFiFo( DAQ_CANNEL_T* pThis )
{
   int j = 0;
   DAQ_DESCRIPTOR_T descriptor;
   memset( &descriptor, 0, sizeof( descriptor ) );

   uint16_t remaining  = daqChannelGetDaqFifoWords( pThis ) + 1;

   int i = 0;
   do
   {
      remaining = daqChannelGetDaqFifoWords( pThis );
      volatile uint16_t data = daqChannelPopDaqFifo( pThis ); //!!*ptr;
      if( i >= 490 )
        mprintf( "data[%d] = %d\n", i, data );
      if( remaining < ARRAY_SIZE( descriptor.index ) )
      {
         SCU_ASSERT( j < ARRAY_SIZE( descriptor.index ) );
         descriptor.index[j++] = data;
      }
      i++;
   }
   while( remaining != 0 );
   mprintf( ESC_FG_WHITE"Received: %d 16-bit words\n"ESC_NORMAL, i );
   daqDescriptorPrintInfo( &descriptor );
   DAQ_DESCRIPTOR_VERIFY_MY( &descriptor, pThis );
}



void main( void )
{
   DAQ_CANNEL_T daqChannel;
   RAM_SCU_T oRam;

   DDR3_PAYLOAD_T toWrite;

   discoverPeriphery();
   uart_init_hw();
   gotoxy( 0, 0 );
   clrscr();
   mprintf( ESC_FG_MAGNETA"Ring-buffer test\n"ESC_NORMAL );

   if( ramInit( &oRam, &g_FifoAdmin ) < 0  )
   {
      mprintf( ESC_FG_RED"ERROR: Could not find DDR3 base address!\n"ESC_NORMAL );
      return;
   }

   daqChannelReset( &daqChannel );
   daqChannel.simulatedDescriptor.name.slotDiob.slot = 1;

   daqDescriptorPrintInfo( &daqChannel.simulatedDescriptor );

   readFiFo( &daqChannel );

   mprintf( "End...\n" );
}

/* ================================= EOF ====================================*/
