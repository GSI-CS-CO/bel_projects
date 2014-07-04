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
#include "w1.h"
#include "fg.h"
#include "cb.h"

//#define DEBUG
//#define FGDEBUG
//#define CBDEBUG

extern struct w1_bus wrpc_w1_bus;

#define SHARED __attribute__((section(".shared")))
uint64_t SHARED board_id = -1;
uint16_t SHARED board_temp = -1;
uint64_t SHARED ext_id = -1;
uint16_t SHARED ext_temp = -1; 
uint64_t SHARED backplane_id = -1;
uint16_t SHARED backplane_temp = -1;
uint32_t SHARED fg_magic_number = 0xdeadbeef;
uint32_t SHARED fg_version = 0x1;
struct circ_buffer SHARED fg_buffer[MAX_FG_DEVICES]; 
struct scu_bus SHARED scub;
struct fg_list SHARED fgs;
volatile uint32_t SHARED fg_control;

volatile unsigned short* scub_base;
volatile unsigned int* BASE_ONEWIRE;
volatile unsigned int* BASE_UART;
volatile unsigned int* scub_irq_base;
sdb_location lm32_irq_endp[10]; // there are two queues for msis

volatile unsigned short icounter[MAX_SCU_SLAVES+1];
volatile int initialized[MAX_SCU_SLAVES] = {0};

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

void slave_irq_handler()
{
  int j = 0;
  char buffer[12];
  struct pset *p;
  unsigned char slave_nr = global_msi.adr>>2 & 0xf;
  volatile unsigned short tmr_irq_cnts; 
  static unsigned short old_tmr_cnt[MAX_SCU_SLAVES];
  volatile unsigned int slv_int_act_reg = scub_base[(slave_nr << 16) + SLAVE_INT_ACT];
  unsigned int slave_acks = 0;
  struct fg_dev* fg1 = 0;
  struct fg_dev* fg2 = 0;


  if (slave_nr < 0 || slave_nr > MAX_SCU_SLAVES) {
    mprintf("unknown IRQ number.\n");
    return;
  }

  j = 0;
  while(scub.slaves[slave_nr-1].devs[j].version) { /* more fgs in list */
    if (scub.slaves[slave_nr-1].devs[j].dev_number == 0) 
      fg1 = &scub.slaves[slave_nr-1].devs[j];
    if (scub.slaves[slave_nr-1].devs[j].dev_number == 1) 
      fg2 = &scub.slaves[slave_nr-1].devs[j];

    j++; 
  } 

  if (slv_int_act_reg & 0x1) { //powerup irq?
    mprintf("ack powerup irq in slave %d\n", slave_nr);
    slave_acks |= 0x1; //ack powerup irq
  }

  if (slv_int_act_reg & 0x2000) { //tmr irq?
    tmr_irq_cnts = scub_base[(slave_nr << 16) + TMR_BASE + TMR_IRQ_CNT];
    // init old_tmr_cnt
    if (!initialized[slave_nr-1]) {
      old_tmr_cnt[slave_nr-1] = tmr_irq_cnts - 1;
      mprintf("init slave: %d with %x\n", slave_nr, tmr_irq_cnts - 1); 
      initialized[slave_nr-1] = 1;
    }  
    // check for lost IRQs
    if ((tmr_irq_cnts == (unsigned short)(old_tmr_cnt[slave_nr-1] + 1))) {
      slave_acks |= (1 << 13); //ack timer irq
      old_tmr_cnt[slave_nr-1] = tmr_irq_cnts;
    } else
      mprintf("irq1 slave: %d, old: %x, act: %x\n", slave_nr, old_tmr_cnt[slave_nr-1], tmr_irq_cnts);
  }
  
  if (slv_int_act_reg & (1<<15)) { //FG1 irq?
    scub_base[(slave_nr << 16) + FG1_BASE + FG_A] = 0x1;
    slave_acks |= (1<<15); //ack FG1 irq
  } 
 
  if (slv_int_act_reg & (1<<14)) { //FG2 irq?
    scub_base[(slave_nr << 16) + FG2_BASE + FG_A] = 0x1;
    slave_acks |= (1<<14); //ack FG2 irq
  } 

  scub_base[(slave_nr << 16) + SLAVE_INT_ACT] = slave_acks; // ack all pending irqs 
}



void enable_msi_irqs() {
  int i;
  //SCU Bus Master
  scub_base[GLOBAL_IRQ_ENA] = 0x20; //enable slave irqs
  scub_irq_base[0] = 0x1; // reset irq master
  for (i = 0; i < 12; i++) {
    scub_irq_base[8] = i;  //channel select
    scub_irq_base[9] = 0x08154711;  //msg
    scub_irq_base[10] = getSdbAdr(&lm32_irq_endp[4]) + ((i+1) << 2); //destination address, do not use lower 2 bits 
  }
  i = 0;
  while(scub.slaves[i].unique_id) {
    initialized[scub.slaves[i].slot - 1] = 0; // counter needs to be resynced
    scub_irq_base[2] |= (1 << (scub.slaves[i].slot - 1)); //enable slaves
    i++;
  }
  mprintf("IRQs for all slave channels enabled.\n");
}


void disable_msi_irqs() {
  scub_irq_base[3] = 0xfff; //disable irqs for all channels
  mprintf("IRQs for all slave channels disabled.\n");
}

void configure_slaves(unsigned int tmr_value) {
  mprintf("configuring slaves.\n");
  int i = 0;
  int slot;
  scub_base[SRQ_ENA] = 0x0; //reset bitmask
  scub_base[MULTI_SLAVE_SEL] = 0x0; //reset bitmask  
  while(scub.slaves[i].unique_id) {
    slot = scub.slaves[i].slot;
    mprintf("enable slave[%d] in slot %d\n", i, slot);
    scub_base[SRQ_ENA] |= (1 << (slot-1));  //enable irqs for the slave
    scub_base[MULTI_SLAVE_SEL] |= (1 << (slot-1)); //set bitmask for broadcast select
    scub_base[(slot << 16) + SLAVE_INT_ENA] = 0x2000; //enable tmr irq in slave macro
    scub_base[(slot << 16) + TMR_BASE + TMR_CNTRL] = 0x1; //reset TMR
    scub_base[(slot << 16) + TMR_BASE + TMR_VALUEL] = tmr_value & 0xffff; //enable generation of tmr irqs, 1ms, 0xe848
    scub_base[(slot << 16) + TMR_BASE + TMR_VALUEH] = tmr_value >> 16; //enable generation of tmr irqs, 1ms, 0x001e
    i++;
  }
}

void configure_fgs() {
  int i = 0, j = 0;
  int slot;
  mprintf("configuring fgs.\n");
  while(scub.slaves[i].unique_id) { /* more slaves in list */
    /* actions per slave card */
    slot = scub.slaves[i].slot;
    scub_base[SRQ_ENA] |= (1 << (slot-1));  //enable irqs for the slave
    scub_base[MULTI_SLAVE_SEL] |= (1 << (slot-1)); //set bitmask for broadcast select
    if(scub.slaves[i].cid_group == 3 || scub.slaves[i].cid_group == 38) { /* ADDAC -> 2 FGs */
      scub_base[(slot << 16) + SLAVE_INT_ENA] |= 0xc000; /* enable fg1 and fg2 irq */
      scub_base[(slot << 16) + DAC1_BASE + DAC_CNTRL] = 0x10; // set FG mode
      scub_base[(slot << 16) + DAC2_BASE + DAC_CNTRL] = 0x10; // set FG mode
    } else if (scub.slaves[i].cid_group == 26) {
      scub_base[(slot << 16) + SLAVE_INT_ENA] |= 0x8000; /* enable fg1 irq */
      scub_base[(slot << 16) + DAC1_BASE + DAC_CNTRL] = 0x10; // set FG mode
    }
    j = 0;
    while(scub.slaves[i].devs[j].version) { /* more fgs in this device */
      /* actions per fg */
      mprintf("enable fg[%d] in slot %d\n", scub.slaves[i].devs[j].dev_number, slot);
      scub_base[(slot << 16) + scub.slaves[i].devs[j].offset + FG_CNTRL] = 0x1; // reset fg
      scub_base[(slot << 16) + scub.slaves[i].devs[j].offset + FG_CNTRL] = (5<<13); //set frequency Bit 15..13  
      scub_base[(slot << 16) + scub.slaves[i].devs[j].offset + FG_CNTRL] |= (2<<10); //set step count Bit 12..10  
      scub_base[(slot << 16) + scub.slaves[i].devs[j].offset + FG_A] = 0x1;
      scub_base[(slot << 16) + scub.slaves[i].devs[j].offset + FG_SHIFTA] = 0x20;
      scub_base[(slot << 16) + scub.slaves[i].devs[j].offset + FG_SHIFTB] = 0x20;
      scub.slaves[i].devs[j].running = 1;
      j++;
    }
    i++;
  }
} 

void reset_slaves() {
  mprintf("resetting slaves.\n");
  int i = 0;
  scub_base[SRQ_ENA] = 0x0; //reset bitmask
  scub_base[MULTI_SLAVE_SEL] = 0x0; //reset bitmask  
  while(scub.slaves[i].unique_id) {
    disp_put_c('x');
    scub_base[(scub.slaves[i].slot << 16) + TMR_BASE + TMR_CNTRL] = 0x1; //reset TMR
    i++;
  }
}

scan_bus() {
  int i=0, j=0;
  scan_scu_bus(&scub, backplane_id, scub_base);
  scan_for_fgs(&scub, &fgs);
  mprintf("ID: 0x%08x%08x\n", (int)(scub.unique_id >> 32), (int)scub.unique_id); 
  while(scub.slaves[i].unique_id) { /* more slaves in list */ 
      mprintf("slaves[%d] ID:  0x%08x%08x\n",i, (int)(scub.slaves[i].unique_id>>32), (int)scub.slaves[i].unique_id); 
      mprintf("slv ver: 0x%x cid_sys: %d cid_grp: %d\n", scub.slaves[i].version, scub.slaves[i].cid_sys, scub.slaves[i].cid_group); 
      mprintf("slot:        %d\n", scub.slaves[i].slot); 
      j = 0; 
      while(scub.slaves[i].devs[j].version) { /* more fgs in list */ 
        mprintf("   fg[%d], version 0x%x \n", j, scub.slaves[i].devs[j].version); 
        j++; 
      } 
      i++; 
  }
  i = 0;
  mprintf("FGs found:\n");
  while(fgs.devs[i]) {
    mprintf("fg[%d] slot: %d \n", i, fgs.devs[i]->slave->slot);
    i++;
  } 
}

void sw_irq_handler() {

  switch(global_msi.adr>>2 & 0xf) {
    case 0:
      disable_msi_irqs();  
    break;
    case 1:
      configure_slaves(global_msi.msg);
      enable_msi_irqs();
      scub_base[(0xd << 16) + TMR_BASE + TMR_CNTRL] = 0x2; //multicast tmr enable
    break;
    case 2:
      disable_msi_irqs();
      scan_bus();
    break;
    case 3:
      enable_msi_irqs();
      configure_fgs();
      scub_base[(0xd << 16) + FG1_BASE + FG_BROAD] = 0x4711; //start all FG1s (and linked FG2s)
    break;
    default:
      mprintf("swi: 0x%x\n", global_msi.adr);
      mprintf("     0x%x\n", global_msi.msg);
    break;
  }
}

void init_msi() {
  isr_table_clr();
  isr_ptr_table[1] = &slave_irq_handler;
  isr_ptr_table[2] = &sw_irq_handler;  
  irq_set_mask(0x06);
  irq_enable();
  mprintf("MSI IRQs configured.\n");
}

void init() {
  uart_init_hw();
  scan_bus();
  //configure_slaves(0x006ee848);
  init_msi();
} 

int main(void) {
  char input;
  int i, j;
  int idx = 0;
  int slot;
  int fg_is_running[MAX_FG_DEVICES];
  discoverPeriphery();  
  scub_base     = (unsigned short*)find_device_adr(GSI, SCU_BUS_MASTER);
  BASE_ONEWIRE  = (unsigned int*)find_device_adr(CERN, WR_1Wire);
  scub_irq_base = (unsigned int*)find_device_adr(GSI, SCU_IRQ_CTRL);
  find_device_multi(lm32_irq_endp, &idx, 10, GSI, IRQ_ENDPOINT);
  

  disp_reset();
  disp_put_c('\f');
  init(); 

  mprintf("number of irq_endpoints found: %d\n", idx);
  for (i=0; i < idx; i++) {
    mprintf("irq_endp[%d] is: 0x%x\n",i, getSdbAdr(&lm32_irq_endp[i]));
  }
  mprintf("scub_irq_base is: 0x%x\n", scub_irq_base); 


  //while(1);
  while(1) { 
    i = 0; 
    while(scub.slaves[i].unique_id) { /* more slaves in list */ 
      slot = scub.slaves[i].slot;
      j = 0; 
      while(scub.slaves[i].devs[j].version) { /* more fgs in list */
        if ((scub_base[(slot << 16) + scub.slaves[i].devs[j].offset + FG_CNTRL] & 0x8) > 0) // fg stopped
          if (scub.slaves[i].devs[j].running > 0) { //report only once
            scub.slaves[i].devs[j].endvalue = scub_base[(slot << 16) + scub.slaves[i].devs[j].offset + 0x8]; // last cnt from from fg macro
            mprintf("fg %d in slot %d stopped at ramp value 0x%x.\n",
                     scub.slaves[i].devs[j].dev_number, slot, scub.slaves[i].devs[j].endvalue);
            scub.slaves[i].devs[j].running = 0;
          }  
        j++; 
      } 
      i++; 
    }
    
  }

  return(0);
}
