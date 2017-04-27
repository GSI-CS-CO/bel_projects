#include "wr_mil_eca_queue.h"

#include "../../ip_cores/wr-cores/modules/wr_eca/eca_queue_regs.h"
#include "../../ip_cores/wr-cores/modules/wr_eca/eca_regs.h"       // register layout ECA controle

void ECAQueue_init(ECAQueue_t *queue, uint32_t *device_addr)
{
  queue->pECAQ        = device_addr;
  queue->pEvtIdHi     = queue->pECAQ + (ECA_QUEUE_EVENT_ID_HI_GET >> 2);
  queue->pEvtIdLo     = queue->pECAQ + (ECA_QUEUE_EVENT_ID_LO_GET >> 2);
  queue->pEvtDeadlHi  = queue->pECAQ + (ECA_QUEUE_DEADLINE_HI_GET >> 2);
  queue->pEvtDeadlLo  = queue->pECAQ + (ECA_QUEUE_DEADLINE_LO_GET >> 2);
  queue->pActTag      = queue->pECAQ + (ECA_QUEUE_TAG_GET         >> 2);
  queue->pQueuePopOwr = queue->pECAQ + (ECA_QUEUE_POP_OWR         >> 2);
}

void ECAQueue_popAction(ECAQueue_t *queue)
{
  *queue->pQueuePopOwr = 0x1;
}

void ECAQueue_getDeadl(ECAQueue_t *queue, TAI_t *deadl)
{
  deadl->part.hi = *queue->pEvtDeadlHi;
  deadl->part.lo = *queue->pEvtDeadlLo;
}

void ECAQueue_getMilEventData(ECAQueue_t *queue, uint32_t *evtNo, 
                                                 uint32_t *evtCode, 
                                                 uint32_t *virtAcc)
{
  uint32_t evtIdHi = *queue->pEvtIdHi;
  uint32_t evtIdLo = *queue->pEvtIdLo;
  *evtNo   = (evtIdHi>>4)  & 0x00000fff;
  *evtCode = *evtNo        & 0x000000ff;
  *virtAcc = (evtIdLo>>24) & 0x0000000f;
}
