/*!
 *  @file scu3test.h
 *  @brief Testprogram for DDR3 within the SCU3.
 *
 *  @see scu_ddr3.h
 *  @see scu_ddr3.c
 *  @see
 *  <a href="https://www-acc.gsi.de/wiki/Hardware/Intern/MacroF%C3%BCr1GbitDDR3MT41J64M16LADesSCUCarrierboards">
 *     DDR3 VHDL Macro der SCU3 </a>
 *  @date 01.02.2019
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
#include <daq_descriptor.h>
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

void main( void )
{
   RAM_SCU_T oRam;

   DDR3_PAYLOAD_T toWrite;

   DDR3_PAYLOAD_T burstFifo[8];

   memset( burstFifo, 0xFF, sizeof( burstFifo ));

   discoverPeriphery();
   uart_init_hw();
   gotoxy( 0, 0 );
   clrscr();
   mprintf( ESC_FG_MAGNETA"DDR3 Test\n"ESC_NORMAL );

   if( ramInit( &oRam, &g_FifoAdmin ) < 0  )
   {
      mprintf( ESC_FG_RED"ERROR: Could not find DDR3 base address!\n"ESC_NORMAL );
      return;
   }

   toWrite.ad16[0] = 0x4711;
   toWrite.ad16[1] = 0x4712;
   toWrite.ad16[2] = 0x4713;
   toWrite.ad16[3] = 0x4714;

   ddr3write64( &oRam.ddr3, 0, &toWrite );

   toWrite.ad16[0] = 0xAAAA;
   toWrite.ad16[1] = 0xBBBB;
   toWrite.ad16[2] = 0xCCCC;
   toWrite.ad16[3] = 0xDDDD;

   ddr3write64( &oRam.ddr3, 1, &toWrite );

   toWrite.ad16[0] = 0x1111;
   toWrite.ad16[1] = 0x2222;
   toWrite.ad16[2] = 0x3333;
   toWrite.ad16[3] = 0x4444;

   ddr3write64( &oRam.ddr3, 2, &toWrite );

   ddrPrint16( &oRam.ddr3, 0 );
   ddrPrint16( &oRam.ddr3, 1 );
#ifndef CONFIG_DDR3_NO_BURST_FUNCTIONS
   mprintf( "Fifo-Status address: 0x%08x\n",    &oRam.ddr3.pBurstModeBase[DDR3_FIFO_STATUS_OFFSET_ADDR] );
   mprintf( "Fifo-low word address: 0x%08x\n",  &oRam.ddr3.pBurstModeBase[DDR3_FIFO_LOW_WORD_OFFSET_ADDR] );
   mprintf( "FiFo-high word address: 0x%08x\n", &oRam.ddr3.pBurstModeBase[DDR3_FIFO_HIGH_WORD_OFFSET_ADDR] );
   mprintf( "FiFo-Status: 0x%08x\n", ddr3GetFifoStatus( &oRam.ddr3 ) );
   ddr3FlushFiFo( &oRam.ddr3, 0, ARRAY_SIZE( burstFifo ), burstFifo, ddr3Poll );
#endif
   printPayloadArray( burstFifo, ARRAY_SIZE( burstFifo ));
}

/* ================================= EOF ====================================*/
