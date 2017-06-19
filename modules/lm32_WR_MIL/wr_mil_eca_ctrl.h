#ifndef WR_MIL_ECA_CTRL_H_
#define WR_MIL_ECA_CTRL_H_

#include <stdint.h>
#include "wr_mil_value64bit.h"

volatile uint32_t *ECACtrl_init();
void ECACtrl_getTAI(volatile uint32_t *eca, TAI_t *tai);

#endif
