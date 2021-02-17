#ifndef _FBAS_H_
#define _FBAS_H_

typedef struct {
  uint64_t evtId;    // event ID
  uint64_t mac;      // MAC address (prepended with zeros)
} mpsEventData_t;

#define MPS_PAYLOAD_SIZE sizeof(mpsEventData_t)/sizeof(uint32_t)

// valid value for data fields in the MPS payload
#define MPS_VID_FBAS     105   // VLAN ID for FBAS

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
#define FBAS_IO_ACTION     0x42

#endif
