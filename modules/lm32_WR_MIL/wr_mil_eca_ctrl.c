#include "wr_mil_eca_ctrl.h"

#include "../../ip_cores/wr-cores/modules/wr_eca/eca_queue_regs.h"
#include "../../ip_cores/wr-cores/modules/wr_eca/eca_regs.h"       // register layout ECA controle

/* initialization routine */
void ECACtrl_init(ECACtrl_t *ctrl, uint32_t *device_addr) // find WB address of ECA Control
{
  ctrl->pECACtrl   =  device_addr;
  ctrl->pECATimeHi = (ctrl->pECACtrl + (ECA_TIME_HI_GET >> 2));
  ctrl->pECATimeLo = (ctrl->pECACtrl + (ECA_TIME_LO_GET >> 2));
}

/* get TAI from ECA control */
void ECACtrl_getTAI(ECACtrl_t *ctrl, TAI_t *tai)
{
  do {
    tai->part.hi = *ctrl->pECATimeHi;
    tai->part.lo = *ctrl->pECATimeLo;
  } while (tai->part.hi != *ctrl->pECATimeHi); 
  /* repeat until high time is did not change while reading low time */
}

