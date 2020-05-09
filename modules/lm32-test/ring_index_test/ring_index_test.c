/*!
 *  @file ring_index_test.c
 *  @brief Testprogram for testing the index-administration of a
 *         ring buffer.
 *
 *  @see scu_ddr3.h
 *  @see scu_ddr3.c
 *  @date 11.02..2019
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

#define OFFSET 10
#define SIZE 10

RAM_RING_INDEXES_T g_FifoAdmin =
{
   .offset = 0,
   .capacity = 10,
   .start    = 0,
   .end   = 0
};

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

typedef struct
{
   RAM_RING_INDEXES_T index;
   int                buffer[8];
} TEST_BUFFER_T;


void initTestBuffer( TEST_BUFFER_T* pThis )
{
   pThis->index.offset = 0;
   pThis->index.capacity = ARRAY_SIZE( pThis->buffer );
   ramRingReset( &pThis->index );
   memset( pThis->buffer, 0, sizeof( pThis->buffer ) );
}

void pushTestBuffer( TEST_BUFFER_T* pThis, int value )
{
   RAM_RING_INDEX_T i = ramRingGetWriteIndex(&pThis->index);
   ramRingAddToWriteIndex( &pThis->index, 1 );
   pThis->buffer[i] = value;
}

int popTestBuffer( TEST_BUFFER_T* pThis )
{
   int ret = pThis->buffer[ ramRingGeReadIndex( &pThis->index ) ];
   ramRingAddToReadIndex( &pThis->index, 1 );
   return ret;
}

void printTestBuffer( TEST_BUFFER_T* pThis )
{
   while( ramRingGetSize(&pThis->index) > 0 )
   {
      mprintf( "Index:  %d, content: %d\n", ramRingGeReadIndex( &pThis->index ), popTestBuffer( pThis ) );
   }
}

void printRingIndexes( RAM_RING_INDEXES_T* pIndexes )
{
   mprintf( "capacity: %d\n"
            "size:     %d\n"
            "free:     %d\n",
            pIndexes->capacity, ramRingGetSize( pIndexes ), ramRingGetRemainingCapacity( pIndexes ) );
   mprintf( "full:     %s\n\n", (pIndexes->end==pIndexes->capacity)? "YES" : "NO " );
}




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
   mprintf( ESC_FG_MAGENTA"Ring-index test\n"ESC_NORMAL );

   if( ramInit( &oRam, &g_FifoAdmin ) < 0  )
   {
      mprintf( ESC_FG_RED"ERROR: Could not find DDR3 base address!\n"ESC_NORMAL );
      return;
   }

   printRingIndexes( &g_FifoAdmin );
   ramRingAddToWriteIndex( &g_FifoAdmin, 1 );
   printRingIndexes( &g_FifoAdmin );
   ramRingAddToReadIndex( &g_FifoAdmin, 1 );
   printRingIndexes( &g_FifoAdmin );
   ramRingAddToWriteIndex( &g_FifoAdmin, 4 );
   printRingIndexes( &g_FifoAdmin );
   ramRingAddToReadIndex( &g_FifoAdmin, 1 );
   ramRingAddToWriteIndex( &g_FifoAdmin, 7 );
   printRingIndexes( &g_FifoAdmin );
   ramRingAddToReadIndex( &g_FifoAdmin, 10 );
   printRingIndexes( &g_FifoAdmin );

   ramRingAddToWriteIndex( &g_FifoAdmin, 9 );
   printRingIndexes( &g_FifoAdmin );

   TEST_BUFFER_T tb;

   initTestBuffer( &tb );
   pushTestBuffer( &tb, 1 );
   pushTestBuffer( &tb, 2 );
   pushTestBuffer( &tb, 3 );
   printTestBuffer( &tb );
   pushTestBuffer( &tb, 4 );
   pushTestBuffer( &tb, 5 );
   pushTestBuffer( &tb, 6 );
   printTestBuffer( &tb );
   pushTestBuffer( &tb, 7 );
   pushTestBuffer( &tb, 8 );
   pushTestBuffer( &tb, 9 );
   printTestBuffer( &tb );
   pushTestBuffer( &tb, 10 );
   pushTestBuffer( &tb, 11 );
   pushTestBuffer( &tb, 12 );
   printTestBuffer( &tb );
   pushTestBuffer( &tb, 13 );
   pushTestBuffer( &tb, 14 );
   pushTestBuffer( &tb, 15 );
   printTestBuffer( &tb );
   pushTestBuffer( &tb, 16 );
   pushTestBuffer( &tb, 17 );
   pushTestBuffer( &tb, 18 );
   printTestBuffer( &tb );
   pushTestBuffer( &tb, 19 );
   pushTestBuffer( &tb, 20 );
   pushTestBuffer( &tb, 21 );
   printTestBuffer( &tb );
   pushTestBuffer( &tb, 22 );
   pushTestBuffer( &tb, 23 );
   pushTestBuffer( &tb, 24 );
   printTestBuffer( &tb );
   pushTestBuffer( &tb, 25 );
   pushTestBuffer( &tb, 26 );
   pushTestBuffer( &tb, 27 );
   printTestBuffer( &tb );

 }

/* ================================= EOF ====================================*/
