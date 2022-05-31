/*!
 *  @file daq.c
 *  @brief Control module for ADDAC Data Acquisition Unit (DAQ)
 *  @see
 *  <a href="https://www-acc.gsi.de/wiki/Hardware/Intern/DataAquisitionMacrof%C3%BCrSCUSlaveBaugruppen">
 *     Data Aquisition Macro fuer SCU Slave Baugruppen</a>
 *  @date 13.11.2018
 *  @copyright (C) 2018 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *
 *  @todo Synchronization with SCU-Bus. It could be there that further devices
 *        which have traffic via this SCU-Bus!
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
#ifndef __DOCFSM__
#include <string.h>   // necessary for memset()
#include <mini_sdb.h> // necessary for ERROR_NOT_FOUND
#include <dbg.h>
#include <scu_wr_time.h>
#ifndef CONFIG_DAQ_SINGLE_APP
 #include <lm32Interrupts.h>
 #include <daq_command_interface.h>
 #include <scu_fg_list.h>
#endif
#if defined( CONFIG_DAQ_DEBUG ) || !defined( CONFIG_NO_DAQ_INFO_PRINT ) || !defined( CONFIG_DAQ_SINGLE_APP )
 #include <eb_console_helper.h>
#endif
#endif
#include "daq.h"
#if defined( CONFIG_DAQ_DEBUG ) || defined(__DOXYGEN__)
//! @brief For debug purposes only
const char* g_pYes = ESC_FG_WHITE ESC_BOLD"yes"ESC_NORMAL;
//! @brief For debug purposes only
const char* g_pNo  = "no";
#endif


/*======================== DAQ channel functions ============================*/
/*! ---------------------------------------------------------------------------
 * @brief Writes the given value in addressed register
 * @param pReg Start address of DAQ-macro.
 * @param index Offset address of register @see
 * @param channel Channel number.
 * @param value Value for writing into register.
 */
STATIC inline void daqChannelSetReg( DAQ_REGISTER_ACCESS_T* volatile pReg,
                                     const DAQ_REGISTER_INDEX_T index,
                                     const unsigned int channel,
                                     const DAQ_REGISTER_T value )
{
   DAQ_ASSERT( channel < DAQ_MAX_CHANNELS );
   DAQ_ASSERT( (index & 0x0F) == 0x00 );
   pReg->i[index | channel] = value;
}

/*! ---------------------------------------------------------------------------
 * @brief Reads a value from a addressed register
 * @param pReg Start address of DAQ-macro.
 * @param index Offset address of register @see
 * @param channel Channel number.
 * @return Register value.
 */
STATIC inline
DAQ_REGISTER_T daqChannelGetReg( DAQ_REGISTER_ACCESS_T* volatile pReg,
                                 const DAQ_REGISTER_INDEX_T index,
                                 const unsigned int channel )
{
   DAQ_ASSERT( channel < DAQ_MAX_CHANNELS );
   DAQ_ASSERT( (index & 0x0F) == 0x00 );
   return pReg->i[index | channel];
}

/*! ---------------------------------------------------------------------------
 */
void daqChannelSetStatus( register DAQ_CANNEL_T* pThis, DAQ_REC_STAT_T state )
{
   DAQ_LAST_STATUS_T* pLastState = &DAQ_CHANNEL_GET_GRANDPARENT_OF( pThis )->
                                                                lastErrorState;
   pLastState->status  = state;
   pLastState->slot    = daqChannelGetSlot( pThis );
   pLastState->channel = pThis->n + 1;
}

/*! ---------------------------------------------------------------------------
 */
bool daqChannelIsPmHiResFiFoFull( register DAQ_CANNEL_T* pThis )
{ /*
   * Because of possible transitions in the FoFo-level register during
   * the post-mortem/high-resolution mode is sill active,
   * it becomes necessary to ask this register more then one time to
   * ensure that the FiFo is really full.
   */
   for( unsigned int i = 0; i < 2; i++ )
   {
      if( daqChannelGetPmFifoWords( pThis ) != DAQ_FIFO_PM_HIRES_WORD_SIZE )
         return false;
   }
   return true;
}

#ifndef CONFIG_DAQ_SIMULATE_CHANNEL
/*! ---------------------------------------------------------------------------
 * @see daq.h
 */
void daqChannelReset( register DAQ_CANNEL_T* pThis )
{
   daqChannelSample10usOff( pThis );
   daqChannelSample100usOff( pThis );
   daqChannelSample1msOff( pThis );
   daqChannelDisableTriggerMode( pThis );
   daqChannelEnableEventTrigger( pThis );
   daqChannelDisablePostMortem( pThis );
   daqChannelDisableHighResolution( pThis );
   daqChannelEnableEventTriggerHighRes( pThis );
   daqChannelSetTriggerConditionLW( pThis, 0 );
   daqChannelSetTriggerConditionHW( pThis, 0 );
   daqChannelSetTriggerDelay( pThis, 0 );

   unsigned int i;
   volatile DAQ_DATA_T dummy UNUSED;
   /*
    * Making the PM_HiRes Fifo empty and discard the content
    */
   for( i = 0; i < DAQ_FIFO_PM_HIRES_WORD_SIZE; i++ )
      dummy = daqChannelPopPmFifo( pThis );

   /*
    * Making the DAQ Fifo empty and discard the content
    */
   for( i = 0; i < DAQ_FIFO_DAQ_WORD_SIZE; i++ )
      dummy = daqChannelPopDaqFifo( pThis );

   daqChannelTestAndClearDaqIntPending( pThis );
   daqChannelTestAndClearHiResIntPending( pThis );
}
#else /* ifndef CONFIG_DAQ_SIMULATE_CHANNEL */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * CAUTION: Following functions are for simulation purposes only!
 */
/*! ---------------------------------------------------------------------------
 * @see daq.h
 */
void daqChannelReset( register DAQ_CANNEL_T* pThis )
{
   memset( pThis, 0x00, sizeof( DAQ_CANNEL_T ) );
}

/*! ---------------------------------------------------------------------------
 * @see daq.h
 */
unsigned int daqChannelGetPmFifoWordsSimulate( register DAQ_CANNEL_T* pThis )
{
   return DAQ_FIFO_PM_HIRES_WORD_SIZE - pThis->callCount;
}

/*! ---------------------------------------------------------------------------
 * @see daq.h
 */
unsigned int daqChannelGetDaqFifoWordsSimulate( register DAQ_CANNEL_T* pThis )
{
   return DAQ_FIFO_DAQ_WORD_SIZE  - pThis->callCount;
}

/*! ---------------------------------------------------------------------------
 * @brief Simulates a DAQ single shot stream finalized by a fake
 *        DAQ-descriptor. The payload becomes simulated by increasing numbers.
 *
 * The time stamp counter will misused as periodic counter.
 * @note CAUTION: This function is for developing and testing purposes only and
 *                becomes compiled if the compiler-switch
 *                CONFIG_DAQ_SIMULATE_CHANNEL defined!
 */
STATIC
DAQ_DATA_T daqChannelPopFifoSimulate( register DAQ_CANNEL_T* pThis,
                                      unsigned int remaining,
                                      const unsigned int limit )
{
   DAQ_DATA_T ret;

   if( pThis->callCount > limit )
      return 0;

   if( remaining >= ARRAY_SIZE( pThis->simulatedDescriptor.index ) )
      ret = pThis->callCount + 1;
   else
   {
      unsigned int i = ARRAY_SIZE( pThis->simulatedDescriptor.index ) - 1
                       - remaining;
      ret = pThis->simulatedDescriptor.index[i];
      DBPRINT2( "DBG: i: %d ret: 0x%04x\n", i, ret );
   }

   if( pThis->callCount < limit )
      pThis->callCount++;
   else
   {
      pThis->callCount = 0;
      pThis->simulatedDescriptor.name.wr.name.utSec++;
   }

   return ret;
}

/*! ---------------------------------------------------------------------------
 * @brief Simulates a DAQ single shot stream finalized by a fake
 *        DAQ-descriptor. The payload becomes simulated by increasing numbers.
 *
 * The time stamp counter will misused as periodic counter.
 * @note CAUTION: This function is for developing and testing purposes only and
 *                becomes compiled if the compiler-switch
 *                CONFIG_DAQ_SIMULATE_CHANNEL defined!
 * @param pThis Pointer to the channel object
 * @return Simulated fake data.
 */
DAQ_DATA_T daqChannelPopPmFifoSimulate( register DAQ_CANNEL_T* pThis )
{
   return daqChannelPopFifoSimulate( pThis,
                                     daqChannelGetPmFifoWordsSimulate( pThis ),
                                     DAQ_FIFO_PM_HIRES_WORD_SIZE );
}

/*! ---------------------------------------------------------------------------
 * @brief Simulates a DAQ continuous stream finalized by a fake
 *        DAQ-descriptor. The payload becomes simulated by increasing numbers.
 *
 * The time stamp counter will misused as periodic counter.
 * @note CAUTION: This function is for developing and testing purposes only and
 *                becomes compiled if the compiler-switch
 *                CONFIG_DAQ_SIMULATE_CHANNEL defined!
 * @param pThis Pointer to the channel object
 * @return Simulated fake data.
 */
DAQ_DATA_T daqChannelPopDaqFifoSimulate( register DAQ_CANNEL_T* pThis )
{
   return daqChannelPopFifoSimulate( pThis,
                                     daqChannelGetDaqFifoWordsSimulate( pThis ),
                                     DAQ_FIFO_DAQ_WORD_SIZE );
}

/*
 * End of prototypes for simulation!
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */
#endif /* ifdef CONFIG_DAQ_SIMULATE_CHANNEL */

#if defined( CONFIG_DAQ_DEBUG ) || defined(__DOXYGEN__)
/*! ---------------------------------------------------------------------------
 * @see daq.h
 */
void daqChannelPrintInfo( register DAQ_CANNEL_T* pThis )
{
   mprintf( ESC_BOLD ESC_FG_CYAN
            "Slot: %d, Channel %d, Address: 0x%08x, Bus address: 0x%08x\n"
            ESC_NORMAL,
            daqChannelGetSlot( pThis ),
            daqChannelGetNumber( pThis ) + 1,
            daqChannelGetRegPtr( pThis ),
            daqChannelGetScuBusSlaveBaseAddress( pThis )
          );
   const uint16_t ctrlReg = *(uint16_t*)daqChannelGetCtrlRegPtr( pThis );
   mprintf( "  CtrlReg: &0x%08x *0x%04x *0b",
            daqChannelGetCtrlRegPtr( pThis ), ctrlReg );

   for( uint16_t i = 1 << (BIT_SIZEOF(uint16_t) - 1); i != 0; i >>= 1 )
      mprintf( "%c", (ctrlReg & i)? '1' : '0' );

   mprintf( "\n    Ena_PM:                %s\n",
            daqChannelIsPostMortemActive( pThis )? g_pYes : g_pNo );
   mprintf( "    Sample10us:            %s\n",
            daqChannelIsSample10usActive( pThis )? g_pYes : g_pNo );
   mprintf( "    Sample100us:           %s\n",
            daqChannelIsSample100usActive( pThis )? g_pYes : g_pNo );
   mprintf( "    Sample1ms:             %s\n",
            daqChannelIsSample1msActive( pThis )? g_pYes : g_pNo );
   mprintf( "    Ena_TrigMod:           %s\n",
            daqChannelIsTriggerModeEnabled( pThis )? g_pYes : g_pNo );
   mprintf( "    ExtTrig_nEvTrig:       %s\n",
            daqChannelGetTriggerSource( pThis )?  g_pYes : g_pNo );
   mprintf( "    Ena_HiRes:             %s\n",
            daqChannelIsHighResolutionEnabled( pThis )? g_pYes : g_pNo );
   mprintf( "    ExtTrig_nEvTrig_HiRes: %s\n",
            daqChannelGetTriggerSourceHighRes( pThis )? g_pYes : g_pNo );
   mprintf( "  Trig_LW:  &0x%08x *0x%04x\n",
            &__DAQ_GET_CHANNEL_REG( TRIG_LW ),
            daqChannelGetTriggerConditionLW( pThis ) );
   mprintf( "  Trig_HW:  &0x%08x *0x%04x\n",
            &__DAQ_GET_CHANNEL_REG( TRIG_HW ),
            daqChannelGetTriggerConditionHW( pThis ) );
   mprintf( "  Trig_Dly: &0x%08x *0x%04x\n",
            &__DAQ_GET_CHANNEL_REG( TRIG_DLY ),
            daqChannelGetTriggerDelay( pThis ) );
   mprintf( "  DAQ int pending:     %s\n",
         (((*daqChannelGetDaqIntPendingPtr( pThis )) & pThis->intMask) != 0 )?
         g_pYes : g_pNo );
   mprintf( "  HiRes int pending:   %s\n",
       (((*daqChannelGetHiResIntPendingPtr( pThis )) & pThis->intMask) != 0 )?
         g_pYes : g_pNo );
   mprintf( "  VHDL Macro version:  %d\n",
            daqChannelGetMacroVersion( pThis ));
   mprintf( "  Level DAQ FiFo:      %d words\n",
            daqChannelGetDaqFifoWords( pThis ));
   mprintf( "  Level PM_HiRes FiFo: %d words \n",
            daqChannelGetPmFifoWords( pThis ));
   mprintf( "  Channels:            %d\n",
            daqChannelGetMaxCannels( pThis ));
}
#endif /* defined( CONFIG_DAQ_DEBUG ) || defined(__DOXYGEN__) */

/*======================== DAQ- Device Functions ============================*/

/*! ---------------------------------------------------------------------------
 * @see daq.h
 */
unsigned int daqDeviceGetUsedChannels( register DAQ_DEVICE_T* pThis )
{
   DAQ_ASSERT( pThis != NULL );
   unsigned int retVal = 0;

   for( int i = daqDeviceGetMaxChannels( pThis )-1; i >= 0; i-- )
   {
      if( !daqDeviceGetChannelObject( pThis, i )->properties.notUsed )
         retVal++;
   }
   return retVal;
}

//IMPLEMENT_CONVERT_BYTE_ENDIAN( uint64_t )

/*! ---------------------------------------------------------------------------
 * @see daq.h
 */
void daqDevicePresetTimeStampCounter( register DAQ_DEVICE_T* pThis,
                                      volatile const uint32_t timeOffset )
{
   DAQ_ASSERT( pThis != NULL );
   DAQ_ASSERT( pThis->pReg != NULL );
   STATIC_ASSERT( TS_COUNTER_WD1+1 == TS_COUNTER_WD2 );
   STATIC_ASSERT( TS_COUNTER_WD1+2 == TS_COUNTER_WD3 );
   STATIC_ASSERT( TS_COUNTER_WD1+3 == TS_COUNTER_WD4 );

#if 0
   mprintf( "Time-Offset: 0x%08X\n", timeOffset );
#endif

   /*
    * CAUTION!
    * Because of a compiler bug it's necessary using the keyword "volatile" for
    * this 64-bit variable.
    */
   volatile const uint64_t futureTime = timeOffset * 1000000L + getWrSysTime();

   for( unsigned int i = 0; i < (sizeof(uint64_t)/sizeof(uint16_t)); i++ )
   {
    #if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
      pThis->pReg->i[TS_COUNTER_WD1+i] = ((uint16_t*)&futureTime)[((sizeof(uint64_t)/sizeof(uint16_t))-1) - i];
    #else
      pThis->pReg->i[TS_COUNTER_WD1+i] = ((uint16_t*)&futureTime)[i];
    #endif
    #if 0
      mprintf( "pTS[%d]: %p, %04X, %04X, %04x\n", i,
               &pThis->pReg->i[TS_COUNTER_WD1+i], pThis->pReg->i[TS_COUNTER_WD1+i],
               ((uint16_t*)&futureTime)[((sizeof(uint64_t)/sizeof(uint16_t))-1) - i],
               ((uint16_t*)&futureTime)[i]
      );
    #endif
   }
}

/*! ---------------------------------------------------------------------------
 * @see daq.h
 */
uint64_t daqDeviceGetTimeStampCounter( register DAQ_DEVICE_T* pThis )
{
   DAQ_ASSERT( pThis != NULL );
   DAQ_ASSERT( pThis->pReg != NULL );
   STATIC_ASSERT( TS_COUNTER_WD1+1 == TS_COUNTER_WD2 );
   STATIC_ASSERT( TS_COUNTER_WD1+2 == TS_COUNTER_WD3 );
   STATIC_ASSERT( TS_COUNTER_WD1+3 == TS_COUNTER_WD4 );

   /*
    * CAUTION!
    * Because of a compiler bug it's necessary using the keyword "volatile" for
    * this 64-bit variable.
    */
   volatile uint64_t ts = 0;

   for( unsigned int i = 0; i < (sizeof(uint64_t)/sizeof(uint16_t)); i++ )
   {
    #if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
      ((uint16_t*)&ts)[((sizeof(uint64_t)/sizeof(uint16_t))-1) - i] = pThis->pReg->i[TS_COUNTER_WD1+i];
    #else
      ((uint16_t*)&ts)[i] = pThis->pReg->i[TS_COUNTER_WD1+i];
    #endif
     // mprintf( "pTS: %p\n", &pThis->pReg->i[TS_COUNTER_WD1+i] );
     // mprintf( "pTS: %p, %04X\n", &pThis->pReg->i[TS_COUNTER_WD1+i], pThis->pReg->i[TS_COUNTER_WD1+i]);
   }
   return ts;
}

/*! ---------------------------------------------------------------------------
 * @see daq.h
 */
void daqDeviceSetTimeStampCounterEcaTag( register DAQ_DEVICE_T* pThis, const uint32_t tsTag )
{
   DAQ_ASSERT( pThis != NULL );
   DAQ_ASSERT( pThis->pReg != NULL );
   STATIC_ASSERT( TS_CNTR_TAG_LW+1 == TS_CNTR_TAG_HW );
#if 0
   mprintf( "ECA-Tag: 0x%08X\n", tsTag );
#endif

   for( unsigned int i = 0; i < (sizeof(uint32_t)/sizeof(uint16_t)); i++ )
   {
    #if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
      pThis->pReg->i[TS_CNTR_TAG_LW+i] = ((uint16_t*)&tsTag)[((sizeof(uint32_t)/sizeof(uint16_t))-1) - i];
    #else
      pThis->pReg->i[TS_CNTR_TAG_LW+i] = ((uint16_t*)&tsTag)[i];
    #endif
    #if 0
      mprintf( "ECA-Tag[%u]: %04X\n", i, pThis->pReg->i[TS_CNTR_TAG_LW+i] );
    #endif
   }
}

/*! ---------------------------------------------------------------------------
 * @see daq.h
 */
uint32_t daqDeviceGetTimeStampTag( register DAQ_DEVICE_T* pThis )
{
   DAQ_ASSERT( pThis != NULL );
   DAQ_ASSERT( pThis->pReg != NULL );
   STATIC_ASSERT( TS_CNTR_TAG_LW+1 == TS_CNTR_TAG_HW );
   uint32_t tsTag = 0;

   for( unsigned int i = 0; i < (sizeof(uint32_t)/sizeof(uint16_t)); i++ )
   {
    #if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
      ((uint16_t*)&tsTag)[((sizeof(uint32_t)/sizeof(uint16_t))-1) - i] = pThis->pReg->i[TS_CNTR_TAG_LW+i];
    #else
      ((uint16_t*)&tsTag)[i] = pThis->pReg->i[TS_CNTR_TAG_LW+i];
    #endif
   }
   return tsTag;
}

#ifndef CONFIG_DAQ_SINGLE_APP
#define FSM_INIT_FSM( s, attr... ) pFeedback->status = s

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_DEVICE
 * @brief Resetting of feedback command buffer.
 * @param pThis Pointer to the DAQ-device object
 */
STATIC inline
void daqDeviceFeedBackReset( register DAQ_DEVICE_T* pThis )
{
   DAQ_FEEDBACK_T* pFeedback = &pThis->feedback;
   FSM_INIT_FSM( FB_READY, label='Reset' );
   QUEUE_INIT_MEMBER( pFeedback, aktionBuffer, DAQ_ACTION_ITEM_T );
}
#endif

/*! ---------------------------------------------------------------------------
 * @see daq.h
 */
void daqDeviceReset( register DAQ_DEVICE_T* pThis )
{
   DAQ_ASSERT( pThis != NULL );
   DAQ_ASSERT( pThis->pReg != NULL );

   for( int i = daqDeviceGetMaxChannels( pThis )-1; i >= 0; i-- )
      daqChannelReset( daqDeviceGetChannelObject( pThis, i ) );

   //!!daqDeviceSetTimeStampCounter( pThis, 0L );
   //daqDeviceSetTimeStampTag( pThis, 0 );

#ifndef CONFIG_DAQ_SINGLE_APP
   daqDeviceFeedBackReset( pThis );
#endif
}

#ifndef CONFIG_DAQ_SINGLE_APP

/*! ---------------------------------------------------------------------------
 * @see daq.h
 */
void daqDevicePutFeedbackSwitchCommand( register DAQ_DEVICE_T* pThis,
                                        const DAQ_FEEDBACK_ACTION_T what,
                                        const unsigned int fgNumber
                                      )
{
   const DAQ_ACTION_ITEM_T act = { .action = what, .fgNumber = fgNumber };
   if( !queuePush( &pThis->feedback.aktionBuffer, &act ) )
   {
   #ifdef CONFIG_USE_LM32LOG
      lm32Log( LM32_LOG_ERROR, ESC_ERROR "ERROR: DAQ command buffer of slot %u full!\n" ESC_NORMAL,
               daqDeviceGetSlot( pThis ) );
   #else
      mprintf( ESC_ERROR "Error: DAQ command buffer of slot %u full!\n" ESC_NORMAL,
               daqDeviceGetSlot( pThis ) );
   #endif
   }
}

#define FSM_TRANSITION( s, attr... ) pFeedback->status = s
#define FSM_TRANSITION_SELF( attr... )
/*!
 * @brief Time distance between two switch-on events of DAQ channels
 */
//#define DAQ_SWITCH_WAITING_TIME 1000000ULL
#define DAQ_SWITCH_WAITING_TIME (1000000ULL - 50000)

/*! ---------------------------------------------------------------------------
 * @brief Finite state machine which handles the on/off switching of
 *        feed-back channels for ADDAC- function generators.

 * @dotfile daq.gv
 */
STATIC bool daqDeviceDoFeedbackSwitchOnOffFSM( register DAQ_DEVICE_T* pThis )
{
   DAQ_FEEDBACK_T* pFeedback = &pThis->feedback;

   const DAQ_FEEDBACK_STATUS_T oldStatus = pFeedback->status;

   /*
    * FSM-do activities:
    */
   switch( oldStatus )
   {
      case FB_READY:
      {
         DAQ_ACTION_ITEM_T act;
         /*
          * Command from FG-layer received?
          */
         if( !queuePop( &pFeedback->aktionBuffer, &act ) )
         { /*
            * No!
            */
            FSM_TRANSITION_SELF( label='No message.' );
            break;
         }

         pFeedback->fgNumber = act.fgNumber;
         DAQ_CANNEL_T* pSetChannel = &pThis->aChannel[daqGetSetDaqNumberOfFg(pFeedback->fgNumber, pThis->type)];
         DAQ_CANNEL_T* pActChannel = &pThis->aChannel[daqGetActualDaqNumberOfFg(pFeedback->fgNumber, pThis->type)];
         pSetChannel->sequenceContinuous = 0;
         pActChannel->sequenceContinuous = 0;

         /*
          * Evaluating of DAQ command coming from function generator.
          */
         switch( act.action )
         {
            case FB_OFF:
            { /*
               * Switching both DAQ channels for set and actual value off.
               */
               ATOMIC_SECTION()
               {
                  daqChannelSample1msOff( pSetChannel );
                  daqChannelTestAndClearDaqIntPending( pSetChannel );
                  daqChannelSample1msOff( pActChannel );
                  daqChannelTestAndClearDaqIntPending( pActChannel );
               }
               FSM_TRANSITION_SELF( label='Switch both channels off\nif stop-message received.' );
               break;
            }
            case FB_ON:
            { /*!
               * @todo Maybe a misunderstanding in the DAQ specification of KHK or
               *       a bug in the VHDL code.\n
               *       The corresponding workaround on Linux is made by the
               *       compiler switch: _CONFIG_PATCH_PHASE.\n
               *       It's a construction site yet!
               */
               daqChannelSetTriggerDelay( pSetChannel, 10 ); //TODO
               // daqChannelEnableTriggerMode( pSetChannel );
               // daqChannelEnableEventTrigger( pSetChannel );

               //daqChannelEnableExtrenTrigger
               daqChannelSample1msOn( pSetChannel );
              // mprintf( "D=%d\n", daqChannelGetTriggerDelay( pSetChannel ) );

               FSM_TRANSITION( FB_FIRST_ON, label='Start message received.\n'
                                            'Switch DAQ for set value on.' );
               break;
            }
            default: DAQ_ASSERT( false );
         } /* End switch( act.action ) */
         break;
      } /* End case FB_READY */

      case FB_FIRST_ON:
      {
         if( getWrSysTimeSafe() < pFeedback->waitingTime )
         {
            FSM_TRANSITION_SELF();
            break;
         }
         DAQ_CANNEL_T* pActChannel = &pThis->aChannel[daqGetActualDaqNumberOfFg(pFeedback->fgNumber, pThis->type)];
         daqChannelSample1msOn( pActChannel );

         FSM_TRANSITION( FB_BOTH_ON, label='Waiting time expired.\n'
                                           'Switch DAQ for actual value on.' );
         break;
      } /* End case FB_FIRST_ON */

      case FB_BOTH_ON:
      {
         if( getWrSysTimeSafe() < pFeedback->waitingTime )
         {
            FSM_TRANSITION_SELF();
            break;
         }
         FSM_TRANSITION( FB_READY, label='Waiting time expired.');
         break;
      } /* End case FB_BOTH_ON */

      default: DAQ_ASSERT( false );
   } /* End switch( oldStatus ) */

   /*
    * Has the FSM- state not changed?
    */
   if( oldStatus == pFeedback->status )
      return true;

   /*
    * FSM- entry activities:
    */
   switch( pFeedback->status )
   {
      case FB_FIRST_ON: /* Immediately to the next case, no break here. */
      case FB_BOTH_ON:
      {
         pFeedback->waitingTime = getWrSysTimeSafe() + DAQ_SWITCH_WAITING_TIME;
         break;
      }

      default: break;
   }
   return true;
}

#endif /* ifndef CONFIG_DAQ_SINGLE_APP */

#if defined( CONFIG_DAQ_DEBUG ) || defined(__DOXYGEN__)
/*! ---------------------------------------------------------------------------
 * @see daq.h
 */
void daqDevicePrintInfo( register DAQ_DEVICE_T* pThis )
{
   mprintf( "Macro version: %d\n", daqDeviceGetMacroVersion( pThis ) );
   unsigned int maxChannels = daqDeviceGetMaxChannels( pThis );
   for( unsigned int i = 0; i < maxChannels; i++ )
      daqChannelPrintInfo( daqDeviceGetChannelObject( pThis, i ));
}

/*! ---------------------------------------------------------------------------
 * @see daq.h
 */
void daqDevicePrintInterruptStatus( register DAQ_DEVICE_T* pThis )
{
   uint16_t flags = scuBusGetSlaveValue16(
                   daqDeviceGetScuBusSlaveBaseAddress( pThis ), Intr_Active );
   mprintf( "SCU slave DAQ interrupt active:   %s\n",
            ((flags & (1 << DAQ_IRQ_DAQ_FIFO_FULL)) != 0)? g_pYes : g_pNo);
   mprintf( "SCU slave HiRes interrupt active: %s\n",
            ((flags & (1 << DAQ_IRQ_HIRES_FINISHED)) != 0)? g_pYes : g_pNo);
}

#endif /* defined( CONFIG_DAQ_DEBUG ) || defined(__DOXYGEN__) */


/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_DEVICE
 * @brief Scans all potential existing input-channels of the given
 *        DAQ-Device ant initialize each found channel with
 *        the slot number.
 * @param pThis Pointer to the DAQ device object
 * @param slot slot number
 * @return Number of real existing channels
 */
ONE_TIME_CALL
int daqDeviceFindChannels( DAQ_DEVICE_T* pThis, const unsigned int slot )
{
   DAQ_ASSERT( pThis != NULL );
   DAQ_ASSERT( pThis->pReg != NULL );

   for( unsigned int channel = 0; channel < ARRAY_SIZE(pThis->aChannel);
        channel++ )
   {
      DBPRINT2( "DBG: Slot: %02d, Channel: %02d, ctrlReg: 0x%04X\n",
             slot, channel, daqChannelGetReg( pThis->pReg, CtrlReg, channel ));

      DAQ_CANNEL_T* pCurrentChannel = &pThis->aChannel[channel];
      pCurrentChannel->n = channel;

      /*
       * Checking whether the macro DAQ_CHANNEL_GET_PARENT_OF works
       * correct.
       */
      DAQ_ASSERT( pThis == DAQ_CHANNEL_GET_PARENT_OF( pCurrentChannel ) );

      /*
       * In the case of a warm start, clearing eventually old values
       * in the entire control register.
       */
      *((DAQ_REGISTER_T*)daqChannelGetCtrlRegPtr( pCurrentChannel )) = 0;

      /*
       * The next three lines probes the channel by writing and read back
       * the slot number. At the first look this algorithm seems not meaningful
       * (writing and immediately reading a value from the same memory place)
       * but we have to keep in mind that is a memory mapped IO areal and
       * its attribute was declared as "volatile".
       *
       * If no (further) channel present the value of the control-register is
       * 0xDEAD. That means the slot number has the hex number 0xD (13).
       * Fortunately the highest slot number is 0xC (12). Therefore no further
       * probing is necessary.
       */
      DBPRINT2( "DBG: ctrReg: 0b%04b\n",
              *((DAQ_REGISTER_T*)daqChannelGetCtrlRegPtr( pCurrentChannel )) );
      daqChannelGetCtrlRegPtr( pCurrentChannel )->slot = slot;
      DBPRINT2( "DBG: ctrReg: 0b%04b\n",
              *((DAQ_REGISTER_T*)daqChannelGetCtrlRegPtr( pCurrentChannel )) );
      if( daqChannelGetSlot( pCurrentChannel ) != slot )
      {
         const char* text = ESC_WARNING
                            "No DAQ-channel %u found on slave %u !\n"
                            ESC_NORMAL;
         lm32Log( LM32_LOG_WARNING, text, channel, slot );
       #ifndef CONFIG_NO_DAQ_INFO_PRINT
         mprintf( text, channel, slot );
       #endif
         continue; /* Supposing this channel isn't present. */
      }
      DAQ_ASSERT((*((DAQ_REGISTER_T*)daqChannelGetCtrlRegPtr(
                                           pCurrentChannel )) & 0x0FFF) == 0 );
      /* If the assertion above has been occurred, check the element types
       * of the structure DAQ_CTRL_REG_T.
       * At least one element type has to be greater or equal like uint16_t.
       */

      DAQ_ASSERT( channel < BIT_SIZEOF( pCurrentChannel->intMask ));
      pCurrentChannel->intMask = 1 << channel;

      DBPRINT2( "DBG: Slot of channel %d: %d\n", channel,
                daqChannelGetSlot( pCurrentChannel ) );

      pThis->maxChannels++;
      const char* text = ESC_FG_CYAN
                         "%s-DAQ channel %2u in slot %2u initialized. Address: 0x%p\n"
                         ESC_NORMAL;
      lm32Log( LM32_LOG_INFO, text, daqDeviceTypeToString( pThis->type ),
               channel, daqChannelGetSlot( pCurrentChannel ),
               pCurrentChannel );
    #ifndef CONFIG_NO_DAQ_INFO_PRINT
      mprintf( text,
               daqDeviceTypeToString( pThis->type ),
               channel, daqChannelGetSlot( pCurrentChannel ),
               pCurrentChannel
             );
    #endif
   }
   return pThis->maxChannels;
}

/*============================ DAQ Bus Functions ============================*/
#ifndef CONFIG_DAQ_SIMULATE_CHANNEL

/*! ---------------------------------------------------------------------------
 * @brief Find a possible ACU-slave on SCU bus slot 1.
 */
ONE_TIME_CALL
SCUBUS_SLAVE_FLAGS_T findAcuMfuDeviceOnSlot1( const void* pScuBusBase )
{ /*
   * A possible ACU / MFU slave device resides always in slot 1 only.
   */
   const void* pSlaveAddr = scuBusGetAbsSlaveAddr( pScuBusBase, 1 );
   switch( scuBusGetSlaveValue16( pSlaveAddr, CID_SYSTEM ) )
   {
      case SYS_PBRF: break;
      case SYS_LOEP: break;
      case SYS_CSCO: break;
      default:       return 0x000;
   }

   if( scuBusGetSlaveValue16( pSlaveAddr, CID_GROUP ) == GRP_MFU )
   { /*
      * ACU device on slot 1 found, return by slave-flags: 0000 0000 0001
      */
      return 0x001;
   }

   return 0x000;
}

/*! ---------------------------------------------------------------------------
 * @brief Finds all ADDAC-devices residing on the SCU bus.
 */
ONE_TIME_CALL
SCUBUS_SLAVE_FLAGS_T findAllAddacDevices( const void* pScuBusBase )
{
   return scuBusFindSpecificSlaves( pScuBusBase, SYS_CSCO, GRP_ADDAC2 ) |
          scuBusFindSpecificSlaves( pScuBusBase, SYS_CSCO, GRP_ADDAC1 );
}

#ifdef CONFIG_DIOB_WITH_DAQ
#warning DIOB-DAQ not compleatly implemented yet!
#endif

/*! ---------------------------------------------------------------------------
 * @see daq.h
 */
int daqBusFindAndInitializeAll( register DAQ_BUS_T* pThis,
                                const void* pScuBusBase
                            #ifndef CONFIG_DAQ_SINGLE_APP
                                ,FG_MACRO_T* pFgList
                            #endif
                              )
{
   /*
    * Paranoia... ;-)
    */
   DAQ_ASSERT( pScuBusBase != (void*)ERROR_NOT_FOUND );
   DAQ_ASSERT( pThis != NULL );

   DAQ_DEVICE_TYP_T currentType = UNKNOWN;
   SCUBUS_SLAVE_FLAGS_T addacSlots;
#ifdef CONFIG_DIOB_WITH_DAQ
   SCUBUS_SLAVE_FLAGS_T diobSlots;
#endif
   /*
    * Pre-initializing
    */
   memset( pThis, 0, sizeof( DAQ_BUS_T ) );

   /*
    * Checking whether in slot 1 is a ACU-device.
    */
   pThis->slotDaqUsedFlags = findAcuMfuDeviceOnSlot1( pScuBusBase );
   if( pThis->slotDaqUsedFlags != 0 )
   { /*
      * ACU device on slot 1 found, no further scans will made.
      */
      currentType = ACU;
   }
   else
   { /*
      * No ACU device found, therefore now scanning the whole SCU-bus
      * for ADDAC-DAQ- slaves.
      */
      pThis->slotDaqUsedFlags = addacSlots = findAllAddacDevices( pScuBusBase );
   #ifdef CONFIG_DIOB_WITH_DAQ
      pThis->slotDaqUsedFlags |= diobSlots = scuBusFindSpecificSlaves( pScuBusBase, SYS_CSCO, GRP_DIOB );
   #endif
      if( pThis->slotDaqUsedFlags == 0 )
      {
         DBPRINT( "DBG: Neither ADDAC nor ACU slaves found!\n" );
         return 0;
      }
   }

   /*
    * Initializing all found devices which have a DAQ and its channels.
    */
   SCU_BUS_FOR_EACH_SLAVE( slot, pThis->slotDaqUsedFlags )
   {
      /*
       * For each found ADDAC-device:
       */

      DAQ_DEVICE_T* pCurrentDaqDevice = &pThis->aDaq[pThis->foundDevices];
      pCurrentDaqDevice->type = UNKNOWN;
      pCurrentDaqDevice->n = pThis->foundDevices;

   #ifndef CONFIG_DAQ_SINGLE_APP
      if( pFgList != NULL )
      {
   #endif
        /*
         * Making the ADDAC resp. ACU function-generator known for SAFT-LIB.
         */
         if( currentType == ACU )
         {
            pCurrentDaqDevice->type = currentType;
          #ifndef CONFIG_DAQ_SINGLE_APP
            addAcuToFgList( pScuBusBase, slot, pFgList );
          #endif
         }
         else if( scuBusIsSlavePresent( addacSlots, slot ) )
         {
            pCurrentDaqDevice->type = ADDAC;
          #ifndef CONFIG_DAQ_SINGLE_APP
            addAddacToFgList( pScuBusBase, slot, pFgList );
          #endif
         }
       #ifdef CONFIG_DIOB_WITH_DAQ
         else if( scuBusIsSlavePresent( diobSlots, slot ) )
         {
            pCurrentDaqDevice->type = DIOB;
          #ifndef CONFIG_DAQ_SINGLE_APP
            addDiobToFgList( pScuBusBase, slot, pFgList );
          #endif
         }
       #endif
   #ifndef CONFIG_DAQ_SINGLE_APP
      }
   #endif /* ifndef CONFIG_DAQ_SINGLE_APP */

     /*
      * Because the register access to the DAQ device is more frequent than
      * to the registers of the SCU slave, therefore the base address of the
      * DAQ registers are noted rather than the SCU bus slave address.
      */
      pCurrentDaqDevice->pReg =
          scuBusGetAbsSlaveAddr( pScuBusBase, slot ) + DAQ_REGISTER_OFFSET;

      DBPRINT2( "DBG: DAQ found in slot: %2d, address: 0x%08X\n", slot,
                pCurrentDaqDevice->pReg );

   #ifndef _CONFIG_IRQ_ENABLE_IN_START_FG
      scuBusEnableSlaveInterrupt( pScuBusBase, slot );
   #endif

      /*
       * Find and initialize all DAQ-channels of the current DAQ-device.
       */
      if( daqDeviceFindChannels( pCurrentDaqDevice, slot ) == 0 )
      {
         DBPRINT2( "DBG: DAQ in slot %d has no input channels - skipping\n", slot );
         continue;
      }

      pCurrentDaqDevice->slot = slot;
     /*
      * At least one channel was found.
      */
      pThis->foundDevices++;

      DAQ_ASSERT( pCurrentDaqDevice->maxChannels ==
                   daqChannelGetMaxCannels( &pCurrentDaqDevice->aChannel[0] ) );
      DAQ_ASSERT( DAQ_DEVICE_GET_PARENT_OF( pCurrentDaqDevice ) == pThis );

    // daqDeviceDisableScuSlaveInterrupt( pCurrentDaqDevice );
      daqDeviceEnableScuSlaveInterrupt( pCurrentDaqDevice ); //!!
      daqDeviceTestAndClearDaqInt( pCurrentDaqDevice );
      daqDeviceTestAndClearHiResInt( pCurrentDaqDevice );

      daqDeviceClearDaqChannelInterrupts( pCurrentDaqDevice );
      daqDeviceClearHiResChannelInterrupts( pCurrentDaqDevice );

      daqDeviceSetTimeStampCounterEcaTag( pCurrentDaqDevice, DAQ_DEFAULT_ECA_SYNC_TAG );
      daqDevicePresetTimeStampCounter( pCurrentDaqDevice, DAQ_DEFAULT_SYNC_TIMEOFFSET );

#if 0
      uint64_t ts = daqDeviceGetTimeStampCounter( pCurrentDaqDevice );
      mprintf( "ts: 0x%08X%08X\n", ((uint32_t*)&ts)[0], ((uint32_t*)&ts)[1] );
#endif


#if DAQ_MAX < MAX_SCU_SLAVES
      if( pThis->foundDevices == ARRAY_SIZE( pThis->aDaq ) )
         break;
#endif
   }

   /*
    * In the case of re-initializing respectively warm-start a
    * reset for all DAQ devices becomes necessary.
    * Because a new start of the software doesn't concern
    * the hardware registers of the SCU bus slaves.
    */

   if( pThis->foundDevices > 0 )
      daqBusReset( pThis );

   return pThis->foundDevices;
}

#endif /* ifndef CONFIG_DAQ_SIMULATE_CHANNEL */

/*! ---------------------------------------------------------------------------
 * @see daq.h
 */
bool daqBusIsAcuDeviceOnly( register DAQ_BUS_T* pThis )
{
   for( int i = 0; i < pThis->foundDevices; i++ )
      if( pThis->aDaq[i].type == ACU )
         return true;

   return false;
}

/*! ---------------------------------------------------------------------------
 * @see daq.h
 */
int daqBusGetNumberOfAllFoundChannels( register DAQ_BUS_T* pThis )
{
   int ret = 0;
   DAQ_ASSERT( pThis->foundDevices <= ARRAY_SIZE(pThis->aDaq) );
   for( int i = 0; i < pThis->foundDevices; i++ )
      ret += pThis->aDaq[i].maxChannels;
   return ret;
}

/*! ---------------------------------------------------------------------------
 * @see daq.h
 */
DAQ_DEVICE_T* daqBusGetDeviceBySlotNumber( register DAQ_BUS_T* pThis,
                                           const unsigned int slot )
{
   for( unsigned int i = 0; i < pThis->foundDevices; i++ )
   {
      DAQ_DEVICE_T* pDevice = daqBusGetDeviceObject( pThis, i );
      if( slot == daqDeviceGetSlot( pDevice ) )
         return pDevice;
   }
   return NULL;
}

/*! ---------------------------------------------------------------------------
 * @see daq.h
 */
DAQ_CANNEL_T* daqBusGetChannelObjectByAbsoluteNumber( register DAQ_BUS_T* pThis,
                                                      const unsigned int n )
{
   unsigned int deviceCounter;
   unsigned int relativeChannelCounter;
   unsigned int absoluteChannelCounter = 0;

   for( deviceCounter = 0; deviceCounter < daqBusGetFoundDevices(pThis); deviceCounter++ )
   {
      DAQ_DEVICE_T* pDevice = daqBusGetDeviceObject( pThis, deviceCounter );
      for( relativeChannelCounter = 0;
           relativeChannelCounter < daqDeviceGetMaxChannels( pDevice );
           relativeChannelCounter++ )
      {
         if( absoluteChannelCounter == n )
            return daqDeviceGetChannelObject( pDevice, relativeChannelCounter );
         absoluteChannelCounter++;
      }
   }
   return NULL;
}

/*! ---------------------------------------------------------------------------
 * @see daq.h
 */
unsigned int daqBusGetUsedChannels( register DAQ_BUS_T* pThis )
{
   DAQ_ASSERT( pThis != NULL );
   unsigned int retVal = 0;

   for( int i = daqBusGetFoundDevices( pThis )-1; i >= 0; i-- )
   {
      retVal += daqDeviceGetUsedChannels( daqBusGetDeviceObject( pThis, i ) );
   }
   return retVal;
}

/*! ---------------------------------------------------------------------------
 * @see daq.h
 */
void daqBusEnableSlaveInterrupts( register DAQ_BUS_T* pThis )
{
   DAQ_ASSERT( pThis != NULL );
   for( int i = daqBusGetFoundDevices( pThis )-1; i >= 0; i-- )
      daqDeviceEnableScuSlaveInterrupt( daqBusGetDeviceObject( pThis, i ) );
}

/*! ---------------------------------------------------------------------------
 * @see daq.h
 */
void daqBusDisablSlaveInterrupts( register DAQ_BUS_T* pThis )
{
   DAQ_ASSERT( pThis != NULL );
   for( int i = daqBusGetFoundDevices( pThis )-1; i >= 0; i-- )
      daqDeviceDisableScuSlaveInterrupt( daqBusGetDeviceObject( pThis, i ) );
}

/*! ---------------------------------------------------------------------------
 * @see daq.h
 */
void daqBusClearAllPendingInterrupts( register DAQ_BUS_T* pThis )
{
   DAQ_ASSERT( pThis != NULL );

   for( int i = daqBusGetFoundDevices( pThis )-1; i >= 0; i-- )
   {
      DAQ_DEVICE_T* pDaqSlave = daqBusGetDeviceObject( pThis, i );
      daqDeviceClearDaqChannelInterrupts( pDaqSlave );
      daqDeviceClearHiResChannelInterrupts( pDaqSlave );
   }
}

/*! ---------------------------------------------------------------------------
 * @see daq.h
 */
void daqBusPresetAllTimeStampCounters( register DAQ_BUS_T* pThis, const uint32_t timeOffset )
{
   DAQ_ASSERT( pThis != NULL );

   for( int i = daqBusGetFoundDevices( pThis )-1; i >= 0; i-- )
      daqDevicePresetTimeStampCounter( daqBusGetDeviceObject( pThis, i ), timeOffset );
}

/*! ---------------------------------------------------------------------------
 * @see daq.h
 */
void daqBusSetAllTimeStampCounterEcaTags( register DAQ_BUS_T* pThis, const uint32_t tsTag )
{
   DAQ_ASSERT( pThis != NULL );

   for( int i = daqBusGetFoundDevices( pThis )-1; i >= 0; i-- )
      daqDeviceSetTimeStampCounterEcaTag( daqBusGetDeviceObject( pThis, i ), tsTag );
}

/*! ---------------------------------------------------------------------------
 * @see daq.h
 */
void daqBusReset( register DAQ_BUS_T* pThis )
{
   DAQ_ASSERT( pThis != NULL );

   pThis->lastErrorState.status  = DAQ_RECEIVE_STATE_OK;
   pThis->lastErrorState.channel = 0;
   pThis->lastErrorState.slot    = 0;

   for( int i = daqBusGetFoundDevices( pThis )-1; i >= 0; i-- )
      daqDeviceReset( daqBusGetDeviceObject( pThis, i ) );
}

#ifndef CONFIG_DAQ_SINGLE_APP
/*! ---------------------------------------------------------------------------
 * @see daq.h
 */
void daqBusDoFeedbackTask( register DAQ_BUS_T* pThis )
{
   DAQ_ASSERT( daqBusGetFoundDevices( pThis ) != 0 );
   static unsigned int currentDevNum = 0;
   if( daqDeviceDoFeedbackSwitchOnOffFSM( daqBusGetDeviceObject( pThis, currentDevNum ) ))
   {
      currentDevNum++;
      currentDevNum %= daqBusGetFoundDevices( pThis );
   }
}

#endif /* ifndef CONFIG_DAQ_SINGLE_APP */

#if defined( CONFIG_DAQ_DEBUG ) || defined(__DOXYGEN__)
/*! ---------------------------------------------------------------------------
 * @see daq.h
 */
void daqBusPrintInfo( register DAQ_BUS_T* pThis )
{
   const unsigned int maxDevices = daqBusGetFoundDevices( pThis );
   for( unsigned int i = 0; i < maxDevices; i++ )
      daqDevicePrintInfo( daqBusGetDeviceObject( pThis, i ) );
}

#endif /* defined( CONFIG_DAQ_DEBUG ) || defined(__DOXYGEN__) */

/*======================== DAQ- Descriptor functions ========================*/
#if defined( CONFIG_DAQ_DEBUG ) || defined(__DOXYGEN__)
/*! --------------------------------------------------------------------------
 * @see daq_descriptor.h
 * @see daq.h
 */
void daqDescriptorPrintInfo( register DAQ_DESCRIPTOR_T* pThis )
{
   IMPLEMENT_CONVERT_BYTE_ENDIAN( uint32_t )

   mprintf( ESC_BOLD ESC_FG_CYAN
            "Device Descriptor:\n" ESC_NORMAL );
   mprintf( "  Slot:            %d\n", daqDescriptorGetSlot( pThis ) );
   mprintf( "  Channel:         %d\n", daqDescriptorGetChannel( pThis ) + 1 );
   mprintf( "  DIOB ID:         %d\n", daqDescriptorGetDiobId( pThis ) );
   mprintf( "  Post Mortem:     %s\n", daqDescriptorWasPM( pThis )?
                                       g_pYes : g_pNo );
   mprintf( "  High Resolution: %s\n", daqDescriptorWasHiRes( pThis )?
                                       g_pYes : g_pNo );
   mprintf( "  DAQ mode:        %s\n", daqDescriptorWasDaq( pThis )?
                                       g_pYes : g_pNo );
   mprintf( "  Trigger low:     0x%04x\n",
            daqDescriptorGetTriggerConditionLW( pThis ) );
   mprintf( "  Trigger high:    0x%04x\n",
            daqDescriptorGetTriggerConditionHW( pThis ) );
   mprintf( "  Trigger delay:   0x%04x\n",
            daqDescriptorGetTriggerDelay( pThis ) );
   mprintf( "  Seconds:       %08u\n",
            convertByteEndian_uint32_t( daqDescriptorGetTimeStampSec( pThis ) ));
   mprintf( "  Nanoseconds:   %09u\n",
            convertByteEndian_uint32_t( daqDescriptorGetTimeStampNanoSec( pThis )));
   mprintf( "  Sequence:        %d\n", daqDescriptorGetSequence( pThis ));
   mprintf( "  CRC:             0x%02x\n", daqDescriptorGetCRC( pThis ));
}

#endif // if defined( CONFIG_DAQ_DEBUG ) || defined(__DOXYGEN__)

/*================================== EOF ====================================*/
