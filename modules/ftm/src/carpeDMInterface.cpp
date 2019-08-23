#include "carpeDMInterface.h"
#include "carpeDM.h"

  CarpeDMInterface::CarpeDMInterface() : impl_(new CarpeDM()) {}
  CarpeDMInterface::CarpeDMInterface(std::ostream& sLog) : impl_(new CarpeDM(sLog)) {}                    
  CarpeDMInterface::CarpeDMInterface(std::ostream& sLog, std::ostream& sErr) : impl_(new CarpeDM(sLog, sErr)) {}
  CarpeDMInterface::~CarpeDMInterface() = default;

// Etherbone interface
  bool CarpeDMInterface::connect(const std::string& en, bool simulation, bool test)             {return impl_->connect(en, simulation, test);} //Open connection to a DM via Etherbone
  bool CarpeDMInterface::disconnect()                                                           {return impl_->disconnect();} //Close connection
               // SDB and DM HW detection Functions


//Internal Hash and Groupstable ///////////////////////////////////////////////////////////////////////////////////////////////
               // Name/Hash Dict
  void CarpeDMInterface::clearHashDict()                                                        { return impl_->clearHashDict();}                          //Clear hash table
  std::string CarpeDMInterface::storeHashDict()                                                 { return impl_->storeHashDict();}                          //save hash table to serialised string
  void CarpeDMInterface::loadHashDict(const std::string& s)                                     { return impl_->loadHashDict(s);}      //initiallise hash table from serialised string
  void CarpeDMInterface::storeHashDictFile(const std::string& fn)                               { return impl_->storeHashDictFile(fn);} //save hash table to file
  void CarpeDMInterface::loadHashDictFile(const std::string& fn)                                { return impl_->loadHashDictFile(fn);}  //load hash table from file
  bool CarpeDMInterface::isInHashDict(const uint32_t hash)                                      { return impl_->isInHashDict(hash);}
  bool CarpeDMInterface::isInHashDict(const std::string& name)                                  { return impl_->isInHashDict(name);}
  bool CarpeDMInterface::isHashDictEmpty()                                                      { return impl_->isHashDictEmpty();}
  int CarpeDMInterface::getHashDictSize()                                                       { return impl_->getHashDictSize();}

               // Group/Entry/Exit Table
        std::string CarpeDMInterface::storeGroupsDict()                                         { return impl_->storeGroupsDict();}
  void CarpeDMInterface::loadGroupsDict(const std::string& s)                                   { return impl_->loadGroupsDict(s);}
  void CarpeDMInterface::storeGroupsDictFile(const std::string& fn)                             { return impl_->storeGroupsDictFile(fn);}
  void CarpeDMInterface::loadGroupsDictFile(const std::string& fn)                              { return impl_->loadGroupsDictFile(fn);}
  void CarpeDMInterface::clearGroupsDict()                                                      { return impl_->clearGroupsDict();} //Clear pattern table
  int CarpeDMInterface::getGroupsSize()                                                         { return impl_->getGroupsSize();}

// Aux Infos from Table lookups and static computation  /////////////////////////////////////////////////////////////////////////////////////////////////////
  uint8_t CarpeDMInterface::getNodeCpu(const std::string& name, TransferDir dir)                { return impl_->getNodeCpu(name, dir);}               // shortcut to obtain a node's cpu by its name
  uint32_t CarpeDMInterface::getNodeAdr(const std::string& name, TransferDir dir, AdrType adrT) { return impl_->getNodeAdr(name, dir, adrT);} // shortcut to obtain a node's address by its name
  const std::string CarpeDMInterface::getNodePattern (const std::string& sNode)                 { return impl_->getNodePattern (sNode);}
  const std::string CarpeDMInterface::getNodeBeamproc(const std::string& sNode)                 { return impl_->getNodeBeamproc(sNode);}
  vStrC CarpeDMInterface::getPatternMembers (const std::string& sPattern)                       { return impl_->getPatternMembers(sPattern);}
  const std::string CarpeDMInterface::getPatternEntryNode(const std::string& sPattern)          { return impl_->getPatternEntryNode(sPattern);}
  const std::string CarpeDMInterface::getPatternExitNode(const std::string& sPattern)           { return impl_->getPatternExitNode(sPattern);}
  vStrC CarpeDMInterface::getBeamprocMembers(const std::string& sBeamproc)                      { return impl_->getBeamprocMembers(sBeamproc);}
  const std::string CarpeDMInterface::getBeamprocEntryNode(const std::string& sBeamproc)        { return impl_->getBeamprocEntryNode(sBeamproc);}
  const std::string CarpeDMInterface::getBeamprocExitNode(const std::string& sBeamproc)         { return impl_->getBeamprocExitNode(sBeamproc);}
  uint64_t CarpeDMInterface::getModTime()                                                       { return impl_->getModTime();}

// Text File IO /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void CarpeDMInterface::writeTextFile(const std::string& fn, const std::string& s)             { return impl_->writeTextFile(fn, s);}
  std::string CarpeDMInterface::readTextFile(const std::string& fn)                             { return impl_->readTextFile(fn);}

// Graphs to Dot
  void CarpeDMInterface::writeDownDotFile(const std::string& fn, bool filterMeta)               { return impl_->writeDownDotFile(fn, filterMeta);}
  void CarpeDMInterface::writeUpDotFile(const std::string& fn, bool filterMeta)                 { return impl_->writeUpDotFile(fn, filterMeta);}

// Schedule Manipulation ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  int CarpeDMInterface::assignNodesToCpus()                                                     { return impl_->assignNodesToCpus();}                            // NOT YET IMPLEMENTED // TODO assign a cpu to each node object. Currently taken from input .dot
  int CarpeDMInterface::download()                                                              { return impl_->download();}                                     // Download binary from LM32 SoC and create Graph
  std::string CarpeDMInterface::downloadDot(bool filterMeta)                                    { return impl_->downloadDot(filterMeta);}
  void CarpeDMInterface::downloadDotFile(const std::string& fn, bool filterMeta)                { return impl_->downloadDotFile(fn, filterMeta);}
  int CarpeDMInterface::addDot(const std::string& s, bool force)                                { return impl_->addDot(s, force);}                   // add all nodes and/or edges in dot file
  int CarpeDMInterface::addDotFile(const std::string& fn, bool force)                           { return impl_->addDotFile(fn, force);}
  int CarpeDMInterface::overwriteDot(const std::string& s, bool force)                          { return impl_->overwriteDot(s, force);} // add all nodes and/or edges in dot file
  int CarpeDMInterface::overwriteDotFile(const std::string& fn, bool force)                     { return impl_->overwriteDotFile(fn, force);}
  int CarpeDMInterface::keepDot(const std::string& s, bool force)                               { return impl_->keepDot(s, force);}      // removes all nodes NOT in input file
  int CarpeDMInterface::keepDotFile(const std::string& fn, bool force)                          { return impl_->keepDotFile(fn, force);}
  int CarpeDMInterface::removeDot(const std::string& s, bool force)                             { return impl_->removeDot(s, force);}    // removes all nodes in input file
  int CarpeDMInterface::removeDotFile(const std::string& fn, bool force)                        { return impl_->removeDotFile(fn, force);}
  int CarpeDMInterface::clear(bool force)                                                       { return impl_->clear(force);}                              // clears all nodes from DM

// Command Generation and Dispatch ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
vStrC CarpeDMInterface::getLockedBlocks(bool checkReadLock, bool checkWriteLock)                                         { return impl_->getLockedBlocks(checkReadLock, checkWriteLock);}
int CarpeDMInterface::sendCommandsDot(const std::string& s)                                                              { return impl_->sendCommandsDot(s);} //Sends a dotfile of commands to the DM
int CarpeDMInterface::sendCommandsDotFile(const std::string& fn)                                                         { return impl_->sendCommandsDotFile(fn);}

void CarpeDMInterface::halt()                                                                                            { return impl_->halt();}
int CarpeDMInterface::staticFlushPattern(const std::string& sPattern, bool prioIl, bool prioHi, bool prioLo, bool force) { return impl_->staticFlushPattern(sPattern, prioIl, prioHi, prioLo, force);}
int CarpeDMInterface::staticFlushBlock(const std::string& sBlock, bool prioIl, bool prioHi, bool prioLo, bool force)     { return impl_->staticFlushBlock(sBlock, prioIl, prioHi, prioLo, force);}


  // Short Live Infos from DM hardware reads /////////////////////////////////////////////////////////////////////////////////////////////////////////////
const std::string CarpeDMInterface::getThrOrigin(uint8_t cpuIdx, uint8_t thrIdx)                                         { return impl_->getThrOrigin(cpuIdx, thrIdx);}      // Returns the Node the Thread will start from
const std::string CarpeDMInterface::getThrCursor(uint8_t cpuIdx, uint8_t thrIdx)                                         { return impl_->getThrCursor(cpuIdx, thrIdx);}      // Returns the Node the Thread is currently processing
uint64_t CarpeDMInterface::getThrMsgCnt(uint8_t cpuIdx, uint8_t thrIdx)                                                  { return impl_->getThrMsgCnt(cpuIdx, thrIdx);}
uint32_t CarpeDMInterface::getThrRun(uint8_t cpuIdx)                                                                     { return impl_->getThrRun(cpuIdx);}                                  // Get bitfield showing running threads
uint32_t CarpeDMInterface::getStatus(uint8_t cpuIdx)                                                                     { return impl_->getStatus(cpuIdx);}
uint64_t CarpeDMInterface::getThrDeadline(uint8_t cpuIdx, uint8_t thrIdx)                                                { return impl_->getThrDeadline(cpuIdx, thrIdx);}
uint64_t CarpeDMInterface::getThrStartTime(uint8_t cpuIdx, uint8_t thrIdx)                                               { return impl_->getThrStartTime(cpuIdx, thrIdx);}
uint32_t CarpeDMInterface::getThrStart(uint8_t cpuIdx)                                                                   { return impl_->getThrStart(cpuIdx);}
uint64_t CarpeDMInterface::getThrPrepTime(uint8_t cpuIdx, uint8_t thrIdx)                                                { return impl_->getThrPrepTime(cpuIdx, thrIdx);}
bool CarpeDMInterface::isThrRunning(uint8_t cpuIdx, uint8_t thrIdx)                                                      { return impl_->isThrRunning(cpuIdx, thrIdx);}                   // true if thread <thrIdx> is running
bool CarpeDMInterface::isSafeToRemove(const std::string& pattern, std::string& report, std::vector<QueueReport>& vQr)    { return impl_->isSafeToRemove(pattern, report, vQr);}
bool CarpeDMInterface::isSafeToRemove(const std::string& pattern, std::string& report)                                   { return impl_->isSafeToRemove(pattern, report);}
std::pair<int, int> CarpeDMInterface::findRunningPattern(const std::string& sPattern)                                    { return impl_->findRunningPattern(sPattern);} // get cpu and thread assignment of running pattern
bool CarpeDMInterface::isPatternRunning(const std::string& sPattern)                                                     { return impl_->isPatternRunning(sPattern);}                  // true if Pattern <x> is running
void CarpeDMInterface::updateModTime()                                                                                   { return impl_->updateModTime();}


void CarpeDMInterface::forceThrCursor(uint8_t cpuIdx, uint8_t thrIdx)                                                    { return impl_->forceThrCursor(cpuIdx, thrIdx);} //DEBUG ONLY !!!


vEbwrs& CarpeDMInterface::startThr(vEbwrs& ew, uint8_t cpuIdx, uint8_t thrIdx)                                           { return impl_->startThr(ew, cpuIdx, thrIdx);} //Requests Thread to start
vEbwrs& CarpeDMInterface::startPattern(vEbwrs& ew, const std::string& sPattern, uint8_t thrIdx)                          { return impl_->startPattern(ew, sPattern, thrIdx);} //Requests Pattern to start
vEbwrs& CarpeDMInterface::startPattern(vEbwrs& ew, const std::string& sPattern)                                          { return impl_->startPattern(ew, sPattern);} //Requests Pattern to start on first free thread
vEbwrs& CarpeDMInterface::startNodeOrigin(vEbwrs& ew, const std::string& sNode, uint8_t thrIdx)                          { return impl_->startNodeOrigin(ew, sNode, thrIdx);} //Requests thread <thrIdx> to start at node <sNode>
vEbwrs& CarpeDMInterface::startNodeOrigin(vEbwrs& ew, const std::string& sNode)                                          { return impl_->startNodeOrigin(ew, sNode);} //Requests a start at node <sNode>
vEbwrs& CarpeDMInterface::stopPattern(vEbwrs& ew, const std::string& sPattern)                                           { return impl_->stopPattern(ew, sPattern);} //Requests Pattern to stop
vEbwrs& CarpeDMInterface::stopNodeOrigin(vEbwrs& ew, const std::string& sNode)                                           { return impl_->stopNodeOrigin(ew, sNode);} //Requests stop at node <sNode> (vEbwrs& ew, flow to idle)
vEbwrs& CarpeDMInterface::abortPattern(vEbwrs& ew, const std::string& sPattern)                                          { return impl_->abortPattern(ew, sPattern);} //Immediately aborts a Pattern
vEbwrs& CarpeDMInterface::abortNodeOrigin(vEbwrs& ew, const std::string& sNode)                                          { return impl_->abortNodeOrigin(ew, sNode);} //Immediately aborts the thread whose pattern <sNode> belongs to
vEbwrs& CarpeDMInterface::abortThr(vEbwrs& ew, uint8_t cpuIdx, uint8_t thrIdx)                                           { return impl_->abortThr(ew, cpuIdx, thrIdx);} //Immediately aborts a Thread
vEbwrs& CarpeDMInterface::setThrStart(vEbwrs& ew, uint8_t cpuIdx, uint32_t bits)                                         { return impl_->setThrStart(ew, cpuIdx, bits);} //Requests Threads to start
vEbwrs& CarpeDMInterface::setThrAbort(vEbwrs& ew, uint8_t cpuIdx, uint32_t bits)                                         { return impl_->setThrAbort(ew, cpuIdx, bits);} //Immediately aborts Threads
vEbwrs& CarpeDMInterface::setThrOrigin(vEbwrs& ew, uint8_t cpuIdx, uint8_t thrIdx, const std::string& name)              { return impl_->setThrOrigin(ew, cpuIdx, thrIdx,  name);} //Sets the Node the Thread will start from
vEbwrs& CarpeDMInterface::setThrStartTime(vEbwrs& ew, uint8_t cpuIdx, uint8_t thrIdx, uint64_t t)                        { return impl_->setThrStartTime(ew, cpuIdx, thrIdx, t);}
vEbwrs& CarpeDMInterface::setThrPrepTime(vEbwrs& ew, uint8_t cpuIdx, uint8_t thrIdx, uint64_t t)                         { return impl_->setThrPrepTime(ew, cpuIdx, thrIdx, t);}
vEbwrs& CarpeDMInterface::deactivateOrphanedCommands(vEbwrs& ew, std::vector<QueueReport>& vQr)                          { return impl_->deactivateOrphanedCommands(ew, vQr);}
vEbwrs& CarpeDMInterface::clearHealth(vEbwrs& ew, uint8_t cpuIdx)                                                        { return impl_->clearHealth(ew, cpuIdx);}
vEbwrs& CarpeDMInterface::clearHealth(vEbwrs& ew)                                                                        { return impl_->clearHealth(ew);}
vEbwrs& CarpeDMInterface::resetThrMsgCnt(vEbwrs& ew, uint8_t cpuIdx, uint8_t thrIdx)                                     { return impl_->resetThrMsgCnt(ew, cpuIdx, thrIdx);}
vEbwrs& CarpeDMInterface::blockAsyncClearQueues(vEbwrs& ew, const std::string& sTarget)                                  { return impl_->blockAsyncClearQueues(ew, sTarget);}
vEbwrs& CarpeDMInterface::switching(vEbwrs& ew, const std::string& sTarget, const std::string& sDst)                     { return impl_->switching(ew, sTarget, sDst);}
vEbwrs& CarpeDMInterface::createNonQCommand(vEbwrs& ew, const std::string& type, const std::string& target)              { return impl_->createNonQCommand(ew, type,  target);}
            


vEbwrs& CarpeDMInterface::createLockCtrlCommand(vEbwrs& ew, const std::string& type, const std::string& target, bool lockRd, bool lockWr )
{ return impl_->createLockCtrlCommand(ew, type, target, lockRd, lockWr );}

vEbwrs& CarpeDMInterface::createQCommand(vEbwrs& ew, const std::string& type, const std::string& target, uint8_t cmdPrio, uint8_t cmdQty, bool vabs, uint64_t cmdTvalid)
{ return impl_->createQCommand(ew, type, target, cmdPrio, cmdQty, vabs, cmdTvalid);}

vEbwrs& CarpeDMInterface::createWaitCommand(vEbwrs& ew, const std::string& type, const std::string& target, uint8_t cmdPrio, uint8_t cmdQty, bool vabs, uint64_t cmdTvalid, uint64_t cmdTwait, bool abswait )
{ return impl_->createWaitCommand(ew, type, target, cmdPrio, cmdQty, vabs, cmdTvalid, cmdTwait, abswait );}

vEbwrs& CarpeDMInterface::createFlowCommand(vEbwrs& ew, const std::string& type, const std::string& target, const std::string& destination, uint8_t  cmdPrio, uint8_t cmdQty, bool vabs, uint64_t cmdTvalid, bool perma)
{ return impl_->createFlowCommand(ew, type, target, destination, cmdPrio, cmdQty, vabs, cmdTvalid, perma);}

vEbwrs& CarpeDMInterface::createFlushCommand(vEbwrs& ew, const std::string& type, const std::string& target, const std::string& destination, uint8_t  cmdPrio, uint8_t cmdQty, bool vabs, uint64_t cmdTvalid, bool qIl, bool qHi, bool qLo)
{ return impl_->createFlushCommand(ew, type, target, destination, cmdPrio, cmdQty, vabs, cmdTvalid, qIl, qHi, qLo);}

vEbwrs& CarpeDMInterface::createFullCommand(vEbwrs& ew, const std::string& type, const std::string& target, const std::string& destination, uint8_t  cmdPrio, uint8_t cmdQty, bool vabs, uint64_t cmdTvalid, bool perma, bool qIl, bool qHi, bool qLo, uint64_t cmdTwait, bool abswait, bool lockRd, bool lockWr)
{ return impl_->createFullCommand(ew, type, target, destination, cmdPrio, cmdQty, vabs, cmdTvalid, perma, qIl, qHi, qLo, cmdTwait, abswait, lockRd, lockWr);}

vEbwrs& CarpeDMInterface::createCommand(vEbwrs& ew, const std::string& type, const std::string& target, const std::string& destination, uint8_t  cmdPrio, uint8_t cmdQty, bool vabs, uint64_t cmdTvalid, bool perma, bool qIl, bool qHi, bool qLo,  uint64_t cmdTwait, bool abswait, bool lockRd, bool lockWr )
{ return impl_->createCommand(ew, type, target, destination, cmdPrio, cmdQty, vabs, cmdTvalid, perma,qIl, qHi, qLo, cmdTwait, abswait, lockRd, lockWr );}
                

                int CarpeDMInterface::send(vEbwrs& ew) {return impl_->send(ew);}
            //FIXME workaround for flawed template approach (disambiguation of member functin pointers failing). no time to figure it out right  now, get the job done first
            //convenience wrappers without eb cycle control, send immediately
            int CarpeDMInterface::startThr(uint8_t cpuIdx, uint8_t thrIdx)                               {return impl_->startThr(cpuIdx, thrIdx);}
            int CarpeDMInterface::startPattern(const std::string& sPattern, uint8_t thrIdx)              {return impl_->startPattern(sPattern, thrIdx);}
            int CarpeDMInterface::startNodeOrigin(const std::string& sNode, uint8_t thrIdx)              {return impl_->startNodeOrigin(sNode, thrIdx);}
            int CarpeDMInterface::startNodeOrigin(const std::string& sNode)                              {return impl_->startNodeOrigin(sNode);}
            int CarpeDMInterface::stopPattern(const std::string& sPattern)                               {return impl_->stopPattern(sPattern);}
            int CarpeDMInterface::stopNodeOrigin(const std::string& sNode)                               {return impl_->stopNodeOrigin(sNode);}
            int CarpeDMInterface::abortPattern(const std::string& sPattern)                              {return impl_->abortPattern(sPattern);}
            int CarpeDMInterface::abortNodeOrigin(const std::string& sNode)                              {return impl_->abortNodeOrigin(sNode);}
            int CarpeDMInterface::abortThr(uint8_t cpuIdx, uint8_t thrIdx)                               {return impl_->abortThr(cpuIdx, thrIdx);}
            int CarpeDMInterface::setThrStart(uint8_t cpuIdx, uint32_t bits)                             {return impl_->setThrStart(cpuIdx, bits);}
            int CarpeDMInterface::setThrAbort(uint8_t cpuIdx, uint32_t bits)                             {return impl_->setThrAbort(cpuIdx, bits);}
            int CarpeDMInterface::setThrOrigin(uint8_t cpuIdx, uint8_t thrIdx, const std::string& name)  {return impl_->setThrOrigin(cpuIdx, thrIdx, name);}
            int CarpeDMInterface::setThrStartTime(uint8_t cpuIdx, uint8_t thrIdx, uint64_t t)            {return impl_->setThrStartTime(cpuIdx, thrIdx, t);}
            int CarpeDMInterface::setThrPrepTime(uint8_t cpuIdx, uint8_t thrIdx, uint64_t t)             {return impl_->setThrPrepTime(cpuIdx, thrIdx, t);}
            int CarpeDMInterface::deactivateOrphanedCommands(std::vector<QueueReport> & vQr)             {return impl_->deactivateOrphanedCommands(vQr);}
            int CarpeDMInterface::clearHealth()                                                          {return impl_->clearHealth();}
            int CarpeDMInterface::clearHealth(uint8_t cpuIdx)                                            {return impl_->clearHealth(cpuIdx);}
            int CarpeDMInterface::resetThrMsgCnt(uint8_t cpuIdx, uint8_t thrIdx)                         {return impl_->resetThrMsgCnt(cpuIdx, thrIdx);}
            int CarpeDMInterface::blockAsyncClearQueues(const std::string& sBlock)                       {return impl_->blockAsyncClearQueues(sBlock);}


     void CarpeDMInterface::verboseOn()                                                              { return impl_->verboseOn();}                             // Turn on Verbose Output
     void CarpeDMInterface::verboseOff()                                                             { return impl_->verboseOff();}                            // Turn off Verbose Output
     bool CarpeDMInterface::isVerbose()  const                                                       { return impl_->isVerbose();}                       // Tell if Output is set to Verbose
     void CarpeDMInterface::debugOn()                                                                { return impl_->debugOn();}                                 // Turn on Verbose Output
     void CarpeDMInterface::debugOff()                                                               { return impl_->debugOff();}                                // Turn off Verbose Output
     bool CarpeDMInterface::isDebug()  const                                                         { return impl_->isDebug();}                            // Tell if Output is set to Verbose
     bool CarpeDMInterface::isSim()  const                                                           { return impl_->isSim();}                               // Tell if this is a simulation. Cannot change while connected !!!
     void CarpeDMInterface::testOn()                                                                 { return impl_->testOn();}                               // Turn on Testmode
     void CarpeDMInterface::testOff()                                                                { return impl_->testOff();}                              // Turn off Testmode
     bool CarpeDMInterface::isTest() const                                                           { return impl_->isTest();}                          // Tell if Testmode is on
     void CarpeDMInterface::optimisedS2ROn()                                                         { return impl_->optimisedS2ROn();}                    // Optimised Safe2remove on
     void CarpeDMInterface::optimisedS2ROff()                                                        { return impl_->optimisedS2ROff();}                   // Optimised Safe2remove off
     bool CarpeDMInterface::isOptimisedS2R() const                                                   { return impl_->isOptimisedS2R();}                                     // tell if Safe2remove optimisation is on or off
     bool CarpeDMInterface::isValidDMCpu(uint8_t cpuIdx)                                             { return impl_->isValidDMCpu(cpuIdx);}                               // Check if CPU is registered as running a valid firmware
     HealthReport& CarpeDMInterface::getHealth(uint8_t cpuIdx, HealthReport &hr)                     { return impl_->getHealth(cpuIdx, hr);}                // FIXME why reference in, reference out ? its not like you can add to this report ...
     QueueReport& CarpeDMInterface::getQReport(const std::string& blockName, QueueReport& qr)        { return impl_->getQReport(blockName, qr);}  // FIXME why reference in, reference out ? its not like you can add to this report ...
     std::string& CarpeDMInterface::getRawQReport(const std::string& blockName, std::string& report) { return impl_->getRawQReport(blockName, report);}
     uint64_t CarpeDMInterface::getDmWrTime()                                                        { return impl_->getDmWrTime();}
     HwDelayReport& CarpeDMInterface::getHwDelayReport(HwDelayReport& hdr)                           { return impl_->getHwDelayReport(hdr);}
     void CarpeDMInterface::clearHwDiagnostics()                                                     { return impl_->clearHwDiagnostics();}
     void CarpeDMInterface::startStopHwDiagnostics(bool enable)                                      { return impl_->startStopHwDiagnostics(enable);}
     void CarpeDMInterface::configHwDiagnostics(uint64_t timeIntvl, uint32_t stallIntvl)             { return impl_->configHwDiagnostics(timeIntvl, stallIntvl);}
     void CarpeDMInterface::configFwDiagnostics(uint64_t warnThrshld)                                { return impl_->configFwDiagnostics(warnThrshld);}

       std::string& CarpeDMInterface::inspectQueues(const std::string& blockName, std::string& report) {return impl_->inspectQueues(blockName, report);}      // Show all command fields in Block Queue
               void CarpeDMInterface::show(const std::string& title, const std::string& logDictFile, TransferDir dir, bool filterMeta ) {return impl_->show(title, logDictFile, dir, filterMeta );}
            void CarpeDMInterface::showUp(bool filterMeta)            { return impl_->showUp(filterMeta);}                                               // show a CPU's Upload address table
            void CarpeDMInterface::showDown(bool filterMeta)          { return impl_->showDown(filterMeta);}
            void CarpeDMInterface::dumpNode(const std::string& name)  { return impl_->dumpNode(name);}                     // hex dump a node
            void CarpeDMInterface::showPaint()                        { return impl_->showPaint();}
            bool CarpeDMInterface::isPainted(const std::string& name) { return impl_->isPainted(name);}
            void CarpeDMInterface::inspectHeap(uint8_t cpuIdx)        { return impl_->inspectHeap(cpuIdx);}
            void CarpeDMInterface::showHashDict()                     { return impl_->showHashDict();}
            void CarpeDMInterface::showGroupsDict()                   { return impl_->showGroupsDict();}
            bool CarpeDMInterface::tableCheck(std::string& report)    { return impl_->tableCheck(report);}
            void CarpeDMInterface::coverage3Upload(uint64_t seed )    { return impl_->coverage3Upload(seed );}
            void CarpeDMInterface::dirtyCtShow()                      { return impl_->dirtyCtShow();}
            void CarpeDMInterface::showCpuList()                      { return impl_->showCpuList();}
         uint8_t CarpeDMInterface::getCpuQty()                        { return impl_->getCpuQty();}
            bool CarpeDMInterface::isCpuIdxValid(uint8_t cpuIdx)      { return impl_->isCpuIdxValid(cpuIdx);}
            void CarpeDMInterface::showMemSpace()                     { return impl_->showMemSpace();}
            void CarpeDMInterface::lockManagerClear()                 { return impl_->lockManagerClear();}
            bool CarpeDMInterface::lockManagerHasEntries()            { return impl_->lockManagerHasEntries();}
            void CarpeDMInterface::softwareReset(bool clearStatistic) { return impl_->softwareReset(clearStatistic);}
               std::vector<std::vector<uint64_t>> CarpeDMInterface::coverage3TestData(uint64_t seedStart, uint64_t cases, uint8_t parts, uint8_t percentage ) {return impl_->coverage3TestData(seedStart, cases, parts, percentage );}











                             

