/** @file irq.h
 *  @brief Header file for MSI capable IRQ handler for the LM32
 *
 *  Copyright (C) 2011-2012 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  Usage:
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

#ifndef __IRQ_H_
#define __IRQ_H_
extern int frame;
extern int frame_cnt;

inline  unsigned int  irq_get_mask(void);

inline void irq_set_mask( unsigned int im);

inline  unsigned int  irq_get_enable(void);

inline void irq_disable(void);

inline void irq_enable(void);

inline void irq_clear( unsigned int mask);

inline void isr_table_clr(void);

inline void irq_process(void);

inline void disable_irq(void);
inline void enable_irq(void);
inline void clear_irq(void);

#endif
