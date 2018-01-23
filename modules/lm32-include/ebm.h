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
#include <stdint.h>
#include "ebm_regs.h"

#define EBC_SRC_IP 0x18 // reg offset in ebconfig space for source IP
#define EBC_DEFAULT_IP 0xc0a80064

extern volatile uint32_t* pEbm;
extern volatile uint32_t* pEbmLast;


volatile uintptr_t EBM_OFFS_DAT;
volatile uintptr_t EBM_OFFS_WR;
volatile uintptr_t EBM_ADR_MASK;
volatile uintptr_t EBM_WRITE;           
volatile uintptr_t EBM_READ;

#define EBM_STAT_CONFIGURED  0x00000001
#define EBM_STAT_BUSY        0x00000002
#define EBM_STAT_ERROR       0x00000004
#define EBM_STAT_EB_SENT     0xFFFF0000
#define EBM_NOREPLY          1<<28
#define EBM_USEFEC           1<<24  


typedef struct {
  /* Contents must fit in 12 bytes */
  unsigned char mac[6];
  unsigned char ipv4[4];
  unsigned short port;		
} eb_lm32_udp_link;
typedef uint32_t adress_type_t;
typedef unsigned char target_t;

static const target_t       SOURCE      = 0;
static const target_t       DESTINATION = 1;
static const adress_type_t  MAC         = 1;
static const adress_type_t  IP          = 2;
static const adress_type_t  PORT        = 3;
static const unsigned short myPort      = 0xEBD0;

void ebm_init();
void ebm_config_if_str(target_t conf, const char* con_info);
void ebm_config_if(target_t conf, uint64_t mac, uint32_t ip, uint16_t port);
void ebm_config_meta(uint32_t mtu, uint32_t hi_bits, uint32_t eb_ops);
void ebm_hi(uint32_t address);
void ebm_op(uint32_t address, uint32_t value, uint32_t optype);
void ebm_flush(void);
void ebm_clr(void);
void ebm_flush(void);


void ebm_udp_start(void);
void ebm_udp(uint32_t value);
void ebm_udp_end(void);

#endif
