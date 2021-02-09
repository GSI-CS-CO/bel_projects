#ifndef _FBAS_H_
#define _FBAS_H_

typedef struct {
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

// FBAS event tag
#define FBAS_IO_ACTION     0x42

#endif
