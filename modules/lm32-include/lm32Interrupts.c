/*!
 * @file   lm32Interrupts.c
 * @brief  General administration of the interrupt handling and
 *         critical resp. atomic sections for LM32
 *
 * @note This module is suitable for FreeRTOS-port.
 *
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      21.01.2020
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
#include "lm32Interrupts.h"
#ifdef CONFIG_USE_INTERRUPT_TIMESTAMP
 #include <scu_wr_time.h>
#endif

/*!
 * @ingroup INTERRUPT
 * @brief Nesting counter for critical sections.
 * @note The nesting counter becomes pre-initialized with 1 so it becomes
 *       possible to use critical sections before the global interrupt
 *       is enabled. \n
 *       The function irqEnable() for non FreeRTOS applications or
 *       the FreeRTOS function vTaskStartScheduler() respectively
 *       the port-function xPortStartScheduler() will reset this counter
 *       to zero.
 */
volatile uint32_t mg_criticalSectionNestingCount = 1;

#ifdef CONFIG_USE_INTERRUPT_TIMESTAMP
/*!
 * @ingroup INTERRUPT
 * @brief White rabbit time stamp of the last occurred interrupt.
 */
volatile uint64_t mg_interruptTimestamp = 0LL;
#endif

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief ISR entry type
 */
typedef struct
{
   /*!
    * @brief Function pointer of the interrupt service routine of
    *        the concerning interrupt number.
    */
   ISRCallback pfCallback;

   /*!
    * @brief User tunnel: second parameter of the interrupt service routine.
    */
   void*       pContext;
} ISR_ENTRY_T;

/*!
 * @ingroup INTERRUPT
 * @brief  ISREntry table
 *
 * The maximum number of items of the table depends on the macro
 * MAX_LM32_INTERRUPTS which can be overwritten by the makefile.
 * The default and maximum value of MAX_LM32_INTERRUPTS is 32.
 *
 * @see MAX_LM32_INTERRUPTS
 */
STATIC ISR_ENTRY_T ISREntryTable[MAX_LM32_INTERRUPTS] = {{NULL, NULL}};

/*! ---------------------------------------------------------------------------
 * @see lm32Interrupts.h
 */
inline unsigned int irqGetAtomicNestingCount( void )
{
   return mg_criticalSectionNestingCount;
}

#ifndef CONFIG_RTOS
/*! ---------------------------------------------------------------------------
 * @see lm32Interrupts.h
 */
inline void irqEnable( void )
{
   mg_criticalSectionNestingCount = 0;
   _irqEnable();
}
#endif

/*! ---------------------------------------------------------------------------
 * @see lm32Interrupts.h
 */
OVERRIDE uint32_t _irqGetPendingMask( const unsigned int intNum )
{
   return (1 << intNum);
}

/*! ---------------------------------------------------------------------------
 * @see lm32Interrupts.h
 */
OVERRIDE unsigned int _irqReorderPriority( const unsigned int prio )
{
   return prio;
}

#ifdef CONFIG_USE_INTERRUPT_TIMESTAMP
/*! ---------------------------------------------------------------------------
 * @see lm32Interrupts.h
 */
uint64_t irqGetTimestamp( void )
{
   criticalSectionEnter();
   const uint64_t timestamp = mg_interruptTimestamp;
   criticalSectionExit();
   return timestamp;
}
#endif /* ifdef CONFIG_USE_INTERRUPT_TIMESTAMP */

#if defined( CONFIG_RTOS ) && !defined( CONFIG_IRQ_ENABLING_IN_ATOMIC_SECTIONS )
  //#define CONFIG_IRQ_ENABLING_IN_ATOMIC_SECTIONS
#endif

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief General Interrupt Handler (invoked by low-level routine in portasm.S)
 *
 * If an interrupt-handler exists for the relevant interrupt (as detected
 * from "ip" and "im" cpu registers), then invoke the handler else disable the
 * interrupt in the register "im".
 *
 * @todo Enhance via compiler-switch to the ability of nested interrupt
 *       handling. E.g.: CONFIG_NESTED_IRQS
 */
void _irq_entry( void )
{
#ifdef CONFIG_USE_INTERRUPT_TIMESTAMP
   mg_interruptTimestamp = getWrSysTime();
#endif

   /*
    * Allows using of atomic sections within interrupt context.
    */
#ifdef CONFIG_IRQ_ENABLING_IN_ATOMIC_SECTIONS
   mg_criticalSectionNestingCount++;
 #ifdef CONFIG_INTERRUPT_PEDANTIC_CHECK
   const volatile uint32_t tempNestingCount = mg_criticalSectionNestingCount;
 #endif
#else
   IRQ_ASSERT( mg_criticalSectionNestingCount == 0 );
   mg_criticalSectionNestingCount = 1;
#endif

   /*!
    * @brief Copy of the interrupt pending register before reset.
    */
   uint32_t ip;

   /*
    * As long as there is an interrupt pending...
    */
#ifdef CONFIG_IRQ_RESET_IP_AFTER
   while( (ip = irqGetPendingRegister()) != 0 )
#else
   while( (ip = irqGetAndResetPendingRegister()) != 0 )
   //while( (ip = irqGetAndResetPendingRegister() & irqGetMaskRegister()) != 0 )
#endif
   { /*
      * Zero has the highest priority.
      */
      for( unsigned int prio = 0; prio < ARRAY_SIZE( ISREntryTable ); prio++ )
      {
         const unsigned int intNum = _irqReorderPriority( prio );
         IRQ_ASSERT( intNum < ARRAY_SIZE( ISREntryTable ) );
         const uint32_t mask = _irqGetPendingMask( intNum );

         if( (mask & ip) == 0 ) /* Is this interrupt pending? */
            continue; /* No, go to next possible interrupt. */

         /*
          * Handling of detected pending interrupt.
          */
         const ISR_ENTRY_T* pCurrentInt = &ISREntryTable[intNum];
         if( pCurrentInt->pfCallback != NULL )
         { /*
            * Invoking the callback function of the current handled
            * interrupt.
            */
            pCurrentInt->pfCallback( intNum, pCurrentInt->pContext );
         }
         else
         { /*
            * Ridding of unregistered interrupt so it doesn't bothering
            * any more...
            */
            irqSetMaskRegister( irqGetMaskRegister() & ~mask );
         }
      #ifdef CONFIG_IRQ_RESET_IP_AFTER
         irqResetPendingRegister( mask );
      #endif
      }
   }

   /*
    * Allows using of atomic sections within interrupt context.
    */
#ifdef CONFIG_IRQ_ENABLING_IN_ATOMIC_SECTIONS
   IRQ_ASSERT( mg_criticalSectionNestingCount == tempNestingCount );
   mg_criticalSectionNestingCount--;
#else
   IRQ_ASSERT( mg_criticalSectionNestingCount == 1 );
   mg_criticalSectionNestingCount = 0;
#endif
}

/*! ---------------------------------------------------------------------------
 * @see lm32Interrupts.h
 *
 * When the compiler and linker has LTO ability so the following inline
 * declarations will be really inline.
 */
inline void criticalSectionEnter( void )
{
   irqDisable();
   mg_criticalSectionNestingCount++;
}

/*! ---------------------------------------------------------------------------
 * @see lm32Interrupts.h
 *
 * When the compiler and linker has LTO ability so the following inline
 * declarations will be really inline.
 */
inline void criticalSectionExit( void )
{
   IRQ_ASSERT( mg_criticalSectionNestingCount != 0 );
   mg_criticalSectionNestingCount--;
   if( mg_criticalSectionNestingCount == 0 )
      _irqEnable();
}

/*! ---------------------------------------------------------------------------
 * @see lm32Interrupts.h
 */
void irqRegisterISR( const unsigned int intNum, void* pContext,
                     ISRCallback pfCallback )
{
   IRQ_ASSERT( intNum < ARRAY_SIZE( ISREntryTable ) );

   criticalSectionEnter();

   ISREntryTable[intNum].pfCallback = pfCallback;
   ISREntryTable[intNum].pContext   = pContext;

   const uint32_t mask = _irqGetPendingMask( intNum );
   const uint32_t im = irqGetMaskRegister();
   irqSetMaskRegister( (pfCallback == NULL)? (im & ~mask) : (im | mask) );

   criticalSectionExit();
}

/*! ---------------------------------------------------------------------------
 * @see lm32Interrupts.h
 */
void irqDisableSpecific( const unsigned int intNum )
{
   IRQ_ASSERT( intNum < ARRAY_SIZE( ISREntryTable ) );

   criticalSectionEnter();
   irqSetMaskRegister( irqGetMaskRegister() & ~_irqGetPendingMask( intNum ) );
   criticalSectionExit();
}

/*! ---------------------------------------------------------------------------
 * @see lm32Interrupts.h
 */
void irqEnableSpecific( const unsigned int intNum )
{
   IRQ_ASSERT( intNum < ARRAY_SIZE( ISREntryTable ) );

   criticalSectionEnter();
   irqSetMaskRegister( irqGetMaskRegister() | _irqGetPendingMask( intNum ) );
   criticalSectionExit();
}

/*================================== EOF ====================================*/
