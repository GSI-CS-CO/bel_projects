#ifndef _FWLIB_H_
#define _FWLIB_H_

/* includes specific for bel_projects */
#include "dbg.h"              // DBPRINT()
#include "mini_sdb.h"         // find_dev()
#include "aux.h"              // atomic_on/off()
#include "ebm.h"              // ebm_ops, SOURCE/DESTINATION

// ip_cores/saftlibi/drivers or
// ip_cores/wr-cores/modules/wr_eca
#include "eca_queue_regs.h"   // register layout ECA queue
#include "eca_regs.h"         // register layout ECA control
#include "eca_flags.h"        // definitions for ECA queue

/* includes for this project */
#include <common-defs.h>      // common definitions
#include "fbas_common.h"      // COMMON_STATUS_

// return (error) status
uint32_t findEcaCtl();

status_t fwlib_getEcaValidCnt(uint32_t *buffer);
status_t fwlib_getEcaOverflowCnt(uint32_t *buffer);
status_t fwlib_getEcaFailureCnt(uint32_t flag, uint32_t *buffer);
void fwlib_setEbmDstAddr(uint64_t dstMac, uint32_t dstIp);

#endif
