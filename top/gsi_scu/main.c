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
#define FGDEBUG
#define CBDEBUG

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

int slaves[SCU_BUS_MAX_SLOTS+1] = {0};
volatile unsigned short icounter[SCU_BUS_MAX_SLOTS+1];

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

void ReadTempDevices(int bus, uint64_t *id, uint16_t *temp) {
  struct w1_dev *d;
  int i;
  int tvalue;
  wrpc_w1_bus.detail = bus; // set the portnumber of the onewire controller
  if (w1_scan_bus(&wrpc_w1_bus) > 0) {
    for (i = 0; i < W1_MAX_DEVICES; i++) {
      d = wrpc_w1_bus.devs + i;
        if (d->rom) {
          #ifdef DEBUG
          mprintf("bus,device (%d,%d): 0x%08x%08x ", wrpc_w1_bus.detail, i, (int)(d->rom >> 32), (int)d->rom);
          #endif
          if ((char)d->rom == 0x42) {
            *id = d->rom;
            tvalue = w1_read_temp(d, 0);
            *temp = (tvalue >> 12); //full precision with 1/16 degree C
            #ifdef DEBUG
            mprintf("temp: %dC", tvalue >> 16); //show only integer part for debug
            #endif
          }
          #ifdef DEBUG
          mprintf("\n");
          #endif
        }  
    }
  } else {
    #ifdef DEBUG
    mprintf("no devices found on bus %d\n", wrpc_w1_bus.detail);
    #endif
  }
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

void isr1()
{
  char buffer[12];
  struct pset *p;
  //p = (struct p *)fesa_if;
  unsigned char slave_nr = global_msi.adr>>2 & 0xf;
  unsigned short tmr_irq_cnts = scub_base[((slave_nr) << 16) + TMR_BASE + TMR_IRQ_CNT]; 
  disp_put_c(slave_nr + '0');
  disp_put_c(' ');

  sprinthex(buffer, icounter[slave_nr - 1], 4);
  disp_put_str(buffer);
  disp_put_c('\n');
  disp_put_c(' '); disp_put_c(' '); 
 
  sprinthex(buffer, tmr_irq_cnts, 4);
  disp_put_str(buffer);
  disp_put_c('\n');

  if ((tmr_irq_cnts == icounter[slave_nr - 1])) {
    scub_base[((slave_nr) << 16) + SLAVE_INT_ACT] |= 5; //ack timer and powerup irq
  }
  icounter[slave_nr-1]++; 
}

void init_irq() {
  //SCU Bus Master
  //enable slave irqs
  scub_base[GLOBAL_IRQ_ENA] = 0x20;
  isr_table_clr();
  isr_ptr_table[1]= isr1;  
  irq_set_mask(0x02);
  irq_enable();
}

void dis_irq() {
  int i = 0;
  irq_set_mask(0x02);
  irq_disable();
  isr_table_clr();
  for (i = 0; i < SCU_BUS_MAX_SLOTS; i++) {
    icounter[i] = 0; //reset counter in ISR
  }
}

void configure_slaves() {
  int i = 0;
  scub_base[SRQ_ENA] = 0x0; //reset bitmask
  scub_base[MULTI_SLAVE_SEL] = 0x0; //reset bitmask  
  while(slaves[i]) {
    disp_put_c('x');
    scub_base[SRQ_ENA] |= (1 << (slaves[i]-1));  //enable irqs for the slave
    scub_base[MULTI_SLAVE_SEL] |= (1 << (slaves[i]-1)); //set bitmask for broadcast select
    scub_base[(slaves[i] << 16) + SLAVE_INT_ENA] = 0x4; //enable tmr irq in slave macro
    scub_base[(slaves[i] << 16) + TMR_BASE + TMR_CNTRL] = 0x1; //reset TMR
    scub_base[(slaves[i] << 16) + TMR_BASE + TMR_VALUEL] = 0xffff; //enable generation of tmr irqs
    scub_base[(slaves[i] << 16) + TMR_BASE + TMR_VALUEH] = 0x005f; //enable generation of tmr irqs
    scub_base[(slaves[i] << 16) + TMR_BASE + TMR_CNTRL] |= 0x2; //enable generation of tmr irqs
    i++;
  }
} 

void reset_slaves() {
  int i = 0;
  scub_base[SRQ_ENA] = 0x0; //reset bitmask
  scub_base[MULTI_SLAVE_SEL] = 0x0; //reset bitmask  
  while(slaves[i]) {
    disp_put_c('x');
    scub_base[(slaves[i] << 16) + TMR_BASE + TMR_CNTRL] = 0x1; //reset TMR
    i++;
  }
}

void updateTemps() {
  #ifdef DEBUG
  mprintf("Onboard Onewire Devices:\n");
  #endif
 //conflicts with WR
 // BASE_ONEWIRE = (unsigned int*)BASE_OW_WR;
//  wrpc_w1_init();
//  ReadTempDevices(0, &board_id, &board_temp);
  #ifdef DEBUG
  mprintf("External Onewire Devices:\n");
  #endif
  //BASE_ONEWIRE = (unsigned int*)BASE_OW_EXT;
  wrpc_w1_init();
  ReadTempDevices(0, &ext_id, &ext_temp);
  ReadTempDevices(1, &backplane_id, &backplane_temp);
}

void init() {
  int i=0, j;
  uart_init_hw();
  updateTemps();
  init_buffers(&fg_buffer);
  #ifdef CBDEBUG
  for (i=0; i < MAX_FG_DEVICES; i++) {
    mprintf("cb[%d]: isEmpty = %d\n", i, cbisEmpty((struct circ_buffer *)&fg_buffer, i));
    mprintf("cb[%d]: isFull = %d\n", i, cbisFull((struct circ_buffer *)&fg_buffer, i));
    mprintf("cb[%d]: getCount = %d\n", i, cbgetCount((struct circ_buffer *)&fg_buffer, i));
  }
  #endif
  scan_scu_bus(&scub, backplane_id, scub_base);
  scan_for_fgs(&scub, &fgs);
  #ifdef FGDEBUG
  mprintf("ID: 0x%08x%08x\n", (int)(scub.unique_id >> 32), (int)scub.unique_id);
  while(scub.slaves[i].unique_id) { /* more slaves in list */
      mprintf("slaves[%d] ID:  0x%08x%08x\n",i, (int)(scub.slaves[i].unique_id>>32), (int)scub.slaves[i].unique_id);
      mprintf("slave macro: 0x%x\n", scub.slaves[i].version);
      mprintf("CID system:  %d\n", scub.slaves[i].cid_sys);
      mprintf("CID group:   %d\n", scub.slaves[i].cid_group);
      mprintf("slot:        %d\n", scub.slaves[i].slot);
      j = 0;
      while(scub.slaves[i].devs[j].version) { /* more fgs in list */
        mprintf("   fg[%d], version 0x%x \n", j, scub.slaves[i].devs[j].version);
        j++;
      }
      i++;
  }
  mprintf("fg_list-------------------\n");
  i = 0;
  while(fgs.devs[i]) {
    mprintf("fgs.devs[%d] 0x%x\n", i, fgs.devs[i]);
    mprintf("fg[%d]: version 0x%x\n", i, fgs.devs[i]->version);
    mprintf("        dev_number 0x%x\n", fgs.devs[i]->dev_number);
    mprintf("        offset 0x%x\n", fgs.devs[i]->offset);
    mprintf("        endvalue 0x%x\n", fgs.devs[i]->endvalue);
    i++;
  }
  #endif
/*  reset_slaves();
  usleep(1000);
  dis_irq();
  usleep(1000000);
  init_irq();
  usleep(1000);
  configure_slaves();
  */
} 

int main(void) {
  char buffer[20];
  int i = 0;
  struct param_set pset;
  unsigned int old[MAX_FG_DEVICES] = {0};

  discoverPeriphery();  
  scub_base    = (unsigned short*)find_device(SCU_BUS_MASTER);
  BASE_ONEWIRE = (unsigned int*)find_device(WR_1Wire);

  disp_reset();
  disp_put_c('\f');
  init(); 

  while(1) {
    //updateTemps();
    
    for(i = 0; i < MAX_FG_DEVICES; i++) {
      if (!cbisEmpty((struct circ_buffer *)&fg_buffer, i)) {
        cbRead((struct circ_buffer *)&fg_buffer, i, &pset);
        if((pset.coeff_c % 10000) == 0) {
          mprintf("cb[%d]: fcnt: %d coeff_a: %x coeff_b: 0x%x coeff_c: 0x%x\n",
          i, cbgetCount((struct circ_buffer *)&fg_buffer, i), pset.coeff_a, pset.coeff_b, pset.coeff_c);
        }
        if (old[i] + 1 != pset.coeff_c) {
          mprintf("cb[%d]: buffer value not consistent old: %x coeff_c: %x\n", i, old[i], pset.coeff_c);
          return(1);
        }
        old[i] = pset.coeff_c;
      }

      usleep(100);
    }
    //placeholder for fg software
    //if (fg_control) {
    //  init();
    //  fg_control = 0;
    //}
  }

  return(0);
}
