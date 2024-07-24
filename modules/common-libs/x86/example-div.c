/********************************************************************************************
 *  example-div.c
 *
 *  created : 2023
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 14-Jun-2023
 *
 *  command-line interface for a few examples
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
 * Last update: 17-May-2017
 *********************************************************************************************/
#define DIV_X86_VERSION  "00.00.03"

// standard includes 
#include <unistd.h> // getopt
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

// common example
#include <common-lib.h>                  // common API
#include <common-defs.h>                 // FW
#include <common-fwlib.h>                // b2bt, hackish

const char*  program;

static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION] <single precision number>\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -h                  display this help and exit\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Small test program for conversion between half precision and \n");
  fprintf(stderr, "single precision numbers \n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");
  fprintf(stderr, "Version %s. Licensed under the LGPL v3.\n", DIV_X86_VERSION);
} //help


// just a copy, hackish
b2bt_t fwlib_cleanB2bt(b2bt_t t_ps)
{
  while (t_ps.ps < -500) {t_ps.ns -= 1; t_ps.ps += 1000;}
  while (t_ps.ps >= 500) {t_ps.ns += 1; t_ps.ps -= 1000;}

  return t_ps;
} // alignB2bt


// just a copy, hackish
b2bt_t fwlib_tfns2tps(float t_ns)
{
  b2bt_t t_ps;

  t_ps.ns = t_ns;
  t_ps.ps = (t_ns - (float)(t_ps.ns)) * 1000.0;
  t_ps    = fwlib_cleanB2bt(t_ps);

  return t_ps;
} // tfns2ps


// just a copy, hackish
float fwlib_tps2tfns(b2bt_t t_ps)
{  
  float  tmp1, tmp2;
  
  tmp1 = (float)(t_ps.ns);
  tmp2 = (float)(t_ps.ps) / 1000.0;

  return tmp1 + tmp2;;
} // fwlib_tps2tfns


int main(int argc, char** argv) {

  // CLI
  int opt, error = 0;
  char *tail;

  // local variables

  float    number;

  uint16_t half;
  float    single;
  float    diff;
  float    relative;
  b2bt_t   t_ps;
  float    t_ns;
  uint16_t half_us;
  float    single_us;

  program = argv[0];    

  while ((opt = getopt(argc, argv, "h")) != -1) {
    switch (opt) {
    case 'h':
      help();
      return 0;
      break;
    default:
      fprintf(stderr, "%s: bad getopt result\n", program);
      return 1;
    } // switch opt
  } // while opt

  if (error) {
    help();
    return 1;
  }
  
  if (optind >= argc) {
    fprintf(stderr, "%s: expecting one non-optional argument: <number>\n", program);
    fprintf(stderr, "\n");
    help();
    return 1;
  }

  number   = atof(argv[optind]);

  half     = comlib_float2half(number);
  single   = comlib_half2float(half);
  diff     = single - number;
  relative = diff / single;

  printf("converting to half precision and back\n");
  printf("original     : %13.6f\n", number);
  printf("half float   :      0x%04x\n", half); 
  printf("back to float: %13.6f\n", single);
  printf("absolute diff: %13.6f\n", diff);
  printf("relative diff: %14.3e\n", relative);

  t_ps     = fwlib_tfns2tps(number);
  t_ns     = fwlib_tps2tfns(t_ps);
  half_us  = comlib_float2half(t_ns / 1000.0);
  single_us= comlib_half2float(half_us);
  
  printf("\nconverting b2bt\n");
  printf("original     : %13.6f\n", number);
  printf("ns part      : %13lu\n" , t_ps.ns);
  printf("ps part      : %13d\n"  , t_ps.ps);
  printf("to float ns  : %13.6f\n", t_ns);
  printf("half float   :      0x%04x\n", half_us);
  printf("to float us  : %13.6f\n", single_us);
  
  
  return 0;
} // main
