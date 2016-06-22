/** @file ebm.c
 *  @brief EtherBone Master API
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
#include <stdlib.h>
#include <string.h>
#include "ebm.h"


static inline char* strsplit(const char*  numstr, const char* delimeter);
static inline unsigned char* numStrToBytes(const char*  numstr, unsigned char* bytes,  unsigned char len,  unsigned char base, const char* delimeter);
static inline unsigned char* addressStrToBytes(const char* addressStr, unsigned char* addressBytes, adress_type_t addtype);
static inline uint32_t ebm_parse_adr(eb_lm32_udp_link* link, const char* address);

void ebm_init()
{
   uintptr_t ebmMask = (uintptr_t)pEbmLast - (uintptr_t)pEbm;
   EBM_ADR_MASK  = ebmMask >> 2;
   EBM_OFFS_DAT  = ebmMask ^ (ebmMask >> 1);
   EBM_OFFS_WR   = EBM_OFFS_DAT >> 1; 
   EBM_WRITE     = EBM_OFFS_WR;
   EBM_READ      = 0;  
}


void ebm_config_if(target_t conf, uint64_t mac, uint32_t ip, uint16_t port)
{
  
  uint32_t offset;
  
  if(conf == SOURCE) {
    *(pEbm + (EBM_SRC_MAC_RW_0 >>2)) = (uint32_t)(mac);      
    *(pEbm + (EBM_SRC_MAC_RW_1 >>2)) = (uint32_t)((mac>>32) & 0xffff);  
    *(pEbm + (EBM_SRC_IP_RW   >>2)) = ip;  
    *(pEbm + (EBM_SRC_PORT_RW >>2)) = (uint32_t)port;
  }

  if(conf == DESTINATION) {
  
    *(pEbm + (EBM_DST_MAC_RW_0 >>2)) = (uint32_t)(mac);      
    *(pEbm + (EBM_DST_MAC_RW_1 >>2)) = (uint32_t)((mac>>32) & 0xffff);  
    *(pEbm + (EBM_DST_IP_RW   >>2)) = ip;  
    *(pEbm + (EBM_DST_PORT_RW >>2)) = (uint32_t)port;
  }
}


void ebm_config_if_str(target_t conf, const char* con_info)
{
  eb_lm32_udp_link link;
  uint32_t offset;
  
  uint32_t tmp;
  ebm_parse_adr(&link, con_info);
  
  
  if(conf == SOURCE) {
    tmp = (link.mac[0] << 24) | (link.mac[1] << 16) | (link.mac[2] << 8) | link.mac[3];
    *(pEbm + ((offset + EBM_SRC_MAC_RW_1)   >>2)) =  tmp;  
    tmp = (link.mac[4] << 8) | (link.mac[5] << 0);
    *(pEbm + ((offset + EBM_SRC_MAC_RW_0)   >>2)) = tmp;
    tmp = (link.ipv4[0] << 24) | (link.ipv4[1] << 16) | (link.ipv4[2] << 8) | link.ipv4[3];
    *(pEbm + ((offset + EBM_SRC_IP_RW)     >>2)) =  tmp;  
    *(pEbm + ((offset + EBM_SRC_PORT_RW) >>2)) = (uint32_t)link.port;

  }
  if(conf == DESTINATION) {

    tmp = (link.mac[0] << 24) | (link.mac[1] << 16) | (link.mac[2] << 8) | link.mac[3];
    *(pEbm + ((offset + EBM_DST_MAC_RW_1)   >>2)) =  tmp;  
    tmp = (link.mac[4] << 8) | (link.mac[5] << 0);
    *(pEbm + ((offset + EBM_DST_MAC_RW_0)   >>2)) = tmp;
    tmp = (link.ipv4[0] << 24) | (link.ipv4[1] << 16) | (link.ipv4[2] << 8) | link.ipv4[3];
    *(pEbm + ((offset + EBM_DST_IP_RW)     >>2)) =  tmp;  
    *(pEbm + ((offset + EBM_DST_PORT_RW) >>2)) = (uint32_t)link.port;


  }
  
  
  
}



void ebm_config_meta(uint32_t mtu, uint32_t hi_bits, uint32_t eb_ops)
{
  *(pEbm + (EBM_MTU_RW      >>2)) = mtu;  
  *(pEbm + (EBM_ADR_HI_RW   >>2)) = hi_bits;
  *(pEbm + (EBM_EB_OPT_RW   >>2)) = eb_ops;
}


void ebm_hi(uint32_t address)
{
    *(pEbm + (EBM_ADR_HI_RW   >>2)) = address;
    return;
}

void ebm_op(uint32_t address, uint32_t value, uint32_t optype)
{
    uint32_t offset = EBM_OFFS_DAT;
    offset += (address & EBM_ADR_MASK) + optype;
    *(pEbm + (offset>>2)) = value;
    return;
}


void ebm_clr(void)
{
  *(pEbm + (EBM_CLEAR_OWR>>2)) = 0x01;
}


void ebm_flush(void)
{
  *(pEbm + (EBM_FLUSH_OWR>>2)) = 0x01;
}

void ebm_udp(uint32_t value)
{
    *(pEbm + (EBM_UDP_DATA_OWR>>2)) = value;
}

void ebm_udp_start(void)
{
  *(pEbm + (EBM_UDP_RAW_RW>>2)) = 0x01;
}

void ebm_udp_end(void)
{
  ebm_flush();
  *(pEbm + (EBM_UDP_RAW_RW>>2)) = 0x00;
}


//String helper functions
static char* strsplit(const char* numstr, const char* delimeter)
{
	char* pch = (char*)numstr;
	
	while (*(pch) != '\0') 
		if(*(pch++) == *delimeter) return pch;		
	
 	return pch;
}
 

static unsigned char* numStrToBytes(const char*  numstr, unsigned char* bytes,  unsigned char len,  unsigned char base, const char* delimeter)
{
	char * pch;
	char * pend;
	unsigned char byteCount=0;
	long tmpconv;	
	pch = (char *) numstr;

	while ((pch != NULL) && byteCount < len )
	{					
		pend = strsplit(pch, delimeter)-1;
		tmpconv = strtol((const char *)pch, &(pend), base);
		// in case of a 16 bit value		
		if(tmpconv > 255) 	bytes[byteCount++] = (unsigned char)(tmpconv>>8 & 0xff);
		bytes[byteCount++] = (unsigned char)(tmpconv & 0xff);					
		pch = pend+1;
	}
 	return bytes;
}

static  unsigned char* addressStrToBytes(const char* addressStr, unsigned char* addressBytes, adress_type_t addtype)
  {
	unsigned char len;
	unsigned char base;
	char del;
	
	if(addtype == MAC)		
	{
		len 	  =  6;
		base 	  = 16;
		del 	  = ':';
		
	}
	else if(addtype == IP)				 
	{
		len 	  =  4;
		base 	  = 10;
		del 	  = '.';
	}
	
	else{
	 return NULL;	
	}
	
	
	return numStrToBytes(addressStr, addressBytes, len, base, &del);
	
}

static uint32_t ebm_parse_adr(eb_lm32_udp_link* link, const char* address) {
  

  char * pch;
  uint32_t stat = 1;

	//a proper address string must contain, MAC, IP and port: "hw/11:22:33:44:55:66/udp/192.168.0.1/port/60368"
	//parse and fill link struct
		
	pch = (char*) address;
	if(pch != NULL)
	{
		if(strncmp("hw", pch, 2) == 0)
		{
			pch = strsplit(pch,"/");
			if(pch != NULL)
			{
				addressStrToBytes((const char*)pch, link->mac, MAC);
				pch = strsplit(pch,"/");
				if(pch != NULL)
				{
					if(strncmp("udp", pch, 3) == 0)
					{
						pch = strsplit(pch,"/");
						if(pch != NULL)	addressStrToBytes(pch, link->ipv4, IP);
						pch = strsplit(pch,"/");
						if(pch != NULL)
						if(strncmp("port", pch, 4) == 0)
						{
							pch = strsplit(pch,"/");
							if(pch != NULL)
							{
								//addressStrToBytes(pch, link->port, PORT);
								link->port = atoi (pch);
								stat = 0;
				
							}		
						}		
					}
				}
			}
		}
	}
	
	return stat;

}

