#include <stdio.h>
#include "display.h"
#include "irq.h"
#include "scu_bus.h"

volatile unsigned int* display            = (unsigned int*)0x02900000;
volatile unsigned int* irq_slave          = (unsigned int*)0x02000d00;
volatile unsigned short* scu_bus_master   = (unsigned short*)0x02400000;
int slaves[SCU_BUS_MAX_SLOTS+1] = {0};


void show_msi()
{
  char buffer[12];
  
  mat_sprinthex(buffer, global_msi.msg);
  disp_put_str("D ");
  disp_put_str(buffer);
  disp_put_c('\n');

  
  mat_sprinthex(buffer, global_msi.src);
  disp_put_str("A ");
  disp_put_str(buffer);
  disp_put_c('\n');

  
  mat_sprinthex(buffer, (unsigned long)global_msi.sel);
  disp_put_str("S ");
  disp_put_str(buffer);
  disp_put_c('\n');
}

void isr1()
{
  unsigned int i;
  
  disp_put_str("ISR1\n");
  //which slave has triggered?
  while(slaves[i]) {
    if (scu_bus_master[SRQ_ACT] & (1 << (slaves[i]-1))) {
      //acknowledge powerup irq
      if (scu_bus_master[(slaves[i] << 16) + SLAVE_INT_ACT] & 1)
        scu_bus_master[(slaves[i] << 16) + SLAVE_INT_ACT] |= 1;
      //ack dreq
      else if (scu_bus_master[(slaves[i] << 16) + SLAVE_INT_ACT] & 2) {
        scu_bus_master[(slaves[i] << 16) + FG_QUAD_BASE + FG_QUAD_B] = 0xFF;
        scu_bus_master[(slaves[i] << 16) + SLAVE_INT_ACT] |= 2;
      }
    }
    i++;
  } 

}

void _irq_entry(void) {
  
  disp_put_c('\f');
  disp_put_str("IRQ_ENTRY\n");
  irq_process();
 
}

//const char mytext[] = "Hallo Welt!...\n\n";

void main(void) {
  int i = 0;
  //char buffer[14];

  
  disp_reset();
  disp_put_c('\f');
  probe_scu_bus(scu_bus_master,55,3,slaves); //probe for ADDAC cards
  
  if (!slaves[0]) {
    disp_put_c('\f');
    disp_put_str("no slaves\n");
    while(1);
  } 
  
  scu_bus_master[SRQ_ENA] = 0x0; //reset bitmask  
  while(slaves[i]) {
    disp_put_c('x');
    scu_bus_master[SRQ_ENA] |= (1 << (slaves[i]-1));  //enable irqs for the slave
    scu_bus_master[(slaves[i] << 16) + SLAVE_INT_ENA] = 0x2; //enable dreq from fg
    i++;
  } 
  //SCU Bus Master
  //enable slave irqs
  scu_bus_master[GLOBAL_IRQ_ENA] = 0x20;

  isr_table_clr();
//  isr_ptr_table[0]= isr0;
  isr_ptr_table[1]= isr1;  
  irq_set_mask(0x02);
  irq_enable();

  scu_bus_master[(slaves[i] << 16) + FG_QUAD_BASE + FG_QUAD_B] = 0xFF;
  scu_bus_master[(slaves[i] << 16) + FG_QUAD_BASE + FG_QUAD_BROAD] = 0x4711; // start

  
 // disp_reset();	
 // disp_put_str(mytext);
  while(1);
}
