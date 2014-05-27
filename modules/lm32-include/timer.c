/** @file pCpuTimer.c
 *  @brief MSI capable Timer Interrupt for the LM32
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

#include "timer.h"
      
inline void irq_tm_rst()
{
    *(pCpuTimer + r_timer.rst) = 1;  
} 

// pCpuTimer arm
inline unsigned int irq_tm_get_arm(void)
{
  return *(pCpuTimer + r_timer.armGet);
}

inline void irq_tm_set_arm(unsigned int val)
{
  *(pCpuTimer + r_timer.armSet) = val;
}

inline void irq_tm_clr_arm(unsigned int val)
{
  *(pCpuTimer + r_timer.armClr) = val;
}

// comparator src
inline unsigned int irq_tm_get_src(void)
{
  return *(pCpuTimer + r_timer.srcGet);
}

inline void irq_tm_set_src(unsigned int val)
{
  *(pCpuTimer + r_timer.srcSet) = val;
}

inline void irq_tm_clr_src(unsigned int val)
{
  *(pCpuTimer + r_timer.srcClr) = val;
}

// counter mode
inline unsigned int irq_tm_get_cnt_mode(void)
{
  return *(pCpuTimer + r_timer.cntModeGet);
}

inline void irq_tm_set_cnt_mode(unsigned int val)
{
  *(pCpuTimer + r_timer.cntModeSet) = val;
}

inline void irq_tm_clr_cnt_mode(unsigned int val)
{
  *(pCpuTimer + r_timer.cntModeClr) = val;
}

// cascaded start
inline unsigned int irq_tm_get_csc(void)
{
  return *(pCpuTimer + r_timer.cscGet);
}

inline void irq_tm_set_csc(unsigned int val)
{
  *(pCpuTimer + r_timer.cscSet) = val;
}

inline void irq_tm_clr_csc(unsigned int val)
{
  *(pCpuTimer + r_timer.cscClr) = val;
}

// counter reset
inline void irq_tm_cnt_rst(unsigned int val)
{
  *(pCpuTimer + r_timer.cntRst) = val;
}

//pCpuTimer select
inline void irq_tm_pCpuTimer_sel(unsigned int val)
{
  *(pCpuTimer + r_timer.timerSel) = val;
}

//deadline
inline void irq_tm_deadl_set(unsigned long long deadline)
{
   *(pCpuTimer + r_timer.timeHi) = (unsigned int)(deadline>>32);
   *(pCpuTimer + r_timer.timeLo) = (unsigned int)(deadline & 0xffffffff);  
}

unsigned long long irq_tm_deadl_get(unsigned char idx)
{
   unsigned long long deadline;
   irq_tm_pCpuTimer_sel(idx);
   deadline =  ((unsigned long long )(*(pCpuTimer + r_timer.timeHi))<<32);
   deadline |=  (unsigned long long ) *(pCpuTimer + r_timer.timeLo);
   return deadline;  
}

//irq address and message
inline void irq_tm_msi_set(unsigned int dst, unsigned int msg)
{
   *(pCpuTimer + r_timer.dstAdr) = dst;
   *(pCpuTimer + r_timer.msg)    = msg;  
}

//cascade counter 'reciever' to pCpuTimer 'sender'. sender -1 means no cascade
inline void irq_tm_cascade(char receiver, char sender)
{
   if(sender > r_timer.cfg_NO_CASCADE) irq_tm_clr_csc(1<<receiver);     
   else
   {
      irq_tm_set_csc(1<<receiver);      
      *(pCpuTimer + r_timer.cscSel)  = sender; 
   }   
}

inline void irq_tm_start(unsigned int val) {irq_tm_set_arm(val);}
inline void irq_tm_stop (unsigned int val) {irq_tm_clr_arm(val);}


void irq_tm_trigger_at_time(unsigned char idx, unsigned long long deadline)
{
   irq_tm_stop(1<<idx);
   irq_tm_clr_src(1<<idx);
   irq_tm_pCpuTimer_sel(idx);
   irq_tm_deadl_set(deadline);
   irq_tm_start(1<<idx);    
}


void irq_tm_write_config(unsigned char idx, s_timer* tm )
{
   irq_tm_stop(1<<idx);   
   irq_tm_cnt_rst(1<<idx);   
   irq_tm_pCpuTimer_sel(idx);
 
   if(tm->src == r_timer.cfg_ABS_TIME)    irq_tm_clr_src(1<<idx);
   else                                   irq_tm_set_src(1<<idx);
   
   if(tm->mode == r_timer.cfg_PERIODIC)   irq_tm_set_cnt_mode(1<<idx);
   else                                   irq_tm_clr_cnt_mode(1<<idx);

   irq_tm_cascade(idx, tm->cascade);
   irq_tm_deadl_set(tm->deadline);
   irq_tm_msi_set(tm->msi_dst, tm->msi_msg);
  
}














