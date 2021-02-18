/*!
 * @file eca_queue_type.h
 * @brief Definition Event Conditioned Action (ECA) register object
 *        type for Wishbone interface of VHDL entity
 * @author Ulrich Becker <u.becker@gsi.de>
 * @copyright   2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @date 30.01.2020
 * @see https://www-acc.gsi.de/wiki/Timing/TimingSystemHowSoftCPUHandleECAMSIs
 */
#ifndef _ECA_QUEUE_TYPE_H
#define _ECA_QUEUE_TYPE_H

#include <stdint.h>
#include <stdbool.h>
#ifdef __lm32__
 #include <scu_lm32_macros.h>
#else
 #include <helper_macros.h>
#endif
#include "mini_sdb.h"
#include "eca_queue_regs.h"
#include <eca_flags.h>
#include "eca_regs.h"

/*!
 * @defgroup ECA Event Conditioned Action
 */

#ifdef __cplusplus
extern "C" {
#endif

#define ECA_CHANNEL_FOR_LM32 2

/*!
 * @brief ECA channel for an embedded CPU (LM32),
 *        connected to ECA queue pointed by pECAQ
 *
 * @see eca_queue_regs.h
 */
#define ECA_SELECT_LM32_CHANNEL (ECA_CHANNEL_FOR_LM32 + 1)

/*! ---------------------------------------------------------------------------
 * @ingroup ECA
 * @brief Event Conditioned Action (ECA) hardware control register.
 */
typedef struct HW_IMAGE
{
   /*!
    * @brief Number of channels implemented by the ECA, including the
    *        internal IO channel #0 (8 Bit)
    */
   const uint32_t channels;

   /*!
    * @brief Total number of search table entries per active page
    *        (16 Bit)
    */
   const uint32_t searchCapacity;

   /*!
    * @brief Total number of walker table entries per active page
    *        (16 Bit)
    */
   const uint32_t walkerCapacity;

   /*!
    * @brief Delay in ticks (typically nanoseconds) between an event's
    *        arrival at the ECA and its earliest possible execution as
    *        an action (32 Bit)
    */
   const uint32_t latency;

   /*!
    * @brief Actions scheduled for execution with a delay in ticks exceeding
    *        offset_bits are executed early. (8 Bit)
    */
   const uint32_t offset;

   /*!
    * @brief Flip the active search and walker tables with the inactive tables
    *        (1 Bit)
    */
   uint32_t flipActive;

   /*!
    * @brief Ticks (nanoseconds) since Jan 1, 1970 (high word)
    *        (32 Bit)
    */
   const uint32_t timeHigh;

   /*!
    * @brief Ticks (nanoseconds) since Jan 1, 1970 (low word)
    *        (32 Bit)
    */
   const uint32_t timeLow;

   /*!
    * @brief Read/write this record in the inactive search tables
    *        (16 Bit)
    */
   uint32_t searchSelect;

   /*!
    * @brief Scratch register to be written to search_ro_first
    *        (16 Bit)
    */
   uint32_t searchFirst;

   /*!
    * @brief Scratch register to be written to search_ro_event_hi
    */
   uint32_t searchEventHigh;

   /*!
    * @brief Scratch register to be written to search_ro_event_lo
    */
   uint32_t searchEventLow;

   /*!
    * @brief Store the scratch registers to the inactive search table record
    *        search_select (1 Bit)
    */
   uint32_t searchWrite;

   /*!
    * @brief The first walker entry to execute if an event matches this record
    *        in the search table (16 Bit)
    */
   const uint32_t searchFirstRo;

   /*!
    * @brief Event IDs greater than or equal to this value match this search
    *        table record (high word) (32 Bit)
    */
   const uint32_t searchRoEventHigh;

   /*!
    * @brief Event IDs greater than or equal to this value match this search
    *        table record (low word) (32 Bit)
    */
   const uint32_t searchRoEventLow;

   /*!
    * @brief Read/write this record in the inactive walker tables
    *        (16 Bit)
    */
   uint32_t walkerSelect;

   /*!
    * @brief Scratch register to be written to walker_ro_next
    *        (16 Bit)
    */
   uint32_t walkerRwNext;

   /*!
    * @brief Scratch register to be written to walker_ro_offset_hi
    *        (32 Bit)
    */
   uint32_t walkerRwOffsetHigh;

   /*!
    * @brief Scratch register to be written to walker_ro_offset_lo
    *        (32 Bit )
    */
   uint32_t walkerRwOffsetLow;

   /*!
    * @brief Scratch register to be written to walker_ro_tag
    *        (32 Bit)
    */
   uint32_t walkerRwTag;

   /*!
    * @brief Scratch register to be written to walker_ro_flags
    *        (4 Bit)
    */
   uint32_t walkerRwFlags;

   /*!
    * @brief Scratch register to be written to walker_ro_channel
    *        (8 Bit )
    */
   uint32_t walkerRwChannel;

   /*!
    * @brief Scratch register to be written to walker_ro_num
    *        (8 bit)
    */
   uint32_t walkerRwNum;

   /*!
    * @brief Store the scratch registers to the inactive walker table record
    *        walker_select (1 Bit)
    */
   uint32_t walkerWrite;

   /*!
    * @brief The next walker entry to execute after this record
    *        (0xffff = end of list) (16 Bit)
    */
   const uint32_t walkerRoNext;

   /*!
    * @brief The resulting action's deadline is the event timestamp plus this
    *        offset (high word) (32 Bit)
    */
   const uint32_t walkerRoOffsetHigh;

   /*!
    * @brief The resulting action's deadline is the event timestamp plus this
    *        offset (low word) (32 Bit)
    */
   const uint32_t walkerRoOffsetLow;

   /*!
    * @brief The resulting actions's tag
    *        (32 Bit)
    */
   const uint32_t walkerRoTag;

   /*!
    * @brief Execute the resulting action even if it suffers from the errors
    *        set in this flag register (4 Bit)
    */
   const uint32_t walkerRoFlags;

   /*!
    * @brief The channel to which the resulting action will be sent
    *        (8 Bit)
    */
   const uint32_t walkerRoChannel;

   /*!
    * @brief The subchannel to which the resulting action will be sent
    *        (8 Bit)
    */
   const uint32_t walkerRoNumber;

   /*!
    * @brief Read/clear this channel
    *        (8 Bit)
    */
   uint32_t channelSelect;

   /*!
    * @brief Read/clear this subchannel
    *        (8 Bit)
    */
   uint32_t channelNumberSelect;

   /*!
    * @brief Read/clear this error condition
    *        (0=late, 1=early, 2=conflict, 3=delayed) (2 Bit)
    */
   uint32_t channelCodeSelect;

   /*!
    * @brief Is necessary here to producing the correct offset for the folowing
    *        member variables.
    */
   uint32_t __padding1;

   /*!
    * @brief Type of the selected channel (0=io, 1=linux, 2=wbm, ...)
    *        (32 Bit)
    */
   const uint32_t channelType;

   /*!
    * @brief Total number of subchannels supported by the selected channel
    *        (8 Bit)
    */
   const uint32_t channelMaxNumber;

   /*!
    * @brief Total number of actions which may be enqueued by the selected
    *        channel at a time. (16 Bit)
    */
   const uint32_t channelCapacity;

   /*!
    * @brief Turn on/off MSI messages for the selected channel.
    *        (1 Bit)
    */
   uint32_t channelSetEnable;

   /*!
    * @brief Check if MSI messages are enabled for the selected channel.
    *        (1 Bit)
    */
   const uint32_t channelGetEnable;

   /*!
    * @brief Set the destination MSI address for the selected channel.
    *        (only possible while it has MSIs disabled) (32 Bit)
    */
   uint32_t channelSetTarget;

   /*!
    * @brief Get the destination MSI address for the selected channel.
    *        (32 Bit)
    */
   const uint32_t channelGetTarget;

   /*!
    * @brief Read the selected channel's fill status
    *       (used_now<<16 | used_most), MSI=(6<<16) will be sent if
    *       used_most changes. (32 Bit)
    */
   const uint32_t channelGetAck;

   /*!
    * @brief Read and clear the selected channel's fill status
    *        (used_now<<16 | used_most), MSI=(6<<16) will be sent
    *        if used_most changes (32 Bit)
    */
   const uint32_t channelMostFullClear;

   /*!
    * @brief Read and clear the number of actions output by the selected
    *        subchannel, MSI=(4<<16|num) will be sent when the count becomes
    *        non-zero (32 Bit)
    */
   const uint32_t channelValidCount;

   /*!
    * @brief Read and clear the number of actions which could not be enqueued
    *        to the selected full channel which were destined for the selected
    *        subchannel, MSI=(5<<16|num) will be sent when the count becomes
    *        non-zero. (32 Bit)
    */
   const uint32_t channelOverflowCount;

   /*!
    * @brief Read and clear the number of actions with the selected error code
    *        which were destined for the selected subchannel,
    *        MSI=(code<<16|num) will be sent when the count becomes non-zero.
    *        (32 Bit)
    */
   const uint32_t channelFailedCount;

   /*!
    * @brief The event ID of the first action with the selected error code on
    *        the selected subchannel, cleared when channel_failed_count is read
    *        (high word) (32 Bit)
    */
   const uint32_t channelEventIdHigh;

   /*!
    * @brief The event ID of the first action with the selected error code on
    *        the selected subchannel, cleared when channel_failed_count is read
    *        (low word) (33 Bit)
    */
   const uint32_t channelEventIdLow;

   /*!
    * @brief The parameter of the first action with the selected error code on
    *        the selected subchannel, cleared when channel_failed_count
    *        is read (high word) (32 Bit)
    */
   const uint32_t channelParamHigh;

   /*!
    * @brief The parameter of the first action with the selected error code on
    *        the selected subchannel, cleared when channel_failed_count is
    *        read (low word) (32 Bit)
    */
   const uint32_t channelParamLow;

   /*!
    * @brief The tag of the first action with the selected error code on the
    *        selected subchannel, cleared when channel_failed_count is read.
    *        (32 Bit)
    */
   const uint32_t channelTag;

   /*!
    * @brief The TEF of the first action with the selected error code on the
    *        selected subchannel, cleared when channel_failed_count is read.
    *        (32 Bit)
    */
   const uint32_t channelTef;

   /*!
    * @brief The deadline of the first action with the selected error code on
    *        the selected subchannel, cleared when channel_failed_count is
    *        read (high word) (32 Bit)
    */
   const uint32_t channelDeadlineHigh;

   /*!
    * @brief The deadline of the first action with the selected error code on
    *        the selected subchannel, cleared when channel_failed_count is
    *        read (low word) (32 Bit)
    */
   const uint32_t channelDeadlineLow;

   /*!
    * @brief The actual execution time of the first action with the selected
    *        error code on the selected subchannel, cleared when
    *        channel_failed_count is read (high word) (32 Bit)
    */
   const uint32_t channelExecutedHigh;

   /*!
    * @brief The actual execution time of the first action with the selected
    *        error code on the selected subchannel, cleared when
    *        channel_failed_count is read (low word) (32 Bit)
    */
   const uint32_t channelExecutedLow;
} ECA_CONTROL_T;

#ifndef __DOXYGEN__
/*
 * Un poco de paranoia no duele ;-)
 */
STATIC_ASSERT( offsetof( ECA_CONTROL_T, channels ) == ECA_CHANNELS_GET );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, searchCapacity ) == ECA_SEARCH_CAPACITY_GET );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, walkerCapacity ) == ECA_WALKER_CAPACITY_GET );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, latency ) == ECA_LATENCY_GET );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, offset ) == ECA_OFFSET_BITS_GET );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, flipActive ) == ECA_FLIP_ACTIVE_OWR );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, timeHigh ) == ECA_TIME_HI_GET );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, timeLow ) == ECA_TIME_LO_GET );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, searchSelect ) == ECA_SEARCH_SELECT_RW );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, searchFirst ) == ECA_SEARCH_RW_FIRST_RW );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, searchEventHigh ) == ECA_SEARCH_RW_EVENT_HI_RW );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, searchEventLow ) == ECA_SEARCH_RW_EVENT_LO_RW );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, searchWrite ) == ECA_SEARCH_WRITE_OWR );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, searchFirstRo ) == ECA_SEARCH_RO_FIRST_GET );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, searchRoEventHigh ) == ECA_SEARCH_RO_EVENT_HI_GET );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, searchRoEventLow ) == ECA_SEARCH_RO_EVENT_LO_GET );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, walkerSelect ) == ECA_WALKER_SELECT_RW );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, walkerRwNext ) == ECA_WALKER_RW_NEXT_RW );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, walkerRwOffsetHigh ) == ECA_WALKER_RW_OFFSET_HI_RW );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, walkerRwOffsetLow ) == ECA_WALKER_RW_OFFSET_LO_RW );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, walkerRwTag ) == ECA_WALKER_RW_TAG_RW );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, walkerRwFlags ) == ECA_WALKER_RW_FLAGS_RW );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, walkerRwChannel ) == ECA_WALKER_RW_CHANNEL_RW );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, walkerRwNum ) == ECA_WALKER_RW_NUM_RW );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, walkerWrite ) == ECA_WALKER_WRITE_OWR );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, walkerRoNext ) == ECA_WALKER_RO_NEXT_GET );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, walkerRoOffsetHigh ) == ECA_WALKER_RO_OFFSET_HI_GET );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, walkerRoOffsetLow ) == ECA_WALKER_RO_OFFSET_LO_GET );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, walkerRoTag ) == ECA_WALKER_RO_TAG_GET );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, walkerRoFlags ) == ECA_WALKER_RO_FLAGS_GET );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, walkerRoChannel ) == ECA_WALKER_RO_CHANNEL_GET );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, walkerRoNumber ) == ECA_WALKER_RO_NUM_GET );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, channelSelect ) == ECA_CHANNEL_SELECT_RW );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, channelNumberSelect ) == ECA_CHANNEL_NUM_SELECT_RW );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, channelCodeSelect ) == ECA_CHANNEL_CODE_SELECT_RW );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, channelType ) == ECA_CHANNEL_TYPE_GET );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, channelMaxNumber ) == ECA_CHANNEL_MAX_NUM_GET );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, channelCapacity ) == ECA_CHANNEL_CAPACITY_GET );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, channelSetEnable ) == ECA_CHANNEL_MSI_SET_ENABLE_OWR );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, channelGetEnable ) == ECA_CHANNEL_MSI_GET_ENABLE_GET );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, channelSetTarget ) == ECA_CHANNEL_MSI_SET_TARGET_OWR );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, channelGetTarget ) == ECA_CHANNEL_MSI_GET_TARGET_GET );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, channelGetAck ) == ECA_CHANNEL_MOSTFULL_ACK_GET );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, channelMostFullClear ) == ECA_CHANNEL_MOSTFULL_CLEAR_GET );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, channelValidCount ) == ECA_CHANNEL_VALID_COUNT_GET );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, channelOverflowCount ) == ECA_CHANNEL_OVERFLOW_COUNT_GET );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, channelFailedCount ) == ECA_CHANNEL_FAILED_COUNT_GET );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, channelEventIdHigh ) == ECA_CHANNEL_EVENT_ID_HI_GET );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, channelEventIdLow ) == ECA_CHANNEL_EVENT_ID_LO_GET );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, channelParamHigh ) == ECA_CHANNEL_PARAM_HI_GET );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, channelParamLow ) == ECA_CHANNEL_PARAM_LO_GET );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, channelTag ) == ECA_CHANNEL_TAG_GET );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, channelTef ) == ECA_CHANNEL_TEF_GET );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, channelDeadlineHigh ) == ECA_CHANNEL_DEADLINE_HI_GET );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, channelDeadlineLow ) == ECA_CHANNEL_DEADLINE_LO_GET );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, channelExecutedHigh ) == ECA_CHANNEL_EXECUTED_HI_GET );
STATIC_ASSERT( offsetof( ECA_CONTROL_T, channelExecutedLow ) == ECA_CHANNEL_EXECUTED_LO_GET );
STATIC_ASSERT( sizeof( ECA_CONTROL_T ) == ECA_CHANNEL_EXECUTED_LO_GET + sizeof(uint32_t) );
#endif

#if defined(__lm32__) || defined(__DOXYGEN__)
/*! ---------------------------------------------------------------------------
 * @ingroup ECA
 * @brief  Macro accomplishes a wishbone access to a ECA control register.
 *
 * It has a logical corresponding to:
 * @code
 * p->r
 * @endcode
 *
 * @see ECA_CONTROL_T
 * @see __WB_ACCESS
 * @param p Pointer to the ECA control register set of type ECA_CONTROL_T*.
 * @param r Name of the concerning control register.
 */
#define ECA_CONTROL_ACCESS( p, r ) __WB_ACCESS( ECA_CONTROL_T, uint32_t, p, r )

/*! ---------------------------------------------------------------------------
 * @ingroup ECA
 * @brief Select the ECA channel for LM32
 * @param pThis Pointer to ECA control register set.
 */
STATIC inline void ecaControlSelectLM32Channel( ECA_CONTROL_T* pThis )
{
   ECA_CONTROL_ACCESS( pThis, channelSelect ) = ECA_SELECT_LM32_CHANNEL;
   ECA_CONTROL_ACCESS( pThis, channelNumberSelect ) = 0;
}

/*! ---------------------------------------------------------------------------
 * @ingroup ECA
 * @brief Read and clear the number of actions which could not be enqueued
 *        to the selected full channel which were destined for the selected
 *        subchannel, MSI=(5<<16|num) will be sent when the count becomes
 *        non-zero.
 * @param pThis Pointer to ECA control register set.
 * @return Number of actions before reset.
 */
STATIC inline
uint32_t ecaControlGetAndResetChannelValidCount( ECA_CONTROL_T* pThis )
{
   return ECA_CONTROL_ACCESS( pThis, channelValidCount );
}

/*! ---------------------------------------------------------------------------
 * @ingroup ECA
 * @brief Read and clear the number of actions of LM32 channel
 * @param pThis Pointer to ECA control register set.
 * @return Number of LM32-actions before reset.
 */
STATIC inline
uint32_t ecaControlGetAndResetLM32ValidCount( ECA_CONTROL_T* pThis )
{
   ecaControlSelectLM32Channel( pThis );
   return ecaControlGetAndResetChannelValidCount( pThis );
}

/*! ---------------------------------------------------------------------------
 * @ingroup ECA
 * @brief Set the destination MSI address for the selected channel.
 * @param pThis Pointer to ECA control register set.
 * @param ptr MSI target address.
 * @param enable If true the selected channel is enabled after this function
 *               call.
 */
STATIC inline
void ecaControlSetMsiTargetAddress( ECA_CONTROL_T* pThis, const void* ptr,
                                    const bool enable )
{
   ECA_CONTROL_ACCESS( pThis, channelSetEnable ) = 0;
   ECA_CONTROL_ACCESS( pThis, channelSetTarget ) = (uint32_t) ptr;
   ECA_CONTROL_ACCESS( pThis, channelSetEnable ) = (uint32_t) enable;
}

/*! ---------------------------------------------------------------------------
 * @ingroup ECA
 * @param pThis Pointer to ECA control register set.
 * @param ptr MSI target address for LM32.
 * @param enable If true the LM32- channel is enabled after this function
 *               call.
 */
STATIC inline
void ecaControlSetMsiLM32TargetAddress( ECA_CONTROL_T* pThis, const void* ptr,
                                        const bool enable )
{
   ecaControlSelectLM32Channel( pThis );
   ecaControlSetMsiTargetAddress( pThis, ptr, enable );
}

#endif /* #if defined(__lm32__) || defined(__DOXYGEN__) */

/*! ---------------------------------------------------------------------------
 * @ingroup ECA
 * @brief Data type of Event Conditioned Action queue
 */
typedef struct HW_IMAGE
{
   /*!
    * @brief The index of a_channel_o from the ECA to which this queue is
    *        connected (set channel_select=queue_id+1)
    */
   const uint32_t id;

   /*!
    * @brief Pop action from the channel's queue
    */
   uint32_t pop;

   /*!
    * @brief Error flags for this action
    *       (0=late, 1=early, 2=conflict, 3=delayed, 4=valid)
    */
   const uint32_t flags;

   /*!
    * @brief Subchannel target
    */
   const uint32_t num;

   /*!
    * @brief Event ID (high word)
    */
   const uint32_t eventIdH;

   /*!
    * @brief Event ID (low word)
    */
   const uint32_t eventIdL;

   /*!
    * @brief Parameter (high word)
    */
   const uint32_t paramH;

   /*!
    * @brief Parameter (low word)
    */
   const uint32_t paramL;

   /*!
    * @brief Tag from the condition
    */
   const uint32_t tag;

   /*!
    * @brief Timing extension field
    */
   const uint32_t tef;

   /*!
    * @brief Deadline (high word)
    */
   const uint32_t deadlineH;

   /*!
    * @brief Deadline (low word)
    */
   const uint32_t deadlineL;

   /*!
    * @brief Actual execution time (high word)
    */
   const uint32_t executedH;

   /*!
    * @brief Actual execution time (low word)
    */
   const uint32_t executedL;
} ECA_QUEUE_ITEM_T;

#ifndef __DOXYGEN__
/*
 * Un poco de paranoia no duele ;-)
 */
STATIC_ASSERT( offsetof( ECA_QUEUE_ITEM_T, id )        == ECA_QUEUE_QUEUE_ID_GET );
STATIC_ASSERT( offsetof( ECA_QUEUE_ITEM_T, pop )       == ECA_QUEUE_POP_OWR );
STATIC_ASSERT( offsetof( ECA_QUEUE_ITEM_T, flags )     == ECA_QUEUE_FLAGS_GET );
STATIC_ASSERT( offsetof( ECA_QUEUE_ITEM_T, num )       == ECA_QUEUE_NUM_GET );
STATIC_ASSERT( offsetof( ECA_QUEUE_ITEM_T, eventIdH )  == ECA_QUEUE_EVENT_ID_HI_GET );
STATIC_ASSERT( offsetof( ECA_QUEUE_ITEM_T, eventIdL )  == ECA_QUEUE_EVENT_ID_LO_GET );
STATIC_ASSERT( offsetof( ECA_QUEUE_ITEM_T, paramH )    == ECA_QUEUE_PARAM_HI_GET );
STATIC_ASSERT( offsetof( ECA_QUEUE_ITEM_T, paramL )    == ECA_QUEUE_PARAM_LO_GET );
STATIC_ASSERT( offsetof( ECA_QUEUE_ITEM_T, tag )       == ECA_QUEUE_TAG_GET );
STATIC_ASSERT( offsetof( ECA_QUEUE_ITEM_T, deadlineH ) == ECA_QUEUE_DEADLINE_HI_GET );
STATIC_ASSERT( offsetof( ECA_QUEUE_ITEM_T, deadlineL ) == ECA_QUEUE_DEADLINE_LO_GET );
STATIC_ASSERT( offsetof( ECA_QUEUE_ITEM_T, executedH ) == ECA_QUEUE_EXECUTED_HI_GET );
STATIC_ASSERT( offsetof( ECA_QUEUE_ITEM_T, executedL ) == ECA_QUEUE_EXECUTED_LO_GET );
#endif


#if defined(__lm32__) || defined(__DOXYGEN__)
/*! ---------------------------------------------------------------------------
 * @ingroup ECA
 * @brief Returns the top pointer of the ECA queue
 * @param id ECA ID to find
 * @retval NULL Queue not found
 * @return Pointer on found ECA queue
 */
ECA_QUEUE_ITEM_T* ecaGetQueue( const unsigned int id );

/*! ---------------------------------------------------------------------------
 * @ingroup ECA
 * @brief Clearing the ECA queue.
 * @param pThis Pointer to ECA queue.
 * @param cnt Number pending actions.
 * @return Number of cleared actions.
 */
unsigned int ecaClearQueue( ECA_QUEUE_ITEM_T* pThis, const unsigned int cnt );

/*! ---------------------------------------------------------------------------
 * @ingroup ECA
 * @brief Returns the top pointer of the ECA queue for LM32
 * @retval NULL Queue not found
 * @return Pointer on found ECA queue
 */
STATIC inline ECA_QUEUE_ITEM_T* ecaGetLM32Queue( void )
{
   return ecaGetQueue( ECA_CHANNEL_FOR_LM32 );
}
#endif /* __lm32__ */

/*! --------------------------------------------------------------------------
 * @ingroup ECA
 * @brief Returns true if ECA object valid.
 */
STATIC inline bool ecaIsValid( volatile ECA_QUEUE_ITEM_T* pThis )
{
   return (pThis->flags & (1 << ECA_VALID)) != 0;
}

/*! ---------------------------------------------------------------------------
 * @ingroup ECA
 * @brief Pops the top action from ECA hardware channel
 */
STATIC inline void ecaPop( volatile ECA_QUEUE_ITEM_T* pThis )
{
   pThis->pop = 1;
}

/*! ---------------------------------------------------------------------------
 * @ingroup ECA
 * @brief Testing whether top ECA object is valid to the given tag
 *        and pop it from channel if was valid.
 * @param pThis Pointer to ECA queue
 * @param tag Tag to test.
 * @retval true is valid
 * @retval false is invalid
 */
STATIC inline bool ecaTestTagAndPop( volatile ECA_QUEUE_ITEM_T* pThis,
                                     const uint32_t tag )
{
   if( !ecaIsValid( pThis ) )
      return false;
   if( pThis->tag != tag )
      return false;
   ecaPop( pThis );
   return true;
}

/*! ---------------------------------------------------------------------------
 * @ingroup ECA
 * @brief Returns the pointer to the hardware ECA control registers.
 * @see ECA_CONTROL_T
 * @retval !=NULL Valid pointer.
 * @retval ==NULL Error.
 */
STATIC inline
ECA_CONTROL_T* ecaControlGetRegisters( void )
{
   ECA_CONTROL_T* ret = (ECA_CONTROL_T*) find_device_adr( ECA_SDB_VENDOR_ID,
                                                          ECA_SDB_DEVICE_ID );
   if( ret == (ECA_CONTROL_T*) ERROR_NOT_FOUND )
      return NULL;
   return ret;
}

#ifdef __cplusplus
}
#endif
#endif /* _ECA_QUEUE_TYPE_H */
/*================================== EOF ====================================*/
