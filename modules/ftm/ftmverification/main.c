#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include "mini_sdb.h"
#include "aux.h"
#include "ftm.h"
#include "dbg.h"

volatile unsigned int* pEC;
uint32_t errCnt, actCnt; 

#define ECA_ACTION      0x9bfa4560

#define ECA_ACT_CTL     0x00 // RW : Control
#define ECA_CMD_POP     0x1
#define ECA_QUEACT      0x10 // R : queued actions
#define ECA_FLAGS       0x1C // R : flags
#define ECA_EVT_ID_HI   0x20
#define ECA_EVT_ID_LO   0x24
#define ECA_EVT_PA_HI   0x28
#define ECA_EVT_PA_LO   0x2C
#define ECA_EVT_TAG     0x30
#define ECA_EVT_TEF     0x34
#define ECA_EVT_TI_HI   0x38
#define ECA_EVT_TI_LO   0x3C


char buffer[12];

void report(uint64_t now)
{
   //acquire info on offending evt
   
   uint64_t id    = ((uint64_t)*(pEC + (ECA_EVT_ID_HI >> 2))) << 32 | (uint64_t)*(pEC + (ECA_EVT_ID_LO >> 2));
   uint64_t par   = ((uint64_t)*(pEC + (ECA_EVT_PA_HI >> 2))) << 32 | (uint64_t)*(pEC + (ECA_EVT_PA_LO >> 2));
   uint64_t time  = (((uint64_t)*(pEC + (ECA_EVT_TI_HI >> 2))) << 32 | (uint64_t)*(pEC + (ECA_EVT_TI_LO >> 2))) << 3;
    
   mprintf("################## %10u/%10u ##################", errCnt, actCnt);
   if( *(pEC + (ECA_FLAGS  >> 2)) & 0x01 ) mprintf("late ");
   if( *(pEC + (ECA_FLAGS  >> 2)) & 0x02 ) mprintf("conflict ");
   mprintf("\n");
   mprintf("id:\t%08x%08x\nFID:\t%u\nGID:\t%u\nEVTNO:\t%u\nSID:\t%u\nBPID:\t%u\npar:\t%08x%08x\ntef:\t\t%08x\ntag:\t\t%08x\nts:\t%08x%08x\nnow:\t%08x%08x\ndif:\t%d\n", 
   (uint32_t)(id>>32), (uint32_t)id, getIdFID(id), getIdGID(id), getIdEVTNO(id), getIdSID(id), getIdBPID(id), 
   (uint32_t)(par>>32), (uint32_t)par,
   *(pEC + (ECA_EVT_TEF >> 2)),
   *(pEC + (ECA_EVT_TAG >> 2)),
   (uint32_t)(time>>32), (uint32_t)time,
   (uint32_t)(now>>32), (uint32_t)now,
   ((int32_t)((int64_t)(time<<3) - (int64_t)(now<<3)))
   ); 
 
}

void init()
{

   discoverPeriphery();
   pEC = find_device_adr(GSI, ECA_ACTION);
   
   uart_init_hw();
   uart_write_string("\nDebug Port\n");
 
   mprintf("\fCore #%u scanning ECA @ 0x%08x for late arrivals / conflicts\n", getCpuIdx(), (uint32_t)pEC); 
}

void main(void) {


  int j;
  uint64_t now;
  uint32_t data;
  
  init();
  
  for (j = 0; j < (125000000/4); ++j) {
        asm("# noop"); // no-op the compiler can't optimize away
      }
  
  
  while (1) {
  
     now = getSysTime(); 
     if( *(pEC + (ECA_QUEACT >> 2))) { // if there is stuff in the action queue ...
        actCnt++;
        if( *(pEC + (ECA_FLAGS  >> 2)) ) {    // check for errors, if so, gather info on offending event and printf preport
          errCnt++;
          report(now<<3);
        }
        *(pEC + (ECA_ACT_CTL >> 2)) = ECA_CMD_POP; //... pop the element
     }
  
  
  
  }

}
