//////////////////////////////////////////////////////////////////////////////////////////////
// wb_api.h
//
//
//  created : Apr 10, 2013
//  author  : Dietrich Beck, GSI-Darmstadt
//            some code for 1-wire stuff borrowed from
//            -- Stefan Rauch <s.rauch@gsi.de>
//            -- Wesley W. Terpstra <w.terpstra@gsi.de>
//            -- Alessandro Rubini <rubini@gnudd.com>
//            -- Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
//  version : 25-Sep-2019
//
#define WB_API_VERSION "0.13.0"
//
// Api for wishbone devices for timing receiver nodes. This is not a timing receiver API.
// 
//
// requires:
//  - etherbone
//  - wb_slaves.h
//  - 1-wire 
//
// example of usage: see monitoring/eb-mon.c
//
// compile flags:
//  - WB_SIMULATE, no access to real devices, just for testing
// definitions of application specific addresses of wishbone devices must be
// included in the main program.
//
//
// -------------------------------------------------------------------------------------------
// License Agreement for this software:
//
// Copyright (C) 2013  Dietrich Beck
// GSI Helmholtzzentrum für Schwerionenforschung GmbH
// Planckstraße 1
// D-64291 Darmstadt
// Germany
//
// Contact: d.beck@gsi.de
//
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program.
// If not, see <http: /www.gnu.org/licenses/>.
//
// For all questions and ideas contact: d.beck@gsi.de
// Last update: 29-Jan-2016
///////////////////////////////////////////////////////////////////////////////////////////////
#ifndef WB_API_H_
#define WB_API_H_

#include <inttypes.h>
#include <etherbone.h>

// opens connection to Wishbone bus via Etherbone
eb_status_t wb_open(const char *dev,                           // EB device name
                    eb_device_t *device,                       // EB device
                    eb_socket_t *socket                        // EB socket/
                    );

// closes connection to Wishbone bus
eb_status_t wb_close(eb_device_t device,                       // EB device
                     eb_socket_t socket                        // EB socket/
                     );

// gets start address of a WB device
eb_status_t wb_get_device_address(eb_device_t device,          // EB device
                                  uint64_t vendor_id,          // vendor ID of WB device
                                  uint32_t product_id,         // product ID of WB device
                                  uint8_t  ver_major,          // major version
                                  uint8_t  ver_minor,          // minor version
                                  int      devIndex,           // 0,1,2... - there may be more than 1 device on the WB bus
                                  eb_address_t *address,       // start address of WB device
                                  int *nDevices                // number of WB devices of this type
                                  );

// gets the actual UTC or TAI time (depends on configuration of clock master)
eb_status_t wb_wr_get_time(eb_device_t  device,                // EB device
                           int devIndex,                       // 0,1,2... - there may be more than 1 device on the WB bus
                           uint64_t *nsecs                     // timestamp [ns]
                           );

// gets MAC of White Rabbit port
eb_status_t wb_wr_get_mac(eb_device_t device,                  // EB device
                          int devIndex,                        // 0,1,2... - there may be more than 1 device on the WB bus
                          uint64_t *mac                        // MAC address
                          );

// gets link state of White Rabbit port
eb_status_t wb_wr_get_link(eb_device_t device,                 // EB device
                           int devIndex,                       // 0,1,2... - there may be more than 1 device on the WB bus
                           int *link                           // link state: 0: link down, 1: link up
                           );

// gets ip address of White Rabbit port
eb_status_t wb_wr_get_ip(eb_device_t device,                   // EB device
                         int devIndex,                         // 0,1,2... - there may be more than 1 device on the WB bus
                         int *ip                               // ip address
                         );

// gets sync state of WR port
eb_status_t wb_wr_get_sync_state(eb_device_t device,           // EB device
                                 int devIndex,                 // 0,1,2... - there may be more than 1 device on the WB bus
                                 int *syncState                // sync state: 0: NO_SYNC, 2: PPS, 4: TIME, 6:LOCK, 14: TRACK
                                 );

// gets uptime of WR lm32
eb_status_t wb_wr_get_uptime(eb_device_t device,               // EB device
                             int devIndex,                     // 0,1,2... - there may be more than 1 device on the WB bus
                             uint32_t *uptime                  // uptime of WR lm32 [s]
                             );

// gets lock statistics about WR 
eb_status_t wb_wr_stats_get_lock(eb_device_t device,           // EB device
                                 int devIndex,                 // 0,1,2... - there may be more than 1 device on the WB bus
                                 uint64_t *lockLossTS,         // WR lock: timestamp of last lost lock
                                 uint64_t *lockAcqTS,          // WR lock: timestamp of last lock acquired
                                 uint32_t *lockNAcq            // WR lock: number of successful acquired locks
                                 );

// gets time continuity statistics about WR 
eb_status_t wb_wr_stats_get_continuity(eb_device_t device,     // EB device
                                       int devIndex,           // 0,1,2... - there may be more than 1 device on the WB bus
                                       uint64_t *contObsT,     // WR time continuity: observation intervall
                                       int64_t  *contMaxPosDT, // WR time continuity: max positive difference between ECA and WR time
                                       uint64_t *contMaxPosTS, // WR time continuity: timestamp of last positive diff update
                                       int64_t  *contMaxNegDT, // WR time continuity: max negative difference between ECA and WR time
                                       uint64_t *contMaxNegTS  // WR time cotinuity: timestamp of last negative diff update
                                       );

// gets lm32 stall statistics
eb_status_t wb_wr_stats_get_stall(eb_device_t device,          // EB device
                                  int devIndex,                // 0,1,2... - there may be more than 1 device on the WB bus
                                  uint32_t stallObsCPU,        // lm32 stall: select # of CPU for data below
                                  uint64_t *stallObsT,         // lm32 stall: observation intervall
                                  uint32_t *stallMaxStreak,    // lm32 stall: obvserved max continous stall in Wishbone cycles
                                  uint32_t *stallN,            // lm32 stall: stall time within observation interval
                                  uint64_t *stallTS            // lm32 stall: timestamp of last update
                                  );

// resets statistics about WR (and clears counters)
eb_status_t wb_wr_stats_reset(eb_device_t device,              // EB device
                              int devIndex,                    // 0,1,2... - there may be more than 1 device on the WB bus
                              uint64_t contObsT,               // WR time continuity: observation interval (consider using '8')
                              uint32_t stallObsT               // lm32 stall: observation interval (consider using '0')
                              );

// gets statistics about ECA
eb_status_t wb_eca_stats_get(eb_device_t device,               // EB device
                             int devIndex,                     // 0,1,2... - there may be more than 1 device on the WB bus
                             uint64_t *nMessage,               // # of messages received
                             int64_t *dtSum,                   // accumulated differences (deadline - timestamp)
                             int64_t *dtMin,                   // minimum difference (deadline - timestamp)
                             int64_t *dtMax                    // maximum difference (deadline - timestamp)
                             );

// resets statistics about ECA (and clears counters)
eb_status_t wb_eca_stats_reset(eb_device_t device,             // EB device
                               int devIndex                    // 0,1,2... - there may be more than 1 device on the WB bus
                               );

// enables/disables capture of statistics about ECA
eb_status_t wb_eca_stats_enable(eb_device_t device,            // EB device
                                int devIndex,                  // 0,1,2... - there may be more than 1 device on the WB bus
                                uint32_t enableFlag            // 1: enables capture; 0: disables capture            
                               );
                             
// get ID of the 1st 1-wire sensor found on the specified bus
eb_status_t wb_1wire_get_id(eb_device_t device,                // EB device
                            int devIndex,                      // 0,1,2... - there may be more than 1 device on the WB bus
                            unsigned int busIndex,             // index of the physical 1-wire bus
                            unsigned int family,               // family code of 1-wire sensor
                            short isUserFlag,                  // 1: 'user' 1-wire; 0: 'WR-periph' 1-wire
                            uint64_t *id                       // ID of 1-wire sensor
                            );

// get temp the 1st 1-wire temperatur sensor found on the specified bus (user)
eb_status_t wb_1wire_get_temp(eb_device_t device,              // EB device
                              int devIndex,                    // 0,1,2... - there may be more than 1 device on the WB bus
                              unsigned int busIndex,           // index of the physical 1-wire bus
                              unsigned int family,             // family code of 1-wire sensor
                              short isUserFlag,                // 1: 'user' 1-wire; 0: 'WR-periph' 1-wire
                              double *temp                     // temperature
                              );

// reset the FPGA, reload new image from flash
eb_status_t wb_wr_reset(eb_device_t device,                    // EB device
                        int devIndex,                          // 0,1,2... - there may be more than 1 device on the WB bus
                        uint32_t value                         // value to be written to the reset controller
                        );

// disable the watchdog and prevent automated restart
eb_status_t wb_wr_watchdog(eb_device_t device,                 // EB device
                           int devIndex                        // 0,1,2... - there may be more than 1 device on the WB bus
                           );

// put user lm32 into reset state
eb_status_t wb_cpu_halt(eb_device_t device,                    // EB device
                        int devIndex,                          // 0,1,2... - there may be more than 1 device on the WB bus
                        uint32_t value                         // number 0..31 of a single cpu, 0xff for all 
                        );

// release user lm32 from reset state
eb_status_t wb_cpu_resume(eb_device_t device,                  // EB device
                          int devIndex,                        // 0,1,2... - there may be more than 1 device on the WB bus
                          uint32_t value                       // number 0..31 of a single cpu, 0xff for all 
                          );

// get reset state of all lm32 
eb_status_t wb_cpu_status(eb_device_t device,                  // EB device
                          int devIndex,                        // 0,1,2... - there may be more than 1 device on the WB bus
                          uint32_t *value                      // one bit per CPU; CPU 0 is rightmost bit
                          );

// get gateware build type 
eb_status_t wb_get_build_type(eb_device_t device,              // EB device
                              int size,                        // array size of builtType
                              char *buildType                  // build Type 
                              );

#endif // wb_api.h
