/*!
 * @file eca_queue_type.h
 * @brief Definition ECA register object type for Wishbone interface of VHDL entity
 * @author Ulrich Becker <u.becker@gsi.de>
 * @copyright   2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @date 30.01.2020
 * @see https://www-acc.gsi.de/wiki/Timing/TimingSystemHowSoftCPUHandleECAMSIs
 */
#ifndef _ECA_QUEUE_TYPE_H
#define _ECA_QUEUE_TYPE_H

#include <stdint.h>
#include <stdbool.h>
#include <helper_macros.h>
#include "eca_queue_regs.h"
#include "eca_flags.h"
#include "eca_regs.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
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

#define ECA_CHANNEL_FOR_LM32 2

#if defined(__lm32__) || defined(__DOXYGEN__)
/*! ---------------------------------------------------------------------------
 * @brief Returns the top pointer of the ECA queue
 * @param id ECA ID to find
 * @retval NULL Queue not found
 * @return Pointer on found ECA queue
 */
ECA_QUEUE_ITEM_T* ecaGetQueue( const unsigned int id );

/*! ---------------------------------------------------------------------------
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
 * @brief Returns true if ECA object valid.
 */
STATIC inline bool ecaIsValid( volatile ECA_QUEUE_ITEM_T* pThis )
{
   return (pThis->flags & (1 << ECA_VALID)) != 0;
}

/*! ---------------------------------------------------------------------------
 * @brief Pops the top action from ECA hardware channel
 */
STATIC inline void ecaPop( volatile ECA_QUEUE_ITEM_T* pThis )
{
   pThis->pop = 1;
}

/*! ---------------------------------------------------------------------------
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

#ifdef __cplusplus
}
#endif
#endif /* _ECA_QUEUE_TYPE_H */
/*================================== EOF ====================================*/
