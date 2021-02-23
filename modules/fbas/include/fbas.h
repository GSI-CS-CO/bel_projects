#ifndef _FBAS_H_
#define _FBAS_H_

// ****************************************************************************************
// DP RAM layout (offsets)
// ****************************************************************************************

// set values for data supply
#define FBAS_SHARED_SET_GID        (COMMON_SHARED_END          + _32b_SIZE_)       // GID of B2B Transfer ('EXTRING_B2B_...')
#define FBAS_SHARED_SET_SID        (FBAS_SHARED_SET_GID        + _32b_SIZE_)       // sequence ID for B2B transfer
#define FBAS_SHARED_SET_NODETYPE   (FBAS_SHARED_SET_SID        + _32b_SIZE_)       // FBAS node type
//get values
#define FBAS_SHARED_GET_GID        (FBAS_SHARED_SET_NODETYPE   + _32b_SIZE_)       // GID of B2B Transfer ('EXTRING_B2B_...')
#define FBAS_SHARED_GET_SID        (FBAS_SHARED_GET_GID        + _32b_SIZE_)       // sequence ID for B2B transfer
#define FBAS_SHARED_GET_NODETYPE   (FBAS_SHARED_GET_SID        + _32b_SIZE_)       // FBAS node type

// diagnosis: end of used shared memory
#define FBAS_SHARED_END            (FBAS_SHARED_GET_NODETYPE   + _32b_SIZE_)

typedef struct {
  uint64_t evtId;    // event ID
  uint64_t mac;      // MAC address (prepended with zeros)
} mpsEventData_t;

#define MPS_PAYLOAD_SIZE sizeof(mpsEventData_t)/sizeof(uint32_t)

// valid value for data fields in the MPS payload
#define MPS_VID_FBAS     105   // VLAN ID for FBAS

// node type
typedef enum {
  FBAS_NODE_TX = 0,   // FBAS transmitter
  FBAS_NODE_RX,       // FBAS receiver
  FBAS_NODE_CM,       // FBAS common
  FBAS_NODE_UNDEF     // undefined
} nodeType_t;

// application-specific commands
#define FBAS_CMD_SET_NODETYPE   0x15   // set the node type
#define FBAS_CMD_SET_LVDS_OE    0x16   // set LVDS output enable
#define FBAS_CMD_GET_LVDS_OE    0x17   // get LVDS output enable
#define FBAS_CMD_TOGGLE_LVDS    0x18   // toggle LVDS output

// flags
#define MPS_FLAG_OK        1   // OK
#define MPS_FLAG_NOK       2   // NOK
#define MPS_FLAG_TEST      3   // TEST

// time interval
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

// ECA action tags
#define FBAS_IO_ACTION     0x42      // ECA condition tag for FBAS TX
#define FBAS_WR_EVT        0x24      // ECA condition tag for FBAS RX

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
