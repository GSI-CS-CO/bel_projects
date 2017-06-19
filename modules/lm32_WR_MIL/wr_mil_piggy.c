#include "wr_mil_piggy.h"
#include "mini_sdb.h"

#include "wr_mil_utils.h"

// instead of including the scu_mil.h header, we redefine the constants to be able to 
// build stand-alone C programs for testing of this module 

#ifndef UNITTEST
#include "../../top/gsi_scu/scu_mil.h"
volatile uint32_t *MilPiggy_init()
{
     return (volatile uint32_t*) find_device_adr(GSI, SCU_MIL);// get Wishbone address for MIL piggy 
}
#endif 

void MilPiggy_writeCmd(volatile uint32_t *piggy, uint32_t cmd)
{
#ifndef UNITTEST
     while(!(*(piggy + (MIL_REG_WR_RD_STATUS/4)) & MIL_CTRL_STAT_TRM_READY)) // wait until ready
     {
          DELAY05us; // delay a bit to have less pressure on the wishbone bus
     }
#endif
	*(piggy + (MIL_REG_WR_CMD/4)) = cmd;
}

void MilPiggy_lemoOut1Enable(volatile uint32_t *piggy)
{
     //disableFilterEvtMil(piggy);
     //enableFilterEvtMil(piggy);
     //configLemoGateEvtMil(piggy, 1);
	*(piggy + (MIL_REG_WR_RF_LEMO_CONF/4)) |= MIL_LEMO_OUT_EN1;
}
void MilPiggy_lemoOut2Enable(volatile uint32_t *piggy)
{
     //disableFilterEvtMil(piggy);
     //enableFilterEvtMil(piggy);
     //configLemoGateEvtMil(piggy, 2);
	*(piggy + (MIL_REG_WR_RF_LEMO_CONF/4)) |= MIL_LEMO_OUT_EN2;
}
void MilPiggy_lemoOut1High(volatile uint32_t *piggy)
{
     setLemoOutputEvtMil(piggy, 1, 1);
}
void MilPiggy_lemoOut2High(volatile uint32_t *piggy)
{
     setLemoOutputEvtMil(piggy, 2, 1);
}
void MilPiggy_lemoOut1Low(volatile uint32_t *piggy)
{
     setLemoOutputEvtMil(piggy, 1, 0);
}
void MilPiggy_lemoOut2Low(volatile uint32_t *piggy)
{
     setLemoOutputEvtMil(piggy, 2, 0);
}

uint32_t MilPiggy_readConf(volatile uint32_t *piggy)
{
     return *(piggy + (MIL_REG_WR_RF_LEMO_CONF/4));
}