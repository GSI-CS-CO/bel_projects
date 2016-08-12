#ifndef _ACCESS_H_
#define _ACCESS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <etherbone.h>
#include "allocator.h"


extern eb_device_t device;
extern eb_socket_t mySocket;
extern uint8_t show_time;

#define SWAP_4(x) ( ((x) << 24) | \
         (((x) << 8) & 0x00ff0000) | \
         (((x) >> 8) & 0x0000ff00) | \
         ((x) >> 24) )

#define MAX_DEVICES 20
#define FWID_LEN 0x400
#define BOOTL_LEN 0x100
#define PACKET_SIZE  500
#define CMD_LM32_RST 0x2

#define ACTIVE    1
#define INACTIVE  2

//#if MOCKUP==1
#define CPU_MAX 8
#define THR_MAX 8
//#endif

//Status report fields
//TODO not all of em meaningful yet

//offsets (in 32b words)
//THESE ARE !!!NOT!!! register addresses, but the word offset in the report buffer 
#define OFFSET_EBM        0
#define EBM_STATUS        (OFFSET_EBM + 2)
#define EBM_SRC_MAC_HI    (EBM_STATUS       +1)       
#define EBM_SRC_MAC_LO    (EBM_SRC_MAC_HI   +1)    
#define EBM_SRC_IPV4      (EBM_SRC_MAC_LO   +1)    
#define EBM_SRC_UDP_PORT  (EBM_SRC_IPV4     +1)   
#define EBM_DST_MAC_HI    (EBM_SRC_UDP_PORT +1)  
#define EBM_DST_MAC_LO    (EBM_DST_MAC_HI   +1)   
#define EBM_DST_IPV4      (EBM_DST_MAC_LO   +1)  
#define EBM_DST_UDP_PORT  (EBM_DST_IPV4     +1)   
#define EBM_MTU           (EBM_DST_UDP_PORT +1)  
#define EBM_ADR_HI        (EBM_MTU          +1)    
#define EBM_OPS_MAX       (EBM_ADR_HI       +1)   

#define BROADCAST_MAC     0xffffffffffffULL
#define BROADCAST_IP      0xffffffff
#define EB_PORT           0xEBD0

#define OFFSET_PRIOQ      14
#define PRIOQ_CFG         (5)
#define PRIOQ_DST_ADR     (8)
#define PRIOQ_MSG_CNTO    (10)
#define PRIOQ_MSG_CNTI    (PRIOQ_MSG_CNTO + 1)
#define PRIOQ_TTRN_HI     (PRIOQ_MSG_CNTI + 1)
#define PRIOQ_TTRN_LO     (PRIOQ_TTRN_HI  + 1)
#define PRIOQ_TDUE_HI     (PRIOQ_TTRN_LO  + 1)
#define PRIOQ_TDUE_LO     (PRIOQ_TDUE_HI  + 1)
#define PRIOQ_CAPACITY    (PRIOQ_TDUE_LO  + 1)
#define PRIOQ_MSG_PAC     (PRIOQ_CAPACITY + 1)  

#define VALID_PRIOQ_CFG   0x7
#define ECA_ADDRESS       0x7ffffff0
#define MAX_MSG_PER_PACKET 36 

#define OFFSET_WR       34
#define WR_STATUS        0
#define WR_UTC_LO       (WR_STATUS        + 1)
#define WR_UTC_HI       (WR_UTC_LO        + 1)
#define WR_STATE_SIZE   (WR_UTC_HI        + 1) //number 32b field for wr status

#define VALID_WR_STATE  0x6

#define OFFSET_CPUS     37
#define CPU_STATUS      0
#define CPU_MSGS        (CPU_STATUS       + 1)
#define CPU_SHARED      (CPU_MSGS         + 1)
#define CPU_TPREP_HI    (CPU_SHARED       + 1)
#define CPU_TPREP_LO    (CPU_TPREP_HI     + 1)

#define CPU_THR_RUNNING (CPU_TPREP_LO     + 1)
#define CPU_THR_IDLE    (CPU_THR_RUNNING  + 1)
#define CPU_THR_WAITING (CPU_THR_IDLE     + 1)
#define CPU_THR_ERROR   (CPU_THR_WAITING  + 1)
#define CPU_THR_ACT_A   (CPU_THR_ERROR    + 1)
#define CPU_THR_ACT_B   (CPU_THR_ACT_A    + 1)
#define CPU_THR_RDY_A   (CPU_THR_ACT_B    + 1)
#define CPU_THR_RDY_B   (CPU_THR_RDY_A    + 1)

#define CPU_STATE_SIZE  (CPU_THR_RDY_B    + 1) //number 32b field for cpu status

#define THR_STATUS      0
#define THR_MSGS        (THR_STATUS       + 1)
#define THR_STATE_SIZE  (THR_MSGS         + 1) //number 32b field for thread status

#ifdef __cplusplus
  extern "C" {
#endif









extern const char* program;






typedef struct {
   uint32_t ramAdr;
   uint32_t mask;
   uint8_t  hasValidFW;
   uint32_t lbt[_LBT_SIZE_>>2];
} t_core;

typedef struct {
   uint8_t cpuQty;
   uint8_t thrQty;
   uint32_t validCpus;
   t_core* pCores;
   uint32_t resetAdr;
   uint32_t clusterAdr;
   uint32_t sharedAdr;
   uint32_t prioQAdr;
   uint32_t ebmAdr;
   uint32_t sysConAdr;
   uint32_t ppsAdr;
} t_ftmAccess;





typedef struct {
  uint32_t state;
  uint64_t tPrep;
  uint32_t msgCnt;
  uint32_t errCnt;
  uint32_t thrRun;
  uint32_t thrWait;
  uint32_t thrIdle;
  uint32_t thrActAB;
  uint32_t thrActP[THR_MAX];
  uint32_t thrError;
  //add support for memory table
} t_statusCore;

typedef struct {
  uint8_t  mode;
  uint32_t maxMsg;
  uint32_t maxWait;
  uint32_t adEbm;
  uint32_t adEca;
  uint64_t msgCnt;
  uint64_t lateCnt;
} t_statusPq;

typedef struct {
  uint64_t mac;
  uint32_t ipv4;
  uint16_t port;
} t_nwAddr;

typedef struct {
  uint32_t state;
  t_nwAddr src;
  t_nwAddr dst;
  uint16_t mtu;
  uint32_t adhi;
  uint32_t opt;
} t_statusEbm;

typedef struct {
  uint8_t state;
  uint64_t tsWr;
  uint64_t tsEca;
} t_statusWr;

typedef struct {
   uint8_t cpuQty;
   uint8_t thrQty;
   uint32_t validCpus;
   t_statusCore  sCore[CPU_MAX];
   t_statusWr    sWr;
   t_statusEbm   sEbm;
   t_statusPq    sPq;
} t_status;

extern uint32_t ftm_shared_offs;
extern t_ftmAccess* p;

uint64_t cpus2thrs(uint32_t cpus);
uint32_t thrs2cpus(uint64_t thrs);

int ftmOpen(const char* netaddress, uint8_t overrideFWcheck); //returns bitField showing CPUs with valid DM firmware
int ftmClose(void);

//per DM
int ftmRst(void);
int ftmSetDuetime(uint64_t tdue);
int ftmSetTrntime(uint64_t ttrn);
int ftmSetMaxMsgs(uint64_t maxmsg);

//per CPU
int ftmCpuRst(uint32_t dstCpus);
int ftmFwLoad(uint32_t dstCpus, const char* filename);
int ftmSetPreptime(uint32_t dstCpus, uint64_t tprep);
int ftmGetStatus(uint32_t srcCpus, uint32_t* buff);
void ftmShowStatus(uint32_t srcCpus, uint32_t* status, uint8_t verbose);
void ftmShowTable(uint32_t srcCpus, uint8_t verbose);

//per thread
int ftmThrRst(uint64_t dstBitField);
//these need wrappers to per thread access for future version compatibility
#if MOCKUP==0
int v02FtmCommand(uint32_t dstCpus, uint32_t command);
int v02FtmPutString(uint32_t dstCpus, const char* sXml);
int v02FtmPutFile(uint32_t dstCpus, const char* filename);
int v02FtmCheckString(const char* sXml);
int v02FtmClear(uint32_t dstCpus, int32_t tabIdx);
int v02FtmDump(uint32_t srcCpus, int32_t tabIdx, char* stringBuf, uint32_t lenStringBuf);
int v02FtmSetBp(uint32_t dstCpus, int32_t planIdx);
int v02FtmFetchStatus(uint32_t* buff, uint32_t len);
int v04ftmSetThread(uint32_t cpuIdx, uint32_t thrIdx, int32_t tabIdx);

#endif

int ftmCommand(uint64_t dstThr, uint32_t command);
int ftmSignal(uint64_t dstThr, uint32_t offset, uint64_t value, uint64_t mask);
int ftmPutString(uint64_t dstThr, const char* sXml);
int ftmCheckString(const char* sXml);
int ftmPutFile(uint64_t dstThr, const char* filename);
int ftmClear(uint64_t dstThr, int32_t tabIdx);
int ftmDump(uint64_t srcThr, int32_t tabIdx, char* stringBuf, uint32_t lenStringBuf);
int ftmSetBp(uint64_t dstThr, int32_t planIdx);
int ftmFetchStatus(uint32_t* buff, uint32_t len);
int ftmSetThread(uint64_t dstThr, int32_t tabIdx);

#ifdef __cplusplus
}
#endif


#endif
