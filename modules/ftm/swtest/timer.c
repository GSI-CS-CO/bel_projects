/** @file timer.c
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
      

const unsigned int TM_REG_RST             = 0x00; // wo, Reset Active Low
const unsigned int TM_REG_ARM_STAT        = 0x04; // ro, Shows armed timers,  (1 armed, 0 disarmed), 1 bit per timer
const unsigned int TM_REG_ARM_SET         = 0x08; //  wo, arm timers,          
const unsigned int TM_REG_ARM_CLR         = 0x0C; //  wo, disarm timers,         
const unsigned int TM_REG_SRC_STAT        = 0x10; //  ro, shows timer sources, (1 counter, 0 time), 1 bit per timer 
const unsigned int TM_REG_SRC_SET         = 0x14; //  wo, select counter as source
const unsigned int TM_REG_SRC_CLR         = 0x18; //  wo, select time as source    
const unsigned int TM_REG_CNT_MODE_STAT   = 0x1C; //  ro, shows counter modes, (1 periodic, 0 1time), 1 bit per timer 
const unsigned int TM_REG_CNT_MODE_SET    = 0x20; //  wo, select periodic mode
const unsigned int TM_REG_CNT_MODE_CLR    = 0x24; //  wo, select 1time mode
const unsigned int TM_REG_CSC_STAT        = 0x28; //  ro, shows cascaded start, (1 cascaded, 0 normal), 1 bit per timer 
const unsigned int TM_REG_CSC_SET         = 0x2C; //  wo, set cascaded start
const unsigned int TM_REG_CSC_CLR         = 0x30; //  wo, select normal start    
const unsigned int TM_REG_CNT_RST         = 0x34; //  wo, reset counters, 1 bit per timer

const unsigned int c_BASE_TIMERS          = 0x40; // 
const unsigned int TM_REG_TIMER_SEL       = 0x40; //  rw, timer select. !!!CAUTION!!! content of all c_TM_... regs depends on this
const unsigned int TM_REG_TIME_HI         = 0x44; //  rw, deadline HI word
const unsigned int TM_REG_TIME_LO         = 0x48; //  rw, deadline LO word
const unsigned int TM_REG_MSG             = 0x4C; //  rw, MSI msg to be sent on MSI when deadline is hit
const unsigned int TM_REG_DST_ADR         = 0x50; //  rw, MSI adr to send the msg to when deadline is hit
const unsigned int TM_REG_CSC_SEL         = 0x54; //  rw, select comparator output for cascaded start



inline void irq_tm_rst()
{
    *(timer+(TM_REG_RST>>2)) = 1;  
} 

// timer arm
inline unsigned int irq_tm_get_arm(void)
{
  return *(timer+(TM_REG_ARM_STAT>>2));
}

inline void irq_tm_set_arm(unsigned int val)
{
  *(timer+(TM_REG_ARM_SET>>2)) = val;
}

inline void irq_tm_clr_arm(unsigned int val)
{
  *(timer+(TM_REG_ARM_CLR>>2)) = val;
}

// comparator src
inline unsigned int irq_tm_get_src(void)
{
  return *(timer+(TM_REG_SRC_STAT>>2));
}

inline void irq_tm_set_src(unsigned int val)
{
  *(timer+(TM_REG_SRC_SET>>2)) = val;
}

inline void irq_tm_clr_src(unsigned int val)
{
  *(timer+(TM_REG_SRC_CLR>>2)) = val;
}

// counter mode
inline unsigned int irq_tm_get_cnt_mode(void)
{
  return *(timer+(TM_REG_CNT_MODE_STAT>>2));
}

inline void irq_tm_set_cnt_mode(unsigned int val)
{
  *(timer+(TM_REG_CNT_MODE_SET>>2)) = val;
}

inline void irq_tm_clr_cnt_mode(unsigned int val)
{
  *(timer+(TM_REG_CNT_MODE_CLR >> 2)) = val;
}

// cascaded start
inline unsigned int irq_tm_get_csc(void)
{
  return *(timer+(TM_REG_CSC_STAT>>2));
}

inline void irq_tm_set_csc(unsigned int val)
{
  *(timer+(TM_REG_CSC_SET>>2)) = val;
}

inline void irq_tm_clr_csc(unsigned int val)
{
  *(timer+(TM_REG_CSC_CLR>>2)) = val;
}

// counter reset
inline void irq_tm_cnt_rst(unsigned int val)
{
  *(timer+(TM_REG_CNT_RST>>2)) = val;
}

//timer select
inline void irq_tm_timer_sel(unsigned int val)
{
  *(timer+(TM_REG_TIMER_SEL>>2)) = val;
}

//deadline
inline void irq_tm_deadl_set(unsigned long long deadline)
{
   *(timer+(TM_REG_TIME_HI>>2)) = (unsigned int)(deadline>>32);
   *(timer+(TM_REG_TIME_LO>>2)) = (unsigned int)(deadline & 0xffffffff);  
}

unsigned long long irq_tm_deadl_get(unsigned char idx)
{
   unsigned long long deadline;
   irq_tm_timer_sel(idx);
   deadline =  ((unsigned long long )(*(timer+(TM_REG_TIME_HI>>2)))<<32);
   deadline |= (unsigned long long )*(timer+(TM_REG_TIME_LO>>2));
   return deadline;  
}

//irq address and message
inline void irq_tm_msi_set(unsigned int dst, unsigned int msg)
{
   *(timer+(TM_REG_DST_ADR>>2)) = dst;
   *(timer+(TM_REG_MSG>>2))     = msg;  
}

//cascade counter 'reciever' to timer 'sender'. sender -1 means no cascade
inline void irq_tm_cascade(char receiver, char sender)
{
   if(sender > TIMER_NO_CASCADE) irq_tm_clr_csc(1<<receiver);     
   else
   {
      irq_tm_set_csc(1<<receiver);      
      *(timer+(TM_REG_CSC_SEL>>2)) = sender; 
   }   
}

inline void irq_tm_start(unsigned int val) {irq_tm_set_arm(val);}
inline void irq_tm_stop (unsigned int val) {irq_tm_clr_arm(val);}


void irq_tm_trigger_at_time(unsigned char idx, unsigned long long deadline)
{
   irq_tm_stop(1<<idx);
   irq_tm_clr_src(1<<idx);
   irq_tm_timer_sel(idx);
   irq_tm_deadl_set(deadline);
   irq_tm_start(1<<idx);    
}


void irq_tm_write_config(unsigned char idx, s_timer* tm )
{
   irq_tm_stop(1<<idx);   
   irq_tm_cnt_rst(1<<idx);   
   irq_tm_timer_sel(idx);
 
   if(tm->src == TIMER_ABS_TIME)     irq_tm_clr_src(1<<idx);
   else                          irq_tm_set_src(1<<idx);
   
   if(tm->mode == TIMER_PERIODIC)   irq_tm_set_cnt_mode(1<<idx);
   else                          irq_tm_clr_cnt_mode(1<<idx);

   irq_tm_cascade(idx, tm->cascade);
   irq_tm_deadl_set(tm->deadline);
   irq_tm_msi_set(tm->msi_dst, tm->msi_msg);
  
}














