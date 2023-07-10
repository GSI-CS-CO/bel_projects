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

/**
 * \brief Return number of the actions output by ECA channel
 *
 * ECA action channels are linked to ECA queue.
 * All actions (valid, failed, overflow, full) output by ECA channel are counted
 * and kept in a set of clear-on-read registers.
 * This function return the number of the actions output by a chosen ECA channel,
 * which is linked to an ECA queue specified by pECAQ (eCPU in this case).
 *
 * \param offset   Offset to the counter register
 * \param buffer   Buffer to keep the number of ECA actions
 *
 * \ret   status   On success return OK, otherwise FAIL
 **/
status_t fwlib_getEcaCnt(uint8_t offset, uint32_t *buffer)
{
  uint32_t count = 0;

  if (pEcaCtl) {
    uint32_t queueId  = *(pECAQ + (ECA_QUEUE_QUEUE_ID_GET >> 2));  // ECA queue ID

    atomic_on();
    *(pEcaCtl + (ECA_CHANNEL_SELECT_RW >> 2)) = queueId + 1;       // select ECA channel, to which the queue is connected
    *(pEcaCtl + (ECA_CHANNEL_NUM_SELECT_RW >> 2)) = 0;             // set the subchannel index to 0
    count = *(pEcaCtl + (offset >> 2));                            // read and clear the action counter
    atomic_off();

    *buffer = count;

    return COMMON_STATUS_OK;
  }

  return COMMON_STATUS_ERROR;
}

/**
 * \brief return number of the valid actions
 *
 * All ECA actions (valid, failed, overflow, full) are counted and kept in a set of
 * clear-on-read registers.
 *
 * \param buffer   buffer to keep the number of ECA actions
 *
 * \ret   status  on success return OK, otherwise FAIL
 **/
status_t fwlib_getEcaValidCnt(uint32_t *buffer)
{
  return fwlib_getEcaCnt(ECA_CHANNEL_VALID_COUNT_GET, buffer);
}

/**
 * \brief return number of the overflow actions
 *
 * All ECA actions (valid, failed, overflow, full) are counted and kept in a set of
 * clear-on-read registers.
 *
 * \param buffer   buffer to keep the number of ECA actions
 *
 * \ret   status  on success return OK, otherwise FAIL
 **/
status_t fwlib_getEcaOverflowCnt(uint32_t *buffer)
{
  return fwlib_getEcaCnt(ECA_CHANNEL_OVERFLOW_COUNT_GET, buffer);
}

/**
 * \brief return number of a failed ECA actions
 *
 * All ECA actions (valid, failed, overflow, full) are counted and kept in a set of
 * clear-on-read registers.
 * To get the number of failed actions a failure flag with valid value
 * (late, early, conflict, delayed) is required.
 *
 * \param flag     error flag (used to read failed action counter)
 * \param buffer   buffer to keep the number of ECA actions
 *
 * \ret   status  on success return OK, otherwise FAIL
 **/
status_t fwlib_getEcaFailureCnt(uint32_t flag, uint32_t *buffer)
{

  if ((flag != ECA_LATE) && (flag != ECA_EARLY) && (flag != ECA_CONFLICT) && (flag != ECA_DELAYED))
    return COMMON_STATUS_ERROR;

  if (!pEcaCtl)
    return COMMON_STATUS_ERROR;

  uint32_t queueId  = *(pECAQ + (ECA_QUEUE_QUEUE_ID_GET >> 2)); // ECA queue ID

  atomic_on();
  *(pEcaCtl + (ECA_CHANNEL_SELECT_RW >> 2)) = queueId + 1;     // select ECA channel, to which the queue is connected
  *(pEcaCtl + (ECA_CHANNEL_NUM_SELECT_RW >> 2)) = 0;           // set the subchannel index to 0
  *(pEcaCtl + (ECA_CHANNEL_CODE_SELECT_RW >> 2)) = flag;       // select failure type (late, early, conflict, delayed)
  *buffer = *(pEcaCtl + (ECA_CHANNEL_FAILED_COUNT_GET >> 2));  // read and clear the failure counter
  atomic_off();

  return COMMON_STATUS_OK;
}
