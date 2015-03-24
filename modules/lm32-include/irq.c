/** @file irq.c
 *  @brief MSI IRQ handler for the LM32
 *
 *  Copyright (C) 2011-2012 GSI Helmholtz Centre for Heavy Ion Research GmbH 
 *
 *  @author Mathias Kreider <m.kreider@gsi.de>
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

volatile isr_ptr_t isr_ptr_table[32]; 

//Global containing last processed MSI message
volatile msi global_msi;

extern inline void       irq_pop_msi( uint32_t irq_no);
extern inline void       isr_table_clr(void);
extern inline uint32_t   irq_get_mask(void);
extern inline void       irq_set_mask( uint32_t im);
extern inline uint32_t   irq_get_enable(void);
extern inline void       irq_disable(void);
extern inline void       irq_enable(void);
extern inline void       irq_clear( uint32_t mask);

void _irq_entry(void)
{
  uint32_t  ip;
  unsigned char irq_no = 0;
#if NESTED_IRQS
  uint32_t  msk;
#endif  
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
      irq_pop_msi(irq_no);      //pop msg from msi queue into global_msi variable
      irq_clear(1<<irq_no);     //clear pending bit
      isr_ptr_table[irq_no]();  //execute isr
#if NESTED_IRQS
      irq_set_mask(msk);
      irq_disable();
#endif
    }  
    irq_no++; 
    ip = ip >> 1; //process next irq
  }
}  

