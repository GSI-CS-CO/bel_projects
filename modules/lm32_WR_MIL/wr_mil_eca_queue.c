#include "wr_mil_eca_queue.h"

#include "../../ip_cores/wr-cores/modules/wr_eca/eca_queue_regs.h"
#include "../../ip_cores/wr-cores/modules/wr_eca/eca_regs.h"       // register layout ECA controle
#include "../../ip_cores/saftlib/drivers/eca_flags.h"

volatile ECAQueueRegs *ECAQueue_init(uint32_t *device_addr)
{
  return (volatile ECAQueueRegs*)device_addr;
}

void ECAQueue_popAction(volatile ECAQueueRegs *queue)
{
  queue->pop_owr = 0x1;
}

void ECAQueue_getDeadl(volatile ECAQueueRegs *queue, TAI_t *deadl)
{
  deadl->part.hi = queue->deadline_hi_get;
  deadl->part.lo = queue->deadline_lo_get;
}
void ECAQueue_getEvtId(volatile ECAQueueRegs *queue, EvtId_t *evtId)
{
  evtId->part.hi = queue->event_id_hi_get;
  evtId->part.lo = queue->event_id_lo_get;
}
uint32_t ECAQueue_getActTag(volatile ECAQueueRegs *queue)
{
  return queue->tag_get;
}
uint32_t ECAQueue_getFlags(volatile ECAQueueRegs *queue)
{
	return queue->flags_get;
}
uint32_t ECAQueue_clear(volatile ECAQueueRegs *queue)
{
	uint32_t n;
	for (n = 0; queue->flags_get & (1<<ECA_VALID); ++n) 
	{
		queue->pop_owr = 0x1;
	}
	return n;
}


void ECAQueue_getMilEventData(volatile ECAQueueRegs *queue, uint32_t *evtNo,
                                                            uint32_t *evtCode,
                                                            uint32_t *virtAcc)
{
  uint32_t evtIdHi = queue->event_id_hi_get;
  uint32_t evtIdLo = queue->event_id_lo_get;
  *evtNo   = (evtIdHi>>4)  & 0x00000fff;
  *evtCode = *evtNo        & 0x000000ff;
  *virtAcc = (evtIdLo>>24) & 0x0000000f;
}
