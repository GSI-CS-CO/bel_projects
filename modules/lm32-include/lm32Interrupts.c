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
#include <helper_macros.h>

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
 *       that the standard interrupt priority will changed.
 * @param intNum Interrupt number.
 * @return Interrupt pending mask.
 */
__attribute__((weak))
uint32_t _getInterruptPendingMask( const unsigned int intNum )
{
   return (1 << intNum);
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
   uint32_t ip, im;
   /*
    * Read the Interrupt Mask register
    */
   asm volatile ( "rcsr %0, im" :"=r"(im) );

   while( true )
   {
      /*
       * Read the Interrupt Pending register
       */
      asm volatile ( "rcsr %0, ip" :"=r"(ip) );
      ip &= im;
      if( ip == 0 ) /* No interrupt pending? */
         return; /* Yes, life the function */

      for( unsigned int intNum = 0; intNum < ARRAY_SIZE( ISREntryTable ); intNum++ )
      {
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
            asm volatile ( "rcsr %0, im" :"=r"(im) );
            im &= ~mask;
            asm volatile ( "wcsr im, %0" ::"r"(im) );
         }
         /*
          * Clear the corresponding Interrupt Pending bit by writing a one!
          */
         asm volatile ( "wcsr ip, %0" ::"r"(mask) );
         break;
      }
   }
}

/*! ---------------------------------------------------------------------------
 * @see lm32Interrupts.h
 */
bool registerISR( const unsigned int intNum, void* pContext, ISRCallback Callback )
{
   if( intNum >= ARRAY_SIZE( ISREntryTable ) )
      return true;

   ISREntryTable[intNum].Callback = Callback;
   ISREntryTable[intNum].pContext = pContext;

   const uint32_t mask = _getInterruptPendingMask( intNum );
   /* mask/unmask bit in the im */
   uint32_t im;
   asm volatile ("rcsr %0, im":"=r"(im));
   im = (Callback == NULL)? (im & ~mask) : (im | mask);
   asm volatile ("wcsr im, %0"::"r"(im));

   return false;
}

/*! ---------------------------------------------------------------------------
 * @see lm32Interrupts.h
 */
bool disableSpecificInterrupt( const unsigned int intNum )
{
   if( intNum >= ARRAY_SIZE( ISREntryTable ) )
      return true;

   const uint32_t invMask = ~_getInterruptPendingMask( intNum );
   uint32_t im;
   /* disable mask-bit in im */
   asm volatile ("rcsr %0, im":"=r"(im));
   im &= invMask;
   asm volatile ("wcsr im, %0"::"r"(im));

   return false;
}

/*! ---------------------------------------------------------------------------
 * @see lm32Interrupts.h
 */
bool enableSpecificInterrupt( const unsigned int intNum )
{
   if( intNum >= ARRAY_SIZE( ISREntryTable ) )
      return true;

   const uint32_t mask = _getInterruptPendingMask( intNum );
   uint32_t im;
   /* enable mask-bit in im */
   asm volatile ("rcsr %0, im":"=r"(im));
   im |= mask;
   asm volatile ("wcsr im, %0"::"r"(im));

   return false;
}

/*================================== EOF ====================================*/
