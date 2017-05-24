#include "wr_mil_eca_queue.h"

#include "../../ip_cores/saftlib/drivers/eca_flags.h"
#ifndef UNITTEST
#include "../../ip_cores/wr-cores/modules/wr_eca/eca_queue_regs.h"
#include "../../ip_cores/wr-cores/modules/wr_eca/eca_regs.h"       // register layout ECA controle
#include "mini_sdb.h"
volatile ECAQueueRegs *ECAQueue_init()
{
  return (volatile ECAQueueRegs*)ECAQueue_findAddress();
}
uint32_t *ECAQueue_findAddress()
{
#define ECAQMAX           4         //  max number of ECA queues
#define ECACHANNELFORLM32 2         //  this is a hack! suggest to implement proper sdb-records with info for queues

  uint32_t *pECAQ = 0;           // the pointer to the ECA queue

  // stuff below needed to get WB address of ECA queue 
  sdb_location ECAQ_base[ECAQMAX]; // base addresses of ECA queues
  uint32_t ECAQidx = UINT32_C(0);  // max number of ECA queues in the SoC
  uint32_t *tmp;                
  uint32_t i;

  // get Wishbone addresses of all ECA Queues
  find_device_multi(ECAQ_base, &ECAQidx, ECAQMAX, ECA_QUEUE_SDB_VENDOR_ID, ECA_QUEUE_SDB_DEVICE_ID);

  // walk through all ECA Queues and find the one for the LM32
  for (i=0; i < ECAQidx; i++) {
    tmp = (uint32_t *)(getSdbAdr(&ECAQ_base[i]));  
    if ( *(tmp + (ECA_QUEUE_QUEUE_ID_GET >> 2)) == ECACHANNELFORLM32) pECAQ = tmp;
  }

  return pECAQ;  
}
#endif 

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
	for (n = 0; ECAQueue_actionPresent(queue); ++n) 
	{
		queue->pop_owr = 0x1;
	}
	return n;
}
uint32_t ECAQueue_actionPresent(volatile ECAQueueRegs *queue)
{
  return (queue->flags_get & (1<<ECA_VALID));
}
void ECAQueue_actionPop(volatile ECAQueueRegs *queue)
{
  queue->pop_owr = 0x1;;
}

// translate some arbitrary eventIds into more interesting ones
uint64_t evtId_translator(uint64_t evtId)
{
  switch (evtId)
  {
    case 59131185602760704:
    case 56877942680062976:
      return 0x0000020008000000; // evtCode = 32 = 0x20
    break;
    case 59127337312064512:
    case 56878011399539712:
      return 0x0000022008000000; // evtCode = 0x22
    break;
    case 59131185602761728:
    case 56877873960586240:
      return 0x0000037008000000; // evtCode = 55 = 0x37
    break;
    default:
      return evtId;
    break;
  }
}

uint32_t ECAQueue_getMilEventData(volatile ECAQueueRegs *queue, uint32_t *evtCode, uint32_t *milTelegram)
{
  // EventID 
  // |---------------evtIdHi---------------|  |---------------evtIdLo---------------|
  // FFFF GGGG GGGG GGGG EEEE EEEE EEEE SSSS  SSSS SSSS BBBB BBBB BBBB BBRR RRRR RRRR
  //                          cccc cccc            vvvv
  //                              
  // F: FID(4)
  // G: GID(12)
  // E: EVTNO(12) = evtNo
  // S: SID(12)
  // B: BPID(14)
  // R: Reserved(10)
  // v: virtAcc = virtual accellerator
  // c: evtCode = MIL relevant part of the evtNo (only 0..255)
  EvtId_t evtId = { 
    .part.hi = queue->event_id_hi_get,
    .part.lo = queue->event_id_lo_get
  };  

  ////////////////////////////////////////////////////////////////////////////////////
  // for testing without the correct event ids: map some of the event to desired ones
  evtId.value = evtId_translator(evtId.value);
  ////////////////////////////////////////////////////////////////////////////////////

  uint32_t evtNo   = (evtId.part.hi>>4)  & 0x00000fff;
          *evtCode =  evtNo              & 0x000000ff;
  uint32_t virtAcc = (evtId.part.lo>>24) & 0x0000000f;

  *milTelegram = (virtAcc << 8) | *evtCode;

  // For MIL events, the upper 4 bits ov evtNo are zero
  return (evtNo & 0x00000f00) == 0; 
}
