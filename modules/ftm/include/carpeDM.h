#ifndef _CARPEDM_H_
#define _CARPEDM_H_

#define SDB_VENDOR_GSI      0x0000000000000651ULL
#define SDB_DEVICE_LM32_RAM 0x54111351

#include <stdio.h>
#include <iostream>
#include <string>
#include <inttypes.h>
#include <map>
#include <boost/graph/graphviz.hpp>
#include <etherbone.h>
#include "graph.h"
#include "common.h"
#include "hashmap.h"
#include "memunit.h"



#define FWID_RAM_TOO_SMALL      -1
#define FWID_BAD_MAGIC          -2
#define FWID_BAD_PROJECT_NAME   -3
#define FWID_NOT_FOUND          -4
#define FWID_BAD_VERSION_FORMAT -5
#define VERSION_MAJOR           0
#define VERSION_MINOR           1
#define VERSION_REVISION        2
#define VERSION_MAJOR_MUL       10000
#define VERSION_MINOR_MUL       100
#define VERSION_REVISION_MUL    1

class MiniCommand;

using namespace etherbone;

class CarpeDM {

private:
  static const unsigned char deadbeef[4];
  static const std::string needle;

protected:

  std::string ebdevname;
  std::string outputfilename;
  std::string inputfilename;

  std::vector<MemUnit> vM;
  std::vector<Graph>  vUp;
  std::vector<int> vFw;
  std::map<uint8_t, uint8_t> cpuIdxMap;


  Socket ebs;
  Device ebd;  
  std::vector<struct sdb_device> myDevs;  

  int cpuQty = -1;
  HashMap hm;
  bool verbose = false;
  std::ostream& sLog;
  std::ostream& sErr;

  int   ebWriteCycle(Device& dev, vAdr va, vBuf& vb);
  vBuf  ebReadCycle(Device& dev, vAdr va);
  int ebWriteWord(Device& dev, uint32_t adr, uint32_t data);
  uint32_t ebReadWord(Device& dev, uint32_t adr);
  boost::dynamic_properties createParser(Graph& g);
  int parseFwVersionString(const std::string& s);

public:
  CarpeDM() : sLog(std::cout), sErr(std::cerr)  {} 
  CarpeDM(std::ostream& sLog) : sLog(sLog), sErr(std::cerr)  {} 
  CarpeDM(std::ostream& sLog, std::ostream& sErr) : sLog(sLog), sErr(sErr)  {}
  ~CarpeDM() {};

  //Open connection to a DM via Etherbone
  bool connect(const std::string& en);

  //Close connection
  bool disconnect();

  //Retrieve the Firmware Version of cpu at sdb dev array idx <cpuIdx>
  int getFwVersion(uint8_t cpuIdx);

  //Check if CPU is registered as running a valid firmware
  bool isValidDMCpu(uint8_t cpuIdx) {return (cpuIdxMap.count(cpuIdx) > 0);}

  //Add all nodes in .dot file to name/hash dictionary
  void addDotToDict(const std::string& fn);

  //Clear the dictionary
  void clearDict();
  
  //Parse a .dot file to create unprocessed Graph
  Graph& parseUpDot(const std::string& fn, Graph& g);

  //Process Graph for uploading to LM32 SoC
  bool prepareUploadToCpu(Graph& g, uint8_t cpuIdx);

  //Upload processed Graph to LM32 SoC via Etherbone
  int upload(uint8_t cpuIdx);

  //Process and upload .dot file to to LM32 SoC via Etherbone
  int uploadDot(uint8_t cpuIdx, const std::string& fn) { Graph gTmp; prepareUploadToCpu( parseUpDot(fn, gTmp), cpuIdx); return upload(cpuIdx); }

  //Send a command to Block <targetName> on CPU <cpuIdx> via Etherbone
  int sendCmd(uint8_t cpuIdx, const std::string& targetName, uint8_t cmdPrio, mc_ptr mc); 

  //TODO NC analysis

  //TODO assign to CPUs/threads

  //Download binary from LM32 SoC and create Graph
  int downloadAndParse(uint8_t cpuIdx);

  //Write out processed Download Graph as .dot file
  void writeDownDot(const std::string& fn, uint8_t cpuIdx, bool filterMeta) { writeDownDot( fn, vM.at(cpuIdxMap.at(cpuIdx)), filterMeta ); }
  void writeDownDot(const std::string& fn, MemUnit& m, bool filterMeta);

  //Turn on Verbose Output
  void verboseOn()  {verbose = true;}

  //Turn off Verbose Output
  void verboseOff() {verbose = false;}

  //Tell if Output is set to Verbose 
  bool isVerbose()  const {return verbose;}

  //Return number of found CPUs (not necessarily valid ones!)
  int getCpuQty()   const {return cpuQty;}

  //Returns the Upload Graph for CPU <cpuIdx>
  Graph& getUpGraph(uint8_t cpuIdx)   {return vM.at(cpuIdxMap.at(cpuIdx)).getUpGraph();}

  //Returns the Download Graph for CPU <cpuIdx>
  Graph& getDownGraph(uint8_t cpuIdx) {return vM.at(cpuIdxMap.at(cpuIdx)).getDownGraph();}

  //Returns if a hash / nodename is known to the dictionary
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

  //show a CPU's Upload address table
  void showUp(uint8_t cpuIdx, bool filterMeta) {MemUnit& m = vM.at(cpuIdxMap.at(cpuIdx));  m.show("Upload Table", "upload_dict.txt", UPLOAD, filterMeta);}

  //show a CPU's Download address table
  void showDown(uint8_t cpuIdx, bool filterMeta) {MemUnit& m = vM.at(cpuIdxMap.at(cpuIdx));  m.show("Download Table", "download_dict.txt", DOWNLOAD, filterMeta);}

  //Show all command fields in a Queue (past and current)
  void dumpQueue(uint8_t cpuIdx, const std::string& blockName, uint8_t cmdPrio);

  //hex dump a node
  void dumpNode(uint8_t cpuIdx, const std::string& name);

  bool isCpuIdxValid(uint8_t cpuIdx) { if (( cpuIdxMap.find(cpuIdx) != cpuIdxMap.end() ) && (cpuIdxMap.at(cpuIdx) < vM.size()) ) return true; else return false;}

  void showCpuList();
};

#endif