#ifndef _MINI_SDB_
#define _MINI_SDB_

#define SDB_INTERCONNET 0x00
#define SDB_DEVICE      0x01
#define SDB_BRIDGE      0x02
#define SDB_EMPTY       0xFF

#define CPU_INFO_ROM          0x10040085
#define IRQ_TIMER_CTRL_IF     0x10040088
#define IRQ_MSI_CTRL_IF       0x10040083
#define SYSTEM_TIME           0x10040084
#define ATOMIC_BUS_ACCESS     0x10040100

#define LM32_CLUSTER_INFO_ROM 0x10040086
#define SCU_OLED_DISPLAY      0x93a6f3c4
#define ETHERBONE_MASTER      0x00000815







extern volatile unsigned int* pSDB_base;

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

struct sdb_interconnect {
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
        struct sdb_interconnect interconnect;
} sdb_record_t;

unsigned char *find_device_deep(unsigned int base, unsigned int sdb,
                                       unsigned int devid);

unsigned char *find_device(unsigned int devid);

#endif
