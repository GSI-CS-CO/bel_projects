#include "wr_mil_eca_ctrl.h"

#include "../../ip_cores/wr-cores/modules/wr_eca/eca_queue_regs.h"
#include "../../ip_cores/wr-cores/modules/wr_eca/eca_regs.h"       // register layout ECA controle
#include "mini_sdb.h"
#include "wr_mil_delay.h"

volatile uint32_t *ECACtrl_init()
{
  return (volatile uint32_t*)find_device_adr(ECA_SDB_VENDOR_ID, ECA_SDB_DEVICE_ID);
}

void ECACtrl_getTAI(volatile uint32_t *eca, TAI_t *tai)
{
  do {
    tai->part.hi = eca[ECA_TIME_HI_GET/4];
    tai->part.lo = eca[ECA_TIME_LO_GET/4];
  } while (tai->part.hi != eca[ECA_TIME_HI_GET/4]); 
  /* repeat until high time did not change while reading low time */
}

uint32_t wait_until_tai(volatile uint32_t *eca, uint64_t stopTAI)
{
  // Get current time, ...
  TAI_t tai_now; 
  ECACtrl_getTAI(eca, &tai_now);
  if (stopTAI < tai_now.value) return tai_now.value-stopTAI; // (stopTAI is in the past)
  // ... calculate waiting time, ...
  uint32_t delay = (stopTAI - tai_now.value)/32; 
  // ... and wait.
  delay_96plus32n_ns(delay);
  return 0;
}