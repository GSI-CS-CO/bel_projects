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
#define FBAS_SHARED_GET_CNT        (FBAS_SHARED_GET_SBINUSER   + 3 * _32b_SIZE_)   // event counter
#define FBAS_SHARED_GET_TS1        (FBAS_SHARED_GET_CNT        + _32b_SIZE_)       // timestamp1 (generator event deadline)
#define FBAS_SHARED_GET_TS2        (FBAS_SHARED_GET_TS1        + _32b_SIZE_ * 2)   // timestamp2 (generator event polled by TX)
#define FBAS_SHARED_GET_TS3        (FBAS_SHARED_GET_TS2        + _32b_SIZE_ * 2)   // timestamp3 (IO action event deadline)
#define FBAS_SHARED_GET_TS4        (FBAS_SHARED_GET_TS3        + _32b_SIZE_ * 2)   // timestamp4 (IO action event polled by TX)
#define FBAS_SHARED_GET_TS5        (FBAS_SHARED_GET_TS4        + _32b_SIZE_ * 2)   // timestamp5 (measure time period)
#define FBAS_SHARED_GET_TS6        (FBAS_SHARED_GET_TS5        + _32b_SIZE_ * 2)   // timestamp6 (measure time period)
#define FBAS_SHARED_GET_AVG        (FBAS_SHARED_GET_TS6        + _32b_SIZE_ * 2)   // average (performance, delay measurements)
#define FBAS_SHARED_GET_MIN        (FBAS_SHARED_GET_AVG        + _32b_SIZE_ * 2)   // min
#define FBAS_SHARED_GET_MAX        (FBAS_SHARED_GET_MIN        + _32b_SIZE_ * 2)   // max
#define FBAS_SHARED_GET_VAL        (FBAS_SHARED_GET_MAX        + _32b_SIZE_ * 2)   // valid counts
#define FBAS_SHARED_GET_ALL        (FBAS_SHARED_GET_VAL        + _32b_SIZE_ * 2)   // all/total counts
#define FBAS_SHARED_ECA_VLD        (FBAS_SHARED_GET_ALL        + _32b_SIZE_ * 2)   // number of the valid actions output by ECA
#define FBAS_SHARED_ECA_OVF        (FBAS_SHARED_ECA_VLD        + _32b_SIZE_)       // number of the overflow actions output by ECA
#define FBAS_SHARED_GET_END        (FBAS_SHARED_ECA_OVF        + _32b_SIZE_)       // end of the 'get' region

// diagnosis: end of used shared memory
#define FBAS_SHARED_END            (FBAS_SHARED_GET_END)

// valid value for data fields in the MPS payload
#define MPS_VID_FBAS     105   // VLAN ID for FBAS

// macro defintions
// gcc.gnu.org/onlinedocs/cpp/Stringizing.html#Stringizing
#define XSTR(s) STR(s)
#define STR(s)  #s

#ifndef PLATFORM
  #define MYPLATFORM XSTR(unknown)
#endif

#ifdef PLATFORM
  #define MYPLATFORM XSTR(PLATFORM)
#endif

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
#define FBAS_CMD_SET_IO_OE      0x16   // set IO output enable
#define FBAS_CMD_GET_IO_OE      0x17   // get IO output enable
#define FBAS_CMD_TOGGLE_IO      0x18   // toggle IO output
#define FBAS_CMD_PROBE_SB_DIOB  0x20   // probe DIOB slave card on SCU bus
#define FBAS_CMD_PROBE_SB_USER  0x21   // probe a given slave (sys and group IDs are expected in shared mem @FBAS_SHARED_SET_SBSLAVES)
#define FBAS_CMD_EN_MPS_FWD     0x30   // enable MPS signal forwarding
#define FBAS_CMD_DIS_MPS_FWD    0x31   // disable MPS signal forwarding
#define FBAS_CMD_PRINT_NW_DLY   0x32   // print result of network delay measurement
#define FBAS_CMD_PRINT_OWD      0x33   // print result of one-way delay measurement
#define FBAS_CMD_PRINT_SG_LTY   0x34   // print result of MSP signalling latency measurement
#define FBAS_CMD_PRINT_TTL      0x35   // print result of TTL interval measurement

// mask bit for MPS-relevant tasks (up to 31)
#define TSK_TX_MPS_FLAGS        0x10000000 // transmit MPS flags
#define TSK_TX_MPS_EVENTS       0x20000000 // transmit MPS events
#define TSK_TTL_MPS_FLAGS       0x40000000 // monitor lifetime of MPS flags

// FBAS timing messages
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

#endif
