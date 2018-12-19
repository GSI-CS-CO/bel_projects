#include <boost/shared_ptr.hpp>
#include <algorithm>
#include <stdio.h>
#include <iostream>
#include <string>
#include <inttypes.h>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/copy.hpp>
#include <boost/algorithm/string.hpp>
#include <thread>
#include <chrono>

#include "common.h"

#include "carpeDM.h"
#include "minicommand.h"
#include "propwrite.h"

#include "node.h"
#include "block.h"
#include "meta.h"
#include "event.h"
#include "dotstr.h"


namespace dnt = DotStr::Node::TypeVal;
namespace det = DotStr::Edge::TypeVal;

namespace carpeDMcommand {
  const std::string exIntro = "carpeDMcommand: ";
}

vEbwrs& CarpeDM::blockAsyncClearQueues(const std::string& sBlock, vEbwrs& ew) {
  uint32_t hash;
  hash      = hm.lookup(sBlock, "block: unknown target ");
  auto it   = atDown.lookupHash(hash, carpeDMcommand::exIntro);
  auto* x   = (AllocMeta*)&(*it);
  uint32_t adrBase    = atDown.adrConv(AdrType::MGMT, AdrType::EXT, x->cpu, x->adr);
  //reset read and write indices
  vAdr tmp = {(adrBase + BLOCK_CMDQ_RD_IDXS), (adrBase + BLOCK_CMDQ_WR_IDXS)};
  ew.va += tmp;
  ew.vb += {0, 0};
  ew.vcs += leadingOne(tmp.size());
  return ew;
}

vStrC CarpeDM::getLockedBlocks(bool checkReadLock, bool checkWriteLock) {
  vStrC ret;
  if (!(checkReadLock & checkWriteLock)) throw std::runtime_error("Get locked Blocks: valid inputs are read, write, or both. None is not permitted.");  


  lm.clear(); //clear lock manager for inspection

  //get a list of all blocks
  BOOST_FOREACH( vertex_t vChkBlock, vertices(gDown) ) {
    //setup check if this block has a lock we're interested in
    if(gDown[vChkBlock].np->isBlock()) { 
      BlockLock& lock = lm.add(gDown[vChkBlock].name);
      lock.rd.set = checkReadLock;
      lock.wr.set = checkWriteLock;
    }
  }
  lm.readInStat(); // read in the lock status flags from DM
  for (const auto& l : lm.getLockVec()) { if (l.isAnySet()) ret.push_back(l.name); }; //
  
  return ret;
}



boost::optional<std::pair<int, int>> CarpeDM::parseCpuAndThr(vertex_t v, Graph& g) {

  uint8_t  cpu, thr;
  std::pair<int, int> res;

  //sLog << "Try parsing CPU " << g[v].cpu << " and Thr " << g[v].thread << std::endl;


  try { cpu = s2u<uint8_t>(g[v].cpu);
        thr = s2u<uint8_t>(g[v].thread);
        res = {cpu, thr};
      } catch (...) {
      //  sLog << "Caused exception, returning none" << std::endl;
        return boost::optional<std::pair<int, int>>();
      }


  if ( (cpu < 0) || (cpu >= ebd.getCpuQty()) || (thr < 0) || (thr >= _THR_QTY_  ) ) {
    //sLog << "OOR, returning none" << std::endl;
    return boost::optional<std::pair<int, int>>();
  }


  //sLog << "Valid, returning " << res.first << " " << res.second << std::endl;
  return res;
}


void CarpeDM::adjustValidTime(uint64_t& tValid, bool abs) {
  // All tValids must be in the future when written so host speed does not influency command availability to Firmware
  // Find a point in time which will safely be in the near future when we write this command
  uint64_t tFuture    = modTime + (testmode ? 0ULL : processingTimeMargin);  // no margin for sim, otherwise coverage testing is too slow.
  // make copy, choose and update original.
  uint64_t tOriginal  = tValid;
  tValid = abs ? std::max(tOriginal, tFuture) : tFuture + tOriginal; // if absolute -> max of original and near future, if relative -> sum of original and future
}


vEbwrs& CarpeDM::createCommand(vEbwrs& ew, const std::string& type, const std::string& target, const std::string& destination, 
  uint8_t  cmdPrio, uint8_t cmdQty, bool vabs, uint64_t cmdTvalid, bool perma, bool qIl, bool qHi, bool qLo,  uint64_t cmdTwait, bool abswait, bool lockRd, bool lockWr )
{
    mc_ptr mc;
    sLog << "Command  type <" << type << ">" << std::endl;
            sLog << "DEBUG: VABS: " << std::boolalpha << " b " << vabs << std::endl;
    adjustValidTime(cmdTvalid, vabs);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Global Commands (not targeted at cmd queues of individual blocks)
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    if (type == dnt::sCmdStart)   {
      if (hm.lookup(target)) {sLog << " Starting at <" << target << ">" << std::endl; startNodeOrigin(target, ew);  }
      else {throw std::runtime_error("Cannot execute command '" + type + "' No valid cpu/thr provided and '" + target + "' is not a valid node name\n");}
      return ew;
    }
    if (type == dnt::sCmdStop)    {
      if (hm.lookup(target)) { sLog << " Stopping at <" << target << ">" << std::endl; stopNodeOrigin(target, ew); }
      else {throw std::runtime_error("Cannot execute command '" + type + "' No valid cpu/thr provided and '" + target + "' is not a valid node name\n");}
      return ew;
    }
    else if (type == dnt::sCmdAbort)   {
      if (hm.lookup(target)) {sLog << " Aborting (trying) at <" << target << ">" << std::endl; abortNodeOrigin(target, ew); }
      else {throw std::runtime_error("Cannot execute command '" + type + "'. No valid cpu/thr provided and '" + target + "' is not a valid node name\n");}
      return ew;
    }

    //FIMXE hack to test compile
    uint8_t thr = 0;

    if (type == dnt::sCmdOrigin)   {
      //Leave out for now and autocorrect cpu
      try { setThrOrigin(getNodeCpu(target, TransferDir::DOWNLOAD), thr, target, ew); } catch (std::runtime_error const& err) {
        throw std::runtime_error("Cannot execute command '" + type + "', " + std::string(err.what()));
      }
      return ew;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Local Commands (targeted at cmd queue of individual blocks)
    // These operations need at least a write lock. Add it to the lock manager
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    BlockLock& lock = lm.add(target);

    // lock all queues of a block
    if (type == dnt::sCmdLock) {
      lock.wr.set = lockWr;
      lock.rd.set = lockRd;  
      return ew;
    // async clear all queues
    } else if (type == dnt::sCmdAsyncClear) {
      blockAsyncClearQueues(target, ew);
      lock.wr.set = true;
      lock.wr.clr = true;
      lock.rd.set = true;
      lock.rd.clr = true;    
      return ew;
    // unlock all queues
    } else if (type == dnt::sCmdUnlock) {
      lock.wr.clr = lockWr;
      lock.rd.clr = lockRd; 
      return ew;
    // Commands targeted at cmd queue of individual blocks
    } else if (type == dnt::sCmdNoop) {
      mc = (mc_ptr) new MiniNoop(cmdTvalid, cmdPrio, cmdQty );
      lock.wr.set = true;
      lock.wr.clr = true;

    } else if (type == dnt::sCmdFlow) {
      
      //sLog << " Flowing from <" << target << "> to <" << destination << ">, permanent defDest change='" << s2u<bool>(g[v].perma) << "'" << std::endl;

      uint32_t adr = LM32_NULL_PTR;
      try { adr = getNodeAdr(destination, TransferDir::DOWNLOAD, AdrType::INT); } catch (std::runtime_error const& err) {
        throw std::runtime_error("Destination '" + destination + "'' invalid: " + std::string(err.what()));
      }
  
      mc = (mc_ptr) new MiniFlow(cmdTvalid, cmdPrio, cmdQty, adr, perma );
      lock.wr.set = true;
      lock.wr.clr = true;

    } else if (type == dnt::sCmdFlush) { // << " Flushing <" << target << "> Queues IL " << s2u<int>(g[v].qIl) << " HI " << s2u<int>(g[v].qHi) << " LO " << s2u<int>(g[v].qLo) <<  std::endl;
      uint32_t adr = LM32_NULL_PTR;
      try { adr = getNodeAdr(destination, TransferDir::DOWNLOAD, AdrType::INT); } catch (std::runtime_error const& err) {
        // empty tag or invalid destination. We'll interpret this as no override desired.
        adr = LM32_NULL_PTR;
      }
      mc = (mc_ptr) new MiniFlush(cmdTvalid, cmdPrio, qIl, qHi, qLo, adr, perma);
      lock.wr.set = true;
      lock.wr.clr = true; 

    } else if (type == dnt::sCmdWait) {
      mc = (mc_ptr) new MiniWait(cmdTvalid, cmdPrio, cmdTwait, false, false );
      lock.wr.set = true;
      lock.wr.clr = true;                                            
    }
    else { throw std::runtime_error("Command type <" + type + "> is not supported!\n");}

    //sLog << std::endl;
    createMiniCommand(target, cmdPrio, mc, ew);

  return ew;
  
}    

//convenience wrappers
//commands with no extras
vEbwrs& CarpeDM::createNonQCommand(vEbwrs& ew, const std::string& type, const std::string& target) {
  return createCommand(ew, type, target, "", 0, 1, true, 0, false, false, false, false, 0, false, false, false);
}

vEbwrs& CarpeDM::createLockCtrlCommand(vEbwrs& ew, const std::string& type, const std::string& target, bool lockRd, bool lockWr ) {
  return createCommand(ew, type, target, "", 0, 1, true, 0, false, false, false, false, 0, false, lockRd, lockWr );
}

//commands with time
vEbwrs& CarpeDM::createQCommand(vEbwrs& ew, const std::string& type, const std::string& target, uint8_t cmdPrio, uint8_t cmdQty, bool vabs, uint64_t cmdTvalid) {
  return createCommand(ew, type, target, "", cmdPrio, cmdQty, vabs, cmdTvalid, false, false, false, false, 0, false, false, false);
}
  
//flows
vEbwrs& CarpeDM::createFlowCommand(vEbwrs& ew, const std::string& type, const std::string& target, const std::string& destination, 
  uint8_t  cmdPrio, uint8_t cmdQty, bool vabs, uint64_t cmdTvalid, bool perma) {
  return createCommand(ew, type, target, destination, cmdPrio, cmdQty, vabs, cmdTvalid, perma, false, false, false, 0, false, false, false);
} 

//flush or flush override
vEbwrs& CarpeDM::createFlushCommand(vEbwrs& ew, const std::string& type, const std::string& target, const std::string& destination, 
  uint8_t  cmdPrio, uint8_t cmdQty, bool vabs, uint64_t cmdTvalid, bool qIl, bool qHi, bool qLo) {
  return createCommand(ew, type, target, destination, cmdPrio, cmdQty, vabs, cmdTvalid, false, qIl, qHi, qLo, 0, false, false, false);
} 

//wait
vEbwrs& CarpeDM::createWaitCommand(vEbwrs& ew, const std::string& type, const std::string& target,  
  uint8_t  cmdPrio, uint8_t cmdQty, bool vabs, uint64_t cmdTvalid, uint64_t cmdTwait, bool abswait ) {
  return createCommand(ew, type, target, "", cmdPrio, cmdQty, vabs, cmdTvalid, false, false, false, false, cmdTwait, abswait, false, false);
}

vEbwrs& CarpeDM::createFullCommand(vEbwrs& ew, const std::string& type, const std::string& target, const std::string& destination, 
  uint8_t  cmdPrio, uint8_t cmdQty, bool vabs, uint64_t cmdTvalid, bool perma, bool qIl, bool qHi, bool qLo, uint64_t cmdTwait, bool abswait,bool lockRd, bool lockWr )
{
  updateModTime();
  return createCommand(ew, type, target, destination, cmdPrio, cmdQty, vabs, perma, qIl, qHi, qLo, cmdTvalid, cmdTwait, abswait, lockRd, lockWr);
}






vEbwrs& CarpeDM::createCommandBurst(vEbwrs& ew, Graph& g) {

  lm.clear();
 

  if ((boost::get_property(g, boost::graph_name)).find(DotStr::Graph::Special::sCmd) == std::string::npos) {throw std::runtime_error("Expected a series of commands, but this appears to be a schedule (Tag '" + DotStr::Graph::Special::sCmd + "' not found in graphname)");}

  //it is vital that the modTime TS is current to handle relative command-valid-times.
  //this might redundantly be done by the implementing FESA class, but better safe than sorry.
  updateModTime();

  BOOST_FOREACH( vertex_t v, vertices(g) ) {

    std::string target, destination, type;
    bool qIl, qHi, qLo, perma, vabs, abswait, lockRd, lockWr;
    uint64_t cmdTvalid, cmdTwait;
    uint32_t cmdQty;
    uint8_t cmdPrio;



    //use the pattern and beamprocess tags to determine the target. Pattern overrides Beamprocess overrides cpu/thread
          if (g[v].patName  != DotStr::Misc::sUndefined)    { target = getPatternExitNode(g[v].patName); }
    else  if (g[v].bpName != DotStr::Misc::sUndefined)      { target = getBeamprocExitNode(g[v].bpName); }
    else  {target = g[v].cmdTarget;}



    //use the destPattern and destBeamprocess tags to determine the destination
    if (g[v].cmdDestPat  != DotStr::Misc::sUndefined)      { destination = getPatternEntryNode(g[v].cmdDestPat); }
    else  if (g[v].cmdDestBp != DotStr::Misc::sUndefined)  { destination = getBeamprocEntryNode(g[v].cmdDestBp); }
    else                                                   { destination = g[v].cmdDest;}

    type      = g[v].type;
    cmdPrio   = s2u<uint8_t>(g[v].prio);
    vabs      = s2u<bool>(g[v].vabs);
    cmdQty    = s2u<uint32_t>(g[v].qty);
    cmdTvalid = s2u<uint64_t>(g[v].tValid);
    cmdTwait  = s2u<uint64_t>(g[v].tWait);
    qIl       = s2u<bool>(g[v].qIl);
    qHi       = s2u<bool>(g[v].qHi);
    qLo       = s2u<bool>(g[v].qLo);
    perma     = s2u<bool>(g[v].perma);
    sLog << "DEBUG: VABS: " << g[v].vabs << std::boolalpha << " recon " << vabs << std::endl;
    lockRd    = false;
    lockWr    = false;
    //fixme hack to test compile
    abswait   = false;
  
    if(verbose) sLog << "Command <" << g[v].name << ">, type <" << g[v].type << ">" << std::endl;
    createCommand(ew, type, target, destination, cmdPrio, cmdQty, vabs, cmdTvalid, perma, qIl, qHi, qLo, cmdTwait, abswait, lockRd, lockWr);
  }  

  return ew;

}

  int CarpeDM::send(vEbwrs& ew) {

    bool locksRdy;
    sLog << "reading lockstates...";
    lm.readInStat(); //read current lock states
    sLog << "done" << std::endl << " Setting locks...";
    lm.processLockRequests(); //set requested locks (or'ed with already set locks)
    sLog << "done" << std::endl; 
    //3 retries for lock readiness
    for(unsigned i = 0; i < 3; i++) {
      sLog << "checking locks";
      locksRdy = lm.isReady();
      sLog << "check performed" << std::endl;
      if (locksRdy) {
        sLog << "locks ready. writing commands";
        ebd.writeCycle(ew.va, ew.vb, ew.vcs); //if ready, write commands. if not, restore locks and throw exception
        //sLog << "writing commands" << std::endl;
        break;
      }
      std::chrono::milliseconds timespan(1);
      std::this_thread::sleep_for(timespan);  
    }
    sLog << "done." << std::endl << "Unlocking...";
    lm.processUnlockRequests(); //restore lock bits
    if (!locksRdy) throw std::runtime_error("Could not write commands, locking failed");
    sLog << "done." << std::endl;
    return ew.vb.size();
  }


  vEbwrs& CarpeDM::createMiniCommand(const std::string& targetName, uint8_t cmdPrio, mc_ptr mc, vEbwrs& ew) {

    uint32_t cmdWrInc, hash;
    uint8_t b[_T_CMD_SIZE_ + _32b_SIZE_];

    //check for covenants
    if(optimisedS2R) {
      cmI x = ct.lookup(targetName);
      if (ct.isOk(x) && isCovenantPending(x)) {
        //there is covenant. lets see if we are a danger to it
        if(x->prio < cmdPrio) throw std::runtime_error("Command preemption (prio " + std::to_string((int)cmdPrio) + ") at block <" + targetName + "> would violate a safe2remove-covenant!");
      }
    }

    hash     = hm.lookup(targetName, "createCommand: unknown target ");
    vAdr tmp = getCmdWrAdrs(hash, cmdPrio);
    ew.va   += tmp;
    ew.vcs  += leadingOne(tmp.size());

    cmdWrInc = getCmdInc(hash, cmdPrio);
    mc->serialise(b);
    writeLeNumberToBeBytes(b + (ptrdiff_t)_T_CMD_SIZE_, cmdWrInc);
    ew.vb.insert( ew.vb.end(), b, b + _T_CMD_SIZE_ + _32b_SIZE_);
    //Save mod information for minicommands

    //special treatment for stop (flow to idle == type flow && dst LM32_NULL_PTR)
    uint8_t opType = OP_TYPE_CMD_BASE + ((mc->getAct() >> ACT_TYPE_POS) & ACT_TYPE_MSK);
    if ((((mc->getAct() >> ACT_TYPE_POS) & ACT_TYPE_MSK) == ACT_TYPE_FLOW)
     && (boost::dynamic_pointer_cast<MiniFlow>(mc)->getDst() == LM32_NULL_PTR)) { opType = OP_TYPE_CMD_STOP; }
    //FIMXE this should be done for all commands, not just the minis  
    createCmdModInfo(getNodeCpu(targetName, TransferDir::DOWNLOAD), 0, opType, ew);

    return ew;
  }


  //Returns the external address of a thread's command register area
  uint32_t CarpeDM::getThrCmdAdr(uint8_t cpuIdx) {
    return atDown.getMemories()[cpuIdx].extBaseAdr + atDown.getMemories()[cpuIdx].sharedOffs + SHCTL_THR_CTL;
  }

  //Returns the external address of a thread's initial node register
  uint32_t CarpeDM::getThrInitialNodeAdr(uint8_t cpuIdx, uint8_t thrIdx) {
    return atDown.getMemories()[cpuIdx].extBaseAdr + atDown.getMemories()[cpuIdx].sharedOffs + SHCTL_THR_STA + thrIdx * _T_TS_SIZE_ + T_TS_NODE_PTR;
  }

  //Returns the external address of a thread's cursor pointer
  uint32_t CarpeDM::getThrCurrentNodeAdr(uint8_t cpuIdx, uint8_t thrIdx) {
    return atDown.getMemories()[cpuIdx].extBaseAdr + atDown.getMemories()[cpuIdx].sharedOffs + SHCTL_THR_DAT + thrIdx * _T_TD_SIZE_ + T_TD_NODE_PTR;
  }


  //Sets the Node the Thread will start from
  vEbwrs& CarpeDM::setThrOrigin(uint8_t cpuIdx, uint8_t thrIdx, const std::string& name, vEbwrs& ew) {
    uint8_t b[4];

    ew.va.push_back(getThrInitialNodeAdr(cpuIdx, thrIdx));
    writeLeNumberToBeBytes<uint32_t>(b, getNodeAdr(name, TransferDir::DOWNLOAD, AdrType::INT));
    ew.vb.insert( ew.vb.end(), b, b + sizeof(b));
    ew.vcs += leadingOne(1);
    return ew;
  }

  //Returns the Node the Thread will start from
  const std::string CarpeDM::getThrOrigin(uint8_t cpuIdx, uint8_t thrIdx) {
    uint32_t adr;

    adr = ebd.read32b( getThrInitialNodeAdr(cpuIdx, thrIdx));

    if (adr == LM32_NULL_PTR) return DotStr::Node::Special::sIdle;
    try {
      auto x = atDown.lookupAdr(cpuIdx, atDown.adrConv(AdrType::INT, AdrType::MGMT,cpuIdx, adr));
      return gDown[x->v].name;
    } catch (...) {
      return DotStr::Misc::sUndefined;
    }
  }



  const std::string CarpeDM::getThrCursor(uint8_t cpuIdx, uint8_t thrIdx) {
    uint32_t adr;

    adr = ebd.read32b( getThrCurrentNodeAdr(cpuIdx, thrIdx));

    //std::cout << "#" << (int) cpuIdx << ", " << (int)thrIdx << std::hex << " 0x" << adr << std::endl;

    if (adr == LM32_NULL_PTR) return DotStr::Node::Special::sIdle;
    try {
      auto x = atDown.lookupAdr(cpuIdx, atDown.adrConv(AdrType::INT, AdrType::MGMT,cpuIdx, adr));
      return gDown[x->v].name;
    } catch (...) {
      return DotStr::Misc::sUndefined;
    }
  }

  //DEBUG ONLY !!! force thread cursor to the value of the corresponding origin
  void CarpeDM::forceThrCursor(uint8_t cpuIdx, uint8_t thrIdx) {
    uint32_t cursor = ebd.read32b( getThrInitialNodeAdr(cpuIdx, thrIdx));
    ebd.write32b(getThrCurrentNodeAdr(cpuIdx, thrIdx), cursor);
  }

  //Get bitfield showing running threads
  uint32_t CarpeDM::getThrRun(uint8_t cpuIdx) {
    return ebd.read32b( getThrCmdAdr(cpuIdx) + T_TC_RUNNING);
  }

  //Get bifield showing running threads
  uint32_t CarpeDM::getStatus(uint8_t cpuIdx) {
    return ebd.read32b( atDown.getMemories()[cpuIdx].extBaseAdr + atDown.getMemories()[cpuIdx].sharedOffs + SHCTL_STATUS);
  }

  //Requests Threads to start
  vEbwrs& CarpeDM::setThrStart(uint8_t cpuIdx, uint32_t bits, vEbwrs& ew) {
    uint8_t b[4];

    ew.va.push_back(getThrCmdAdr(cpuIdx) + T_TC_START);
    writeLeNumberToBeBytes<uint32_t>(b, bits);
    ew.vb.insert( ew.vb.end(), b, b + sizeof(b));
    ew.vcs += leadingOne(1);
    createCmdModInfo(cpuIdx, 0, OP_TYPE_CMD_START, ew);
    return ew;
  }

  uint32_t CarpeDM::getThrStart(uint8_t cpuIdx) {
    return ebd.read32b( getThrCmdAdr(cpuIdx) + T_TC_START);
  }



  //Requests Threads to stop
  vEbwrs& CarpeDM::setThrAbort(uint8_t cpuIdx, uint32_t bits, vEbwrs& ew) {
    uint8_t b[4];

    ew.va.push_back(getThrCmdAdr(cpuIdx) + T_TC_ABORT);
    writeLeNumberToBeBytes<uint32_t>(b, bits);
    ew.vb.insert( ew.vb.end(), b, b + sizeof(b));
    ew.vcs += leadingOne(1);
    createCmdModInfo(cpuIdx, 0, OP_TYPE_CMD_ABORT, ew);
    return ew;
  }

  //hard abort everything, emergency only
  void CarpeDM::halt() {
    if (verbose) sLog << "Aborting all activity" << std::endl;
    vEbwrs ew;
    uint8_t b[4];
    writeLeNumberToBeBytes<uint32_t>(b, (1 << _THR_QTY_)-1 );


    for(uint8_t cpuIdx=0; cpuIdx < ebd.getCpuQty(); cpuIdx++) {
      setThrStart(cpuIdx, 0, ew);
      ew.va.push_back(getThrCmdAdr(cpuIdx) + T_TC_ABORT);
      ew.vb.insert( ew.vb.end(), b, b + sizeof(b));
      ew.vcs.push_back(true); // each one is a new wb device, so we always need a new eb cycle
      createCmdModInfo(cpuIdx, 0, OP_TYPE_CMD_HALT, ew);
    }

    ebd.writeCycle(ew.va, ew.vb, ew.vcs);
  }


  bool CarpeDM::isThrRunning(uint8_t cpuIdx, uint8_t thrIdx) {
    return (bool)(getThrRun(cpuIdx) & (1<< thrIdx));
  }

  //Requests Thread to start
  vEbwrs& CarpeDM::startThr(uint8_t cpuIdx, uint8_t thrIdx, vEbwrs& ew) {
    return setThrStart(cpuIdx, (1<<thrIdx), ew);
  }
/*
  //Requests Thread to stop
  void CarpeDM::stopThr(uint8_t cpuIdx, uint8_t thrIdx) {
    setThrStop(cpuIdx, (1<<thrIdx));
  }
*/
  //Immediately aborts a Thread
  vEbwrs& CarpeDM::abortThr(uint8_t cpuIdx, uint8_t thrIdx, vEbwrs& ew) {
    return setThrAbort(cpuIdx, (1<<thrIdx), ew);
  }




void  CarpeDM::resetThrMsgCnt(uint8_t cpuIdx, uint8_t thrIdx) {
  vEbwrs ew;
  resetThrMsgCnt(cpuIdx, thrIdx, ew);
  ebd.writeCycle(ew.va, ew.vb, ew.vcs);
}

vEbwrs& CarpeDM::resetThrMsgCnt(uint8_t cpuIdx, uint8_t thrIdx, vEbwrs& ew) {
  uint32_t msgCntAdr = atDown.getMemories()[cpuIdx].extBaseAdr + atDown.getMemories()[cpuIdx].sharedOffs + SHCTL_THR_DAT + thrIdx * _T_TD_SIZE_ + T_TD_MSG_CNT;

  ew.va.push_back(msgCntAdr + 0);
  ew.va.push_back(msgCntAdr + _32b_SIZE_);
  ew.vb.insert(ew.vb.end(), _64b_SIZE_, 0x00);
  ew.vcs += leadingOne(2);

  return ew;

}

uint64_t CarpeDM::getThrMsgCnt(uint8_t cpuIdx, uint8_t thrIdx) {
  return ebd.read64b(atDown.getMemories()[cpuIdx].extBaseAdr + atDown.getMemories()[cpuIdx].sharedOffs + SHCTL_THR_DAT + thrIdx * _T_TD_SIZE_ + T_TD_MSG_CNT);
}

uint64_t CarpeDM::getThrDeadline(uint8_t cpuIdx, uint8_t thrIdx) {
  return ebd.read64b(atDown.getMemories()[cpuIdx].extBaseAdr + atDown.getMemories()[cpuIdx].sharedOffs + SHCTL_THR_DAT + thrIdx * _T_TD_SIZE_ + T_TD_DEADLINE);
}

//FIXME wtf ... this doesnt queue anything!
vEbwrs&  CarpeDM::setThrStartTime(uint8_t cpuIdx, uint8_t thrIdx, uint64_t t, vEbwrs& ew) {
  uint32_t startAdr = atDown.getMemories()[cpuIdx].extBaseAdr + atDown.getMemories()[cpuIdx].sharedOffs + SHCTL_THR_STA + thrIdx * _T_TS_SIZE_ + T_TS_STARTTIME;
  ew.va += {startAdr, startAdr + _32b_SIZE_};
  writeLeNumberToBeBytes<uint64_t>(ew.vb, t );
  ew.vcs += leadingOne(2);
  return ew;
}

uint64_t CarpeDM::getThrStartTime(uint8_t cpuIdx, uint8_t thrIdx) {
  return ebd.read64b(atDown.getMemories()[cpuIdx].extBaseAdr + atDown.getMemories()[cpuIdx].sharedOffs + SHCTL_THR_STA + thrIdx * _T_TS_SIZE_ + T_TS_STARTTIME);
}

//FIXME wtf ... this doesnt queue anything!
vEbwrs&  CarpeDM::setThrPrepTime(uint8_t cpuIdx, uint8_t thrIdx, uint64_t t, vEbwrs& ew) {
  uint32_t startAdr = atDown.getMemories()[cpuIdx].extBaseAdr + atDown.getMemories()[cpuIdx].sharedOffs + SHCTL_THR_STA + thrIdx * _T_TS_SIZE_ + T_TS_PREPTIME;
  ew.va += {startAdr, startAdr + _32b_SIZE_};
  writeLeNumberToBeBytes<uint64_t>(ew.vb, t );
  ew.vcs += leadingOne(2);
  return ew;
}

uint64_t CarpeDM::getThrPrepTime(uint8_t cpuIdx, uint8_t thrIdx) {
  return ebd.read64b(atDown.getMemories()[cpuIdx].extBaseAdr + atDown.getMemories()[cpuIdx].sharedOffs + SHCTL_THR_STA + thrIdx * _T_TS_SIZE_ + T_TS_PREPTIME);
}

const vAdr CarpeDM::getCmdWrAdrs(uint32_t hash, uint8_t prio) {
  vAdr ret;

  //find the address corresponding to given name
  auto it = atDown.lookupHash(hash, carpeDMcommand::exIntro);
  auto* x = (AllocMeta*)&(*it);

  //Check if requested queue priority level exists
  uint32_t blAdr = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&x->b[BLOCK_CMDQ_PTRS + prio * _PTR_SIZE_]);
  //sLog << "Block BListAdr 0x" << std::hex << blAdr << std::endl;
  if(blAdr == LM32_NULL_PTR) {throw std::runtime_error( "Block Node does not have requested queue"); return ret; }

    //get Write and Read indices
  uint8_t eWrIdx = ( writeBeBytesToLeNumber<uint32_t>((uint8_t*)&x->b[BLOCK_CMDQ_WR_IDXS]) >> (prio * 8)) & Q_IDX_MAX_OVF_MSK;
  uint8_t wrIdx  = eWrIdx & Q_IDX_MAX_MSK;
  uint8_t eRdIdx = ( writeBeBytesToLeNumber<uint32_t>((uint8_t*)&x->b[BLOCK_CMDQ_RD_IDXS]) >> (prio * 8)) & Q_IDX_MAX_OVF_MSK;
  uint8_t rdIdx  = eRdIdx & Q_IDX_MAX_MSK;

  //Check if queue is not full
  //sLog << "wrIdx " << (int)wrIdx << " rdIdx " << (int)rdIdx << " ewrIdx " << (int)eWrIdx << " rdIdx " << (int)rdIdx << " eRdIdx " << eRdIdx << std::endl;
  if ((wrIdx == rdIdx) && (eWrIdx != eRdIdx)) {throw std::runtime_error( gDown[x->v].name + " queue of prio " + std::to_string((int)prio) + " is full, can't write.\n");}
  //lookup Buffer List
  it = atDown.lookupAdr(x->cpu, atDown.adrConv(AdrType::INT, AdrType::MGMT, x->cpu, blAdr), carpeDMcommand::exIntro);
  auto* pmBl = (AllocMeta*)&(*it);

  //calculate write offset

  ptrdiff_t bufIdx   = wrIdx / (_MEM_BLOCK_SIZE / _T_CMD_SIZE_  );
  ptrdiff_t elemIdx  = wrIdx % (_MEM_BLOCK_SIZE / _T_CMD_SIZE_  );

  //sLog << "bIdx " << bufIdx << " eIdx " << elemIdx << " @ 0x" << std::hex << pmBl->adr << std::endl;
  uint32_t  startAdr = atDown.adrConv(AdrType::INT, AdrType::EXT,pmBl->cpu, writeBeBytesToLeNumber<uint32_t>((uint8_t*)&pmBl->b[bufIdx * _PTR_SIZE_])) + elemIdx * _T_CMD_SIZE_;

  //sLog << "Current BufAdr 0x" << std::hex << startAdr << std::endl;

  //generate command address range
  for(uint32_t adr = startAdr; adr < startAdr + _T_CMD_SIZE_; adr += _32b_SIZE_) ret.push_back(adr);

  //and insert address for wr idx increment
  ret.push_back(atDown.adrConv(AdrType::MGMT, AdrType::EXT, x->cpu, x->adr) + BLOCK_CMDQ_WR_IDXS);
  return ret;


}


  const uint32_t CarpeDM::getCmdInc(uint32_t hash, uint8_t prio) {
    uint32_t newIdxs;
    uint8_t  eWrIdx;

    //find the address corresponding to given name
    auto it = atDown.lookupHash(hash, carpeDMcommand::exIntro);
    auto* x = (AllocMeta*)&(*it);
        //sLog << "indices: 0x" << std::hex << writeBeBytesToLeNumber<uint32_t>((uint8_t*)&x->b[BLOCK_CMDQ_WR_IDXS]) << std::endl;
    //get incremented Write index of requested prio
    eWrIdx = ( writeBeBytesToLeNumber<uint32_t>((uint8_t*)&x->b[BLOCK_CMDQ_WR_IDXS]) >> (prio * 8)) & Q_IDX_MAX_OVF_MSK;
    //assign to index vector
    newIdxs = ( writeBeBytesToLeNumber<uint32_t>((uint8_t*)&x->b[BLOCK_CMDQ_WR_IDXS]) & ~(0xff << (prio * 8))) | (((eWrIdx +1)  & Q_IDX_MAX_OVF_MSK) << (prio * 8));
    //write back so next call will find correct value without need for another download
    writeLeNumberToBeBytes<uint32_t>((uint8_t*)&x->b[BLOCK_CMDQ_WR_IDXS], newIdxs);


    return newIdxs;
  }


std::pair<int, int> CarpeDM::findRunningPattern(const std::string& sPattern) {
  std::pair<int, int> res = {-1, -1};
  vStrC members   = getPatternMembers (sPattern);
  try { res.first = getNodeCpu(firstString(members), TransferDir::DOWNLOAD); } catch (...) {res.first = -1; return res;}

  uint32_t thrds  = getThrRun(res.first);

  vStrC cursors;

  for (int i = 0; i < _THR_QTY_; i++ ) { cursors.push_back(getThrCursor(res.first, i)); }

  for (int i = 0; i < _THR_QTY_; i++ ) {
    //Get cursors of all running threads not idle or uninitialised
    if ( (thrds & (1<<i)) && (cursors[i] != DotStr::Misc::sUndefined) && (cursors[i] != DotStr::Node::Special::sIdle) ) {
      // now the footwork: check if this cursor was pointing to a member of our pattern
      bool found = false;
      for ( auto& sM : members ) { if (cursors[i] == sM) found = true; }
      if (found) {res.second = i; break;} //yep, we found the idx of the thread running our pattern
    }
  }

  return res;

}

bool CarpeDM::isPatternRunning(const std::string& sPattern) {
  auto cpuAndThr = findRunningPattern(sPattern);
  return ((cpuAndThr.first >= 0) && (cpuAndThr.second >= 0));
}

//Returns the the index of the first idle thread at cpu <cpuIdx>
int CarpeDM::getIdleThread(uint8_t cpuIdx) {
  uint32_t thrds = getThrRun(cpuIdx);
  int i;
  for (i = 0; i < _THR_QTY_; i++) { if(!(bool)(thrds & (1<<i))) return i; } // aborts at free thrIdx or returns _THR_QTY_ if no frees found
  return _THR_QTY_;
}

//Requests Pattern to start on thread <x>
vEbwrs& CarpeDM::startPattern(const std::string& sPattern, uint8_t thrIdx, vEbwrs& ew) { return startNodeOrigin(getPatternEntryNode(sPattern), thrIdx, ew);}
//Requests Pattern to start
vEbwrs& CarpeDM::startPattern(const std::string& sPattern, vEbwrs& ew) { return startNodeOrigin(getPatternEntryNode(sPattern), ew); }

//Requests Pattern to stop
vEbwrs& CarpeDM::stopPattern(const std::string& sPattern, vEbwrs& ew) { return stopNodeOrigin(getPatternExitNode(sPattern), ew); }

//Immediately aborts a Pattern
vEbwrs& CarpeDM::abortPattern(const std::string& sPattern, vEbwrs& ew) {
  std::pair<int, int> cpuAndThr = findRunningPattern(sPattern);
  //if we didn't find it, it's not running now. So no problem that we cannot abort it
  if ((cpuAndThr.first >= 0) && (cpuAndThr.second >= 0)) return abortThr((uint8_t)cpuAndThr.first, (uint8_t)cpuAndThr.second, ew);
  return ew;
}


//Requests thread <thrIdx> to start at node <sNode>
vEbwrs& CarpeDM::startNodeOrigin(const std::string& sNode, uint8_t thrIdx, vEbwrs& ew) {
  uint8_t cpuIdx    = getNodeCpu(sNode, TransferDir::DOWNLOAD);
  setThrOrigin(cpuIdx, thrIdx, sNode, ew); //configure thread and run it
  startThr(cpuIdx, thrIdx, ew);
  return ew;
}
//Requests a start at node <sNode>
vEbwrs& CarpeDM::startNodeOrigin(const std::string& sNode, vEbwrs& ew) {
  uint8_t cpuIdx    = getNodeCpu(sNode, TransferDir::DOWNLOAD);
  int thrIdx = 0; //getIdleThread(cpuIdx); //find a free thread we can use to run our pattern
  if (thrIdx == _THR_QTY_) throw std::runtime_error( "Found no free thread on " + std::to_string(cpuIdx) + "'s hosting cpu");
  setThrOrigin(cpuIdx, thrIdx, sNode, ew); //configure thread and run it
  startThr(cpuIdx, (uint8_t)thrIdx, ew);
  return ew;
}

//Requests stop at node <sNode> (flow to idle)
vEbwrs& CarpeDM::stopNodeOrigin(const std::string& sNode, vEbwrs& ew) {
  //send a command: tell patternExitNode to change the flow to Idle
  return createFlowCommand(ew, dnt::sCmdFlow, sNode, DotStr::Node::Special::sIdle, PRIO_LO, 1, true, 0, false);
}

//Immediately aborts the thread whose pattern <sNode> belongs to
vEbwrs& CarpeDM::abortNodeOrigin(const std::string& sNode, vEbwrs& ew) {
  std::string sPattern = getNodePattern(sNode);
  std::pair<int, int> cpuAndThr = findRunningPattern(sPattern);
  //if we didn't find it, it's not running now. So no problem that we cannot abort it
  if ((cpuAndThr.first >= 0) && (cpuAndThr.second >= 0)) return abortThr((uint8_t)cpuAndThr.first, (uint8_t)cpuAndThr.second, ew);
  return ew;
}


  const std::string CarpeDM::getNodePattern (const std::string& sNode)          {return firstString(gt.getGroups<Groups::Node, &GroupMeta::pattern>(sNode));}
  const std::string CarpeDM::getNodeBeamproc(const std::string& sNode)          {return firstString(gt.getGroups<Groups::Node, &GroupMeta::beamproc>(sNode));}
              vStrC CarpeDM::getPatternMembers (const std::string& sPattern)    {return gt.getMembers<Groups::Pattern>(sPattern);}
  const std::string CarpeDM::getPatternEntryNode(const std::string& sPattern)   {return firstString(gt.getPatternEntryNodes(sPattern));}
  const std::string CarpeDM::getPatternExitNode(const std::string& sPattern)    {return firstString(gt.getPatternExitNodes(sPattern));}
              vStrC CarpeDM::getBeamprocMembers(const std::string& sBeamproc)   {return gt.getMembers<Groups::Pattern>(sBeamproc);}
  const std::string CarpeDM::getBeamprocEntryNode(const std::string& sBeamproc) {return firstString(gt.getBeamprocEntryNodes(sBeamproc));}
  const std::string CarpeDM::getBeamprocExitNode(const std::string& sBeamproc)  {return firstString(gt.getBeamprocExitNodes(sBeamproc));}



  vStrC CarpeDM::getGraphPatterns(Graph& g)  {
    std::set<std::string> sP, log;
    vStrC ret;

    BOOST_FOREACH( vertex_t v, vertices(g) ) {

      //std::cout << g[v].name << " ---> " << getNodePattern(g[v].name) << std::endl;
      std::string tmpPatName = g[v].patName;
      if (tmpPatName == DotStr::Misc::sUndefined) log.insert(g[v].name);
      sP.insert(tmpPatName);

    }

    for(auto& itP : sP) ret.push_back(itP);
    if (log.size() > 0) {
      sErr << "Warning: getGraphPatterns found no valid patterns for the following nodes:" << std::endl;
      for(auto& itL : log) sErr << itL << std::endl;
    }
    return ret;


  }




vertex_set_t CarpeDM::getAllCursors(bool activeOnly) {
  vertex_set_t ret;

  //TODO - this is dirty and cumbersome, make it streamlined


  for(uint8_t cpu = 0; cpu < ebd.getCpuQty(); cpu++) { //cycle all CPUs
    for(uint8_t thr = 0; thr < _THR_QTY_; thr++) {
      uint32_t adr = ebd.read32b( getThrCurrentNodeAdr(cpu, thr));
      uint64_t  dl = getThrDeadline(cpu, thr);
      if (adr == LM32_NULL_PTR || (activeOnly && ((int64_t)dl == -1))) continue; // only active cursors: no dead end idles, no aborted threads
      try {
        auto x = atDown.lookupAdr(cpu, atDown.adrConv(AdrType::INT, AdrType::MGMT,cpu, adr));
        ret.insert(x->v);
      } catch(...) {}
    }
      //add all thread cursors addresses of CPU <i>

      //create triplicate version to allow majority vote on all values

      //pass to ebReadCycle

      //majority vote on results

      //parse pointers. each pointer successfully translated to a vertex idx goes into the result set

  }

  return ret;

}


int CarpeDM::staticFlushPattern(const std::string& sPattern, bool prioIl, bool prioHi, bool prioLo, vEbwrs& ew, bool force) {
  Graph& g = gDown;
  AllocTable& at = atDown;


  bool found = false;

  for (auto& nodeIt : getPatternMembers(sPattern)) {
    if (hm.contains(nodeIt) ) {
      found = true;
      auto x = at.lookupHash(hm.lookup(nodeIt), carpeDMcommand::exIntro);
      if (g[x->v].np->isBlock()) { staticFlush(g[x->v].name, prioIl, prioHi, prioLo, ew, force); }
    }
  }
  if (!found) {throw std::runtime_error( "staticFlush: No member nodes found for pattern <" + sPattern + ">. Wrong name?");}

  send(ew);
  return ew.va.size();
}

int CarpeDM::staticFlushBlock(const std::string& sBlock, bool prioIl, bool prioHi, bool prioLo, vEbwrs& ew, bool force) {

  send(staticFlush(sBlock, prioIl, prioHi, prioLo, ew, force));
  return ew.va.size();
}


vEbwrs& CarpeDM::staticFlush(const std::string& sBlock, bool prioIl, bool prioHi, bool prioLo, vEbwrs& ew, bool force) {
  Graph& g = gDown;
  AllocTable& at = atDown;

  //check if the block can safely be modified
  //call safe2remove on the block's pattern. If remove is ok, so is Flushing
  std::string dbgReport;
  std::string report;
  const std::string sPattern = getNodePattern(sBlock);
  //if (!isSafeToRemove(sPattern, report,)) {printf("Cannot safely be removed\n"); writeTextFile("safetyReportNormal.dot", report); }


  if ( (!isSafeToRemove(sPattern, dbgReport)) && !force)  {
    if(debug) sLog << dbgReport << std::endl;
    throw std::runtime_error(carpeDMcommand::exIntro + "staticFlush: Pattern <" + sPattern + "> of block member <" + sBlock + "> is active, static flush not safely possible!");
  }

  //check against covenants

  if(optimisedS2R && isCovenantPending(sBlock)) throw std::runtime_error(carpeDMcommand::exIntro + "staticFlush: cannot flush, block <" + sBlock + "> is in a safe2remove-covenant!");


  if(verbose) sLog << "Trying to flush block <" << sBlock << ">" << std::endl;

  //get the block
  auto x = at.lookupHash(hm.lookup(sBlock, carpeDMcommand::exIntro));
  uint32_t cpyMsk = 0;
  uint32_t wrIdxs = boost::dynamic_pointer_cast<Block>(g[x->v].np)->getWrIdxs();
  uint32_t rdIdxs = boost::dynamic_pointer_cast<Block>(g[x->v].np)->getRdIdxs();

  if (prioIl) cpyMsk |= (0xff << (PRIO_IL*8));
  if (prioHi) cpyMsk |= (0xff << (PRIO_HI*8));
  if (prioLo) cpyMsk |= (0xff << (PRIO_LO*8));

  if (!cpyMsk) return ew; //no queues to flush, abort

  //create new rdIdx values
  uint8_t bTmp[4];
  uint32_t newRdIdxs = (rdIdxs & ~cpyMsk) | (wrIdxs & cpyMsk);
  writeLeNumberToBeBytes(bTmp, newRdIdxs);

  //add to etherbone write
  ew.va.push_back(at.adrConv(AdrType::MGMT, AdrType::EXT, x->cpu, x->adr) + BLOCK_CMDQ_RD_IDXS); //ext address of this block + BLOCK_CMDQ_RD_IDXS
  ew.vcs += leadingOne(1);
  ew.vb.insert( ew.vb.end(), bTmp, bTmp + _32b_SIZE_);

  return ew;

}

vEbwrs& CarpeDM::deactivateOrphanedCommands(std::vector<QueueReport>& vQr, vEbwrs& ew) {
  for (auto& qr : vQr) {
     for (int8_t prio = PRIO_IL; prio >= PRIO_LO; prio--) {

      if (!qr.hasQ[prio]) {continue;}
      //find buffers of all non empty slots
      for (uint8_t i, idx = qr.aQ[prio].rdIdx; idx < qr.aQ[prio].rdIdx + 4; idx++) {
        i = idx & Q_IDX_MAX_MSK;
        QueueElement& qe = qr.aQ[prio].aQe[i];

        if(qe.orphaned) {
          if (verbose) sLog << "Deactivating orphaned command @ " << qr.name << " prio " << std::dec << (int)prio << " slot " << (int)i << std::hex << ", adr of action field is 0x" << qe.extAdr + T_CMD_ACT << std::endl;
          qe.qty = 0; // deactivate command execution
          //reconstruct action field from report. bit awkward, but not really bad either.
          uint32_t action = (qe.type << ACT_TYPE_POS) | ((prio & ACT_PRIO_MSK) << ACT_PRIO_POS) | ((qe.qty & ACT_QTY_MSK) << ACT_QTY_POS) | (qe.validAbs << ACT_VABS_POS) | (qe.flowPerma << ACT_CHP_POS );
          ew.va.push_back(qe.extAdr + T_CMD_ACT);
          ew.vcs += leadingOne(1);
          uint8_t bTmp[4];
          writeLeNumberToBeBytes(bTmp, action);
          ew.vb.insert( ew.vb.end(), bTmp, bTmp + _32b_SIZE_);
        }
      }
    }
  }

  return ew;
}
