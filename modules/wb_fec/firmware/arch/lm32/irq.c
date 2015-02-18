/** @file irq.c
 *  @brief MSI IRQ handler for the LM32
 *
 *  Copyright (C) 2011-2012 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Cesar Prados <c.prados@gsi.de>
 *
 *  @bug None!
 *
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

#include "irq.h"

inline  unsigned int  irq_get_mask(void)
{
    //read IRQ mask
    unsigned int im;
    asm ( "rcsr %0, im": "=&r" (im));
    return im;
}


inline void irq_set_mask( unsigned int im)
{
    //write IRQ mask
    asm (   "wcsr im, %0" \
            :             \
            : "r" (im)    \
        );
}

inline  unsigned int  irq_get_enable(void)
{
    //read global IRQ enable bit
    unsigned int ie;
    asm ( "rcsr %0, ie\n"  \
          "andi %0, %0, 1" \
         : "=&r" (ie));
    return ie;
}

inline void irq_disable(void)
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

inline void irq_enable(void)
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


inline void irq_clear( unsigned int mask)
{
    //clear pending interrupt flag(s)
    asm           (  "wcsr ip, %0"  \
                     :              \
                     : "r" (mask)   \
                     :              \
                     );
}


inline void clear_irq()
{
  //unsigned int val = 0xffffffff;
  unsigned int val = 0x1;
  asm volatile ("wcsr ip, %0"::"r" (val));
}

inline void disable_irq()
{
  unsigned int ie, im;
  unsigned int Mask = ~1;

  /* disable peripheral interrupts in case they were enabled */
  asm volatile ("rcsr %0,ie":"=r" (ie));
  ie &= (~0x1);
  asm volatile ("wcsr ie, %0"::"r" (ie));

  /* disable mask-bit in im */
  asm volatile ("rcsr %0, im":"=r" (im));
  im &= Mask;
  asm volatile ("wcsr im, %0"::"r" (im));

}


inline void enable_irq()
{
  unsigned int ie, im;
  unsigned int Mask = 1;

  /* disable peripheral interrupts in-case they were enabled */
  asm volatile ("rcsr %0,ie":"=r" (ie));
  ie &= (~0x1);
  asm volatile ("wcsr ie, %0"::"r" (ie));

  /* enable mask-bit in im */
  asm volatile ("rcsr %0, im":"=r" (im));
  im |= Mask;
  asm volatile ("wcsr im, %0"::"r" (im));

  ie |= 0x1;
  asm volatile ("wcsr ie, %0"::"r" (ie));

}
//void _irq_entry(void)
//{
//  unsigned int  ip;
//  unsigned char irq_no = 0;
//  unsigned int  msk;
//
//  asm ("rcsr %0, ip": "=r"(ip)); //get pending flags
//
//  while(ip)
//  {
//    if(ip & 1) //check if irq with lowest number is pending
//    {
//      irq_clear(1<<irq_no);     //clear pending bit
//      irq_disable();
//    }
//    irq_no++;
//    ip = ip >> 1; //process next irq
//  }
//}
