#ifndef _FBAS_H_
#define _FBAS_H_

// ****************************************************************************************
// DP RAM layout (offsets)
// ****************************************************************************************

// set values for data supply
#define FBAS_SHARED_SET_GID        (COMMON_SHARED_END          + _32b_SIZE_)       // GID of B2B Transfer ('EXTRING_B2B_...')
#define FBAS_SHARED_SET_SID        (FBAS_SHARED_SET_GID        + _32b_SIZE_)       // sequence ID for B2B transfer
#define FBAS_SHARED_SET_NODETYPE   (FBAS_SHARED_SET_SID        + _32b_SIZE_)       // FBAS node type
#define FBAS_SHARED_SET_SBSLAVES   (FBAS_SHARED_SET_NODETYPE   + _32b_SIZE_)       // SCU bus slaves (bit1=slot1, 1-12)
//get values
#define FBAS_SHARED_GET_GID        (FBAS_SHARED_SET_SBSLAVES   + _32b_SIZE_)       // GID of B2B Transfer ('EXTRING_B2B_...')
#define FBAS_SHARED_GET_SID        (FBAS_SHARED_GET_GID        + _32b_SIZE_)       // sequence ID for B2B transfer
#define FBAS_SHARED_GET_NODETYPE   (FBAS_SHARED_GET_SID        + _32b_SIZE_)       // FBAS node type
#define FBAS_SHARED_GET_SBSLAVES   (FBAS_SHARED_GET_NODETYPE   + _32b_SIZE_)       // SCU bus slaves (bit1=slot1, 1-12)
#define FBAS_SHARED_GET_SBSTDBEGIN (FBAS_SHARED_GET_SBSLAVES   + _32b_SIZE_)       // begin of the standard register set
#define FBAS_SHARED_GET_SBSTDEND   (FBAS_SHARED_GET_SBSTDBEGIN + 48 * _32b_SIZE_)  // end of the standard register set
#define FBAS_SHARED_GET_SBCFGDIOB  (FBAS_SHARED_GET_SBSTDEND   + _32b_SIZE_)       // DIOB configuration
#define FBAS_SHARED_GET_SBSTSDIOB  (FBAS_SHARED_GET_SBCFGDIOB  + 2 * _32b_SIZE_)   // DIOB status
#define FBAS_SHARED_GET_SBCFGUSER  (FBAS_SHARED_GET_SBSTSDIOB  + 2 * _32b_SIZE_)   // USER configuration
#define FBAS_SHARED_GET_SBSTSUSER  (FBAS_SHARED_GET_SBCFGUSER  + 2 * _32b_SIZE_)   // USER status
#define FBAS_SHARED_GET_SBOUTUSER  (FBAS_SHARED_GET_SBSTSUSER  + 2 * _32b_SIZE_)   // USER output
#define FBAS_SHARED_GET_SBINUSER   (FBAS_SHARED_GET_SBOUTUSER  + 3 * _32b_SIZE_)   // USER input
#define FBAS_SHARED_GET_TS1        (FBAS_SHARED_GET_SBINUSER   + _32b_SIZE_)       // timestamp1 (generator event deadline)
#define FBAS_SHARED_GET_TS2        (FBAS_SHARED_GET_TS1        + _32b_SIZE_ * 2)   // timestamp2 (generator event polled by TX)
#define FBAS_SHARED_GET_TS3        (FBAS_SHARED_GET_TS2        + _32b_SIZE_ * 2)   // timestamp3 (IO action event deadline)
#define FBAS_SHARED_GET_TS4        (FBAS_SHARED_GET_TS3        + _32b_SIZE_ * 2)   // timestamp4 (IO action event polled by TX)
#define FBAS_SHARED_GET_TS5        (FBAS_SHARED_GET_TS4        + _32b_SIZE_ * 2)   // timestamp5 (measure time period)
#define FBAS_SHARED_GET_TS6        (FBAS_SHARED_GET_TS5        + _32b_SIZE_ * 2)   // timestamp6 (measure time period)
#define FBAS_SHARED_GET_END        (FBAS_SHARED_GET_TS6        + _32b_SIZE_ * 2)   // end of the 'get' region

// diagnosis: end of used shared memory
#define FBAS_SHARED_END            (FBAS_SHARED_GET_END)

typedef uint32_t status_t;

typedef struct {
  uint64_t evtId;    // event ID
  uint64_t mac;      // MAC address (prepended with zeros)
} mpsEventData_t;

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

#define MPS_PAYLOAD_SIZE sizeof(mpsEventData_t)/sizeof(uint32_t)
#define N_MPS_CHANNELS       32  // total number of MPS channels
#define N_EXTRA_MPS_EVENTS   2   // number of MPS event transmissions

// valid value for data fields in the MPS payload
#define MPS_VID_FBAS     105   // VLAN ID for FBAS

// node type
typedef enum {
  FBAS_NODE_TX = 0,   // FBAS transmitter
  FBAS_NODE_RX,       // FBAS receiver
  FBAS_NODE_CM,       // FBAS common
  FBAS_NODE_UNDEF     // undefined
} nodeType_t;

// operation mode
typedef enum {
  FBAS_OPMODE_DEF = 0, // default mode
  FBAS_OPMODE_TEST,    // test mode
  FBAS_OPMODE_INVALID  // invalid
} opMode_t;

// application-specific commands
#define FBAS_CMD_SET_NODETYPE   0x15   // set the node type
#define FBAS_CMD_SET_LVDS_OE    0x16   // set LVDS output enable
#define FBAS_CMD_GET_LVDS_OE    0x17   // get LVDS output enable
#define FBAS_CMD_TOGGLE_LVDS    0x18   // toggle LVDS output
#define FBAS_CMD_PROBE_SB_DIOB  0x20   // probe DIOB slave card on SCU bus
#define FBAS_CMD_PROBE_SB_USER  0x21   // probe a given slave (sys and group IDs are expected in shared mem @FBAS_SHARED_SET_SBSLAVES)
#define FBAS_CMD_EN_MPS_FWD     0x30   // enable MPS signal forwarding
#define FBAS_CMD_DIS_MPS_FWD    0x31   // disable MPS signal forwarding

// mask bit for MPS-relevant tasks (up to 31)
#define TSK_TX_MPS_FLAGS        0x10000000 // transmit MPS flags
#define TSK_TX_MPS_EVENTS       0x20000000 // transmit MPS events
#define TSK_TTL_MPS_FLAGS       0x40000000 // monitor lifetime of MPS flags

// flags
#define MPS_FLAG_OK        1   // OK
#define MPS_FLAG_NOK       2   // NOK
#define MPS_FLAG_TEST      3   // TEST

// LEMO signals (signed int)
enum {
  MPS_SIGNAL_LOW = 0,   // logical '0'
  MPS_SIGNAL_HIGH,      // logical '1'
  MPS_SIGNAL_INVALID    // invalid
};

// time interval
#define WR_TIM_1_MS       1000000ULL         // 1 ms
#define WR_TIM_52_MS      5200 * WR_TIM_1_MS   // 52 ms
#define WR_TIM_1000_MS    1000 * WR_TIM_1_MS // 1 second
#define TIM_2000_MS     2000   // 2000 ms
#define TIM_10_SEC    100000000   // 1 second

// MAC/IP addresses
#define BROADCAST_MAC 0xffffffffffff // broadcast MAC
#define BROADCAST_IP      0xffffffff // broadcast IP

// FBAS timing messages
#define FBAS_TM_FID        0x1       // format ID, 2-bit
#define FBAS_TM_GID        0xfcaUL   // group ID = 4042, 12-bit
#define FBAS_TM_EVTNO      0xfcaUL   // event number = 4042, 12-bit
#define FBAS_TM_FLAGS      0x0       // flags, 4-bit
#define FBAS_TM_SID        0x0       // sequence ID, 12-bit
#define FBAS_TM_BPID       0x0       // beam process ID, 14-bit
#define FBAS_TM_RES        0x0       // reserved, 6-bit

#define FBAS_FLG_FID       0x1       // format ID, 2-bit
#define FBAS_FLG_GID       0xfcbUL   // group ID = 4043, 12-bit
#define FBAS_FLG_EVTNO     0xfcbUL   // event number = 4043, 12-bit
#define FBAS_FLG_FLAGS     0x0       // flags, 4-bit
#define FBAS_FLG_SID       0x0       // sequence ID, 12-bit
#define FBAS_FLG_BPID      0x0       // beam process ID, 14-bit
#define FBAS_FLG_RES       0x0       // reserved, 6-bit

#define FBAS_EVT_FID       0x1       // format ID, 2-bit
#define FBAS_EVT_GID       0xfccUL   // group ID = 4044, 12-bit
#define FBAS_EVT_EVTNO     0xfccUL   // event number = 4044, 12-bit
#define FBAS_EVT_FLAGS     0x0       // flags, 4-bit
#define FBAS_EVT_SID       0x0       // sequence ID, 12-bit
#define FBAS_EVT_BPID      0x0       // beam process ID, 14-bit
#define FBAS_EVT_RES       0x0       // reserved, 6-bit

// ECA action tags
#define FBAS_GEN_EVT       0x42      // ECA condition tag for generator event (handled by TX)
#define FBAS_TLU_EVT       0x43      // ECA condition tag for TLU event (handled by TX)
#define FBAS_WR_EVT        0x24      // ECA condition tag for MPS event via WR (handled by RX)
#define FBAS_WR_FLG        0x25      // ECA condition tag for MPS flag via WR (handled by RX)
#define FBAS_AUX_NEWCYCLE  0x26      // ECA condition tag for MPS auxiliary signal (clear internal errors in TX & RX)
#define FBAS_AUX_OPMODE    0x27      // ECA condition tag for MPS auxiliary signal (specify operation mode for TX & RX)

// IO-CTRL register map (ip_cores/saftlib/drivers/io_control_regs.h)
#define IO_CFG_CHANNEL_GPIO          0
#define IO_CFG_CHANNEL_LVDS          1
#define IO_CFG_CHANNEL_FIXED         2

#define IO_GPIO_OE_LEGACYLOW  0x0000
#define IO_GPIO_OE_LEGACYHIGH 0x0008
#define IO_CONFIG             0x0010
#define IO_VERSION            0x0100
#define IO_GPIO_INFO          0x0104
#define IO_GPIO_OE_SETLOW     0x0200
#define IO_GPIO_OE_SETHIGH    0x0204
#define IO_GPIO_OE_RESETLOW   0x0208
#define IO_GPIO_OE_RESETHIGH  0x020c
#define IO_LVDS_OE_SETLOW     0x0300
#define IO_LVDS_OE_SETHIGH    0x0304
#define IO_LVDS_OE_RESETLOW   0x0308
#define IO_LVDS_OE_RESETHIGH  0x030c
#define IO_GPIO_SET_OUTBEGIN  0xa000
#define IO_LVDS_SET_OUTBEGIN  0xb000
#define IO_GPIO_GET_INBEGIN   0xc000
#define IO_LVDS_GET_INBEGIN   0xd000

#endif
