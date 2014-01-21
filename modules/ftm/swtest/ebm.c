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

extern unsigned int* ebm;



static inline char* strsplit(const char*  numstr, const char* delimeter);
static inline unsigned char* numStrToBytes(const char*  numstr, unsigned char* bytes,  unsigned char len,  unsigned char base, const char* delimeter);
static inline unsigned char* addressStrToBytes(const char* addressStr, unsigned char* addressBytes, adress_type_t addtype);
static inline unsigned int ebm_parse_adr(struct eb_lm32_udp_link* link, const char* address);

void ebm_config_if(target_t conf, const char* con_info)
{
  struct eb_lm32_udp_link* link;
  unsigned int offset;
  
  unsigned int tmp;
  ebm_parse_adr(link, con_info);
  
  if(conf == LOCAL) offset = EBM_OFFS_LOCAL;
  if(conf == REMOTE) offset = EBM_OFFS_REMOTE;
  
  tmp = (link->mac[0] << 24) | (link->mac[1] << 16) | (link->mac[2] << 8) | link->mac[3];
  *(ebm + ((offset + EBM_OFFS_MAC_HI)   >>2)) =  tmp;  
  
  tmp = (link->mac[4] << 24) | (link->mac[5] << 16);
  *(ebm + ((offset + EBM_OFFS_MAC_LO)   >>2)) = tmp;
  
  tmp = (link->ipv4[0] << 24) | (link->ipv4[1] << 16) | (link->ipv4[2] << 8) | link->ipv4[3];
  *(ebm + ((offset + EBM_OFFS_IPV4)     >>2)) =  tmp;  
  
  *(ebm + ((offset + EBM_OFFS_UDP_PORT) >>2)) = (unsigned int)link->port;
  
}

void ebm_config_meta(unsigned int pac_len, unsigned int hi_bits, unsigned int max_ops, unsigned int eb_ops)
{
  *(ebm + (EBM_REG_PAC_LEN  >>2)) = pac_len;  
  *(ebm + (EBM_REG_OPA_HI   >>2)) = hi_bits;
  *(ebm + (EBM_REG_OPS_MAX  >>2)) = max_ops;
  *(ebm + (EBM_REG_EB_OPT   >>2)) = eb_ops;
}


void ebm_op(unsigned int address, unsigned int value, unsigned int optype)
{
    unsigned int offset = EBM_OFFS_DAT;
    //set hibits according to desired address
    //*(ebm + (EBM_REG_OPA_HI >> 2)) = (address & ~ADR_MASK);
    
    offset += optype;
     offset += (address & ADR_MASK);
    *(ebm + (offset>>2)) = value;
    return;
}

void ebm_flush(void)
{
  *(ebm + (EBM_REG_FLUSH>>2)) = 0x01;
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

static unsigned int ebm_parse_adr(struct eb_lm32_udp_link* link, const char* address) {
  

  char * pch;
  unsigned int stat = 1;

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

