#ifndef _FWLIB_H_
#define _FWLIB_H_

/* includes specific for bel_projects */
#include "dbg.h"              // DBPRINT()
#include "mini_sdb.h"         // find_dev()
#include "aux.h"              // atomic_on/off()
#include "eca_queue_regs.h"   // register layout ECA queue
#include "eca_regs.h"         // register layout ECA control
#include "eca_flags.h"        // definitions for ECA queue

/* includes for this project */
#include <common-defs.h>      // common definitions

// return (error) status
uint32_t findEcaCtl();

// return number of the ECA valid actions
uint32_t fwlib_getEcaValidCnt();

#endif
