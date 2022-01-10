/*!
 * @file   timerTest.c
 * @brief  Test of LM32 timer-interrupt necessary for FreeRTOS.
 *
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      23.01.2020
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
#include <stdbool.h>
#include "lm32signal.h"
#include "eb_console_helper.h"
#include "scu_lm32Timer.h"
#include "lm32Interrupts.h"
#include "event_measurement.h"

#include "scu_msi.h"

volatile uint32_t pxCurrentTCB = 0;

#define configCPU_CLOCK_HZ   (USRCPUCLK * 10)

TIME_MEASUREMENT_T g_evTime = TIME_MEASUREMENT_INITIALIZER;




void _onException( const uint32_t sig )
{
   char* str;
   #define _CASE_SIGNAL( S ) case S: str = #S; break;
   switch( sig )
   {
      _CASE_SIGNAL( SIGINT )
      _CASE_SIGNAL( SIGTRAP )
      _CASE_SIGNAL( SIGFPE )
      _CASE_SIGNAL( SIGSEGV )
      default: str = "unknown"; break;
   }
   mprintf( ESC_ERROR "%s( %d ): %s\n" ESC_NORMAL, __func__, sig, str );
   while( true );
}

void _onSysCall( const uint32_t sp )
{
   mprintf( "%s( 0x%08x )\n", __func__, sp );
}

volatile static unsigned int g_count = 0;

static void onTimerInterrupt( const unsigned int intNum, const void* pContext )
{
   timeMeasure( &g_evTime );

//   mprintf( "%s( %d, 0x%p ), count: %d\n", __func__, intNum, pContext, g_count );
   g_count++;
//  ATOMIC_SECTION()
//  mprintf( "Period: %d\n", lm32TimerGetPeriod( (SCU_LM32_TIMER_T*)pContext ) );
  // lm32TimerDisable( (SCU_LM32_TIMER_T*)pContext );
   //lm32TimerSetPeriod( (SCU_LM32_TIMER_T*)pContext, configCPU_CLOCK_HZ );
}


void main( void )
{
   mprintf( "Timer IRQ test\nCompiler: " COMPILER_VERSION_STRING "\n" );
   mprintf( "CPU frequency: %d Hz\n", configCPU_CLOCK_HZ );
   unsigned int oldCount = g_count - 1;

   SCU_LM32_TIMER_T* pTimer = lm32TimerGetWbAddress();
   if( (unsigned int)pTimer == ERROR_NOT_FOUND )
   {
      mprintf( ESC_ERROR "ERROR: Timer not found!\n" ESC_NORMAL );
      while( true );
   }

   mprintf( "Timer found at wishbone base address 0x%p\n", pTimer );

   lm32TimerSetPeriod( pTimer, configCPU_CLOCK_HZ );
   lm32TimerEnable( pTimer );
   irqRegisterISR( TIMER_IRQ, (void*)pTimer, onTimerInterrupt );
   irqEnable();
   asm volatile ( "wcsr ie, r0" );
   int x = 100;
   while( true )
   {
      volatile unsigned int currentCount;
      ATOMIC_SECTION() currentCount = g_count;
      if( oldCount != currentCount )
      {
         //uint32_t evt = timeMeasureGetLast32Safe( &g_evTime );
         //mprintf( "\nEVT: %d\n", evt );
         mprintf( "\nEVT: " );
        // timeMeasurePrintSecondsSafe( &g_evTime );
         if( timeMeasureIsValid( &g_evTime ) )
            timeMeasurePrintMilliseconds( &g_evTime );
         mprintf( "\nC: %d\n", currentCount );
         oldCount = currentCount;
         /*
          * CAUTION: When oldCount the value 10 reached it will trigger
          *          an LM32 exception (SIGFPE division by zero).
          */
         x = x / (10 - oldCount);
         mprintf( "x = %d\n", x );
         //ATOMIC_SECTION()
        // while( true );
      }
   }
}

/*================================== EOF ====================================*/
