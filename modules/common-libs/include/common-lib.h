/******************************************************************************
 *  common-lib.h
 *
 *  created : 2019
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 02-April-2020
 *
 * common x86 routines for firmware
 *
 * -------------------------------------------------------------------------------------------
 * License Agreement for this software:
 *
 * Copyright (C) 2013  Dietrich Beck
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
 * Last update: 15-April-2019
 ********************************************************************************************/
#ifndef _COMMON_LIB_H_
#define _COMMON_LIB_H_

#define COMMON_LIB_VERSION "0.01.01"

#include <etherbone.h>

// small helper function
uint64_t comlib_getSysTime();

// convert state code to state text
const char* comlib_stateText(uint32_t  bit             // state code
                             );
// convert status code to status text
const char* comlib_statusText(uint32_t  bit            // status code
                              );
// init for communicaiton with shared mem
void comlib_initShared(eb_address_t lm32_base,         // base address of lm32
                       eb_address_t sharedOffset       // offset of shared area
                       );

// read (and print) diagnostic data, returns eb_status
int comlib_readDiag(eb_device_t device,                // Etherbone device
                    uint64_t    *statusArray,          // array with status bits
                    uint32_t    *state,                // state
                    uint32_t    *version,              // firmware version
                    uint64_t    *mac,                  // WR MAC
                    uint32_t    *ip,                   // WR IP
                    uint32_t    *nBadStatus,           // # of bad status incidents
                    uint32_t    *nBadState,            // # of bad state incidents
                    uint64_t    *tDiag,                // time, when diag data was reset
                    uint64_t    *tS0,                  // time, when entering S0 state (firmware boot)
                    uint32_t    *nTransfer,            // # of transfers
                    uint32_t    *nInjection,           // # of injection within ongoing transfers
                    uint32_t    *statTrans,            // status bits of transfer (application specific)
                    uint32_t    *usedSize,             // used size of shared memory
                    uint32_t    printFlag              // '1' print information to stdout
                    );

void comlib_printDiag(uint64_t  statusArray,           // array with status bits
                      uint32_t  state,                 // state
                      uint32_t  version,               // firmware version
                      uint64_t  mac,                   // WR MAC
                      uint32_t  ip,                    // WR IP
                      uint32_t  nBadStatus,            // # of bad status incidents
                      uint32_t  nBadState,             // # of bad state incidents
                      uint64_t  tDiag,                 // time, when diag data was reset
                      uint64_t  tS0,                   // time, when entering S0 state (firmware boot)
                      uint32_t  nTransfer,             // # of transfers
                      uint32_t  nInjection,            // # of injection within ongoing transfers
                      uint32_t  statTrans,             // status bits of transfer (application specific)
                      uint32_t  usedSize               // used size of shared memory
                      );

#endif
