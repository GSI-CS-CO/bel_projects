#ifndef _FBAS_H_
#define _FBAS_H_

// ****************************************************************************************
// DP RAM layout (offsets)
// ****************************************************************************************

// set values for data supply
#define FBAS_SHARED_BEGIN          (COMMON_SHARED_END          + _32b_SIZE_)       // begin of the app-spec region
#define FBAS_SHARED_SET_ID         (FBAS_SHARED_BEGIN          + _32b_SIZE_)       // reserved
#define FBAS_SHARED_SET_NODETYPE   (FBAS_SHARED_SET_ID         + _32b_SIZE_)       // FBAS node type
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
#define FBAS_SHARED_GET_TS2        (FBAS_SHARED_GET_TS1        + 2 * _32b_SIZE_)   // timestamp2 (generator event polled by TX)
#define FBAS_SHARED_GET_TS3        (FBAS_SHARED_GET_TS2        + 2 * _32b_SIZE_)   // timestamp3 (IO action event deadline)
#define FBAS_SHARED_GET_TS4        (FBAS_SHARED_GET_TS3        + 2 * _32b_SIZE_)   // timestamp4 (IO action event polled by TX)
#define FBAS_SHARED_GET_TS5        (FBAS_SHARED_GET_TS4        + 2 * _32b_SIZE_)   // timestamp5 (measure time period)
#define FBAS_SHARED_GET_TS6        (FBAS_SHARED_GET_TS5        + 2 * _32b_SIZE_)   // timestamp6 (measure time period)
#define FBAS_SHARED_GET_AVG        (FBAS_SHARED_GET_TS6        + 2 * _32b_SIZE_)   // average (performance, delay measurements)
#define FBAS_SHARED_GET_MIN        (FBAS_SHARED_GET_AVG        + 2 * _32b_SIZE_)   // min
#define FBAS_SHARED_GET_MAX        (FBAS_SHARED_GET_MIN        + 2 * _32b_SIZE_)   // max
#define FBAS_SHARED_GET_VLD        (FBAS_SHARED_GET_MAX        + 2 * _32b_SIZE_)   // valid counts
#define FBAS_SHARED_GET_ALL        (FBAS_SHARED_GET_VLD        + 2 * _32b_SIZE_)   // all/total counts
#define FBAS_SHARED_ECA_VLD        (FBAS_SHARED_GET_ALL        + 2 * _32b_SIZE_)   // number of the valid actions output by ECA
#define FBAS_SHARED_ECA_OVF        (FBAS_SHARED_ECA_VLD        + _32b_SIZE_)       // number of the overflow actions output by ECA
#define FBAS_SHARED_SENDERID       (FBAS_SHARED_ECA_OVF        + _32b_SIZE_)       // location of valid sender ID that is passed to RX node
#define FBAS_SHARED_TX_DLY_AVG     (FBAS_SHARED_SENDERID       + 2 * _32b_SIZE_)   // transmission delay (LEMO feedback), avg
#define FBAS_SHARED_TX_DLY_MIN     (FBAS_SHARED_TX_DLY_AVG     + 2 * _32b_SIZE_)   // min
#define FBAS_SHARED_TX_DLY_MAX     (FBAS_SHARED_TX_DLY_MIN     + 2 * _32b_SIZE_)   // max
#define FBAS_SHARED_TX_DLY_VLD     (FBAS_SHARED_TX_DLY_MAX     + 2 * _32b_SIZE_)   // valid count
#define FBAS_SHARED_TX_DLY_ALL     (FBAS_SHARED_TX_DLY_VLD     + _32b_SIZE_)       // all/total count
#define FBAS_SHARED_SG_LTY_AVG     (FBAS_SHARED_TX_DLY_ALL     + _32b_SIZE_)       // signalling latency (LEMO feedback), avg
#define FBAS_SHARED_SG_LTY_MIN     (FBAS_SHARED_SG_LTY_AVG     + 2 * _32b_SIZE_)   // min
#define FBAS_SHARED_SG_LTY_MAX     (FBAS_SHARED_SG_LTY_MIN     + 2 * _32b_SIZE_)   // max
#define FBAS_SHARED_SG_LTY_VLD     (FBAS_SHARED_SG_LTY_MAX     + 2 * _32b_SIZE_)   // valid count
#define FBAS_SHARED_SG_LTY_ALL     (FBAS_SHARED_SG_LTY_VLD     + _32b_SIZE_)       // all/total count
#define FBAS_SHARED_MSG_DLY_AVG    (FBAS_SHARED_SG_LTY_ALL     + _32b_SIZE_)       // messaging delay, avg
#define FBAS_SHARED_MSG_DLY_MIN    (FBAS_SHARED_MSG_DLY_AVG    + 2 * _32b_SIZE_)   // min
#define FBAS_SHARED_MSG_DLY_MAX    (FBAS_SHARED_MSG_DLY_MIN    + 2 * _32b_SIZE_)   // max
#define FBAS_SHARED_MSG_DLY_VLD    (FBAS_SHARED_MSG_DLY_MAX    + 2 * _32b_SIZE_)   // valid count
#define FBAS_SHARED_MSG_DLY_ALL    (FBAS_SHARED_MSG_DLY_VLD    + _32b_SIZE_)       // all/total count
#define FBAS_SHARED_TTL_PRD_AVG    (FBAS_SHARED_MSG_DLY_ALL    + _32b_SIZE_)       // TTL, avg
#define FBAS_SHARED_TTL_PRD_MIN    (FBAS_SHARED_TTL_PRD_AVG    + 2 * _32b_SIZE_)   // min
#define FBAS_SHARED_TTL_PRD_MAX    (FBAS_SHARED_TTL_PRD_MIN    + 2 * _32b_SIZE_)   // max
#define FBAS_SHARED_TTL_PRD_VLD    (FBAS_SHARED_TTL_PRD_MAX    + 2 * _32b_SIZE_)   // valid count
#define FBAS_SHARED_TTL_PRD_ALL    (FBAS_SHARED_TTL_PRD_VLD    + _32b_SIZE_)       // all/total count
#define FBAS_SHARED_ECA_HNDL_AVG   (FBAS_SHARED_TTL_PRD_ALL    + _32b_SIZE_)       // ECA event handling, avg
#define FBAS_SHARED_ECA_HNDL_MIN   (FBAS_SHARED_ECA_HNDL_AVG   + 2 * _32b_SIZE_)   // min
#define FBAS_SHARED_ECA_HNDL_MAX   (FBAS_SHARED_ECA_HNDL_MIN   + 2 * _32b_SIZE_)   // max
#define FBAS_SHARED_ECA_HNDL_VLD   (FBAS_SHARED_ECA_HNDL_MAX   + 2 * _32b_SIZE_)   // valid count
#define FBAS_SHARED_ECA_HNDL_ALL   (FBAS_SHARED_ECA_HNDL_VLD   + _32b_SIZE_)       // all/total count
#define FBAS_SHARED_ML_PRD_AVG     (FBAS_SHARED_ECA_HNDL_ALL   + _32b_SIZE_)       // main loop period, avg
#define FBAS_SHARED_ML_PRD_MIN     (FBAS_SHARED_ML_PRD_AVG     + 2 * _32b_SIZE_)   // min
#define FBAS_SHARED_ML_PRD_MAX     (FBAS_SHARED_ML_PRD_MIN     + 2 * _32b_SIZE_)   // max
#define FBAS_SHARED_ML_PRD_VLD     (FBAS_SHARED_ML_PRD_MAX     + 2 * _32b_SIZE_)   // valid count
#define FBAS_SHARED_ML_PRD_ALL     (FBAS_SHARED_ML_PRD_VLD     + _32b_SIZE_)       // all/total count
#define FBAS_SHARED_BAD_MSG_CNT    (FBAS_SHARED_ML_PRD_ALL     + _32b_SIZE_)       // bad message count
#define FBAS_SHARED_END            (FBAS_SHARED_BAD_MSG_CNT    + _32b_SIZE_)       // end of the app-spec region

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
#define FBAS_CMD_GET_SENDERID   0x19   // get sender IDs (sender MAC addresses)
#define FBAS_CMD_PROBE_SB_DIOB  0x20   // probe DIOB slave card on SCU bus
#define FBAS_CMD_PROBE_SB_USER  0x21   // probe a given slave (sys and group IDs are expected in shared mem @FBAS_SHARED_SET_SBSLAVES)
#define FBAS_CMD_EN_MPS_FWD     0x30   // enable MPS signal forwarding
#define FBAS_CMD_DIS_MPS_FWD    0x31   // disable MPS signal forwarding
#define FBAS_CMD_PRINT_NW_DLY   0x32   // print result of network delay measurement
#define FBAS_CMD_PRINT_MSG_DLY  0x33   // print the measurement result of messaging delay
#define FBAS_CMD_PRINT_SG_LTY   0x34   // print result of MPS signalling latency measurement
#define FBAS_CMD_PRINT_TTL      0x35   // print result of TTL interval measurement
#define FBAS_CMD_PRINT_MPS_BUF  0x36   // print all MPS message relevant buffers
#define FBAS_CMD_PRINT_ECA_HANDLE  0x37   // print the measurement result of the ECA handling delay
#define FBAS_CMD_CLR_SUM_STATS  0x38   // clear the summar statistics

// mask bit for MPS-relevant tasks (up to 31)
#define TSK_TX_MPS_FLAGS        0x10000000 // transmit MPS flags
#define TSK_TX_MPS_EVENTS       0x20000000 // transmit MPS events
#define TSK_MONIT_MPS_TTL       0x40000000 // monitor lifetime of MPS flags
#define TSK_EVAL_MPS_TTL        0x80000000 // evaluate the lifetime of MPS flags
#define TSK_REG_COMPLETE        0x01000000 // registration is complete
#define TSK_REG_PER_OVER        0x02000000 // registration period is over

// ECA action tags
#define FBAS_GEN_EVT       0x42      // ECA condition tag for generator event (handled by TX)
#define FBAS_TLU_EVT       0x43      // ECA condition tag for TLU event (handled by TX)
#define FBAS_NODE_REG      0x45      // ECA condition tag for the node registration
#define FBAS_WR_EVT        0x24      // ECA condition tag for MPS event via WR (handled by RX)
#define FBAS_WR_FLG        0x25      // ECA condition tag for MPS flag via WR (handled by RX)
#define FBAS_AUX_NEWCYCLE  0x26      // ECA condition tag for MPS auxiliary signal (clear internal errors in TX & RX)
#define FBAS_AUX_OPMODE    0x27      // ECA condition tag for MPS auxiliary signal (specify operation mode for TX & RX)

#endif
