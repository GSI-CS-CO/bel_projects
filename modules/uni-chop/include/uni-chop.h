#ifndef _UNICHOP_
#define _UNICHOP_

#include <common-defs.h>

// this file is structured in two parts
// 1st: definitions of general things like error messages
// 2nd: definitions for data exchange via DP RAM

// ****************************************************************************************
// general things
// ****************************************************************************************

// (error) status, each status will be represented by one bit, bits will be ORed into a 32bit word
#define UNICHOP_STATUS_MIL                    16   // an error on MIL hardware occured (MIL piggy etc...)

// activity requested by ECA Handler, the relevant codes are also used as "tags"
#define UNICHOP_ECADO_TIMEOUT    COMMON_ECADO_TIMEOUT
#define UNICHOP_ECADO_UNKOWN                   1   // unkown activity requested (unexpected action by ECA)
#define UNICHOP_ECADO_STRAHLWEG_WRITE      0xfa0   // writes data to chopper control;         param 0..15: Strahlwegregister; param 16..31: Strahlwegmaske
#define UNICHOP_ECADO_STRAHLWEG_READ       0xfa1   // request read data from chopper control; param 0..15: Strahlwegregister; param 16..31: Strahlwegmaske
#define UNICHOP_ECADO_MIL_SWRITE           0xfb0   // standard data write to MIL;             param: 32..39 ifb addr, 24..31 mod addr, 16..23: reg addr, 0..15 data
#define UNICHOP_ECADO_MIL_SREAD            0xfb1   // standard data read from MIL;            param: 32..39 ifb addr, 24..31 mod addr, 16..23: reg addr, 0..15 reserved
#define UNICHOP_ECADO_QRSTOP               0xfc0   // read actual chopper data QR             param: 48..63 t ctrl falling edge, 32..47 t chop rising edge, 16..31 t chop falling edge, 0..15 chop length [us] value 0: no pulse, value 0xffff invalid;
#define UNICHOP_ECADO_QLSTOP               0xfc1   // read actual chopper data QL             param: 48..63 t ctrl falling edge, 32..47 t chop rising edge, 16..31 t chop falling edge, 0..15 chop length [us] value 0: no pulse, value 0xffff invalid;
#define UNICHOP_ECADO_HLISTOP              0xfc2   // read actual chopper data HLI            param: 48..63 t ctrl falling edge, 32..47 t chop rising edge, 16..31 t chop falling edge, 0..15 chop length [us] value 0: no pulse, value 0xffff invalid;
#define UNICHOP_ECADO_HSISTOP              0xfc3   // read actual chopper data HSI            param: 48..63 t ctrl falling edge, 32..47 t chop rising edge, 16..31 t chop falling edge, 0..15 chop length [us] value 0: no pulse, value 0xffff invalid;
#define UNICHOP_ECADO_HLICMD               0xfc4   // received EVT_COMMAND @HLI
#define UNICHOP_ECADO_HSICMD               0xfc5   // received EVT_COMMAND @HSI

#define UNICHOP_ECADO_IQSTOP               0xf0a   // read RPG ion source;                    param: 16..20 value 1: QR value 2: QL, 0..15 length of RPG length counter [us] value 0: no pulse; value 0xffff invalid; 

// commands from the outside

// GIDs
#define GID_INVALID                          0x0   // invalid GID
#define GID_PZU_QR                         0x1c0   // UNILAC Timing - Source Right
#define GID_PZU_QL                         0x1c1   // UNILAC Timing - Source Left
#define GID_PZU_UN                         0x1c3   // UNILAC Timing - High Charge State Injector (HLI)
#define GID_PZU_UH                         0x1c4   // UNILAC Timing - High Current Injector (HSI)
#define GID_LOCAL_ECPU_TO                  0xff0   // internal: group for local timing messages that are sent (from the host) TO the lm32 firmware
#define GID_LOCAL_ECPU_FROM                0xff1   // internal: group for local timing messages that are sent FROM the lm32 firmware (to the host)


// specialities
#define UNICHOP_NSID                          16   // max number of data (SID)


// part below provided by Ludwig Hechler and Stefan Rauch
// interface boards: common function codes
#define IFB_FC_ADDR_BUS_W                   0x11   // function code, address bus write
#define IFB_FC_DATA_BUS_W                   0x10   // function code, data bus write
#define IFB_FC_DATA_BUS_R                   0x90   // function code, data bus read

// interface board: main chopper control
#define IFB_ADDR_CU                         0x60   // MIL address of chopper IF; FG 380.221

// modulebus module Logic1
#define MOD_LOGIC1_ADDR                     0x09   // logic module 1 (Strahlwege et al),  module bus address, FG 450.410/411
#define MOD_LOGIC1_REG_STRAHLWEG_REG        0x60   // logic module 1, register/address for Strahlwegregister
#define MOD_LOGIC1_REG_STRAHLWEG_MASK       0x62   // logic module 1, register/subaddress for Strahlwegmaske
#define MOD_LOGIC1_REG_STATUSGLOBAL         0x66   // logic module 1, register/subaddress for global status, contains version number (2024: 0x14)
#define MOD_LOGIC1_REG_HSI_ACT_POSEDGE_RD   0x6c   // logic module 1, register/subaddress for time of rising edge of chopper readback pulse (HSI)
#define MOD_LOGIC1_REG_HSI_ACT_NEGEDGE_RD   0x70   // logic module 1, register/subaddress for time of falling edge of chopper readback pulse (HSI)
#define MOD_LOGIC1_REG_HSI_CTRL_NEGEDGE_RD  0x6e   // logic module 1, register/subaddress for time of falling edge of chopper control pulse (HSI)
#define MOD_LOGIC1_REG_HLI_ACT_POSEDGE_RD   0x74   // logic module 1, register/subaddress for time of rising edge of chopper readback pulse (HLI)
#define MOD_LOGIC1_REG_HLI_ACT_NEGEDGE_RD   0x76   // logic module 1, register/subaddress for time of falling edge of chopper readback pulse (HLI)
#define MOD_LOGIC1_REG_HLI_CTRL_NEGEDGE_RD  0x78   // logic module 1, register/subaddress for time of falling edge of chopper control pulse (HLI)

// modulebus module Logic2
#define MOD_LOGIC2_ADDR                     0x08   // logic module 2 (Strahlwege et al),  module bus address, FG 450.410/411
#define MOD_LOGIC2_REG_ANFORDER_MASK        0x68   // logic module 2, register/subaddress for Strahlwegmaske
#define MOD_LOGIC2_REG_STATUSGLOBAL         0x66   // logic module 2, register/subaddress for global status, contains version number (2024: 0x03)

// modulebus modules for Rahmenpulsgeneratoren
#define MOD_RPG_IQR_ADDR                    0x01   // Rahmenpulsgenerator IQL, module bus address, FG 450.681
#define MOD_RPG_IQL_ADDR                    0x02   // Rahmenpulsgenerator IQL, module bus address, FG 450.681
#define MOD_RPG_HLI_ADDR                    0x03   // Rahmenpulsgenerator HLI, module bus address, FG 450.681
#define MOD_RPG_HSI_ADDR                    0x04   // Rahmenpulsgenerator HSI, module bus address, FG 450.681
#define MOD_RPG_XXX_GATELENHI_REG           0x42   // Rahmenpulsgeneratoren, register for gate puls length counter, hi word
#define MOD_RPG_XXX_GATELENLO_REG           0x40   // Rahmenpulsgeneratoren, register for gate puls length counter, lo word (reading this register resets register values)
#define MOD_RPG_XXX_ENABLE_REG              0x10   // Rahmenpulsgeneratoren, registe for 'enable'

// interrupts
//#define UNICHOP_GW_MSI_LATE_EVENT         0x1
//#define UNICHOP_GW_MSI_STATE_CHANGED      0x2
//#define UNICHOP_GW_MSI_EVENT              0x3  // ored with (mil_event_number << 8)

// constants
#define UNICHOP_MILMODULE_ACCESST         100000   // time required to access (read/write) HW via MIL<->Modulebus  [ns]
#define UNICHOP_U16_NODATA                0x7fff   // no data available
#define UNICHOP_U16_INVALID               0xffff   // data invalid
#define MOD_RPG_XXX_ENABLE_TRUE           0x00f5   // enable word for RPG enable register
#define MOD_RPG_XXX_ENABLE_FALSE          0x0000   // disable word for RPG enable register


// default values

// ****************************************************************************************
// DP RAM
// ****************************************************************************************

// offsets
// set values
#define UNICHOP_SHARED_SET_MIL_DEV           (COMMON_SHARED_END                   + _32b_SIZE_)  // MIL device for sending MIL messages; 0: MIL Piggy; 1..: SIO in slot 1..

// get values
#define UNICHOP_SHARED_GET_N_MIL_SND_HI      (UNICHOP_SHARED_SET_MIL_DEV          + _32b_SIZE_)  // number of sent MIL telegrams, high word
#define UNICHOP_SHARED_GET_N_MIL_SND_LO      (UNICHOP_SHARED_GET_N_MIL_SND_HI     + _32b_SIZE_)  // number of sent MIL telegrams, low word
#define UNICHOP_SHARED_GET_N_MIL_SND_ERR     (UNICHOP_SHARED_GET_N_MIL_SND_LO     + _32b_SIZE_)  // number of failed MIL writes
#define UNICHOP_SHARED_GET_N_EVTS_REC_HI     (UNICHOP_SHARED_GET_N_MIL_SND_ERR    + _32b_SIZE_)  // number of received timing messages with Strahlweg data, high word
#define UNICHOP_SHARED_GET_N_EVTS_REC_LO     (UNICHOP_SHARED_GET_N_EVTS_REC_HI    + _32b_SIZE_)  // number of received timing messages with Strahlweg data, low word

// diagnosis: end of used shared memory
#define UNICHOP_SHARED_END                   (UNICHOP_SHARED_GET_N_EVTS_REC_LO    + _32b_SIZE_) 

#endif
