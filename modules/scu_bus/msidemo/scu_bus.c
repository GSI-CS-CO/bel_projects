#include "scu_bus.h"
#include "display.h"


char* mat_sprinthex(char* buffer, unsigned short val)
{
  unsigned char i,ascii;
  const unsigned short mask = 0x000F;

  for(i=0; i<4;i++)
  {
    ascii= (val>>(i<<2)) & mask;
    if(ascii > 9) ascii = ascii - 10 + 'A';
    else        ascii = ascii      + '0';
    buffer[4-i] = ascii;
  }

  buffer[4] = 0x00;
  return buffer;
}

/*  for every found slave the slotnumber is added to the slave array
    e.g. [2,3] means slaves in slot 2 and 3
*/
void probe_scu_bus(volatile unsigned short* bus_addr, unsigned short system_addr, unsigned short group_addr, int* slaves) {
  int slot;
  unsigned short cid_sys, cid_group;
  char buffer[5];
  for (slot = 1; slot <= SCU_BUS_MAX_SLOTS; slot++) {
    cid_sys = bus_addr[(slot<<16) + CID_SYS];     //CID system addr from slave
    cid_group = bus_addr[(slot<<16) + CID_GROUP]; //CID group addr from slave
    if (cid_sys == system_addr && cid_group == group_addr) 
      *(slaves++) = slot;  
  }
  *slaves = 0; // end of device list 
} 
