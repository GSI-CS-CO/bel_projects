/** @file irq.h
 *  @brief Header file for MSI capable IRQ handler for the LM32
 *
 *  @copyright (C) 2011-2012 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  Usage:
 *  @code
 *  void <an ISR>(void) { <evaluate global_msi and do something useful> }
 *  ...
 *  void _irq_entry(void) {irq_process();}
 *  ...
 *  void main(void) {
 *
 *    isr_table_clr();
 *    isr_ptr_table[0]= <an ISR>;
 *    isr_ptr_table[1]= ...
 *    ...   
 *    irq_set_mask(0x03); //Enable used IRQs ...
 *    irq_enable(); 
 *    ...
 *  }
 *  @endcode
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

#ifndef __IRQ_H_
#define __IRQ_H_

#include <inttypes.h>
#include <stdint.h>
#include "mprintf.h"

#ifndef __GNUC_STDC_INLINE__
#error NEEDS gnu99 EXTENSIONS - ADD '-std=gnu99' TO THE CFGLAGS OF YOUR Makefile!
#endif

extern volatile uint32_t* pCpuIrqSlave;
extern volatile uint32_t* pCpuMsiBox;
extern volatile uint32_t* pMyMsi;

#define NESTED_IRQS 0

#define IRQ_REG_RST  0x00000000
#define IRQ_REG_STAT 0x00000004
#define IRQ_REG_POP  0x00000008
#define IRQ_OFFS_QUE 0x00000020
#define IRQ_OFFS_MSG 0x00000000
#define IRQ_OFFS_ADR 0x00000004
#define IRQ_OFFS_SEL 0x00000008

typedef struct
{
   uint32_t  msg;
   uint32_t  adr;
   uint32_t  sel;
} msi; 

//ISR function pointer table
typedef void (*isr_ptr_t)(void);

extern volatile isr_ptr_t isr_ptr_table[32]; 

//Global containing last processed MSI message
extern volatile msi global_msi;


inline void irq_pop_msi( uint32_t irq_no)
{
   
   uint32_t  offset    = (IRQ_OFFS_QUE + (irq_no<<4));    //queue is at 32 + irq_no * 16
   uint32_t* msg_queue = (uint32_t*)(pCpuIrqSlave + (offset >>2));
    global_msi.msg =  *(msg_queue+((uint32_t)IRQ_OFFS_MSG>>2));
    global_msi.adr =  *(msg_queue+((uint32_t)IRQ_OFFS_ADR>>2)); 
    global_msi.sel =  *(msg_queue+((uint32_t)IRQ_OFFS_SEL>>2));
    *(pCpuIrqSlave + (IRQ_REG_POP>>2)) = 1<<irq_no;   
} 

inline void isr_table_clr(void)
{
  //set all ISR table entries to Null
  uint32_t i;
  for(i=0;i<32;i++)  isr_ptr_table[i] = 0; 
}

inline  uint32_t  irq_get_mask(void)
{
    //read IRQ mask
    uint32_t im;
    asm ( "rcsr %0, im": "=&r" (im));
    return im;                   
}


inline void irq_set_mask( uint32_t im)
{
    //write IRQ mask
    asm (   "wcsr im, %0" \
            :             \
            : "r" (im)    \
        );
}

inline  uint32_t  irq_get_enable(void)
{
    //read global IRQ enable bit
    uint32_t eie;
    asm ( "rcsr %0, ie\n"  \
          "srui %0, %0, 1\n" \
          "andi %0, %0, 1" \
         : "=&r" (eie));
    return eie;
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


inline void irq_clear( uint32_t mask)
{
    //clear pending interrupt flag(s)
    asm           (  "wcsr ip, %0"  \
                     :              \
                     : "r" (mask)   \
                     :              \
                     );
}

void _irq_entry(void); 
void cfgMsiBox(uint8_t slot, uint32_t myOffs);
int getMsiBoxSlot(uint32_t myOffs);
int getMsiBoxCpuSlot(uint32_t cpuIdx, uint32_t myOffs);

#endif // include guard
