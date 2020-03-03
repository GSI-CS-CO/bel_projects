/*!
 * @file   lm32Interrupts.c
 * @brief  Administration of the interrupt callback functions for LM32
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

/*!
 * @ingroup INTERRUPT
 * @brief Nesting counter for critical sections.
 */
volatile static unsigned int mg_criticalSectionNestingCount = 0;

/*!
 * @ingroup INTERRUPT
 * @brief ISR entry type
 */
typedef struct
{
   ISRCallback Callback;
   void*       pContext;
} ISR_ENTRY_T;

/*!
 * @ingroup INTERRUPT
 * @brief  ISREntry table
 */
static ISR_ENTRY_T ISREntryTable[MAX_LM32_INTERRUPTS] = {{NULL, NULL}};

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT OVERWRITABLE
 * @brief Returns the interrupt flag mask calculated by the given
 *        interrupt number
 * @note It's possible to overwrite this function for the case
 *       that the standard interrupt input line will changed.
 * @param intNum Interrupt number.
 * @return Interrupt pending mask.
 */
__attribute__((weak))
uint32_t _getInterruptPendingMask( const unsigned int intNum )
{
   return (1 << intNum);
}

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT OVERWRITABLE
 * @brief Reordering the interrupt priority.
 *
 * By default the interrupt number is equal to the interrupt priority.
 * @note It's possible to overwrite this function for the case
 *       that the interrupt number isn't equal to the interrupt priority.
 * @param prio Interrupt priority.
 * @return Interrupt number.
 */
__attribute__((weak))
unsigned int _isrReorder( const unsigned int prio )
{
   return prio;
}

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Returns the current value of the LM32 interrupt mask register
 * @return Current value of the interrupt mask register.
 */
STATIC inline
uint32_t irqGetMaskRegister( void )
{
   uint32_t im;
   asm volatile ( "rcsr %0, im" :"=r"(im) );
   return im;
}

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Sets the interrupt mask register by the given value.
 * @param im Value to set.
 */
STATIC inline
void irqSetMaskRegister( const uint32_t im )
{
   asm volatile ( "wcsr im, %0" ::"r"(im) );
}

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Returns the current value of the LM32 interrupt pending register.
 * @return Value of the interrupt pending register.
 */
STATIC inline
uint32_t irqGetPendingRegister( void )
{
   uint32_t ip;
   asm volatile ( "rcsr %0, ip" :"=r"(ip) );
   return ip;
}

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Resets the bits in the LM32 interrupt pending register.
 * @param ip Bit mast to reset the corresponding pending bit.
 * @note The clearing of the bits in the pending register will accomplished
 *       by writing a one in the concerning bit-position!
 */
STATIC inline
void irqResetPendingRegister( const uint32_t ip )
{
   asm volatile ( "wcsr ip, %0" ::"r"(ip) );
}

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief General Interrupt Handler (invoked by low-level routine in portasm.S)
 *
 * If an interrupt-handler exists for the relevant interrupt (as detected
 * from "ip" and "im" cpu registers), then invoke the handler else disable the
 * interrupt in the register "im".
 */
void _irq_entry( void )
{
   const uint32_t im = irqGetMaskRegister();
   while( true )
   {
      const uint32_t ip = irqGetPendingRegister() & im;
      if( ip == 0 ) /* No interrupt pending? */
         return; /* Yes, life the function */

      for( unsigned int prio = 0; prio < ARRAY_SIZE( ISREntryTable ); prio++ )
      {
         const unsigned int intNum = _isrReorder( prio );
         const uint32_t mask = _getInterruptPendingMask( intNum );
         if( (mask & ip) == 0 )
            continue;

         const ISR_ENTRY_T* pCurrentInt = &ISREntryTable[intNum];
         if( pCurrentInt->Callback != NULL )
         {
            pCurrentInt->Callback( intNum, pCurrentInt->pContext );
         }
         else
         {
            irqSetMaskRegister( irqGetMaskRegister() & ~mask );
         }

         irqResetPendingRegister( mask );
         break;
      }
   }
}

/*! ---------------------------------------------------------------------------
 * @see lm32Interrupts.h
 */
void irqRegisterISR( const unsigned int intNum, void* pContext, ISRCallback Callback )
{
   IRQ_ASSERT( intNum < ARRAY_SIZE( ISREntryTable ) );

   ISREntryTable[intNum].Callback = Callback;
   ISREntryTable[intNum].pContext = pContext;

   const uint32_t mask = _getInterruptPendingMask( intNum );
   const uint32_t im = irqGetMaskRegister();
   irqSetMaskRegister( (Callback == NULL)? (im & ~mask) : (im | mask) );
}

/*! ---------------------------------------------------------------------------
 * @see lm32Interrupts.h
 */
void irqDisableSpecific( const unsigned int intNum )
{
   IRQ_ASSERT( intNum < ARRAY_SIZE( ISREntryTable ) );
   irqSetMaskRegister( irqGetMaskRegister() & ~_getInterruptPendingMask( intNum ) );
}

/*! ---------------------------------------------------------------------------
 * @see lm32Interrupts.h
 */
void irqEnableSpecific( const unsigned int intNum )
{
   IRQ_ASSERT( intNum < ARRAY_SIZE( ISREntryTable ) );
   irqSetMaskRegister( irqGetMaskRegister() & _getInterruptPendingMask( intNum ) );
}

/*! ---------------------------------------------------------------------------
 * @see lm32Interrupts.h
 *
 * When the compiler and linker has LTO ability so the following inline
 * declarations will be really inline.
 */
inline void criticalSectionEnter( void )
{
   mg_criticalSectionNestingCount++;
   irqDisable();
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
      irqEnable();
}


/*================================== EOF ====================================*/
