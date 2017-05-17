#ifndef WR_MIL_ECA_QUEUE_H_
#define WR_MIL_ECA_QUEUE_H_

#include "wr_mil_value64bit.h"

/* stucture and functions to read events from the eca queue */

typedef struct 
{
	uint32_t queue_id_get;
	uint32_t pop_owr;
	uint32_t flags_get;
	uint32_t num_get;
	uint32_t event_id_hi_get;
	uint32_t event_id_lo_get;
	uint32_t param_hi_get;
	uint32_t param_lo_get;
	uint32_t tag_get;
	uint32_t tef_get;
	uint32_t deadline_hi_get;
	uint32_t deadline_lo_get;
	uint32_t executed_hi_get;
	uint32_t executed_lo_get;
} ECAQueueRegs;

volatile ECAQueueRegs *ECAQueue_init(uint32_t *device_addr);

// locate the ECAQueue via SDB and return a pointer to the channel for the soft CPU
uint32_t *ECAQueue_findAddress();

void ECAQueue_popAction(volatile ECAQueueRegs *queue);

void ECAQueue_getDeadl(volatile ECAQueueRegs *queue, TAI_t *deadl);

void ECAQueue_getEvtId(volatile ECAQueueRegs *queue, EvtId_t *evtId);

uint32_t ECAQueue_getActTag(volatile ECAQueueRegs *queue);

// remove all events from the ECA queue and return the number of removed events
uint32_t ECAQueue_clear(volatile ECAQueueRegs *queue);

uint32_t ECAQueue_getFlags(volatile ECAQueueRegs *queue);

void ECAQueue_getMilEventData(volatile ECAQueueRegs *queue, uint32_t *evtNo, 
                                                            uint32_t *evtCode, 
                                                            uint32_t *virtAcc);

#endif
