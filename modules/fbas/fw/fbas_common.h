#ifndef _FBAS_COMMON_H_
#define _FBAS_COMMON_H_

typedef uint32_t status_t;

// timing message
#define N_MAX_TIMMSG   8 // maximum number of timing messages in an Ethernet frame

// broadcast MAC and IP addresses
#define BROADCAST_MAC 0xffffffffffff // broadcast MAC
#define BROADCAST_IP      0xffffffff // broadcast IP

// time intervals
#define TIM_1_US       1000ULL              // 1 us
#define TIM_1_MS       1000000ULL           // 1 ms
#define TIM_52_MS      TIM_1_MS * 52ULL     // 52 ms
#define TIM_100_MS     TIM_1_MS * 100ULL    // 100 ms
#define TIM_1000_MS    TIM_1_MS * 1000ULL   // 1 second
#define TIM_2000_MS    TIM_1_MS * 2000ULL   // 2 seconds

// timer prescaler
#define PSCR_1S_TIM_1MS    1000  // prescaler for 1 second (at 1ms timer period)

// MPS definitions
#define N_MAX_TX_NODES     16  // maximum number of the TX nodes

#ifdef MULTI_MPS_CH
  #define N_MPS_CHANNELS   8   // MPS channels supported by a single TX node
#else
  #define N_MPS_CHANNELS   1
#endif

#define N_MAX_MPS_CHANNELS ((N_MAX_TX_NODES) * (N_MPS_CHANNELS))   // total number of MPS channels
#define N_EXTRA_MPS_NOK    2   // extra transmissions of MPS NOK event
#define F_MPS_BCAST        30  // frequency to broadcast MPS flags [MPS_FS_530]

// MPS flags
#define MPS_FLAG_OK        1   // OK
#define MPS_FLAG_NOK       2   // NOK
#define MPS_FLAG_TEST      3   // TEST

// Ethernet MAC address
#define ETH_ALEN           6
#define ETH_ALEN_STR       18

// number of destination addresses
enum DST_ADDR {
  DST_ADDR_EBM = 0,         // current Endpoint destination address
  DST_ADDR_RXNODE,          // RX node address
  DST_ADDR_BROADCAST,       // broadcast address
  N_DST_ADDR                // total
};

// structure for an MPS protocol
typedef struct mpsProtocol mpsProtocol_t;
struct mpsProtocol {
  uint8_t  addr[ETH_ALEN];  // Ethernet MAC addr
  uint8_t  idx;             // index (0-127: MPS flag, 128-255: refer to regCmd_t)
  uint8_t  flag;            // MPS flag
};

// index field in the MPS protocol (for intern usage)
typedef enum {
  REG_REQ  = 128,       // registration request (by TX)
  REG_RSP  = 129,       // registration response (by RX)
  REG_EREQ = 192,       // extended registration request (with sender ID)
  REG_UNDEF             // undefined
} regCmd_t;

typedef struct mpsMsg mpsMsg_t;
struct mpsMsg {
  union {
    uint64_t      param;    // 'param' field of timing message
    mpsProtocol_t prot;     // MPS protocol
  };

  uint64_t tsRx;            // reception timestamp (RX)
  uint8_t  ttl;             // time-to-live (RX)
  uint8_t  pending;         // flag change indicator (RX)
};

// control structure for MPS messaging
typedef struct msgCtrl msgCtrl_t;
struct msgCtrl {
  uint8_t  total;    // total number of elements
  uint64_t last;     // timestamp of last access
  uint64_t period;   // time period between accesses
  uint8_t  ttl;      // TTL value used to evaluate validity
};

typedef enum VERBOSITY {
  DISABLE_VERBOSITY = 0,
  ENABLE_VERBOSITY
} verbosity_t;

#endif
