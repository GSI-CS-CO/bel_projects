/** @file        dm_diag_regs.h
  * DesignUnit   dm_diag
  * @author      M. Kreider <m.kreider@gsi.de>
  * @date        28/06/2018
  * @version     0.0.1
  * @copyright   2018 GSI Helmholtz Centre for Heavy Ion Research GmbH
  *
  * @brief       Register map for Wishbone interface of VHDL entity <dm_diag_auto>
  */

#ifndef _DM_DIAG_H_
#define _DM_DIAG_H_

  #define DM_DIAG_SDB_VENDOR_ID 0x00000651
  #define DM_DIAG_SDB_DEVICE_ID 0x18060200

  #define DM_DIAG_RESET_OWR                      0x000 //wo,  1 b, Resets/clears the diagnostic
  #define DM_DIAG_ENABLE_RW                      0x004 //rw,  1 b, Enables/disables update. Default is enabled
  #define DM_DIAG_TIME_OBSERVATION_INTERVAL_RW_0 0x008 //rw, 32 b, TAI time observation interval in ns
  #define DM_DIAG_TIME_OBSERVATION_INTERVAL_RW_1 0x00c //rw, 32 b, TAI time observation interval in ns
  #define DM_DIAG_TIME_DIF_POS_GET_0             0x010 //ro, 32 b, Observed max pos. ECA time difference in ns between ref clock ticks
  #define DM_DIAG_TIME_DIF_POS_GET_1             0x014 //ro, 32 b, Observed max pos. ECA time difference in ns between ref clock ticks
  #define DM_DIAG_TIME_DIF_POS_TS_GET_0          0x018 //ro, 32 b, (approximate) timestamp of last pos dif update
  #define DM_DIAG_TIME_DIF_POS_TS_GET_1          0x01c //ro, 32 b, (approximate) timestamp of last pos dif update
  #define DM_DIAG_TIME_DIF_NEG_GET_0             0x020 //ro, 32 b, Observed max neg. ECA time difference in ns between ref clock ticks
  #define DM_DIAG_TIME_DIF_NEG_GET_1             0x024 //ro, 32 b, Observed max neg. ECA time difference in ns between ref clock ticks
  #define DM_DIAG_TIME_DIF_NEG_TS_GET_0          0x028 //ro, 32 b, (approximate) timestamp of last neg dif update
  #define DM_DIAG_TIME_DIF_NEG_TS_GET_1          0x02c //ro, 32 b, (approximate) timestamp of last neg dif update
  #define DM_DIAG_STALL_OBSERVATION_INTERVAL_RW  0x030 //rw, 32 b, Stall observation interval in cycles
  #define DM_DIAG_STALL_STAT_SELECT_RW           0x034 //rw,  8 b, Page selector register for Stall observers
  #define DM_DIAG_STALL_STREAK_MAX_GET           0x100 //ro, 32 b, Observed max continuous stall in cycles
  #define DM_DIAG_STALL_CNT_GET                  0x104 //ro, 32 b, Stall time within observation interval in cycles
  #define DM_DIAG_STALL_MAX_TS_GET_0             0x108 //ro, 32 b, Timestamp of last max update
  #define DM_DIAG_STALL_MAX_TS_GET_1             0x10c //ro, 32 b, Timestamp of last max update

#endif
