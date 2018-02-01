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
#include <boost/optional.hpp>
#include <etherbone.h>
#include "graph.h"
#include "common.h"
#include "hashmap.h"
#include "alloctable.h"
#include "grouptable.h"
#include "validation.h"
#include "ftm_shared_mmap.h"




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
  int remove(Graph& g, bool force);
  int keep(Graph& g, bool force);  
  int overwrite(Graph& g);
  bool validate(Graph& g, AllocTable& at);
  
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
  int getIdleThread(uint8_t cpuIdx);

  const std::string& firstString(const vStrC& v) {return ((v.size() > 0) ? *(v.begin()) : DotStr::Misc::sUndefined);}
  boost::optional<std::pair<int, int>> parseCpuAndThr(vertex_t v, Graph& g);

  bool findDefPath(vertex_t start, vertex_t goal, Graph& g);
  bool hasIncomingDefDsts(const std::string& pattern, vertex_t v, bool strict);
  vertex_set_t getDynamicDestinations(vertex_t vQ, Graph& g, AllocTable& at);
  bool hasIncomingDynamicFlows(vertex_t v);
  bool hasIncomingResidentFlows(vertex_t v);
  void getReverseNodeTree(vertex_t v, vertex_set_t& sV, Graph& g);
  vertex_set_t getAllActiveCursors();
  bool findDefPath(vertex_t start, vertex_t goal);
  vStrC getGraphPatterns(Graph& g);
  bool isSafeToRemoveAdv(Graph& gRem);

protected:

  std::string ebdevname;
  std::string outputfilename;
  std::string inputfilename;

  std::vector<int> vFw;
  std::map<uint8_t, uint8_t> cpuIdxMap;

  Socket ebs;
  Device ebd;  
  std::vector<struct sdb_device> cpuDevs;  
  std::vector<struct sdb_device> ppsDev; 

  int cpuQty = -1;
  HashMap hm;
  GroupTable gt;
  AllocTable atUp;
  Graph gUp;
  AllocTable atDown;
  Graph gDown;
  uint64_t modTime;
  bool freshDownload = false;

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
  CarpeDM() : sLog(std::cout), sErr(std::cerr) {Validation::init();} 
  CarpeDM(std::ostream& sLog) : sLog(sLog), sErr(std::cerr)   {Validation::init();} 
  CarpeDM(std::ostream& sLog, std::ostream& sErr) : sLog(sLog), sErr(sErr){Validation::init();}
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
  void clearHashDict() {hm.clear();}; //Clear the dictionary
  std::string storeHashDict() {return hm.store();}; 
  void loadHashDict(const std::string& s) {hm.load(s);}
  void storeHashDictFile(const std::string& fn) {writeTextFile(fn, storeHashDict());};
  void loadHashDictFile(const std::string& fn) {loadHashDict(readTextFile(fn));};
  bool isInHashDict(const uint32_t hash);
  bool isInHashDict(const std::string& name);
  bool isHashDictEmpty() {return (bool)(hm.size() == 0);};
  int  getHashDictSize() {return hm.size();};
  void showHashDict() {hm.debug(sLog);};

  // Group/Entry/Exit Table ///////////////////////////////////////////////////////////////////////////////
  std::string storeGroupsDict() {return gt.store();}; 
  void loadGroupsDict(const std::string& s) {gt.load(s);}
  void storeGroupsDictFile(const std::string& fn) {writeTextFile(fn, storeGroupsDict());};
  void loadGroupsDictFile(const std::string& fn) {loadGroupsDict(readTextFile(fn));}; 
  void clearGroupsDict() {gt.clear();}; //Clear pattern table
 int getGroupsSize() {return gt.getSize();};
 void showGroupsDict() {gt.debug(sLog);};

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
  int keepDot(const std::string& s, bool force) {Graph gTmp; return keep(parseDot(s, gTmp), force);};
  int keepDotFile(const std::string& fn, bool force) {return keepDot(readTextFile(fn), force);};
  //removes all nodes in input file                                            
  int removeDot(const std::string& s, bool force) {Graph gTmp; return remove(parseDot(s, gTmp), force);};
  int removeDotFile(const std::string& fn, bool force) {return removeDot(readTextFile(fn), force);};
  // Safe removal check
  bool isSafe2RemoveDotFile(const std::string& fn) {Graph gTmp; return isSafeToRemoveAdv(parseDot(readTextFile(fn), gTmp));};
  //clears all nodes from DM 
  int clear();

  vEbwrs& createCommandBurst(Graph& g, vEbwrs& ew);
  vEbwrs& createCommand(const std::string& targetName, uint8_t cmdPrio, mc_ptr mc, vEbwrs& ew);

  // Command Generation and Dispatch //////////////////////////////////////////////////////////////
  int sendCommandsDot(const std::string& s) {Graph gTmp; vEbwrs ew; return send(createCommandBurst(parseDot(s, gTmp), ew));}; //Sends a dotfile of commands to the DM
  int sendCommandsDotFile(const std::string& fn) {Graph gTmp; vEbwrs ew; return send(createCommandBurst(parseDot(readTextFile(fn), gTmp), ew));};
  //Send a command to Block <targetName> on CPU <cpuIdx> via Etherbone
  int sendCommand(const std::string& targetName, uint8_t cmdPrio, mc_ptr mc) {vEbwrs ew; return send(createCommand(targetName, cmdPrio, mc, ew));}; 



         const vAdr getCmdWrAdrs(uint32_t hash, uint8_t prio); 
     const uint32_t getCmdInc(uint32_t hash, uint8_t prio);
           uint32_t getThrCmdAdr(uint8_t cpuIdx);  //Returns the external address of a thread's command register area
           uint32_t getThrInitialNodeAdr(uint8_t cpuIdx, uint8_t thrIdx); //Returns the external address of a thread's initial node register
           uint32_t getThrCurrentNodeAdr(uint8_t cpuIdx, uint8_t thrIdx); //Returns the external address of a thread's current node register 
  const std::string getThrOrigin(uint8_t cpuIdx, uint8_t thrIdx); //Returns the Node the Thread will start from
  const std::string getThrCursor(uint8_t cpuIdx, uint8_t thrIdx); //Returns the Node the Thread is currently processing
           uint64_t getThrMsgCnt(uint8_t cpuIdx, uint8_t thrIdx);
           uint32_t getThrRun(uint8_t cpuIdx); //Get bitfield showing running threads
           uint32_t getStatus(uint8_t cpuIdx);
           uint64_t getThrDeadline(uint8_t cpuIdx, uint8_t thrIdx);
           uint64_t getThrStartTime(uint8_t cpuIdx, uint8_t thrIdx);
           uint64_t getThrPrepTime(uint8_t cpuIdx, uint8_t thrIdx); 
               bool isThrRunning(uint8_t cpuIdx, uint8_t thrIdx); //true if thread <thrIdx> is running
            
// The lazy interface ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            uint8_t getNodeCpu(const std::string& name, TransferDir dir); //shortcut to obtain a node's cpu by its name 
           uint32_t getNodeAdr(const std::string& name, TransferDir dir, AdrType adrT); //shortcut to obtain a node's address by its name
  const std::string getNodePattern (const std::string& sNode);
  const std::string getNodeBeamproc(const std::string& sNode);
              vStrC getPatternMembers (const std::string& sPattern);
 const std::string getPatternEntryNode(const std::string& sPattern);
 const std::string getPatternExitNode(const std::string& sPattern);
              vStrC getBeamprocMembers(const std::string& sBeamproc);
  const std::string getBeamprocEntryNode(const std::string& sBeamproc);
  const std::string getBeamprocExitNode(const std::string& sBeamproc);
               void inspectHeap(uint8_t cpuIdx);


  // The very lazy interface ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::pair<int, int> findRunningPattern(const std::string& sPattern); //get cpu and thread assignment of running pattern
               bool isPatternRunning(const std::string& sPattern); //true if Pattern <x> is running
               vEbwrs& startThr(uint8_t cpuIdx, uint8_t thrIdx, vEbwrs& ew); //Requests Thread to start
               vEbwrs& startPattern(const std::string& sPattern, uint8_t thrIdx, vEbwrs& ew); //Requests Pattern to start
               vEbwrs& startPattern(const std::string& sPattern, vEbwrs& ew); //Requests Pattern to start on first free thread
               vEbwrs& startNodeOrigin(const std::string& sNode, uint8_t thrIdx, vEbwrs& ew); //Requests thread <thrIdx> to start at node <sNode>
               vEbwrs& startNodeOrigin(const std::string& sNode, vEbwrs& ew); //Requests a start at node <sNode>
               vEbwrs& stopPattern(const std::string& sPattern, vEbwrs& ew); //Requests Pattern to stop
               vEbwrs& stopNodeOrigin(const std::string& sNode, vEbwrs& ew); //Requests stop at node <sNode> (flow to idle)
               vEbwrs& abortPattern(const std::string& sPattern, vEbwrs& ew); //Immediately aborts a Pattern
               vEbwrs& abortNodeOrigin(const std::string& sNode, vEbwrs& ew); //Immediately aborts the thread whose pattern <sNode> belongs to
               vEbwrs& abortThr(uint8_t cpuIdx, uint8_t thrIdx, vEbwrs& ew); //Immediately aborts a Thread
               vEbwrs& setThrStart(uint8_t cpuIdx, uint32_t bits, vEbwrs& ew); //Requests Threads to start
               vEbwrs& setThrAbort(uint8_t cpuIdx, uint32_t bits, vEbwrs& ew); //Immediately aborts Threads
               vEbwrs& setThrOrigin(uint8_t cpuIdx, uint8_t thrIdx, const std::string& name, vEbwrs& ew); //Sets the Node the Thread will start from
               vEbwrs& setThrStartTime(uint8_t cpuIdx, uint8_t thrIdx, uint64_t t, vEbwrs& ew);
               vEbwrs& setThrPrepTime(uint8_t cpuIdx, uint8_t thrIdx, uint64_t t, vEbwrs& ew);
               int send(vEbwrs& ew); 

               //direct send wrappers. bit cumbersome, can possibly be done by a template
               int startThr(uint8_t cpuIdx, uint8_t thrIdx)                              { vEbwrs ew; return send(startThr(cpuIdx, thrIdx, ew));} //Requests Thread to start
               int startPattern(const std::string& sPattern, uint8_t thrIdx)             { vEbwrs ew; return send(startPattern(sPattern, thrIdx, ew));}//Requests Pattern to start
               int startPattern(const std::string& sPattern)                             { vEbwrs ew; return send(startPattern(sPattern, ew));}//Requests Pattern to start on first free thread
               int startNodeOrigin(const std::string& sNode, uint8_t thrIdx)             { vEbwrs ew; return send(startNodeOrigin(sNode, thrIdx, ew));}//Requests thread <thrIdx> to start at node <sNode>
               int startNodeOrigin(const std::string& sNode)                             { vEbwrs ew; return send(startNodeOrigin(sNode, ew));}//Requests a start at node <sNode>
               int stopPattern(const std::string& sPattern)                              { vEbwrs ew; return send(stopPattern(sPattern, ew));}//Requests Pattern to stop
               int stopNodeOrigin(const std::string& sNode)                              { vEbwrs ew; return send(stopNodeOrigin(sNode, ew));}//Requests stop at node <sNode> (flow to idle)
               int abortPattern(const std::string& sPattern)                             { vEbwrs ew; return send(abortPattern(sPattern, ew));}//Immediately aborts a Pattern
               int abortNodeOrigin(const std::string& sNode)                             { vEbwrs ew; return send(abortNodeOrigin(sNode, ew));}//Immediately aborts the thread whose pattern <sNode> belongs to
               int abortThr(uint8_t cpuIdx, uint8_t thrIdx)                              { vEbwrs ew; return send(abortThr(cpuIdx, thrIdx, ew));} //Immediately aborts a Thread
               int setThrStart(uint8_t cpuIdx, uint32_t bits)                            { vEbwrs ew; return send(setThrStart(cpuIdx, bits, ew));} //Requests Threads to start
               int setThrAbort(uint8_t cpuIdx, uint32_t bits)                            { vEbwrs ew; return send(setThrAbort(cpuIdx, bits, ew));}//Immediately aborts Threads
               int setThrOrigin(uint8_t cpuIdx, uint8_t thrIdx, const std::string& name) { vEbwrs ew; return send(setThrOrigin(cpuIdx, thrIdx, name, ew));}//Sets the Node the Thread will start from
               int setThrStartTime(uint8_t cpuIdx, uint8_t thrIdx, uint64_t t)           { vEbwrs ew; return send(setThrStartTime(cpuIdx, thrIdx, t, ew));}
               int setThrPrepTime(uint8_t cpuIdx, uint8_t thrIdx, uint64_t t)            { vEbwrs ew; return send(setThrPrepTime(cpuIdx, thrIdx, t, ew));}
     HealthReport& getHealth(uint8_t cpuIdx, HealthReport &hr);
          uint64_t getDmWrTime();
              bool isSafeToRemove(const std::string& pattern, bool strict);


               

  // Screen Output //////////////////////////////////////////////////////////////
  void show(const std::string& title, const std::string& logDictFile, TransferDir dir, bool filterMeta );
  void showUp(bool filterMeta) {show("Upload Table", "upload_dict.txt", TransferDir::UPLOAD, false);} //show a CPU's Upload address table
  void showDown(bool filterMeta) {  //show a CPU's Download address table
    show("Download Table" + (filterMeta ? std::string(" (noMeta)") : std::string("")), "download_dict.txt", TransferDir::DOWNLOAD, filterMeta);
  }
  void showCpuList();
  void dumpQueue(uint8_t cpuIdx, const std::string& blockName, uint8_t cmdPrio); //Show all command fields in a Queue (past and current)
  void dumpNode(uint8_t cpuIdx, const std::string& name); //hex dump a node
  void verboseOn()  {verbose = true;}  //Turn on Verbose Output
  void verboseOff() {verbose = false;} //Turn off Verbose Output
  bool isVerbose()  const {return verbose;} //Tell if Output is set to Verbose 

};

#endif
