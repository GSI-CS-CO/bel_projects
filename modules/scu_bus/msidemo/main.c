#include <stdio.h>
#include "display.h"
#include "irq.h"
#include "scu_bus.h"

extern unsigned int* _startshared[];
extern unsigned int* _endshared[];
volatile unsigned int* fesa_if = (unsigned int*)_startshared;

volatile unsigned int* display            = (unsigned int*)0x02900000;
volatile unsigned int* irq_slave          = (unsigned int*)0x02000d00;
volatile unsigned short* scu_bus_master   = (unsigned short*)0x02400000;
int slaves[SCU_BUS_MAX_SLOTS+1] = {0};


struct pset {
  int a;
  int l_a;
  int b;
  int l_b;
  int c;
  int n;
};


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
  struct pset *p;
  unsigned int i;
  p = (struct p *)fesa_if;

  //which slave has triggered?
  while(slaves[i]) {
    //check bit in master act reg
    if (scu_bus_master[SRQ_ACT] & (1 << (slaves[i]-1))) {
      //acknowledge powerup irq
      if (scu_bus_master[(slaves[i] << 16) + SLAVE_INT_ACT] & 1)
        scu_bus_master[(slaves[i] << 16) + SLAVE_INT_ACT] |= 1;
      //ack dreq
      if (scu_bus_master[(slaves[i] << 16) + SLAVE_INT_ACT] & 2) {
        if (p->a != 0xdeadbeef) { //no more data
          scu_bus_master[(slaves[i] << 16) + FG_QUAD_BASE + FG_QUAD_A]      = p->a;
          scu_bus_master[(slaves[i] << 16) + FG_QUAD_BASE + FG_QUAD_SHIFTA] = 64 - 24 - p->l_a * -1;
          scu_bus_master[(slaves[i] << 16) + FG_QUAD_BASE + FG_QUAD_B]      = p->b;
          scu_bus_master[(slaves[i] << 16) + FG_QUAD_BASE + FG_QUAD_SHIFTB] = 64 - 24 - p->l_b * -1;
          scu_bus_master[(slaves[i] << 16) + FG_QUAD_BASE + FG_QUAD_L]      = p->c;
          scu_bus_master[(slaves[i] << 16) + FG_QUAD_BASE + FG_QUAD_H]      = p->c >> 16;
          scu_bus_master[(slaves[i] << 16) + SLAVE_INT_ACT]                 |= 2; //ack slave dreq
        }
      }
    }
    i++;
  } 
  fesa_if += 6;
}

void _irq_entry(void) {
  
  //disp_put_c('\f');
  //disp_put_str("IRQ_ENTRY\n");
  irq_process();
 
}

//const char mytext[] = "Hallo Welt!...\n\n";

int main(void) {
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
  scu_bus_master[MULTI_SLAVE_SEL] = 0x0; //reset bitmask  
  while(slaves[i]) {
    disp_put_c('x');
    scu_bus_master[SRQ_ENA] |= (1 << (slaves[i]-1));  //enable irqs for the slave
    scu_bus_master[MULTI_SLAVE_SEL] |= (1 << (slaves[i]-1)); //set bitmask for broadcast select
    scu_bus_master[(slaves[i] << 16) + SLAVE_INT_ENA] = 0x2; //enable dreq from fg
    i++;
  } 
  //SCU Bus Master
  //enable slave irqs
  scu_bus_master[GLOBAL_IRQ_ENA] = 0x20;

  isr_table_clr();
  isr_ptr_table[1]= isr1;  
  irq_set_mask(0x02);
  irq_enable();
  
  //config of DAC and FG
  i=0;
  while(slaves[i]) {
    
    scu_bus_master[(slaves[i] << 16) + DAC2_BASE + DAC_CNTRL] = 0x10; //set FG mode
    scu_bus_master[(slaves[i] << 16) + SLAVE_INT_ACT] |= 1;
    scu_bus_master[(slaves[i] << 16) + SLAVE_INT_ACT] |= 2;
    
    scu_bus_master[(slaves[i] << 16) + FG_QUAD_BASE + FG_QUAD_CNTRL] = 0x1; //reset fg
    scu_bus_master[(slaves[i] << 16) + FG_QUAD_BASE + FG_QUAD_CNTRL] = (5 << 13); //set frequency Bit 15..13
    scu_bus_master[(slaves[i] << 16) + FG_QUAD_BASE + FG_QUAD_CNTRL] |= (2 << 10); //set step count Bit 12..10
    scu_bus_master[(slaves[i] << 16) + FG_QUAD_BASE + FG_QUAD_A] = 0x0;
    //scu_bus_master[(slaves[i] << 16) + FG_QUAD_BASE + FG_QUAD_SHIFTA] = 0x20;
    //scu_bus_master[(slaves[i] << 16) + FG_QUAD_BASE + FG_QUAD_SHIFTB] = 0x20;
    scu_bus_master[(slaves[i] << 16) + FG_QUAD_BASE + FG_QUAD_BROAD] = 0x4711; // start signal to all fg slaves
    i++;
  }
  
 // disp_reset();	
 // disp_put_str(mytext);
  while(1);

  return(0);
}
