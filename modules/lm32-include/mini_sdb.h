#ifndef _MINI_SDB_
#define _MINI_SDB_

#include <inttypes.h>
#include <stdint.h>
#include <sdb_ids.h>

#ifdef __cplusplus
extern "C" {
#endif

//periphery device pointers
volatile uint32_t* pTlu; 
volatile uint32_t* pEbm;
volatile uint32_t* pEbCfg;

volatile uint32_t* pEbmLast;
volatile uint32_t* pOledDisplay;     
volatile uint32_t* pFpqCtrl;
volatile uint32_t* pFpqData;
volatile uint32_t* pEca;
volatile uint32_t* pCpuId;
volatile uint32_t* pCpuIrqSlave;
volatile uint32_t* pCpuAtomic;
volatile uint32_t* pCpuSysTime;
volatile uint32_t* pCluInfo;
volatile uint32_t* pCpuMsiBox;
volatile uint32_t* pMyMsi;
volatile uint32_t* pUart;
volatile uint32_t* pPps;
//volatile uint32_t* BASE_UART;
volatile uint32_t* pCluCB;
volatile uint32_t* pOneWire;

volatile uint32_t* pCfiPFlash;

volatile uint32_t* pDDR3_if1;
volatile uint32_t* pDDR3_if2;


typedef struct pair64 {
  uint32_t high;
  uint32_t low;
} pair64_t;

struct sdb_empty {
  char reserved[63];
  uint8_t record_type;
};

struct sdb_product {
  pair64_t  vendor_id;
  uint32_t  device_id;
  uint32_t  version;
  uint32_t  date;
  char      name[19];
  uint8_t   record_type;
};

struct sdb_component {
  pair64_t addr_first;
  pair64_t addr_last;
  struct sdb_product product;
};

struct sdb_msi {
  uint32_t msi_flags;
  uint32_t bus_specific;
  struct sdb_component sdb_component;
};

struct sdb_device {
  uint16_t abi_class;
  uint8_t abi_ver_major;
  uint8_t abi_ver_minor;
  uint32_t bus_specific;
  struct sdb_component sdb_component;
};

struct sdb_bridge {
  pair64_t sdb_child;
  struct sdb_component sdb_component;
};

struct SDB_INTERCONNECT {
  uint32_t sdb_magic;
  uint16_t sdb_records;
  uint8_t sdb_version;
  uint8_t sdb_bus_type;
  struct sdb_component sdb_component;
};

typedef union sdb_record {
  struct sdb_empty empty;
  struct sdb_msi msi;
  struct sdb_device device;
  struct sdb_bridge bridge;
  struct SDB_INTERCONNECT interconnect;
} sdb_record_t;

typedef struct sdb_location {
  sdb_record_t* sdb;
  uint32_t adr;
  uint32_t msi_first;
  uint32_t msi_last;
} sdb_location;

sdb_location*  find_device_multi(sdb_location *found_sdb, uint32_t *idx, uint32_t qty, uint32_t venId, uint32_t devId);
uint32_t*      find_device_adr(uint32_t venId, uint32_t devId);
sdb_location*  find_device_multi_in_subtree(sdb_location *loc, sdb_location *found_sdb, uint32_t *idx, uint32_t qty, uint32_t venId, uint32_t devId);
uint32_t*      find_device_adr_in_subtree(sdb_location *loc, uint32_t venId, uint32_t devId);

sdb_location *find_sdb_deep(sdb_record_t *parent_sdb, sdb_location *found_sdb, uint32_t base, uint32_t msi_base,  uint32_t msi_last, uint32_t *idx, uint32_t qty, uint32_t venId, uint32_t devId);
uint32_t       getSdbAdr(sdb_location *loc);
uint32_t       getSdbAdrLast(sdb_location *loc);
uint32_t       getMsiAdr(sdb_location *loc);
uint32_t       getMsiAdrLast(sdb_location *loc);
sdb_record_t*  getChild(sdb_location *loc);
uint32_t       getMsiUpperRange();


uint8_t*       find_device(uint32_t devid); //DEPRECATED, USE find_device_adr INSTEAD!

void           discoverPeriphery(void);

#ifdef __cplusplus
}
#endif

#endif
