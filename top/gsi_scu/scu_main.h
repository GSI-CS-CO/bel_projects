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
#include <scu_syslog.h>

#include "scu_circular_buffer.h"
#include "event_measurement.h"

#ifdef CONFIG_USE_MMU
 #include <scu_mmu_lm32.h>
 #include <scu_mmu_tag.h>
#endif

#include <sw_queue.h>

/*!
 * @defgroup MIL_FSM Functions and macros which concerns the MIL-FSM
 */

/*!
 * @defgroup TASK Cooperative multitasking entry functions invoked by the scheduler.
 */

#ifdef __cplusplus
extern "C" {
#endif

//#define    CONFIG_TRACE_MIL_DRQ

#ifdef CONFIG_TRACE_MIL_DRQ
   #define TRACE_MIL_DRQ( arg... ) mprintf( arg );
   #warning Macro TRACE_MIL_DRQ is active!
#else
   #define TRACE_MIL_DRQ( arg... ) ((void)0)
#endif

#define INTERVAL_1000MS 1000000000ULL
#define INTERVAL_2000MS 2000000000ULL
#define INTERVAL_100MS  100000000ULL
#define INTERVAL_84MS   84000000ULL
#define INTERVAL_10MS   10000000ULL
#define INTERVAL_5MS    5000000ULL
#define INTERVAL_1MS    1000000ULL
#define INTERVAL_200US  200000ULL
#define INTERVAL_150US  150000ULL
#define INTERVAL_100US  100000ULL
#define INTERVAL_10US   10000ULL

#define ALWAYS          0ULL

#define CLK_PERIOD (1000000 / USRCPUCLK) // USRCPUCLK in KHz

/*!
 * @see scu_shared_mem.h
 */
extern SCU_SHARED_DATA_T g_shared;


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
 * @ingroup LM32_LOG
 * @ingroup PRINTF
 * @brief Sends a log-message via vLm32Log() and vprintf().
 * @note This function is only during the initialization allowed! 
 */
void scuLog( const unsigned int filter, const char* format, ... );

#define CONFIG_QUEUE_ALARM

#ifdef CONFIG_QUEUE_ALARM
/*! ---------------------------------------------------------------------------
 * @brief Put a message in the given queue object.
 * 
 * If the concerned queue is full, then a alarm-item will put in the 
 * alarm-queue which becomes evaluated in the function queuePollAlarm().
 *
 * @see queuePollAlarm.
 * @param pThis Pointer to the queue object.
 * @param pItem Pointer to the payload object.
 */
void pushInQueue( SW_QUEUE_T* pThis, const void* pItem );

#else
#define pushInQueue queuePush
#endif

/*! ---------------------------------------------------------------------------
 * @brief Prints a error-message via UART and stop the LM32 firmware.
 * @param pErrorMessage String containing a error message.
 */
void die( const char* pErrorMessage );

/*! ---------------------------------------------------------------------------
 * @brief Scans for function generators on mil extension and scu bus.
 */
void scanFgs( void );

#ifdef __cplusplus
}
#endif
#endif /* _SCU_MAIN_H */
/* ================================= EOF ====================================*/

