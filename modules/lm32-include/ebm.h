/** @file ebm.h
 *  @brief Header file for EB Master Core
 *
 *  Copyright (C) 2011-2012 GSI Helmholtz Centre for Heavy Ion Research GmbH 
 *
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
#ifndef EBM_H
#define EBM_H
#include <inttypes.h>
extern volatile unsigned int* pEbm;
extern volatile unsigned int* pEbmLast;


volatile uintptr_t EBM_OFFS_DAT;
volatile uintptr_t EBM_OFFS_WR;
volatile uintptr_t EBM_ADR_MASK;
volatile uintptr_t EBM_WRITE;           
volatile uintptr_t EBM_READ;

#define EBM_REG_CLEAR         0                         
#define EBM_REG_FLUSH         (EBM_REG_CLEAR        +4)        
#define EBM_REG_STATUS        (EBM_REG_FLUSH        +4)         
#define EBM_REG_SRC_MAC_HI    (EBM_REG_STATUS       +4)       
#define EBM_REG_SRC_MAC_LO    (EBM_REG_SRC_MAC_HI   +4)    
#define EBM_REG_SRC_IPV4      (EBM_REG_SRC_MAC_LO   +4)    
#define EBM_REG_SRC_UDP_PORT  (EBM_REG_SRC_IPV4     +4)   
#define EBM_REG_DST_MAC_HI    (EBM_REG_SRC_UDP_PORT +4)  
#define EBM_REG_DST_MAC_LO    (EBM_REG_DST_MAC_HI   +4)   
#define EBM_REG_DST_IPV4      (EBM_REG_DST_MAC_LO   +4)  
#define EBM_REG_DST_UDP_PORT  (EBM_REG_DST_IPV4     +4)   
#define EBM_REG_MTU           (EBM_REG_DST_UDP_PORT +4)  
#define EBM_REG_ADR_HI        (EBM_REG_MTU          +4)    
#define EBM_REG_OPS_MAX       (EBM_REG_ADR_HI       +4) 
#define EBM_REG_EB_OPT        (EBM_REG_OPS_MAX      +4) 
#define EBM_REG_LAST          (EBM_REG_EB_OPT; 

#define EBM_STAT_CONFIGURED  0x00000001
#define EBM_STAT_BUSY        0x00000002
#define EBM_STAT_ERROR       0x00000004
#define EBM_STAT_EB_SENT     0xFFFF0000

#define EBM_OFFS_LOCAL  (EBM_REG_SRC_MAC_HI)
#define EBM_OFFS_REMOTE (EBM_REG_DST_MAC_HI)

#define EBM_OFFS_MAC_HI    0       
#define EBM_OFFS_MAC_LO    (EBM_OFFS_MAC_HI   +4)    
#define EBM_OFFS_IPV4      (EBM_OFFS_MAC_LO   +4)    
#define EBM_OFFS_UDP_PORT  (EBM_OFFS_IPV4     +4)

typedef struct {
  /* Contents must fit in 12 bytes */
  unsigned char mac[6];
  unsigned char ipv4[4];
  unsigned short port;		
} eb_lm32_udp_link;
typedef unsigned int adress_type_t;
typedef unsigned char target_t;

static const target_t       LOCAL   = 0;
static const target_t       REMOTE  = 1;
static const adress_type_t  MAC     = 1;
static const adress_type_t  IP      = 2;
static const adress_type_t  PORT    = 3;
static const unsigned short myPort  = 0xEBD0;


void ebm_config_if(target_t conf, const char* con_info);
void ebm_config_meta(unsigned int mtu, unsigned int hi_bits, unsigned int max_ops, unsigned int eb_ops);
void ebm_hi(unsigned int address);
void ebm_op(unsigned int address, unsigned int value, unsigned int optype);
void ebm_flush(void);


#endif
