#ifndef _CARPEDM_H_
#define _CARPEDM_H_

#define SDB_VENDOR_GSI      0x0000000000000651ULL
#define SDB_DEVICE_LM32_RAM 0x54111351

#include <stdio.h>
#include <iostream>
#include <string>
#include <inttypes.h>
#include <boost/graph/graphviz.hpp>
#include <etherbone.h>
#include "graph.h"
#include "common.h"
#include "hashmap.h"
#include "memunit.h"

class MiniCommand;

using namespace etherbone;


class CarpeDM {

private:
  static const unsigned char deadbeef[4];
  static const std::string needle;

protected:

  std::string ebdevname;
  std::string outputfilename;
  
  std::string  inputfilename;

  std::vector<MemUnit> vM;
  std::vector<Graph>  vUp;

  Socket ebs;
  Device ebd;  
  std::vector<struct sdb_device> myDevs;  

  int cpuQty = -1;
  HashMap hm;
  bool verbose = false;
  std::ostream& sLog;
  std::ostream& sErr;

  int   ftmRamWrite(Device& dev, vAdr va, vBuf& vb);
  vBuf  ftmRamRead(Device& dev, vAdr va);

  boost::dynamic_properties createParser(Graph& g);

public:
  CarpeDM() : sLog(std::cout), sErr(std::cerr)  {} 
  CarpeDM(std::ostream& sLog) : sLog(sLog), sErr(std::cerr)  {} 
  CarpeDM(std::ostream& sLog, std::ostream& sErr) : sLog(sLog), sErr(sErr)  {}
  ~CarpeDM() {};

  bool  connect(const std::string& en);

  bool  disconnect();
  void  addDotToDict(const std::string& fn);

  void  clearDict();
  
  Graph& parseUpDot(const std::string& fn, Graph& g);

  bool  prepareUploadToCpu(Graph& g, uint8_t cpuIdx);

  int   upload(uint8_t cpuIdx);

     //TODO NC analysis

    //TODO assign to CPUs/threads

  int downloadAndParse(uint8_t cpuIdx);
  void writeDownDot(const std::string& fn, uint8_t cpuIdx) { writeDownDot( fn, vM[cpuIdx]); }
  void writeDownDot(const std::string& fn, MemUnit& m);

  void verboseOn()  {verbose = true;}
  void verboseOff() {verbose = false;}
  bool isVerbose()  const {return verbose;}
  int getCpuQty()   const {return cpuQty;}

  Graph& getUpGraph(uint8_t cpuIdx)   {return vM.at(cpuIdx).getUpGraph();}
  Graph& getDownGraph(uint8_t cpuIdx) {return vM.at(cpuIdx).getDownGraph();}

  

  vAdr getCmdWrAdrs(uint8_t cpuIdx, const std::string& targetName, uint8_t);
  vBuf getCmdData(uint8_t cpuIdx, const std::string& targetName, uint8_t prio, mc_ptr m);
  

  bool isKnown(const uint32_t hash)     const {return hm.contains(hash);}
  bool isKnown(const std::string& name) const {return hm.contains(name);}

  void showUp(uint8_t cpuIdx) {MemUnit& m = vM.at(cpuIdx);  m.showUp("Upload Table", "upload_dict.txt");}
  void showDown(uint8_t cpuIdx) {MemUnit& m = vM.at(cpuIdx);  m.showDown("Download Table", "download_dict.txt");}
};

#endif