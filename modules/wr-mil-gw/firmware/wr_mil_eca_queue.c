#include "wr_mil_eca_queue.h"

#include "../../ip_cores/saftlib/drivers/eca_flags.h"
#include "../../ip_cores/wr-cores/modules/wr_eca/eca_queue_regs.h"
#include "../../ip_cores/wr-cores/modules/wr_eca/eca_regs.h"       // register layout ECA control
#include "mini_sdb.h"

volatile uint32_t *ECAQueue_init()
{
  return (volatile uint32_t*)ECAQueue_findAddress();
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
    if ( tmp[ECA_QUEUE_QUEUE_ID_GET/4] == ECACHANNELFORLM32) pECAQ = tmp;
  }

  return pECAQ;  
}

void ECAQueue_popAction(volatile uint32_t *queue)
{
  queue[ECA_QUEUE_POP_OWR/4] = 0x1;
}

void ECAQueue_getDeadl(volatile uint32_t *queue, TAI_t *deadl)
{
  deadl->part.hi = queue[ECA_QUEUE_DEADLINE_HI_GET/4];
  deadl->part.lo = queue[ECA_QUEUE_DEADLINE_LO_GET/4];

}

void ECAQueue_getEvtId(volatile uint32_t *queue, EvtId_t *evtId)
{
  evtId->part.hi = queue[ECA_QUEUE_EVENT_ID_HI_GET/4];
  evtId->part.lo = queue[ECA_QUEUE_EVENT_ID_LO_GET/4];
}

uint32_t ECAQueue_getActTag(volatile uint32_t *queue)
{
  return queue[ECA_QUEUE_TAG_GET/4];
}

uint32_t ECAQueue_getFlags(volatile uint32_t *queue)
{
  return queue[ECA_QUEUE_FLAGS_GET/4];
}

uint32_t ECAQueue_clear(volatile uint32_t *queue)
{
	uint32_t n;
	for (n = 0; ECAQueue_actionPresent(queue); ++n) 
	{
    queue[ECA_QUEUE_POP_OWR/4] = 0x1;
	}
	return n;
}

uint32_t ECAQueue_actionPresent(volatile uint32_t *queue)
{
  return queue[ECA_QUEUE_FLAGS_GET/4] & (1<<ECA_VALID);
}

void ECAQueue_actionPop(volatile uint32_t *queue)
{
  queue[ECA_QUEUE_POP_OWR/4] = 0x1;
}

uint32_t ECAQueue_getMilEventData(volatile uint32_t *queue, uint32_t *evtCode, uint32_t *milTelegram)
{
  // EventID 
  // |---------------evtIdHi---------------|  |---------------evtIdLo---------------|
  // FFFF GGGG GGGG GGGG EEEE EEEE EEEE SSSS  SSSS SSSS BBBB BBBB BBBB BBRR RRRR RRRR
  //                          cccc cccc       ssss vvvv
  //                                          xxxx xxxx
  //                              
  // F: FID(4)
  // G: GID(12)
  // E: EVTNO(12) = evtNo
  // S: SID(12)
  // B: BPID(14)
  // R: Reserved(10)
  // s: status bits
  // v: virtAcc = virtual accellerator
  // c: evtCode = MIL relevant part of the evtNo (only 0..255)
  // x: other content for special events like (code=200..208 command events)
  //    or (code=200 beam status event) 
  EvtId_t evtId = { 
    .part.hi = queue[ECA_QUEUE_EVENT_ID_HI_GET/4],
    .part.lo = queue[ECA_QUEUE_EVENT_ID_LO_GET/4]
  };  


  uint32_t evtNo   = (evtId.part.hi>>4)  & 0x00000fff;
          *evtCode =  evtNo              & 0x000000ff;
  uint32_t tophalf = (evtId.part.lo>>24) & 0x000000ff; // the top half bits (15..8) of the mil telegram.
                                                       // the meaning depends on the evtCode.

  *milTelegram = (tophalf << 8) | *evtCode;

  // For MIL events, the upper 4 bits ov evtNo are zero
  return (evtNo & 0x00000f00) == 0; 
}
