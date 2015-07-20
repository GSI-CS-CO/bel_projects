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

volatile unsigned short* scub_base;
volatile unsigned int* BASE_UART;
volatile unsigned int* scu_mil_base;



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
} 



int main(void) {
  discoverPeriphery();  
  scub_base     = (unsigned short*)find_device_adr(GSI, SCU_BUS_MASTER);
  scu_mil_base = (unsigned int*)find_device(SCU_MIL);



  unsigned short ifc_adr           = 0x0001;           // Interface Karten Adresse
  unsigned short slave_base_adr    = 0x5000;           // Basisadr. des Slaves


  unsigned short rw_tag0_base_adr  = (slave_base_adr + 0x0280);  // base-adr tag0 array
  unsigned short rw_tag1_base_adr  = (slave_base_adr + 0x0290);  // base-adr tag1 array

  unsigned short wr_scub_adr  = 0x11 << 8; // place function code wr scub-adr  (0x11) to high byte; low byte holds ifc-card-address
  unsigned short wr_scub_data = 0x10 << 8; // place function code wr scub-data (0x10) to high byte; low byte holds ifc-card-address
  unsigned short rd_scub_data = 0x90 << 8; // place function code rd scub-data (0x90) to high byte; low byte holds ifc-card-address  



//  unsigned short ifc_adr           = 0x0001;           // Interface Karten Adresse



 
  disp_reset();
  disp_put_c('\f');
  disp_put_str("Tag1-Oct-01");
  disp_put_str("0304,  10us");
  disp_put_str("0305,  10us");
  disp_put_str("0501,  10us");
  disp_put_str("0502,  10us");
  disp_put_str("0503,  20us");


  
  usleep(1500000); //wait for powerup of the slave cardsrootroo
  init(); 

  mprintf("\n");
  mprintf("Tag1-Test\n");
  mprintf("\n");

  mprintf("scub_base is: 0x%x\n", scub_base);
  mprintf("mil_base  is: 0x%x\n", scu_mil_base);

/*
  mprintf("ifc_adr   is: 0x%x\n", ifc_adr);
  mprintf("slave_adr is: 0x%x\n", slave_base_adr);

  //run_mil_test(scu_mil_base, 0x1);


 //--------------------------------- Reset Poer-Up ---------------------------------------------
    write_mil(scu_mil_base, (slave_base_adr + 0x0024), (wr_scub_adr  |= ifc_adr)); // Set Adr.
    write_mil(scu_mil_base, 0x0001,                    (wr_scub_data |= ifc_adr)); // D0 = 1 ==> Reset PowerUp

 //----------------------------------- init Tag0 ---------------------------------------------
    write_mil(scu_mil_base, (rw_tag0_base_adr + 0x0000), (wr_scub_adr  |= ifc_adr));
    write_mil(scu_mil_base, 0x0304,                      (wr_scub_data |= ifc_adr)); // Tag0: H-Word
    write_mil(scu_mil_base, (rw_tag0_base_adr + 0x0001), (wr_scub_adr  |= ifc_adr));
    write_mil(scu_mil_base, 0x0304,                      (wr_scub_data |= ifc_adr)); // Tag0: L-Word
    write_mil(scu_mil_base, (rw_tag0_base_adr + 0x0002), (wr_scub_adr  |= ifc_adr));
    write_mil(scu_mil_base, 0x0080,                      (wr_scub_data |= ifc_adr)); // Tag0: Maske
    write_mil(scu_mil_base, (rw_tag0_base_adr + 0x0003), (wr_scub_adr  |= ifc_adr));
    write_mil(scu_mil_base, 0x8003,                      (wr_scub_data |= ifc_adr)); // Tag0: D15=Level, D[3..0] = Reg.-Nr.

 //----------------------------------- init Tag1 ---------------------------------------------

    write_mil(scu_mil_base, (rw_tag1_base_adr + 0x0000), (wr_scub_adr  |= ifc_adr));
    write_mil(scu_mil_base, 0x0305,                      (wr_scub_data |= ifc_adr)); // Tag1: H-Word
    write_mil(scu_mil_base, (rw_tag1_base_adr + 0x0001), (wr_scub_adr  |= ifc_adr));
    write_mil(scu_mil_base, 0x0305,                      (wr_scub_data |= ifc_adr)); // Tag1: L-Word
    write_mil(scu_mil_base, (rw_tag1_base_adr + 0x0002), (wr_scub_adr  |= ifc_adr));
    write_mil(scu_mil_base, 0x0080,                      (wr_scub_data |= ifc_adr)); // Tag1: Maske
    write_mil(scu_mil_base, (rw_tag1_base_adr + 0x0003), (wr_scub_adr  |= ifc_adr));
    write_mil(scu_mil_base, 0x0003,                      (wr_scub_data |= ifc_adr)); // Tag1: D15=Level, D[3..0] = Reg.-Nr.

*/

//  ------------- PZ-Loop --------------------------

  while(1) {

    write_mil(scu_mil_base, 0x0000, 0x0304);
    usleep(10);
    write_mil(scu_mil_base, 0x0000, 0x0305);
    usleep(10);
    write_mil(scu_mil_base, 0x0000, 0x0501);
    usleep(10);
    write_mil(scu_mil_base, 0x0000, 0x0502);
    usleep(10);
    write_mil(scu_mil_base, 0x0000, 0x0503);
    usleep(20);
  }

  return(0);
}
