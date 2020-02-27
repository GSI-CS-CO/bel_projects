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
#include "eb_console_helper.h"
#include "scu_lm32Timer.h"
#include "lm32Interrupts.h"

volatile uint32_t pxCurrentTCB = 0;

/* Port Enable Interrupts */
#define portENABLE_INTERRUPTS()             \
{                                           \
   const uint32_t ie = 0x01;                \
   asm volatile ( "wcsr ie, %0"::"r"(ie) ); \
}

#define portDISABLE_INTERRUPTS() asm volatile ("wcsr   ie, r0");

#define configCPU_CLOCK_HZ   (USRCPUCLK * 1000)


static inline void init( void )
{
   discoverPeriphery(); // mini-sdb: get info on important Wishbone infrastructure
   uart_init_hw();      // init UART, required for printf...
}

volatile static unsigned int g_count = 0;

static void onTimerInterrupt( const unsigned int intNum, const void* pContext )
{
   mprintf( "%s( %d, 0x%x ), count: %d\n", __func__, intNum, (unsigned int)pContext, g_count );
   g_count++;
   mprintf( "Period: %d\n", lm32TimerGetPeriod( (SCU_LM32_TIMER_T*)pContext ) );
  // lm32TimerDisable( (SCU_LM32_TIMER_T*)pContext );
   //lm32TimerSetPeriod( (SCU_LM32_TIMER_T*)pContext, configCPU_CLOCK_HZ );
}

volatile uint32_t* wb_timer_base   = 0;

void main( void )
{
   init();
   mprintf( "Timer IRQ test\nCompiler: " COMPILER_VERSION_STRING "\n" );
   mprintf( "CPU frequency: %d Hz\n", configCPU_CLOCK_HZ );
   unsigned int oldCount = g_count - 1;

   SCU_LM32_TIMER_T* pTimer = lm32TimerGetWbAddress();
   if( (unsigned int)pTimer == ERROR_NOT_FOUND )
   {
      mprintf( ESC_ERROR "ERROR: Timer not found!\n" ESC_NORMAL );
      while( true );
   }

   mprintf( "Timer found at wishbone base address 0x%x\n", (unsigned int)pTimer );

   lm32TimerSetPeriod( pTimer, configCPU_CLOCK_HZ );
   lm32TimerEnable( pTimer );
   registerISR( TIMER_IRQ, (void*)pTimer, onTimerInterrupt );
   portENABLE_INTERRUPTS();

   while( true )
   {
      if( oldCount != g_count )
      {
         mprintf( "C: %d\n", g_count );
         oldCount = g_count;
         if( oldCount == 10 )
         {
          //  portDISABLE_INTERRUPTS();
            disableSpecificInterrupt( TIMER_IRQ );
         }
      }
   }
}

/*================================== EOF ====================================*/
