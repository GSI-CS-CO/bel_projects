#ifndef WR_MIL_ECA_CTRL_H_
#define WR_MIL_ECA_CTRL_H_

#include "wr_mil_tai.h"

typedef struct
{
  volatile uint32_t *pECACtrl;
  volatile uint32_t *pECATimeHi;
  volatile uint32_t *pECATimeLo;
} ECACtrl_t;

/* device_addr can be obtained using find_device_adr(ECA_SDB_VENDOR_ID, ECA_SDB_DEVICE_ID); */
void ECACtrl_init(ECACtrl_t *ctrl, uint32_t *device_addr);
void ECACtrl_getTAI(ECACtrl_t *ctrl, TAI_t *tai);

#endif
