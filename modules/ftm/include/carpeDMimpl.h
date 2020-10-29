#ifndef _CARPEDM_IMPL_H_
#define _CARPEDM_IMPL_H_

#include <stdio.h>
#include <iostream>
#include <string>
#include <inttypes.h>

#include "carpeDM.h"
#include "graph.h"
#include "common.h"
#include "hashmap.h"
#include "alloctable.h"
#include "grouptable.h"
#include "covenanttable.h"
#include "validation.h"
#include "lockmanager.h"




//class CarpeDM;
class MiniCommand;



class CarpeDM::CarpeDMimpl {

private:

  std::string outputfilename;
  std::string inputfilename;

  int cpuQty = -1;
  HashMap hm;
//  GroupTable gt;
  CovenantTable ct;
  AllocTable atUp;
  Graph gUp;
  AllocTable atDown;
  Graph gDown;

  uint64_t modTime;
  bool freshDownload = false;

  bool verbose  = false;
  bool debug    = false;
  bool sim      = false;
  bool testmode = false;
  bool optimisedS2R = true;
  std::ostream& sLog;
  std::ostream& sErr;

  EbWrapper ebd = EbWrapper(sLog, sErr, verbose, debug);
  LockManager lm = LockManager(ebd, hm, ct, atDown); //get us an instance of the lock manager

  void updateListDstStaging(vertex_t v);
  void updateStaging(vertex_t v, edge_t e);
  void pushMetaNeighbours(vertex_t v, Graph& g, vertex_set_t& s);
  void generateBlockMeta(Graph& g);
  void generateDstLst(Graph& g, vertex_t v);
  void generateQmeta(Graph& g, vertex_t v, int prio);
  void generateMgmtData();
  void completeId(vertex_t v, Graph& g);

  void addition(Graph& g);
  void subtraction(Graph& g);
  void nullify();

  template <typename R, typename ... As1, typename ... As2>
  R safeguardTransaction(R(CarpeDMimpl::*func)(As1...), As2 ... args);

  template <typename R, typename ... As1, typename ... As2>
  int dSend(R(CarpeDMimpl::*func)(As1...), As2 ... args);

  int add(Graph& g, bool force);
  int remove(Graph& g, bool force);
  int keep(Graph& g, bool force);
  int overwrite(Graph& g,  bool force);
  int clear_raw(bool force);
  bool validate(Graph& g, AllocTable& at, bool force);

  // Upload
  vEbwrs gatherUploadVector(std::set<uint8_t> moddedCpus, uint32_t modCnt, uint8_t opType);
  vEbwrs& createModInfo     (vEbwrs& ew, uint8_t cpu, uint32_t modCnt, uint8_t opType, uint32_t adrOffs);
  vEbwrs& createSchedModInfo(vEbwrs& ew, uint8_t cpu, uint32_t modCnt, uint8_t opType);
  vEbwrs& createCmdModInfo  (vEbwrs& ew, uint8_t cpu, uint32_t modCnt, uint8_t opType);
  int upload(uint8_t opType, std::vector<QueueReport>& vQr); //Upload processed Graph to LM32 SoC via Etherbone
  int upload(uint8_t opType) {std::vector<QueueReport> vQr; return upload(opType, vQr);}
  // Download
  vEbrds gatherDownloadBmpVector();
  vEbrds gatherDownloadDataVector();

  void parseDownloadData(const vBuf& downloadData);
  void parseDownloadMgmt(const vBuf& downloadData);
  void checkTablesForSubgraph(Graph& g);

  //void resetThrMsgCnt(uint8_t cpuIdx, uint8_t thrIdx);
  void baseUploadOnDownload();
  void prepareUpload(); //Process Graph for uploading to LM32 SoC
  void mergeUploadDuplicates(vertex_t borg, vertex_t victim);

  void addToDict(Graph& g);
  void removeFromDict(Graph& g);
  int getIdleThread(uint8_t cpuIdx);

  const std::string& firstString(const vStrC& v);
  boost::optional<std::pair<int, int>> parseCpuAndThr(vertex_t v, Graph& g);

  bool addResidentDestinations(Graph& gEq,  Graph& gOrig, vertex_set_t cursors);
  bool addDynamicDestinations(Graph& g, AllocTable& at);
  bool updateStaleDefaultDestinations(Graph& g, AllocTable& at, CovenantTable& cov, std::string& qAnalysis);
  vertex_set_t getDominantFlowDst(vertex_t vQ, Graph& g, AllocTable& at, CovenantTable& covTab, std::string& qAnalysis);
  vertex_set_t getDynamicDestinations(vertex_t vQ, Graph& g, AllocTable& at);
  void getReverseNodeTree(vertex_t v, vertex_set_t& sV, Graph& g, vertex_set_map_t& covenantsPerVertex, vertex_t covenant = null_vertex, int32_t maxNtEdges=-1, int32_t tNtEdges=0);
  bool isOptimisableEdge(edge_t e, Graph& g);
  bool isCovenantPending(const std::string& covName);
  bool isCovenantPending(cmI cov);
  unsigned updateCovenants();
  void addCovenants(CovenantTable& ctAdditions);

  bool isSafetyCritical(vertex_set_t& covenants);
  //Coverage Tests for safe2remove

  bool coverage3IsSeedValid(uint64_t seed);
  Graph& coverage3GenerateBase(Graph& g);
  Graph& coverage3GenerateStatic(Graph& g, uint64_t seed);
  Graph& coverage3GenerateDynamic(Graph& g, uint64_t seed);
  std::string coverage3GenerateCursor(Graph& g, uint64_t seed );

  vertex_set_t getAllCursors(bool activeOnly);
  vStrC getGraphPatterns(Graph& g);

  bool isSafeToRemove(std::set<std::string> patterns, std::string& report, std::vector<QueueReport>& vQr, CovenantTable& ctAdditions);
  bool isSafeToRemove(std::set<std::string> patterns, std::string& report) {CovenantTable ctAdditions; std::vector<QueueReport> vQr; return isSafeToRemove(patterns, report, vQr, ctAdditions);}
  const std::string readFwIdROMTag(const std::string& fwIdROM, const std::string& tag, size_t maxlen, bool stopAtCr );

  vBuf compress(const vBuf& in);
  vBuf decompress(const vBuf& in);

  void readMgmtLLMeta();

  
  vEbwrs& setThrDeadline(vEbwrs& ew, uint8_t cpuIdx, uint8_t thrIdx, uint64_t t);
  vEbwrs& setThrCursor(vEbwrs& ew, uint8_t cpuIdx, uint8_t thrIdx, const std::string& name);
  void resetAllThreads();

  vEbwrs& staticFlush(const std::string& sBlock, bool prioIl, bool prioHi, bool prioLo, vEbwrs& ew, bool force);

  QueueElement& getQelement(Graph& g, AllocTable& at, uint8_t idx, amI allocIt, QueueElement& qe, const vStrC& futureOrphan);
  QueueElement& getQelement(Graph& g, AllocTable& at, uint8_t idx, amI allocIt, QueueElement& qe) {vStrC fo; return getQelement(g, at, idx, allocIt, qe, fo);}
  QueueReport& getQReport(Graph& g, AllocTable& at, const std::string& blockName, QueueReport& qr, const vStrC& futureOrphan);

  const uint32_t getCmdInc(uint32_t hash, uint8_t prio);
  uint32_t getThrCmdAdr(uint8_t cpuIdx);                                       // Returns the external address of a thread's command register area
  uint32_t getThrInitialNodeAdr(uint8_t cpuIdx, uint8_t thrIdx);               // Returns the external address of a thread's initial node register
  uint32_t getThrCurrentNodeAdr(uint8_t cpuIdx, uint8_t thrIdx);               // Returns the external address of a thread's current node register
  const vAdr getCmdWrAdrs(uint32_t hash, uint8_t prio);

  void adjustValidTime(uint64_t& tValid, bool abs);                            // Makes sure the given valid time is slightly in the future. Uses modTime as 'now'

  boost::dynamic_properties createParser(Graph& g);


  Graph& getUpGraph(); //Returns the Upload Graph for CPU <cpuIdx>

protected:




public:
  CarpeDMimpl();
  CarpeDMimpl(std::ostream& sLog);
  CarpeDMimpl(std::ostream& sLog, std::ostream& sErr);
  ~CarpeDMimpl();

  GroupTable gt;

// Etherbone interface
               bool connect(const std::string& en, bool simulation=false, bool test=false);
               bool disconnect(); //Close connection
               // SDB and DM HW detection Functions


//Internal Hash and Groupstable ///////////////////////////////////////////////////////////////////////////////////////////////
               // Name/Hash Dict
               void clearHashDict();                          //Clear hash table
        std::string storeHashDict();                          //save hash table to serialised string
               void loadHashDict(const std::string& s);       //initiallise hash table from serialised string
               void storeHashDictFile(const std::string& fn); //save hash table to file
               void loadHashDictFile(const std::string& fn);  //load hash table from file
               bool isInHashDict(const uint32_t hash);
               bool isInHashDict(const std::string& name);
               bool isHashDictEmpty();
                int getHashDictSize();

               // Group/Entry/Exit Table
               std::string storeGroupsDict();
               void loadGroupsDict(const std::string& s);
               void storeGroupsDictFile(const std::string& fn);
               void loadGroupsDictFile(const std::string& fn);
               void clearGroupsDict(); //Clear pattern table
                int getGroupsSize();

// Aux Infos from Table lookups and static computation  /////////////////////////////////////////////////////////////////////////////////////////////////////
            uint8_t getNodeCpu(const std::string& name, TransferDir dir);               // shortcut to obtain a node's cpu by its name
           uint32_t getNodeAdr(const std::string& name, TransferDir dir, AdrType adrT); // shortcut to obtain a node's address by its name
  const std::string getNodePattern (const std::string& sNode);
  const std::string getNodeBeamproc(const std::string& sNode);
              vStrC getPatternMembers (const std::string& sPattern);
  const std::string getPatternEntryNode(const std::string& sPattern);
  const std::string getPatternExitNode(const std::string& sPattern);
              vStrC getBeamprocMembers(const std::string& sBeamproc);
  const std::string getBeamprocEntryNode(const std::string& sBeamproc);
  const std::string getBeamprocExitNode(const std::string& sBeamproc);
           uint64_t getModTime() { return modTime; }

// Text File IO /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
               void writeTextFile(const std::string& fn, const std::string& s);
        std::string readTextFile(const std::string& fn);

// Graphs to Dot
             Graph& parseDot(const std::string& s, Graph& g); //Parse a .dot string to create unprocessed Graph
        std::string createDot( Graph& g, bool filterMeta);
               void writeDotFile(const std::string& fn, Graph& g, bool filterMeta);
               void writeDownDotFile(const std::string& fn, bool filterMeta);
               void writeUpDotFile(const std::string& fn, bool filterMeta);

// Schedule Manipulation ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int assignNodesToCpus();                            // NOT YET IMPLEMENTED // TODO assign a cpu to each node object. Currently taken from input .dot
                int download();                                     // Download binary from LM32 SoC and create Graph
        std::string downloadDot(bool filterMeta);
               void downloadDotFile(const std::string& fn, bool filterMeta);
                int addDot(const std::string& s, bool force);                   // add all nodes and/or edges in dot file
                int addDotFile(const std::string& fn, bool force);
                int overwriteDot(const std::string& s, bool force); // add all nodes and/or edges in dot file
                int overwriteDotFile(const std::string& fn, bool force);
                int keepDot(const std::string& s, bool force);      // removes all nodes NOT in input file
                int keepDotFile(const std::string& fn, bool force);
                int removeDot(const std::string& s, bool force);    // removes all nodes in input file
                int removeDotFile(const std::string& fn, bool force);
                int clear(bool force);                              // clears all nodes from DM

// Command Generation and Dispatch ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
              vStrC getLockedBlocks(bool checkReadLock, bool checkWriteLock);
                int sendCommandsDot(const std::string& s); //Sends a dotfile of commands to the DM
                int sendCommandsDotFile(const std::string& fn);
                int sendCommand(const std::string& targetName, uint8_t cmdPrio, mc_ptr mc); //Send a command to Block <targetName> on CPU <cpuIdx> via Etherbone
               void halt();
                int staticFlushPattern(const std::string& sPattern, bool prioIl, bool prioHi, bool prioLo, bool force);
                int staticFlushBlock(const std::string& sBlock, bool prioIl, bool prioHi, bool prioLo, bool force);


  // Short Live Infos from DM hardware reads /////////////////////////////////////////////////////////////////////////////////////////////////////////////
  const std::string getThrOrigin(uint8_t cpuIdx, uint8_t thrIdx);      // Returns the Node the Thread will start from
  const std::string getThrCursor(uint8_t cpuIdx, uint8_t thrIdx);      // Returns the Node the Thread is currently processing
           uint64_t getThrMsgCnt(uint8_t cpuIdx, uint8_t thrIdx);
           uint32_t getThrRun(uint8_t cpuIdx);                                  // Get bitfield showing running threads
           uint32_t getStatus(uint8_t cpuIdx);
           uint64_t getThrDeadline(uint8_t cpuIdx, uint8_t thrIdx);
           uint64_t getThrStartTime(uint8_t cpuIdx, uint8_t thrIdx);
           uint32_t getThrStart(uint8_t cpuIdx);
           uint64_t getThrPrepTime(uint8_t cpuIdx, uint8_t thrIdx);
               bool isThrRunning(uint8_t cpuIdx, uint8_t thrIdx);                   // true if thread <thrIdx> is running

               bool isSafeToRemove(const std::string& pattern, std::string& report, std::vector<QueueReport>& vQr, CovenantTable& ctAdditions);
               bool isSafeToRemove(const std::string& pattern, std::string& report) {CovenantTable ctAdditions; std::vector<QueueReport> vQr; return isSafeToRemove(pattern, report, vQr, ctAdditions); }
               bool isSafeToRemove(Graph& gRem, std::string& report, std::vector<QueueReport>& vQr, CovenantTable& ctAdditions);
               bool isSafeToRemove(Graph& gRem, std::string& report) {CovenantTable ctAdditions; std::vector<QueueReport> vQr; return isSafeToRemove(gRem, report, vQr, ctAdditions);}
std::pair<int, int> findRunningPattern(const std::string& sPattern); // get cpu and thread assignment of running pattern
               bool isPatternRunning(const std::string& sPattern);                  // true if Pattern <x> is running
               void updateModTime();


               void forceThrCursor(uint8_t cpuIdx, uint8_t thrIdx); //DEBUG ONLY !!!


            vEbwrs& startThr(vEbwrs& ew, uint8_t cpuIdx, uint8_t thrIdx); //Requests Thread to start
            vEbwrs& startPattern(vEbwrs& ew, const std::string& sPattern, uint8_t thrIdx, uint64_t t=0ULL); //Requests Pattern to start
            vEbwrs& startPattern(vEbwrs& ew, const std::string& sPattern, uint64_t t=0ULL); //Requests Pattern to start on first free thread
            vEbwrs& startNodeOrigin(vEbwrs& ew, const std::string& sNode, uint8_t thrIdx, uint64_t t=0ULL); //Requests thread <thrIdx> to start at node <sNode>
            vEbwrs& startNodeOrigin(vEbwrs& ew, const std::string& sNode, uint64_t t=0ULL); //Requests a start at node <sNode>
            vEbwrs& stopPattern(vEbwrs& ew, const std::string& sPattern); //Requests Pattern to stop
            vEbwrs& stopNodeOrigin(vEbwrs& ew, const std::string& sNode); //Requests stop at node <sNode> (vEbwrs& ew, flow to idle)
            vEbwrs& abortPattern(vEbwrs& ew, const std::string& sPattern); //Immediately aborts a Pattern
            vEbwrs& abortNodeOrigin(vEbwrs& ew, const std::string& sNode); //Immediately aborts the thread whose pattern <sNode> belongs to
            vEbwrs& abortThr(vEbwrs& ew, uint8_t cpuIdx, uint8_t thrIdx); //Immediately aborts a Thread
            vEbwrs& setThrStart(vEbwrs& ew, uint8_t cpuIdx, uint32_t bits); //Requests Threads to start
            vEbwrs& setThrAbort(vEbwrs& ew, uint8_t cpuIdx, uint32_t bits); //Immediately aborts Threads
            vEbwrs& setThrOrigin(vEbwrs& ew, uint8_t cpuIdx, uint8_t thrIdx, const std::string& name); //Sets the Node the Thread will start from
            vEbwrs& setThrStartTime(vEbwrs& ew, uint8_t cpuIdx, uint8_t thrIdx, uint64_t t);
            vEbwrs& setThrPrepTime(vEbwrs& ew, uint8_t cpuIdx, uint8_t thrIdx, uint64_t t);
            vEbwrs& deactivateOrphanedCommands(vEbwrs& ew, std::vector<QueueReport>& vQr);
            vEbwrs& clearHealth(vEbwrs& ew, uint8_t cpuIdx);
            vEbwrs& clearHealth(vEbwrs& ew);
            vEbwrs& resetThrMsgCnt(vEbwrs& ew, uint8_t cpuIdx, uint8_t thrIdx);
            vEbwrs& blockAsyncClearQueues(vEbwrs& ew, const std::string& sTarget);
            vEbwrs& switching(vEbwrs& ew, const std::string& sTarget, const std::string& sDst);
            vEbwrs& createNonQCommand(vEbwrs& ew, const std::string& type, const std::string& target);
            vEbwrs& createLockCtrlCommand(vEbwrs& ew, const std::string& type, const std::string& target, bool lockRd, bool lockWr );
            vEbwrs& createQCommand(vEbwrs& ew, const std::string& type, const std::string& target, uint8_t cmdPrio, uint8_t cmdQty, bool vabs, uint64_t cmdTvalid);
            vEbwrs& createWaitCommand(vEbwrs& ew, const std::string& type, const std::string& target, uint8_t cmdPrio, uint8_t cmdQty, bool vabs, uint64_t cmdTvalid, uint64_t cmdTwait, bool abswait );
            vEbwrs& createFlowCommand(vEbwrs& ew, const std::string& type, const std::string& target, const std::string& destination, uint8_t  cmdPrio, uint8_t cmdQty, bool vabs, uint64_t cmdTvalid, bool perma);
            vEbwrs& createFlushCommand(vEbwrs& ew, const std::string& type, const std::string& target, const std::string& destination, uint8_t  cmdPrio, uint8_t cmdQty, bool vabs, uint64_t cmdTvalid, bool qIl, bool qHi, bool qLo);
            vEbwrs& createFullCommand(vEbwrs& ew, const std::string& type, const std::string& target, const std::string& destination, uint8_t  cmdPrio, uint8_t cmdQty, bool vabs, uint64_t cmdTvalid, bool perma, bool qIl, bool qHi, bool qLo, uint64_t cmdTwait, bool abswait, bool lockRd, bool lockWr);
            vEbwrs& createCommandBurst(vEbwrs& ew, Graph& g);
            vEbwrs& createMiniCommand(vEbwrs& ew, const std::string& targetName, uint8_t cmdPrio, mc_ptr mc);
            vEbwrs& createCommand(vEbwrs& ew, const std::string& type, const std::string& target, const std::string& destination, uint8_t  cmdPrio, uint8_t cmdQty, bool vabs, uint64_t cmdTvalid, bool perma, bool qIl, bool qHi, bool qLo,  uint64_t cmdTwait, bool abswait, bool lockRd, bool lockWr );
                int send(vEbwrs& ew);
/*
            //convenience wrappers without eb cycle control, send immediately
            int startThr(uint8_t cpuIdx, uint8_t thrIdx)                              { return <>(static_cast<vEbwrs& (CarpeDM::*)(vEbwrs& ew, uint8_t, uint8_t)                            > this->startThr, cpuIdx, thrIdx                         );}
            int startPattern(const std::string& sPattern, uint8_t thrIdx)             { return dSend(static_cast<vEbwrs& (CarpeDM::*)(vEbwrs& ew, const std::string& sPattern, uint8_t thrIdx) > (&CarpeDM::startPattern), sPattern, thrIdx);}
            int startNodeOrigin(const std::string& sNode, uint8_t thrIdx)             { return dSend(static_cast<vEbwrs& (CarpeDM::*)(vEbwrs& ew, const std::string& sNode, uint8_t thrIdx )   > (&CarpeDM::startNodeOrigin), sNode, thrIdx );}
            int startNodeOrigin(const std::string& sNode)                             { return dSend(static_cast<vEbwrs& (CarpeDM::*)(vEbwrs& ew, const std::string& sNode)                    > (&CarpeDM::startNodeOrigin), sNode);}
            int stopPattern(const std::string& sPattern)                              { return dSend(static_cast<vEbwrs& (CarpeDM::*)(vEbwrs& ew, const std::string& sPattern                            ) > (&CarpeDM::stopPattern), sPattern                            );}
            int stopNodeOrigin(const std::string& sNode)                              { return dSend(static_cast<vEbwrs& (CarpeDM::*)(vEbwrs& ew, const std::string& sNode)                                > (&CarpeDM::stopNodeOrigin), sNode);}
            int abortNodeOrigin(const std::string& sNode)                             { return dSend(static_cast<vEbwrs& (CarpeDM::*)(vEbwrs& ew, const std::string& sNode )                               > (&CarpeDM::abortNodeOrigin), sNode );}
            int abortThr(uint8_t cpuIdx, uint8_t thrIdx)                              { return dSend(static_cast<vEbwrs& (CarpeDM::*)(vEbwrs& ew, uint8_t cpuIdx, uint8_t thrIdx                         ) > (&CarpeDM::abortThr), cpuIdx, thrIdx);}
            int setThrStart(uint8_t cpuIdx, uint32_t bits)                            { return dSend(static_cast<vEbwrs& (CarpeDM::*)(vEbwrs& ew, uint8_t cpuIdx, uint32_t bits                          ) > (&CarpeDM::setThrStart), cpuIdx,  bits );}
            int setThrAbort(uint8_t cpuIdx, uint32_t bits)                            { return dSend(static_cast<vEbwrs& (CarpeDM::*)(vEbwrs& ew, uint8_t cpuIdx, uint32_t bits                          ) > (&CarpeDM::setThrAbort), cpuIdx,  bits );}
            int setThrOrigin(uint8_t cpuIdx, uint8_t thrIdx, const std::string& name) { return dSend(static_cast<vEbwrs& (CarpeDM::*)(vEbwrs& ew, uint8_t cpuIdx, uint8_t thrIdx, const std::string& name) > (&CarpeDM::setThrOrigin), cpuIdx, thrIdx, name);}
            int setThrStartTime(uint8_t cpuIdx, uint8_t thrIdx, uint64_t t)           { return dSend(static_cast<vEbwrs& (CarpeDM::*)(vEbwrs& ew, uint8_t cpuIdx, uint8_t thrIdx, uint64_t t )             > (&CarpeDM::setThrStartTime), cpuIdx, thrIdx, t );}
            int setThrPrepTime(uint8_t cpuIdx, uint8_t thrIdx, uint64_t t)            { return dSend(static_cast<vEbwrs& (CarpeDM::*)(vEbwrs& ew, uint8_t cpuIdx, uint8_t thrIdx, uint64_t t)              > (&CarpeDM::setThrPrepTime), cpuIdx, thrIdx, t);}
            int deactivateOrphanedCommands(std::vector<QueueReport> & vQr)            { return dSend(static_cast<vEbwrs& (CarpeDM::*)(vEbwrs& ew, std::vector<QueueReport >& vQr)                         > (&CarpeDM::deactivateOrphanedCommands), std::ref(vQr));}
            int clearHealth()                                                         { return dSend(static_cast<vEbwrs& (CarpeDM::*)(vEbwrs& ew                                        )  > (&CarpeDM::clearHealth));}
            int clearHealth(uint8_t cpuIdx)                                           { return dSend(static_cast<vEbwrs& (CarpeDM::*)(vEbwrs& ew, uint8_t cpuIdx                                        )  > (&CarpeDM::clearHealth), cpuIdx);}
            int resetThrMsgCnt(uint8_t cpuIdx, uint8_t thrIdx)                        { return dSend(static_cast<vEbwrs& (CarpeDM::*)(vEbwrs& ew, uint8_t cpuIdx, uint8_t thrIdx)                          > (&CarpeDM::resetThrMsgCnt), cpuIdx, thrIdx);}
            int blockAsyncClearQueues(const std::string& sBlock)                      { return dSend(static_cast<vEbwrs& (CarpeDM::*)(vEbwrs& ew, const std::string& sBlock)                               > (&CarpeDM::blockAsyncClearQueues), sBlock);}
*/
            //FIXME workaround for flawed template approach (disambiguation of member functin pointers failing). no time to figure it out right  now, get the job done first
            //convenience wrappers without eb cycle control, send immediately
            int startThr(uint8_t cpuIdx, uint8_t thrIdx)                              ;
            int startPattern(const std::string& sPattern, uint8_t thrIdx)             ;
            int startNodeOrigin(const std::string& sNode, uint8_t thrIdx)             ;
            int startNodeOrigin(const std::string& sNode)                             ;
            int stopPattern(const std::string& sPattern)                              ;
            int stopNodeOrigin(const std::string& sNode)                              ;
            int abortPattern(const std::string& sPattern)                             ;
            int abortNodeOrigin(const std::string& sNode)                             ;
            int abortThr(uint8_t cpuIdx, uint8_t thrIdx)                              ;
            int setThrStart(uint8_t cpuIdx, uint32_t bits)                            ;
            int setThrAbort(uint8_t cpuIdx, uint32_t bits)                            ;
            int setThrOrigin(uint8_t cpuIdx, uint8_t thrIdx, const std::string& name) ;
            int setThrStartTime(uint8_t cpuIdx, uint8_t thrIdx, uint64_t t)           ;
            int setThrPrepTime(uint8_t cpuIdx, uint8_t thrIdx, uint64_t t)            ;
            int deactivateOrphanedCommands(std::vector<QueueReport> & vQr)            ;
            int clearHealth()                                                         ;
            int clearHealth(uint8_t cpuIdx)                                           ;
            int resetThrMsgCnt(uint8_t cpuIdx, uint8_t thrIdx)                        ;
            int blockAsyncClearQueues(const std::string& sBlock)                      ;


               void verboseOn()  {verbose = true;}                              // Turn on Verbose Output
               void verboseOff() {verbose = false;}                             // Turn off Verbose Output
               bool isVerbose()  const {return verbose;}                        // Tell if Output is set to Verbose
               void debugOn()  {debug = true;}                                  // Turn on Verbose Output
               void debugOff() {debug = false;}                                 // Turn off Verbose Output
               bool isDebug()  const {return debug;}                            // Tell if Output is set to Verbose
               bool isSim()  const {return sim;}                                // Tell if this is a simulation. Cannot change while connected !!!
               void testOn()  {testmode = true;}                                // Turn on Testmode
               void testOff() {testmode = false;}                               // Turn off Testmode
               bool isTest() const {return testmode;}                           // Tell if Testmode is on
               void optimisedS2ROn() {optimisedS2R = true;}                     // Optimised Safe2remove on
               void optimisedS2ROff(){optimisedS2R = false;}                    // Optimised Safe2remove off
               bool isOptimisedS2R() const {return optimisedS2R;}               // tell if Safe2remove optimisation is on or off
               bool isValidDMCpu(uint8_t cpuIdx);                               // Check if CPU is registered as running a valid firmware
      HealthReport& getHealth(uint8_t cpuIdx, HealthReport &hr);                // FIXME why reference in, reference out ? its not like you can add to this report ...
       QueueReport& getQReport(const std::string& blockName, QueueReport& qr);  // FIXME why reference in, reference out ? its not like you can add to this report ...
       std::string& getRawQReport(const std::string& blockName, std::string& report) ;
           uint64_t getDmWrTime() {return ebd.getDmWrTime();}
     HwDelayReport& getHwDelayReport(HwDelayReport& hdr);
               void clearHwDiagnostics();
               void startStopHwDiagnostics(bool enable);
               void configHwDiagnostics(uint64_t timeIntvl, uint32_t stallIntvl);
               void configFwDiagnostics(uint64_t warnThrshld);

       std::string& inspectQueues(const std::string& blockName, std::string& report);      // Show all command fields in Block Queue
               void show(const std::string& title, const std::string& logDictFile, TransferDir dir, bool filterMeta );
               void showUp(bool filterMeta);                                               // show a CPU's Upload address table
               void showDown(bool filterMeta);
               void dumpNode(const std::string& name);                     // hex dump a node
               void showPaint();
               bool isPainted(const std::string& name);
               void inspectHeap(uint8_t cpuIdx);
               void showHashDict();
               void showGroupsDict();
               bool tableCheck(std::string& report);
               void coverage3Upload(uint64_t seed );
               std::vector<std::vector<uint64_t>> coverage3TestData(uint64_t seedStart, uint64_t cases, uint8_t parts, uint8_t percentage );
             Graph& getDownGraph(); //Returns the Download Graph for CPU <cpuIdx
               void dirtyCtShow() {ct.debug(sLog);}
               void showCpuList() {return ebd.showCpuList();}
            uint8_t getCpuQty() {return ebd.getCpuQty();}
               bool isCpuIdxValid(uint8_t cpuIdx) {return ebd.isCpuIdxValid(cpuIdx);}
               void showMemSpace();
               void lockManagerClear() {lm.clear();}
               bool lockManagerHasEntries() {return (lm.getLockVec().size() > 0);}
               void softwareReset(bool clearStatistic); 



};

#endif
