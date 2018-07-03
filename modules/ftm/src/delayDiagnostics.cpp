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
    hdr.timeMinNegUDts    = getDmWrTime();
    hdr.stallObservIntvl  = 250000;

    for(int i = 0; i<cpuQty; i++) {
      // Simulation, fill with dummies
      hdr.sdr.push_back(StallDelayReport());
      hdr.sdr[i].stallStreakMax     = 10*i;
      hdr.sdr[i].stallStreakCurrent = 1*i;
      hdr.sdr[i].stallStreakMaxUDts = getDmWrTime();
    }
  } else {
    
    uint32_t devAdr = diagDevs[0].sdb_component.addr_first;
    vAdr vRa; vBuf rb; uint8_t* b;
    const uint32_t base = DM_DIAG_ENABLE_RW;
    for(uint32_t a = devAdr + DM_DIAG_ENABLE_RW; a <= devAdr + DM_DIAG_STALL_OBSERVATION_INTERVAL_RW; a += _32b_SIZE_) vRa.push_back(a);
    rb = ebReadCycle(ebd, vRa);
    b = (uint8_t*)&rb[0];

    hdr.enabled           = writeBeBytesToLeNumber<uint32_t>(b + DM_DIAG_ENABLE_RW - base);
    hdr.timeObservIntvl   = writeBeBytesToLeNumber<uint64_t>(b + DM_DIAG_TIME_OBSERVATION_INTERVAL_RW_0 - base); 
    hdr.timeMaxPosDif     = writeBeBytesToLeNumber<int64_t>(b + DM_DIAG_TIME_DIF_POS_GET_0 - base);
    hdr.timeMaxPosUDts    = writeBeBytesToLeNumber<uint64_t>(b + DM_DIAG_TIME_DIF_POS_TS_GET_0 - base);
    hdr.timeMinNegDif     = writeBeBytesToLeNumber<int64_t>(b + DM_DIAG_TIME_DIF_NEG_GET_0 - base);
    hdr.timeMinNegUDts    = writeBeBytesToLeNumber<uint64_t>(b + DM_DIAG_TIME_DIF_NEG_TS_GET_0 - base);
    hdr.stallObservIntvl  = writeBeBytesToLeNumber<uint32_t>(b + DM_DIAG_STALL_OBSERVATION_INTERVAL_RW - base);

    for(int i = 0; i<cpuQty; i++) {
      ebWriteWord(ebd, devAdr + DM_DIAG_STALL_STAT_SELECT_RW, i);

      vAdr vRa; vBuf rb; uint8_t* b;
      const uint32_t base = DM_DIAG_STALL_STREAK_MAX_GET;


      for(uint32_t a = devAdr + DM_DIAG_STALL_STREAK_MAX_GET; a <= devAdr + DM_DIAG_STALL_MAX_TS_GET_1; a += _32b_SIZE_) vRa.push_back(a);
      rb = ebReadCycle(ebd, vRa);
      b = (uint8_t*)&rb[0];
      // Simulation, fill with dummies
      hdr.sdr.push_back(StallDelayReport());
      hdr.sdr[i].stallStreakMax     = writeBeBytesToLeNumber<uint32_t>(b + DM_DIAG_STALL_STREAK_MAX_GET - base); 
      hdr.sdr[i].stallStreakCurrent = writeBeBytesToLeNumber<uint32_t>(b + DM_DIAG_STALL_CNT_GET        - base); 
      hdr.sdr[i].stallStreakMaxUDts = writeBeBytesToLeNumber<uint64_t>(b + DM_DIAG_STALL_MAX_TS_GET_0   - base);
    }

    


  }

  return hdr;  

}

void CarpeDM::clearHwDiagnostics() {
  uint32_t devAdr = diagDevs[0].sdb_component.addr_first;
  ebWriteWord(ebd, devAdr + DM_DIAG_RESET_OWR, 1);
}

void CarpeDM::startStopHwDiagnostics(bool enable) {
  uint32_t devAdr = diagDevs[0].sdb_component.addr_first;
  ebWriteWord(ebd, devAdr + DM_DIAG_STALL_OBSERVATION_INTERVAL_RW, (uint32_t)enable);
}

void CarpeDM::configHwDiagnostics(uint64_t timeIntvl, uint32_t stallIntvl) {
  uint32_t devAdr = diagDevs[0].sdb_component.addr_first;

  //quick n dirty
  write64b(devAdr + DM_DIAG_TIME_OBSERVATION_INTERVAL_RW_0,   timeIntvl);
  ebWriteWord(ebd, devAdr + DM_DIAG_STALL_OBSERVATION_INTERVAL_RW, stallIntvl);

}  