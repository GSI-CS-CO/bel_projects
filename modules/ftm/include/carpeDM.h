#ifndef _CARPEDM_H_
#define _CARPEDM_H_

#include <stdio.h>
#include <iostream>
#include <string>
#include <inttypes.h>
#include <memory>
#include "common.h"
#include "graph.h"



class CarpeDM {



public:
  class CarpeDMimpl;
  CarpeDM();
  CarpeDM(std::ostream& sLog);
  CarpeDM(std::ostream& sLog, std::ostream& sErr);
  ~CarpeDM();

// Etherbone interface
               bool connect(const std::string& en, bool simulation=false, bool test=false); //Open connection to a DM via Etherbone
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
           uint64_t getModTime();

// Text File IO /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
               void writeTextFile(const std::string& fn, const std::string& s);
        std::string readTextFile(const std::string& fn);

               void completeId(vertex_t v, Graph& g);
// Graphs to Dot
             Graph& parseDot(const std::string& dotString, Graph& g); //Parse a .dot string to create unprocessed Graph
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
               //bool isSafeToRemove(const std::string& pattern, std::string& report, std::vector<QueueReport>& vQr);
               bool isSafeToRemove(const std::string& pattern, std::string& report) ;
std::pair<int, int> findRunningPattern(const std::string& sPattern); // get cpu and thread assignment of running pattern
               bool isPatternRunning(const std::string& sPattern);                  // true if Pattern <x> is running
               void updateModTime();


               void forceThrCursor(uint8_t cpuIdx, uint8_t thrIdx); //DEBUG ONLY !!!


            vEbwrs& startThr(vEbwrs& ew, uint8_t cpuIdx, uint8_t thrIdx); //Requests Thread to start
            vEbwrs& startPattern(vEbwrs& ew, const std::string& sPattern, uint8_t thrIdx); //Requests Pattern to start
            vEbwrs& startPattern(vEbwrs& ew, const std::string& sPattern); //Requests Pattern to start on first free thread
            vEbwrs& startNodeOrigin(vEbwrs& ew, const std::string& sNode, uint8_t thrIdx); //Requests thread <thrIdx> to start at node <sNode>
            vEbwrs& startNodeOrigin(vEbwrs& ew, const std::string& sNode); //Requests a start at node <sNode>
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
            vEbwrs& createNonQCommand(vEbwrs& ew, const std::string& type, const std::string& target, uint8_t cmdThr);
            vEbwrs& createLockCtrlCommand(vEbwrs& ew, const std::string& type, const std::string& target, bool lockRd, bool lockWr );
            vEbwrs& createQCommand(vEbwrs& ew, const std::string& type, const std::string& target, uint8_t cmdPrio, uint8_t cmdQty, bool vabs, uint64_t cmdTvalid, uint8_t cmdThr);
            vEbwrs& createWaitCommand(vEbwrs& ew, const std::string& type, const std::string& target, uint8_t cmdPrio, uint8_t cmdQty, bool vabs, uint64_t cmdTvalid, uint64_t cmdTwait, bool abswait );
            vEbwrs& createFlowCommand(vEbwrs& ew, const std::string& type, const std::string& target, const std::string& destination, uint8_t  cmdPrio, uint8_t cmdQty, bool vabs, uint64_t cmdTvalid, bool perma);
            vEbwrs& createFlushCommand(vEbwrs& ew, const std::string& type, const std::string& target, const std::string& destination, uint8_t  cmdPrio, uint8_t cmdQty, bool vabs, uint64_t cmdTvalid, bool qIl, bool qHi, bool qLo);
            vEbwrs& createFullCommand(vEbwrs& ew, const std::string& type, const std::string& target, const std::string& destination, uint8_t  cmdPrio, uint8_t cmdQty, bool vabs, uint64_t cmdTvalid, bool perma, bool qIl, bool qHi, bool qLo, uint64_t cmdTwait, bool abswait, bool lockRd, bool lockWr, uint8_t cmdThr);
            vEbwrs& createCommand(vEbwrs& ew, const std::string& type, const std::string& target, const std::string& destination, uint8_t  cmdPrio, uint8_t cmdQty, bool vabs, uint64_t cmdTvalid, bool perma, bool qIl, bool qHi, bool qLo,  uint64_t cmdTwait, bool abswait, bool lockRd, bool lockWr, uint8_t cmdThr );
                int send(vEbwrs& ew);
            //FIXME workaround for flawed template approach (disambiguation of member functin pointers failing). no time to figure it out right  now, get the job done first
            //convenience wrappers without eb cycle control, send immediately
            int startThr(uint8_t cpuIdx, uint8_t thrIdx)                              ;
            int startPattern(const std::string& sPattern, uint8_t thrIdx)             ;
            int startNodeOrigin(const std::string& sNode, uint8_t thrIdx)             ;
//            int startNodeOrigin(const std::string& sNode)                             ;
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


               void verboseOn();                             // Turn on Verbose Output
               void verboseOff();                            // Turn off Verbose Output
               bool isVerbose()  const;                       // Tell if Output is set to Verbose
               void debugOn();                                 // Turn on Verbose Output
               void debugOff();                                // Turn off Verbose Output
               bool isDebug()  const;                            // Tell if Output is set to Verbose
               bool isSim()  const;                               // Tell if this is a simulation. Cannot change while connected !!!
               void testOn();                               // Turn on Testmode
               void testOff();                              // Turn off Testmode
               bool isTest() const;                          // Tell if Testmode is on
               void optimisedS2ROn();                    // Optimised Safe2remove on
               void optimisedS2ROff();                   // Optimised Safe2remove off
               bool isOptimisedS2R() const;                                     // tell if Safe2remove optimisation is on or off
               bool isValidDMCpu(uint8_t cpuIdx);                               // Check if CPU is registered as running a valid firmware
      HealthReport& getHealth(uint8_t cpuIdx, HealthReport &hr);                // FIXME why reference in, reference out ? its not like you can add to this report ...
       QueueReport& getQReport(const std::string& blockName, QueueReport& qr);  // FIXME why reference in, reference out ? its not like you can add to this report ...
       std::string& getRawQReport(const std::string& blockName, std::string& report) ;
           uint64_t getDmWrTime();
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
               void dirtyCtShow();
               void showCpuList() ;
            uint8_t getCpuQty() ;
            uint8_t getThrQty() ;
            uint32_t getCtlAdr(const uint8_t& idx);
               bool isCpuIdxValid(uint8_t cpuIdx) ;
               void showMemSpace();
               void lockManagerClear();
               bool lockManagerHasEntries();
               void softwareReset(bool clearStatistic);

  private:
  
    std::unique_ptr<CarpeDMimpl> impl_;               
};

#endif
