/*!
 *  @file daq_main.h
 *  @brief Main module for daq_control (including main())
 *
 *  @date 27.02.2019
 *  @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
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
#ifndef _DAQ_MAIN_H
#define _DAQ_MAIN_H
#ifndef __lm32__
  #error Module is for target Lattice Micro 32 (LM32) only!
#endif

#if defined( CONFIG_SCU_DAQ_INTEGRATION ) && defined( CONFIG_DAQ_SINGLE_APP )
 #error Either CONFIG_SCU_DAQ_INTEGRATION or CONFIG_DAQ_SINGLE_APP !
#endif
#if !defined( CONFIG_SCU_DAQ_INTEGRATION ) && !defined( CONFIG_DAQ_SINGLE_APP )
 #error Nither CONFIG_SCU_DAQ_INTEGRATION nor CONFIG_DAQ_SINGLE_APP defined!
#endif

#include <daq.h>
#include <daq_ramBuffer.h>

#ifndef CONFIG_DAQ_SINGLE_APP
 #include <scu_main.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CONFIG_DAQ_SINGLE_APP

/*!
 * @ingroup DAQ
 * @brief Payload data type for the message queue g_queueAddacDaq. 
 */
typedef uint32_t DAQ_QUEUE_SLOT_T;

/*!
 * @ingroup DAQ
 * @brief Message queue for ADDAC/ACU DAQs
 */
extern SW_QUEUE_T g_queueAddacDaq;

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ
 * @ingroup TASK
 * @brief Handles all detected ADDAC-DAQs
 * @see schedule
 */
void addacDaqTask( void );

#endif /* ifndef CONFIG_DAQ_SINGLE_APP */

/*! ---------------------------------------------------------------------------
 * @brief Type of data-object for administrating all non-MIL DAQs
 */
typedef struct
{
   DAQ_BUS_T oDaqDevs;
   RAM_SCU_T oRam;
   volatile bool isIrq;
} DAQ_ADMIN_T;

/*! ---------------------------------------------------------------------------
 * @brief Initializes the memory (at now the DDR3-RAM) for the received
 *        non-MIL-DAQ data.
 */
int initBuffer( RAM_SCU_T* poRam );

/*! ---------------------------------------------------------------------------
 * @brief Scanning the SCU bus for non-MIL-DAQ slaves.
 */
int daqScanScuBus( DAQ_BUS_T* pDaqDevices
                #ifndef CONFIG_DAQ_SINGLE_APP
                  ,FG_MACRO_T* pFgList
                #endif
                 );

/*! ---------------------------------------------------------------------------
 * @brief Finding and initializing of all non-MIL DAQs.
 * @param pDaqAdmin Pointer to the nom-MIL-DAQ administrating object
 */
STATIC // Doxygen 1.8.5 seems to have a problem...
inline void scuDaqInitialize( DAQ_ADMIN_T* pDaqAdmin
                            #ifndef CONFIG_DAQ_SINGLE_APP
                             ,FG_MACRO_T* pFgList
                            #endif
                           )

{
#ifdef CONFIG_DAQ_SINGLE_APP
   daqScanScuBus( &pDaqAdmin->oDaqDevs );
#else
   daqScanScuBus( &pDaqAdmin->oDaqDevs, pFgList );
#endif
   initBuffer( &pDaqAdmin->oRam );
}

#ifndef CONFIG_DAQ_SINGLE_APP

/*! ---------------------------------------------------------------------------
 * @brief Executes one DAQ channel for each call of this function if requested.
 *
 */
void daqExeNextDevice( void );

/*! ---------------------------------------------------------------------------
 * @brief Enables the feedback-channel for the given function generator number
 *        in the given slot.
 *
 * Counterpart to daqDisableFgFeedBack
 * @see daqDisableFgFeedBack
 * @param slot SCU-bus slot number
 * @param fgNum Function generator number 0 or 1
 */
void daqEnableFgFeedback( const unsigned int slot, const unsigned int fgNum );

/*! ---------------------------------------------------------------------------
 * @brief Disables the feedback-channel for the given function generator number
 *        in the given slot.
 *
 * Counterpart to daqEnableFgFeedback
 * @see daqEnableFgFeedback
 * @param slot SCU-bus slot number
 * @param fgNum Function generator number 0 or 1
 */
void daqDisableFgFeedback( const unsigned int slot, const unsigned int fgNum );

extern DAQ_ADMIN_T g_scuDaqAdmin;
#endif

#ifdef __cplusplus
}
#endif

#endif /* ifndef _DAQ_MAIN_H */
/*================================== EOF ====================================*/
