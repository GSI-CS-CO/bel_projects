#include "wr_mil_eca_ctrl.h"

#include "../../ip_cores/wr-cores/modules/wr_eca/eca_queue_regs.h"
#include "../../ip_cores/wr-cores/modules/wr_eca/eca_regs.h"       // register layout ECA controle
#include "mini_sdb.h"

volatile ECACtrlRegs *ECACtrl_init(uint32_t *device_addr)
{
	if (!device_addr)
	{
		return (volatile ECACtrlRegs*)find_device_adr(ECA_SDB_VENDOR_ID, ECA_SDB_DEVICE_ID);
	}
	return (volatile ECACtrlRegs*)device_addr;
}

void ECACtrl_getTAI(volatile ECACtrlRegs *eca, TAI_t *tai)
{
  do {
    tai->part.hi = eca->time_hi_get;
    tai->part.lo = eca->time_lo_get;
  } while (tai->part.hi != eca->time_hi_get); 
  /* repeat until high time is did not change while reading low time */
}

