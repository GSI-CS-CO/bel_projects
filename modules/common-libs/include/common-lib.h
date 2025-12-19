/******************************************************************************
 *  common-lib.h
 *
 *  created : 2019
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 19-Dec-2025
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

#define COMMON_LIB_VERSION "0.05.00"

#include <etherbone.h>

// small helper function; actual time [ns]
uint64_t comlib_getSysTime();

// small helper function; very expensive sleep function!!
void comlib_nsleep(uint64_t t                         // time to sleep [ns]
                   );

// get character from stdin, 0: no character
char comlib_term_getChar();

// clear teminal windows and jump to 1,1
void comlib_term_clear();

// move cursor position in terminal
void comlib_term_curpos(int column, int line);

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
                    uint32_t    *nTransfer,            // # of transfers                                ; PSM: # phase shifts SIS18
                    uint32_t    *nInjection,           // # of injection within ongoing transfers       ; PSM: # phase shifts ESR, CRYRING
                    uint32_t    *statTrans,            // status bits of transfer (application specific); PSM: # phase shifts SIS100
                    uint32_t    *nLate,                // number of ECA 'late' incidents                                            
                    uint32_t    *nEarly,               // number of ECA 'early' incidents                                           
                    uint32_t    *nConflict,            // number of ECA 'conflict' incidents                                        
                    uint32_t    *nDelayed,             // number of ECA 'delayed' incidents                                         
                    uint32_t    *nSlow,                // number of incidents, when 'wait4eca' was called after the deadline        
                    uint32_t    *offsSlow,             // if 'slow': offset deadline to start wait4eca; else '0'                  
                    uint32_t    *comLatency,           // if 'slow': offset start to stop wait4eca; else deadline to stop wait4eca
                    uint32_t    *offsDone,             // offset event deadline to time when we are done [ns]
                    uint32_t    *usedSize,             // used size of shared memory
                    int         printFlag              // '1' print information to stdout
                    );

// prints diagnostic data
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
                      uint32_t  nLate,                 // number of ECA 'late' incidents                                            
                      uint32_t  nEarly,                // number of ECA 'early' incidents                                           
                      uint32_t  nConflict,             // number of ECA 'conflict' incidents                                        
                      uint32_t  nDelayed,              // number of ECA 'delayed' incidents                                         
                      uint32_t  nSlow,                 // number of incidents, when 'wait4eca' was called after the deadline        
                      uint32_t  offsSlow,              // if 'slow': offset deadline to start wait4eca; else '0'                  
                      uint32_t  comLatency,            // if 'slow': offset start to stop wait4eca; else deadline to stop wait4eca
                      uint32_t  offsDone,              // offset event deadline to time when we are done [ns]
                      uint32_t  usedSize               // used size of shared memory
                      );

// open Etherbone connection to ECA queue
uint32_t comlib_ecaq_open(const char* devName,         // EB device name such as dev/wbm0
                          uint32_t qIdx,               // index of action queue we'd like to connect to
                          eb_device_t *device,         // EB device
                          eb_address_t *ecaq_base      // EB address
                          );

// closes Etherbone connection to ECA queue
uint32_t comlib_ecaq_close(eb_device_t device          // EB device
                           );

// directly reads messages from an ECA queue via Etherbone(not via saftlib)
uint32_t comlib_wait4ECAEvent(uint32_t     timeout_ms, // timeout [ms]
                              eb_device_t  device,     // EB device 
                              eb_address_t ecaq_base,  // EB address
                              uint32_t     *tag,       // tag
                              uint64_t     *deadline,  // messages deadline
                              uint64_t     *evtId,     // EvtId
                              uint64_t     *param,     // parameter field
                              uint32_t     *tef,       // TEF field
                              uint32_t     *isLate,    // flags ...
                              uint32_t     *isEarly,
                              uint32_t     *isConflict,
                              uint32_t     *isDelayed
                              );

// converts half precision float to single precision float
float comlib_half2float(uint16_t h                     // half precision float
                        );
// converts single precision float to half precision float
uint16_t comlib_float2half(float f                     // single precision float
                           );

#endif
