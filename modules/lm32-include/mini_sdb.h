#ifndef _MINI_SDB_
#define _MINI_SDB_

#include <inttypes.h>
#include <stdint.h>

///////////////////////////////////////////////////////////////
//  SBD BASE ADR IS AUTOMAPPED IN GATEWARE. USE getRootSdb() //
///////////////////////////////////////////////////////////////

#define SDB_INTERCONNET 0x00
#define SDB_DEVICE      0x01
#define SDB_BRIDGE      0x02
#define SDB_MSI         0x03
#define SDB_EMPTY       0xFF



#define ERROR_NOT_FOUND  0xFFFFFFFE
#define NO_MSI           0XDEADBEE3
#define OWN_MSI          (1<<31)


#define GSI                   0x00000651
#define CERN                  0x0000ce42


//MSI message forwarding box for master2master MSI
#define MSI_MSG_BOX           0xfab0bdd8

//CPU periphery
#define CPU_INFO_ROM          0x10040085
#define CPU_ATOM_ACC          0x10040100
#define CPU_SYSTEM_TIME       0x10040084
#define CPU_TIMER_CTRL_IF     0x10040088
#define CPU_MSI_CTRL_IF       0x10040083
#define CPU_MSI_TGT           0x1f1a4e39

//Cluster periphery
#define LM32_CB_CLUSTER       0x10041000
#define CLU_INFO_ROM          0x10040086
#define LM32_RAM_SHARED       0x81111444
#define FTM_PRIOQ_CTRL        0x10040200
#define FTM_PRIOQ_DATA        0x10040201

//External interface to CPU RAMs & IRQs
#define LM32_RAM_USER         0x54111351
#define LM32_IRQ_EP           0x10050083

//Generic stuff
#define CB_GENERIC            0xeef0b198
#define DPRAM_GENERIC         0x66cfeb52
#define IRQ_ENDPOINT          0x10050082
#define PCIE_IRQ_ENDP         0x8a670e73

//IO Devices
#define OLED_DISPLAY          0x93a6f3c4
#define SSD1325_SER_DRIVER    0x55d1325d
#define ETHERBONE_MASTER      0x00000815
#define ETHERBONE_CFG         0x68202b22


#define ECA_EVENT             0x8752bf45
#define ECA_CTRL              0x8752bf44
#define TLU                   0x10051981 
#define WR_UART               0xe2d13d04
#define SCU_BUS_MASTER        0x9602eb6f
#define SCU_IRQ_CTRL          0x9602eb70
#define WB_FG_IRQ_CTRL        0x9602eb71
#define MIL_IRQ_CTRL          0x9602eb72

#define SCU_BUS_MASTER        0x9602eb6f
#define WR_1Wire              0x779c5443
#define WB_FG_QUAD            0x863e07f0

#define WR_CFIPFlash          0x12122121
#define WB_DDR3_if1           0x20150828
#define WB_DDR3_if2           0x20160525
#define WR_SYS_CON            0xff07fc47  
#define WB_REMOTE_UPDATE      0x38956271
#define WB_ASMI               0x48526423
#define WB_SCU_REG            0xe2d13d04

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

#endif
