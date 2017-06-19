#include "wr_mil_eca_ctrl.h"

#include "../../ip_cores/wr-cores/modules/wr_eca/eca_queue_regs.h"
#include "../../ip_cores/wr-cores/modules/wr_eca/eca_regs.h"       // register layout ECA controle

#ifndef UNITTEST
#include "mini_sdb.h"
volatile uint32_t *ECACtrl_init()
{
  return (volatile uint32_t*)find_device_adr(ECA_SDB_VENDOR_ID, ECA_SDB_DEVICE_ID);
}
#endif

void ECACtrl_getTAI(volatile uint32_t *eca, TAI_t *tai)
{
  do {
    tai->part.hi = *((uint32_t*)eca + ECA_TIME_HI_GET/4);
    tai->part.lo = *((uint32_t*)eca + ECA_TIME_LO_GET/4);
  } while (tai->part.hi != *((uint32_t*)eca + ECA_TIME_HI_GET/4)); 
  /* repeat until high time did not change while reading low time */
}

