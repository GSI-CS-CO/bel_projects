/*******************************************************************************************
 *  b2b-sim.c
 *
 *  created : 2023
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 21-Sep-2023
 *
 * simple simulation program for b2b measurements
 * - phase diagnostics
 *
 * ------------------------------------------------------------------------------------------
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
 *********************************************************************************************/
#define B2BSIM_VERSION 0x000808
#define MAXSAMPLES     1000
#define MAXDATA        10000000

// standard includes 
#include <unistd.h> // getopt
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

// b2b includes
#include <b2blib.h>

const char* program;
static int getVersion = 0;

typedef struct{                                      
  uint64_t ns;                             // full nanoseconds of time
  int32_t  ps;                             // ps fraction of time, should be positive
  uint32_t dps;                            // uncertainty [ps]
} b2bt_t;


uint64_t   TH1_as      = 1283767311562;     // h=1 period [as]
uint64_t   one_ns_as   = 1000000000;        // 1 ns [as]
int        mode        = 1;                 // simulate, 1: single phase 2: phase difference
int        fit         = 1;                 // fit method, 1: sub-ns, 2: average
int        nSamples    = 3;                 // number of samples to be used
int        nData       = 1;                 // number of data
uint64_t   noise_as    = 0;                 // amplitude of noise on timestamps[as]
uint64_t   tOffset1_as = 1000000000;        // offset of first timestamp of 1st series
uint64_t   tOffset2_as = 2000000000;        // offset of first timestamp of 2nd series
uint64_t   noiseO_as   = 0;                 // amplitude of noise on tOffset 
char       filename[1024];                  // file name for output
FILE       *dataFile;                       // file for data

uint64_t   tEdge_as[MAXSAMPLES];            // rising edges of h=1 signal
uint64_t   tEdgeNoisy_as[MAXSAMPLES];       // rising edges of h=1 signal with noise
uint64_t   tStamp[MAXSAMPLES];              // tStamps
double     dev[MAXDATA];                    // deviation for stdev


static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION] <etherbone-device>\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -h                      display this help and exit\n");
  fprintf(stderr, "  -e                      display version\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  -T <rf-period>          hf period (h=1) [fs])\n");
  fprintf(stderr, "  -s <nSamples>           number of samples (timestamps)\n");
  fprintf(stderr, "  -r <noise ts>           noise on timestamps [fs] \n");
  fprintf(stderr, "  -d <nData>              number of measurements\n");
  fprintf(stderr, "  -o <phase offset>       offset on rf-phase at 'beginning of flattop' [fs]\n");
  fprintf(stderr, "  -p <noise phase offset> offset on rf-phase [fs]\n");
  fprintf(stderr, "  -m <mode>               1: single timestamp; 2: difference between two timestamps\n");
  fprintf(stderr, "  -t <fit method>         1: sub-ns fit; else: average fit \n");
  fprintf(stderr, "  -c <scan type>          0: don't scan; 1: scan phase offset, 2:???\n");
  fprintf(stderr, "  -i <scan increment>     scan increment [fs]\n");
  fprintf(stderr, "  -f <filename>           write data to file\n");
  fprintf(stderr, "  -n <nPeriods>           number of periods between two measurements\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Example1: '%s -T732996993 -m1 -t1 -c1 -i20000 -d500 -s30'\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");
  fprintf(stderr, "Version %x. Licensed under the LGPL v3.\n", B2BSIM_VERSION);
} //help


// perform fit of phase with sub-ns
int32_t phaseFitAverage(uint64_t TH1_as, uint32_t nSamples, b2bt_t *phase_t, uint32_t *width_as) {
  int64_t  one_ns_as        = 1000000000;   // conversion ns to as
  int64_t  max_diff_as = TH1_as >> 2;       // maximum difference shall be a quarter of the rf-period
  
  int      i;
  uint64_t tFirst_ns;                       // first timestamp [ns]
  //uint64_t nPeriods;                      // number of rf periods
  uint64_t diff_stamp_as;                   // difference between actual and first timestamp [as]
  int64_t  sum_rfperiods_as;                // sum of all rf-periods [as] 
  int64_t  deviation_as;                    // deviation between measured and projected timestamp [as]
  int64_t  abs_deviation_as;                // absolute value of deviation [as]
  int64_t  sum_deviation_as;                // sum of all deviations [as]
  int64_t  ave_deviation_as;                // average of all deviations [as]
  int64_t  max_deviation_as;                // maximum of all deviations [as]
  int64_t  min_deviation_as;                // minimum of all deviations [as]
  int64_t  subnsfit_dev_as;                 // sub-ns-fit deviation [as]
  uint32_t dev_width_as;                    // width of window of all 'deviation_as'
  b2bt_t   ts_t;                            // timestamp [ps]
  int      nGood;                           // number of good timestamps

  // The idea is similar to the native sub-ns fit. As the main difference, the fractional part is 
  // not calculated by the _two_ extremes only, but by using the average of _all_ samples.
  // The algorithm is as follows
  // - always start from the 1st 'good' timestamp (which is the tStamp[1], tStamp[0] might be bad)
  // - for tStamp[i], add (i-1)*rf-period to the first timestamp and calc the difference to tStamp[i]
  // - average all the differences -> one obtains the mean value of all differences
  // - use the mean values as fractional part and add this to the value of tStamp[1]
  // Be aware: timestamps are sorted, but maybe incomplete. The algorithm stops at the first missing timestamp.

  if (TH1_as==0)    return 1;               // rf period must be known
  if (nSamples < 3) return 1;               // need at least three measurements

  // init stuff
  tFirst_ns = tStamp[1];              // don't use 1st timestamp tStamp[0]: start with 2nd timestamp tStamp[1]
  sum_deviation_as = 0;
  sum_rfperiods_as = 0;
  nGood            = 0;
  max_deviation_as = 0;
  min_deviation_as = 0;

  // calc sum deviation of all timestamps
  for (i=1; i<nSamples; i++) {        // include 1st timestamp too (important for little statistics)
  //for (i=1; i<27; i++) {  // HACK      // include 1st timestamp too (important for little statistics)
    
    diff_stamp_as = (tStamp[i] - tFirst_ns) * one_ns_as;

    // we use ther number of iterations as the number of rf-periods; thus, the algorithm will
    // stop at the first missing timestamp
    deviation_as  = diff_stamp_as - sum_rfperiods_as;

    //printf("diff_stamp_ns    %13.3f\n", (double)diff_stamp_as     / 1000000000.0);
    //printf("sum_rfperiods_ns %13.3f\n", (double)sum_rfperiods_as  / 1000000000.0);
    //printf("deviation_as    %13.3f\n",  (double)deviation_as      / 1000000000.0);

    if (deviation_as < 0) abs_deviation_as = -deviation_as;
    else                  abs_deviation_as =  deviation_as;
    // do a simple consistency check - don't accept differences larger than max_diff
    if (abs_deviation_as < max_diff_as) {
      sum_deviation_as += deviation_as;
      nGood++;
      // simple statistics
      if (deviation_as > max_deviation_as) max_deviation_as = deviation_as;
      if (deviation_as < min_deviation_as) min_deviation_as = deviation_as;
    } // if abs_deviation in range

    // increment rf period for next iteration
    sum_rfperiods_as += TH1_as;
  } // for i

  // if result invalid, return with error
  if (nGood < 1) {
    (*phase_t).ns  = 0x7fffffffffffffff;
    (*phase_t).ps  = 0x7fffffff;
    (*phase_t).dps = 0x7fffffff;
    return 1;
  } // if nGood < 1

  // calculate average and jitter (= a quarter of the max-min window)
  ave_deviation_as = sum_deviation_as / nGood;
  subnsfit_dev_as  = (max_deviation_as + min_deviation_as) >> 1; 
  dev_width_as     = (uint32_t)(max_deviation_as - min_deviation_as);
  // calculate a phase value and convert to ps
  ts_t.ns          = tFirst_ns;
  if (fit ==1 ) ts_t.ps = subnsfit_dev_as  / 1000000;  // sub-ns fit
  else          ts_t.ps = ave_deviation_as / 1000000;  // average fit
  ts_t.dps         = dev_width_as  / nGood / 1000000;  // chk 
  *phase_t         = ts_t;
  *width_as        = dev_width_as;

  //printf("nSamples %d, nGood %d, ts_t [ns] %d, ts_t [ps] %d, dt [ps] %d\n", nSamples, nGood, ts_t.ns, ts_t.ps, ts_t.dps);
  return 0;
} //phaseFitAverage


void calcTEdge_as(uint64_t t0_as) {                                // calculates a series of rising h=1 edges
  int    i;

  for (i=0; i<nSamples; i++) tEdge_as[i] = t0_as + i * TH1_as;
} // calcTEdge


// random gaussian noise
// formula: z = sqrt(-2 * ln(x1)) * cos(2*pi*x2)
double randGauss(double sigma, double x0) {
  double x1, x2, z;

  x1 = (double)random() / (double)RAND_MAX;
  x2 = (double)random() / (double)RAND_MAX;
  z  = sqrt(-2 * log(x1)) * cos(2 * 3.14 * x2);
  z  = z * sigma + x0;

  //printf("z: %13.3f\n", z);

  return z;
}  // randGauss



int64_t calcNoise_as(int64_t sigma_noise_as) {
  double   noise;
  int64_t  noise_as;
  double   sigma;

  //noise = (double)noise_fs * ((double)random() / (double)RAND_MAX); // fs, 0..noise_fs
  //noise = noise - (double)noise_fs / 2.0;                           // fs, -noise_fs/2 .. +noise_fs/2
  //noise = noise / 1000000.0;                                        // ns

  sigma    = (double)sigma_noise_as / (double)one_ns_as;
  noise    = randGauss(sigma, 0);
  noise_as = (int64_t)(noise * one_ns_as);
  //printf("sigmas_noise   %13ld\n", sigma_noise_as);
  //printf("gauss noise    %13.0f\n", noise * one_ns_as);
  //printf("gauss_noise_as %13ld\n",  noise_as);

  return noise_as;
} // calcNoise


void calcTEdgeNoisy_as() {                            // adds noise to h=1 eges 
  int      i;
  int64_t  cnoise_as;

  for (i=0; i<nSamples; i++) {
    cnoise_as        =  calcNoise_as(noise_as);
    tEdgeNoisy_as[i] = tEdge_as[i] + cnoise_as;
    //printf("--\n");
    //printf("edge      %14lu\n",  tEdge_as[i]);
    //printf("noise     %14ld\n",  cnoise_as);
    //printf("edgeNoisy %14lu\n",  tEdgeNoisy_as[i]);
  } // for i
} // calcTEdgenoisy_as


void calcTStamp() {
  int    i;

  for (i=0; i<nSamples; i++) tStamp[i] = floor(tEdgeNoisy_as[i] / one_ns_as);
  //printf("tstamp   %lu \n", tStamp[0]);
  //printf("edgenosy %lu \n", tEdgeNoisy_as[0]);
} // calcTstamps


int main(int argc, char** argv) {
  //const char* command;

  int  opt, error = 0;
  int  exitCode   = 0;
  char *tail;
  char *tmp;

  b2bt_t   phase_t;
  int64_t  tPhase1_as, tPhase2_as;
  double   tEdge1        = 0;
  double   tEdgeNoisy1;
  int64_t  diff_as;
  uint32_t width_as;
  int      scanType      = 0;                 // 0: don't scan, 1: tOffset1
  uint64_t scanInc_as    = 1;                 // increment of scan
  int64_t  offsetNoise_as;
  double   max           = -1000;
  double   min           = 1000;
  double   ave           = 0;
  double   stdev         = 0;
  double   ave_width     = 0;
  uint32_t max_width_as  = 0;
  uint64_t nPeriods;
  
  int     i;
  
  sprintf(filename, "%s", "");
  nPeriods         = (floor)(((uint64_t)15900000 * one_ns_as) / TH1_as);
 
  program = argv[0];    

  while ((opt = getopt(argc, argv, "T:c:i:t:n:f:m:o:p:s:r:d:eh")) != -1) {
    switch (opt) {
      case 'e' :
        getVersion = 1;
        break;
      case 'h' :
        help();
        return 0;
        break;
      case 'T' :
        TH1_as      = strtol(optarg, &tail, 0) * 1000;                     // fs -> as
        break;
      case 's' :
        nSamples    = strtol(optarg, &tail, 0);
        break;
      case 'r' :
        noise_as    = strtol(optarg, &tail, 0) * 1000;                     // fs -> as
        break;
      case 'd' :
        nData       = strtol(optarg, &tail, 0);
        break;
      case 'o' :
        tOffset1_as = strtol(optarg, &tail, 0) * 1000;                     // fs -> as
        break;
      case 'p' :
        noiseO_as   = strtol(optarg, &tail, 0) * 1000;                     // fs -> as              
        break;
      case 'm' :
        mode        = strtol(optarg, &tail, 0);
        break;
      case 't' :
        fit         = strtol(optarg, &tail, 0);
        break;
      case 'c' :
        scanType    = strtol(optarg, &tail, 0);
        break;
      case 'i' :
        scanInc_as  = strtol(optarg, &tail, 0) * 1000;                     // fs -> as
        break;
      case 'n' :
        nPeriods    = strtol(optarg, &tail, 0);
        break;
      case 'f' :
        tmp = strtok(optarg, " ");
        if (strlen(tmp) == 0) {
          fprintf(stderr, "specify a proper name, not '%s'!\n", optarg);
          exit(1);
        } // if strlen
        sprintf(filename, "%s", tmp);
        break;
      default:
        fprintf(stderr, "%s: bad getopt result\n", program);
        return 1;
    } /* switch opt */
  } /* while opt */

  if (error) {
    help();
    return 1;
  }

  if ((nData == 0) || (nData > MAXDATA)) {
    printf("option 'd' (nData): valid range is 1..%d\n", MAXDATA);
    return 1;
  } // if nData

  if ((nSamples < 3) || (nSamples > MAXSAMPLES)) {
    printf("option 's' (nSamples): valid range is 3..%d\n", MAXSAMPLES);
    return 1;
  } // if nSamples

  if ((mode < 1) || (mode > 2)) {
    printf("option 'm' (mode): valid range is 1..2\n");
    return 1;
  } // if mode
  
  if (getVersion) {
    printf("b2b: sim version %x\n",  B2BSIM_VERSION);     
  } // if getVersion

  if (strlen(filename) > 0) {
    dataFile = fopen(filename, "w");
  } // if output file

  srandom(time(NULL));
  tEdgeNoisy1      = 0;
  tPhase1_as       = 0;
  tPhase2_as       = 0;
  diff_as          = 0;
  ave_width        = 0;
  // looping over j can be used to produce nice figures 
  /*for (int j=0; j<1000; j++) {
    TH1_as += 1000000;
    max = 0;*/
  for (i=0; i<nData; i++) {
    switch (scanType) {
      case 1 : 
        tOffset1_as    += scanInc_as;
        break;
      case 2 :
        nPeriods       += scanInc_as / 1000;
      default :
        break;
    } // switch scanType
    tOffset2_as     = tOffset1_as + nPeriods * TH1_as;

    offsetNoise_as   = calcNoise_as(noiseO_as);
    calcTEdge_as(tOffset1_as + offsetNoise_as);
    calcTEdgeNoisy_as();
    calcTStamp();
    phaseFitAverage(TH1_as, nSamples, &phase_t, &width_as);
    tPhase1_as     = phase_t.ns * one_ns_as + phase_t.ps * 1000000 + one_ns_as / 2;
    tEdge1         = (double)(tEdge_as[1])      / one_ns_as;
    tEdgeNoisy1    = (double)(tEdgeNoisy_as[1]) / one_ns_as;
    ave_width     += (double)width_as           / one_ns_as;
    if (width_as > max_width_as) max_width_as = width_as;

    switch (mode) {
      case 1 :
        diff_as    = tPhase1_as - tEdgeNoisy_as[1];
        break;
      case 2 :
        calcTEdge_as(tOffset2_as + offsetNoise_as);
        calcTEdgeNoisy_as();
        calcTStamp();
        phaseFitAverage(TH1_as, nSamples, &phase_t, &width_as);
        tPhase2_as = phase_t.ns * one_ns_as + phase_t.ps * 1000000 + one_ns_as / 2;
        diff_as    = tPhase2_as - tPhase1_as;
        diff_as    = diff_as % TH1_as;
        if (diff_as > (TH1_as >> 1)) diff_as = diff_as - TH1_as;
        break;
      default :
        break;
    } //switch mode
    dev[i]  = (double)diff_as / (double)one_ns_as;
    //printf("dev %13.3f\n", dev[i]);
    ave    += dev[i];
    if (dev[i] > max) max = dev[i];
    if (dev[i] < min) min = dev[i];

    if (dataFile) fprintf(dataFile, "%13.6f    %13.6f    %13lu\n", (double)tOffset1_as/1000000000.0, dev[i], nPeriods);
  } // for i;
  // looping over 'j'; consider commenting the fprintf statement just above
  /*if (dataFile) fprintf(dataFile, "ps offset; %4d; maxDev; %13.6f; diffSim; %13.6f\n", j, (double)calcMaxSysDev_ps(TH1_as / 1000, nSamples)/1000.0, (double)max * 1000.0);
  } // for j;*/
  ave       = ave       / nData;
  ave_width = ave_width / nData;

  for (i=0; i<nData; i++) stdev += (dev[i] - ave)*(dev[i] - ave);
  stdev = stdev / nData;
  stdev = sqrt(stdev);

  printf("parameters [ns]:\n");
  printf("mode          (-m): %13d\n"    , mode);
  printf("fit           (-t): %13d\n"    , fit);
  printf("scan type     (-c): %13d\n"    , scanType);
  printf("scan incrmnt  (-i): %13.6f\n"  , (double)scanInc_as  / (double)one_ns_as);
  printf("T_rev         (-T): %13.6f\n"  , (double)TH1_as  / (double)one_ns_as);
  if (strlen(filename) > 0)
    printf("file          (-f): %13s\n"  , filename);
  if (mode == 2)
    printf("nPeriods      (-n): %13lu\n"  , nPeriods);
  printf("n samples     (-s): %13d\n"    , nSamples);
  printf("n data        (-d): %13d\n"    , nData);
  printf("offset1       (-o): %13.3f\n"  , (double)tOffset1_as / (double)one_ns_as);
  if (mode == 2)
    printf("offset2       (-o): %13.3f\n", (double)tOffset2_as / (double)one_ns_as);
  printf("noise tstamps (-r): %13.3f\n"  , (double)noise_as    / (double)one_ns_as);
  printf("noise toffset (-p): %13.3f\n"  , (double)noiseO_as   / (double)one_ns_as);
  printf("1st h=1           : %13.3f\n"  , tEdge1);
  printf("1st h=1 w noise   : %13.3f\n"  , tEdgeNoisy1);  
  printf("1st phase         : %13.3f\n"  , (double)tPhase1_as  / (double)one_ns_as);
  printf("1st deviation     : %13.3f\n"  , (double)tPhase1_as  / (double)one_ns_as - tEdgeNoisy1);
  printf("\n");
  printf("stats [ps]:\n");
  printf("comb              : %13.3f\n", 1000.0 / (double)nSamples);
  printf("ave_width (of fit): %13.3f\n", ave_width *      1000);
  printf("max_width (of fit): %13.3f\n", (double)max_width_as / 1000000.0);  
  printf("average deviation : %13.3f\n", ave   *          1000);
  printf("min deviation     : %13.3f\n", min   *          1000);
  printf("max deviation     : %13.3f\n", max   *          1000);
  printf("stdev             : %13.3f\n", stdev *          1000);                    // ... moreover, this should be added quadratically
  printf("FWHM              : %13.3f\n", stdev * 2.3548 * 1000);                    // ... moreover, if the phase at the beginning of that flat-top is not fixed, the systematic deviation will cancel out ...

  printf("\n");
  printf("'native' max_sysdev from b2blib [ps]: %u\n", b2b_calc_max_sysdev_ps(TH1_as, nSamples, 1));
  
  if (dataFile) fclose(dataFile);
  
  return exitCode;
}
