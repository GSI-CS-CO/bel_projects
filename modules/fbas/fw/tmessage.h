#ifndef _BROADCAST_H_
#define _BROADCAST_H_

#include <stddef.h>
#include <stdint.h>

#include "aux.h"
#include "common-fwlib.h"
#include "fbas_common.h"

extern mpsTimParam_t bufMpsFlag[N_MPS_CHANNELS];   // buffer for MPS flags
extern timedItr_t rdItr;                           // read-access iterator for MPS flags

void initItr(timedItr_t* itr, uint8_t total, uint64_t now, uint32_t freq);
void resetItr(timedItr_t* itr, uint64_t now);
void sendMpsFlag(timedItr_t* itr, uint64_t evtid);
void sendMpsEvent(timedItr_t* itr, mpsTimParam_t* buf, uint64_t evtid, uint8_t extra);
mpsTimParam_t* updateMpsFlag(mpsTimParam_t* buf, uint64_t evt);
mpsTimParam_t* storeMpsFlag(mpsTimParam_t* buf, uint64_t raw);
mpsTimParam_t* expireMpsFlag(timedItr_t* itr);
void resetMpsFlag(size_t len, mpsTimParam_t* buf);

#endif
