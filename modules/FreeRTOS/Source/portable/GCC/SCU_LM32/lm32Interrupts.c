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
 * @brief ISR entry type
 */
typedef struct
{
   ISRCallback Callback;
   void*       pContext;
} ISR_ENTRY_T;

/*!
 * @brief  ISREntry table
 */
static ISR_ENTRY_T ISREntryTable[MAX_LM32_INTERRUPTS] = {{NULL, NULL}};

/*! ---------------------------------------------------------------------------
 * @brief General Interrupt Handler (invoked by low-level routine in portasm.S)
 *
 * If an interrupt-handler exists for the relevant interrupt (as detected
 * from "ip" and "im" cpu registers), then invoke the handler else disable the
 * interrupt in the register "im".
 */
void ISRHandler( void )
{
   uint32_t ip, im;
   asm volatile ( "rcsr %0, im" :"=r"(im) );

   while( true )
   {
      /* read ip and calculate effective ip */
      asm volatile ( "rcsr %0, ip" :"=r"(ip) );
      ip &= im;
      if( ip == 0 )
         break;

      for( unsigned int intNum = 0; intNum < ARRAY_SIZE( ISREntryTable ); intNum++ )
      {
         const uint32_t mask = (1 << intNum);
         if( (mask & ip) != 0 )
         {
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
            asm volatile ( "wcsr ip, %0" ::"r"(mask) );
            break;
         }
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

   const uint32_t mask = (1 << intNum);
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

   const uint32_t invMask = ~(1 << intNum);
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

   const uint32_t mask = (0x1 << intNum);
   uint32_t im;
   /* enable mask-bit in im */
   asm volatile ("rcsr %0, im":"=r"(im));
   im |= mask;
   asm volatile ("wcsr im, %0"::"r"(im));

   return false;
}

/*================================== EOF ====================================*/
