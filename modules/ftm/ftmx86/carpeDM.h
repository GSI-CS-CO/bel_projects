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

#define UPLOAD    1
#define DOWNLOAD  0

#define EXTERNAL  1
#define INTERNAL  0


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
  int ftmRamWriteWord(Device& dev, uint32_t adr, uint32_t data);
  uint32_t ftmRamReadWord(Device& dev, uint32_t adr);

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

  int sendCmd(uint8_t cpuIdx, const std::string& targetName, uint8_t cmdPrio, mc_ptr mc); 

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

  
  //generates addresses for command to be sent
  vAdr getCmdWrAdrs(uint8_t cpuIdx, const std::string& targetName, uint8_t);

  //generates binary data for command to be sent
  vBuf getCmdData(uint8_t cpuIdx, const std::string& targetName, uint8_t prio, mc_ptr m);
  

  //Returns if a hash / nodename is known to the hashmap
  bool isKnown(const uint32_t hash)     const {return hm.contains(hash);}
  bool isKnown(const std::string& name) const {return hm.contains(name);}


  //Returns the external address of a thread's command register area
  uint32_t getThrCmdAdr(uint8_t cpuIdx);

  //Returns the external address of a thread's initial node register 
  uint32_t getThrInitialNodeAdr(uint8_t cpuIdx, uint8_t thrIdx);

  //Returns the external address of a thread's current node register 
  uint32_t getThrCurrentNodeAdr(uint8_t cpuIdx, uint8_t thrIdx);

  //Sets the Node the Thread will start from
  void setThrOrigin(uint8_t cpuIdx, uint8_t thrIdx, const std::string& name);

  //Returns the Node the Thread will start from
  const std::string getThrOrigin(uint8_t cpuIdx, uint8_t thrIdx);

  //Returns the Node the Thread is currently processing
  const std::string getThrCursor(uint8_t cpuIdx, uint8_t thrIdx);

  //Get bifield showing running threads
  uint32_t getThrRun(uint8_t cpuIdx);

  //Requests Threads to start
  void setThrStart(uint8_t cpuIdx, uint32_t bits);

  //Requests Threads to stop
  void setThrStop(uint8_t cpuIdx, uint32_t bits);

  //hard abort, emergency only
  void clrThrRun(uint8_t cpuIdx, uint32_t bits);

  //true if thread <thrIdx> is running
  bool isThrRunning(uint8_t cpuIdx, uint8_t thrIdx);

  //Requests Thread to start
  void startThr(uint8_t cpuIdx, uint8_t thrIdx);
  
  //Requests Thread to stop
  void stopThr(uint8_t cpuIdx, uint8_t thrIdx);

  //Immediately aborts a Thread
  void abortThr(uint8_t cpuIdx, uint8_t thrIdx);

  //shortcut to obtain a node's address by its name
  uint32_t getNodeAdr(uint8_t cpuIdx, const std::string& name, bool direction, bool intExt); 

  //show upload table
  void showUp(uint8_t cpuIdx) {MemUnit& m = vM.at(cpuIdx);  m.showUp("Upload Table", "upload_dict.txt");}

  //show download table
  void showDown(uint8_t cpuIdx) {MemUnit& m = vM.at(cpuIdx);  m.showDown("Download Table", "download_dict.txt");}
};

#endif