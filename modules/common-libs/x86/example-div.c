/********************************************************************************************
 *  example-div.c
 *
 *  created : 2023
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 24-Feb-2023
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
#define DIV_X86_VERSION  "00.00.02"

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
  
  return 0;
} // main
