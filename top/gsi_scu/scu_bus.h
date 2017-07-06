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

#include "inttypes.h"


#define CID_SYS           0x4
#define CID_GROUP         0x5
#define SLAVE_VERSION     0x6
#define SLAVE_INT_ENA     0x21
#define SLAVE_INT_PEND    0x22
#define SLAVE_INT_ACT     0x24
#define SLAVE_EXT_CLK     0x30

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


#define FG1_BASE          0x300
#define FG2_BASE          0x340
#define FG_CNTRL          0x0
#define FG_A              0x1
#define FG_B              0x2
#define FG_BROAD          0x3
#define FG_SHIFT          0x4
#define FG_STARTH         0x5
#define FG_STARTL         0x6
#define FG_RAMP_CNT_LO    0x7
#define FG_RAMP_CNT_HI    0x8
#define FG_TAG_LOW        0x9
#define FG_TAG_HIGH       0xa
#define FG_VER            0xb

#define FG1_IRQ           (1<<15)
#define FG2_IRQ           (1<<14)

#define WB_FG_CNTRL       0x0
#define WB_FG_A           0x1
#define WB_FG_B           0x2
#define WB_FG_BROAD       0x3
#define WB_FG_SHIFTA      0x4
#define WB_FG_SHIFTB      0x5
#define WB_FG_START       0x6
#define WB_RAMP_CNT       0x7
#define WB_FG_SW_DST      0x8

#define TMR_BASE          0x330
#define TMR_CNTRL         0x0
#define TMR_IRQ_CNT       0x1
#define TMR_VALUEL        0x2
#define TMR_VALUEH        0x3
#define TMR_REPEAT        0x4
 
#define GLOBAL_IRQ_ENA    0x2
#define SRQ_ENA           0x6
#define SRQ_ACT           0x8
#define MULTI_SLAVE_SEL   0xc
#define MULTICAST_ACC     0x8
#define MAX_SCU_SLAVES    12

#define SYS_LOEP    3
#define SYS_CSCO    55
#define SYS_PBRF    42

#define GRP_ADDAC1  3
#define GRP_ADDAC2  38
#define GRP_DIOB    26
#define GRP_FIB_DDS 1
#define GRP_MFU     2
#define GRP_SIO3    69


extern struct w1_bus wrpc_w1_bus;
void ReadTemperatureDevices(int bus, uint64_t *id, uint16_t *temp);
void probe_scu_bus(volatile unsigned short*, unsigned short, unsigned short, int*);
void ReadTempDevices(int bus, uint64_t *id, uint32_t *temp);
#endif
