/*!
 *  @file scu_ramBuffer.c
 *  @brief Abstraction layer for handling RAM buffer for DAQ data blocks.
 *
 *  @see scu_ramBuffer.h
 *
 *  @see scu_ddr3.h
 *  @see scu_ddr3.c
 *  @date 07.02.2019
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
#include <scu_ramBuffer.h>

/*! ---------------------------------------------------------------------------
 * @see scu_ramBuffer.h
 */
RAM_RING_INDEX_T ramRingGetSize( RAM_RING_INDEXES_T* pThis )
{
   if( pThis->end == pThis->capacity) /* Is ring-buffer full? */
      return pThis->capacity;
   if( pThis->end >= pThis->start )
      return pThis->end - pThis->start;
//   mprintf( "***\n" );
   return (pThis->capacity - pThis->start) + pThis->end;
}

/*! ---------------------------------------------------------------------------
 * @see scu_ramBuffer.h
 */
void ramRingAddToWriteIndex( RAM_RING_INDEXES_T* pThis, RAM_RING_INDEX_T toAdd )
{
   RAM_ASSERT( ramRingGetRemainingCapacity( pThis ) >= toAdd );
   RAM_ASSERT( pThis->end < pThis->capacity );

   pThis->end = (pThis->end + toAdd) % pThis->capacity;

   if( pThis->end == pThis->start )
      pThis->end = pThis->capacity; /* Ring buffer is full. */
}

/*! ---------------------------------------------------------------------------
 * @see scu_ramBuffer.h
 */
void ramRingAddToReadIndex( RAM_RING_INDEXES_T* pThis, RAM_RING_INDEX_T toAdd )
{
   RAM_ASSERT( ramRingGetSize( pThis ) >= toAdd );

   if( (toAdd != 0) && (pThis->end == pThis->capacity) )  /* Is ring-buffer full? */
      pThis->end = pThis->start;

   pThis->start = (pThis->start + toAdd) % pThis->capacity;
}

/*! ---------------------------------------------------------------------------
 * @see scu_ramBuffer.h
 */
int ramInit( register RAM_SCU_T* pThis, RAM_RING_INDEXES_T* pRingIndexes )
{
   pThis->pRingIndexes = pRingIndexes;
   ramRingReset( pRingIndexes );
#ifdef CONFIG_SCU_USE_DDR3
   return ddr3init( &pThis->ram );
#endif
}

static inline
void ramRreadItem( register RAM_SCU_T* pThis, const RAM_RING_INDEX_T index,
                   RAM_DAQ_PAYLOAD_T* pItem )
{
#ifdef CONFIG_SCU_USE_DDR3
   ddr3read64( &pThis->ram, pItem, index );
#else
   #error Nothing implemented in function ramRreadItem()!
#endif
}

/*! ---------------------------------------------------------------------------
 * @see scu_ramBuffer.h
 */
RAM_DAQ_BLOCK_T ramRingGetTypeOfOldestBlock( register RAM_SCU_T* pThis )
{
#ifdef CONFIG_SCU_USE_DDR3
   //TODO
   return RAM_DAQ_UNDEFINED;
#else
 #error Nothing implemented in function ramRingGetTypeOfOldestBlock()!
#endif
}

#if defined(__lm32__) || defined(__DOXYGEN__)

/*! ---------------------------------------------------------------------------
 */
static int ramRemoveOldestBlock( register RAM_SCU_T* pThis )
{
   //TODO
   return 0;
}

/*! ---------------------------------------------------------------------------
 */
static bool ramDoesBlockFit( register RAM_SCU_T* pThis, bool isShort )
{
   //TODO
   return false;
}

/*! ---------------------------------------------------------------------------
 */
static inline
void ramWriteItem( register RAM_SCU_T* pThis, const RAM_RING_INDEX_T index,
                   RAM_DAQ_PAYLOAD_T* pItem )
{
#ifdef CONFIG_SCU_USE_DDR3
   ddr3write64( &pThis->ram, index, pItem );
#else
   #error Nothing implemented in function ramWriteItem()!
#endif
}


/*! ---------------------------------------------------------------------------
 * @brief Helper function for ramWriteDaqData
 */
static inline ALWAYS_INLINE
void ramFillItem( RAM_DAQ_PAYLOAD_T* pItem, const unsigned int i,
                  DAQ_DATA_T data )
{
#ifdef CONFIG_SCU_USE_DDR3
   RAM_ASSERT( i < ARRAY_SIZE( pItem->ad16 ) );
   pItem->ad16[i] = data;
#else
   #error Nothing implemented in function ramFillItem()!
#endif
}

/*! ---------------------------------------------------------------------------
 */
static inline
void ramWriteDaqData( register RAM_SCU_T* pThis, DAQ_CANNEL_T* pDaqChannel,
                      bool isShort )
{
   unsigned int (*getRemaining)( register DAQ_CANNEL_T* );
   volatile DAQ_DATA_T (*pop)( register DAQ_CANNEL_T* );
   unsigned int remainingDataWords;
   unsigned int dataWordCounter;
   unsigned int payloadIndex;
   RAM_RING_INDEX_T ramIndex;
   RAM_DAQ_PAYLOAD_T ramItem;
   DAQ_DATA_T firstData[RAM_DAQ_DESCRIPTOR_REST];

   if( isShort )
   {
      getRemaining = daqChannelGetDaqFifoWords;
      pop          = daqChannelPopDaqFifo;
   }
   else
   {
      getRemaining = daqChannelGetPmFifoWords;
      pop          = daqChannelPopPmFifo;
   }

   dataWordCounter = 0;
   do
   {
      remainingDataWords = getRemaining( pDaqChannel );
      if( dataWordCounter < ARRAY_SIZE( firstData ) )
      {
         firstData[dataWordCounter] = pop( pDaqChannel );
      }
      else
      {
         if( dataWordCounter == ARRAY_SIZE( firstData ) )
         {
            payloadIndex = 0;
            /*
             * Skipping over the intended place of the device descriptor.
             */
            ramIndex = ramRingGetWriteIndex( pThis->pRingIndexes ) +
                       RAM_DAQ_DATA_START_OFFSET;
         }

         ramFillItem( &ramItem, payloadIndex, pop( pDaqChannel ) );

         /*
          * Was the last data word of payload received?
          */
         if( remainingDataWords == DAQ_DISCRIPTOR_WORD_SIZE )
         { /*
            * Yes, possibly completion of the last RAM item if necessary.
            * This will be the case, by receiving a short block its
            * total length isn't dividable by RAM_DAQ_PAYLOAD_T.
            */
            while( payloadIndex < (RAM_DAQ_DATA_WORDS_PER_RAM_INDEX-1) )
            { mprintf( "Da!\n" );
               payloadIndex++;
               ramFillItem( &ramItem, payloadIndex, 0xDEAD );
            }
         }
         /*
          * Has the block been received completely?
          */
         else if( remainingDataWords == 0 )
         { /*
            * Yes, but because the length of the device descriptor isn't
            * dividable by RAM_DAQ_PAYLOAD_T so the rest becomes filled with
            * the first received data words.
            */
            for( unsigned int i = 0; i < ARRAY_SIZE( firstData ); i++ )
            { mprintf( "Hier!\n" );
               payloadIndex++;
               RAM_ASSERT( payloadIndex < RAM_DAQ_DATA_WORDS_PER_RAM_INDEX );
               ramFillItem( &ramItem, payloadIndex, firstData[i] );
            }
         }

         payloadIndex++;
         /*
          * Next RAM item completed?
          */
         if( payloadIndex == RAM_DAQ_DATA_WORDS_PER_RAM_INDEX )
         {
            payloadIndex = 0;
            /*
             * Store item in RAM.
             */
            ramWriteItem( pThis, ramIndex, &ramItem );
            /*
             * Is the next data word the first word of the device descriptor?
             */
            if( remainingDataWords == DAQ_DISCRIPTOR_WORD_SIZE )
            { /*
               * Yes, skipping back at the begin.
               */
               mprintf( "Da da! %d\n", ramIndex );
               ramIndex = ramRingGetWriteIndex( pThis->pRingIndexes );
               mprintf( "Da da! %d\n", ramIndex );
            }
            else
            { /*
               * Next RAM- item
               */
               ramIndex++;
            }
         } /* if( payloadIndex == RAM_DAQ_DATA_WORDS_PER_RAM_INDEX ) */
      }
      dataWordCounter++;
   }
   while( remainingDataWords > 0 );

   mprintf( "HuHuuuu %d\n", dataWordCounter );
   /*
    * Making the new received data block in ring buffer valid.
    */
   ramRingAddToWriteIndex( pThis->pRingIndexes, isShort?
                                                RAM_DAQ_SHORT_BLOCK_LEN :
                                                RAM_DAQ_LONG_BLOCK_LEN );
}


/*! ---------------------------------------------------------------------------
 * @see scu_ramBuffer.h
 */
int ramPushDaqDataBlock( register RAM_SCU_T* pThis, DAQ_CANNEL_T* pDaqChannel,
                         bool isShort )
{
   RAM_ASSERT( pThis != NULL );
   RAM_ASSERT( pDaqChannel != NULL );
   RAM_ASSERT( daqChannelGetDaqFifoWords( pDaqChannel ) > DAQ_DISCRIPTOR_WORD_SIZE );
   //TODO
   ramWriteDaqData( pThis, pDaqChannel, isShort );
   return 0;
}

#endif /* if defined(__lm32__) || defined(__DOXYGEN__) */

/*================================== EOF ====================================*/
