/*!
 *  @file irq_test.c
 *  @brief Listen to all 32 external interrupts of LM32
 *  @date 16.11.2018
 *  @copyright (C) 2018 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *
 *
 *******************************************************************************
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT AqNY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************
 */
#include <string.h>
#include <stdbool.h>
#include "mini_sdb.h"
//#include "../../top/gsi_scu/scu_bus.h"
#include "eb_console_helper.h"
#include "lm32_assert.h"
#include "helper_macros.h"
#include "lm32_hexdump.h"
#include "dbg.h"

volatile unsigned int g_aEvents[32];
static void onIrq( uint8_t n );

// ----------------------------------------------------------------------------
static inline  uint32_t  irq_get_mask(void)
{
    //read IRQ mask
    uint32_t im;
    asm ( "rcsr %0, im": "=&r" (im));
    return im;
}

// ----------------------------------------------------------------------------
static inline void irq_set_mask( uint32_t im)
{
    //write IRQ mask
    asm (   "wcsr im, %0" \
            :             \
            : "r" (im)    \
        );
}

// ----------------------------------------------------------------------------
static inline  uint32_t  irq_get_enable(void)
{
    //read global IRQ enable bit
    uint32_t eie;
    asm ( "rcsr %0, ie\n"  \
          "srui %0, %0, 1\n" \
          "andi %0, %0, 1" \
         : "=&r" (eie));
    return eie;
}

// ----------------------------------------------------------------------------
static inline void irq_disable(void)
{
   //globally disable interrupts
   unsigned foo;
   asm volatile   (  "rcsr %0, IE\n"            \
                     "andi  %0, %0, 0xFFFE\n"   \
                     "wcsr IE, %0"              \
                     : "=r" (foo)               \
                     :                          \
                     :
                     );
}

// -----------------------------------------------------------------------------
static inline void irq_enable(void)
{
   //globally enable interrupts
   unsigned foo;
   asm volatile   (  "rcsr %0, IE\n"      \
                     "ori  %0, %0, 1\n"   \
                     "wcsr IE, %0"        \
                     : "=r" (foo)         \
                     :                    \
                     :                    \
                     );
}

// -----------------------------------------------------------------------------
static inline void irq_clear( uint32_t mask)
{
    //clear pending interrupt flag(s)
    asm           (  "wcsr ip, %0"  \
                     :              \
                     : "r" (mask)   \
                     :              \
                     );
}

// -----------------------------------------------------------------------------
void _irq_entry(void)
{
  uint32_t  ip;
  unsigned char irq_no = 0;

#if NESTED_IRQS
  uint32_t  msk;
#endif
  mprintf( "\nIRQ++++++++++++++++++++++++++++++\n" );
  asm ("rcsr %0, ip": "=r"(ip)); //get pending flags
  while(ip)
  {
    if(ip & 1) //check if irq with lowest number is pending
    {
#if NESTED_IRQS
      msk = irq_get_mask();
      irq_set_mask(msk & ((1<<irq_no)-1) ); //mask out all priorities matching and below current
      irq_enable();
#endif
      //irq_pop_msi(irq_no);      //pop msg from msi queue into global_msi variable
      onIrq( irq_no );
      irq_clear(1<<irq_no);     //clear pending bit
#if NESTED_IRQS
      irq_set_mask(msk);
      irq_disable();
#endif
    }
    irq_no++;
    ip = ip >> 1; //process next irq
  }
}

// ----------------------------------------------------------------------------
static void onIrq( uint8_t n )
{
   LM32_ASSERT( n < ARRAY_SIZE( g_aEvents ) );
   g_aEvents[n]++;
}

// ----------------------------------------------------------------------------
static unsigned int getEventOf( unsigned int n )
{
   unsigned int ret;
   LM32_ASSERT( n < ARRAY_SIZE( g_aEvents ) );
   irq_disable();
   ret = g_aEvents[n];
   irq_enable();
   return ret;
}

#define LINE_START 3

//Slot: 11, Channel 0, Address: 0x80564000
//  CtrlReg: &0x80564000 *0xb002 *0b1011000000000010


//=============================================================================
void main( void )
{
   unsigned int oldEvents[sizeof(g_aEvents)];
   memset( (void*)g_aEvents, 0, sizeof(g_aEvents) );
   memset( oldEvents, 0, sizeof(oldEvents) );
   bool first = true;

   discoverPeriphery();
   uart_init_hw();
   gotoxy( 1, 1 );
   clrscr();
   mprintf( ESC_BOLD "Interrupt listener, compiler: " COMPILER_VERSION_STRING "\n" ESC_NORMAL );
   irq_set_mask( (uint32_t)~0 );
   irq_enable();
   while( true )
   {
      for( int i = 0; i < ARRAY_SIZE( g_aEvents ); i++ )
      {
         unsigned int newEvent = getEventOf( i );
         if( first || (oldEvents[i] != newEvent) )
         {
            oldEvents[i] = newEvent;
            gotoxy( 1, LINE_START + i );
            mprintf( "IRQ %02d: %04d", i, newEvent );
         }
      }
      first = false;
   }
}

/*================================== EOF ====================================*/
