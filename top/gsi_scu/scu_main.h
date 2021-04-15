/*!
 *  @file scu_main.h
 *  @brief Main module of SCU function generators in LM32.
 *
 *  @date 31.01.2020
 *  @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *
 *  For testing:
 *  @see https://www-acc.gsi.de/wiki/Hardware/Intern/Saft_Fg_Ctl
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
#ifndef _SCU_MAIN_H
#define _SCU_MAIN_H

#if !defined(__lm32__) && !defined(__DOXYGEN__) && !defined(__DOCFSM__)
  #error This module is for the target LM32 only!
#endif
#ifndef MICO32_FULL_CONTEXT_SAVE_RESTORE
  #warning Macro MICO32_FULL_CONTEXT_SAVE_RESTORE is not defined in Makefile!
#endif

#include <stdint.h>
#include "syscon.h"
#include "eb_console_helper.h"

#include "scu_wr_time.h"

#ifdef _CONFIG_OLD_IRQ
 #include "scu_lm32_macros.h"
 #include "irq.h"
#else
 #include "scu_msi.h"
#endif
#include "scu_bus.h"
#include "mini_sdb.h"
#include "board.h"
#include "uart.h"
#include "w1.h"
#include "scu_shared_mem.h"
#include "scu_fg_list.h"
#include "scu_mil.h"
#include "eca_queue_type.h"
#include "history.h"
#include "scu_circular_buffer.h"
#include "event_measurement.h"

/*!
 * @defgroup MIL_FSM Functions and macros which concerns the MIL-FSM
 */

/*!
 * @defgroup TASK Cooperative multitasking entry functions invoked by the scheduler.
 */

#ifdef __cplusplus
extern "C" {
#endif


#define INTERVAL_1000MS 1000000000ULL
#define INTERVAL_2000MS 2000000000ULL
#define INTERVAL_100MS  100000000ULL
#define INTERVAL_84MS   84000000ULL
#define INTERVAL_10MS   10000000ULL
#define INTERVAL_1MS    1000000ULL
#define INTERVAL_200US  200000ULL
#define INTERVAL_150US  150000ULL
#define INTERVAL_100US  100000ULL
#define INTERVAL_10US   10000ULL
#define INTERVAL_5MS    5000000ULL
#define ALWAYS          0ULL

#define CLK_PERIOD (1000000 / USRCPUCLK) // USRCPUCLK in KHz
//#define OFFS(SLOT) ((SLOT) * (1 << 16))

/*!
 * @see scu_shared_mem.h
 */
extern SCU_SHARED_DATA_T g_shared;

/*!
 * @brief Number of message queues.
 */
#ifdef CONFIG_SCU_DAQ_INTEGRATION
  #define QUEUE_CNT 4
#else
  #define QUEUE_CNT 3
#endif
/*!
 * @brief Type of message origin
 */
typedef enum
{
   DEVBUS = 0, /*!<@brief From MIL-device.            */
   DEVSIO = 1, /*!<@brief From MIL-device via SCU-bus */
   SWI    = 2  /*!<@brief From Linux host             */
#ifdef CONFIG_SCU_DAQ_INTEGRATION
   ,DAQ   = 3  /*!<@brief From ADDAC-DAQ              */
#endif
} MSG_ORIGIN_T;

/*!
 * @brief Reset counter becomes incremented after each LM32-Reset.
 *
 * This global variable is implemented and initialized in the assembler module
 * crt0ScuLm32.S \n
 * This becomes incremented in the startup-routine _crt0.
 *
 * @see crt0ScuLm32.S
 */
extern volatile uint32_t __reset_count;

extern volatile FG_MESSAGE_BUFFER_T g_aMsg_buf[QUEUE_CNT];

#ifdef CONFIG_DBG_MEASURE_IRQ_TIME
/*!
 * @brief Holding the time between the last two happened interrupts.
 * @note For debugging purposes only!
 */
extern TIME_MEASUREMENT_T g_irqTimeMeasurement;
#endif


/*!
 * @todo find the related definitions in the source code of SAFTLIB and
 *       replace it by a common header file!
 */
#define ADDR_SCUBUS 0x00
#define ADDR_SWI    0x10
#ifdef CONFIG_MIL_FG
#define ADDR_DEVBUS 0x20
#endif

/*! ---------------------------------------------------------------------------
 * @ingroup TASK
 * @brief Declaration of the task type
 */
typedef struct _TASK_T
{
   const void*    pTaskData;  /*!<@brief Pointer to the memory-space of the current task */
   const uint64_t interval;   /*!<@brief interval of the task */
   uint64_t       lasttick;   /*!<@brief when was the task ran last */
   void (*func)(struct _TASK_T*); /*!<@brief pointer to the function of the task */
} TASK_T;


/*! ---------------------------------------------------------------------------
 * @brief Scans for function generators on mil extension and scu bus.
 */
void scanFgs( void );


//#define CONFIG_DEBUG_FG_SIGNAL
/*! ---------------------------------------------------------------------------
 * @ingroup MAILBOX
 * @brief Send a signal back to the Linux-host (SAFTLIB)
 * @param sig Signal
 * @param channel Concerning channel number.
 */
STATIC inline void sendSignal( const SIGNAL_T sig, const unsigned int channel )
{
   STATIC_ASSERT( sizeof( pCpuMsiBox[0] ) == sizeof( uint32_t ) );
   FG_ASSERT( channel < ARRAY_SIZE( g_shared.fg_regs ) );

   ATOMIC_SECTION()
      MSI_BOX_SLOT_ACCESS( g_shared.fg_regs[channel].mbx_slot, signal ) = sig;

#ifdef CONFIG_LOG_ALL_SIGNALS
   hist_addx( HISTORY_XYZ_MODULE, signal2String( sig ), channel );
#endif

#ifdef CONFIG_DEBUG_FG_SIGNAL
   #warning CONFIG_DEBUG_FG_SIGNAL is defined this will destroy the timing!
   mprintf( ESC_DEBUG "Signal: %s, channel: %d sent\n" ESC_NORMAL,
            signal2String( sig ), channel );
#endif
}

#ifdef __cplusplus
}
#endif
#endif /* _SCU_MAIN_H */
/* ================================= EOF ====================================*/

