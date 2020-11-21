/********************************************************************************************
 *  b2b-api.c
 *
 *  created : 2018
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 21-November-2020
 *
 *  implementation for b2b
 * 
 *  see b2b-api.h for version, license and documentation 
 *
 ********************************************************************************************/
// standard includes
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

// etherbone
#include <etherbone.h>

// API (x86) b2b
#include <b2b.h>                   // b2b defs
#include <common-defs.h>           // lm32 common defs
#include <b2b-api.h>               // b2b api defs

// public variables
// something like eb_address_t b2b_statusLo;         // status of b2b, read (low word)

/* chk, I think we don't need this
static void die(const char* where, eb_status_t status)
{
  fprintf(stderr, "%s failed: %s\n", where, eb_status(status));
  exit(1);
} //die
*/

// init for communicaiton with shared mem
void api_initShared(eb_address_t lm32_base, eb_address_t sharedOffset)
{
  // do something like b2b_statusLo     = lm32_base + sharedOffset + COMMON_SHARED_STATUSLO;
} // api_initShared


double api_flsa2fdds(double flsa)
{
  double twoep32;
  double twoem32;
  double fclk;
  double fdds;

  twoep32 = pow(2,  32);
  twoem32 = pow(2, -32);
  fclk    = (double)B2B_F_CLK;

  fdds   = twoem32 * floor(twoep32 * flsa / fclk) * fclk;

  return fdds;
} // api_flsa2fdds


