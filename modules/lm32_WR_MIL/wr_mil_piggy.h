#ifndef WR_MIL_PIGGY_H_
#define WR_MIL_PIGGY_H_

#include <stdint.h>

// #ifdef UNITTEST
// #define   MIL_LEMO_OUT_EN1    0x0001    // '1' ==> LEMO 1 configured as output (MIL Piggy)
// #define   MIL_LEMO_OUT_EN2    0x0002    // '1' ==> LEMO 2 configured as output (MIL Piggy)
// #define   MIL_LEMO_EVENT_EN1  0x0010    // '1' ==> LEMO 1 can be controlled by event (MIL Piggy)
// #define   MIL_LEMO_EVENT_EN2  0x0020    // '1' ==> LEMO 2 can be controlled by event (MIL Piggy)
// #define   MIL_LEMO_DAT1    0x0001    // '1' ==> LEMO 1 is switched active HIGH (MIL Piggy & SIO)
// #define   MIL_LEMO_DAT2    0x0002    // '1' ==> LEMO 2 is switched active HIGH (MIL Piggy & SIO)
// #define  SCU_MIL          0x35aa6b96
// #define MIL_CTRL_STAT_TRM_READY 0x0080
// #endif


volatile uint32_t *MilPiggy_init();

/* write the lower 16 bit of cmd to Mil device bus */
void MilPiggy_writeCmd(volatile uint32_t *piggy, uint32_t cmd);

void MilPiggy_lemoOut1Enable(volatile uint32_t *piggy);
void MilPiggy_lemoOut2Enable(volatile uint32_t *piggy);
void MilPiggy_lemoOut1High(volatile uint32_t *piggy);
void MilPiggy_lemoOut2High(volatile uint32_t *piggy);
void MilPiggy_lemoOut1Low(volatile uint32_t *piggy);
void MilPiggy_lemoOut2Low(volatile uint32_t *piggy);
uint32_t MilPiggy_readConf(volatile uint32_t *piggy);

#endif 
