#include "ebsim.h"

  bool EbSim::disconnect() {


    bool ret = false;

    if(verbose) sLog << "Disconnecting ... ";
    if(verbose) sLog << " Done" << std::endl;
    
    return ret;
  }

bool EbSim::connect() {
    simRam.clear();
    simRamAdrMap.clear();
 
    ebdevname = "simDummy"; //copy to avoid mem trouble later
    uint8_t mappedIdx = 0;
    uint32_t const intBaseAdr   = 0x1000000;
    uint32_t const sharedSize   = 98304;
    uint32_t const rawSize      = 131072;
    uint32_t const sharedOffs   = 0x500;
    uint32_t const devBaseAdr   = 0x4120000;
    cpuQty = 4;

    atUp.clear();
    atUp.removeMemories();
    gUp.clear();
    atDown.clear();
    atDown.removeMemories();
    gDown.clear();
    cpuIdxMap.clear();
    cpuDevs.clear();

    sLog << "Connecting to Sim... ";
    simRam.reserve(cpuQty);

    for(int cpuIdx = 0; cpuIdx< cpuQty; cpuIdx++) {
      simRam[cpuIdx]        = new uint32_t [(rawSize + _32b_SIZE_ -1) >> 2];
      cpuIdxMap[cpuIdx]     = mappedIdx;
      uint32_t extBaseAdr   = devBaseAdr + cpuIdx * rawSize;
      simRamAdrMap[cpuIdx]  = extBaseAdr;
      uint32_t peerBaseAdr  = WORLD_BASE_ADR  + extBaseAdr;
      uint32_t space        = sharedSize - _SHCTL_END_;

      atUp.addMemory(cpuIdx, extBaseAdr, intBaseAdr, peerBaseAdr, sharedOffs, space, rawSize );
      atDown.addMemory(cpuIdx, extBaseAdr, intBaseAdr, peerBaseAdr, sharedOffs, space, rawSize );
      mappedIdx++;
    }
    sLog << "done" << std::endl;
    return true;

}



vBuf EbSim::readCycle(vAdr va)
{


  eb_data_t veb[va.size()];
  vBuf ret = vBuf(va.size() * 4);
  if (debug) sLog << "Starting Read Cycle" << std::endl;
  //sLog << "Got Adr Vec with " << va.size() << " Adrs" << std::endl;


  for(int i = 0; i < (va.end()-va.begin()); i++) {
    if (debug) sLog << " Reading @ 0x" << std::hex << std::setfill('0') << std::setw(8) << va[i] << std::endl;
    simRamRead(va[i], (eb_data_t*)&veb[i]);
  }

  //FIXME use endian functions
  for(unsigned int i = 0; i < va.size(); i++) {
    ret[i * 4]     = (uint8_t)(veb[i] >> 24);
    ret[i * 4 + 1] = (uint8_t)(veb[i] >> 16);
    ret[i * 4 + 2] = (uint8_t)(veb[i] >> 8);
    ret[i * 4 + 3] = (uint8_t)(veb[i] >> 0);
  }

  return ret;
}

void EbSim::adrTranslation (uint32_t a, uint8_t& cpu, uint32_t& arIdx) {
  //get cpu
  /*
   if (debug) sLog << "cpuQty "  << getCpuQty() << std::endl;
  for (uint8_t cpuIdx = 0; cpuIdx < getCpuQty(); cpuIdx++) {
    if (debug) sLog << "a : " << std::hex << a << ", cmpA " << simRamAdrMap[cpuIdx] << " cpu " << (int)cpuIdx << " arIdx "  << arIdx << std::endl;
    if (simRamAdrMap[cpuIdx] > a) break;
    cpu = cpuIdx;
  }
  */
  cpu = ((a >> 17) & 0x7) -1;
  arIdx = atDown.adrConv(AdrType::EXT, AdrType::MGMT, cpu, a) >> 2;
}

void EbSim::ramWrite (uint32_t a, eb_data_t d) {
  uint8_t cpu = -1;
  uint32_t arIdx;
  if (debug) sLog << "cpu : " << (int)cpu << " arIdx " << std::hex << arIdx << std::endl;
  simAdrTranslation (a, cpu, arIdx);
  simRam[cpu][arIdx] = d;
}


void EbSim::ramRead (uint32_t a, eb_data_t* d) {
  uint8_t cpu;
  uint32_t arIdx;
  simAdrTranslation (a, cpu, arIdx);
  *d = simRam[cpu][arIdx];
}

int EbSim::writeCycle(vAdr va, vBuf& vb) {
  if (debug) sLog << "Starting Write Cycle" << std::endl;
  eb_data_t veb[va.size()];

  for(int i = 0; i < (va.end()-va.begin()); i++) {
   uint32_t data = vb[i*4 + 0] << 24 | vb[i*4 + 1] << 16 | vb[i*4 + 2] << 8 | vb[i*4 + 3];
   veb[i] = (eb_data_t)data;
  }


  for(int i = 0; i < (va.end()-va.begin()); i++) {

    if (debug) sLog << " Writing @ 0x" << std::hex << std::setfill('0') << std::setw(8) << va[i] << " : 0x" << std::hex << std::setfill('0') << std::setw(8) << veb[i] << std::endl;
    simRamWrite(va[i], veb[i]);
  }


  return 0;

}


uint64_t EbSim::getDmWrTime() const {
    /* get time from Cluster Time Module (ECA Time with or wo ECA) */
    uint64_t wrTime;

      timeval ts;
      gettimeofday(&ts, NULL);

      wrTime = (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_usec * 1000ULL;

      return wrTime;
  }



void EbSim::showCpuList() {
  int expVersionMin = parseFwVersionString(EXP_VER);
  int expVersionMax = (expVersionMin / (int)FwId::VERSION_MAJOR_MUL) * (int)FwId::VERSION_MAJOR_MUL
                   + 99 * (int)FwId::VERSION_MINOR_MUL
                   + 99 * (int)FwId::VERSION_REVISION_MUL;

  sLog << std::endl << std::setfill(' ') << std::setw(5) << "CPU" << std::setfill(' ') << std::setw(11) << "FW found"
       << std::setfill(' ') << std::setw(11) << "Min" << std::setw(11) << "Max" << std::setw(11) << "Space" << std::setw(11) << "Free" << std::endl;
  for (int x = 0; x < cpuQty; x++) {

    sLog << std::dec << std::setfill(' ') << std::setw(5) << x << std::setfill(' ') << std::setw(11) << "Sim"
                                                                       << std::setfill(' ') << std::setw(11) << "Sim"
                                                                       << std::setfill(' ') << std::setw(11) << "Sim";

    sLog << std::dec << std::setfill(' ') << std::setw(11) << atDown.getTotalSpace(x) << std::setw(10) << atDown.getFreeSpace(x) * 100 / atDown.getTotalSpace(x) << "%";
    sLog << std::endl;
  }

}  