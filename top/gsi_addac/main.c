#include <stdint.h>

#include <inttypes.h>
#include <stdarg.h>

#include "board.h"
#include "uart.h"
#include "ownet.h"
#include "findtype.h"
#include "temp42.h"
#include "eep43.h"

#define MAXDEVICES 10 
#define ONEWIRE_PORT 0 

void _irq_entry(void) {}
void DisplaySerialNum(uchar sn[8]);
int NumDevices = 0;
int portnum = 0;
uchar FamilySN[MAXDEVICES][8];
volatile unsigned short* scu_reg = (unsigned short*)BASE_SCU_REG;


void msDelay(int msecs) {
	int i;
	for(i = msecs * CPU_CLOCK/4; i > 0; i--)
		asm("# noop");
}

void ReadTempDevice() {
  int i, j, cnt = 0;
	int current_temp;
	int c_frac;
  SMALLINT didRead = 0;
  short rawtemp;
  
  if (NumDevices) { //we have found devices
    mprintf("\r\n");
    // read the temperature and print serial number and temperature
    for (i = NumDevices; i; i--) {
      mprintf("(%d) ", j++);
      DisplaySerialNum(FamilySN[i-1]);
	  	owLevel(portnum, MODE_NORMAL); 
		
	    if (FamilySN[i-1][0] == 0x42) {
        scu_reg[0] = (FamilySN[i-1][7] << 8) | FamilySN[i-1][6];
        scu_reg[1] = (FamilySN[i-1][5] << 8) | FamilySN[i-1][4];
        scu_reg[2] = (FamilySN[i-1][3] << 8) | FamilySN[i-1][2];
        scu_reg[3] = (FamilySN[i-1][1] << 8) | FamilySN[i-1][0];
        didRead = ReadTemperature42(portnum, FamilySN[i-1],&current_temp,&c_frac,&rawtemp);
	    }
      if (didRead) {
        scu_reg[4] = rawtemp;
        mprintf(" %d",current_temp);
		    if (c_frac)
			    mprintf(".5");
		    else
			    mprintf(".0");
		    mprintf(" deegree celsius\r\n");
	    } else {
        mprintf("  Convert failed.  Device is");
        if(!owVerify(portnum, FALSE))
          mprintf(" not");
        mprintf(" present.\r\n");
      }
    }
  } else
      mprintf("No temperature devices found!\r\n");

}

void init() {

	owInit();
	uart_init();
	uart_write_string("Debug Port\n");
 
  // Find the device(s)
  // on the ADDAC card are two ow ports
  NumDevices  = 0;
  NumDevices += FindDevices(portnum, &FamilySN[NumDevices], 0x42, MAXDEVICES-NumDevices);
  NumDevices += FindDevices(portnum, &FamilySN[NumDevices], 0x20, MAXDEVICES-NumDevices);
  NumDevices += FindDevices(portnum, &FamilySN[NumDevices], 0x43, MAXDEVICES-NumDevices);
  ReadTempDevice(); 
	
} //end of init()

// -------------------------------------------------------------------------------
// Read and print the serial number.
//
void DisplaySerialNum(uchar sn[8])
{
   int i;
   for (i = 7; i>=1; i--)
     mprintf("%x:", (int)sn[i]);
   mprintf("%x", (int)sn[0]);
	
}

int main(void)
{
	init();
	
	while(1) {
    ReadTempDevice(); 
	}
}
