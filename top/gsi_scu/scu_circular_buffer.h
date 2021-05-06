/*!
 *  @file scu_circular_buffer.h
 *  @brief SCU- circular buffer resp. ring buffer administration in
 *         shared memory of LM32.
 *
 *  @date 21.10.2019
 *  @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Stefan Rauch perhaps...
 *  @revision Ulrich Becker <u.becker@gsi.de>
 *
 ******************************************************************************
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */
#ifndef _SCU_CIRCULAR_BUFFER_H
#define _SCU_CIRCULAR_BUFFER_H

#include <scu_function_generator.h>
#include <stdbool.h>
#if defined(__lm32__)
  #include <lm32Interrupts.h>
#endif

#ifdef __cplusplus
extern "C" {
namespace Scu
{
#endif

typedef uint32_t RING_POS_T;

#ifdef __cplusplus
namespace FG
{
#endif

#define DAQ_RING_SIZE  2048
//#define DAQ_RING_SIZE 512
//#define DAQ_RING_SIZE  1024

#if defined(__lm32__) || defined(__DOXYGEN__)

/*! --------------------------------------------------------------------------
 * @brief check if a channel buffer is empty
 * @param cr channel register
 * @param channel number of the channel
 * @retval true Buffer is empty.
 * @retval false Buffer is not empty.
 */
STATIC inline
bool cbisEmpty(volatile FG_CHANNEL_REG_T* cr, const unsigned int channel)
{
   return cr[channel].wr_ptr == cr[channel].rd_ptr;
}

/*! ---------------------------------------------------------------------------
 * @brief get the fill level  of a channel buffer
 * @param cr channel register
 * @param channel number of the channel
 * @return Number of items of type FG_CHANNEL_REG_T
 */
STATIC inline
RING_POS_T cbgetCount(volatile FG_CHANNEL_REG_T* cr, const unsigned int channel )
{
   if( cr[channel].wr_ptr > cr[channel].rd_ptr )
      return cr[channel].wr_ptr - cr[channel].rd_ptr;

   if( cr[channel].rd_ptr > cr[channel].wr_ptr )
      return BUFFER_SIZE - cr[channel].rd_ptr + cr[channel].wr_ptr;

   return 0;
}

/*! ------------------------------------------------------------------------
 * @brief Thread save version of cbgetCoun
 * @see cbgetCount
 */
STATIC inline
RING_POS_T cbgetCountSave( volatile FG_CHANNEL_REG_T* pCr, const unsigned int channel )
{
   criticalSectionEnter();
   const RING_POS_T ret = cbgetCount( pCr, channel );
   criticalSectionExit();
   return ret;
}

/*! -------------------------------------------------------------------------
 * @brief check if a channel buffer is full
 * @param cr channel register
 * @param channel number of the channel
 * @retval true Buffer is full.
 * @retval false Buffer is not full.
 */
STATIC inline
bool cbisFull(volatile FG_CHANNEL_REG_T* cr, const unsigned int channel)
{
   return (cr[channel].wr_ptr + 1) % (BUFFER_SIZE) == cr[channel].rd_ptr;
}

/*! ---------------------------------------------------------------------------
 * @brief read a parameter set from a channel buffer
 * @param pCb pointer to the first channel buffer
 * @param pCr pointer to the first channel register
 * @param channel number of the channel
 * @param pPset the data from the buffer is written to this address
 * @retval false Buffer is empty no data read.
 * @retval true Date successful read.
 */
STATIC inline
bool cbRead( volatile FG_CHANNEL_BUFFER_T* pCb, volatile FG_CHANNEL_REG_T* pCr,
            const unsigned int channel, FG_PARAM_SET_T* pPset )
{
   const uint32_t rptr = pCr[channel].rd_ptr;

   /* check empty */
   if( pCr[channel].wr_ptr == rptr )
      return false;

  /* read element */
#ifdef __cplusplus
   //TODO Workaround, I don't know why yet!
   *pPset = *((FG_PARAM_SET_T*) &(pCb[channel].pset[rptr]));
#else
   *pPset = pCb[channel].pset[rptr];
#endif
   /* move read pointer forward */
   pCr[channel].rd_ptr = (rptr + 1) % (BUFFER_SIZE);
   return true;
}

/*! ---------------------------------------------------------------------------
 * @brief Thread save version of cbRead
 * @see cbRead
 */
STATIC inline
bool cbReadSave( volatile FG_CHANNEL_BUFFER_T* pCb,
                 volatile FG_CHANNEL_REG_T* pCr,
                 const unsigned int channel, FG_PARAM_SET_T* pPset )
{
   criticalSectionEnter();
   const bool ret = cbRead( pCb, pCr, channel, pPset );
   criticalSectionExit();
   return ret;
}

#endif /* ifdef __lm32__ */

#ifdef __cplusplus
} /* namespace FG */
namespace MiLdaq
{
#endif

#ifndef CONFIG_MIL_DAQ_USE_RAM

/*!
 * @brief Definition if MIL_DAQ measurement unit.
 */
typedef struct PACKED_SIZE
{
   uint32_t   setvalue; /*!<@brief set value */
   uint32_t   actvalue; /*!<@brief actual value */
   uint32_t   tmstmp_l; /*!<@brief WR-timestamp [31:0] */
   uint32_t   tmstmp_h; /*!<@brief WR-timestamp [63:32] */
   FG_MACRO_T fgMacro;  /*!<@brief Channel info. @see FG_MACRO_T */
} MIL_DAQ_OBJ_T;

/*!
 * @brief Definition of ring buffer object type for the
 *        MIL-DAQ ringbuffer.
 */
typedef struct PACKED_SIZE
{
   RING_POS_T    ring_head; /*!<@brief Write index */
   RING_POS_T    ring_tail; /*!<@brief Read index */
   MIL_DAQ_OBJ_T ring_data[DAQ_RING_SIZE]; /*!<@brief MIL-DAQ data buffer */
} MIL_DAQ_BUFFER_T;

#if defined(__lm32__) || defined(__DOXYGEN__)

/*! --------------------------------------------------------------------------
 * @brief Adds a MIL-DAQ data item of type MIL_DAQ_OBJ_T
 *        in the ring buffer.
 */
void add_daq_msg(volatile MIL_DAQ_BUFFER_T* db, MIL_DAQ_OBJ_T d );

/*! --------------------------------------------------------------------------
 * @brief Returns true when the MIL-DAQ-buffer is full.
 */
STATIC inline bool isMilDaqBufferFull( const MIL_DAQ_BUFFER_T* pDaqBuffer )
{
   return (pDaqBuffer->ring_tail + 1) % DAQ_RING_SIZE == pDaqBuffer->ring_head;
}

/*! --------------------------------------------------------------------------
 * @brief Removes the oldest item in the MIL-DAQ ring-buffer.
 */
STATIC inline void removeOldestItem( MIL_DAQ_BUFFER_T* pDaqBuffer )
{
   pDaqBuffer->ring_tail++;
   pDaqBuffer->ring_tail %= DAQ_RING_SIZE;
}

#endif /* ifdef __lm32__ */
#endif /* ifndef CONFIG_MIL_DAQ_USE_RAM */

#ifdef __cplusplus
} /* namespace MiLdaq */
} /* namespace Scu */
} /* extern "C" */
#endif
#endif /* ifndef _SCU_CIRCULAR_BUFFER_H */
/*================================== EOF ====================================*/
