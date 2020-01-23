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
#include "mini_sdb.h"
#include "helper_macros.h"
#include "lm32Timer.h"
#include "lm32Interrupts.h"

volatile uint32_t pxCurrentTCB = 0;

static inline void init( void )
{
   discoverPeriphery(); // mini-sdb: get info on important Wishbone infrastructure
   uart_init_hw();      // init UART, required for printf...
}

volatile static unsigned int g_count = 0;

static void onTimerInterrupt( const unsigned int intNum, const void* pContext )
{
   g_count++;
}

#define TIMER_IRQ_ 0

int main( void )
{
   init();
   mprintf( "Timer IRQ test\nCompiler: " COMPILER_VERSION_STRING "\n" );
   unsigned int oldCount = g_count - 1;

   registerISR( TIMER_IRQ_, NULL, onTimerInterrupt );

   while( true )
   {
      if( oldCount != g_count )
      {
         mprintf( "C: %d\n", g_count );
         oldCount = g_count;
      }
   }
   return 0;
}

/*================================== EOF ====================================*/
