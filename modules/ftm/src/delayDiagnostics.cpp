#include <stdio.h>
#include <iostream>
#include <string>
#include <inttypes.h>
#include "common.h"

#include "carpeDM.h"
#include "dm_diag_regs.h"

HwDelayReport& CarpeDM::getHwDelayReport(HwDelayReport& hdr) {
  //create delayreport
  if (sim) {
    hdr.timeObservIntvl   = 1000000;
    hdr.timeMaxPosDif     = 0;
    hdr.timeMaxPosUDts    = 0;
    hdr.timeMinNegDif     = 0;
    hdr.timeMinNegUDts    = ebd.getDmWrTime();
    hdr.stallObservIntvl  = 250000;

    for(int i = 0; i<ebd.getCpuQty(); i++) {
      // Simulation, fill with dummies
      hdr.sdr.push_back(StallDelayReport());
      hdr.sdr[i].stallStreakMax     = 10*i;
      hdr.sdr[i].stallStreakCurrent = 1*i;
      hdr.sdr[i].stallStreakMaxUDts = ebd.getDmWrTime();
    }
  } else {

    uint32_t devAdr = ebd.getDiagDevAdr();
    vAdr vRa; vBuf rb; uint8_t* b;
    const uint32_t base = DM_DIAG_ENABLE_RW;
    for(uint32_t a = devAdr + DM_DIAG_ENABLE_RW; a <= devAdr + DM_DIAG_STALL_OBSERVATION_INTERVAL_RW; a += _32b_SIZE_) vRa.push_back(a);
    rb = ebd.readCycle(vRa);
    b = (uint8_t*)&rb[0];

    hdr.enabled           = writeBeBytesToLeNumber<uint32_t>(b + DM_DIAG_ENABLE_RW - base);
    hdr.timeObservIntvl   = writeBeBytesToLeNumber<uint64_t>(b + DM_DIAG_TIME_OBSERVATION_INTERVAL_RW_1 - base) + 8;
    hdr.timeMaxPosDif     = writeBeBytesToLeNumber<int64_t>(b + DM_DIAG_TIME_DIF_POS_GET_1 - base);
    hdr.timeMaxPosUDts    = writeBeBytesToLeNumber<uint64_t>(b + DM_DIAG_TIME_DIF_POS_TS_GET_1 - base);
    hdr.timeMinNegDif     = writeBeBytesToLeNumber<int64_t>(b + DM_DIAG_TIME_DIF_NEG_GET_1 - base);
    hdr.timeMinNegUDts    = writeBeBytesToLeNumber<uint64_t>(b + DM_DIAG_TIME_DIF_NEG_TS_GET_1 - base);
    hdr.stallObservIntvl  = writeBeBytesToLeNumber<uint32_t>(b + DM_DIAG_STALL_OBSERVATION_INTERVAL_RW - base);

    for(int i = 0; i<ebd.getCpuQty(); i++) {
      ebd.write32b(devAdr + DM_DIAG_STALL_STAT_SELECT_RW, i);

      vAdr vRa; vBuf rb; uint8_t* b;
      const uint32_t base = DM_DIAG_STALL_STREAK_MAX_GET;


      for(uint32_t a = devAdr + DM_DIAG_STALL_STREAK_MAX_GET; a <= devAdr + DM_DIAG_STALL_MAX_TS_GET_1; a += _32b_SIZE_) vRa.push_back(a);
      rb = ebd.readCycle(vRa);
      b = (uint8_t*)&rb[0];
      // Simulation, fill with dummies
      hdr.sdr.push_back(StallDelayReport());
      hdr.sdr[i].stallStreakMax     = writeBeBytesToLeNumber<uint32_t>(b + DM_DIAG_STALL_STREAK_MAX_GET - base);
      hdr.sdr[i].stallStreakCurrent = writeBeBytesToLeNumber<uint32_t>(b + DM_DIAG_STALL_CNT_GET        - base);
      hdr.sdr[i].stallStreakMaxUDts = writeBeBytesToLeNumber<uint64_t>(b + DM_DIAG_STALL_MAX_TS_GET_1   - base);
    }




  }

  return hdr;

}

void CarpeDM::clearHwDiagnostics() {
  uint32_t devAdr = ebd.getDiagDevAdr();
  ebd.write32b(devAdr + DM_DIAG_RESET_OWR, 1);
}

void CarpeDM::startStopHwDiagnostics(bool enable) {
  uint32_t devAdr = ebd.getDiagDevAdr();
  ebd.write32b(devAdr + DM_DIAG_STALL_OBSERVATION_INTERVAL_RW, (uint32_t)enable);
}

void CarpeDM::configHwDiagnostics(uint64_t timeIntvl, uint32_t stallIntvl) {
  uint32_t devAdr = ebd.getDiagDevAdr();
  timeIntvl = timeIntvl < 8 ? 0 : timeIntvl - 8; // hardware has 1 cycle to latch, so there is always intvl of 8ns + x
  //quick n dirty
  ebd.write64b(devAdr + DM_DIAG_TIME_OBSERVATION_INTERVAL_RW_1,   timeIntvl);
  ebd.write32b(devAdr + DM_DIAG_STALL_OBSERVATION_INTERVAL_RW, stallIntvl);

}

void CarpeDM::configFwDiagnostics(uint64_t warnThrshld) {
  vEbwrs ew;
  uint8_t b[_TS_SIZE_];
  writeLeNumberToBeBytes<uint64_t>(b, warnThrshld);


  for(int cpuIdx = 0; cpuIdx<ebd.getCpuQty(); cpuIdx++) {
    const uint32_t base = atDown.getMemories()[cpuIdx].extBaseAdr + atDown.getMemories()[cpuIdx].sharedOffs + SHCTL_DIAG;

    ew.va.push_back(base + T_DIAG_DIF_WTH + 0);
    ew.va.push_back(base + T_DIAG_DIF_WTH + _32b_SIZE_);
    ew.vb.insert( ew.vb.end(), b, b + _TS_SIZE_ );
    ew.vcs  += leadingOne(2);


  }

  ebd.writeCycle(ew.va, ew.vb, ew.vcs);

}