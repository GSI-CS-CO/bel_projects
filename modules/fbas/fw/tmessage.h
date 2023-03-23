#ifndef _BROADCAST_H_
#define _BROADCAST_H_

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "aux.h"
#include "ebm.h"
#include "dbg.h"
#include "common-fwlib.h"
#include "common-defs.h"
#include "fbas_common.h"

extern mpsMsg_t bufMpsMsg[N_MPS_CHANNELS];         // buffer for MPS messages
extern timedItr_t rdItr;                           // read-access iterator for MPS flags

void initItr(timedItr_t* itr, uint8_t total, uint64_t now, uint32_t freq);
void resetItr(timedItr_t* itr, uint64_t now);
status_t sendMpsMsgPeriodic(timedItr_t* itr, uint64_t evtid);
status_t sendMpsMsgSpecific(timedItr_t* itr, mpsMsg_t* buf, uint64_t evtid, uint8_t extra);
status_t sendMpsMsgBlock(size_t len, timedItr_t* itr, uint64_t evtId);
mpsMsg_t* updateMpsMsg(mpsMsg_t* buf, uint64_t evt);
mpsMsg_t* storeMpsMsg(uint64_t raw, uint64_t ts, timedItr_t* itr);
mpsMsg_t* evalMpsMsgTtl(uint64_t now, int idx);
void resetMpsMsg(size_t len, mpsMsg_t* buf);
void setMpsMsgSenderId(mpsMsg_t* msg, uint64_t raw, uint8_t verbose);

status_t buildRegReq(mpsMsg_t* buf, int len, int req);

int addr_equal(uint8_t a[ETH_ALEN], uint8_t b[ETH_ALEN]); // wr-switch-sw/userspace/libwr
uint8_t *addr_copy(uint8_t dst[ETH_ALEN], uint8_t src[ETH_ALEN]);

#endif
