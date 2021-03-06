/* Synopsis */
/* ==================================================================================================== */
/*
 * @file device-test-vetar2a.cpp
 * @brief Simple IO test for Vetar2a board
 *
 * Copyright (C) 2014 GSI Helmholtz Centre for Heavy Ion Research GmbH 
 *
 * @author A. Hahn <a.hahn@gsi.de>
 *
 * @bug No know bugs.
 *
 * *****************************************************************************
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 * *****************************************************************************
 */

/* Includes */
/* ==================================================================================================== */
#include <etherbone.h>
#include <tlu.h>
#include <eca.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

/* Namespaces */
/* ==================================================================================================== */
using namespace GSI_ECA;
using namespace GSI_TLU;

/* Defines */
/* ==================================================================================================== */
#define HIGH_NS             50000000 /* Duration of a high pulse */
#define LOW_NS              50000000 /* Duration of a low pulse */
#define EVENTS              14       /* Events/GPIOs to toggle */
#define EVENT_MULTI         4        /* How often those events should occur */ 
#define SPECIAL_MUTLI       256      /* Needed for 200MHz input clock, will fill the TLU FIFO completely */
#define CPU_DELAY_MS        20       /* Sleep for test cpu */
#define TLU_CHANNELS        10       /* Inputs */
#define TOLERANCE_FACTOR_NS 2        /* Tolerance for frequency measurement */
#define GPIO_LVDS_RATE      40       /* Detection rate of HDMI clock signal */
#define GPIO_TOGGLE_RATE    ((EVENTS)*(HIGH_NS+LOW_NS))

/* Test case setup */
/* ==================================================================================================== */
uint32_t a_uEdgesExpectedTestCase1[EVENTS] = {EVENT_MULTI,  EVENT_MULTI,  EVENT_MULTI,    EVENT_MULTI,   EVENT_MULTI, 
                                              EVENT_MULTI,  0,            0,              0,             0          ,
                                              0,            0};
                                              
uint32_t a_uEdgesExpectedTestCase2[EVENTS] = {0,            0,            EVENT_MULTI,    EVENT_MULTI,  EVENT_MULTI, 
                                              EVENT_MULTI,  EVENT_MULTI,  SPECIAL_MUTLI,  EVENT_MULTI,  EVENT_MULTI,
                                              0,            0};
                                              
uint32_t a_uEdgesExpectedTestCase3[EVENTS] = {0,            0,            0,              0,            0, 
                                              0,            0,            0,              0,            0,
                                              0,            0};
                                              
uint32_t a_uFrequencyExpectedTestCase1[EVENTS] = {GPIO_TOGGLE_RATE,  GPIO_TOGGLE_RATE,  GPIO_TOGGLE_RATE,   GPIO_TOGGLE_RATE,   GPIO_TOGGLE_RATE, 
                                                  GPIO_TOGGLE_RATE,  0,                 0,                  0,                  0,
                                                  0,                 0};
                                                  
uint32_t a_uFrequencyExpectedTestCase2[EVENTS] = {0,                 0,                 GPIO_TOGGLE_RATE,   GPIO_TOGGLE_RATE,   GPIO_TOGGLE_RATE, 
                                                  GPIO_TOGGLE_RATE,  GPIO_TOGGLE_RATE,  GPIO_LVDS_RATE,     GPIO_TOGGLE_RATE,   GPIO_TOGGLE_RATE,
                                                  0,                 0};
                                                  
uint32_t a_uFrequencyExpectedTestCase3[EVENTS] = {0,                 0,                 0,                  0,                  0, 
                                                  0,                 0,                 0,                  0,                  0,
                                                  0,                 0};
/* Function main(...) */
/* ==================================================================================================== */
int main (int argc, const char** argv)
{

  /* Helpers */
  Socket socket;
  Device device;
  status_t status;
  std::vector<ECA> ecas;
  std::vector<TLU> tlus;
  std::vector<std::vector<uint64_t> > queues;
  Table table;
  uint32_t uQueueInterator = 0;
  uint32_t uQueueItemIterator = 0;
  uint32_t uQueuesTotal = 0;
  uint32_t uQueneItems = 0;
  uint64_t uTimeDiff = 0;
  uint32_t uShiftPosition = 0;
  uint32_t a_uEdges[EVENTS];
  uint64_t a_uFrequency[EVENTS];
  uint32_t uArrayIterator = 0;
  uint32_t uTestCase = 0;
  uint32_t uIOConfig = 0;
  uint32_t * p_uEdgesExpected = NULL;
  uint32_t * p_uFrequencyExpected = NULL;
  double dExpectedFrequency = 0.0;
  double dMeasuredFrequency = 0.0;
  
  /* Plausibility check for arguments */
  if (argc != 3)
  {
    fprintf(stderr, "%s: expecting argument <device> <test case>\n", argv[0]);
    return 1;
  }
  
  /* Initialize variables */
  for(uArrayIterator=0; uArrayIterator<EVENTS; uArrayIterator++)
  {
    a_uEdges[uArrayIterator] = 0;
    a_uFrequency[uArrayIterator] = 0;
  }
  
  /* Get the test case ID */
  if(!strcmp(argv[2],"testcase1"))
  {
    uTestCase = 1;
    uIOConfig = 0x0; /* All IOs = Inputs */
    p_uEdgesExpected = a_uEdgesExpectedTestCase1;
    p_uFrequencyExpected = a_uFrequencyExpectedTestCase1;
  }
  else  if(!strcmp(argv[2],"testcase2"))
  {
    uTestCase = 2;
    uIOConfig = 0x7; /* All IOs = Outputs */
    p_uEdgesExpected = a_uEdgesExpectedTestCase2;
    p_uFrequencyExpected = a_uFrequencyExpectedTestCase2;
  }
  else  if(!strcmp(argv[2],"testcase3"))
  {
    uTestCase = 3;
    uIOConfig = 0x0; /* All IOs = Inputs */
    p_uEdgesExpected = a_uEdgesExpectedTestCase3;
    p_uFrequencyExpected = a_uFrequencyExpectedTestCase3;
  }
  else
  {
    fprintf(stdout, "%s: unknown test case!\n", argv[0]);
    return 1;
  }
  fprintf(stdout, "%s: test case %d\n", argv[0], uTestCase);
  
  /* Try to open a (etherbone-) socket */
  socket.open();
  if ((status = device.open(socket, argv[1])) != EB_OK) 
  {
    fprintf(stderr, "%s: failed to open %s: (status %s)\n", argv[0], argv[1], eb_status(status));
    return 1;
  }
  else
  {
    fprintf(stdout, "%s: succeeded to open %s (status %s)\n", argv[0], argv[1], eb_status(status));
  }
  
  /* Find the ECA */
  ECA::probe(device, ecas);
  if (ecas.size() == 1)
  {
    fprintf(stdout, "%s: found one eca unit!\n", argv[0]);
  }
  else
  {
    fprintf(stdout, "%s: missing eca unit!\n", argv[0]);
    return 1;
  }
  ECA& eca = ecas[0];
  
  /* Find the TLU */
  TLU::probe(device, tlus);
  assert (tlus.size() == 1);
  TLU& tlu = tlus[0];
  /* Configure the TLU to record rising edge timestamps */
  tlu.hook(-1, false);
  tlu.set_enable(false); // no interrupts, please
  tlu.clear(-1);
  tlu.listen(-1, true, true, 8); /* Listen on all inputs */
  
  /* Find the IO reconfig to enable/disable outputs to specific IOs */
  std::vector<sdb_device> devs;
  device.sdb_find_by_identity(0x651, 0x4d78adfdU, devs);
  assert (devs.size() == 1);
  address_t ioconf = devs[0].sdb_component.addr_first;
  device.write(ioconf, EB_DATA32, uIOConfig);
  
  /* Show time */
  eca.refresh();
  fprintf(stdout, "%s: time (fmt): %s\n", argv[0], eca.date().c_str());
  fprintf(stdout, "%s: time (hex): 0x%"PRIx64"\n", argv[0], eca.time);
  
  /* Configure ECA to create IO pulses on GPIO and LVDS */
  eca.channels[0].drain(false); // GPIO
  eca.channels[1].drain(true); // PCIe
  eca.channels[2].drain(true); // LVDS
  eca.channels[0].freeze(false);
  eca.channels[1].freeze(false);
  eca.channels[2].freeze(false);
  eca.disable(true);
  eca.interrupt(false);
  
  /* Build table */   
  for (int i = 0; i < EVENTS*EVENT_MULTI; ++i)
  {
    table.add(TableEntry(0xdeadbeef, i*(HIGH_NS+LOW_NS)/8,               (0x1<<uShiftPosition),      0, 64));
    table.add(TableEntry(0xdeadbeef, i*(HIGH_NS+LOW_NS)/8+(HIGH_NS/8),   (0x1<<(uShiftPosition+16)), 0, 64));
    /* Take care on shift position */
    if(uShiftPosition==EVENTS-1) { uShiftPosition=0; }
    else                         { uShiftPosition++; }
  }

  /* Manage ECA */
  eca.store(table);
  eca.flipTables();
  eca.disable(false);
  eca.refresh();
  uint64_t start = eca.time + (CPU_DELAY_MS*1000*1000)/8;
  eca.streams[0].send(EventEntry(0xdeadbeef, 0, 0, start));
   
  /* Sleep ... until all events have finished */
  usleep(CPU_DELAY_MS*2000 + (long)(HIGH_NS+LOW_NS)*(EVENTS*EVENT_MULTI)/1000); 

  /* Read-out result */
  tlu.pop_all(queues);
  uQueuesTotal = queues.size();
  fprintf(stdout, "%s: found queues %d ...\n", argv[0], uQueuesTotal);
  
  /* Check each queue now */
  for(uQueueInterator=0; uQueueInterator<uQueuesTotal; uQueueInterator++)
  {
    std::vector<uint64_t>& queue = queues[uQueueInterator];
    uQueneItems = queue.size(); /* Get the actual size */
    fprintf(stdout, "%s: queue %d has a size of %d ...\n", argv[0], uQueueInterator, uQueneItems);
    /* Inspect items with queue contains data */
    if(uQueneItems)
    {
      for(uQueueItemIterator=0; uQueueItemIterator<uQueneItems; uQueueItemIterator++)
      {
        a_uEdges[uQueueInterator]++; /* Count up seen edges */
        fprintf(stdout, "%s: queue[%d][%d]: 0x%"PRIx64" (0x%"PRIx64")... ", argv[0], uQueueInterator, uQueueItemIterator, queue[uQueueItemIterator]/8, queue[uQueueItemIterator]);
        if(uQueueItemIterator>=1)
        {
          uTimeDiff = queue[uQueueItemIterator]-queue[uQueueItemIterator-1]; 
          fprintf(stdout, "(difference to previous time stamp 0x%"PRIx64" (%"PRIu64"))\n", uTimeDiff, uTimeDiff);
          a_uFrequency[uQueueInterator] += uTimeDiff; /* Sum up time stamps */
          /* Allow little jitter/uncertainty */
          if( ((*p_uFrequencyExpected+TOLERANCE_FACTOR_NS)<uTimeDiff) ||
              ((*p_uFrequencyExpected-TOLERANCE_FACTOR_NS)>uTimeDiff)
          )
          {
            fprintf(stdout, "%s: too much jitter for IO %d!\n", argv[0], uQueueInterator);
            return 1;
          }
        }
        else
        {
          fprintf(stdout, "\n");
        }
      }
    }
    p_uFrequencyExpected++;
  }
  
  /* Reset Frequency expected pointer */
  if(uTestCase==1)      { p_uFrequencyExpected = a_uFrequencyExpectedTestCase1; }
  else if(uTestCase==2) { p_uFrequencyExpected = a_uFrequencyExpectedTestCase2; }
  else                  { p_uFrequencyExpected = a_uFrequencyExpectedTestCase3; }
  
  /* Evaluate test case */
  for(uArrayIterator=0; uArrayIterator<EVENTS; uArrayIterator++)
  {
    /* Compare expected edges */
    if(a_uEdges[uArrayIterator] != *p_uEdgesExpected)
    {
      fprintf(stdout, "%s: wrong number of seen edges for IO %d!\n", argv[0], uArrayIterator);
      fprintf(stdout, "%s: expected %d edges!\n", argv[0], *p_uEdgesExpected);
      fprintf(stdout, "%s: measured %d edges!\n", argv[0], a_uEdges[uArrayIterator]);
      return 1;
    }
    
    /* Compare expected frequency (if we expected edges on this IO) */
    if(*p_uEdgesExpected)
    {
      dExpectedFrequency = *p_uFrequencyExpected;
      dMeasuredFrequency = a_uFrequency[uArrayIterator] / (a_uEdges[uArrayIterator]-1);
      
      if(dMeasuredFrequency!=dExpectedFrequency)
      {
        /* Allow little jitter/uncertainty */
        if( ((dExpectedFrequency+TOLERANCE_FACTOR_NS)<dMeasuredFrequency) ||
            ((dExpectedFrequency-TOLERANCE_FACTOR_NS)>dMeasuredFrequency)
          )
        {
          fprintf(stdout, "%s: average frequency does not match for IO %d!\n", argv[0], uArrayIterator);
          fprintf(stdout, "%s: expected average frequency on %f: \n", argv[0], dExpectedFrequency);
          fprintf(stdout, "%s: measured average frequency on %f: \n", argv[0], dMeasuredFrequency);
          return 1;
        }
      }
    }
    
    /* Increase compare pointers */
    p_uFrequencyExpected++;
    p_uEdgesExpected++;
    
  }

  /* Done */
  return 0;
  
}
