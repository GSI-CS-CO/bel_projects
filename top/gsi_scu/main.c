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

extern struct w1_bus wrpc_w1_bus;

#define SHARED __attribute__((section(".shared")))
uint64_t SHARED board_id = -1;
uint16_t SHARED board_temp = -1;
uint64_t SHARED ext_id = -1;
uint16_t SHARED ext_temp = -1; 
uint64_t SHARED backplane_id = -1;
uint16_t SHARED backplane_temp = -1;
unsigned int SHARED status;
volatile unsigned int SHARED fg_control;

volatile unsigned int* pSDB_base    = (unsigned int*)0x7FFFFA00;
volatile unsigned int* display;
volatile unsigned int* irq_slave;
volatile unsigned short* scu_bus_master;
volatile unsigned int* cpu_ID;
volatile unsigned int* time_sys;
volatile unsigned int* cores;
volatile unsigned int* atomic;
volatile unsigned int BASE_ONEWIRE;

int slaves[SCU_BUS_MAX_SLOTS+1] = {0};
volatile unsigned short icounter[SCU_BUS_MAX_SLOTS+1];


struct pset {
  int a;
  int l_a;
  int b;
  int l_b;
  int c;
  int n;
};

void usleep(int x)
{
  int i;
  for (i = x * CPU_CLOCK/1000/4; i > 0; i--) asm("# noop");
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
          mprintf("bus,device (%d,%d): 0x%08x%08x ", wrpc_w1_bus.detail, i, (int)(d->rom >> 32), (int)d->rom);
          if ((char)d->rom == 0x42) {
            *id = d->rom;
            tvalue = w1_read_temp(d, 0);
            *temp = (tvalue >> 12); //full precision with 1/16 degree C
            mprintf("temp: %dC", tvalue >> 16); //show only integer part for debug
          }
          mprintf("\n");
        }  
    }
  } else {
    mprintf("no devices found on bus %d\n", wrpc_w1_bus.detail);
  }
  //int x = w1_read_temp_bus(&wrpc_w1_bus, 0);
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
  unsigned short tmr_irq_cnts = scu_bus_master[((slave_nr) << 16) + TMR_BASE + TMR_IRQ_CNT]; 
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
    scu_bus_master[((slave_nr) << 16) + SLAVE_INT_ACT] |= 5; //ack timer and powerup irq
  }
  icounter[slave_nr-1]++; 
}

void scan_scu_bus() {
  int i = 0;
  probe_scu_bus(scu_bus_master,55,3,slaves); //probe for ADDAC cards
  
  if (!slaves[0]) {
    disp_put_c('\f');
    disp_put_str("no slaves\n");
    while(1);
  }
  
  status = 0;
  while (slaves[i++]) {
    status++; //FIXME
  }

}

void init_irq() {
  //SCU Bus Master
  //enable slave irqs
  scu_bus_master[GLOBAL_IRQ_ENA] = 0x20;
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
  irq_clear_queue(0xf);
  for (i = 0; i < SCU_BUS_MAX_SLOTS; i++) {
    icounter[i] = 0; //reset counter in ISR
  }
}

void configure_slaves() {
  int i = 0;
  scu_bus_master[SRQ_ENA] = 0x0; //reset bitmask
  scu_bus_master[MULTI_SLAVE_SEL] = 0x0; //reset bitmask  
  while(slaves[i]) {
    disp_put_c('x');
    scu_bus_master[SRQ_ENA] |= (1 << (slaves[i]-1));  //enable irqs for the slave
    scu_bus_master[MULTI_SLAVE_SEL] |= (1 << (slaves[i]-1)); //set bitmask for broadcast select
    scu_bus_master[(slaves[i] << 16) + SLAVE_INT_ENA] = 0x4; //enable tmr irq in slave macro
    scu_bus_master[(slaves[i] << 16) + TMR_BASE + TMR_CNTRL] = 0x1; //reset TMR
    scu_bus_master[(slaves[i] << 16) + TMR_BASE + TMR_VALUEL] = 0xffff; //enable generation of tmr irqs
    scu_bus_master[(slaves[i] << 16) + TMR_BASE + TMR_VALUEH] = 0x005f; //enable generation of tmr irqs
    scu_bus_master[(slaves[i] << 16) + TMR_BASE + TMR_CNTRL] |= 0x2; //enable generation of tmr irqs
    i++;
  }
} 

void reset_slaves() {
  int i = 0;
  scu_bus_master[SRQ_ENA] = 0x0; //reset bitmask
  scu_bus_master[MULTI_SLAVE_SEL] = 0x0; //reset bitmask  
  while(slaves[i]) {
    disp_put_c('x');
    scu_bus_master[(slaves[i] << 16) + TMR_BASE + TMR_CNTRL] = 0x1; //reset TMR
    i++;
  }
}

void init() {
  
  uart_init_hw();
  uart_write_string("\nDebug Port\n");
  
  /*
  scan_scu_bus();
  reset_slaves();
  usleep(1000);
  dis_irq();
  usleep(1000000);
  init_irq();
  usleep(1000);
  configure_slaves();
  */
} 

int main(void) {
  int i = 0;
  
  display      = (unsigned int*)find_device(SCU_OLED_DISPLAY);
  irq_slave    = (unsigned int*)find_device(IRQ_MSI_CTRL_IF);
  scu_bus_master = (unsigned short*)find_device(SCU_BUS_MASTER);  

  
  disp_reset();
  disp_put_c('\f');

  init(); 
  
  //config of DAC and FG
  while(slaves[i]) {
    i++;
  }
  
  // disp_reset();	
  // disp_put_str(mytext);
  
  // wait for WR deamon to star 
  msDelay(5000);

  while(1) {
      
    mprintf("Onboard Onewire Devices:\n");
    BASE_ONEWIRE = BASE_OW_WR;
    wrpc_w1_init();
    ReadTempDevices(0, &board_id, &board_temp);
    mprintf("External Onewire Devices:\n");
    BASE_ONEWIRE = BASE_OW_EXT;
    wrpc_w1_init();
    ReadTempDevices(0, &ext_id, &ext_temp);
    ReadTempDevices(1, &backplane_id, &backplane_temp);
    //placeholder for fg software
    //if (fg_control) {
    //  init();
    //  fg_control = 0;
    //}
  }

  return(0);
}
