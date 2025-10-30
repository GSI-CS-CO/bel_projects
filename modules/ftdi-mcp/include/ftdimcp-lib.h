/******************************************************************************
 *  ftdimcp-lib.h
 *
 *  created : 2023
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 20-Oct-2025
 * 
 * x86 routines for a MCP4725 connected via FT232H
 * as an example, this can be used to set the level of a comparator circuit
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
#ifndef _FTDIMCP_LIB_H_
#define _FTDIMCP_LIB_H_

#define FTDIMCP_LIB_VERSION 0x000008

// ftdi, i2c
#include <ftd2xx.h>
#include <libmpsse_i2c.h>

// input: pin 0..3; output pin 4..7
#define FTDIMCP_PINLED               5                   // c<N> pin to which the activiy LED is connected
#define FTDIMCP_PINSTRETCH           1                   // c<N> pin to which the stretched comparator output is connected
#define FTDIMCP_PINACT               2                   // c<N> pin to which the current comparator output is connected
#define FTDIMCP_GPIODIR           0xf0                   // bitwise direction, c0..c3: input, c4..c7: output
#define FTDIMCP_I2CADDR           0x64                   // i2c address of MCP4725
#define FTDIMCP_POLLINTERVAL_US 100000                   // interval used for polling [us]


// get library versions
void ftdimcp_getVersion(uint32_t *ftdimcp_version,       // version of this library
                        uint32_t *ftdi_version,          // version of ftdi library
                        uint32_t *mpsse_version          // version of mpsse library
                        );


// opens connection to the channel (device)
FT_STATUS ftdimcp_open(int cIdx,                         // channel index (usually '0')
                       FT_HANDLE *cHandle,               // handle to channel
                       int flagDebug                     // 1: debug on (print debug info)
                       );


// closes connection to the channel
void ftdimcp_close(FT_HANDLE cHandle                     // handle to channel
                   );


// gets and prints info of the channel
FT_STATUS ftdimcp_info(int cIdx,                         // channel index
                       uint32_t *deviceId,               // USB device ID
                       char *deviceSerial,               // serial number of device
                       int flagPrint                     // flag; 0: don't print info to screen, 1: print info to screen
                       );

// init channel
FT_STATUS ftdimcp_init(FT_HANDLE cHandle                 // handle to channel
                       );


// set level of comparator
FT_STATUS ftdimcp_setLevel(FT_HANDLE cHandle,            // handle to channel
                           uint32_t  dacAddr,            // I2C address of DAC
                           double    dacLevel,           // level [%]
                           int       flagEeprom,         // 1: write to EEPROM too
                           int       flagDebug           // 1: print debug info
                           );


// get level of comparator
FT_STATUS ftdimcp_getLevel(FT_HANDLE cHandle,            // handle to channel
                           uint32_t  dacAddr             // I2C address of DAC
                           );


// sets value of activity LED
FT_STATUS ftdimpc_setLed(FT_HANDLE cHandle,              // handle to channel
                         uint32_t  on                    // value to set
                         );


// get actual value at comparator output
FT_STATUS ftdimpc_getCompOutAct(FT_HANDLE cHandle,       // handle to channel
                                uint32_t *on             // value received
                                );


// get stretched value of comparator output
FT_STATUS ftdimpc_getCompOutStretched(FT_HANDLE cHandle, // handel to channel
                                      uint32_t *on       // value received
                                      );

#endif
