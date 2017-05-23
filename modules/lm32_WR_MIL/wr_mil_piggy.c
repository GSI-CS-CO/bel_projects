#include "wr_mil_piggy.h"
#include "mini_sdb.h"
#include "../../top/gsi_scu/scu_mil.h"

/* function implementation */
volatile MilPiggyRegs *MilPiggy_init(uint32_t *device_addr)
{
     if (!device_addr) // find device address via SDB
     {
          return (volatile MilPiggyRegs*) find_device_adr(GSI, SCU_MIL);// get Wishbone address for MIL piggy 
     }
	return (volatile MilPiggyRegs*) device_addr;
}

void MilPiggy_writeCmd(volatile MilPiggyRegs *piggy, uint32_t cmd)
{
     while(!(piggy->wr_rd_status & MIL_CTRL_STAT_TRM_READY)); // wait until ready
	piggy->wr_cmd = cmd;
}

void MilPiggy_lemoOut1Enable(volatile MilPiggyRegs *piggy)
{
	piggy->wr_rf_lemo_conf |= MIL_LEMO_OUT_EN1;
}
void MilPiggy_lemoOut2Enable(volatile MilPiggyRegs *piggy)
{
	piggy->wr_rf_lemo_conf |= MIL_LEMO_OUT_EN2;
}
void MilPiggy_lemoOut1High(volatile MilPiggyRegs *piggy)
{
	piggy->wr_rd_lemo_dat |= MIL_LEMO_DAT1;
}
void MilPiggy_lemoOut2High(volatile MilPiggyRegs *piggy)
{
	piggy->wr_rd_lemo_dat |= MIL_LEMO_DAT2;
}
void MilPiggy_lemoOut1Low(volatile MilPiggyRegs *piggy)
{
	piggy->wr_rd_lemo_dat &= ~MIL_LEMO_DAT1;
}
void MilPiggy_lemoOut2Low(volatile MilPiggyRegs *piggy)
{
	piggy->wr_rd_lemo_dat &= ~MIL_LEMO_DAT2;
}

