#ifndef __SB_SCAN_
#define __SB_SCAN_

// global definitions
#define N_SB_SLOTS        12 // number of SCU bus slots
#define N_DIOB_CFG        2  // number of the configuration registers in DIOB
#define N_DIOB_STS        2  // number of the status registers in DIOB
#define N_USR_CFG         2  // number of the configuration registers in user interface card
#define N_USR_STS         2  // number of the status registers in user interface card
#define N_USR_OUT         3  // number of the output registers in user interface card
#define N_USR_IN          2  // number of the input registers in user interface card
#define SBS_CID_NO_EXT    0xDEAD // no extension card is installed

// CID for DIOB
#define CID_SYS_DIOB      55
#define CID_GRP_DIOB      26

// base address of standard registers for all slave devices on the SCU bus
#define STD_REG_BASE      0x0000

// standard registers for SCU bus slaves, https://www-acc.gsi.de/wiki/Hardware/Intern/StdRegScuBusSlave
#define SBS_SLAVE_ID      0x01 // slave ID, reserved
#define SBS_FW_VER        0x02 // firmware version, rd
#define SBS_FW_REL        0x03 // firmware release, rd
#define SBS_CID_SYS       0x04 // CID (component ID) system ID, rd
#define SBS_CID_GRP       0x05 // CID group ID, rd
#define SBS_MACRO_VER     0x06 // macro version and release, rd
#define SBS_EXT_CID_SYS   0x07 // CID system ID of extension card (dead=no extension), rd
#define SBS_EXT_CID_GRP   0x08 // CID group ID of extension card (dead=no extension), rd
#define SBS_CLK_10K       0x09 // macro clock frequency, 10KHz resolution, rd
#define SBS_ECHO          0x10 // echo register
#define SBS_STATUS        0x11 // status register, rd
#define SBS_INT_IN        0x20 // interrupt input register, rd
#define SBS_INT_ENA       0x21 // interrupt enable register, 1=enabled
#define SBS_INT_PEND      0x22 //
#define SBS_INT_ACT       0x24 // interrupt active register

//
#define GLOBAL_IRQ_ENA    0x2
#define SRQ_ENA           0x6
#define SRQ_ACT           0x8
#define MULTI_SLAVE_SEL   0xc
#define MULTICAST_ACC     0x8

// SCU bus SDB record
#define SB_SDB_BASE      0x7FFFFA00

// base address of user interface card (DIOBx+Interface)
#define USR_REG_BASE     0x0500

// offsets for DIOB specific registers: 16-bit register, offset, access
#define DIOB_Config_Reg1 0x00 // rw
#define DIOB_Config_Reg2 0x01 // rw
#define DIOB_Status_Reg1 0x02 // r
#define DIOB_Status_Reg2 0x03 // r

#define Usr_Config_Reg1  0x07 // rw
#define Usr_Config_Reg2  0x08 // rw
#define Usr_Status_Reg1  0x09 // r
#define Usr_Status_Reg2  0x0A // r
#define MirrorMode_OutReg_Mask 0x0E // rw

#define Usr_Out_Reg1     0x10 // rw
#define Usr_Out_Reg2     0x11 // rw
#define Usr_Out_Reg3     0x12 // rw
#define Usr_Out_Reg4     0x13 // rw
#define Usr_Out_Reg5     0x14 // rw
#define Usr_Out_Reg6     0x15 // rw
#define Usr_Out_Reg7     0x16 // rw

#define Out_Puls_Sel_Reg1 0x17 // rw
#define Out_Puls_Width_Reg10 0x18 // rw
#define Out_Puls_Width_Reg11 0x19 // rw
#define Out_Puls_Width_Reg12 0x1A // rw
#define Out_Puls_Width_Reg13 0x1B // rw

#define Usr_In_Reg1      0x20 // r
#define Usr_In_Reg2      0x21 // r
#define Usr_In_Reg3      0x22 // r
#define Usr_In_Reg4      0x23 // r
#define Usr_In_Reg5      0x24 // r
#define Usr_In_Reg6      0x25 // r
#define Usr_In_Reg7      0x26 // r

#define Usr_InLatch_Edge_Sel1 0x27 // rw
#define Usr_In_Reg_Latch1     0x28 // r
#define Usr_InLatch_Edge_Sel2 0x29 // rw
#define Usr_In_Reg_Latch2     0x2A // r

#define CmpUnit_Config_Reg1 0x50 // rw
#define Cmp_Val_Reg1        0x52 // 32-bit, rw
#define CmpUnit_Config_Reg2 0x54 // rw
#define Cmp_Val_Reg2        0x56 // 32-bit, rw
#define CounterUnit_Config_Reg1 0x60 // rw
#define Cnt_Val_Reg1            0x62 // 32-bit, rw
#define CounterUnit_Config_Reg2 0x64 // rw
#define Cnt_Val_Reg2            0x66 // 32-bit, rw

#define Config_TagID1       0x80 // rw
#define Config_TagID2       0x90 // rw
#define Config_TagID3       0xA0 // rw
#define Config_TagID4       0xB0 // rw
#define Config_TagID5       0xC0 // rw
#define Config_TagID6       0xD0 // rw
#define Config_TagID7       0xE0 // rw
#define Config_TagID8       0xF0 // rw

// bit masks for DIOB-Config-Reg1
#define MSK_TEST_MODE      0x8000 // bit 15, 0=normal, 1=test mode (diagnostics, Inbetriebnahme)
#define MSK_DEBOUNCE_TIME  0x7000 // bit 14-12, debounce time for inputs, range=1-128 us (power of 2)
#define MSK_DEBOUNCE_EN    0x0800 // bit 11, debounce enable, 0=enabled
#define MSK_MIRROR_INSEL   0x0700 // bit 10-8, selection of input registers for the mirror mode, 0=none, 1-7=input register 1-7
#define MSK_MIRROR_OUTSEL  0x00E0 // bit 7-5, selection of output registers for the mirror mode, 0=none, 1-7=output register 1-7
#define MSK_MIRROR_EN      0x0008 // bit 3, enable the mirror mode, 1=enabled
#define MSK_CLR_CNT        0x0004 // bit 2, clear all configuration registers for counters, 1=clear
#define MSK_CLR_CMP        0x0002 // bit 1, clear all configuration registers for compators, 1=clear
#define MSK_CLR_TAG        0x0001 // bit 0, clear all configuration registers for events, 1=clear

// bit positions for DIOB-Config-Reg1
#define POS_TEST_MODE      15 // bit 15, 0=normal, 1=test mode (diagnostics, Inbetriebnahme)
#define POS_DEBOUNCE_TIME  12 // bit 14-12, debounce time for inputs, range=1-128 us (exponent)
#define POS_DEBOUNCE_EN    11 // bit 11, debounce enable, 0=enabled
#define POS_MIRROR_INSEL   8 // bit 10-8, selection of input registers for the mirror mode, 0=none, 1-7=input register 1-7
#define POS_MIRROR_OUTSEL  5 // bit 7-5, selection of output registers for the mirror mode, 0=none, 1-7=output register 1-7
#define POS_MIRROR_EN      3 // bit 3, enable the mirror mode, 1=enabled
#define POS_CLR_CNT        2 // bit 2, clear all configuration registers for counters, 1=clear
#define POS_CLR_CMP        1 // bit 1, clear all configuration registers for compators, 1=clear
#define POS_CLR_TAG        0 // bit 0, clear all configuration registers for events, 1=clear

typedef struct regset {
  uint16_t base;   // base address of register set
  uint16_t offset; // start offset of registers
  uint16_t len;    // number of registers
} regset_t;


#endif
