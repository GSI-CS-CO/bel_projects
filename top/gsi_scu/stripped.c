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
  char buffer[12];
  struct pset *p;
  unsigned char slave_nr = global_msi.adr>>2 & 0xf;
  volatile unsigned short tmr_irq_cnts; 
  static unsigned short old_tmr_cnt[MAX_SCU_SLAVES];


  if (slave_nr < 0 || slave_nr > MAX_SCU_SLAVES) {
    mprintf("unknown IRQ number.\n");
    return;
  }

  if (scub_base[((slave_nr) << 16) + SLAVE_INT_ACT] & 0x1) { //powerup irq?
    mprintf("ack powerup irq in slave %d\n", slave_nr);
    scub_base[((slave_nr) << 16) + SLAVE_INT_ACT] |= 0x1; //ack powerup irq
  }

  if (scub_base[((slave_nr) << 16) + SLAVE_INT_ACT] & 0x2000) { //tmr irq?
    tmr_irq_cnts = scub_base[((slave_nr) << 16) + TMR_BASE + TMR_IRQ_CNT];
    // init old_tmr_cnt
    if (!initialized[slave_nr-1]) {
      old_tmr_cnt[slave_nr-1] = tmr_irq_cnts - 1;
      mprintf("init slave: %d with %x\n", slave_nr, tmr_irq_cnts - 1); 
      initialized[slave_nr-1] = 1;
    }  
    // check for lost IRQs
    if ((tmr_irq_cnts == (unsigned short)(old_tmr_cnt[slave_nr-1] + 1))) {
      scub_base[((slave_nr) << 16) + SLAVE_INT_ACT] |= (1 << 13); //ack timer irq
      old_tmr_cnt[slave_nr-1] = tmr_irq_cnts;
    } else
      mprintf("irq1 slave: %d, old: %x, act: %x\n", slave_nr, old_tmr_cnt[slave_nr-1], tmr_irq_cnts);
  }
}



void enable_slave_irqs() {
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


void disable_slave_irqs() {
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
  scub_base[(0xf << 16) + TMR_BASE + TMR_CNTRL] |= 0x2; //multicast tmr enable
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
  //scan_for_fgs(&scub, &fgs);
  mprintf("ID: 0x%08x%08x\n", (int)(scub.unique_id >> 32), (int)scub.unique_id); 
  while(scub.slaves[i].unique_id) { /* more slaves in list */ 
      mprintf("slaves[%d] ID:  0x%08x%08x\n",i, (int)(scub.slaves[i].unique_id>>32), (int)scub.slaves[i].unique_id); 
      mprintf("slv ver: 0x%x cid_sys: %d cid_grp: %d\n", scub.slaves[i].version, scub.slaves[i].cid_sys, scub.slaves[i].cid_group); 
      mprintf("slot:        %d\n", scub.slaves[i].slot); 
      j = 0; 
      //while(scub.slaves[i].devs[j].version) { /* more fgs in list */ 
      //  mprintf("   fg[%d], version 0x%x \n", j, scub.slaves[i].devs[j].version); 
      //  j++; 
      //} 
      i++; 
  } 
}

void sw_irq_handler() {

  switch(global_msi.adr>>2 & 0xf) {
    case 0:
      disable_slave_irqs();  
    break;
    case 1:
      configure_slaves(global_msi.msg);
      enable_slave_irqs();
    break;
    case 2:
      disable_slave_irqs();
      scan_bus();
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
  configure_slaves(0x006ee848);
  init_msi();
} 

int main(void) {
  char input;
  int i;
  int idx = 0;
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


  input = uart_read_byte();

  if (input == 's')
    mprintf("input: %c\n", input);
  
  while(1);

  return(0);
}
