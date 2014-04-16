#ifndef _MINI_SDB_
#define _MINI_SDB_

////////////////////////////////////////////
//  SBD BASE ADR IS GATEWARE DEPENDENT!   //
//  SEE modules/ftm/ftm_lm32.vhd          // 
//                                        //   
#define SBD_BASE        0x3FFFE000        //
//                                        //
////////////////////////////////////////////

#define SDB_INTERCONNET 0x00
#define SDB_DEVICE      0x01
#define SDB_BRIDGE      0x02
#define SDB_EMPTY       0xFF

#define CPU_INFO_ROM          0x10040085
#define CPU_ATOM_ACC          0x10040100
#define CPU_SYSTEM_TIME       0x10040084
#define CPU_CLU_INFO_ROM      0x10040086
#define IRQ_TIMER_CTRL_IF     0x10040088
#define IRQ_MSI_CTRL_IF       0x10040083

#define OLED_DISPLAY          0x93a6f3c4
#define ETHERBONE_MASTER      0x00000815

#define FTM_PRIOQ_CTRL        0x10040200
#define FTM_PRIOQ_DATA        0x10040201

#define ECA_EVENT             0x8752bf45
#define ECA_CTRL              0x8752bf44 
#define WR_UART               0xe2d13d04
#define WR_1Wire              0x779c5443
#define SCU_BUS_MASTER        0x9602eb6f

//periphery device pointers
volatile unsigned int* pEbm;     
volatile unsigned int* pOledDisplay;     
volatile unsigned int* pFpqCtrl;
volatile unsigned int* pFpqData;
volatile unsigned int* pEca;
volatile unsigned int* pCpuId;
volatile unsigned int* pCpuIrqSlave;
volatile unsigned int* pCpuAtomic;
volatile unsigned int* pCpuSysTime;
volatile unsigned int* pCpuTimer;
volatile unsigned int* pCluInfo;
volatile unsigned int* pUart;
volatile unsigned int* BASE_UART;

typedef struct pair64 {
        unsigned int high;
        unsigned int low;
} pair64_t;

struct sdb_empty {
        char reserved[63];
        unsigned char record_type;
};

struct sdb_product {
        pair64_t vendor_id;
        unsigned int device_id;
        unsigned int version;
        unsigned int date;
        char name[19];
        unsigned char record_type;
};

struct sdb_component {
        pair64_t addr_first;
        pair64_t addr_last;
        struct sdb_product product;
};

struct sdb_device {
        unsigned short abi_class;
        unsigned char abi_ver_major;
        unsigned char abi_ver_minor;
        unsigned int bus_specific;
        struct sdb_component sdb_component;
};

struct sdb_bridge {
	pair64_t sdb_child;
	struct sdb_component sdb_component;
};

struct SDB_INTERCONNECT {
        unsigned int sdb_magic;
        unsigned short sdb_records;
        unsigned char sdb_version;
        unsigned char sdb_bus_type;
        struct sdb_component sdb_component;
};

typedef union sdb_record {
        struct sdb_empty empty;
        struct sdb_device device;
        struct sdb_bridge bridge;
        struct SDB_INTERCONNECT interconnect;
} sdb_record_t;

unsigned char *find_device_deep(unsigned int base, unsigned int sdb,
                                       unsigned int devid);

unsigned char *find_device(unsigned int devid);

void discoverPeriphery();


#endif
