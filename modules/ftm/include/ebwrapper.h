#ifndef _EBWRAPPER_H_
#define _EBWRAPPER_H_

#include <stdio.h>
#include <iostream>
#include <string>
#include <inttypes.h>
#include <map>
#include "common.h"
#include <etherbone.h>
#include "alloctable.h"
#include "reflocation.h"
#include "ftm_common.h"

#define SDB_VENDOR_GSI      0x0000000000000651ULL
#define SDB_DEVICE_LM32_RAM 0x54111351
#define SDB_DEVICE_DIAG     0x18060200




using namespace etherbone;

class EbWrapper {

protected:
  //bool& sim;
  std::ostream& sLog;
  std::ostream& sErr;
  bool& verbose;
  bool& debug;

  Socket ebs;
  Device ebd;

  std::vector<struct sdb_device> cpuDevs;
  std::vector<struct sdb_device> cluTimeDevs;
  std::vector<struct sdb_device> diagDevs;

  std::vector<int> vFoundVersion;
  std::map<uint8_t, uint8_t> cpuIdxMap;
  uint8_t cpuQty;
  uint8_t thrQty;
  uint32_t adrLut[(SHCTL_STATUS - SHCTL_ADR_TAB + _32b_SIZE_ - 1) / _32b_SIZE_]; //alloc

  std::string ebdevname;

  vStrC vFwIdROM;

  static const int expVersionMin;
  static const int expVersionMax;
  
  void checkReadCycle (const vAdr& va, const vBl& vcs) const;

  void checkWriteCycle(const vAdr& va, const vBuf& vb, const vBl& vcs) const;
  //returns firmware version as int <xxyyzz> (x Major Version, y Minor Version, z Revison; negative values for error codes)
  int getFwVersion(const std::string& fwIdROM) const;
  std::string getFwIdROM(uint8_t cpuIdx) const;
  std::string createFwVersionString(const int fwVer) const;
  static int parseFwVersionString(const std::string& s);
  std::string parseFwIdROMTag(const std::string& fwIdROM, const std::string& tag, size_t maxlen, bool stopAtCr ) const;
  // SDB Functions

public:
  EbWrapper(std::ostream& sLog, std::ostream& sErr, bool& verbose, bool& debug) : sLog(sLog), sErr(sErr), verbose(verbose), debug(debug) {
    if (expVersionMin <= 0) {throw std::runtime_error("Bad required minimum firmware version string received from Makefile");}
  };
  ~EbWrapper() {};
  
  bool connect(const std::string& ebdevname, AllocTable& atUp, AllocTable& atDown, RefLocation& rl);
  bool disconnect(); //Close connection
  int writeCycle(const vEbwrs& ew) const;
  int writeCycle(const vAdr& va, const vBuf& vb, const vBl& vcs) const;
  int writeCycle(const vAdr& va, const vBuf& vb) const;
  vBuf readCycle(const vEbrds& er) const;
  vBuf readCycle(const vAdr& va, const vBl& vcs) const ;
  vBuf readCycle(const vAdr& va ) const ;
  uint32_t read32b(uint32_t adr) const;
  uint64_t read64b(uint32_t startAdr) const;
  int write32b(uint32_t adr, uint32_t data) const;
  int write64b(uint32_t startAdr, uint64_t data) const;
  //bool isSimulation() ;
  uint64_t getDmWrTime() const;
  bool isValidDMCpu(uint8_t cpuIdx) {return (cpuIdxMap.count(cpuIdx) > 0);}; //Check if CPU is registered as running a valid firmware
  uint8_t getCpuQty()   const {return cpuQty;} //Return number of found CPUs (not necessarily valid ones!)
  uint8_t getThrQty()   const {return thrQty;} //Return number of found Threads per CPU
  uint32_t getCtlAdr(const uint8_t& idx) const {return adrLut[idx];} //Return the corresponding  ctl address (necessary to support different number of thread-FWs)
  bool isCpuIdxValid(uint8_t cpuIdx) { if ( cpuIdxMap.find(cpuIdx) != cpuIdxMap.end() ) return true; else return false;}
  uint32_t getDiagDevAdr() {return diagDevs[0].sdb_component.addr_first;}
  int getExpVersionMin() const {return expVersionMin;} 
  int getExpVersionMax() const {return expVersionMax;}
  uint32_t parseIntBaseAdr(const std::string& fwIdROM) const; 
  uint32_t parseSharedOffs(const std::string& fwIdROM) const;
  uint32_t parseSharedSize(const std::string& fwIdROM) const;
  uint32_t parseThrQty(const std::string& fwIdROM) const;
  int readAdrLUT(uint32_t extBaseAdr, uint32_t sharedOffs, uint32_t* lut) const;

  std::string getFwVersionString(uint8_t cpuIdx) const {return createFwVersionString(vFoundVersion[cpuIdx]);}
  void showCpuList() const;



};

#endif