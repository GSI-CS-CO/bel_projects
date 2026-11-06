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
#include "wr_mil_config.h"
#include "wr_mil_delay.h"
#include "wr_mil_events.h"
#include "../../../top/gsi_scu/scu_mil.h"

int init()
{
  int cpu_id;
  discoverPeriphery();    // mini-sdb: get info on important Wishbone infrastructure, such as (this) CPU, flash, ...
  cpu_id = getCpuIdx();   // get ID of THIS CPU
  irq_disable();          // ...
  return cpu_id;
}

// write 16bit word (as command) on MIL device bus that will mimic a Mil timing event
uint32_t mil_piggy_write_event(volatile uint32_t *piggy, uint32_t cmd)
{
  piggy[MIL_SIO3_TX_CMD] = cmd;
  return 0;
}

void inject_mil_event(uint32_t cmd, TAI_t time) 
{
  // protect from nonsense hi-frequency bursts
  static uint64_t previous_time = 0;
  if (time.value < previous_time+25000) {
    time.value = previous_time+25000;
  }
  previous_time = time.value;

  // inject event 
  atomic_on();
  *pEca = 0xffffffff;
  *pEca = cmd & 0x0000ffff;
  *pEca = 0;
  *pEca = 0;
  *pEca = 0;
  *pEca = 0;
  *pEca = time.part.hi;
  *pEca = time.part.lo;
  atomic_off();
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
  {                                       //                p: Pulszentralenkennung ( SIS = 1, ESR = 2)
    tophalf = ( pzKennung << 4 ) | 0xf;
  }
  else                                    // all other events: top haltf of MIL bits (15..8) are ppppvvvv
  {                                       //                     p: Pulszentralenkennung ( SIS = 1, ESR = 2)
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


uint32_t inhibit_fill_events = 0; // this is a counter to block any sending of fill events for some cycles after a real event was sent

#define MIL_PIGGY_SEND_LATENCY 25000      // nanoseconds from pushing to mil piggy queue to last transition on the mil bus
#define MICROSECONDS           1000       // so many nanoseconds per microsecond
#define RESET_INHIBIT_COUNTER  1000       // count so many main loops before the inhibit for fill event sending is released
#define N_UTC_EVENTS           5          // number of generated EVT_UTC events
#define ECA_QUEUE_LM32_TAG     0x00000004 // the tag for ECA actions we (the LM32) want to receive
#define WR_MIL_GATEWAY_LATENCY 70650      // additional latency in units of nanoseconds
                                          // this value was determined by measuring the time difference
                                          // of the MIL event rising edge and the ECA output rising edge (no offset)
                                          // and tuning this time difference to 100.0(5)us
void eventHandler(volatile uint32_t    *eca_queue, 
                  volatile uint32_t    *mil_piggy,
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
      // add latency to the deadline and subtract the transmission time of one MIL event
      tai_deadl.value += ((int32_t)config->latency)*MICROSECONDS - MIL_PIGGY_SEND_LATENCY;
      inject_mil_event(milTelegram, tai_deadl);
      ECAQueue_actionPop(eca_queue); // remove event from queue
      ++config->num_events.value;
      ++config->mil_histogram[milTelegram & 0xff];
      if (evtCode == config->utc_trigger)
      {
        // generate EVT_UTC_1/2/3/4/5 EVENTS
        make_mil_timestamp(tai_deadl.value, EVT_UTC, config->utc_offset_ms.value);
        tai_deadl.value += config->trigger_utc_delay*MICROSECONDS;
        for (int i = 0; i < N_UTC_EVENTS; ++i)
        {
          tai_deadl.value += config->utc_delay*MICROSECONDS; // some delay between utc events
          inject_mil_event(EVT_UTC[i], tai_deadl);
          ++config->mil_histogram[EVT_UTC[i] & 0xff];
          ++config->num_events.value;
        }
      }
      // a mil event was just sent, any pending send-fill request it is invalidated
      config->request_fill_evt = 0;
      inhibit_fill_events = RESET_INHIBIT_COUNTER;
    }
  }
  // take care of sending fill events when it is appropriate
  if (inhibit_fill_events) {
    --inhibit_fill_events;
  }
  if (config->request_fill_evt) 
  {
    // there was a request and the last mil event is so far in the past that the inhibit counter is zero
    //  so lets send the fill event, clear the request and reset inhibit counter
    mil_piggy_write_event(mil_piggy, MIL_EVT_FILL); 
    ++config->mil_histogram[MIL_EVT_FILL & 0xff];
    //++config->num_events.value; // fill events don't count as mil events
    config->request_fill_evt = 0;
    inhibit_fill_events = RESET_INHIBIT_COUNTER;
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

  // Command
  volatile WrMilConfig *config = config_init();
  pp_printf("mil cmd regs at %08x\n", config);

  mil_piggy_reset(mil_piggy);

  uint32_t i = 0;
  uint32_t debug_numbers[6] = {0,};
  while (1) {
    //poll user commands
    config_poll(config);

    if (config->state == WR_MIL_GW_STATE_UNCONFIGURED)
    {
      ECAQueue_clear(eca_queue);
    }
    // do whatever has to be done
    if (config->state == WR_MIL_GW_STATE_CONFIGURED)
    {
      eventHandler(eca_queue, mil_piggy, config);
    }

    DELAY1us; // little delay 
  } 
} 
