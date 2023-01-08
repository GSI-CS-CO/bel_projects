/*******************************************************************************************
 *  b2b-sim.c
 *
 *  created : 2013
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 07-Jan-2023
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
#define B2BSIM_VERSION 0x000420
#define MAXSAMPLES     1000
#define MAXDATA        100000

// standard includes 
#include <unistd.h> // getopt
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

const char* program;
static int getVersion = 0;

typedef struct{                                      
  uint64_t ns;                                        // full nanoseconds of time
  int32_t  ps;                                        // ps fraction of time, should be positive
  uint32_t dps;                                       // uncertainty [ps]
} b2bt_t;


uint64_t   TH1_as     = 1283767311562;     // h=1 period [as]
int        mode       = 1;                 // simulte 1: single phase 2: phase difference
int        nSamples   = 3;                 // number of samples to be used
int        nData      = 1;                 // number of data
int64_t    noise_fs   = 0;                 // amplitude of noise on timestamps[fs]
double     tOffset    = 1000.0;            // offset of first timestamp
int64_t    noiseO_fs  = 0;                 // amplitude of noise on tOffset 
double     tEdge[MAXSAMPLES];              // rising edges of h=1 signal
double     tEdgeNoisy[MAXSAMPLES];         // rising edges of h=1 signal with noise
double     tStamp[MAXSAMPLES];             // tStamps
double     dev[MAXDATA];                   // deviation for stdev


static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION] <etherbone-device> [COMMAND]\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -h                  display this help and exit\n");
  fprintf(stderr, "  -e                  display version\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");
  fprintf(stderr, "Version %x. Licensed under the LGPL v3.\n", B2BSIM_VERSION);
} //help


// perform fit of phase with sub-ns
int32_t phaseFitAverage(uint64_t TH1_as, uint32_t nSamples, b2bt_t *phase_t) {
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
  uint32_t jitter_as;                       // jitter
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
  jitter_as        = ((uint32_t)(max_deviation_as - min_deviation_as)) >> 2;
  // calculate a phase value and convert to ps
  ts_t.ns          = tFirst_ns;
  ts_t.ps          = ave_deviation_as / 1000000;     // average fit
  //ts_t.ps          = subnsfit_dev_as  / 1000000;     // sub-ns fit
  ts_t.dps         = jitter_as        / 1000000;
  *phase_t         = ts_t;

  //printf("nSamples %d, nGood %d, ts_t [ns] %d, ts_t [ps] %d, dt [ps] %d\n", nSamples, nGood, ts_t.ns, ts_t.ps, ts_t.dps);
  return 0;
} //phaseFitAverage


void calcTEdge(double t0) {                                   // calculates a series of rising h=1 edges
  int    i;

  for (i=0; i<nSamples; i++) tEdge[i] = t0 + (double)i * (double)TH1_as / 1000000000.0;  
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



double calcNoise(int64_t noise_fs) {
  double noise;
  double sigma;

  //noise = (double)noise_fs * ((double)random() / (double)RAND_MAX); // fs, 0..noise_fs
  //noise = noise - (double)noise_fs / 2.0;                           // fs, -noise_fs/2 .. +noise_fs/2
  //noise = noise / 1000000.0;                                        // ns

  sigma = (double)noise_fs / 1000000.0;
  noise = randGauss(sigma, 0);

  return noise;
} // calcNoise


void calcTEdgeNoisy() {                              // adds noise to h=1 eges 
  int    i;

  for (i=0; i<nSamples; i++) {
    tEdgeNoisy[i] = tEdge[i] + calcNoise(noise_fs);
    //printf("noise %f %d\n", noise, RAND_MAX);
  } // for i
} // calcTEdgenoisy


void calcTStamp() {
  int    i;

  for (i=0; i<nSamples; i++) tStamp[i] = (uint64_t)(floor(tEdgeNoisy[i]));
} // calcTstamps



int main(int argc, char** argv) {
  const char* command;

  int opt, error = 0;
  int exitCode   = 0;
  char *tail;

  b2bt_t  phase_t;
  double  tPhase1, tPhase2;
  double  tEdge1;
  double  tEdgeNoisy1;
  double  diff;
  double  max   = 0;
  double  min   = 1000000;
  double  ave   = 0;
  double  stdev = 0;

  int     i;
  
  nSamples       = 0;

  program = argv[0];    

  while ((opt = getopt(argc, argv, "m:o:p:s:r:d:eh")) != -1) {
    switch (opt) {
      case 'e' :
        getVersion = 1;
        break;
      case 'h' :
        help();
        return 0;
        break;
      case 's' :
        nSamples  = strtol(optarg, &tail, 0);
        break;
      case 'r' :
        noise_fs  = strtol(optarg, &tail, 0);
        break;
      case 'd' :
        nData     = strtol(optarg, &tail, 0);
        break;
      case 'o' :
        tOffset   = ((double)strtol(optarg, &tail, 0)) / 1000.0;
        break;
      case 'p' :
        noiseO_fs = strtol(optarg, &tail, 0);
        break;
      case 'm' :
        mode      = strtol(optarg, &tail, 0);
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

  srandom(time(NULL));
  tEdgeNoisy1 = 0;
  tPhase1     = 0;
  diff        = 0;

  for (i=0; i<nData; i++) {
    calcTEdge(tOffset+calcNoise(noiseO_fs));
    calcTEdgeNoisy();
    calcTStamp();
    phaseFitAverage(TH1_as, nSamples, &phase_t);
    tPhase1     = (double)phase_t.ns + (double)phase_t.ps / (double)1000.0 + 0.5;
    tEdge1      = tEdge[1];
    tEdgeNoisy1 = tEdgeNoisy[1];

    switch (mode) {
      case 1 :
        diff    = tPhase1 - tEdgeNoisy[1];
        break;
      case 2 :
        calcTEdgeNoisy();
        calcTStamp();
        phaseFitAverage(TH1_as, nSamples, &phase_t);
        tPhase2 = (double)phase_t.ns + (double)phase_t.ps / (double)1000.0 + 0.5;
        diff    = tPhase2 - tPhase1;
        break;
      default :
        break;
    } //switch mode
    dev[i]  = diff;
    ave    += diff;
    if (diff > max) max = diff;
    if (diff < min) min = diff;
  } // for i;
  ave       = ave / nData;

  for (i=0; i<nData; i++) stdev += (dev[i] - ave)*(dev[i] - ave);
  stdev = stdev / nData;
  stdev = sqrt(stdev);

  printf("infodata [ns]:\n");
  printf("mode      : %13d\n"  , mode);
  printf("n samples : %13d\n"  , nSamples);
  printf("phase     : %13.3f\n", tPhase1);
  printf("edgeNoisy : %13.3f\n", tEdgeNoisy1);
  printf("edge      : %13.3f\n", tEdge1);
  printf("diff      : %13.3f\n", tPhase1 - tEdgeNoisy1);
  printf("\n");
  printf("stats [ps]:\n");
  printf("average   : %13.3f\n", ave   *          1000);
  printf("min       : %13.3f\n", min   *          1000);
  printf("max       : %13.3f\n", max   *          1000);
  printf("stdev     : %13.3f\n", stdev *          1000);
  printf("FWHM      : %13.3f\n", stdev * 2.3548 * 1000);
  //stdev = sqrt(2)*stdev;
  //printf("stdev *1.4: %13.3f\n", stdev);
  //printf("FWHM  *1.4: %13.3f\n", stdev * 2.3548);
  
  return exitCode;
}
