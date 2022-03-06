/*!
 *  @file daq_ramBuffer.c
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
#include <dbg.h>
#include <eb_console_helper.h>
#include <daq_ramBuffer.h>

#if defined( CONFIG_MIL_IN_TIMER_INTERRUPT) || defined( CONFIG_USE_INTERRUPT_TIMESTAMP ) || defined( CONFIG_RTOS )
   #include <lm32Interrupts.h>
#endif
#ifdef _CONFIG_PATCH_DAQ_TIMESTAMP
 #ifdef CONFIG_USE_INTERRUPT_TIMESTAMP
  #include <lm32Interrupts.h>
 #else
  #include <scu_wr_time.h>
 #endif
#endif
//////////////////////////////////////////////////////////////////////////////

/*! ---------------------------------------------------------------------------
 * @see scu_ramBuffer.h
 */
#ifdef _CONFIG_WAS_READ_FOR_ADDAC_DAQ
int ramInit( register RAM_SCU_T* pThis, RAM_RING_SHARED_INDEXES_T* pSharedObj
           #ifdef __linux__
         //   , EB_HANDLE_T* pEbHandle
           #endif
           )
{
   pThis->pSharedObj = pSharedObj;
   ramRingSharedReset( pSharedObj );

#ifdef CONFIG_SCU_USE_DDR3
 #if defined( __linux__ ) && defined( CONFIG_NO_FE_ETHERBONE_CONNECTION)
   return ddr3init( &pThis->ram, pEbHandle );
 #else
   return ddr3init( &pThis->ram );
 #endif
#endif
}
#else
int ramInit( register RAM_SCU_T* pThis, RAM_RING_SHARED_OBJECT_T* pSharedObj
       #if defined( __linux__ ) && defined( CONFIG_NO_FE_ETHERBONE_CONNECTION)
           , EB_HANDLE_T* pEbHandle
       #endif
           )
{
   pThis->pSharedObj = pSharedObj;
   ramRingReset( &pSharedObj->ringIndexes );   

#ifdef CONFIG_SCU_USE_DDR3
 #if defined( __linux__ ) && defined( CONFIG_NO_FE_ETHERBONE_CONNECTION)
   return ddr3init( &pThis->ram, pEbHandle );
 #else
   return ddr3init( &pThis->ram );
 #endif
#endif
}
#endif // else ifdef _CONFIG_WAS_READ_FOR_ADDAC_DAQ

//#define CONFIG_MIL_IN_TIMER_INTERRUPT

#if defined(__lm32__) || defined(__DOXYGEN__)
/*! ---------------------------------------------------------------------------
 * @brief Generalized function to read a ADDAC-DAQ-item from the
 *        shared data buffer.
 */
STATIC inline
void ramRreadAddacDaqItem( RAM_SCU_T* pThis, const RAM_RING_INDEX_T index,
                           RAM_DAQ_PAYLOAD_T* pItem )
{
#ifdef CONFIG_SCU_USE_DDR3
 #if defined( CONFIG_MIL_IN_TIMER_INTERRUPT ) || defined( CONFIG_RTOS )
   criticalSectionEnter();
   ddr3read64( &pThis->ram, pItem, index );
   criticalSectionExit();
 #else
   ddr3read64( &pThis->ram, pItem, index );
 #endif
#else
   #error Nothing implemented in function ramRreadItem()!
#endif
}

/*! ---------------------------------------------------------------------------
 * @brief Generalized function to write a ADDAC-DAQ-item in the shared
 *        date buffer.
 */
STATIC inline ALWAYS_INLINE
void ramWriteAddacDaqItem( register RAM_SCU_T* pThis,
                           const RAM_RING_INDEX_T index,
                           RAM_DAQ_PAYLOAD_T* pItem )
{
#if defined( CONFIG_MIL_IN_TIMER_INTERRUPT ) || defined( CONFIG_RTOS )
   criticalSectionEnter();
   ramWriteItem( pThis, index, pItem );
   criticalSectionExit();
#else
   ramWriteItem( pThis, index, pItem );
#endif
}

/*! ---------------------------------------------------------------------------
 * @brief Macro ease the read access to the channel mode register in the
 *        device descriptor storing as data block in the DAQ-RAM.
 * @see _DAQ_CHANNEL_CONTROL
 * @see _DAQ_DISCRIPTOR_STRUCT_T
 * @see RAM_DAQ_PAYLOAD_T
 * @param item RAM payload item including the channel mode register of
 *             the device descriptor.
 */
#define RAM_ACCESS_CHANNEL_MODE( item )                                       \
(                                                                             \
   *((_DAQ_CHANNEL_CONTROL*)&item.ad16                                        \
   [                                                                          \
      (RAM_DAQ_INDEX_OFFSET_OF_CHANNEL_CONTROL %                              \
       offsetof(_DAQ_DISCRIPTOR_STRUCT_T, cControl )) /                       \
       sizeof(DAQ_DATA_T)                                                     \
   ])                                                                         \
)
/* Awful I know... */

/*! ---------------------------------------------------------------------------
 * @brief Definition of return values of function ramRingGetTypeOfOldestBlock
 */
typedef enum
{
   RAM_DAQ_EMPTY,     //!<@brief No block present
   RAM_DAQ_UNDEFINED, //!<@brief No block recognized
   RAM_DAQ_SHORT,     //!<@brief Short block
   RAM_DAQ_LONG       //!<@brief Long block
} RAM_DAQ_BLOCK_T;

/*! ---------------------------------------------------------------------------
 * @see RAM_DAQ_BLOCK_T
 */
STATIC inline
RAM_DAQ_BLOCK_T ramRingGetTypeOfOldestBlock( register RAM_SCU_T* pThis )
{
#ifdef _CONFIG_WAS_READ_FOR_ADDAC_DAQ
   unsigned int size = ramRingSharedGetSize( pThis->pSharedObj );
#else
   unsigned int size = ramRingGetSize( &pThis->pSharedObj->ringIndexes );
#endif
   if( size == 0 )
      return RAM_DAQ_EMPTY;

   if( (size % RAM_DAQ_SHORT_BLOCK_LEN) != 0 )
   {
      DBPRINT1( ESC_FG_RED ESC_BOLD
                "DBG: ERROR: RAM content not dividable by "
                "minimum block length!\n"ESC_NORMAL );
      return RAM_DAQ_UNDEFINED;
   }

   RAM_DAQ_PAYLOAD_T  item;
#ifdef _CONFIG_WAS_READ_FOR_ADDAC_DAQ
   RAM_RING_INDEXES_T indexes = pThis->pSharedObj->indexes;
#else   
   RAM_RING_INDEXES_T indexes = pThis->pSharedObj->ringIndexes;
#endif
   ramRingAddToReadIndex( &indexes, RAM_DAQ_INDEX_OFFSET_OF_CHANNEL_CONTROL );
   ramRreadAddacDaqItem( pThis, ramRingGetReadIndex( &indexes ), &item );

#if  (DEBUGLEVEL>1)
   for( unsigned int i = 0; i < ARRAY_SIZE(item.ad16); i++ )
      DBPRINT2( "DBG: item: %d 0x%04x %d\n", i, item.ad16[i], item.ad16[i] );

   DBPRINT2( "DBG: daq:   0x%x\n", RAM_ACCESS_CHANNEL_MODE( item ).daqMode );
   DBPRINT2( "DBG: pm:    0x%x\n", RAM_ACCESS_CHANNEL_MODE( item ).pmMode );
   DBPRINT2( "DBG: hiRes: 0x%x\n", RAM_ACCESS_CHANNEL_MODE( item ).hiResMode );
#endif

   /*
    * Rough check of the device descriptors integrity.
    */
   if( ((int)RAM_ACCESS_CHANNEL_MODE( item ).daqMode)   +
       ((int)RAM_ACCESS_CHANNEL_MODE( item ).hiResMode) +
       ((int)RAM_ACCESS_CHANNEL_MODE( item ).pmMode)    != 1 )
   {
       DBPRINT1( ESC_FG_RED ESC_BOLD
                "DBG: ERROR: RAM wrong modes!\n"
                 ESC_NORMAL);
       return RAM_DAQ_UNDEFINED;
   }

   if( RAM_ACCESS_CHANNEL_MODE( item ).daqMode )
      return RAM_DAQ_SHORT;

   return RAM_DAQ_LONG;
}

/*! ---------------------------------------------------------------------------
 * @brief Removes the oldest DAQ- block in the ring boffer
 */
STATIC inline
void ramRemoveOldestBlock( register RAM_SCU_T* pThis )
{
   switch( ramRingGetTypeOfOldestBlock( pThis ) )
   {
      case RAM_DAQ_UNDEFINED:
      {
       #ifdef _CONFIG_WAS_READ_FOR_ADDAC_DAQ
         ramRingSharedReset( pThis->pSharedObj );
       #else
         ramRingReset( &pThis->pSharedObj->ringIndexes );
       #endif
         break;
      }
      case RAM_DAQ_SHORT:
      {
      #ifdef _CONFIG_WAS_READ_FOR_ADDAC_DAQ
         ramRingSharedAddToReadIndex( pThis->pSharedObj,
                                      RAM_DAQ_SHORT_BLOCK_LEN );
      #else
         ramRingAddToReadIndex( &pThis->pSharedObj->ringIndexes,
                                RAM_DAQ_SHORT_BLOCK_LEN );
      #endif
         break;
      }
      case RAM_DAQ_LONG:
      {
      #ifdef _CONFIG_WAS_READ_FOR_ADDAC_DAQ
         ramRingSharedAddToReadIndex( pThis->pSharedObj,
                                      RAM_DAQ_LONG_BLOCK_LEN );
      #else
         ramRingAddToReadIndex( &pThis->pSharedObj->ringIndexes,
                                RAM_DAQ_LONG_BLOCK_LEN );
      #endif
         break;
      }
      default: break;
   }
}

/*! ---------------------------------------------------------------------------
 * @brief Checks whether a additional DAQ-block can stored in the ring buffer.
 */
STATIC inline
bool ramDoesBlockFit( register RAM_SCU_T* pThis, const bool isShort )
{
#ifdef _CONFIG_WAS_READ_FOR_ADDAC_DAQ
   return (ramRingSharedGetRemainingCapacity( pThis->pSharedObj ) >=
           (isShort? RAM_DAQ_SHORT_BLOCK_LEN : RAM_DAQ_LONG_BLOCK_LEN));

#else
   return (ramRingGetRemainingCapacity( &pThis->pSharedObj->ringIndexes ) >=
           (isShort? RAM_DAQ_SHORT_BLOCK_LEN : RAM_DAQ_LONG_BLOCK_LEN));
#endif
}

/*! ---------------------------------------------------------------------------
 * @brief Removes the oldest blocks in ring buffer until it is enough space
 *        for a new Block.
 * @note If the blocks are not correctly recognized so the entire ring buffer
 *       becomes deleted.
 */
STATIC inline
void ramMakeSpaceIfNecessary( register RAM_SCU_T* pThis, const bool isShort )
{
   while( !ramDoesBlockFit( pThis, isShort ) )
   {
      DBPRINT1( "DBG: "ESC_FG_YELLOW"Removing block!\n"ESC_NORMAL );
      ramRemoveOldestBlock( pThis );
   }
}

/*! ---------------------------------------------------------------------------
 * @brief Helper function for ramWriteDaqData
 */
STATIC inline ALWAYS_INLINE
void ramFillItem( RAM_DAQ_PAYLOAD_T* pItem, const unsigned int i,
                  const DAQ_DATA_T data )
{
#if defined( CONFIG_SCU_USE_DDR3 ) || defined(__DOXYGEN__)
   RAM_ASSERT( i < ARRAY_SIZE( pItem->ad16 ) );
   ramSetPayload16( pItem, data, i );
#else
   #error Nothing implemented in function ramFillItem()!
#endif
}

#ifdef CONFIG_DEBUG_RAM_WRITE_DATA
  #define DBG_RAM_INFO DBPRINT1
#else
  #define DBG_RAM_INFO( ... )
#endif

/*! ---------------------------------------------------------------------------
 * @brief Publishing written data in shared memory.
 */
STATIC inline
void publishWrittenData( register RAM_SCU_T* pThis,
                         RAM_RING_INDEXES_T* poIndexes )
{
#ifdef _CONFIG_WAS_READ_FOR_ADDAC_DAQ
   pThis->pSharedObj->indexes.end = poIndexes->end;
   //pThis->pSharedObj->serverHasWritten = 1;
   DBG_RAM_INFO( "DBG: RAM-items: %d\n",
                 ramRingSharedGetSize( &pThis->pSharedObj->ringAdmin ) );
#else
   pThis->pSharedObj->ringIndexes.end = poIndexes->end;
   pThis->pSharedObj->serverHasWritten = 1;
   DBG_RAM_INFO( "DBG: RAM-items: %d\n",
                 ramRingGetSize( &pThis->pSharedObj->ringIndexes ) );
#endif
}

#ifndef _CONFIG_WAS_READ_FOR_ADDAC_DAQ
/*! ---------------------------------------------------------------------------
 */
STATIC inline void ramPollAccessLock( RAM_SCU_T* pThis )
{
#ifdef CONFIG_DEBUG_RAM_WRITE_DATA
   if( pThis->pSharedObj->ramAccessLock )
   {
      unsigned int pollCount = 0;
      DBG_RAM_INFO( ESC_FG_MAGENTA ESC_BOLD
                    "DBG: Enter RAM-access polling\n"
                    ESC_NORMAL );
      while( pThis->pSharedObj->ramAccessLock )
      {
         pollCount++;
      }
      DBG_RAM_INFO( ESC_FG_MAGENTA ESC_BOLD
                    "DBG: Leaving RAM-access polling. %d loops\n"
                    ESC_NORMAL, pollCount );
   }
#else
   while( pThis->pSharedObj->ramAccessLock ) {}
#endif
}
#endif
//#define CONFIG_DAQ_DECREMENT

/*! ---------------------------------------------------------------------------
 * @brief Copies the data of the given DAQ-channel in to the RAM and
 *        exchanges the order of DAQ data with device descriptor.
 *
 * That means, the device descriptor, which will received at last,
 * becomes copied in the ring buffer at the begin of the data block, following
 * by the payload data.
 */
STATIC inline
void ramWriteDaqData( register RAM_SCU_T* pThis, DAQ_CANNEL_T* pDaqChannel,
                      const bool isShort )
{
   DAQ_REGISTER_T (*getRemaining)( register DAQ_CANNEL_T* );
   volatile DAQ_DATA_T (*pop)( register DAQ_CANNEL_T* );
#ifdef CONFIG_DAQ_SW_SEQUENCE
   uint8_t*     pSequence;
#endif
   DAQ_REGISTER_T remainingDataWords;
   unsigned int dataWordCounter;
   unsigned int payloadIndex;
   DAQ_REGISTER_T expectedWords;

   /*
    * A local object of the current descriptor will use for verifying purposes
    * to check the block integrity.
    */
   DAQ_DESCRIPTOR_T    oDescriptor;
   unsigned int        descriptorIndex;
   RAM_RING_INDEXES_T  oDescriptorIndexes;

   RAM_RING_INDEXES_T  oDataIndexes;
   RAM_RING_INDEXES_T* poIndexes;

   RAM_DAQ_PAYLOAD_T ramItem;
   DAQ_DATA_T        firstData[RAM_DAQ_DESCRIPTOR_COMPLETION];
#ifdef _CONFIG_WAS_READ_FOR_ADDAC_DAQ
   oDescriptorIndexes = pThis->pSharedObj->indexes;
#else
   oDescriptorIndexes = pThis->pSharedObj->ringIndexes;
#endif
   oDataIndexes       = oDescriptorIndexes;
   poIndexes          = &oDataIndexes;

   /*
    * Skipping over the intended place of the device descriptor
    * which will received at last.
    */
   ramRingAddToWriteIndex( poIndexes, RAM_DAQ_DATA_START_OFFSET );

   ramRingDbgPrintIndexes( &pThis->pSharedObj->ringIndexes, "Origin indexes:");
   ramRingDbgPrintIndexes( poIndexes, "Data indexes:" );

   DBG_RAM_INFO( "DBG: %s(): "
                 ESC_BOLD " Slot: %d, Channel: %d\n" ESC_NORMAL,
                 __func__,
                 daqChannelGetSlot( pDaqChannel ),
                 daqChannelGetNumber( pDaqChannel ) + 1 );

   if( isShort )
   {
      getRemaining  = daqChannelGetDaqFifoWords;
      pop           = daqChannelPopDaqFifo;
      expectedWords = DAQ_FIFO_DAQ_WORD_SIZE_CRC;
#ifdef CONFIG_DAQ_SW_SEQUENCE
      pSequence     = &pDaqChannel->sequenceContinuous;
#endif
   }
   else
   {
      getRemaining  = daqChannelGetPmFifoWords;
      pop           = daqChannelPopPmFifo;
      expectedWords = DAQ_FIFO_PM_HIRES_WORD_SIZE_CRC;
#ifdef CONFIG_DAQ_SW_SEQUENCE
      pSequence     = &pDaqChannel->sequencePmHires;
#endif
   }

#ifndef CONFIG_DAQ_DECREMENT
   /*
    * The data wort which includes the CRC isn't a part of the fifo content,
    * therefore we have to add it here.
    */
   remainingDataWords = getRemaining( pDaqChannel ) + 1;
   if( remainingDataWords != expectedWords )
   {
      DBPRINT1( ESC_BOLD ESC_FG_RED
                "DBG ERROR: remainingDataWords != expectedWords\n"
                "           remainingDataWords: %d\n"
                "           expectedWords:      %d\n"
                ESC_NORMAL,
                remainingDataWords,
                expectedWords );
      daqChannelSetStatus( pDaqChannel, DAQ_RECEIVE_STATE_DATA_LOST );
      return;
   }
#endif

#ifdef _CONFIG_PATCH_DAQ_TIMESTAMP
   unsigned int wrIndex = (sizeof( _DAQ_WR_T ) / sizeof(DAQ_DATA_T));
   _DAQ_WR_T wrTime;
 #ifdef CONFIG_USE_INTERRUPT_TIMESTAMP
   wrTime.timeStamp = irqGetTimestamp();
 #else
   wrTime.timeStamp = getWrSysTimeSafe();
 #endif
#endif

   descriptorIndex = 0;
   dataWordCounter = 0;
   do
   {
   #ifdef CONFIG_DAQ_DECREMENT
      remainingDataWords = getRemaining( pDaqChannel );
   #else
      remainingDataWords--;
   #endif
      DAQ_DATA_T data = pop( pDaqChannel );

      if( dataWordCounter < ARRAY_SIZE( firstData ) )
      { /*
         * The first two received data words will stored in a temporary buffer.
         * They will copied in the place immediately after the device
         * descriptor. This manner making the intended RAM- place
         * of the device descriptor dividable by RAM_DAQ_PAYLOAD_T.
         */
         firstData[dataWordCounter] = data;
      #ifdef CONFIG_DAQ_DECREMENT
         DBG_RAM_INFO( "DBG: Words in Fifo: %d\n", remainingDataWords );
      #endif
      }
      else
      {
         if( dataWordCounter == ARRAY_SIZE( firstData ) )
         { /*
            * The first two data words making the device descriptor dividable
            * by RAM_DAQ_PAYLOAD_T has been received. The filling of the
            * ring buffer starts here.
            */
            payloadIndex = 0;
         }

         if( poIndexes == &oDescriptorIndexes )
         { /*
            * Descriptor becomes received.
            */
            RAM_ASSERT( descriptorIndex < ARRAY_SIZE(oDescriptor.index) );
         #ifdef _CONFIG_PATCH_DAQ_TIMESTAMP
           #warning "Patch of WR-timestamp in ADDAC-DAQ! Remove this asap! ASAP!!!"
            if( (descriptorIndex >= (offsetof(_DAQ_DISCRIPTOR_STRUCT_T, wr ) / sizeof(DAQ_DATA_T))) &&
                (wrIndex > 0)
              )
            {  /*
                * Overwriting the bullshit time-stamp! :-(
                */
               data = wrTime.wordIndex[--wrIndex];
            }
         #endif

         #ifdef CONFIG_DAQ_SW_SEQUENCE
            if( descriptorIndex == offsetof(_DAQ_DISCRIPTOR_STRUCT_T, crcReg ) /
                               sizeof(DAQ_DATA_T) )
            { /*
               * Setting of the sequence number in the device descriptor.
               * Sequence number has been already incremented before in
               * functions handleHiresMode(), handleContinuousMode(),
               * and handlePostMortemMode() in daq_main.c therefore the 1
               * must be deducted here again.
               */
               ((_DAQ_BF_CRC_REG*)&data)->sequence = *pSequence - 1;
            }
         #endif
           oDescriptor.index[descriptorIndex++] = data;
         }

         ramFillItem( &ramItem, payloadIndex, data );

         /*
          * Was the last data word of payload received?
          */
         if( remainingDataWords == DAQ_DESCRIPTOR_WORD_SIZE )
         { /*
            * Yes, possibly completion of the last RAM item if necessary.
            * This will be the case, by receiving a short block its
            * total length isn't dividable by RAM_DAQ_PAYLOAD_T.
            */
            while( payloadIndex < (RAM_DAQ_DATA_WORDS_PER_RAM_INDEX-1) )
            {
               payloadIndex++;
               DBG_RAM_INFO( "DBG: Complete with dummy data %d\n",
                             payloadIndex );
               ramFillItem( &ramItem, payloadIndex, 0xCAFE );
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
            {
               DBG_RAM_INFO( "DBG: Finalize with first data %d\n", i );
               payloadIndex++;
               RAM_ASSERT( payloadIndex < RAM_DAQ_DATA_WORDS_PER_RAM_INDEX );
               ramFillItem( &ramItem, payloadIndex, firstData[i] );
            }
         }

         /*
          * Next RAM item completed?
          */
         if( payloadIndex == (RAM_DAQ_DATA_WORDS_PER_RAM_INDEX-1) )
         {
            payloadIndex = 0;
            /*
             * Store item in RAM.
             */
            ramWriteAddacDaqItem( pThis, ramRingGetWriteIndex(poIndexes), &ramItem );
            ramRingIncWriteIndex( poIndexes );

            /*
             * Is the next data word the first word of the device descriptor?
             */
            if( remainingDataWords == DAQ_DESCRIPTOR_WORD_SIZE )
            { /*
               * Yes, skipping back at the begin.
               */
               poIndexes = &oDescriptorIndexes;
            }
         }
         else
            payloadIndex++;
      }

      dataWordCounter++;
      if( dataWordCounter > expectedWords )
         break;
   }
   while( remainingDataWords > 0 );

   /*
    * Is the block integrity given?
    */
   if( (dataWordCounter == expectedWords)
#ifndef CONFIG_DAQ_DECREMENT
       && (getRemaining( pDaqChannel ) == 0)
#endif
       && daqDescriptorVerifyMode( &oDescriptor )
       && (isShort == daqDescriptorIsShortBlock( &oDescriptor )) )
   { /*
      * Block integrity seems to be okay.
      * Making the new received data block in ring buffer valid.
      */
      publishWrittenData( pThis, &oDataIndexes );
   #ifdef CONFIG_DAQ_SW_SEQUENCE
      DBG_RAM_INFO( "DBG: Sequence: %d\n", *pSequence - 1 );
   #endif
      if( pDaqChannel->properties.restart )
      {
         if( daqDescriptorWasHiRes( &oDescriptor ) )
         {
            DBG_RAM_INFO( "DBG: Restarting High Resolution\n" );
            daqChannelEnableHighResolution( pDaqChannel );
         }
         else if( daqDescriptorWasPM( &oDescriptor ) )
         {
            DBG_RAM_INFO( "DBG: Restarting Post Mortem\n" );
            daqChannelEnablePostMortem( pDaqChannel );
         }
      }
   }
   else
   { /*
      * Block integrity is corrupt!
      */
      daqChannelSetStatus( pDaqChannel, DAQ_RECEIVE_STATE_CORRUPT_BLOCK );
#ifdef DEBUGLEVEL
   #ifndef CONFIG_DAQ_DECREMENT
      if( getRemaining( pDaqChannel ) != 0 )
      {
         DBPRINT1( ESC_BOLD ESC_FG_RED
                   "DBG ERROR: PmHires fifo: %d\n"
                   ESC_NORMAL, getRemaining( pDaqChannel ) );
      }
      else
   #endif
         DBPRINT1( ESC_BOLD ESC_FG_RED
                   "DBG ERROR: dataWordCounter > expectedWords\n"
                   "           dataWordCounter: %d\n"
                   "           expectedWords:   %d\n"
                   ESC_NORMAL,
                   dataWordCounter,
                   expectedWords );
#endif /* ifdef DEBUGLEVEL */
   }
   ramRingDbgPrintIndexes( &pThis->pSharedObj->ringIndexes,
                           ESC_FG_WHITE ESC_BOLD "Final indexes" ESC_NORMAL );
   daqDescriptorPrintInfo( &oDescriptor );
} /* ramWriteDaqData() */

/*! ---------------------------------------------------------------------------
 * @see scu_ramBuffer.h
 */
int ramPushDaqDataBlock( register RAM_SCU_T* pThis, DAQ_CANNEL_T* pDaqChannel,
                         const bool isShort )
{
   RAM_ASSERT( pThis != NULL );
   RAM_ASSERT( pDaqChannel != NULL );

   /*
    * Sequence number becomes incremented here in any cases,
    * so the possible lost of blocks can be detected by the Linux-host.
    *
    * Following debug error messages can be happen, when the external
    * interrupts follows to fast.
    */
   if( isShort )
   {
      if( daqChannelGetDaqFifoWords( pDaqChannel ) < DAQ_DESCRIPTOR_WORD_SIZE )
      {
         DBPRINT1( ESC_BOLD ESC_FG_RED
                   "DBG ERROR: Short block < descriptor size!\n"
                   ESC_NORMAL );
         daqChannelSetStatus( pDaqChannel, DAQ_RECEIVE_STATE_CORRUPT_BLOCK );
         return -1;
      }
   #ifdef CONFIG_DAQ_SIMULATE_CHANNEL
      daqDescriptorSetPM( &pDaqChannel->simulatedDescriptor, false );
      daqDescriptorSetHiRes( &pDaqChannel->simulatedDescriptor, true );
      daqDescriptorSetDaq( &pDaqChannel->simulatedDescriptor, false );
   #endif
   }
   else
   {
      if( daqChannelGetPmFifoWords( pDaqChannel ) < DAQ_DESCRIPTOR_WORD_SIZE )
      {
         DBPRINT1( ESC_BOLD ESC_FG_RED
                   "DBG ERROR: Long block < descriptor size!\n"
                   ESC_NORMAL );
         daqChannelSetStatus( pDaqChannel, DAQ_RECEIVE_STATE_CORRUPT_BLOCK );
         return -1;
      }
   #ifdef CONFIG_DAQ_SIMULATE_CHANNEL
      daqDescriptorSetPM( &pDaqChannel->simulatedDescriptor, false );
      daqDescriptorSetHiRes( &pDaqChannel->simulatedDescriptor, false );
      daqDescriptorSetDaq( &pDaqChannel->simulatedDescriptor, true );
   #endif
   }
#ifndef _CONFIG_WAS_READ_FOR_ADDAC_DAQ
   ramPollAccessLock( pThis );
#endif
   ramMakeSpaceIfNecessary( pThis, isShort );
   ramWriteDaqData( pThis, pDaqChannel, isShort );
   return 0;
}

#endif /* if defined(__lm32__) || defined(__DOXYGEN__) */
///////////////////////////////////////////////////////////////////////////////
#if (defined(__linux__) || defined(__DOXYGEN__))
/*! ---------------------------------------------------------------------------
 * @todo Make a cleaner separation of the software-layers.
 */
#if defined( CONFIG_DDR3_NO_BURST_FUNCTIONS )
int ramReadDaqDataBlock( register RAM_SCU_T* pThis, RAM_DAQ_PAYLOAD_T* pData,
                         unsigned int len )
{
#ifdef CONFIG_SCU_USE_DDR3
   RAM_RING_INDEXES_T indexes = pThis->pSharedObj->ringIndexes;
   EB_HANDLE_T*       pEb     = pThis->ram.pEbHandle;

   EB_MEMBER_INFO_T info[len * ARRAY_SIZE( pData->ad32 )];
   for( size_t i = 0; i < ARRAY_SIZE( info ); i++ )
   {
      info[i].pData = (uint8_t*)&pData[i / ARRAY_SIZE( pData->ad32 )].ad32
                                               [i % ARRAY_SIZE( pData->ad32 )];
      info[i].size = sizeof( pData->ad32[0] );
   }

   EB_CYCLE_OR_CB_ARG_T arg;
   arg.aInfo   = info;
   arg.infoLen = ARRAY_SIZE( info );
   arg.exit    = false;

   if( ebObjectReadCycleOpen(  pEb, &arg ) != EB_OK )
   {
      fprintf( stderr, ESC_FG_RED ESC_BOLD
                       "Error: failed to create cycle for read: %s\n"
                       ESC_NORMAL,
                       ebGetStatusString( pEb ));
      return ebGetStatus( pEb );
   }

   for( size_t i = 0; i < len; i++ )
   { /*
      * The read-index of the ring buffer doesn't becomes incremented
      * continuously, therefore it's necessary to reinitialize
      * the variable readIndex for each new iteration step.
      */
      size_t readIndex = pThis->ram.pTrModeBase +
                       ramRingGetReadIndex( &indexes ) * sizeof(DDR3_PAYLOAD_T);
      ramRingAddToReadIndex( &indexes, 1 );
      for( size_t j = 0; j < ARRAY_SIZE( pData->ad32 ); j++  )
      {
         ebCycleRead( pEb, readIndex, EB_DATA32 | EB_LITTLE_ENDIAN, NULL );
         readIndex += sizeof( pData->ad32[0] );
      }
   }

   ebCycleClose( pEb );

   while( !arg.exit )
      ebSocketRun( pEb );

   _ebSetOrStatus( pEb, &arg );

   if( ebGetStatus( pEb ) == EB_OK )
      pThis->pSharedObj->ringIndexes = indexes;

   return ebGetStatus( pEb );
#else
   #error Unknown memory type for function: ramReadDaqDataBlock()
#endif
}
#else /* if defined( CONFIG_DDR3_NO_BURST_FUNCTIONS ) */
#warning At the moment the burst mode is slow!
/*
 * In the theory the burst mode should to be clearly faster than the
 * conventional 64 bit oriented mode like above.
 * But at the moment the libEtherbone.so seems doesn't allow a memory mapped IO
 * respectively the access to the same address within the same EB-cycle.
 */
int ramReadDaqDataBlock( register RAM_SCU_T* pThis, RAM_DAQ_PAYLOAD_T* pData,
                         unsigned int len, RAM_DAQ_POLL_FT poll )
{
#ifdef CONFIG_SCU_USE_DDR3
   int ret;
   RAM_RING_INDEXES_T indexes = pThis->pSharedObj->ringIndexes;
   unsigned int lenToEnd = indexes.capacity - indexes.start;

   if( lenToEnd < len )
   {
      ret = ddr3FlushFiFo( &pThis->ram, ramRingGeReadIndex( &indexes ),
                           lenToEnd, pData, poll );
      if( ret != EB_OK )
         return ret;
      ramRingAddToReadIndex( &indexes, lenToEnd );
      len   -= lenToEnd;
      pData += lenToEnd;
   }

   ret = ddr3FlushFiFo( &pThis->ram, ramRingGeReadIndex( &indexes ),
                        len, pData, poll );
   if( ret != EB_OK )
      return ret;
   ramRingAddToReadIndex( &indexes, len );
   pThis->pSharedObj->ringIndexes = indexes;

   return EB_OK;
#else
   #error Unknown memory type for function: ramReadDaqDataBlock()
#endif
}
#endif /* if defined( CONFIG_DDR3_NO_BURST_FUNCTIONS ) */
#endif /* defined(__linux__) || defined(__DOXYGEN__) */

/*================================== EOF ====================================*/
