#ifndef WR_MIL_ECA_QUEUE_H_
#define WR_MIL_ECA_QUEUE_H_

#include "wr_mil_value64bit.h"

volatile uint32_t *ECAQueue_init();

// locate the ECAQueue via SDB and return a pointer to the channel for the soft CPU
uint32_t *ECAQueue_findAddress();

void ECAQueue_popAction(volatile uint32_t *queue);

void ECAQueue_getDeadl(volatile uint32_t *queue, TAI_t *deadl);

void ECAQueue_getEvtId(volatile uint32_t *queue, EvtId_t *evtId);

uint32_t ECAQueue_getActTag(volatile uint32_t *queue);

uint32_t ECAQueue_getFlags(volatile uint32_t *queue);

// remove all events from the ECA queue and return the number of removed events
uint32_t ECAQueue_clear(volatile uint32_t *queue);

// returns 1 if there is at least one event in the ECA queue
uint32_t ECAQueue_actionPresent(volatile uint32_t *queue);

// remove single event from the ECA queue
void ECAQueue_actionPop(volatile uint32_t *queue);

// Extract event number, event code and virtual accelerator number from the EventId in the ECA queue. 
// parameters: 
//    queue:   pointer to ECA queue registers as obtained from ECAQueue_init()
//    evtCode: the event code of the MIL event (this is only valid if function returns 1)
//    milTelegram: correcly formatted MIL telegram that can be passed to the mil piggy 
//
// return value: is nonzero if the event code must be forwarded to MIL event bus, zero otherwise
uint32_t ECAQueue_getMilEventData(volatile uint32_t *queue, uint32_t *evtCode, uint32_t *milTelegram);

#endif
