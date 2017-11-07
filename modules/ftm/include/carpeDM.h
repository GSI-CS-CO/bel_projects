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
#include "alloctable.h"
#include "patterntable.h"
#include "ftm_shared_mmap.h"



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
  void updateListDstStaging(vertex_t v);
  void updateStaging(vertex_t v, edge_t e);
  void pushMetaNeighbours(vertex_t v, Graph& g, vertex_set_t& s);
  void generateBlockMeta(Graph& g);
  void generateDstLst(Graph& g, vertex_t v);
  void generateQmeta(Graph& g, vertex_t v, int prio);
  void completeId(vertex_t v, Graph& g);

  void addition(Graph& g);
  void subtraction(Graph& g);
  void nullify();

  int add(Graph& g);
  int remove(Graph& g);
  int keep(Graph& g);  
  int overwrite(Graph& g);
  
  int sendCommands(Graph &); //Sends a dotfile of commands to the DM

  Graph& parseDot(const std::string& s, Graph& g); //Parse a .dot string to create unprocessed Graph

  // Upload
  vAdr getUploadAdrs();
  vBuf getUploadData();
  int upload(); //Upload processed Graph to LM32 SoC via Etherbone
  
  // Download
  const vAdr getDownloadBMPAdrs();
  const vAdr getDownloadAdrs();
  void parseDownloadData(vBuf downloadData);
  

  void baseUploadOnDownload();
  void prepareUpload(); //Process Graph for uploading to LM32 SoC
  void mergeUploadDuplicates(vertex_t borg, vertex_t victim); 

  void addToDict(Graph& g);      
  void removeFromDict(Graph& g); 


protected:

  std::string ebdevname;
  std::string outputfilename;
  std::string inputfilename;

  std::vector<int> vFw;
  std::map<uint8_t, uint8_t> cpuIdxMap;

  Socket ebs;
  Device ebd;  
  std::vector<struct sdb_device> myDevs;  

  int cpuQty = -1;
  HashMap hm;
  PatternTable pt;
  AllocTable atUp;
  Graph gUp;
  AllocTable atDown;
  Graph gDown;

  bool verbose = false;
  std::ostream& sLog;
  std::ostream& sErr;

  int   ebWriteCycle(Device& dev, vAdr va, vBuf& vb);
  vBuf  ebReadCycle(Device& dev, vAdr va);
  int ebWriteWord(Device& dev, uint32_t adr, uint32_t data);
  uint32_t ebReadWord(Device& dev, uint32_t adr);
  boost::dynamic_properties createParser(Graph& g);

  //std::string getFwInfo(uint8_t cpuIdx);
  int parseFwVersionString(const std::string& s);
  uint64_t read64b(uint32_t startAdr);
  int write64b(uint32_t startAdr, uint64_t d);

  Graph& getUpGraph()   {return gUp;}   //Returns the Upload Graph for CPU <cpuIdx>
  Graph& getDownGraph() {return gDown;} //Returns the Download Graph for CPU <cpuIdx>



public:
  CarpeDM() : sLog(std::cout), sErr(std::cerr) {} 
  CarpeDM(std::ostream& sLog) : sLog(sLog), sErr(std::cerr)   {} 
  CarpeDM(std::ostream& sLog, std::ostream& sErr) : sLog(sLog), sErr(sErr){}
  ~CarpeDM() {};

  // Etherbone interface
  bool connect(const std::string& en); //Open connection to a DM via Etherbone
  bool disconnect(); //Close connection

  // SDB Functions
  bool isValidDMCpu(uint8_t cpuIdx) {return (cpuIdxMap.count(cpuIdx) > 0);} //Check if CPU is registered as running a valid firmware  
  int getFwVersion(uint8_t cpuIdx); //Retrieve the Firmware Version of cpu at sdb dev array idx <cpuIdx>
  uint32_t getIntBaseAdr(uint8_t cpuIdx) {return INT_BASE_ADR;} //mockup for now, this info should be taken from found firmware binary
  uint32_t getSharedOffs(uint8_t cpuIdx) {return SHARED_OFFS;}
  uint32_t getSharedSize(uint8_t cpuIdx) {return SHARED_SIZE;}
  int getCpuQty()   const {return cpuQty;} //Return number of found CPUs (not necessarily valid ones!)
  bool isCpuIdxValid(uint8_t cpuIdx) { if ( cpuIdxMap.find(cpuIdx) != cpuIdxMap.end() ) return true; else return false;}  


  //Wrappers for everything

  // Name/Hash Dict ///////////////////////////////////////////////////////////////////////////////
  //Add all nodes in .dot file to name/hash dictionary
  void addDotToDict(const std::string& s)           {Graph gTmp; addToDict(parseDot(s, gTmp));};
  void addDotFileToDict(const std::string& fn)      {addDotToDict(readTextFile(fn));};
  //Remove all nodes in .dot file from name/hash dictionary
  void removeDotFromDict(const std::string& s)      {Graph gTmp; removeFromDict(parseDot(s, gTmp));};
  void removeDotFileFromDict(const std::string& fn) {removeDotFromDict(readTextFile(fn));};
  void clearDict() {hm.clear();}; //Clear the dictionary



  std::string storeDict() {return hm.store();}; 
  void loadDict(const std::string& s) {hm.load(s);}
  void storeDictFile(const std::string& fn) {writeTextFile(fn, storeDict());};
  void loadDictFile(const std::string& fn) {loadDict(readTextFile(fn));};

  bool isInDict(const uint32_t hash);
  bool isInDict(const std::string& name);
  bool isDictEmpty() {return (bool)(hm.size() == 0);};

  // Pattern/Entry/Exit Table ///////////////////////////////////////////////////////////////////////////////
  std::string storePatterns() {return pt.store();}; 
  void loadPatterns(const std::string& s) {pt.load(s);}
  void storePatternsFile(const std::string& fn) {writeTextFile(fn, storePatterns());};
  void loadPatternsFile(const std::string& fn) {loadPatterns(readTextFile(fn));}; 
  void clearPatterns() {pt.clear();}; //Clear pattern table
  void testPattern(const std::string& pattern) {bool dummy; pt.insertPattern(pattern, dummy);} 

  // Text File IO /////////////////////////////////////////////////////////////////////////////////
  void writeTextFile(const std::string& fn, const std::string& s);
  std::string  readTextFile(const std::string& fn);

  // Graphs to Dot
  std::string createDot( Graph& g, bool filterMeta);
  void writeDotFile(const std::string& fn, Graph& g, bool filterMeta) { writeTextFile(fn, createDot(g, filterMeta)); }
  void writeDownDotFile(const std::string& fn, bool filterMeta)       { writeTextFile(fn, createDot(gDown, filterMeta)); }
  void writeUpDotFile(const std::string& fn, bool filterMeta)         { writeTextFile(fn, createDot(gUp, filterMeta)); }

  // Schedule Manipulation and Dispatch ///////////////////////////////////////////////////////////
  //TODO assign a cpu to each node object. Currently taken from input .dot
  int assignNodesToCpus() {return 0;};
  //get all nodes from DM
  int download();  //Download binary from LM32 SoC and create Graph
  std::string downloadDot(bool filterMeta) {download(); return createDot( gDown, filterMeta);};            
  void downloadDotFile(const std::string& fn, bool filterMeta) {download(); writeDownDotFile(fn, filterMeta);};   
  //add all nodes and/or edges in dot file
  int addDot(const std::string& s) {Graph gTmp; return add(parseDot(s, gTmp));};            
  int addDotFile(const std::string& fn) {return addDot(readTextFile(fn));};                 
  //add all nodes and/or edges in dot file                                                                                     
  int overwriteDot(const std::string& s) {Graph gTmp; return overwrite(parseDot(s, gTmp));};
  int overwriteDotFile(const std::string& fn) {return overwriteDot(readTextFile(fn));};
  //removes all nodes NOT in input file
  int keepDot(const std::string& s) {Graph gTmp; return keep(parseDot(s, gTmp));};
  int keepDotFile(const std::string& fn) {return keepDot(readTextFile(fn));};
  //removes all nodes in input file                                            
  int removeDot(const std::string& s) {Graph gTmp; return remove(parseDot(s, gTmp));};
  int removeDotFile(const std::string& fn) {return removeDot(readTextFile(fn));};
  //clears all nodes from DM 
  int clear();



  // Command Generation and Dispatch //////////////////////////////////////////////////////////////
  int sendCommandsDot(const std::string& s) {Graph gTmp; return sendCommands(parseDot(s, gTmp));}; //Sends a dotfile of commands to the DM
  int sendCommandsDotFile(const std::string& fn) {return sendCommandsDot(readTextFile(fn));};
  //Send a command to Block <targetName> on CPU <cpuIdx> via Etherbone
  int sendCmd(const std::string& targetName, uint8_t cmdPrio, mc_ptr mc); 

         const vAdr getCmdWrAdrs(uint32_t hash, uint8_t prio); 
     const uint32_t getCmdInc(uint32_t hash, uint8_t prio);
           uint32_t getThrCmdAdr(uint8_t cpuIdx);  //Returns the external address of a thread's command register area
           uint32_t getThrInitialNodeAdr(uint8_t cpuIdx, uint8_t thrIdx); //Returns the external address of a thread's initial node register
           uint32_t getThrCurrentNodeAdr(uint8_t cpuIdx, uint8_t thrIdx); //Returns the external address of a thread's current node register 
               void setThrOrigin(uint8_t cpuIdx, uint8_t thrIdx, const std::string& name); //Sets the Node the Thread will start from
  const std::string getThrOrigin(uint8_t cpuIdx, uint8_t thrIdx); //Returns the Node the Thread will start from
  const std::string getThrCursor(uint8_t cpuIdx, uint8_t thrIdx); //Returns the Node the Thread is currently processing
           uint32_t getThrRun(uint8_t cpuIdx); //Get bitfield showing running threads
           uint32_t getStatus(uint8_t cpuIdx);
           uint64_t getThrDeadline(uint8_t cpuIdx, uint8_t thrIdx);
           uint64_t getThrStartTime(uint8_t cpuIdx, uint8_t thrIdx);
           uint64_t getThrPrepTime(uint8_t cpuIdx, uint8_t thrIdx); 
               void setThrStartTime(uint8_t cpuIdx, uint8_t thrIdx, uint64_t t);
               void setThrPrepTime(uint8_t cpuIdx, uint8_t thrIdx, uint64_t t);
               void inspectHeap(uint8_t cpuIdx);
               void setThrStart(uint8_t cpuIdx, uint32_t bits); //Requests Threads to start
               void setThrStop(uint8_t cpuIdx, uint32_t bits); //Requests Threads to stop
               void clrThrRun(uint8_t cpuIdx, uint32_t bits); //hard abort, emergency only
               bool isThrRunning(uint8_t cpuIdx, uint8_t thrIdx); //true if thread <thrIdx> is running
               void startThr(uint8_t cpuIdx, uint8_t thrIdx); //Requests Thread to start
               void stopThr(uint8_t cpuIdx, uint8_t thrIdx); //Requests Thread to stop
               void abortThr(uint8_t cpuIdx, uint8_t thrIdx); //Immediately aborts a Thread
            uint8_t getNodeCpu(const std::string& name, bool direction); //shortcut to obtain a node's cpu by its name 
           uint32_t getNodeAdr(const std::string& name, bool direction, bool intExt); //shortcut to obtain a node's address by its name   

  // Screen Output //////////////////////////////////////////////////////////////
  void show(const std::string& title, const std::string& logDictFile, bool direction, bool filterMeta );
  void showUp(bool filterMeta) {show("Upload Table", "upload_dict.txt", UPLOAD, false);} //show a CPU's Upload address table
  void showDown(bool filterMeta) {  //show a CPU's Download address table
    show("Download Table" + (filterMeta ? std::string(" (noMeta)") : std::string("")), "download_dict.txt", DOWNLOAD, filterMeta);
  }
  void showCpuList();
  void dumpQueue(uint8_t cpuIdx, const std::string& blockName, uint8_t cmdPrio); //Show all command fields in a Queue (past and current)
  void dumpNode(uint8_t cpuIdx, const std::string& name); //hex dump a node
  void verboseOn()  {verbose = true;}  //Turn on Verbose Output
  void verboseOff() {verbose = false;} //Turn off Verbose Output
  bool isVerbose()  const {return verbose;} //Tell if Output is set to Verbose 

};

#endif