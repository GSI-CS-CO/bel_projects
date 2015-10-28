#include <stdint.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "syscon.h"
#include "hw/memlayout.h"
#include "mprintf.h"
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
#include "scu_mil.h"

#define MSI_SLAVE 0
#define MSI_WB_FG 2
#define SEND_SIG(SIG)     *(volatile unsigned int *)(char*)(fg_regs[channel].irq | 0x80000000UL) = SIG
#define SIG_REFILL      0
#define SIG_START       1
#define SIG_STOP_EMPTY  2
#define SIG_STOP_NEMPTY 3
#define SIG_ARMED       4
#define SIG_DISARMED    5

#define FG_RUNNING  0x4
#define FG_ENABLED  0x2
#define FG_DREQ     0x8

#define CLK_PERIOD (1000000 / USRCPUCLK) // USRCPUCLK in KHz

extern struct w1_bus wrpc_w1_bus;

#define SHARED __attribute__((section(".shared")))
uint64_t SHARED board_id = -1;
uint64_t SHARED ext_id = -1;
uint64_t SHARED backplane_id = -1;
uint32_t SHARED board_temp = -1;
uint32_t SHARED ext_temp = -1; 
uint32_t SHARED backplane_temp = -1;
uint32_t SHARED fg_magic_number = 0xdeadbeef;
uint32_t SHARED fg_version = 0x2;
uint32_t SHARED fg_num_channels = MAX_FG_CHANNELS;
uint32_t SHARED fg_buffer_size = BUFFER_SIZE+1;
uint32_t SHARED fg_macros[MAX_FG_MACROS]; // hi..lo bytes: slot, device, version, output-bits
struct channel_regs SHARED fg_regs[MAX_FG_CHANNELS]; 
struct channel_buffer SHARED fg_buffer[MAX_FG_CHANNELS];

struct scu_bus scub;
struct fg_list fgs;

volatile unsigned short* scub_base = 0;
volatile unsigned int* scub_irq_base = 0;
volatile unsigned int* scu_mil_base = 0;
sdb_location lm32_irq_endp[10];       // there are three queues for msis
sdb_location ow_base[2];              // there should be two controllers
volatile unsigned int* pcie_irq_endp = 0;
volatile unsigned int* cpu_info_base = 0;

volatile unsigned int param_sent[MAX_FG_CHANNELS];
volatile int initialized[MAX_SCU_SLAVES] = {0};
int endp_idx = 0;

void enable_msis(int channel) {
  int slot;
  //SCU Bus Master
  scub_base[GLOBAL_IRQ_ENA] = 0x20;                                 //enable slave irqs in scu bus master
  if (channel >= 0 && channel < MAX_FG_CHANNELS) {
    slot = fg_macros[fg_regs[channel].macro_number] >> 24;          //dereference slot number
    scub_irq_base[8] = slot-1;                                      //channel select
    scub_irq_base[9] = 0x08154711;                                  //msg
    scub_irq_base[10] = getSdbAdr(&lm32_irq_endp[endp_idx + MSI_SLAVE]) + (slot << 2); //destination address, do not use lower 2 bits
    scub_irq_base[2] = (1 << (slot - 1));                          //enable slave
    //mprintf("IRQs for slave %d enabled.\n", slot);
  }
}

void disable_slave_irq(int channel) {
  int slot, dev;
  if (channel >= 0 && channel < MAX_FG_CHANNELS) {
    slot = fg_macros[fg_regs[channel].macro_number] >> 24;          //slot number
    dev = (fg_macros[fg_regs[channel].macro_number] >> 16) & 0xff;  //dev number
    if (dev == 0)
      scub_base[(slot << 16) + SLAVE_INT_ENA] &= ~(0x8000);         //disable fg1 irq
    else if (dev == 1)
      scub_base[(slot << 16) + SLAVE_INT_ENA] &= ~(0x4000);         //disable fg2 irq
      

    //mprintf("IRQs for slave %d disabled.\n", slot);
  }
}

void msDelayBig(uint64_t ms)
{
  uint64_t later = getSysTime() + ms * 1000000ULL / 8;
  while(getSysTime() < later) {asm("# noop");}
}

void msDelay(uint32_t msecs) {
  usleep(1000 * msecs);
}

void send_fg_param(int slave_nr, int fg_base) {
  struct param_set pset;
  int fg_num, add_freq_sel, step_cnt_sel;
  
  fg_num = (scub_base[(slave_nr << 16) + fg_base + FG_CNTRL] & 0x3f0) >> 4; // virtual fg number Bits 9..4
  if(!cbisEmpty(&fg_regs[0], fg_num)) {
    cbRead(&fg_buffer[0], &fg_regs[0], fg_num, &pset);
    step_cnt_sel = pset.control & 0x7;
    add_freq_sel = (pset.control & 0x38) >> 3;
    scub_base[(slave_nr << 16) + fg_base + FG_CNTRL] &= ~(0xfc00); // clear freq and step select
    scub_base[(slave_nr << 16) + fg_base + FG_CNTRL] |= (add_freq_sel << 13) | (step_cnt_sel << 10);
    scub_base[(slave_nr << 16) + fg_base + FG_A] = pset.coeff_a;
    scub_base[(slave_nr << 16) + fg_base + FG_B] = pset.coeff_b;
    scub_base[(slave_nr << 16) + fg_base + FG_SHIFT] = (pset.control & 0x3ffc0) >> 6; //shift a 17..12 shift b 11..6 
    scub_base[(slave_nr << 16) + fg_base + FG_STARTL] = pset.coeff_c & 0xffff;
    scub_base[(slave_nr << 16) + fg_base + FG_STARTH] = (pset.coeff_c & 0xffff0000) >> 16; // data written with high word
    param_sent[fg_num]++;
  }
}

void handle(int slave_nr, unsigned FG_BASE)
{

    short cntrl_reg = scub_base[(slave_nr << 16) + FG_BASE + FG_CNTRL];
    int channel = (cntrl_reg & 0x3f0) >> 4;        // virtual fg number Bits 9..4
    fg_regs[channel].ramp_count = scub_base[(slave_nr << 16) + FG_BASE + FG_RAMP_CNT_LO];          // last cnt from from fg macro, read from LO address copies hardware counter to shadow reg
    fg_regs[channel].ramp_count |= scub_base[(slave_nr << 16) + FG_BASE + FG_RAMP_CNT_HI] << 16;    // last cnt from from fg macro
    //mprintf("irq received for channel[%d]\n", channel);
    if (!(cntrl_reg  & FG_RUNNING)) {  // fg stopped
      //mprintf("fg 0x%x in slave %d stopped after %d tuples\n", FG_BASE, slave_nr, fg_regs[channel].ramp_count);
      if (cbisEmpty(&fg_regs[0], channel))
        SEND_SIG(SIG_STOP_EMPTY); // normal stop
      else
        SEND_SIG(SIG_STOP_NEMPTY); // something went wrong
      //disable_slave_irq(channel);
      fg_regs[channel].state = 0;
    } else if ((cntrl_reg & FG_RUNNING) && !(cntrl_reg & FG_DREQ)) {
      SEND_SIG(SIG_START); // fg has received the tag
      if (cbgetCount(&fg_regs[0], channel) == THRESHOLD)
        SEND_SIG(SIG_REFILL);
      send_fg_param(slave_nr, FG_BASE);
    } else if ((cntrl_reg & FG_RUNNING) && (cntrl_reg & FG_DREQ)) {
      if (cbgetCount(&fg_regs[0], channel) == THRESHOLD)
        SEND_SIG(SIG_REFILL);
      send_fg_param(slave_nr, FG_BASE);
    }
}

void slave_irq_handler()
{
  int i, j = 0;
  unsigned char slave_nr = global_msi.adr>>2 & 0xf;
  volatile unsigned short tmr_irq_cnts; 
  static unsigned short old_tmr_cnt[MAX_SCU_SLAVES];
  volatile unsigned int slv_int_act_reg = scub_base[(slave_nr << 16) + SLAVE_INT_ACT];
  unsigned int slave_acks = 0;
  int channel;

  if (slave_nr < 0 || slave_nr > MAX_SCU_SLAVES) {
    mprintf("IRQ unknown.\n");
    return;
  }

  if (slv_int_act_reg & 0x1) {// powerup interrupt
    slave_acks |= 0x1;
  }
  
  if (slv_int_act_reg & 0x2000) { //tmr irq?
    //tmr_irq_cnts = scub_base[(slave_nr << 16) + TMR_BASE + TMR_IRQ_CNT];
    // init old_tmr_cnt
    if (!initialized[slave_nr-1]) {
      //old_tmr_cnt[slave_nr-1] = tmr_irq_cnts - 1;
      old_tmr_cnt[slave_nr-1] = 0;
      mprintf("init slave: %d with %x\n", slave_nr, tmr_irq_cnts - 1); 
      initialized[slave_nr-1] = 1;
    }  
    old_tmr_cnt[slave_nr-1]++;
    slave_acks |= (1 << 13); //ack timer irq
    mprintf("irq1 slave: %d, cnt: %x, act: %x\n", slave_nr, old_tmr_cnt[slave_nr-1], tmr_irq_cnts);
  }

  if (slv_int_act_reg & FG1_IRQ) { //FG irq?
    handle(slave_nr, FG1_BASE);
    slave_acks |= FG1_IRQ;
  } 
  if (slv_int_act_reg & FG2_IRQ) { //FG irq?
    handle(slave_nr, FG2_BASE);
    slave_acks |= FG2_IRQ;
  } 
  scub_base[(slave_nr << 16) + SLAVE_INT_ACT] = slave_acks; // ack all pending irqs 
}

void configure_timer(unsigned int tmr_value) {
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
    scub_base[(slot << 16) + TMR_BASE + TMR_REPEAT] = 0x14; //number of generated irqs
    i++;
  }
}

int configure_fg_macro(int channel) {
  int i = 0;
  int slot, dev, fg_base, dac_base;
  struct param_set pset;
  int add_freq_sel, step_cnt_sel;
  
  if (channel >= 0 && channel < MAX_FG_CHANNELS) {
    /* actions per slave card */
    slot = fg_macros[fg_regs[channel].macro_number] >> 24;          //dereference slot number
    dev =  (fg_macros[fg_regs[channel].macro_number] >> 16) & 0xff; //dereference dev number
    scub_base[SRQ_ENA] |= (1 << (slot-1));                          //enable irqs for the slave
    /* enable irqs in the slave cards */
    scub_base[(slot << 16) + SLAVE_INT_ENA] |= 0xc000;      //enable fg1 and fg2 irq
    
    /* which macro are we? */
    if (dev == 0) {
      fg_base = FG1_BASE;
      dac_base = DAC1_BASE;
    } else if (dev == 1) {
      fg_base = FG2_BASE;
      dac_base = DAC2_BASE;
    } else
      return -1;
    
    scub_base[(slot << 16) + dac_base + DAC_CNTRL] = 0x10; // set FG mode
    scub_base[(slot << 16) + fg_base + FG_CNTRL] = 0x1; // reset fg
    //set virtual fg number Bit 9..4
    scub_base[(slot << 16) + fg_base + FG_CNTRL] |= (channel << 4);
    //fetch first parameter set from buffer
    //mprintf("wrptr 0x%x rdptr 0x%x\n", fg_regs[channel].wr_ptr, fg_regs[channel].rd_ptr);
    if(!cbisEmpty(&fg_regs[0], channel)) {
      cbRead(&fg_buffer[0], &fg_regs[0], channel, &pset);
      step_cnt_sel = pset.control & 0x7;
      add_freq_sel = (pset.control & 0x38) >> 3;
      scub_base[(slot << 16) + fg_base + FG_CNTRL] |= add_freq_sel << 13 | step_cnt_sel << 10;
      scub_base[(slot << 16) + fg_base + FG_A] = pset.coeff_a;
      scub_base[(slot << 16) + fg_base + FG_B] = pset.coeff_b;
      scub_base[(slot << 16) + fg_base + FG_SHIFT] = (pset.control & 0x3ffc0) >> 6; //shift a 17..12 shift b 11..6 
      scub_base[(slot << 16) + fg_base + FG_STARTL] = pset.coeff_c & 0xffff;
      scub_base[(slot << 16) + fg_base + FG_STARTH] = (pset.coeff_c & 0xffff0000) >> 16; // data written with high word
      param_sent[i]++;
    }
    //mprintf("enable channel[%d] 0x%x PLEASE REMOVE AFTER USE!!!!!!!\n", channel, fg_regs[channel].irq);
    scub_base[(slot << 16) + fg_base + FG_TAG_LOW] = fg_regs[channel].tag & 0xffff;
    scub_base[(slot << 16) + fg_base + FG_TAG_HIGH] = fg_regs[channel].tag >> 16;
    //enable the fg macro
    scub_base[(slot << 16) + fg_base + FG_CNTRL] |= FG_ENABLED;
    fg_regs[channel].state = 1; 
    SEND_SIG(SIG_ARMED);
  }
  return 0; 
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

/* scans for slaves and then for fgs */
void print_fgs() {
  int i=0, j=0;
  scan_scu_bus(&scub, backplane_id, scub_base);
  scan_for_fgs(&scub, &fg_macros[0]);
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

  while(i < MAX_FG_MACROS) {
    // hi..lo bytes: slot, device, version, output-bits
    if (fg_macros[i] != 0)
      mprintf("fg-%d-%d\n", fg_macros[i] >> 24, (fg_macros[i] >> 16) & 0xff);
    i++;
  } 
}

void print_regs() {
  int i;
  for(i=0; i < MAX_FG_CHANNELS; i++) {
    mprintf("channel[%d].wr_ptr %d\n", i, fg_regs[i].wr_ptr);
    mprintf("channel[%d].rd_ptr %d\n", i, fg_regs[i].rd_ptr);
    mprintf("channel[%d].irq 0x%x\n", i, fg_regs[i].irq);
    mprintf("channel[%d].macro_number %d\n", i, fg_regs[i].macro_number);
    mprintf("channel[%d].ramp_count %d\n", i, fg_regs[i].ramp_count);
    mprintf("channel[%d].tag 0x%x\n", i, fg_regs[i].tag);
    mprintf("channel[%d].state %d\n", i, fg_regs[i].state);
    mprintf("\n");
  } 
}

void disable_channel(unsigned int channel) {
  int slot, dev, fg_base, dac_base;
  if (fg_regs[channel].macro_number == -1) return;
  slot = fg_macros[fg_regs[channel].macro_number] >> 24;         //dereference slot number
  dev = (fg_macros[fg_regs[channel].macro_number] >> 16) & 0xff; //dereference dev number
  //mprintf("disarmed slot %d dev %d in channel[%d] state %d\n", slot, dev, channel, fg_regs[channel].state); 
  /* which macro are we? */
  if (dev == 0) {
    fg_base = FG1_BASE;
    dac_base = DAC1_BASE;
  } else if (dev == 1) {
    fg_base = FG2_BASE;
    dac_base = DAC2_BASE;
  } else
    return;
  // disarm hardware
  scub_base[(slot >> 16) + fg_base + FG_CNTRL] &= ~(0x2);
  scub_base[(slot << 16) + dac_base + DAC_CNTRL] &= ~(0x10); // set FG mode
  if (fg_regs[channel].state == 1) {    // hw is running
    fg_regs[channel].rd_ptr = fg_regs[channel].wr_ptr;
  } else {
    SEND_SIG(SIG_DISARMED);
  } 
}

void sw_irq_handler() {
  int i;
  struct param_set pset;
  switch(global_msi.adr>>2 & 0xf) {
    case 0:
      init_buffers(&fg_regs[0], global_msi.msg, &fg_macros[0], scub_base);
      param_sent[global_msi.msg] = 0;
      //mprintf("swi flush %d\n", global_msi.msg);
    break;
    case 1:
      configure_timer(global_msi.msg);
      enable_msis(global_msi.msg);
      scub_base[(0xd << 16) + TMR_BASE + TMR_CNTRL] = 0x2; //multicast tmr enable
    break;
    case 2:
      enable_msis(global_msi.msg);
      configure_fg_macro(global_msi.msg);
      //print_regs();
    break;
    case 3:
      disable_channel(global_msi.msg);
    break;
    case 4:
      for (i = 0; i < MAX_FG_CHANNELS; i++)
        mprintf("fg[%d] buffer: %d param_sent: %d\n", i, cbgetCount(&fg_regs[0], i), param_sent[i]);
    break;
    case 5:
      if (global_msi.msg >= 0 && global_msi.msg < MAX_FG_CHANNELS) {
        if(!cbisEmpty(&fg_regs[0], global_msi.msg)) {
          cbRead(&fg_buffer[0], &fg_regs[0], global_msi.msg, &pset);
          mprintf("read buffer[%d]: a %d, l_a %d, b %d, l_b %d, c %d, n %d\n",
                   global_msi.msg, pset.coeff_a, (pset.control & 0x1f000) >> 12, pset.coeff_b,
                   (pset.control & 0xfc0) >> 6, pset.coeff_c, (pset.control & 0x7));
        } else {
          mprintf("read buffer[%d]: buffer empty!\n", global_msi.msg);
        }
          
      }
    break;
    case 6:
      run_mil_test(scu_mil_base, global_msi.msg & 0xff);
    break;
    default:
      mprintf("swi: 0x%x\n", global_msi.adr);
      mprintf("     0x%x\n", global_msi.msg);
    break;
  }
}
void updateTemp() {
  BASE_ONEWIRE = (unsigned char *)getSdbAdr(&ow_base[0]);
  wrpc_w1_init();
  ReadTempDevices(0, &board_id, &board_temp);
  BASE_ONEWIRE = (unsigned char *)getSdbAdr(&ow_base[1]);
  wrpc_w1_init();
  ReadTempDevices(0, &ext_id, &ext_temp);
  ReadTempDevices(1, &backplane_id, &backplane_temp);
  BASE_ONEWIRE = (unsigned char *)getSdbAdr(&ow_base[0]); // important for PTP deamon 
  wrpc_w1_init();
}

void tmr_irq_handler() {
  //updateTemp();
}

void init_irq_handlers() {
  isr_table_clr();
  isr_ptr_table[0] = &tmr_irq_handler;
  isr_ptr_table[1] = &slave_irq_handler;
  isr_ptr_table[2] = &sw_irq_handler;
  isr_ptr_table[3] = &tmr_irq_handler;  
  irq_set_mask(0x06);
  irq_enable();
  mprintf("MSI IRQs configured.\n");
}


void init() {
  int i;
  for (i=0; i < MAX_FG_CHANNELS; i++)
    fg_regs[i].macro_number = -1; //no macros assigned to channels at startup
  uart_init_hw();           //enables the uart for debug messages
  updateTemp();             //update 1Wire ID and temperatures
  print_fgs();              //scans for slave cards and fgs
  init_irq_handlers();      //enable the irqs
} 

void _segfault(int sig)
{
  mprintf("KABOOM!\n");
  //while (1) {}
  return;
}

int main(void) {
  int i;
  sdb_location found_sdb[20];
  uint32_t lm32_endp_idx = 0;
  uint32_t ow_base_idx = 0;
  uint32_t clu_cb_idx = 0;
  discoverPeriphery();
  /* additional periphery needed for scu */
  cpu_info_base   = (unsigned int*)find_device_adr(GSI, CPU_INFO_ROM);  
  scub_base       = (unsigned short*)find_device_adr(GSI, SCU_BUS_MASTER);
  scub_irq_base   = (unsigned int*)find_device_adr(GSI, SCU_IRQ_CTRL);    // irq controller for scu bus
  find_device_multi(&found_sdb[0], &clu_cb_idx, 20, GSI, LM32_CB_CLUSTER); // find location of cluster crossbar
  find_device_multi_in_subtree(&found_sdb[0], lm32_irq_endp, &lm32_endp_idx, 10, GSI, LM32_IRQ_EP); // list irq endpoints in cluster crossbar
  pcie_irq_endp   = (unsigned int *)find_device_adr(GSI, PCIE_IRQ_ENDP);
  scu_mil_base    = (unsigned int*)find_device(SCU_MIL);
  find_device_multi(ow_base, &ow_base_idx, 2, CERN, WR_1Wire);
  
  
  msDelayBig(1500); //wait for wr deamon to read sdbfs
  init();  
  if (BASE_SYSCON)
    mprintf("SYS_CON found on adr: 0x%x\n", BASE_SYSCON);
  else
    mprintf("no SYS_CON found!\n"); 

  timer_init(1); //needed by usleep_init() 
  usleep_init();
  
  if(cpu_info_base) {
    mprintf("CPU ID: 0x%x\n", cpu_info_base[0]);
    mprintf("number MSI endpoints: %d\n", cpu_info_base[1]);
  } else 
    mprintf("no CPU INFO ROM found!\n");

  if(cpu_info_base[1] < 3) {
    mprintf("not enough MSI endpoints for FG program!\n");
    while(1);
  }

  endp_idx = getCpuIdx() * cpu_info_base[1]; // calculate index from CPU ID and number of endpoints
 
  disp_reset();
  disp_put_c('\f');
  

  mprintf("number of lm32_irq_endpoints found: %d\n", lm32_endp_idx);
  for (i=0; i < lm32_endp_idx; i++) {
    mprintf("irq_endp[%d] is: 0x%x\n",i, getSdbAdr(&lm32_irq_endp[i]));
  }
  mprintf("number of 1Wire controllers found: %d\n", ow_base_idx);
  for (i=0; i < ow_base_idx; i++) {
    mprintf("ow_base[%d] is: 0x%x\n",i, getSdbAdr(&ow_base[i]));
  }
  mprintf("pcie_irq_endp is: 0x%x\n", pcie_irq_endp);
  mprintf("scub_irq_base is: 0x%x\n", scub_irq_base);

  while(1) {
 //   updateTemp();
 //   msDelayBig(15000);
  }

  return(0);
}
