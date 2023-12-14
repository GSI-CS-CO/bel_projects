#ifndef _BROADCAST_H_
#define _BROADCAST_H_

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "aux.h"
#include "ebm.h"
#include "dbg.h"
#include "common-fwlib.h"
#include "common-defs.h"
#include "fbas_common.h"

// FBAS timing messages
#define FBAS_FLG_FID       0x1ULL    // format ID, 2-bit
#define FBAS_FLG_GID       0xfcbULL  // group ID = 4043, 12-bit
#define FBAS_FLG_EVTNO     0xfcbULL  // event number = 4043, 12-bit
#define FBAS_FLG_FLAGS     0x0ULL    // flags, 4-bit
#define FBAS_FLG_SID       0x0ULL    // sequence ID, 12-bit
#define FBAS_FLG_BPID      0x0ULL    // beam process ID, 14-bit
#define FBAS_FLG_RES       0x0ULL    // reserved, 6-bit

#define FBAS_EVT_FID       0x1ULL    // format ID, 2-bit
#define FBAS_EVT_GID       0xfccULL  // group ID = 4044, 12-bit
#define FBAS_EVT_EVTNO     0xfccULL  // event number = 4044, 12-bit
#define FBAS_EVT_FLAGS     0x0ULL    // flags, 4-bit
#define FBAS_EVT_SID       0x0ULL    // sequence ID, 12-bit
#define FBAS_EVT_BPID      0x0ULL    // beam process ID, 14-bit
#define FBAS_EVT_RES       0x0ULL    // reserved, 6-bit

#define FBAS_REG_FID       0x1ULL    // format ID, 2-bit
#define FBAS_REG_GID       0xfcdULL  // group ID = 4045, 12-bit
#define FBAS_REG_EVTNO     0xfcdULL  // event number = 4045, 12-bit
#define FBAS_REG_FLAGS     0x0ULL    // flags, 4-bit
#define FBAS_REG_SID       0x0ULL    // sequence ID, 12-bit
#define FBAS_REG_BPID      0x0ULL    // beam process ID, 14-bit
#define FBAS_REG_RES       0x0ULL    // reserved, 6-bit

enum FBAS_EIDS {
  FBAS_FLG_EID = (((FBAS_FLG_FID) << (60)) | ((FBAS_FLG_GID) << (48)) | \
                  ((FBAS_FLG_EVTNO) << (36)) | ((FBAS_FLG_FLAGS) << (32)) | \
                  ((FBAS_FLG_SID) << (20)) | ((FBAS_FLG_BPID) << (6)) | \
                  (FBAS_FLG_RES)),

  FBAS_EVT_EID = (((FBAS_EVT_FID) << (60)) | ((FBAS_EVT_GID) << (48)) | \
                  ((FBAS_EVT_EVTNO) << (36)) | ((FBAS_EVT_FLAGS) << (32)) | \
                  ((FBAS_EVT_SID) << (20)) | ((FBAS_EVT_BPID) << (6)) | \
                  (FBAS_EVT_RES)),

  FBAS_REG_EID = (((FBAS_REG_FID) << (60)) | ((FBAS_REG_GID) << (48)) | \
                  ((FBAS_REG_EVTNO) << (36)) | ((FBAS_REG_FLAGS) << (32)) | \
                  ((FBAS_REG_SID) << (20)) | ((FBAS_REG_BPID) << (6)) | \
                   (FBAS_REG_RES))

} fbas_eid_t;

// a pair of MAC and IP addresses as network address
typedef struct nw_addr nw_addr_t;
struct nw_addr {
  uint64_t mac;
  uint32_t ip;
};

extern uint64_t myMac;                             // own MAC address
extern mpsMsg_t bufMpsMsg[N_MPS_CHANNELS];         // buffer for MPS messages
extern timedItr_t rdItr;                           // read-access iterator for MPS flags

void initItr(timedItr_t* itr, uint8_t total, uint64_t now, uint32_t freq);
void resetItr(timedItr_t* itr, uint64_t now);
uint32_t sendMpsMsgPeriodic(timedItr_t* itr, uint64_t evtid);
uint32_t sendMpsMsgSpecific(timedItr_t* itr, mpsMsg_t* buf, uint64_t evtid, uint8_t extra);
uint32_t sendMpsMsgBlock(size_t len, timedItr_t* itr, uint64_t evtId);
mpsMsg_t* updateMpsMsg(mpsMsg_t* buf, uint64_t evt);
status_t storeMpsMsg(uint64_t raw, uint64_t ts, timedItr_t* itr, int* offset);
mpsMsg_t* evalMpsMsgTtl(uint64_t now, int idx);
void msgInitMpsMsgBuf();
void resetMpsMsg(size_t len, mpsMsg_t* buf);
void setMpsMsgSenderId(mpsMsg_t* msg, uint64_t raw, uint8_t verbose);

status_t sendRegReq(int req);
status_t sendRegRsp(void);
bool isSenderKnown(uint64_t raw);

void diagPrintMpsMsgBuf(void);

#endif
