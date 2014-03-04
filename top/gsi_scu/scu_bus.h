/** @file scu_bus.h
 *  
 *  Copyright (C) 2011-2012 GSI Helmholtz Centre for Heavy Ion Research GmbH 
 *
 *  @author Stefan Rauch <s.rauch@gsi.de>
 *
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

#ifndef __SCU_H_
#define __SCU_H

#define CID_SYS           0x4
#define CID_GROUP         0x5
#define SLAVE_INT_ENA     0x21
#define SLAVE_INT_PEND    0x22
#define SLAVE_INT_ACT     0x24

#define DAC1_BASE         0x200
#define DAC2_BASE         0x210
#define DAC_CNTRL         0x0
#define DAC_DATA          0x1

#define IO4x8             0x220
#define ADC_BASE          0x230
#define ADC_CNTRL         0x0
#define ADC_CHN1          0x1   
#define ADC_CHN2          0x2   
#define ADC_CHN3          0x3   
#define ADC_CHN4          0x4   
#define ADC_CHN5          0x5   
#define ADC_CHN6          0x6   
#define ADC_CHN7          0x7   
#define ADC_CHN8          0x8   


#define FG_QUAD_BASE      0x300
#define FG_QUAD_CNTRL     0x0
#define FG_QUAD_A         0x1
#define FG_QUAD_B         0x2
#define FG_QUAD_BROAD     0x3
#define FG_QUAD_SHIFTA    0x4
#define FG_QUAD_SHIFTB    0x5
#define FG_QUAD_H         0x6
#define FG_QUAD_L         0x7

#define TMR_BASE          0x330
#define TMR_CNTRL         0x0
#define TMR_IRQ_CNT       0x1
#define TMR_VALUEL        0x2
#define TMR_VALUEH        0x3
 
#define GLOBAL_IRQ_ENA    0x2
#define SRQ_ENA           0x6
#define SRQ_ACT           0x8
#define MULTI_SLAVE_SEL   0xc
#define MULTICAST_ACC     0x8
#define SCU_BUS_MAX_SLOTS 5

void probe_scu_bus(volatile unsigned short*, unsigned short, unsigned short, int*);

#endif
