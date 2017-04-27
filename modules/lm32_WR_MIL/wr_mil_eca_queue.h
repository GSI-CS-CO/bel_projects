#ifndef WR_MIL_ECA_QUEUE_H_
#define WR_MIL_ECA_QUEUE_H_

#include "wr_mil_tai.h"

/* stucture and functions to read events from the eca queue */

typedef struct 
{
  volatile uint32_t *pECAQ;
  volatile uint32_t *pEvtIdHi;
  volatile uint32_t *pEvtIdLo;
  volatile uint32_t *pEvtDeadlHi;
  volatile uint32_t *pEvtDeadlLo;
  volatile uint32_t *pActTag;
  volatile uint32_t *pQueuePopOwr;
} ECAQueue_t;

void ECAQueue_init(ECAQueue_t *queue, uint32_t *device_addr);
void ECAQueue_popAction(ECAQueue_t *queue);
void ECAQueue_getDeadl(ECAQueue_t *queue, TAI_t *deadl);
void ECAQueue_getMilEventData(ECAQueue_t *queue, uint32_t *evtNo, 
                                                 uint32_t *evtCode, 
                                                 uint32_t *virtAcc);

#endif
