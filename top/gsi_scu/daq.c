////////////////////////////////////////////////////////////////////////////////
//
// filename: daq.c
// desc: functions for servicing the daq macro
// creation date: 29.11.2017
// last modified: 29.11.2017
// author: Stefan Rauch
//
// Copyright (C) 2017 GSI Helmholtz Centre for Heavy Ion Research GmbH 
//
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//  
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.
///////////////////////////////////////////////////////////////////////////////// 
#include "daq.h"
#include "scu_bus.h"

//int configure_daq_channel(volatile unsigned short *scub_base, int slot, int channel, unsigned int data) {
  //if ((slot < 1) || (slot > 12))
    //return DAQ_ERROR;
//
  //if ((channel < 0) || (channel >= DAQ_MAX_CHN))
    //return DAQ_ERROR;
//
  //return DAQ_OKAY;
//}

int add_to_daqlist(int slot, int chncount, int cid_sys, int cid_group, uint32_t *daqlist) {
  int found = 0;
  int count = 0;

  /* find first free slot */
  while ((daqlist[count] != 0) && (count < MAX_SCU_SLAVES)) {
    count++;
  }
  
  if (cid_sys == SYS_CSCO || cid_sys == SYS_PBRF || cid_sys == SYS_LOEP) {
    if (cid_group == GRP_ADDAC1 ||
        cid_group == GRP_ADDAC2 ||
        cid_group == GRP_DIOB) {
      /* list daq macro */
      if (count < MAX_SCU_SLAVES) {
        daqlist[count] = 0;               // not used
        daqlist[count] |= 0;              // version
        daqlist[count] |= chncount << 16; // number of channels
        daqlist[count] |= slot << 24;     // slot
        count++;                          // next macro
        found++;
      }
    } 
    
  }

  return count; //return number of found daq macros 
}
