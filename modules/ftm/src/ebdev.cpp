#include "ebebd.h"

int EbDev::writeCycle(vAdr va, vBuf& vb, vBl vcs)
{

  //eb_status_t status;
  //FIXME What about MTU? What about returned eb status ??

  if (debug) sLog << "Starting Write Cycle" << std::endl;
  Cycle cyc;
  eb_data_t veb[va.size()];

  for(int i = 0; i < (va.end()-va.begin()); i++) {
   uint32_t data = vb[i*4 + 0] << 24 | vb[i*4 + 1] << 16 | vb[i*4 + 2] << 8 | vb[i*4 + 3];
   veb[i] = (eb_data_t)data;
  }
  try {
    cyc.open(dev);
    for(int i = 0; i < (va.end()-va.begin()); i++) {
    if (i && vcs.at(i)) {
      cyc.close();
      if (debug) sLog << "Close and open next Write Cycle" << std::endl;
      cyc.open(dev);
    }

    if (debug) sLog << " Writing @ 0x" << std::hex << std::setfill('0') << std::setw(8) << va[i] << " : 0x" << std::hex << std::setfill('0') << std::setw(8) << veb[i] << std::endl;
    cyc.write(va[i], EB_BIG_ENDIAN | EB_DATA32, veb[i]);

    }
    cyc.close();
  } catch (etherbone::exception_t const& ex) {
    throw std::runtime_error("Etherbone " + std::string(ex.method) + " returned " + std::string(eb_status(ex.status)) + "\n" );
  }

   return 0;
}


int   EbDev::writeCycle(vAdr va, vBuf& vb) {
  vBl vcs = leadingOne(va.size());
  EbWrapper::writeCycle(ebd, va, vb, vcs);
  return  writeCycle(ebd, va, vb, vcs);
}



vBuf EbDev::readCycle(vAdr va, vBl vcs)
{
  //FIXME What about MTU? What about returned eb status ??

  Cycle cyc;
  eb_data_t veb[va.size()];
  vBuf ret = vBuf(va.size() * 4);
  if (debug) sLog << "Starting Read Cycle" << std::endl;
  //sLog << "Got Adr Vec with " << va.size() << " Adrs" << std::endl;

  try {
    cyc.open(dev);
    for(int i = 0; i < (va.end()-va.begin()); i++) {
    //FIXME dirty break into cycles
    if (i && vcs.at(i)) {
      cyc.close();
      if (debug) sLog << "Close and open next Read Cycle" << std::endl;
      cyc.open(dev);
    }
    if (debug) sLog << " Reading @ 0x" << std::hex << std::setfill('0') << std::setw(8) << va[i] << std::endl;
    cyc.read(va[i], EB_BIG_ENDIAN | EB_DATA32, (eb_data_t*)&veb[i]);
    }
    cyc.close();

  } catch (etherbone::exception_t const& ex) {
    throw std::runtime_error("Etherbone " + std::string(ex.method) + " returned " + std::string(eb_status(ex.status)) + "\n" );
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

vBuf EbDev::readCycle(vAdr va) {
  return  readCycle(ebd, va, leadingOne(va.size()));
}

int EbDev::writeWord(uint32_t adr, uint32_t data)
{
  vAdr vA({adr});
  vBuf vD;
  writeLeNumberToBeBytes(vD, data);
  vBl vcs = leadingOne(vA.size());
  EbWrapper::writeCycle(ebd, vA, vD, vcs);
  return writeCycle(ebd, vA, vD, vcs);
}

uint32_t EbDev::readWord(uint32_t adr)
{
  vAdr vA({adr});
  vBuf vD = readCycle(ebd, vA);

  return writeBeBytesToLeNumber<uint32_t>(vD);
}

 //Reads and returns a 64 bit word from DM
uint64_t EbDev::read64b(uint32_t startAdr) {
  vAdr vA({startAdr + 0, startAdr + _32b_SIZE_});
  vBuf vD = readCycle(ebd, vA);
  return writeBeBytesToLeNumber<uint64_t>(vD);
}

int EbDev::write64b(uint32_t startAdr, uint64_t d) {
  vBuf vD;
  writeLeNumberToBeBytes(vD, d);
  vAdr vA({startAdr + 0, startAdr + _32b_SIZE_});
  vBl vcs = leadingOne(vA.size());
  EbWrapper::writeCycle(ebd, vA, vD, vcs);

  return writeCycle(ebd, vA, vD, vcs);

}


bool EbDev::connect(const std::string& en) {
    bool  ret = false;
    uint8_t mappedIdx = 0;
    int expVersion = parseFwVersionString(EXP_VER), foundVersion;
    int foundVersionMax = -1;

    if (expVersion <= 0) {throw std::runtime_error("Bad required minimum firmware version string received from Makefile"); return false;}

    atUp.clear();
    atUp.removeMemories();
    gUp.clear();
    atDown.clear();
    atDown.removeMemories();
    gDown.clear();
    cpuIdxMap.clear();
    cpuDevs.clear();

    vFw.clear();

    if(verbose) sLog << "Connecting to " << en << "... ";
    try {
      ebs.open(0, EB_DATAX|EB_ADDRX);
      ebd.open(ebs, ebdevname.c_str(), EB_DATAX|EB_ADDRX, 3);
      ebd.sdb_find_by_identity(CluTime::vendID, CluTime::devID, cluTimeDevs);
      if (cluTimeDevs.size() < 1) throw std::runtime_error("Could not find Cluster Time Module on DM (needed for WR time). Something is wrong\n");

      ebd.sdb_find_by_identity(SDB_VENDOR_GSI,SDB_DEVICE_DIAG, diagDevs);

      ebd.sdb_find_by_identity(SDB_VENDOR_GSI,SDB_DEVICE_LM32_RAM, cpuDevs);
      if (cpuDevs.size() >= 1) {
        cpuQty = cpuDevs.size();


        for(int cpuIdx = 0; cpuIdx< cpuQty; cpuIdx++) {
          //only create MemUnits for valid DM CPUs, generate Mapping so we can still use the cpuIdx supplied by User
          const std::string fwIdROM = getFwIdROM(cpuIdx);
          foundVersion = getFwVersion(fwIdROM);
          foundVersionMax = foundVersionMax < foundVersion ? foundVersion : foundVersionMax;
          vFw.push_back(foundVersion);
          int expVersionMin = expVersion;
          int expVersionMax = (expVersion / (int)FwId::VERSION_MAJOR_MUL) * (int)FwId::VERSION_MAJOR_MUL
                             + 99 * (int)FwId::VERSION_MINOR_MUL
                             + 99 * (int)FwId::VERSION_REVISION_MUL;

          if ( (foundVersion >= expVersionMin) && (foundVersion <= expVersionMax) ) {
            //FIXME check for consequent use of cpu index map!!! I'm sure there'll be absolute chaos throughout the lib if CPUs indices were not continuous
            cpuIdxMap[cpuIdx]    = mappedIdx;

            uint32_t extBaseAdr   = cpuDevs[cpuIdx].sdb_component.addr_first;
            uint32_t intBaseAdr   = getIntBaseAdr(fwIdROM);
            uint32_t peerBaseAdr  = WORLD_BASE_ADR + extBaseAdr;
            uint32_t rawSize      = cpuDevs[cpuIdx].sdb_component.addr_last - cpuDevs[cpuIdx].sdb_component.addr_first;
            uint32_t sharedOffs   = getSharedOffs(fwIdROM);
            uint32_t space        = getSharedSize(fwIdROM) - _SHCTL_END_;

              atUp.addMemory(cpuIdx, extBaseAdr, intBaseAdr, peerBaseAdr, sharedOffs, space, rawSize );
            atDown.addMemory(cpuIdx, extBaseAdr, intBaseAdr, peerBaseAdr, sharedOffs, space, rawSize );
            mappedIdx++;
          }
          /*
          sLog << "#" << (int)cpuIdx << " Shared Offset 0x" << std::hex <<  atDown.getMemories()[cpuIdx].sharedOffs << std::endl;
          sLog << "#" << (int)cpuIdx << " BmpSize 0x" << std::hex <<  atDown.getMemories()[cpuIdx].bmpSize << std::endl;
          sLog << "#" << (int)cpuIdx << " SHCTL 0x" << std::hex <<  _SHCTL_END_ << std::endl;
          sLog << "#" << (int)cpuIdx << " Start Offset 0x" << std::hex <<  atDown.getMemories()[cpuIdx].startOffs << std::endl;
          */
        }
        ret = true;
      }
    } catch (etherbone::exception_t const& ex) {
      throw std::runtime_error("Etherbone " + std::string(ex.method) + " returned " + std::string(eb_status(ex.status)) + "\n" );
    } catch(...) {
      throw;// std::runtime_error("Could not find CPUs running valid DM Firmware\n" );
    }

    if(verbose) {
      sLog << " Done."  << std::endl << "Found " << ebd.getCpuQty() << " Cores, " << cpuIdxMap.size() << " of them run a valid DM firmware." << std::endl;
    }
    std::string fwCause = foundVersionMax == -1 ? "" : "Requires FW v" + createFwVersionString(expVersion) + ", found " + createFwVersionString(foundVersionMax);
    if (cpuIdxMap.size() == 0) {throw std::runtime_error("No CPUs running a valid DM firmware found. " + fwCause);}


    return ret;

  }

  bool EbDev::disconnect() {
    bool ret = false;

    if(verbose) sLog << "Disconnecting ... ";
    try {
      ebd.close();
      ebs.close();
      cpuQty = -1;
      ret = true;
    } catch (etherbone::exception_t const& ex) {
      throw std::runtime_error("Etherbone " + std::string(ex.method) + " returned " + std::string(eb_status(ex.status)) + "\n" );
      //TODO report why we could not disconnect
    }
    
    if(verbose) sLog << " Done" << std::endl;
    
    return ret;
  }



 

  uint64_t EbDev::getDmWrTime() const {
    /* get time from Cluster Time Module (ECA Time with or wo ECA) */
    uint64_t wrTime;

    eb_data_t    nsHi, nsLo;
    Cycle cyc;
    uint32_t cluTimeAddr = cluTimeDevs[0].sdb_component.addr_first;

      try {
        cyc.open(dev);
        cyc.read( cluTimeAddr + CluTime::timeHiW, EB_BIG_ENDIAN|EB_DATA32, &nsHi);
        cyc.read( cluTimeAddr + CluTime::timeLoW, EB_BIG_ENDIAN|EB_DATA32, &nsLo);
        cyc.close();
      } catch (etherbone::exception_t const& ex) {
        throw std::runtime_error("Etherbone " + std::string(ex.method) + " returned " + std::string(eb_status(ex.status)) + "\n" );
      }


    /* time */
    wrTime = (uint64_t)nsHi << 32;
    wrTime = wrTime + (uint64_t)nsLo;

    return wrTime;
  }

void EbDev::showCpuList() {
  int expVersionMin = parseFwVersionString(EXP_VER);
  int expVersionMax = (expVersionMin / (int)FwId::VERSION_MAJOR_MUL) * (int)FwId::VERSION_MAJOR_MUL
                   + 99 * (int)FwId::VERSION_MINOR_MUL
                   + 99 * (int)FwId::VERSION_REVISION_MUL;

  sLog << std::endl << std::setfill(' ') << std::setw(5) << "CPU" << std::setfill(' ') << std::setw(11) << "FW found"
       << std::setfill(' ') << std::setw(11) << "Min" << std::setw(11) << "Max" << std::setw(11) << "Space" << std::setw(11) << "Free" << std::endl;
  for (int x = 0; x < cpuQty; x++) {

    sLog << std::dec << std::setfill(' ') << std::setw(5) << x << std::setfill(' ') << std::setw(11) << createFwVersionString(vFw[x])
                                                                       << std::setfill(' ') << std::setw(11) << createFwVersionString(expVersionMin)
                                                                       << std::setfill(' ') << std::setw(11) << createFwVersionString(expVersionMax);

    sLog << std::dec << std::setfill(' ') << std::setw(11) << atDown.getTotalSpace(x) << std::setw(10) << atDown.getFreeSpace(x) * 100 / atDown.getTotalSpace(x) << "%";
    sLog << std::endl;
  }

}