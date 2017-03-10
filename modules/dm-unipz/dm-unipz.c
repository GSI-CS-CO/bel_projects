/********************************************************************************************
 *  dm-unipz.c
 *
 *  created : 2017
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 02-Mar-2017
 *
 *  lm32 program for gateway between UNILAC Pulszentrale and FAIR style Data Master
 * 
 * -------------------------------------------------------------------------------------------
 * License Agreement for this software:
 *
 * Copyright (C) 2017  Dietrich Beck
 * GSI Helmholtzzentrum für Schwerionenforschung GmbH
 * Planckstraße 1
 * D-64291 Darmstadt
 * Germany
 *
 * Contact: d.beck@gsi.de
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For all questions and ideas contact: d.beck@gsi.de
 * Last update: 25-April-2015
 ********************************************************************************************/

/* standard includes */
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include "mprintf.h"
#include "mini_sdb.h"

/* includes specific for bel_projects */
#include "irq.h"
#include "ebm.h"
#include "aux.h"
#include "dbg.h"
#include "../../ip_cores/wr-cores/modules/wr_eca/eca_queue_regs.h"

/* register maps for some selected Wishbone devices  */
#include "../../tools/wb_slaves.h" /* this is a hack */

/* shared memory map for communication via Wishbone  */
#include "dmunipz_shared_mmap.h"
#include "dm-unipz_smmap.h"

/* stuff required for environment */
extern uint32_t*       _startshared[];
unsigned int cpuId, cpuQty;
#define SHARED __attribute__((section(".shared")))
uint64_t SHARED dummy = 0;

uint32_t *pECAQ;              /* WB address of ECA queue                                                    */

uint32_t *pShared;            /* pointer to begin of shared memory region                                   */
uint32_t *pSharedStatus;      /* pointer to a "user defined" u32 register; here: publish status             */
volatile uint32_t *pSharedCmd;   /* pointer to a "user defined" u32 register; here: get commnand from host  */
volatile uint32_t *pSharedData1; /* pointer to a "user defined" u32 register; here: data                    */
volatile uint32_t *pSharedData2; /* pointer to a "user defined" u32 register; here: data                    */
volatile uint32_t *pSharedData3; /* pointer to a "user defined" u32 register; here: data                    */
volatile uint32_t *pSharedData4; /* pointer to a "user defined" u32 register; here: data                    */

uint32_t *pCpuRamExternal;    /* external address (seen from host bridge) of this CPU's RAM                 */
uint32_t *pSharedData1External; /* external address (seen from host bridge) of Data                         */
uint32_t *pSharedData2External; /* external address (seen from host bridge) of Data                         */
uint32_t *pSharedData3External; /* external address (seen from host bridge) of Data                         */
uint32_t *pSharedData4External; /* external address (seen from host bridge) of Data                         */

uint32_t *pRemotePPSGen;        /* WB address of remote PPS_GEN */
uint32_t *pRemotePPSSecs;       /* TAI full seconds             */
uint32_t *pRemotePPSNsecs;      /* TAI nanoseconds part         */

uint32_t *pLocalPPSGen;         /* WB address of PPS_GEN        */
uint32_t *pLocalPPSSecs;        /* TAI full seconds             */
uint32_t *pLocalPPSNsecs;       /* TAI nanoseconds part         */

/*
void show_msi()
{
  mprintf(" Msg:\t%08x\nAdr:\t%08x\nSel:\t%01x\n", global_msi.msg, global_msi.adr, global_msi.sel);
}

void isr0()
{
  mprintf("ISR0\n");   
  show_msi();
}
*/


void ebmInit()
{
  int j;
  
  while (*(pEbCfg + (EBC_SRC_IP>>2)) == EBC_DEFAULT_IP) {
    for (j = 0; j < (125000000/2); ++j) { asm("nop"); }
    mprintf("#%02u: DM cores Waiting for IP from WRC...\n", cpuId);  
  } /* pEbCfg */
  
  ebm_init();
  ebm_config_meta(1500, 42, 0x00000000 );

  ebm_config_if(DESTINATION, 0x00267b00022b, 0xc0a81410,                  0xebd0); //Dst: EB broadcast - CAREFUL HERE!
  ebm_config_if(SOURCE,      0x00267b000401, *(pEbCfg + (EBC_SRC_IP>>2)), 0xebd0); //Src: MAC is a hack!, WR IP

  mprintf("my IP:  0x%08x\n",  *(pEbCfg + (EBC_SRC_IP>>2)));
  mprintf("pEbCfg: 0x%08x\n",  pEbCfg);
  ebm_clr();
} /* ebminit */


void init()
{
  discoverPeriphery();           /* mini-sdb ...             */
  uart_init_hw();                
  cpuId = getCpuIdx();

  /* set MSI IRQ handler */
  isr_table_clr();
  irq_set_mask(0x01);
  irq_disable(); 
} /* init */

void initRemotePPSGen(){
  int      i;

  /* get Wishbone address of remote  WR PPS GEN                 */
  pRemotePPSGen  = (uint32_t *)0x8060300;   /* this is a hack    */

  /* get Wishbone addresses of (nano)seconds counters           */
  pRemotePPSSecs  = (uint32_t *)(pRemotePPSGen + (WR_PPS_GEN_CNTR_UTCLO >> 2));
  pRemotePPSNsecs = (uint32_t *)(pRemotePPSGen + (WR_PPS_GEN_CNTR_NSEC >> 2));
} /* init external PPS */

void initLocalPPSGen(){
  /* get Wishbone address of local WR PPS GEN                   */
  pLocalPPSGen    = find_device_adr(WR_PPS_GEN_VENDOR, WR_PPS_GEN_PRODUCT);

  /* get Wishbone addresses of (nano)seconds counters           */
  pLocalPPSSecs   = (uint32_t *)(pLocalPPSGen + (WR_PPS_GEN_CNTR_UTCLO >> 2));
  pLocalPPSNsecs  = (uint32_t *)(pLocalPPSGen + (WR_PPS_GEN_CNTR_NSEC >> 2));
} /* init local PPS */


void getWishboneTAI(uint32_t *secs, uint32_t *nsecs)
{
  /* get data from WR PPS GEN and print to UART              */
  *secs  = *pLocalPPSSecs;
  *nsecs = *pLocalPPSNsecs;
} /* getWishboneTAI */

void getEtherboneTAI(uint32_t *secs, uint32_t *nsecs)
{
  int i;
  
  /* setup and commit EB cycle to remote device */
  ebm_hi((uint32_t)pRemotePPSSecs);
  ebm_op((uint32_t)pRemotePPSSecs, (uint32_t)pSharedData1External, EBM_READ);
  ebm_op((uint32_t)pRemotePPSNsecs,(uint32_t)pSharedData2External, EBM_READ);
  ebm_flush();

  *secs  = 0x0;
  *nsecs = 0x0;

  /* reset shared data                                          */
  *pSharedData1 = 0x0;
  *pSharedData2 = 0x0;

  /* wait until timeout values are received via shared mem or timeout */
  i=0;
  while (! (*pSharedData1)) {
    i++;
    if (i > 125000000/16) break; /* timeout */
    asm("nop");
  } /* while */

  if (*pSharedData1 > 0) {
    i = 0;
    *secs  = *pSharedData1;
    *nsecs = *pSharedData2;
  } /* if */

    
} /* getEtherboneTAI */

void checkSync(uint32_t *inSync, uint32_t *dT)
{
  uint32_t rSecs, lSecs1, lSecs2; 
  uint32_t rNsecs, lNsecs1, lNsecs2;

  uint32_t meanLNsecs;
  uint32_t j;

  getWishboneTAI(&lSecs1, &lNsecs1);

  /* wait until the next full second */
  lSecs2 = lSecs1;
  while (lSecs2 == lSecs1) {
    getWishboneTAI(&lSecs2, &lNsecs2);
    asm("nop");
  } 

  /* wait for 100ms to be on the safe side */
  for (j = 0; j < (125000000/40); ++j) { asm("nop"); }
  
  getWishboneTAI(&lSecs1, &lNsecs1);
  getEtherboneTAI(&rSecs, &rNsecs);
  getWishboneTAI(&lSecs2, &lNsecs2);

  /* if same number of seconds: assume we are in synch */
  *inSync = ((lSecs1 == lSecs2) && (lSecs1 == rSecs));
  if (*inSync) {
    /* get mean time to correct for execution time */
    meanLNsecs = ((lNsecs1 + lNsecs2) >> 1);

    if (meanLNsecs > rNsecs) *dT = meanLNsecs - rNsecs;
    else                     *dT = rNsecs - meanLNsecs;
  } /* if in sync */
  else *dT = 0;
  mprintf("\n\n\nelapsed time : %09u ns\n", (lNsecs2 - lNsecs1));
  
  mprintf("local  WB TAI: %08u.%09u\n", lSecs1, lNsecs1);
  mprintf("remote EB TAI: %08u.%09u\n", rSecs, rNsecs);
  mprintf("local  WB TAI: %08u.%09u\n", lSecs2, lNsecs2);

  mprintf("in sync: %u; dT = %09u ns\n", *inSync, *dT);
} /* checkSynch */


void initSharedMem()
{
  int      i,j;
  uint32_t idx;
  const uint32_t c_Max_Rams = 10;
  sdb_location found_sdb[c_Max_Rams];
  sdb_location found_clu;

  /* get pointer to shared memory       */                    
  pShared        = (uint32_t *)_startshared;
  pSharedStatus  = (uint32_t *)(pShared + (DMUNIPZ_SHARED_STATUS >> 2));
  pSharedCmd     = (uint32_t *)(pShared + (DMUNIPZ_SHARED_CMD >> 2));
  pSharedData1   = (uint32_t *)(pShared + (DMUNIPZ_SHARED_DATA1 >> 2));
  pSharedData2   = (uint32_t *)(pShared + (DMUNIPZ_SHARED_DATA2 >> 2));
  pSharedData3   = (uint32_t *)(pShared + (DMUNIPZ_SHARED_DATA3 >> 2));
  pSharedData4   = (uint32_t *)(pShared + (DMUNIPZ_SHARED_DATA4 >> 2));

  /* print local pointer info to UART   */
  mprintf("internal shared memory: start            @ 0x%08x\n", (uint32_t)pShared);
  mprintf("internal shared memory: status address   @ 0x%08x\n", (uint32_t)pSharedStatus);
  mprintf("internal shared memory: data1  address   @ 0x%08x\n", (uint32_t)pSharedData1);
  mprintf("internal shared memory: data2  address   @ 0x%08x\n", (uint32_t)pSharedData2);
  mprintf("internal shared memory: data3  address   @ 0x%08x\n", (uint32_t)pSharedData3);
  mprintf("internal shared memory: data4  address   @ 0x%08x\n", (uint32_t)pSharedData4);

  idx = 0;
  find_device_multi(&found_clu, &idx, 1, GSI, LM32_CB_CLUSTER);	
  idx = 0;
  find_device_multi_in_subtree(&found_clu, &found_sdb[0], &idx, c_Max_Rams, GSI, LM32_RAM_USER);
  if(idx >= cpuId) {
    pCpuRamExternal = (uint32_t*)(getSdbAdr(&found_sdb[cpuId]) & 0x7FFFFFFF); // CPU sees the 'world' under 0x8..., remove that bit to get host bridge perspective

    /*
    pSharedData1External = (uint32_t *)(pEbCfg + ((DMUNIPZ_SHARED_DATA1  + SHARED_OFFS) >> 2));
    pSharedData2External = (uint32_t *)(pEbCfg + ((DMUNIPZ_SHARED_DATA2  + SHARED_OFFS) >> 2));
    pSharedData3External = (uint32_t *)(pEbCfg + ((DMUNIPZ_SHARED_DATA3  + SHARED_OFFS) >> 2));
    pSharedData4External = (uint32_t *)(pEbCfg + ((DMUNIPZ_SHARED_DATA4  + SHARED_OFFS) >> 2));
    */

    pSharedData1External = (uint32_t *)(pCpuRamExternal + ((DMUNIPZ_SHARED_DATA1  + SHARED_OFFS) >> 2));
    pSharedData2External = (uint32_t *)(pCpuRamExternal + ((DMUNIPZ_SHARED_DATA2  + SHARED_OFFS) >> 2));
    pSharedData3External = (uint32_t *)(pCpuRamExternal + ((DMUNIPZ_SHARED_DATA3  + SHARED_OFFS) >> 2));
    pSharedData4External = (uint32_t *)(pCpuRamExternal + ((DMUNIPZ_SHARED_DATA4  + SHARED_OFFS) >> 2));
    
    /* print external WB info to UART     */
    mprintf("external WB address   : start            @ 0x%08x\n", (uint32_t)(pCpuRamExternal));
    mprintf("external WB address   : status           @ 0x%08x\n", (uint32_t)(pCpuRamExternal + ((DMUNIPZ_SHARED_STATUS + SHARED_OFFS) >> 2)));
    mprintf("external WB address   : data1            @ 0x%08x\n", (uint32_t)pSharedData1External);
    mprintf("external WB address   : data2            @ 0x%08x\n", (uint32_t)pSharedData2External);
    mprintf("external WB address   : data3            @ 0x%08x\n", (uint32_t)pSharedData3External);
    mprintf("external WB address   : data4            @ 0x%08x\n", (uint32_t)pSharedData4External);
  }
  else {
    pCpuRamExternal = (uint32_t*)ERROR_NOT_FOUND;
    mprintf("Could not find external WB address of my own RAM !\n");
  }
  
  /* initialize values of shared memory */
  *pSharedStatus = 0x0;
  *pSharedCmd    = 0x0;
  *pSharedData1  = 0x0;
  *pSharedData2  = 0x0;
  *pSharedData3  = 0x0;
  *pSharedData4  = 0x0;

} /* initSharedMem */


void initEca()
{
#define ECAQMAX           4
#define ECACHANNELFORLM32 2     /* this is a hack!           */
  /* stuff below needed to get WB address of ECA queue       */
  sdb_location ECAQ_base[ECAQMAX];
  uint32_t ECAQidx = 0;         
  uint32_t *tmp;                
  int i;

  /* get Wishbone address of ECA queue                       */
  /* get list of ECA queues                                  */
  find_device_multi(ECAQ_base, &ECAQidx, ECAQMAX, ECA_QUEUE_SDB_VENDOR_ID, ECA_QUEUE_SDB_DEVICE_ID);
  pECAQ = 0x0;
  /* find ECA queue connected to ECA chanel for LM32         */
  for (i=0; i < ECAQidx; i++) {
    tmp = (uint32_t *)(getSdbAdr(&ECAQ_base[i]));  
    if ( *(tmp + (ECA_QUEUE_QUEUE_ID_GET >> 2)) == ECACHANNELFORLM32) pECAQ = tmp;
  }
  mprintf("\n");
  if (!pECAQ) { mprintf("FATAL: can't find ECA queue for lm32, good bye! \n"); while(1) asm("nop"); }
  mprintf("ECA queue found at: %08x. Waiting for actions ...\n", pECAQ);
  mprintf("\n");

} /* initeca */


void ecaHandler()
{
  uint32_t flag;                /* flag for the next action  */
  uint32_t evtIdHigh;           /* high 32bit of eventID     */
  uint32_t evtIdLow;            /* low 32bit of eventID      */
  uint32_t evtDeadlHigh;        /* high 32bit of deadline    */
  uint32_t evtDeadlLow;         /* low 32bit of deadline     */
  uint32_t actTag;              /* tag of action             */

  /* read flag and check if there was an action              */
  flag         = *(pECAQ + (ECA_QUEUE_FLAGS_GET >> 2));
  if (flag & 0x10) { /* data is valid?                       */
    
    /* read data */
    evtIdHigh    = *(pECAQ + (ECA_QUEUE_EVENT_ID_HI_GET >> 2));
    evtIdLow     = *(pECAQ + (ECA_QUEUE_EVENT_ID_LO_GET >> 2));
    evtDeadlHigh = *(pECAQ + (ECA_QUEUE_DEADLINE_HI_GET >> 2));
    evtDeadlLow  = *(pECAQ + (ECA_QUEUE_DEADLINE_LO_GET >> 2));
    actTag       = *(pECAQ + (ECA_QUEUE_TAG_GET >> 2));
    
    /* pop action from channel */
    *(pECAQ + (ECA_QUEUE_POP_OWR >> 2)) = 0x1;

    /* here: do s.th. according to action                    */
    switch (actTag) {
    case 0x4:
      mprintf("EvtID: 0x%08x%08x; deadline: %08x%08x; flag: %08x\n", evtIdHigh, evtIdLow, evtDeadlHigh, evtDeadlLow, flag);
      break;
    default:
      mprintf("ecaHandler: unknown tag\n");
    } /* switch */

  } /* if data is valid */
} /* ecaHandler */


void initCmds()
{
  /* get pointer to shared memory */ 
  pSharedCmd     = (uint32_t *)(pShared + (DMUNIPZ_SHARED_CMD >> 2));

  /* print pointer info to UART */
  mprintf("\n");
  mprintf("internal shared memory: command address  @ 0x%08x\n", (uint32_t)pSharedCmd);
  mprintf("external WB address   : command          @ 0x%08x\n", (uint32_t)(pCpuRamExternal + ((DMUNIPZ_SHARED_CMD + SHARED_OFFS) >> 2)));
  mprintf("\n");

  /* initalize command value: 0x0 means 'no command        */
  *pSharedCmd     = 0x0;

  mprintf("Waiting for commands...\n");
} /* initCmds */


void cmdHandler()
{
  /* check, if a command has been issued (no cmd: 0x0)       */
  //  mprintf("value of command input : 0x%08x\n", *pSharedCmd);
  if (*pSharedCmd) {
    switch (*pSharedCmd) {
    case 0x1:
      mprintf("received cmd 0x1\n");
      break;
    case 0x2:
      mprintf("received cmd 0x2\n");
      break;
    case 0x3:
      mprintf("received cmd 0x3\n");
      break;
    default:
      mprintf("cmdHandler: unknown command '0x%08x'\n",*pSharedCmd);
    } /* switch */
    *pSharedCmd = 0x0; /* reset cmd value in shared memory   */
  } /* if command */
} /* ecaHandler */

void main(void) {
  
  int i,j;

  uint32_t inSync;
  uint32_t dT;
  
  /* initialize 'boot' lm32 */
  init();
  //initSharedMem();       /* read/write to shared memory      */
  //initEca();             /* init for actions from ECA        */
  //initCmds();            /* init for cmds from shared mem    */
  ebmInit();               /* init EB master                   */
  initSharedMem();         /* read/write to shared memory      */
  initRemotePPSGen();
  initLocalPPSGen();

  checkSync(&inSync, &dT);
  
  i=0;
  while (1) {
    /* do the things that have to be done                    */
    //ecaHandler();
    //cmdHandler();

    /* increment and update iteration counter                */
    i++;
    //*pSharedStatus = i;

    /* wait for 100  microseconds                            */
    for (j = 0; j < (125000000/40000); ++j) { asm("nop"); }
  } /* while */
} /* main */
