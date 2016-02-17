#ifndef __HKBOARD_H
#define __HKBOARD_H

#ifdef  CPU_CLOCK
#undef  CPU_CLOCK
#endif

#define CPU_CLOCK 125000000ULL

#define CONFIG_SRC    0x0
#define WD_TIMEOUT    0x2
#define WD_EN         0x3
#define PAGE_SEL      0x4
#define CONFIG_MODE   0x5
#define CONFIG        0x8
#define POF_ERROR     0x9

#define CRCERROR      0x1
#define NSTATUS       0x2
#define LOGIC         0x4
#define NCONFIG       0x8
#define WDTIMER       0x10

#define ASMI_PARAM    0x20
#define ASMI_CMD      0x24
#define ASMI_STATUS   0x28
#define ASMI_RDID     0x2c
#define FW_VERSION    0x30
#define ASMI_BUFFER   0x40

#define DONE              0x0
#define READ_ASMI_STATUS  0x1
#define READ_ASMI_ID      0x2
#define SECTOR_ERASE      0x3
#define PAGE_WRITE        0x4
#define PAGE_READ         0x5
#define RECONFIG          0x6
#define READ_TEMP         0x7


#endif
