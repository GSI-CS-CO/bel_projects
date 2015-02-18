/** @file irq.h
 *  @brief Header file for ethernet definitions
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

#ifndef __ETHERNET_H_
#define __ETHERNET_H_
#define ETH_HEADER_L 14

/* Ethernet header without VLAN, 112 bits / 32 bit wb = 3 reads plus 16 bits */
#define ETH_HEADER  3
/* ip/tcp/udp header 64 bits x 2 wb */
#define PROTO_HEADER 4
/* ts 64 bits x 2 wb */
#define PROTO_ID    8
#define PROTO_TS_B  11
#define PROTO_TS_E  12

#define MASK_HEADER 0xffff
#define MASK_SWAP   0xffff
#define MASK_PROT   0xff

#endif
