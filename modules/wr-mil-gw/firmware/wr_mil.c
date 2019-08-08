/********************************************************************************************
 *  wr_mil.c 
 *   based on ../lm32-example/example.c
 *
 *  created : 2017
 *  author  : Michael Reese, Dietrich Beck, Mathias Kreider GSI-Darmstadt
 *  version : 21-Jun-2017
 *
 * -------------------------------------------------------------------------------------------
 * License Agreement for this software:
 *
 * Copyright (C) 2017  Michael Reese
 * GSI Helmholtzzentrum für Schwerionenforschung GmbH
 * Planckstraße 1
 * D-64291 Darmstadt
 * Germany
 *
 * Contact: m.reese@gsi.de
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
 * For all questions and ideas contact: m.reese@gsi.de
 * Last update: 25-April-2015
 ********************************************************************************************/

/* standard includes */
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>

/* includes specific for bel_projects */
#include "pp-printf.h"
#include "mini_sdb.h"
#include "irq.h"
#include "aux.h"
#include "dbg.h"

/* local includes for wr_mil firmware*/
#include "wr_mil_value64bit.h"
#include "wr_mil_eca_queue.h"
#include "wr_mil_eca_ctrl.h"
#include "wr_mil_config.h"
#include "wr_mil_delay.h"
#include "wr_mil_events.h"
#include "wr_mil_msi.h"
#include "wr_mil_oled.h"
#include "../../../top/gsi_scu/scu_mil.h"

// for the event handler
//#include "../../ip_cores/saftlib/drivers/eca_flags.h"

int init()
{
  int cpu_id;
  discoverPeriphery();    // mini-sdb: get info on important Wishbone infrastructure, such as (this) CPU, flash, ...
  //uart_init_hw();         // init UART, required for printf... . To view print message, you may use 'eb-console' from the host
  cpu_id = getCpuIdx();            // get ID of THIS CPU
  isr_table_clr();        // set MSI IRQ handler
  irq_set_mask(0x01);     // ...
  irq_disable();          // ...
  return cpu_id;
}

// write 16bit word (as command) on MIL device bus that will mimic a Mil timing event
uint32_t mil_piggy_write_event(volatile uint32_t *piggy, uint32_t cmd)
{
  piggy[MIL_SIO3_TX_CMD] = cmd;
  return 0;
}

uint32_t mil_piggy_reset(volatile uint32_t *piggy)
{
  piggy[MIL_SIO3_RST] = 0x0;
  DELAY1000us;
  piggy[MIL_SIO3_RST] = 0xff;
  DELAY100us;
  return 0;
}


uint32_t convert_WReventID_to_milTelegram(EvtId_t evtId, uint32_t *evtCode, uint32_t *milTelegram, uint32_t evtSource)
{
  // EventID 
  // |---------------evtIdHi---------------|  |---------------evtIdLo---------------|
  // FFFF GGGG GGGG GGGG EEEE EEEE EEEE FFFF  SSSS SSSS SSSS BBBB BBBB BBBB BBRR RRRR
  //                          cccc cccc irrr  ssss ssss vvvv
  //                                               xxxx xxxx
  //                              
  // F: FID(4)
  // G: GID(12)
  // E: EVTNO(12) = evtNo
  // F: FLAGS(4)
  // S: SID(12)
  // B: BPID(14)
  // R: Reserved(10)
  // s: status bits
  // v: virtAcc = virtual accellerator
  // c: evtCode = MIL relevant part of the evtNo (only 0..255)
  // i: InBeam(1)
  // r: reserved(3)
  // x: other content for special events like (code=200..208 command events)
  //    or (code=200 beam status event) 

  uint32_t evtNo   = (evtId.part.hi>>4)  & 0x00000fff;
          *evtCode =  evtNo              & 0x000000ff;
  uint32_t tophalf = (evtId.part.lo>>20) & 0x000000ff; // the top half bits (15..8) of the mil telegram.
                                                       // the meaning depends on the evtCode.
  uint32_t virtAcc     = tophalf & 0xf;
  uint32_t statusBits  = tophalf >> 4;
  uint32_t pzKennung   = evtSource;

  if (*evtCode >= 200 && *evtCode <= 208) // commands: top half of the MIL bits (15..8) are extracted from the status bits of EventID
  {
    // tophalf = tophalf;  // no modification (just take all bits from the sequence-ID)
  }
  else if (*evtCode == 255)               // command event: top half of the MIL bits (15..8) are pppp1111
  {                                       //                p: Pulszentralenkennung ( SIS = 1, ESR = 2[?])
    tophalf = ( pzKennung << 4 ) | 0xf;
  }
  else                                    // all other events: top haltf of MIL bits (15..8) are ppppvvvv
  {                                       //                     p: Pulszentralenkennung ( SIS = 1, ESR = 2[?])
                                          //                     v: virtial accelerator
    tophalf = ( pzKennung << 4 ) | virtAcc;
  }

  *milTelegram = (tophalf << 8) | *evtCode; 
                                           
  // For MIL events, the upper 4 bits ov evtNo are zero
  return (evtNo & 0x00000f00) == 0; 
}


// convert 64-bit TAI from WR into an array of five MIL events (EVT_UTC_1/2/3/4/5 events with evtNr 0xE0 - 0xE4)
// arguments:
//   TAI:     a 64-bit WR-TAI value
//   EVT_UTC: points to an array of 5 uint32_t and will be filled 
//            with valid special MIL events:
//                            EVT_UTC_1 = EVT_UTC[0] =  ms[ 9: 2]          , code = 0xE0
//                            EVT_UTC_2 = EVT_UTC[1] =  ms[ 1: 0] s[30:25] , code = 0xE1
//                            EVT_UTC_3 = EVT_UTC[2] =   s[24:16]          , code = 0xE2
//                            EVT_UTC_4 = EVT_UTC[3] =   s[15: 8]          , code = 0xE3
//                            EVT_UTC_5 = EVT_UTC[4] =   s[ 7: 0]          , code = 0xE4
//            where s is a 30 bit number (seconds since 2008) and ms is a 10 bit number
//            containing the  milisecond fraction.
void make_mil_timestamp(uint64_t TAI, uint32_t *EVT_UTC, uint64_t UTC_offset_ms)
{
  uint64_t msNow  = TAI / UINT64_C(1000000); // conversion from ns to ms (since 1970)
  //uint64_t ms2008 = UINT64_C(1199142000000); // miliseconds at 01/01/2008  (since 1970)
                                             // the number was caluclated using: date --date='01/01/2008' +%s
  uint64_t mil_timestamp_ms = msNow - UTC_offset_ms;//ms2008;
  uint32_t mil_ms           = mil_timestamp_ms % 1000;
  uint32_t mil_sec          = mil_timestamp_ms / 1000;

  // The following converion code for the UTC timestamps is based on 
  // some sample code that was kindly provided by Peter Kainberger.
  union UTCtime_t
  {
    uint8_t bytes[8];
    struct {
      uint32_t timeMs;
      uint32_t timeS;
    } bit;
  } utc_time = { .bit.timeS  =  mil_sec & 0x3fffffff,
                 .bit.timeMs = (mil_ms & 0x3ff) << 6 };

  EVT_UTC[0] =  utc_time.bytes[2] *256 + MIL_EVT_UTC_1;
  EVT_UTC[1] = (utc_time.bytes[3] | 
                utc_time.bytes[4])*256 + MIL_EVT_UTC_2;
  EVT_UTC[2] =  utc_time.bytes[5] *256 + MIL_EVT_UTC_3;
  EVT_UTC[3] =  utc_time.bytes[6] *256 + MIL_EVT_UTC_4;
  EVT_UTC[4] =  utc_time.bytes[7] *256 + MIL_EVT_UTC_5;
}


uint64_t mil_event_time = 0; // this is global so that main loop can see when the last MIL-Event was sent

#define N_UTC_EVENTS           5          // number of generated EVT_UTC events
#define ECA_QUEUE_LM32_TAG     0x00000004 // the tag for ECA actions we (the LM32) want to receive
#define WR_MIL_GATEWAY_LATENCY 70650      // additional latency in units of nanoseconds
                                          // this value was determined by measuring the time difference
                                          // of the MIL event rising edge and the ECA output rising edge (no offset)
                                          // and tuning this time difference to 100.0(5)us
void eventHandler(TAI_t tai_now,
                  volatile uint32_t    *eca,
                  volatile uint32_t    *eca_queue, 
                  volatile uint32_t    *mil_piggy,
                  volatile uint32_t    *oled,
                  volatile WrMilConfig *config)
{
  if (ECAQueue_actionPresent(eca_queue))
  {
    uint32_t evtCode, milTelegram;
    EvtId_t  evtId;
    TAI_t    tai_deadl; 
    // read info about event from eca queue
    ECAQueue_getEvtId(eca_queue, &evtId);
    ECAQueue_getDeadl(eca_queue, &tai_deadl);
    // select all events from the eca queue that are for the LM32 
    // AND that have an evtNo that is supposed to be translated into a MIL event (indicated
    //     by the return value of ECAQueue_getMilEventData being != 0)
    if ((ECAQueue_getActTag(eca_queue) == ECA_QUEUE_LM32_TAG) &&
         convert_WReventID_to_milTelegram(evtId, &evtCode, &milTelegram, config->event_source))
    {
      uint32_t EVT_UTC[N_UTC_EVENTS];
      uint32_t too_late;
      uint32_t trials;
      mil_event_time = tai_deadl.value + WR_MIL_GATEWAY_LATENCY + ((int32_t)config->latency-100)*1000; // add latency to the deadline

      // precision wait 
      too_late = wait_until_tai(eca, mil_event_time, tai_now);
      // generate MIL event
      trials = mil_piggy_write_event(mil_piggy, milTelegram); 
      ECAQueue_actionPop(eca_queue); // remove event from queue
      ++config->num_events.value;
      ++config->mil_histogram[milTelegram & 0xff];
      if (evtCode == config->utc_trigger)
      {
        // generate EVT_UTC_1/2/3/4/5 EVENTS
        make_mil_timestamp(mil_event_time, EVT_UTC, config->utc_offset_ms.value);
        //delay_96plus32n_ns(config->trigger_utc_delay*32);
        for (int i = 0; i < N_UTC_EVENTS; ++i)
        {
          // Churn out the EVT_UTC MIL events with a configurable delay between the individual events.
          trials = mil_piggy_write_event(mil_piggy, EVT_UTC[i]); 
          if (i < N_UTC_EVENTS-1)
          {
            delay_96plus32n_ns(config->utc_delay*32);
          }
        }
        oled_array(config, oled); // mil piggy is busy writing events. 
                                   // use the time to update OLED display
      }
      if (too_late || trials)
      { 
        // inform saftlib that there was a late event
        send_MSI(config->mb_slot, WR_MIL_GW_MSI_LATE_EVENT);
        ++config->late_events;
        //pp_printf("evtCode: %u trials: %u  late: %u\n",evtCode, trials, too_late);
        for (int i = 0; i < 16; ++i) {
          if (too_late>>(i+10) == 0 || i == 15) {
            ++config->late_histogram[i];
            break;
          } 
        }
      }
    }
  }
}


// After some initialization, the program enters a tight loop where the event handler is called.
// in the event handler, the ECA queue is polled for events and if the event number is in the range [0...255]
// a MIL event is generated. If the event number matches the WR_MIL_GW_REG_UTC_TRIGGER register in shared memory
// in shared memory (default is MIL_EVT_END_CYCLE) five MIL events are generated in addition to the requested event.
// The UTC events contain a converted  WR timestamp with milisecond precision.
void main(void) 
{
  init();   

  // MilPiggy 
  volatile uint32_t *mil_piggy = (volatile uint32_t*) find_device_adr(GSI, SCU_MIL);
  pp_printf("mil_piggy adr: %08x\n", mil_piggy);

  // ECAQueue 
  volatile uint32_t *eca_queue = ECAQueue_init();
  pp_printf("eca_queue adr: %08x\n", eca_queue);
  uint32_t n_events = ECAQueue_clear(eca_queue);
  pp_printf("popped %d events from the eca queue\n", n_events);

  // ECACtrl 
  volatile uint32_t *eca_ctrl = ECACtrl_init();
  pp_printf("eca ctrl regs at %08x\n", eca_ctrl);

  // Command
  volatile WrMilConfig *config = config_init();
  pp_printf("mil cmd regs at %08x\n", config);


  volatile uint32_t *oled = (volatile uint32_t*) find_device_adr(GSI, 0x93a6f3c4);
  oled[0] = 0;

  // // Where is the MSI message box
  // pp_printf("pCpuMsiBox %08x      pMyMsi %08x\n", pCpuMsiBox, pMyMsi);
  // config->mb_slot = getMsiBoxSlot(0xa0);
  // pp_printf("mb_slot %d\n", config->mb_slot);

  // say hello on the console
  TAI_t nowTAI; 
  ECACtrl_getTAI(eca_ctrl, &nowTAI);
  pp_printf("TAI now: 0x%08x%08x\n", nowTAI.part.hi, nowTAI.part.lo);

  mil_piggy_reset(mil_piggy);


  oled_array(config, oled);

  int i = 0;
  while (1) {
    //poll user commands
    config_poll(config, oled);
    if (config->state == WR_MIL_GW_STATE_UNCONFIGURED)
    {
      ECAQueue_clear(eca_queue);
    }
    // do whatever has to be done
    if (config->state == WR_MIL_GW_STATE_CONFIGURED)
    {
      ECACtrl_getTAI(eca_ctrl, &nowTAI);
      if (nowTAI.value - mil_event_time > 10000000000) { // if gateway wasn't used, generate MIL fill events
        const fill_event = 0x000000c7;
        mil_piggy_write_event(mil_piggy, fill_event); 
        mil_event_time = nowTAI.value;
        ++config->mil_histogram[fill_event & 0xff];
      }
      eventHandler(nowTAI, eca_ctrl, eca_queue, mil_piggy, oled, config);
    }

    DELAY1us; // little delay 
  } 
} 
