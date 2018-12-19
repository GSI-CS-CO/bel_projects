#include <boost/algorithm/string.hpp>
#include "ebwrapper.h"

int EbWrapper::writeCycle(const vAdr& va, const vBuf& vb, const vBl& vcs) const {

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
    cyc.open(ebd);
    for(int i = 0; i < (va.end()-va.begin()); i++) {
    if (i && vcs.at(i)) {
      cyc.close();
      if (debug) sLog << "Close and open next Write Cycle" << std::endl;
      cyc.open(ebd);
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


int EbWrapper::writeCycle(const vAdr& va, const vBuf& vb) const {
  vBl vcs = leadingOne(va.size());
  checkWriteCycle(va, vb, vcs);
  return  writeCycle(va, vb, vcs);
}



vBuf EbWrapper::readCycle(const vAdr& va, const vBl& vcs) const {
  //FIXME What about MTU? What about returned eb status ??

  Cycle cyc;
  eb_data_t veb[va.size()];
  vBuf ret = vBuf(va.size() * 4);
  if (debug) sLog << "Starting Read Cycle" << std::endl;
  //sLog << "Got Adr Vec with " << va.size() << " Adrs" << std::endl;

  try {
    cyc.open(ebd);
    for(int i = 0; i < (va.end()-va.begin()); i++) {
    //FIXME dirty break into cycles
    if (i && vcs.at(i)) {
      cyc.close();
      if (debug) sLog << "Close and open next Read Cycle" << std::endl;
      cyc.open(ebd);
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

vBuf EbWrapper::readCycle(const vAdr& va) const {
  return  readCycle(va, leadingOne(va.size()));
}

int EbWrapper::write32b(uint32_t adr, uint32_t data) const {
  vAdr va({adr});
  vBuf vb;
  writeLeNumberToBeBytes(vb, data);
  vBl vcs = leadingOne(va.size());
  checkWriteCycle(va, vb, vcs);
  return writeCycle(va, vb, vcs);
}

uint32_t EbWrapper::read32b(uint32_t adr) const {
  vAdr va({adr});
  vBl vcs = leadingOne(va.size());
  checkReadCycle(va, vcs);
  vBuf vb = readCycle(va);
  return writeBeBytesToLeNumber<uint32_t>(vb);
}

 //Reads and returns a 64 bit word from DM
uint64_t EbWrapper::read64b(uint32_t startAdr) const {
  vAdr va({startAdr + 0, startAdr + _32b_SIZE_});
  vBuf vb = readCycle(va);
  return writeBeBytesToLeNumber<uint64_t>(vb);
}

int EbWrapper::write64b(uint32_t startAdr, uint64_t d) const {
  vBuf vb;
  writeLeNumberToBeBytes(vb, d);
  vAdr va({startAdr + 0, startAdr + _32b_SIZE_});
  vBl vcs = leadingOne(va.size());
  checkWriteCycle(va, vb, vcs);

  return writeCycle(va, vb, vcs);

}


bool EbWrapper::connect(const std::string& en, AllocTable& atUp, AllocTable& atDown) {
  
    bool  ret = false;
    ebdevname = en;
    
    uint8_t mappedIdx = 0;
    int foundVersionMax = -1;
    
    cpuIdxMap.clear();
    cpuDevs.clear();
    vFoundVersion.clear();
    vFwIdROM.clear();

    if(verbose) sLog << "Connecting to " << ebdevname << "... ";
   
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
          
          vFwIdROM.push_back(getFwIdROM(cpuIdx));
          
          int foundVersion = getFwVersion(vFwIdROM[cpuIdx]);
          
          foundVersionMax = foundVersionMax < foundVersion ? foundVersion : foundVersionMax;
          vFoundVersion.push_back(foundVersion);
          

          if ( (foundVersion >= expVersionMin) && (foundVersion <= expVersionMax) ) {
            //FIXME check for consequent use of cpu index map!!! I'm sure there'll be absolute chaos throughout the lib if CPUs indices were not continuous
            cpuIdxMap[cpuIdx]    = mappedIdx;

            uint32_t extBaseAdr   = cpuDevs[cpuIdx].sdb_component.addr_first;
            uint32_t intBaseAdr   = getIntBaseAdr(vFwIdROM[cpuIdx]);
            uint32_t peerBaseAdr  = WORLD_BASE_ADR + extBaseAdr;
            uint32_t rawSize      = cpuDevs[cpuIdx].sdb_component.addr_last - cpuDevs[cpuIdx].sdb_component.addr_first;
            uint32_t sharedOffs   = getSharedOffs(vFwIdROM[cpuIdx]);
            uint32_t space        = getSharedSize(vFwIdROM[cpuIdx]) - _SHCTL_END_;

              atUp.addMemory(cpuIdx, extBaseAdr, intBaseAdr, peerBaseAdr, sharedOffs, space, rawSize );
            atDown.addMemory(cpuIdx, extBaseAdr, intBaseAdr, peerBaseAdr, sharedOffs, space, rawSize );
            mappedIdx++;
          }
          
       
        }
        ret = true;
      }
      
    } catch (etherbone::exception_t const& ex) {
      throw std::runtime_error("Etherbone " + std::string(ex.method) + " returned " + std::string(eb_status(ex.status)) + "\n" );
    } catch(...) {
      throw;// std::runtime_error("Could not find CPUs running valid DM Firmware\n" );
    }

    if(verbose) {
      sLog << " Done."  << std::endl << "Found " << cpuQty << " Cores, " << cpuIdxMap.size() << " of them run a valid DM firmware." << std::endl;
    }
    std::string fwCause = foundVersionMax == -1 ? "" : "Requires FW v" + createFwVersionString(expVersionMin) + ", found " + createFwVersionString(foundVersionMax);
    if (cpuIdxMap.size() == 0) {throw std::runtime_error("No CPUs running a valid DM firmware found. " + fwCause);}


    return ret;

  }

  bool EbWrapper::disconnect() {
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


  uint64_t EbWrapper::getDmWrTime() const {
    /* get time from Cluster Time Module (ECA Time with or wo ECA) */
    uint64_t wrTime;

    eb_data_t    nsHi, nsLo;
    Cycle cyc;
    uint32_t cluTimeAddr = cluTimeDevs[0].sdb_component.addr_first;

      try {
        cyc.open(ebd);
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


    //returns firmware version as int <xxyyzz> (x Major Version, y Minor Version, z Revison; negative values for error codes)
  std::string EbWrapper::getFwIdROM(uint8_t cpuIdx) const {
    //FIXME replace with FW ID string constants
    const std::string tagMagic      = "UserLM32";
    const std::string tagProject    = "Project     : ";
    const std::string tagExpName    = "ftm";
    std::string version;
    size_t pos;
    
    const struct sdb_device& ram = cpuDevs.at(cpuIdx);
    vAdr fwIdAdr;
    //FIXME get rid of SHARED_OFFS somehow and replace with an end tag and max limit
    for (uint32_t adr = ram.sdb_component.addr_first + BUILDID_OFFS; adr < ram.sdb_component.addr_first + BUILDID_OFFS + BUILDID_SIZE; adr += 4) fwIdAdr.push_back(adr);
    
    vBuf fwIdData = readCycle(fwIdAdr);
    std::string s(fwIdData.begin(),fwIdData.end());
    
    //check for magic word
    pos = 0;
    if(s.find(tagMagic, 0) == std::string::npos) {throw std::runtime_error( "Bad Firmware Info ROM: Magic word not found\n");}
    //check for project name
    
    pos = s.find(tagProject, 0);
    if (pos == std::string::npos || (s.find(tagExpName, pos + tagProject.length()) != pos + tagProject.length())) {throw std::runtime_error( "Bad Firmware Info ROM: Not a DM project\n");}
    
    return s;
  }

   std::string EbWrapper::createFwVersionString(const int fwVer) const {

    unsigned int fwv = (unsigned int)fwVer;
    std::string ret;

    unsigned int verMaj = fwv / (unsigned int)FwId::VERSION_MAJOR_MUL; fwv %= (unsigned int)FwId::VERSION_MAJOR_MUL;
    unsigned int verMin = fwv / (unsigned int)FwId::VERSION_MINOR_MUL; fwv %= (unsigned int)FwId::VERSION_MINOR_MUL;
    unsigned int verRev = fwv;

    ret = std::to_string(verMaj) + "." + std::to_string(verMin) + "." + std::to_string(verRev);
    return ret;

  }


  int EbWrapper::parseFwVersionString(const std::string& s) const {
    int verMaj, verMin, verRev;
    std::vector<std::string> x;

    try { boost::split(x, s, boost::is_any_of(".")); } catch (...) {};
    if (x.size() != 3) {return (int)FwId::FWID_BAD_VERSION_FORMAT;}

    verMaj = std::stoi (x[(int)FwId::VERSION_MAJOR]);
    verMin = std::stoi (x[(int)FwId::VERSION_MINOR]);
    verRev = std::stoi (x[(int)FwId::VERSION_REVISION]);

    if (verMaj < 0 || verMaj > 99 || verMin < 0 || verMin > 99 || verRev < 0 || verRev > 99) {return (int)FwId::FWID_BAD_VERSION_FORMAT;}
    else {return verMaj * (int)FwId::VERSION_MAJOR_MUL + verMin * (int)FwId::VERSION_MINOR_MUL  + verRev * (int)FwId::VERSION_REVISION_MUL;}
  }

  //returns firmware version as int <xxyyzz> (x Major Version, y Minor Version, z Revison; negative values for error codes)
  int EbWrapper::getFwVersion(const std::string& fwIdROM) const {
    //FIXME replace with FW ID string constants
    //get Version string xx.yy.zz

    std::string version = readFwIdROMTag(fwIdROM, "Version     : ", 10, true);

    int ret = parseFwVersionString(version);

    return ret;
  }


  std::string EbWrapper::readFwIdROMTag(const std::string& fwIdROM, const std::string& tag, size_t maxlen, bool stopAtCr ) const {
    size_t pos, posEnd, tmp;
    std::string s = fwIdROM;

    tmp = s.find(tag, 0);
    if(tmp == std::string::npos) throw std::runtime_error( "Could not find tag <" + tag + ">in FW ID ROM\n");
    pos = tmp + tag.length();

    tmp = s.find("\n", pos);
    if( (tmp == std::string::npos) || (tmp > (pos + maxlen)) ) posEnd = (pos + maxlen);
    else posEnd = tmp;

    return s.substr(pos, posEnd - pos);
  }



   uint32_t EbWrapper::getIntBaseAdr(const std::string& fwIdROM) const {
    //FIXME replace with FW ID string constants
    //CAREFUL: Get the EXACT position. If you miss out on leading spaces, the parsed number gets truncated!
    std::string value = readFwIdROMTag(fwIdROM, "IntAdrOffs  : ", 10, true);
    //sLog << "IntAdrOffs : " << value << " parsed: 0x" << std::hex << s2u<uint32_t>(value) << std::endl;
    return s2u<uint32_t>(value);

  }

  uint32_t EbWrapper::getSharedOffs(const std::string& fwIdROM) const {
    //FIXME replace with FW ID string constants
    std::string value = readFwIdROMTag(fwIdROM, "SharedOffs  : ", 10, true);
    //sLog << "Parsing SharedOffs : " << value << " parsed: 0x" << std::hex << s2u<uint32_t>(value) << std::endl;
    return s2u<uint32_t>(value);

  }

  uint32_t EbWrapper::getSharedSize(const std::string& fwIdROM) const{
    std::string value = readFwIdROMTag(fwIdROM, "SharedSize  : ", 10, true);
    //sLog << "SharedSize : " << value << " parsed: "  << std::dec << s2u<uint32_t>(value) << std::endl;
    return s2u<uint32_t>(value);

  }

  void EbWrapper::showCpuList() const {
    sLog << std::endl << std::setfill(' ') << std::setw(5) << "CPU" << std::setfill(' ') << std::setw(11) << "FW found"
         << std::setfill(' ') << std::setw(11) << "Min" << std::setw(11) << "Max" << std::setw(11)  << std::endl;
    for (int x = 0; x < cpuQty; x++) {
  
      sLog << std::dec << std::setfill(' ') << std::setw(5) << x << std::setfill(' ') << std::setw(11) << getFwVersionString(x)
           << std::setfill(' ') << std::setw(11) << createFwVersionString(getExpVersionMin())
           << std::setfill(' ') << std::setw(11) << createFwVersionString(getExpVersionMax());
    }
  }