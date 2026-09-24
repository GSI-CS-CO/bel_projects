#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include <pp-printf.h>
#include "mprintf.h"
#include "mini_sdb.h"
#include "irq.h"
#include "ebm.h"
#include "aux.h"
#include "dbg.h"
#include "ftm_common.h"
#include "dm.h"


uint8_t cpuId;
uint8_t cpuQty;



/// Init. Discovers periphery and inits all modules.
void init()
{

  discoverPeriphery();
  cpuId = getCpuIdx();
  uart_init_hw();

  isr_table_clr();
  irq_set_mask(0x01);
  irq_disable();

  for (int j = 0; j < ((125000000/8)); ++j) { asm("nop"); }
  
  atomic_on(); mprintf("#%02u: Rdy\n", cpuId); atomic_off();

}



void main(void) {

    init();

    mprintf("#%02u: Waiting 1s before 64b fuzzing adr 0x%08x. Interval %u, hi/lo delay %u\n", cpuId);
    for (uint32_t j = 0; j < ((125000000/4)); ++j) { asm("nop"); }
                                           
    volatile uint32_t* dst = (uint32_t*)0x84120000; //cpu 0 target adr
    uint64_t interval   = 20000000ULL;  //20ms
    uint64_t delay      = 5000ULL;      //500us
    uint8_t  hi_first   = 1;
    uint64_t preptime   = 2000000ULL;  //2ms
    uint64_t val        = getSysTime()+preptime; //start value now + 2ms

    while (1) {
      ///if time not yet reached, wait
      uint64_t now = getSysTime();
      uint64_t due = val - preptime;
      if (now > due) {for (uint64_t j = 0; j < ((now - due)>>2); ++j) { asm("nop"); } continue;};  
      
      if (hi_first) {
        //write high dword first
        dst[0] = (uint32_t)(val >> 32);
        for (uint32_t j = 0; j < (delay >> 2); ++j) { asm("nop"); }
        dst[1] = (uint32_t)(val);
      } else {
        //write low dword first
        dst[1] = (uint32_t)(val);
        for (uint32_t j = 0; j < (delay >> 2); ++j) { asm("nop"); }
        dst[0] = (uint32_t)(val >> 32);
      }
      val += interval; //increment every loop
      
      
    }    


    while (1) { asm("nop"); }
  
}
