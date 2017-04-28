#include "wr_mil_eca_ctrl.h"

#include "../../ip_cores/wr-cores/modules/wr_eca/eca_queue_regs.h"
#include "../../ip_cores/wr-cores/modules/wr_eca/eca_regs.h"       // register layout ECA controle


volatile ECARegs *ECACtrl_init(uint32_t *device_addr)
{
	return (volatile ECARegs*)device_addr;
}

void ECACtrl_getTAI(volatile ECARegs *eca, TAI_t *tai)
{
  do {
    tai->part.hi = eca->time_hi_get;
    tai->part.lo = eca->time_lo_get;
  } while (tai->part.hi != eca->time_hi_get); 
  /* repeat until high time is did not change while reading low time */
}

