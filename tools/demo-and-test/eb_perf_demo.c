/******************************************************************************
 * eb_perf_demo.c
 *
 *  created : 2018
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 30-Jan-2018
 *
 *  Command line tool demonstrating some Etherbone performance measurements 
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
 * Last update: 25-April-2015
 ********************************************************************************************/
#define EBPERFDEMO_VERSION "0.1.0"

//standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

// Etherbone
#include <etherbone.h>

// WB devices
#include <wb_slaves.h>

#define MAXTS                        1000000                 // max number of timestamps

// globals
static const char* program;   // name of this program
static int         n_eb_cb;   // counts number of received callbacks

void die(const char* where, eb_status_t status) {
  fprintf(stderr, "%s: %s failed: %s\n",
    program, where, eb_status(status));
  exit(1);
} // die

static void help(void) {
  printf("Usage: %s <etherbone-device> <lSustain> <nIterations> <mReadsPerIteration> <mode>\n", program);
  printf("\n");
  printf("l(Sustain)   : usually use a value of 1. Use a higher value to get numbers for sustained operation.\n");
  printf("n(Iterations): number of iterations of the Etherbone cycle\n");
  printf("m(Reads)     : number of 'reads' per Etherbone cycle\n");
  printf("\n");
  printf("mode 0: use 'm * calls' of the convenience functions\n");
  printf("mode 1: use 'm * reads' within each Etherbone cycle, Etherbone cycles are blocking \n");
  printf("mode 2: use 'm * reads' within each Etherbone cycle, Etherbone cycles are pipelined\n");
  printf("\n");
  printf("Use this tool to do some performance measurements with Etherbone by reading 64bit timestamps from the ECA.\n");
  printf("It reads l * n * m timestamps.\n"); 
  printf("Example: '%s dev/wbm0 1 90 8 1'\n", program);
  printf("         l=1(not sustained), do >90< iterations of a >blocking< EB cycle including >8< reads of timestamps\n");
  printf("\n");
  printf("Report software bugs to <d.beck@gsi.de>\n");
  printf("Version %s. Licensed under the LGPL v3.\n", EBPERFDEMO_VERSION);
}


// prints results to stdout
void printResult(uint64_t ts_start, uint64_t ts_stop, int lSustain, int nIter, int mOp)
{
  uint64_t      elapsed_time = 0;
  double        timePerEBCycle;
  double        timePerTS;


  elapsed_time   = ts_stop - ts_start;
  timePerEBCycle = ((double)elapsed_time / (double)(nIter * lSustain)) / 1000.0;
  timePerTS      = timePerEBCycle / (double)mOp;
  printf("elapsed time     : %f [ms] \n", (double)elapsed_time / 1000000.0);
  printf("# of 32bit words : %d\n", mOp * nIter * lSustain * 2);
  printf("time per EB cycle: %f [us] \n", timePerEBCycle);
  printf("time per 64bit TS: %f [us] \n", timePerTS);
  printf("EB cycle rate    : %.1f [kHz] \n", 1000.0 / (double)timePerEBCycle);
  printf("TS rate          : %.1f [kHz] \n", 1000.0 / (double)timePerTS);
  printf("throughput       : %.1f [MBit/s] \n", 1000.0 / (double)elapsed_time * lSustain * nIter * mOp * 8 * 8);
} // printResult



// callback function for non-blocking Etherbone cycles. This one is really primitive.
static void eb_cb(eb_user_data_t user, eb_device_t dev, eb_operation_t op, eb_status_t status) {
  n_eb_cb++;
} //eb_cb


int main(int argc, const char** argv) {
  eb_status_t status;
  eb_device_t device;
  eb_socket_t socket;
  eb_cycle_t  cycle;
  struct sdb_device sdbDevice;
  eb_address_t wrECA;
  int nDevices;

  eb_data_t     data1, data2;
  eb_data_t     *TSLo, *TSHi;
  
  const char*   devName;
  int           mode;

  int           lSustain;               // used to get data for sustained operaiton
  int           nIterations;            // used to get a series of EB cycles
  int           mReads;                 // used for set the number of reads per EB cycle
  
  uint64_t      timestamp_start = 0;
  uint64_t      timestamp_stop  = 0;

  uint64_t      ts4Calc_start = 0;
  uint64_t      ts4Calc_stop  = 0;

  int           l = 1;
  int           m = 1;       
  int           n = 1;

  
  program = argv[0];
  if (argc < 6) {help(); return 1;}
  
  devName     	 = argv[1];
  mode           = strtol(argv[5],0,10);
  mReads         = strtol(argv[4],0,10);
  nIterations 	 = strtol(argv[3],0,10);
  lSustain 	 = strtol(argv[2],0,10);

  if ((nIterations * mReads) > MAXTS) die("too much data requested", 0);

  TSHi = (eb_data_t *)malloc (nIterations * mReads * sizeof(eb_data_t));
  TSLo = (eb_data_t *)malloc (nIterations * mReads * sizeof(eb_data_t));

  printf("measure time using %d iterations with %d reads\n", nIterations, mReads);


  // open socket
  if ((status = eb_socket_open(EB_ABI_CODE, 0, EB_ADDR32|EB_DATA32, &socket)) != EB_OK) die("eb_socket_open", status);
  
  // open device
  if ((status = eb_device_open(socket, devName, EB_ADDR32|EB_DATA32, 3, &device)) != EB_OK) die("eb_device_open", status);
  
  // find ECA. Assumption: only one ECA
  nDevices = 1;
  if ((status = eb_sdb_find_by_identity(device, ECA_CTRL_VENDOR, ECA_CTRL_PRODUCT, &sdbDevice, &nDevices)) != EB_OK) die("ECA eb_sdb_find_by_identity", status);
  
  if (nDevices == 0)
    die("no ECA found", EB_FAIL);
  if (nDevices > 1)
    die("too many ECAs found", EB_FAIL);
  
  // found ECA address
  wrECA = sdbDevice.sdb_component.addr_first;
  
  // get actual timestamp from the ECA
  if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die("EP eb_cycle_open", status);
  eb_cycle_read(cycle, wrECA + ECA_CTRL_TIME_HI_GET, EB_BIG_ENDIAN|EB_DATA32, &data1);
  eb_cycle_read(cycle, wrECA + ECA_CTRL_TIME_LO_GET, EB_BIG_ENDIAN|EB_DATA32, &data2);
  if ((status = eb_cycle_close(cycle)) != EB_OK) die("EP eb_cycle_close", status);        
  timestamp_start = data1;
  timestamp_start = (timestamp_start << 32);
  timestamp_start = timestamp_start + data2;
  printf("Current TAI (start): %"PRIu64" ns\n",timestamp_start);

  n_eb_cb = 0;
  for (l=0; l<lSustain; l++) {
  // using convenience functions
    if (mode == 0) {
      for (n=0; n<nIterations; n++) {
        for (m=0; m<mReads; m++) {
          eb_device_read(device, wrECA + ECA_CTRL_TIME_HI_GET, EB_BIG_ENDIAN|EB_DATA32, &(TSHi[n*mReads+m]), 0, eb_block);
          eb_device_read(device, wrECA + ECA_CTRL_TIME_LO_GET, EB_BIG_ENDIAN|EB_DATA32, &(TSLo[n*mReads+m]), 0, eb_block);
        } // for mReads
      } // for nIterations
    } // mode = 0; convenience functions
    
    // using blocking cycles
    if (mode == 1) {
      for (n=0; n<nIterations; n++) {
        if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die("EP eb_cycle_open", status);
        
        for (m=0; m<mReads; m++) {
          eb_cycle_read(cycle, wrECA + ECA_CTRL_TIME_HI_GET, EB_BIG_ENDIAN|EB_DATA32, &(TSHi[n*mReads+m]));
          eb_cycle_read(cycle, wrECA + ECA_CTRL_TIME_LO_GET, EB_BIG_ENDIAN|EB_DATA32, &(TSLo[n*mReads+m]));
        } // for mReads
        
        if ((status = eb_cycle_close(cycle)) != EB_OK) die("EP eb_cycle_close", status);        
      } // for nIterations
    } // mode = 1; EB blocking cycles ("single cycles")

  // using non-blocking cycles
    if (mode == 2) {
      for (n=0; n<nIterations; n++) {
        if ((status = eb_cycle_open(device, 0, eb_cb, &cycle)) != EB_OK) die("EP eb_cycle_open", status);
        
        for (m=0; m<mReads; m++) {
          eb_cycle_read(cycle, wrECA + ECA_CTRL_TIME_HI_GET, EB_BIG_ENDIAN|EB_DATA32, &(TSHi[n*mReads+m]));
          eb_cycle_read(cycle, wrECA + ECA_CTRL_TIME_LO_GET, EB_BIG_ENDIAN|EB_DATA32, &(TSLo[n*mReads+m]));
        } // for mReads

        if ((status = eb_cycle_close(cycle)) != EB_OK) die("EP eb_cycle_close", status);        
        //eb_socket_run(socket, 2000);
      } // for nIterations
      eb_socket_run(socket, 2000); //hm... this is better
    } // mode = 2; non-blocking via callback ("pipelined")
  } // for l...
    
  // read actual timestamp from the ECA
  if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die("EP eb_cycle_open", status);
  eb_cycle_read(cycle, wrECA + ECA_CTRL_TIME_HI_GET, EB_BIG_ENDIAN|EB_DATA32, &data1);
  eb_cycle_read(cycle, wrECA + ECA_CTRL_TIME_LO_GET, EB_BIG_ENDIAN|EB_DATA32, &data2);
  if ((status = eb_cycle_close(cycle)) != EB_OK) die("EP eb_cycle_close", status);        
  timestamp_stop = data1;
  timestamp_stop = (timestamp_stop << 32);
  timestamp_stop = timestamp_stop + data2;
  printf("Current TAI ( stop): %"PRIu64" ns\n",timestamp_stop);

  printf("\n");  

  printf(">>>> timestamps of first cycle: \n");
  for (m=0; m<mReads; m++) printf("TAI %"PRIu64"\n", (((uint64_t)TSHi[m]) << 32) +( uint64_t)TSLo[m]);

  printf(">>>> timestamps of last cycle: \n");
  for (m=0; m<mReads; m++) printf("TAI %"PRIu64"\n", (((uint64_t)TSHi[(nIterations-1)*mReads + m]) << 32) + (uint64_t)TSLo[(nIterations-1)*mReads + m]);

  if (mode == 2)  printf("\ngot %d callbacks\n", n_eb_cb);

  printf("\n");
  
  //calculate performance data from 1st separate and last acquired timestamps
  ts4Calc_stop  = ((uint64_t)TSHi[(nIterations-1)*mReads + mReads - 1] << 32) + (uint64_t)TSLo[(nIterations-1)*mReads + mReads - 1];
  ts4Calc_start = timestamp_start;
  printf("--- realistic result: based on one leading (dedicated EB cycle) timestamp and the last acquired timestamp ---\n");
  printResult(ts4Calc_start, ts4Calc_stop, lSustain, nIterations, mReads);
  printf("\n");

  //calculate performance data from aquired timestamps
  if (lSustain == 1) {
    ts4Calc_stop  = ((uint64_t)TSHi[(nIterations-1)*mReads + mReads - 1] << 32) + (uint64_t)TSLo[(nIterations-1)*mReads + mReads - 1];
    ts4Calc_start = ((uint64_t)TSHi[0] << 32) + (uint64_t)TSLo[0];
    printf("--- underestimated result: numbers based on first and last aquired timestamp ---\n");
    printResult(ts4Calc_start, ts4Calc_stop, lSustain, nIterations, mReads);
    printf("\n");
  } //if non sustained

  
  // close handler cleanly
  if ((status = eb_device_close(device)) != EB_OK)
    die("eb_device_close", status);
  if ((status = eb_socket_close(socket)) != EB_OK)
    die("eb_socket_close", status);

  free(TSHi);
  free(TSLo);
  
  return 0;
}



