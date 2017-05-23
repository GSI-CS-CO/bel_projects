#include "wr_mil_eca_ctrl.h"

#include "../../ip_cores/wr-cores/modules/wr_eca/eca_queue_regs.h"
#include "../../ip_cores/wr-cores/modules/wr_eca/eca_regs.h"       // register layout ECA controle
#include "mini_sdb.h"

#ifndef UNITTEST
volatile ECACtrlRegs *ECACtrl_init()
{
  return (volatile ECACtrlRegs*)find_device_adr(ECA_SDB_VENDOR_ID, ECA_SDB_DEVICE_ID);
}
#endif

void ECACtrl_getTAI(volatile ECACtrlRegs *eca, TAI_t *tai)
{
  do {
    tai->part.hi = eca->time_hi_get;
    tai->part.lo = eca->time_lo_get;
  } while (tai->part.hi != eca->time_hi_get); 
  /* repeat until high time is did not change while reading low time */
}

