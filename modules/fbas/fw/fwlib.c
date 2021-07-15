#include "fwlib.h"

volatile uint32_t *pEcaCtl;             // WB address of ECA control unit
extern volatile uint32_t *pECAQ;               // WB address of ECA queue

uint32_t findEcaCtl()
{
  pEcaCtl = find_device_adr(ECA_SDB_VENDOR_ID, ECA_SDB_DEVICE_ID);

  if (pEcaCtl)
    return COMMON_STATUS_OK;
  else {
    DBPRINT1("common-fwlib: cannot find ECA control unit\n");
    return COMMON_STATUS_ERROR;
  }
}

// return number of the ECA valid actions
uint32_t fwlib_getEcaValidCnt()
{
  uint32_t actions = 0;  // number of valid actions

  if (pEcaCtl) {
    uint32_t queueId  = *(pECAQ + (ECA_QUEUE_QUEUE_ID_GET >> 2)); // ECA queue ID

    atomic_on();
    *(pEcaCtl + (ECA_CHANNEL_SELECT_RW >> 2)) = queueId + 1;    // select ECA channel, to which the queue is connected
    *(pEcaCtl + (ECA_CHANNEL_NUM_SELECT_RW >> 2)) = 0;          // set the subchannel index to 0
    actions = *(pEcaCtl + (ECA_CHANNEL_VALID_COUNT_GET >> 2));  // read and clear valid counter
    atomic_off();
  }

  return actions;
}

