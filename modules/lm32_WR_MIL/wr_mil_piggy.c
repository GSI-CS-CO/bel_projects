#include "wr_mil_piggy.h"
#include "mini_sdb.h"

#include "wr_mil_utils.h"

// instead of including the scu_mil.h header, we redefine the constants to be able to 
// build stand-alone C programs for testing of this module 

#ifndef UNITTEST
#include "../../top/gsi_scu/scu_mil.h"
volatile MilPiggyRegs *MilPiggy_init()
{
     return (volatile MilPiggyRegs*) find_device_adr(GSI, SCU_MIL);// get Wishbone address for MIL piggy 
}
#endif 

void MilPiggy_writeCmd(volatile MilPiggyRegs *piggy, uint32_t cmd)
{
#ifndef UNITTEST
     while(!(piggy->wr_rd_status & MIL_CTRL_STAT_TRM_READY)) // wait until ready
     {
          DELAY05us; // delay a bit to have less pressure on the wishbone bus
     }
#endif
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

