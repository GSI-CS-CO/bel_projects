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

#ifdef __lm32__
  #include <mini_sdb.h>
#endif

#include <scu_ramBuffer.h>
#include <eb_console_helper.h>
#include <daq_descriptor.h>
#include <string.h>

#if 1

RAM_RING_SHARED_OBJECT_T  g_FifoAdmin = RAM_RING_SHARED_OBJECT_INITIALIZER;

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
      mprintf( "   %d: 0x%04x\n", i, ddr3GetPayload16( pPl, i ) );
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

#ifdef __lm32__
  #define EB_HANDLE
#else
EB_HANDLE_T g_ebHandle;
#define EB_HANDLE ,&g_ebHandle
#endif

int main( int argc, char** ppArgv )
{
   RAM_SCU_T oRam;

   DDR3_PAYLOAD_T toWrite;

   DDR3_PAYLOAD_T burstFifo[8];

   memset( (void*)burstFifo, 0xFF, sizeof( burstFifo ));
#ifdef __lm32__
   discoverPeriphery();
   uart_init_hw();
#else
#endif

   gotoxy( 0, 0 );
   clrscr();
   mprintf( ESC_FG_MAGENTA"DDR3 Test\n"ESC_NORMAL );

#ifdef __linux__
   if( ebOpen( &g_ebHandle, ppArgv[1] ) != EB_OK )
      return 1;
#endif

   if( ramInit( &oRam, &g_FifoAdmin EB_HANDLE ) < 0  )
   {
      mprintf( ESC_FG_RED"ERROR: Could not find DDR3 base address!\n"ESC_NORMAL );
      return 1;
   }

   ddr3SetPayload16( &toWrite, 0x4711, 0 );
   ddr3SetPayload16( &toWrite, 0x4712, 1 );
   ddr3SetPayload16( &toWrite, 0x4713, 2 );
   ddr3SetPayload16( &toWrite, 0x4714, 3 );
   ddr3write64( &oRam.ram, 0, &toWrite );

   ddr3SetPayload16( &toWrite, 0xAAAA, 0 );
   ddr3SetPayload16( &toWrite, 0xBBBB, 1 );
   ddr3SetPayload16( &toWrite, 0xCCCC, 2 );
   ddr3SetPayload16( &toWrite, 0xDDDD, 3 );

   ddr3write64( &oRam.ram, 1, &toWrite );

   ddr3SetPayload16( &toWrite, 0x0123, 0 );
   ddr3SetPayload16( &toWrite, 0x4567, 1 );
   ddr3SetPayload16( &toWrite, 0x89AB, 2 );
   ddr3SetPayload16( &toWrite, 0xCDEF, 3 );

   ddr3write64( &oRam.ram, 2, &toWrite );

   ddrPrint16( &oRam.ram, 0 );
   ddrPrint16( &oRam.ram, 1 );
   ddrPrint16( &oRam.ram, 2 );

#ifndef CONFIG_DDR3_NO_BURST_FUNCTIONS
   mprintf( "Fifo-Status address:    0x%08x\n", oRam.ram.pBurstModeBase + DDR3_FIFO_STATUS_OFFSET_ADDR );
   mprintf( "Fifo-low word address:  0x%08x\n", oRam.ram.pBurstModeBase + DDR3_FIFO_LOW_WORD_OFFSET_ADDR );
   mprintf( "FiFo-high word address: 0x%08x\n", oRam.ram.pBurstModeBase + DDR3_FIFO_HIGH_WORD_OFFSET_ADDR );
   mprintf( "FiFo-Status: 0x%08x\n", ddr3GetFifoStatus( &oRam.ram ) );
   ddr3FlushFiFo( &oRam.ram, 0, ARRAY_SIZE( burstFifo ), burstFifo, ddr3Poll );

   printPayloadArray( burstFifo, ARRAY_SIZE( burstFifo ));
#endif
//#endif // __lm32__

#ifdef __linux__
   ebClose( &g_ebHandle );
#endif
   mprintf( ESC_FG_MAGENTA"End of DDR3 test\n"ESC_NORMAL );
   return 0;
}

/* ================================= EOF ====================================*/
