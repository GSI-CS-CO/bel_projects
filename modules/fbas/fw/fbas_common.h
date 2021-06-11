#ifndef _FBAS_COMMON_H_
#define _FBAS_COMMON_H_

typedef uint32_t status_t;

// broadcast MAC and IP addresses
#define BROADCAST_MAC 0xffffffffffff // broadcast MAC
#define BROADCAST_IP      0xffffffff // broadcast IP

// time intervals
#define TIM_1_US       1000ULL              // 1 us
#define TIM_1_MS       1000000ULL           // 1 ms
#define TIM_52_MS      TIM_1_MS * 52ULL     // 52 ms
#define TIM_1000_MS    TIM_1_MS * 1000ULL   // 1 second
#define TIM_2000_MS    TIM_1_MS * 2000ULL   // 2 seconds

// timer prescaler
#define PSCR_1S_TIM_1MS    1000  // prescaler for 1 second (at 1ms timer period)

// MPS definitions
#define N_MPS_CHANNELS     32  // total number of MPS channels
#define N_EXTRA_MPS_NOK    2   // extra transmissions of MPS NOK event
#define F_MPS_BCAST        30  // frequency to broadcast MPS flags [MPS_FS_530]

// MPS flags
#define MPS_FLAG_OK        1   // OK
#define MPS_FLAG_NOK       2   // NOK
#define MPS_FLAG_TEST      3   // TEST

// structure for an MPS protocol
typedef struct mpsProt mpsProt_t;
struct mpsProt {
  uint8_t  flag;     // flag (FBAS signal state)
  uint8_t  grpId;    // group ID
  uint16_t evtId;    // event ID
  uint8_t  ttl;      // time-to-live (RX)
  uint8_t  pending;  // pending is set if flag is changed
};

// MPS protocol as parameter field in timing message
typedef union mpsTimParam mpsTimParam_t;
union mpsTimParam {
  mpsProt_t prot;    // MPS protocol data
  uint64_t param;    // parameter field in timing message
};

// iterator used to access available MPS flags
typedef struct timedItr timedItr_t;
struct timedItr {
  uint8_t idx;       // index of current element
  uint8_t total;     // total number of elements
  uint64_t last;     // timestamp of last access
  uint64_t period;   // time period between accesses
};

#endif
