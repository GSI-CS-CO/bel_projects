#include <stdint.h>
#include <stdio.h>
#include "board.h"

#define   R_CSR  0x0
#define   R_CDR  0x4

#define   CSR_DAT_MSK  (1<<0)
#define   CSR_RST_MSK  (1<<1)
#define   CSR_OVD_MSK  (1<<2)
#define   CSR_CYC_MSK  (1<<3)
#define   CSR_PWR_MSK  (1<<4)
#define   CSR_IRQ_MSK  (1<<6)
#define   CSR_IEN_MSK  (1<<7)
#define   CSR_SEL_OFS  8
#define   CSR_SEL_MSK  (0xF<<8)
#define   CSR_POWER_OFS  16
#define   CSR_POWER_MSK  (0xFFFF<<16)
#define   CDR_NOR_MSK  (0xFFFF<<0)
#define   CDR_OVD_OFS  16
#define   CDR_OVD_MSK  (0xFFFF<<16)


#define CLK_DIV_NOR CPU_CLOCK/357     //clock divider for normal mode
#define CLK_DIV_OVD CPU_CLOCK/1008    //clock divider for overdrive mode

static inline void ow_writel(uint32_t reg, uint32_t data)
{
  *(volatile uint32_t *) (BASE_ONEWIRE + reg) = data;
}

static inline uint32_t ow_readl(uint32_t reg)
{
  return *(volatile uint32_t *)(BASE_ONEWIRE + reg);
}

void ow_init()
{
  //set clock dividers for normal and overdrive mode
  ow_writel( R_CDR, ((CLK_DIV_NOR & CDR_NOR_MSK) | (( CLK_DIV_OVD << CDR_OVD_OFS) & CDR_OVD_MSK)) );
}

static uint32_t ow_reset(uint32_t port)
{
  uint32_t reg, data;
  
  data = (  (port<<CSR_SEL_OFS) & CSR_SEL_MSK) |
             CSR_CYC_MSK | CSR_RST_MSK;   //start cycle, rst pulse request
  ow_writel(R_CSR, data);
  //wait until cycle done
  while(ow_readl(R_CSR) & CSR_CYC_MSK);
  reg = ow_readl(R_CSR);
  return ~reg & CSR_DAT_MSK;
}

static uint32_t slot(uint32_t port, uint32_t bit)
{
  uint32_t data, reg;
  
  data = (  (port<<CSR_SEL_OFS) & CSR_SEL_MSK) | 
            CSR_CYC_MSK | (bit & CSR_DAT_MSK);  //start cycle, write bit
  ow_writel(R_CSR, data);
  //wait until cycle done
  while( ow_readl(R_CSR) & CSR_CYC_MSK );
  reg = ow_readl(R_CSR);
  return reg & CSR_DAT_MSK;
}

static uint32_t ow_read_bit(uint32_t port) { return slot(port, 0x1); }
static uint32_t ow_write_bit(uint32_t port, uint32_t bit) { return slot(port, bit); }


uint8_t ow_read_byte(uint32_t port) 
{
  uint32_t data = 0, i;
  
  for(i=0;i<8;i++)
    data |= ow_read_bit(port) << i;
  return (uint8_t)data;
}

int8_t ow_write_byte(uint32_t port, uint32_t byte)
{
  uint32_t  data = 0;
  uint8_t   i;
  uint32_t  byte_old = byte;
  
  for (i=0;i<8;i++)
  {
    data |= ow_write_bit(port, (byte & 0x1)) << i;
    byte >>= 1;
  }
  return byte_old == data ? 0 : -1;
}

int ow_write_block(int port, uint8_t *block, int len)
{
  uint32_t i;

  for(i=0;i<len;i++)
    *block++ = ow_write_byte(port, *block);

  return 0;
}

int ow_read_block(int port, uint8_t *block, int len)
{
  uint32_t i;

  for(i=0;i<len;i++)
    *block++ = ow_read_byte(port);

  return 0;
}

#define    ROM_SEARCH  0xF0
#define    ROM_READ  0x33
#define    ROM_MATCH  0x55
#define    ROM_SKIP  0xCC
#define    ROM_ALARM_SEARCH  0xEC

#define    CONVERT_TEMP  0x44
#define    WRITE_SCRATCHPAD  0x4E
#define    READ_SCRATCHPAD  0xBE
#define    COPY_SCRATCHPAD  0x48
#define    RECALL_EEPROM  0xB8
#define    READ_POWER_SUPPLY  0xB4

static uint8_t ds18x_id [8];


int8_t ds18x_read_serial(uint8_t *id)
{
  uint8_t i;
  
  if(!ow_reset(0))
    return -1;
  
  if(ow_write_byte(0, ROM_READ) < 0)
    return -1;
  for(i=0;i<8;i++)
  {
    *id = ow_read_byte(0);
    id++;
  }
  
  return 0;
}

static int8_t ds18x_access(uint8_t *id)
{
  int i;
  if(!ow_reset(0))
    return -1;
  
  if(ow_write_byte(0, ROM_MATCH) < 0)
    return -1;
  for(i=0;i<8;i++)
    if(ow_write_byte(0, id[i]) < 0)
  	  return -1;

  return 0;
}

int8_t ds18x_read_temp(uint8_t *id, int *temp_r)
{
  int i, temp;
  uint8_t data[9];
  
  if(ds18x_access(id) < 0)
    return -1;
mprintf("found.");
  ow_write_byte(0, READ_SCRATCHPAD);
  
  for(i=0;i<9;i++) data[i] = ow_read_byte(0);
  
  temp = ((int)data[1] << 8) | ((int)data[0]);
  if(temp & 0x1000)
    temp = -0x10000 + temp;
  
  ds18x_access(id);
  ow_write_byte(0, CONVERT_TEMP);
  
  if(temp_r) *temp_r = temp;
  return 0;
}

int ds18x_init()
{
	ow_init();
	if(ds18x_read_serial(ds18x_id) < 0)
		return -1;

	return ds18x_read_temp(ds18x_id, NULL);
}
