/*!
 * @file      time_measurement.c
 * @brief     Testprogram to measure timings of some LM32 functions
 *
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      13.01.2020
 *******************************************************************************
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************
*/
#include <stdbool.h>
#include "eb_console_helper.h"
#include "mini_sdb.h"
#include "aux.h"

#include "helper_macros.h"

void init( void )
{
   discoverPeriphery();   // mini-sdb: get info on important Wishbone infrastructure
   uart_init_hw();        // init UART, required for printf...
}

static inline void irqDdisable(void)
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

static inline void irqEnable(void)
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

uint32_t getTimeOffset( void )
{
   volatile uint64_t t1 = getSysTime();
   volatile uint64_t t2 = getSysTime();
   return  (uint32_t)(t2 - t1);
}

  #define CONFIG_FULL_CONTEXT_SAVE

#if __GNUC__ >= 9
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wattributes"
#endif
ALWAYS_INLINE void saveRegs( void )
{
   asm volatile (
               #ifdef CONFIG_FULL_CONTEXT_SAVE
                  "addi    sp, sp, -128\n\t"
               #else
                  "addi    sp, sp, -60\n\t"
               #endif
                  "sw      (sp+4),  r1\n\t"
                  "sw      (sp+8),  r2\n\t"
                  "sw      (sp+12), r3\n\t"
                  "sw      (sp+16), r4\n\t"
                  "sw      (sp+20), r5\n\t"
                  "sw      (sp+24), r6\n\t"
                  "sw      (sp+28), r7\n\t"
                  "sw      (sp+32), r8\n\t"
                  "sw      (sp+36), r9\n\t"
                  "sw      (sp+40), r10\n\t"
               #ifdef CONFIG_FULL_CONTEXT_SAVE
                  "sw      (sp+44), r11\n\t"
                  "sw      (sp+48), r12\n\t"
                  "sw      (sp+52), r13\n\t"
                  "sw      (sp+56), r14\n\t"
                  "sw      (sp+60), r15\n\t"
                  "sw      (sp+64), r16\n\t"
                  "sw      (sp+68), r17\n\t"
                  "sw      (sp+72), r18\n\t"
                  "sw      (sp+76), r19\n\t"
                  "sw      (sp+80), r20\n\t"
                  "sw      (sp+84), r21\n\t"
                  "sw      (sp+88), r22\n\t"
                  "sw      (sp+92), r23\n\t"
                  "sw      (sp+96), r24\n\t"
                  "sw      (sp+100), r25\n\t"
                  "sw      (sp+104), r26\n\t"
                  "sw      (sp+108), r27\n\t"
                  "sw      (sp+120), ea\n\t"
                  "sw      (sp+124), ba\n\t"
                  /* ra and sp need special handling, as they have been modified */
                  "lw      r1, (sp+128)\n\t"
                  "sw      (sp+116), r1\n\t"
                  "mv      r1, sp\n\t"
                  "addi    r1, r1, 128\n\t"
                  "sw      (sp+112), r1\n\t"
               #else
                  "sw      (sp+52), ea\n\t"
                  "sw      (sp+56), ba\n\t"
                 /* ra and sp need special handling, as they have been modified */
                  "lw      r1, (sp+60)\n\t"
                  "sw      (sp+48), r1\n\t"
                  "mv      r1, sp\n\t"
                  "addi    r1, r1, 60\n\t"
                  "sw      (sp+44), r1\n\t"
               #endif
                  : : :
                );
}
#if __GNUC__ >= 9
  #pragma GCC diagnostic pop
#endif

#if __GNUC__ >= 9
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wattributes"
#endif
ALWAYS_INLINE void restoreRegs( void )
{
   asm volatile ( "lw      r1, (sp+4)\n\t"
                  "lw      r2, (sp+8)\n\t"
                  "lw      r3, (sp+12)\n\t"
                  "lw      r4, (sp+16)\n\t"
                  "lw      r5, (sp+20)\n\t"
                  "lw      r6, (sp+24)\n\t"
                  "lw      r7, (sp+28)\n\t"
                  "lw      r8, (sp+32)\n\t"
                  "lw      r9, (sp+36)\n\t"
                  "lw      r10, (sp+40)\n\t"
                #ifdef CONFIG_FULL_CONTEXT_SAVE
                  "lw      r11, (sp+44)\n\t"
                  "lw      r12, (sp+48)\n\t"
                  "lw      r13, (sp+52)\n\t"
                  "lw      r14, (sp+56)\n\t"
                  "lw      r15, (sp+60)\n\t"
                  "lw      r16, (sp+64)\n\t"
                  "lw      r17, (sp+68)\n\t"
                  "lw      r18, (sp+72)\n\t"
                  "lw      r19, (sp+76)\n\t"
                  "lw      r20, (sp+80)\n\t"
                  "lw      r21, (sp+84)\n\t"
                  "lw      r22, (sp+88)\n\t"
                  "lw      r23, (sp+92)\n\t"
                  "lw      r24, (sp+96)\n\t"
                  "lw      r25, (sp+100)\n\t"
                  "lw      r26, (sp+104)\n\t"
                  "lw      r27, (sp+108)\n\t"
                  "lw      ra, (sp+116)\n\t"
                  "lw      ea, (sp+120)\n\t"
                  "lw      ba, (sp+124)\n\t"
                /* Stack pointer must be restored last, in case it has been updated */
                  "lw      sp, (sp+112)\n\t"
                #else
                  "lw      ra, (sp+48)\n\t"
                  "lw      ea, (sp+52)\n\t"
                  "lw      ba, (sp+56)\n\t"
               /* Stack pointer must be restored last, in case it has been updated */
                  "lw      sp, (sp+44)\n\t"
                #endif
                  : : :
                );
}
#if __GNUC__ >= 9
  #pragma GCC diagnostic pop
#endif



int main( void )
{
   init();
   clrscr();
   gotoxy( 0, 0 );
   uint32_t  toff = getTimeOffset();
   mprintf("Start measurement. Compiler: "COMPILER_VERSION_STRING"\ntime-offset = %u\n", toff );

   volatile uint64_t t1 = getSysTime();
   //irqEnable();
   //irqDdisable();
   saveRegs();
   restoreRegs();
   asm volatile( "nop\n\t" : : : );
   asm volatile( "nop\n\t" : : : );
   volatile uint64_t t2 = getSysTime();

   mprintf( "T = %u, %u\n", (unsigned int)t1, (unsigned int)t2 );
   volatile uint64_t d = t2 - t1;

   mprintf( "D = %u\n", (unsigned int)d - toff );
   while( true );
   return 0;
}

/*================================== EOF ====================================*/
