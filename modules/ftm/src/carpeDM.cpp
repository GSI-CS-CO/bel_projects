#include "carpeDM.h"
#include "carpeDMimpl.h"

  CarpeDM::CarpeDM() : impl_(new CarpeDMimpl()) {}
  CarpeDM::CarpeDM(std::ostream& sLog) : impl_(new CarpeDMimpl(sLog)) {}
  CarpeDM::CarpeDM(std::ostream& sLog, std::ostream& sErr) : impl_(new CarpeDMimpl(sLog, sErr)) {}
  CarpeDM::~CarpeDM() = default;

// Etherbone interface
  bool CarpeDM::connect(const std::string& en, bool simulation, bool test)             {return impl_->connect(en, simulation, test);} //Open connection to a DM via Etherbone
  bool CarpeDM::disconnect()                                                           {return impl_->disconnect();} //Close connection
               // SDB and DM HW detection Functions


//Internal Hash and Groupstable ///////////////////////////////////////////////////////////////////////////////////////////////
               // Name/Hash Dict
  void CarpeDM::clearHashDict()                                                        { return impl_->clearHashDict();}                          //Clear hash table
  std::string CarpeDM::storeHashDict()                                                 { return impl_->storeHashDict();}                          //save hash table to serialised string
  void CarpeDM::loadHashDict(const std::string& s)                                     { return impl_->loadHashDict(s);}      //initiallise hash table from serialised string
  void CarpeDM::storeHashDictFile(const std::string& fn)                               { return impl_->storeHashDictFile(fn);} //save hash table to file
  void CarpeDM::loadHashDictFile(const std::string& fn)                                { return impl_->loadHashDictFile(fn);}  //load hash table from file
  bool CarpeDM::isInHashDict(const uint32_t hash)                                      { return impl_->isInHashDict(hash);}
  bool CarpeDM::isInHashDict(const std::string& name)                                  { return impl_->isInHashDict(name);}
  bool CarpeDM::isHashDictEmpty()                                                      { return impl_->isHashDictEmpty();}
  int CarpeDM::getHashDictSize()                                                       { return impl_->getHashDictSize();}

               // Group/Entry/Exit Table
        std::string CarpeDM::storeGroupsDict()                                         { return impl_->storeGroupsDict();}
  void CarpeDM::loadGroupsDict(const std::string& s)                                   { return impl_->loadGroupsDict(s);}
  void CarpeDM::storeGroupsDictFile(const std::string& fn)                             { return impl_->storeGroupsDictFile(fn);}
  void CarpeDM::loadGroupsDictFile(const std::string& fn)                              { return impl_->loadGroupsDictFile(fn);}
  void CarpeDM::clearGroupsDict()                                                      { return impl_->clearGroupsDict();} //Clear pattern table
  int CarpeDM::getGroupsSize()                                                         { return impl_->getGroupsSize();}

// Aux Infos from Table lookups and static computation  /////////////////////////////////////////////////////////////////////////////////////////////////////
  uint8_t CarpeDM::getNodeCpu(const std::string& name, TransferDir dir)                { return impl_->getNodeCpu(name, dir);}               // shortcut to obtain a node's cpu by its name
  uint32_t CarpeDM::getNodeAdr(const std::string& name, TransferDir dir, AdrType adrT) { return impl_->getNodeAdr(name, dir, adrT);} // shortcut to obtain a node's address by its name
  const std::string CarpeDM::getNodePattern (const std::string& sNode)                 { return impl_->getNodePattern (sNode);}
  const std::string CarpeDM::getNodeBeamproc(const std::string& sNode)                 { return impl_->getNodeBeamproc(sNode);}
  vStrC CarpeDM::getPatternMembers (const std::string& sPattern)                       { return impl_->getPatternMembers(sPattern);}
  const std::string CarpeDM::getPatternEntryNode(const std::string& sPattern)          { return impl_->getPatternEntryNode(sPattern);}
  const std::string CarpeDM::getPatternExitNode(const std::string& sPattern)           { return impl_->getPatternExitNode(sPattern);}
  vStrC CarpeDM::getBeamprocMembers(const std::string& sBeamproc)                      { return impl_->getBeamprocMembers(sBeamproc);}
  const std::string CarpeDM::getBeamprocEntryNode(const std::string& sBeamproc)        { return impl_->getBeamprocEntryNode(sBeamproc);}
  const std::string CarpeDM::getBeamprocExitNode(const std::string& sBeamproc)         { return impl_->getBeamprocExitNode(sBeamproc);}
  uint64_t CarpeDM::getModTime()                                                       { return impl_->getModTime();}

// Text File IO /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void CarpeDM::writeTextFile(const std::string& fn, const std::string& s)             { return impl_->writeTextFile(fn, s);}
  std::string CarpeDM::readTextFile(const std::string& fn)                             { return impl_->readTextFile(fn);}

  void CarpeDM::completeId(vertex_t v, Graph& g)                                       {return impl_->completeId(v, g);}
// Graphs to Dot
  Graph& CarpeDM::parseDot(const std::string& dotString, Graph& g)                     { return impl_->parseDot(dotString, g);}
  void CarpeDM::writeDotFile(const std::string& fn, Graph& g, bool filterMeta)         { return impl_->writeDotFile(fn, g, filterMeta);}
  void CarpeDM::writeDownDotFile(const std::string& fn, bool filterMeta)               { return impl_->writeDownDotFile(fn, filterMeta);}
  void CarpeDM::writeUpDotFile(const std::string& fn, bool filterMeta)                 { return impl_->writeUpDotFile(fn, filterMeta);}

// Schedule Manipulation ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  int CarpeDM::assignNodesToCpus()                                                     { return impl_->assignNodesToCpus();}                            // NOT YET IMPLEMENTED // TODO assign a cpu to each node object. Currently taken from input .dot
  int CarpeDM::download()                                                              { return impl_->download();}                                     // Download binary from LM32 SoC and create Graph
  std::string CarpeDM::downloadDot(bool filterMeta)                                    { return impl_->downloadDot(filterMeta);}
  void CarpeDM::downloadDotFile(const std::string& fn, bool filterMeta)                { return impl_->downloadDotFile(fn, filterMeta);}
  int CarpeDM::addDot(const std::string& s, bool force)                                { return impl_->addDot(s, force);}                   // add all nodes and/or edges in dot file
  int CarpeDM::addDotFile(const std::string& fn, bool force)                           { return impl_->addDotFile(fn, force);}
  int CarpeDM::overwriteDot(const std::string& s, bool force)                          { return impl_->overwriteDot(s, force);} // add all nodes and/or edges in dot file
  int CarpeDM::overwriteDotFile(const std::string& fn, bool force)                     { return impl_->overwriteDotFile(fn, force);}
  int CarpeDM::keepDot(const std::string& s, bool force)                               { return impl_->keepDot(s, force);}      // removes all nodes NOT in input file
  int CarpeDM::keepDotFile(const std::string& fn, bool force)                          { return impl_->keepDotFile(fn, force);}
  int CarpeDM::removeDot(const std::string& s, bool force)                             { return impl_->removeDot(s, force);}    // removes all nodes in input file
  int CarpeDM::removeDotFile(const std::string& fn, bool force)                        { return impl_->removeDotFile(fn, force);}
  int CarpeDM::clear(bool force)                                                       { return impl_->clear(force);}                              // clears all nodes from DM

// Command Generation and Dispatch ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

vStrC CarpeDM::getLockedBlocks(bool checkReadLock, bool checkWriteLock)                                         { return impl_->getLockedBlocks(checkReadLock, checkWriteLock);}
int CarpeDM::sendCommandsDot(const std::string& s)                                                              { return impl_->sendCommandsDot(s);} //Sends a dotfile of commands to the DM
int CarpeDM::sendCommandsDotFile(const std::string& fn)                                                         { return impl_->sendCommandsDotFile(fn);}

void CarpeDM::halt()                                                                                            { return impl_->halt();}
int CarpeDM::staticFlushPattern(const std::string& sPattern, bool prioIl, bool prioHi, bool prioLo, bool force) { return impl_->staticFlushPattern(sPattern, prioIl, prioHi, prioLo, force);}
int CarpeDM::staticFlushBlock(const std::string& sBlock, bool prioIl, bool prioHi, bool prioLo, bool force)     { return impl_->staticFlushBlock(sBlock, prioIl, prioHi, prioLo, force);}


  // Short Live Infos from DM hardware reads /////////////////////////////////////////////////////////////////////////////////////////////////////////////
const std::string CarpeDM::getThrOrigin(uint8_t cpuIdx, uint8_t thrIdx)                                         { return impl_->getThrOrigin(cpuIdx, thrIdx);}      // Returns the Node the Thread will start from
const std::string CarpeDM::getThrCursor(uint8_t cpuIdx, uint8_t thrIdx)                                         { return impl_->getThrCursor(cpuIdx, thrIdx);}      // Returns the Node the Thread is currently processing
uint64_t CarpeDM::getThrMsgCnt(uint8_t cpuIdx, uint8_t thrIdx)                                                  { return impl_->getThrMsgCnt(cpuIdx, thrIdx);}
uint32_t CarpeDM::getThrRun(uint8_t cpuIdx)                                                                     { return impl_->getThrRun(cpuIdx);}                                  // Get bitfield showing running threads
uint32_t CarpeDM::getStatus(uint8_t cpuIdx)                                                                     { return impl_->getStatus(cpuIdx);}
uint64_t CarpeDM::getThrDeadline(uint8_t cpuIdx, uint8_t thrIdx)                                                { return impl_->getThrDeadline(cpuIdx, thrIdx);}
uint64_t CarpeDM::getThrStartTime(uint8_t cpuIdx, uint8_t thrIdx)                                               { return impl_->getThrStartTime(cpuIdx, thrIdx);}
uint32_t CarpeDM::getThrStart(uint8_t cpuIdx)                                                                   { return impl_->getThrStart(cpuIdx);}
uint64_t CarpeDM::getThrPrepTime(uint8_t cpuIdx, uint8_t thrIdx)                                                { return impl_->getThrPrepTime(cpuIdx, thrIdx);}
bool CarpeDM::isThrRunning(uint8_t cpuIdx, uint8_t thrIdx)                                                      { return impl_->isThrRunning(cpuIdx, thrIdx);}                   // true if thread <thrIdx> is running
//bool CarpeDM::isSafeToRemove(const std::string& pattern, std::string& report, std::vector<QueueReport>& vQr)    { return impl_->isSafeToRemove(pattern, report, vQr);}
bool CarpeDM::isSafeToRemove(const std::string& pattern, std::string& report)                                   { return impl_->isSafeToRemove(pattern, report);}
std::pair<int, int> CarpeDM::findRunningPattern(const std::string& sPattern)                                    { return impl_->findRunningPattern(sPattern);} // get cpu and thread assignment of running pattern
bool CarpeDM::isPatternRunning(const std::string& sPattern)                                                     { return impl_->isPatternRunning(sPattern);}                  // true if Pattern <x> is running
void CarpeDM::updateModTime()                                                                                   { return impl_->updateModTime();}


void CarpeDM::forceThrCursor(uint8_t cpuIdx, uint8_t thrIdx)                                                    { return impl_->forceThrCursor(cpuIdx, thrIdx);} //DEBUG ONLY !!!


vEbwrs& CarpeDM::startThr(vEbwrs& ew, uint8_t cpuIdx, uint8_t thrIdx)                                           { return impl_->startThr(ew, cpuIdx, thrIdx);} //Requests Thread to start
vEbwrs& CarpeDM::startPattern(vEbwrs& ew, const std::string& sPattern, uint8_t thrIdx)                          { return impl_->startPattern(ew, sPattern, thrIdx);} //Requests Pattern to start
//vEbwrs& CarpeDM::startPattern(vEbwrs& ew, const std::string& sPattern)                                          { return impl_->startPattern(ew, sPattern);} //Requests Pattern to start on first free thread
vEbwrs& CarpeDM::startNodeOrigin(vEbwrs& ew, const std::string& sNode, uint8_t thrIdx)                          { return impl_->startNodeOrigin(ew, sNode, thrIdx);} //Requests thread <thrIdx> to start at node <sNode>
//vEbwrs& CarpeDM::startNodeOrigin(vEbwrs& ew, const std::string& sNode)                                          { return impl_->startNodeOrigin(ew, sNode);} //Requests a start at node <sNode>
vEbwrs& CarpeDM::stopPattern(vEbwrs& ew, const std::string& sPattern)                                           { return impl_->stopPattern(ew, sPattern);} //Requests Pattern to stop
vEbwrs& CarpeDM::stopNodeOrigin(vEbwrs& ew, const std::string& sNode)                                           { return impl_->stopNodeOrigin(ew, sNode);} //Requests stop at node <sNode> (vEbwrs& ew, flow to idle)
vEbwrs& CarpeDM::abortPattern(vEbwrs& ew, const std::string& sPattern)                                          { return impl_->abortPattern(ew, sPattern);} //Immediately aborts a Pattern
vEbwrs& CarpeDM::abortNodeOrigin(vEbwrs& ew, const std::string& sNode)                                          { return impl_->abortNodeOrigin(ew, sNode);} //Immediately aborts the thread whose pattern <sNode> belongs to
vEbwrs& CarpeDM::abortThr(vEbwrs& ew, uint8_t cpuIdx, uint8_t thrIdx)                                           { return impl_->abortThr(ew, cpuIdx, thrIdx);} //Immediately aborts a Thread
vEbwrs& CarpeDM::setThrStart(vEbwrs& ew, uint8_t cpuIdx, uint32_t bits)                                         { return impl_->setThrStart(ew, cpuIdx, bits);} //Requests Threads to start
vEbwrs& CarpeDM::setThrAbort(vEbwrs& ew, uint8_t cpuIdx, uint32_t bits)                                         { return impl_->setThrAbort(ew, cpuIdx, bits);} //Immediately aborts Threads
vEbwrs& CarpeDM::setThrOrigin(vEbwrs& ew, uint8_t cpuIdx, uint8_t thrIdx, const std::string& name)              { return impl_->setThrOrigin(ew, cpuIdx, thrIdx,  name);} //Sets the Node the Thread will start from
vEbwrs& CarpeDM::setThrStartTime(vEbwrs& ew, uint8_t cpuIdx, uint8_t thrIdx, uint64_t t)                        { return impl_->setThrStartTime(ew, cpuIdx, thrIdx, t);}
vEbwrs& CarpeDM::setThrPrepTime(vEbwrs& ew, uint8_t cpuIdx, uint8_t thrIdx, uint64_t t)                         { return impl_->setThrPrepTime(ew, cpuIdx, thrIdx, t);}
vEbwrs& CarpeDM::deactivateOrphanedCommands(vEbwrs& ew, std::vector<QueueReport>& vQr)                          { return impl_->deactivateOrphanedCommands(ew, vQr);}
vEbwrs& CarpeDM::clearHealth(vEbwrs& ew, uint8_t cpuIdx)                                                        { return impl_->clearHealth(ew, cpuIdx);}
vEbwrs& CarpeDM::clearHealth(vEbwrs& ew)                                                                        { return impl_->clearHealth(ew);}
vEbwrs& CarpeDM::resetThrMsgCnt(vEbwrs& ew, uint8_t cpuIdx, uint8_t thrIdx)                                     { return impl_->resetThrMsgCnt(ew, cpuIdx, thrIdx);}
vEbwrs& CarpeDM::blockAsyncClearQueues(vEbwrs& ew, const std::string& sTarget)                                  { return impl_->blockAsyncClearQueues(ew, sTarget);}
vEbwrs& CarpeDM::switching(vEbwrs& ew, const std::string& sTarget, const std::string& sDst)                     { return impl_->switching(ew, sTarget, sDst);}
vEbwrs& CarpeDM::createNonQCommand(vEbwrs& ew, const std::string& type, const std::string& target, uint8_t cmdThr)              { return impl_->createNonQCommand(ew, type,  target, cmdThr);}
            


vEbwrs& CarpeDM::createLockCtrlCommand(vEbwrs& ew, const std::string& type, const std::string& target, bool lockRd, bool lockWr )
{ return impl_->createLockCtrlCommand(ew, type, target, lockRd, lockWr );}

vEbwrs& CarpeDM::createQCommand(vEbwrs& ew, const std::string& type, const std::string& target, uint8_t cmdPrio, uint8_t cmdQty, bool vabs, uint64_t cmdTvalid, uint8_t cmdThr)
{ return impl_->createQCommand(ew, type, target, cmdPrio, cmdQty, vabs, cmdTvalid, cmdThr);}

vEbwrs& CarpeDM::createWaitCommand(vEbwrs& ew, const std::string& type, const std::string& target, uint8_t cmdPrio, uint8_t cmdQty, bool vabs, uint64_t cmdTvalid, uint64_t cmdTwait, bool abswait )
{ return impl_->createWaitCommand(ew, type, target, cmdPrio, cmdQty, vabs, cmdTvalid, cmdTwait, abswait );}

vEbwrs& CarpeDM::createFlowCommand(vEbwrs& ew, const std::string& type, const std::string& target, const std::string& destination, uint8_t  cmdPrio, uint8_t cmdQty, bool vabs, uint64_t cmdTvalid, bool perma)
{ return impl_->createFlowCommand(ew, type, target, destination, cmdPrio, cmdQty, vabs, cmdTvalid, perma);}

vEbwrs& CarpeDM::createFlushCommand(vEbwrs& ew, const std::string& type, const std::string& target, const std::string& destination, uint8_t  cmdPrio, uint8_t cmdQty, bool vabs, uint64_t cmdTvalid, bool qIl, bool qHi, bool qLo)
{ return impl_->createFlushCommand(ew, type, target, destination, cmdPrio, cmdQty, vabs, cmdTvalid, qIl, qHi, qLo);}

vEbwrs& CarpeDM::createFullCommand(vEbwrs& ew, const std::string& type, const std::string& target, const std::string& destination, uint8_t  cmdPrio, uint8_t cmdQty, bool vabs, uint64_t cmdTvalid, bool perma, bool qIl, bool qHi, bool qLo, uint64_t cmdTwait, bool abswait, bool lockRd, bool lockWr, uint8_t cmdThr)
{ return impl_->createFullCommand(ew, type, target, destination, cmdPrio, cmdQty, vabs, cmdTvalid, perma, qIl, qHi, qLo, cmdTwait, abswait, lockRd, lockWr, cmdThr);}

vEbwrs& CarpeDM::createCommand(vEbwrs& ew, const std::string& type, const std::string& target, const std::string& destination, uint8_t  cmdPrio, uint8_t cmdQty, bool vabs, uint64_t cmdTvalid, bool perma, bool qIl, bool qHi, bool qLo,  uint64_t cmdTwait, bool abswait, bool lockRd, bool lockWr, uint8_t cmdThr)
{ return impl_->createCommand(ew, type, target, destination, cmdPrio, cmdQty, vabs, cmdTvalid, perma,qIl, qHi, qLo, cmdTwait, abswait, lockRd, lockWr, cmdThr );}
                

                int CarpeDM::send(vEbwrs& ew) {return impl_->send(ew);}
            //FIXME workaround for flawed template approach (disambiguation of member functin pointers failing). no time to figure it out right  now, get the job done first
            //convenience wrappers without eb cycle control, send immediately
            int CarpeDM::startThr(uint8_t cpuIdx, uint8_t thrIdx)                               {return impl_->startThr(cpuIdx, thrIdx);}
            int CarpeDM::startPattern(const std::string& sPattern, uint8_t thrIdx)              {return impl_->startPattern(sPattern, thrIdx);}
            int CarpeDM::startNodeOrigin(const std::string& sNode, uint8_t thrIdx)              {return impl_->startNodeOrigin(sNode, thrIdx);}
            //int CarpeDM::startNodeOrigin(const std::string& sNode)                              {return impl_->startNodeOrigin(sNode);}
            int CarpeDM::stopPattern(const std::string& sPattern)                               {return impl_->stopPattern(sPattern);}
            int CarpeDM::stopNodeOrigin(const std::string& sNode)                               {return impl_->stopNodeOrigin(sNode);}
            int CarpeDM::abortPattern(const std::string& sPattern)                              {return impl_->abortPattern(sPattern);}
            int CarpeDM::abortNodeOrigin(const std::string& sNode)                              {return impl_->abortNodeOrigin(sNode);}
            int CarpeDM::abortThr(uint8_t cpuIdx, uint8_t thrIdx)                               {return impl_->abortThr(cpuIdx, thrIdx);}
            int CarpeDM::setThrStart(uint8_t cpuIdx, uint32_t bits)                             {return impl_->setThrStart(cpuIdx, bits);}
            int CarpeDM::setThrAbort(uint8_t cpuIdx, uint32_t bits)                             {return impl_->setThrAbort(cpuIdx, bits);}
            int CarpeDM::setThrOrigin(uint8_t cpuIdx, uint8_t thrIdx, const std::string& name)  {return impl_->setThrOrigin(cpuIdx, thrIdx, name);}
            int CarpeDM::setThrStartTime(uint8_t cpuIdx, uint8_t thrIdx, uint64_t t)            {return impl_->setThrStartTime(cpuIdx, thrIdx, t);}
            int CarpeDM::setThrPrepTime(uint8_t cpuIdx, uint8_t thrIdx, uint64_t t)             {return impl_->setThrPrepTime(cpuIdx, thrIdx, t);}
            int CarpeDM::deactivateOrphanedCommands(std::vector<QueueReport> & vQr)             {return impl_->deactivateOrphanedCommands(vQr);}
            int CarpeDM::clearHealth()                                                          {return impl_->clearHealth();}
            int CarpeDM::clearHealth(uint8_t cpuIdx)                                            {return impl_->clearHealth(cpuIdx);}
            int CarpeDM::resetThrMsgCnt(uint8_t cpuIdx, uint8_t thrIdx)                         {return impl_->resetThrMsgCnt(cpuIdx, thrIdx);}
            int CarpeDM::blockAsyncClearQueues(const std::string& sBlock)                       {return impl_->blockAsyncClearQueues(sBlock);}


     void CarpeDM::verboseOn()                                                              { return impl_->verboseOn();}                             // Turn on Verbose Output
     void CarpeDM::verboseOff()                                                             { return impl_->verboseOff();}                            // Turn off Verbose Output
     bool CarpeDM::isVerbose()  const                                                       { return impl_->isVerbose();}                       // Tell if Output is set to Verbose
     void CarpeDM::debugOn()                                                                { return impl_->debugOn();}                                 // Turn on Verbose Output
     void CarpeDM::debugOff()                                                               { return impl_->debugOff();}                                // Turn off Verbose Output
     bool CarpeDM::isDebug()  const                                                         { return impl_->isDebug();}                            // Tell if Output is set to Verbose
     bool CarpeDM::isSim()  const                                                           { return impl_->isSim();}                               // Tell if this is a simulation. Cannot change while connected !!!
     void CarpeDM::testOn()                                                                 { return impl_->testOn();}                               // Turn on Testmode
     void CarpeDM::testOff()                                                                { return impl_->testOff();}                              // Turn off Testmode
     bool CarpeDM::isTest() const                                                           { return impl_->isTest();}                          // Tell if Testmode is on
     void CarpeDM::optimisedS2ROn()                                                         { return impl_->optimisedS2ROn();}                    // Optimised Safe2remove on
     void CarpeDM::optimisedS2ROff()                                                        { return impl_->optimisedS2ROff();}                   // Optimised Safe2remove off
     bool CarpeDM::isOptimisedS2R() const                                                   { return impl_->isOptimisedS2R();}                                     // tell if Safe2remove optimisation is on or off
     bool CarpeDM::isValidDMCpu(uint8_t cpuIdx)                                             { return impl_->isValidDMCpu(cpuIdx);}                               // Check if CPU is registered as running a valid firmware
     HealthReport& CarpeDM::getHealth(uint8_t cpuIdx, HealthReport &hr)                     { return impl_->getHealth(cpuIdx, hr);}                // FIXME why reference in, reference out ? its not like you can add to this report ...
     QueueReport& CarpeDM::getQReport(const std::string& blockName, QueueReport& qr)        { return impl_->getQReport(blockName, qr);}  // FIXME why reference in, reference out ? its not like you can add to this report ...
     std::string& CarpeDM::getRawQReport(const std::string& blockName, std::string& report) { return impl_->getRawQReport(blockName, report);}
     uint64_t CarpeDM::getDmWrTime()                                                        { return impl_->getDmWrTime();}
     HwDelayReport& CarpeDM::getHwDelayReport(HwDelayReport& hdr)                           { return impl_->getHwDelayReport(hdr);}
     void CarpeDM::clearHwDiagnostics()                                                     { return impl_->clearHwDiagnostics();}
     void CarpeDM::startStopHwDiagnostics(bool enable)                                      { return impl_->startStopHwDiagnostics(enable);}
     void CarpeDM::configHwDiagnostics(uint64_t timeIntvl, uint32_t stallIntvl)             { return impl_->configHwDiagnostics(timeIntvl, stallIntvl);}
     void CarpeDM::configFwDiagnostics(uint64_t warnThrshld)                                { return impl_->configFwDiagnostics(warnThrshld);}

       std::string& CarpeDM::inspectQueues(const std::string& blockName, std::string& report) {return impl_->inspectQueues(blockName, report);}      // Show all command fields in Block Queue
               void CarpeDM::show(const std::string& title, const std::string& logDictFile, TransferDir dir, bool filterMeta ) {return impl_->show(title, logDictFile, dir, filterMeta );}
            void CarpeDM::showUp(bool filterMeta)            { return impl_->showUp(filterMeta);}                                               // show a CPU's Upload address table
            void CarpeDM::showDown(bool filterMeta)          { return impl_->showDown(filterMeta);}
            void CarpeDM::dumpNode(const std::string& name)  { return impl_->dumpNode(name);}                     // hex dump a node
            void CarpeDM::showPaint()                        { return impl_->showPaint();}
            bool CarpeDM::isPainted(const std::string& name) { return impl_->isPainted(name);}
            void CarpeDM::inspectHeap(uint8_t cpuIdx)        { return impl_->inspectHeap(cpuIdx);}
            void CarpeDM::showHashDict()                     { return impl_->showHashDict();}
            void CarpeDM::showGroupsDict()                   { return impl_->showGroupsDict();}
            bool CarpeDM::tableCheck(std::string& report)    { return impl_->tableCheck(report);}
            void CarpeDM::coverage3Upload(uint64_t seed )    { return impl_->coverage3Upload(seed );}
            void CarpeDM::dirtyCtShow()                      { return impl_->dirtyCtShow();}
            void CarpeDM::showCpuList()                      { return impl_->showCpuList();}
         uint8_t CarpeDM::getCpuQty()                        { return impl_->getCpuQty();}
         uint8_t CarpeDM::getThrQty()                        { return impl_->getThrQty();}
         uint32_t CarpeDM::getCtlAdr(const uint8_t& idx)     { return impl_->getCtlAdr(idx);}
            bool CarpeDM::isCpuIdxValid(uint8_t cpuIdx)      { return impl_->isCpuIdxValid(cpuIdx);}
            void CarpeDM::showMemSpace()                     { return impl_->showMemSpace();}
            void CarpeDM::lockManagerClear()                 { return impl_->lockManagerClear();}
            bool CarpeDM::lockManagerHasEntries()            { return impl_->lockManagerHasEntries();}
            void CarpeDM::softwareReset(bool clearStatistic) { return impl_->softwareReset(clearStatistic);}
            std::vector<std::vector<uint64_t>> CarpeDM::coverage3TestData(uint64_t seedStart, uint64_t cases, uint8_t parts, uint8_t percentage ) {return impl_->coverage3TestData(seedStart, cases, parts, percentage );}
