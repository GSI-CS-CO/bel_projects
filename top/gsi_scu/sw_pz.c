#include <stdint.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "display.h"
#include "irq.h"
#include "scu_bus.h"
#include "aux.h"
#include "mini_sdb.h"
#include "board.h"
#include "uart.h"
#include "scu_mil.h"
#include "fg.h"

#define SHARED __attribute__((section(".shared")))
volatile unsigned short* scub_base;
volatile unsigned int* BASE_UART;
volatile unsigned int* scu_mil_base;
volatile unsigned int* wb_fg_base;
uint64_t SHARED backplane_id = -1;
struct scu_bus SHARED scub;

int slaves[MAX_SCU_SLAVES+1] = {0};


void usleep(int us)
{

  unsigned i;
  unsigned long long delay = us;
  /* prevent arithmetic overflow */
  delay *= CPU_CLOCK;
  delay /= 1000000;
  delay /= 4; // instructions per loop
  for (i = delay; i > 0; i--) asm("# noop");
}  

void msDelay(int msecs) {
  usleep(1000 * msecs);
}


void show_msi()
{
  char buffer[12];
  
  mat_sprinthex(buffer, global_msi.msg);
  disp_put_str("D ");
  disp_put_str(buffer);
  disp_put_c('\n');

  
  mat_sprinthex(buffer, global_msi.adr);
  disp_put_str("A ");
  disp_put_str(buffer);
  disp_put_c('\n');

  
  mat_sprinthex(buffer, (unsigned long)global_msi.sel);
  disp_put_str("S ");
  disp_put_str(buffer);
  disp_put_c('\n');
}

void init() {
  uart_init_hw();           //enables the uart for debug messages
//  scan_scu_bus(&scub, backplane_id, scub_base);
} 



int main(void) {
  discoverPeriphery();  
  scub_base     = (unsigned short*)find_device_adr(GSI, SCU_BUS_MASTER);
  scu_mil_base  = (unsigned int*)find_device(SCU_MIL);
  wb_fg_base    = (unsigned int*)find_device_adr(GSI, WB_FG_QUAD);



  unsigned short ifc_adr           = 0x0001;           // Interface Karten Adresse
  unsigned short slave_nr          = 0x0005;           // Slave-Adresse
  unsigned short tag1_base         = 0x0580;           // Base-ADR Tag1
  unsigned short tag2_base         = 0x0590;           // Base-ADR Tag2
  unsigned short tag3_base         = 0x05a0;           // Base-ADR Tag3
  unsigned short tad4_base         = 0x05b0;           // Base-ADR Tag4
  unsigned short tag5_base         = 0x05c0;           // Base-ADR Tag5
  unsigned short tag6_base         = 0x05d0;           // Base-ADR Tag6
  unsigned short tag7_base         = 0x05e0;           // Base-ADR Tag7
  unsigned short tag8_base         = 0x05f0;           // Base-ADR Tag8
  unsigned short slave_base_adr    = 0x5000;           // Basisadr. des Slaves


  unsigned short rw_tag0_base_adr  = (slave_base_adr + 0x0280);  // base-adr tag0 array
  unsigned short rw_tag1_base_adr  = (slave_base_adr + 0x0290);  // base-adr tag1 array

  unsigned short wr_scub_adr  = 0x11 << 8; // place function code wr scub-adr  (0x11) to high byte; low byte holds ifc-card-address
  unsigned short wr_scub_data = 0x10 << 8; // place function code wr scub-data (0x10) to high byte; low byte holds ifc-card-address
  unsigned short rd_scub_data = 0x90 << 8; // place function code rd scub-data (0x90) to high byte; low byte holds ifc-card-address  

 
  disp_reset();
  disp_put_c('\f');
  disp_put_str("Tag1-Dec-15");
  disp_put_str("   07:40   ");


  
  usleep(1500000); //wait for powerup of the slave cardsrootroo
  init(); 

  mprintf("\n");
  mprintf("Tag1-Test\n");
  mprintf("\n");

  mprintf("scub_base is: 0x%x\n", scub_base);
  mprintf("mil_base  is: 0x%x\n", scu_mil_base);

//  -------------- Config: Alles auf Output ----------------------------------
  scub_base[( slave_nr<< 16) + 0x024 ] = 0x0001; //Reset Powerup
  scub_base[( slave_nr<< 16) + 0x500 ] = 0x007f; //set Output


//  ----------------------- Init Tag1 ----------------------------------
  scub_base[( slave_nr<< 16) + tag1_base + 0x0 ] = 0x00f1; //ID-HW
  scub_base[( slave_nr<< 16) + tag1_base + 0x1 ] = 0x00f1; //ID-LW
  scub_base[( slave_nr<< 16) + tag1_base + 0x2 ] = 0x8000; //D[15..0] Maske
  scub_base[( slave_nr<< 16) + tag1_base + 0x3 ] = 0x8002; //D15=Output-Pegel, D[3..0] Register-Nr.
  scub_base[( slave_nr<< 16) + tag1_base + 0x4 ] = 0x0000; //Delay-Count Verzögerung
  scub_base[( slave_nr<< 16) + tag1_base + 0x5 ] = 0x0000; //Puls-With "Monoflopp"
  scub_base[( slave_nr<< 16) + tag1_base + 0x6 ] = 0x0000; //Prescale: D[15..8] Verz, D[7..0] Puls
  scub_base[( slave_nr<< 16) + tag1_base + 0x7 ] = 0x0000; //Trigger: 

//  ----------------------- Init Tag1 ----------------------------------
  scub_base[( slave_nr<< 16) + tag2_base + 0x0 ] = 0x00f8; //ID-HW
  scub_base[( slave_nr<< 16) + tag2_base + 0x1 ] = 0x00f8; //ID-LW
  scub_base[( slave_nr<< 16) + tag2_base + 0x2 ] = 0x8000; //D[15..0] Maske
  scub_base[( slave_nr<< 16) + tag2_base + 0x3 ] = 0x0002; //D15=Output-Pegel, D[3..0] Register-Nr.
  scub_base[( slave_nr<< 16) + tag2_base + 0x4 ] = 0x0000; //Delay-Count Verzögerung
  scub_base[( slave_nr<< 16) + tag2_base + 0x5 ] = 0x0000; //Puls-With "Monoflopp"
  scub_base[( slave_nr<< 16) + tag2_base + 0x6 ] = 0x0000; //Prescale: D[15..8] Verz, D[7..0] Puls
  scub_base[( slave_nr<< 16) + tag2_base + 0x7 ] = 0x0000; //Trigger: 

//  ----------------------- Init Tag1 ----------------------------------
  scub_base[( slave_nr<< 16) + tag3_base + 0x0 ] = 0x00fa; //ID-HW
  scub_base[( slave_nr<< 16) + tag3_base + 0x1 ] = 0x00fa; //ID-LW
  scub_base[( slave_nr<< 16) + tag3_base + 0x2 ] = 0x8000; //D[15..0] Maske
  scub_base[( slave_nr<< 16) + tag3_base + 0x3 ] = 0x8002; //D15=Output-Pegel, D[3..0] Register-Nr.
  scub_base[( slave_nr<< 16) + tag3_base + 0x4 ] = 0x0000; //Delay-Count Verzögerung
  scub_base[( slave_nr<< 16) + tag3_base + 0x5 ] = 0x0032; //Puls-With "Monoflopp"
  scub_base[( slave_nr<< 16) + tag3_base + 0x6 ] = 0x0000; //Prescale: D[15..8] Verz, D[7..0] Puls
  scub_base[( slave_nr<< 16) + tag3_base + 0x7 ] = 0x0000; //Trigger: 



//  ------------- Loop --------------------------

//  while(1) {

//  usleep(1);
//  scub_base[(2 << 16) + 0x210] = 0x1000; // write AWOut1
//    scub_base[(slave_nr << 16) + 0x210] = 0x2000; // write AWOut1
//  scub_base[(slave_nr << 16) + 0x211] = 0x2001; // write AWOut1

//  }


  return(0);
}
