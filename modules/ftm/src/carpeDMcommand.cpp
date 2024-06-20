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

#include "carpeDMimpl.h"
#include "minicommand.h"
#include "propwrite.h"

#include "node.h"
#include "block.h"
#include "meta.h"
#include "global.h"
#include "event.h"
#include "dotstr.h"


namespace dnt = DotStr::Node::TypeVal;
namespace det = DotStr::Edge::TypeVal;

namespace carpeDMcommand {
  const std::string exIntro = "carpeDMcommand: ";
}

vEbwrs& CarpeDM::CarpeDMimpl::blockAsyncClearQueues(vEbwrs& ew, const std::string& sTarget) {
  uint32_t adrBase    = getNodeAdr(sTarget, TransferDir::DOWNLOAD, AdrType::EXT);
  //reset read and write indices
  vAdr tmp = {(adrBase + BLOCK_CMDQ_RD_IDXS), (adrBase + BLOCK_CMDQ_WR_IDXS)};
  ew.va += tmp;
  ew.vb += {0, 0, 0, 0, 0, 0, 0, 0}; // TODO: tidy this up, this is hackish
  ew.vcs += leadingOne(tmp.size());
  return ew;
}

vEbwrs& CarpeDM::CarpeDMimpl::switching(vEbwrs& ew, const std::string& sTarget, const std::string& sDst) {
  uint32_t tadr = getNodeAdr(sTarget, TransferDir::DOWNLOAD, AdrType::EXT) + NODE_DEF_DEST_PTR;
  uint32_t dadr = getNodeAdr(sDst, TransferDir::DOWNLOAD, AdrType::INT);
  //sLog << "switch conv 0x" << std::hex << dadr << std::endl;
  //overwrite def dst ptr
  ew.va += tadr;
  writeLeNumberToBeBytes<uint32_t>(ew.vb, dadr);
  ew.vcs += leadingOne(1);
  return ew;
}


vStrC CarpeDM::CarpeDMimpl::getLockedBlocks(bool checkReadLock, bool checkWriteLock) {
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



boost::optional<std::pair<int, int>> CarpeDM::CarpeDMimpl::parseCpuAndThr(vertex_t v, Graph& g) {

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


  if ( (cpu < 0) || (cpu >= ebd.getCpuQty()) || (thr < 0) || (thr >= ebd.getThrQty()  ) ) {
    //sLog << "OOR, returning none" << std::endl;
    return boost::optional<std::pair<int, int>>();
  }


  //sLog << "Valid, returning " << res.first << " " << res.second << std::endl;
  return res;
}


void CarpeDM::CarpeDMimpl::adjustValidTime(uint64_t& tValid, bool abs) {
  // All tValids must be in the future when written so host speed does not influency command availability to Firmware
  // Find a point in time which will safely be in the near future when we write this command
  uint64_t tFuture    = modTime + (testmode ? 0ULL : processingTimeMargin);  // no margin for sim, otherwise coverage testing is too slow.
  // make copy, choose and update original.
  uint64_t tOriginal  = tValid;
  tValid = abs ? tOriginal : tFuture + tOriginal; // if absolute -> max of original and near future, if relative -> sum of original and future
}


vEbwrs& CarpeDM::CarpeDMimpl::createCommand(vEbwrs& ew, const std::string& type, const std::string& target, const std::string& destination,
  uint8_t  cmdPrio, uint8_t cmdQty, bool vabs, uint64_t cmdTvalid, bool perma, bool qIl, bool qHi, bool qLo,  uint64_t cmdTwait, bool abswait, bool lockRd, bool lockWr, uint8_t cmdThr )
{
    mc_ptr mc;
    adjustValidTime(cmdTvalid, vabs);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Global Commands (not targeted at cmd queues of individual blocks)
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////



    //Start is different to stop - start uses 'destination', stop uses target (entry node vs exit block)
    if (type == dnt::sCmdStart)   {
      sLog << "Yep, its a start allright" << std::endl;
      if (hm.lookup(destination)) {sLog << " Starting at <" << destination << ">" << std::endl; startNodeOrigin(ew, destination, cmdThr, cmdTvalid);  }
      else {throw std::runtime_error("Cannot execute command '" + type + "' No valid cpu/thr provided and '" + destination + "' is not a valid node name\n");}
      return ew;
    }
    if (type == dnt::sCmdStop)    {
      if (hm.lookup(target)) { sLog << " Stopping at <" << target << ">" << std::endl; stopNodeOrigin(ew, target); }
      else {throw std::runtime_error("Cannot execute command '" + type + "' No valid cpu/thr provided and '" + target + "' is not a valid node name\n");}
      return ew;
    }
    else if (type == dnt::sCmdAbort)   {
      if (hm.lookup(target)) {sLog << " Aborting (trying) at <" << target << ">" << std::endl; abortNodeOrigin(ew, target); }
      else {throw std::runtime_error("Cannot execute command '" + type + "'. No valid cpu/thr provided and '" + target + "' is not a valid node name\n");}
      return ew;
    }

    //FIMXE hack to test compile
    uint8_t thr __attribute__((unused)) = 0;

    //Origin
    if (type == dnt::sCmdOrigin)   {
      if (hm.lookup(destination)) {
        sLog << " Setting Origin to <" << destination << ">" << std::endl;
        uint8_t cpuIdx    = getNodeCpu(destination, TransferDir::DOWNLOAD); // a node can only run on the cpu it resides
        setThrOrigin(ew, cpuIdx, cmdThr, destination); //configure thread and run it
      }
      else {throw std::runtime_error("Cannot execute command '" + type + "' No valid cpu/thr provided and '" + destination + "' is not a valid node name\n");}
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
      blockAsyncClearQueues(ew, target);
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

    } else if (type == dnt::sSwitch) {

      switching(ew, target, destination);
      return ew;

    } else if (type == dnt::sCmdFlush) { // << " Flushing <" << target < < "> Queues IL " << s2u<int>(g[v].qIl) << " HI " << s2u<int>(g[v].qHi) << " LO " << s2u<int>(g[v].qLo) <<  std::endl;
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
    createMiniCommand(ew, target, cmdPrio, mc);

  return ew;

}


// FIXME god this is awful ... replace with builder pattern!
//wrappers
//commands with no extras
vEbwrs& CarpeDM::CarpeDMimpl::createNonQCommand(vEbwrs& ew, const std::string& type, const std::string& target, uint8_t cmdThr) {
  return createCommand(ew, type, target, "", 0, 1, true, 0, false, false, false, false, 0, false, false, false, cmdThr);
}

vEbwrs& CarpeDM::CarpeDMimpl::createLockCtrlCommand(vEbwrs& ew, const std::string& type, const std::string& target, bool lockRd, bool lockWr ) {
  return createCommand(ew, type, target, "", 0, 1, true, 0, false, false, false, false, 0, false, lockRd, lockWr, 0 );
}

//commands with time
vEbwrs& CarpeDM::CarpeDMimpl::createQCommand(vEbwrs& ew, const std::string& type, const std::string& target, uint8_t cmdPrio, uint8_t cmdQty, bool vabs, uint64_t cmdTvalid, uint8_t cmdThr) {
  return createCommand(ew, type, target, "", cmdPrio, cmdQty, vabs, cmdTvalid, false, false, false, false, 0, false, false, false, cmdThr);
}

//flows
vEbwrs& CarpeDM::CarpeDMimpl::createFlowCommand(vEbwrs& ew, const std::string& type, const std::string& target, const std::string& destination,
  uint8_t  cmdPrio, uint8_t cmdQty, bool vabs, uint64_t cmdTvalid, bool perma) {
  return createCommand(ew, type, target, destination, cmdPrio, cmdQty, vabs, cmdTvalid, perma, false, false, false, 0, false, false, false, 0);
}

//flush or flush override
vEbwrs& CarpeDM::CarpeDMimpl::createFlushCommand(vEbwrs& ew, const std::string& type, const std::string& target, const std::string& destination,
  uint8_t  cmdPrio, uint8_t cmdQty, bool vabs, uint64_t cmdTvalid, bool qIl, bool qHi, bool qLo) {
  return createCommand(ew, type, target, destination, cmdPrio, cmdQty, vabs, cmdTvalid, false, qIl, qHi, qLo, 0, false, false, false, 0);
}

//wait
vEbwrs& CarpeDM::CarpeDMimpl::createWaitCommand(vEbwrs& ew, const std::string& type, const std::string& target,
  uint8_t  cmdPrio, uint8_t cmdQty, bool vabs, uint64_t cmdTvalid, uint64_t cmdTwait, bool abswait ) {
  return createCommand(ew, type, target, "", cmdPrio, cmdQty, vabs, cmdTvalid, false, false, false, false, cmdTwait, abswait, false, false, 0);
}

vEbwrs& CarpeDM::CarpeDMimpl::createFullCommand(vEbwrs& ew, const std::string& type, const std::string& target, const std::string& destination,
  uint8_t  cmdPrio, uint8_t cmdQty, bool vabs, uint64_t cmdTvalid, bool perma, bool qIl, bool qHi, bool qLo, uint64_t cmdTwait, bool abswait,bool lockRd, bool lockWr, uint8_t cmdThr )
{
  updateModTime();
  return createCommand(ew, type, target, destination, cmdPrio, cmdQty, vabs, perma, qIl, qHi, qLo, cmdTvalid, cmdTwait, abswait, lockRd, lockWr, cmdThr);
}






vEbwrs& CarpeDM::CarpeDMimpl::createCommandBurst(vEbwrs& ew, Graph& g) {

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
    uint8_t cmdThr;

    //use the pattern and beamprocess tags to determine the target. Pattern overrides Beamprocess overrides cpu/thread
          if (g[v].patName  != DotStr::Misc::sUndefined)  { target = getPatternExitNode(g[v].patName); destination = getPatternEntryNode(g[v].patName); }
    else  if (g[v].bpName   != DotStr::Misc::sUndefined)  { target = getBeamprocExitNode(g[v].bpName); destination = getBeamprocEntryNode(g[v].patName); }
    else  { target = g[v].cmdTarget; }

    //use the destPattern and destBeamprocess tags to determine the destination
    if       (g[v].cmdDestPat   != DotStr::Misc::sUndefined)  { destination = getPatternEntryNode(g[v].cmdDestPat); }
    else  if (g[v].cmdDestBp    != DotStr::Misc::sUndefined)  { destination = getBeamprocEntryNode(g[v].cmdDestBp); }
    else  if (g[v].cmdDest      != DotStr::Misc::sUndefined)  { destination = g[v].cmdDest;}
    else  { destination = getPatternEntryNode(g[v].patName); } // if there is no destination, assign target to destination

    try {
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

    cmdThr    = s2u<uint8_t>(g[v].cmdDestThr);
    sLog << "g[v].cmdDestThr == " << g[v].cmdDestThr << " == Num" << cmdThr << std::endl;
    lockRd    = true;
    lockWr    = true;
    //fixme hack to test compile
    abswait   = false;

    if(verbose) sLog << "Command <" << g[v].name << ">, type <" << g[v].type << ">" << std::endl;
    createCommand(ew, type, target, destination, cmdPrio, cmdQty, vabs, cmdTvalid, perma, qIl, qHi, qLo, cmdTwait, abswait, lockRd, lockWr, cmdThr);
    } catch (std::runtime_error const& err) {
        throw std::runtime_error( "Parser error when processing command <" + g[v].name + ">. Cause: " + err.what());
    }
  }

  return ew;

}

  int CarpeDM::CarpeDMimpl::send(vEbwrs& ew) {

    bool locksRdy;
    //sLog << "reading lockstates...";
    lm.readInStat(); //read current lock states
    //sLog << "done" << std::endl << " Setting locks...";
    lm.processLockRequests(); //set requested locks (or'ed with already set locks)
    //sLog << "done" << std::endl;
    //3 retries for lock readiness
    for(unsigned i = 0; i < 3; i++) {
      //sLog << "checking locks";
      locksRdy = lm.isReady();
      //sLog << "check performed" << std::endl;
      if (locksRdy) {
        //sLog << "locks ready. writing commands";
        ebd.writeCycle(ew.va, ew.vb, ew.vcs); //if ready, write commands. if not, restore locks and throw exception
        //sLog << "writing commands" << std::endl;
        break;
      }
      std::chrono::milliseconds timespan(1);
      std::this_thread::sleep_for(timespan);
    }
    //sLog << "done." << std::endl << "Unlocking...";
    lm.processUnlockRequests(); //restore lock bits
    if (!locksRdy) throw std::runtime_error("Could not write commands, locking failed");
    //sLog << "done." << std::endl;
    return ew.vb.size();
  }


  vEbwrs& CarpeDM::CarpeDMimpl::createMiniCommand(vEbwrs& ew, const std::string& targetName, uint8_t cmdPrio, mc_ptr mc) {

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
    createCmdModInfo(ew, getNodeCpu(targetName, TransferDir::DOWNLOAD), 0, opType);

    return ew;
  }


  //Returns the external address of a thread's command register area
  uint32_t CarpeDM::CarpeDMimpl::getThrCmdAdr(uint8_t cpuIdx) {
    return atDown.getMemories()[cpuIdx].extBaseAdr + atDown.getMemories()[cpuIdx].sharedOffs + SHCTL_THR_CTL;
  }



  //Returns the external address of a thread's initial node register
  uint32_t CarpeDM::CarpeDMimpl::getThrInitialNodeAdr(uint8_t cpuIdx, uint8_t thrIdx) {
    return atDown.getMemories()[cpuIdx].extBaseAdr + atDown.getMemories()[cpuIdx].sharedOffs + ebd.getCtlAdr(ADRLUT_SHCTL_THR_STA) + thrIdx * _T_TS_SIZE_ + T_TS_NODE_PTR;
  }

  //Returns the external address of a thread's cursor pointer
  uint32_t CarpeDM::CarpeDMimpl::getThrCurrentNodeAdr(uint8_t cpuIdx, uint8_t thrIdx) {
    return atDown.getMemories()[cpuIdx].extBaseAdr + atDown.getMemories()[cpuIdx].sharedOffs + ebd.getCtlAdr(ADRLUT_SHCTL_THR_DAT) + thrIdx * _T_TD_SIZE_ + T_TD_NODE_PTR;
  }


  //Sets the Node the Thread will start from
  vEbwrs& CarpeDM::CarpeDMimpl::setThrOrigin(vEbwrs& ew, uint8_t cpuIdx, uint8_t thrIdx, const std::string& name) {
    uint8_t b[4];

    ew.va.push_back(getThrInitialNodeAdr(cpuIdx, thrIdx));
    writeLeNumberToBeBytes<uint32_t>(b, getNodeAdr(name, TransferDir::DOWNLOAD, AdrType::INT));
    ew.vb.insert( ew.vb.end(), b, b + sizeof(b));
    ew.vcs += leadingOne(1);
    return ew;
  }

  //Returns the Node the Thread will start from
  const std::string CarpeDM::CarpeDMimpl::getThrOrigin(uint8_t cpuIdx, uint8_t thrIdx) {
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

  //DEBUG Sets the cursor
  vEbwrs& CarpeDM::CarpeDMimpl::setThrCursor(vEbwrs& ew, uint8_t cpuIdx, uint8_t thrIdx, const std::string& name) {
    uint8_t b[4];

    ew.va.push_back(getThrCurrentNodeAdr(cpuIdx, thrIdx));
    writeLeNumberToBeBytes<uint32_t>(b, getNodeAdr(name, TransferDir::DOWNLOAD, AdrType::INT));
    ew.vb.insert( ew.vb.end(), b, b + sizeof(b));
    ew.vcs += leadingOne(1);
    return ew;
  }

  const std::string CarpeDM::CarpeDMimpl::getThrCursor(uint8_t cpuIdx, uint8_t thrIdx) {
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
  void CarpeDM::CarpeDMimpl::forceThrCursor(uint8_t cpuIdx, uint8_t thrIdx) {
    uint32_t cursor = ebd.read32b( getThrInitialNodeAdr(cpuIdx, thrIdx));
    ebd.write32b(getThrCurrentNodeAdr(cpuIdx, thrIdx), cursor);
  }

  //Get bitfield showing running threads
  uint32_t CarpeDM::CarpeDMimpl::getThrRun(uint8_t cpuIdx) {
    return ebd.read32b( getThrCmdAdr(cpuIdx) + T_TC_RUNNING);
  }


  //Get bifield showing running threads
  uint32_t CarpeDM::CarpeDMimpl::getStatus(uint8_t cpuIdx) {
    return ebd.read32b( atDown.getMemories()[cpuIdx].extBaseAdr + atDown.getMemories()[cpuIdx].sharedOffs + SHCTL_STATUS);
  }

  //Requests Threads to start
  vEbwrs& CarpeDM::CarpeDMimpl::setThrStart(vEbwrs& ew, uint8_t cpuIdx, uint32_t bits) {
    uint8_t b[4];
    uint32_t mask = (uint32_t)((1ll<<ebd.getThrQty())-1);
    ew.va.push_back(getThrCmdAdr(cpuIdx) + T_TC_START);
    writeLeNumberToBeBytes<uint32_t>(b, bits & mask);
    ew.vb.insert( ew.vb.end(), b, b + sizeof(b));
    ew.vcs += leadingOne(1);
    createCmdModInfo(ew, cpuIdx, 0, OP_TYPE_CMD_START);
    return ew;
  }

  uint32_t CarpeDM::CarpeDMimpl::getThrStart(uint8_t cpuIdx) {
    return ebd.read32b( getThrCmdAdr(cpuIdx) + T_TC_START);
  }



  //Requests Threads to stop
  vEbwrs& CarpeDM::CarpeDMimpl::setThrAbort(vEbwrs& ew, uint8_t cpuIdx, uint32_t bits) {
    uint8_t b[4];
    uint32_t mask = (uint32_t)((1ll<<ebd.getThrQty())-1);
    ew.va.push_back(getThrCmdAdr(cpuIdx) + T_TC_ABORT);
    writeLeNumberToBeBytes<uint32_t>(b, bits & mask);
    ew.vb.insert( ew.vb.end(), b, b + sizeof(b));
    ew.vcs += leadingOne(1);
    createCmdModInfo(ew, cpuIdx, 0, OP_TYPE_CMD_ABORT);
    return ew;
  }

  //hard abort everything, emergency only
  void CarpeDM::CarpeDMimpl::halt() {
    if (verbose) sLog << "Aborting all activity" << std::endl;
    vEbwrs ew;
    uint8_t b[4];
    writeLeNumberToBeBytes<uint32_t>(b, (uint32_t)((1ll << ebd.getThrQty())-1) );


    for(uint8_t cpuIdx=0; cpuIdx < ebd.getCpuQty(); cpuIdx++) {
      setThrStart(ew, cpuIdx, 0);
      ew.va.push_back(getThrCmdAdr(cpuIdx) + T_TC_ABORT);
      ew.vb.insert( ew.vb.end(), b, b + sizeof(b));
      ew.vcs.push_back(true); // each one is a new wb device, so we always need a new eb cycle
      createCmdModInfo(ew, cpuIdx, 0, OP_TYPE_CMD_HALT);
    }

    ebd.writeCycle(ew.va, ew.vb, ew.vcs);
  }


  bool CarpeDM::CarpeDMimpl::isThrRunning(uint8_t cpuIdx, uint8_t thrIdx) {
    return (bool)(getThrRun(cpuIdx) & (1<< thrIdx));
  }

  //Requests Thread to start
  vEbwrs& CarpeDM::CarpeDMimpl::startThr(vEbwrs& ew, uint8_t cpuIdx, uint8_t thrIdx) {
    return setThrStart(ew, cpuIdx, (1<<thrIdx));
  }

  //Immediately aborts a Thread
  vEbwrs& CarpeDM::CarpeDMimpl::abortThr(vEbwrs& ew, uint8_t cpuIdx, uint8_t thrIdx) {
    return setThrAbort(ew, cpuIdx, (1<<thrIdx));
  }


vEbwrs& CarpeDM::CarpeDMimpl::resetThrMsgCnt(vEbwrs& ew, uint8_t cpuIdx, uint8_t thrIdx) {
  uint32_t msgCntAdr = atDown.getMemories()[cpuIdx].extBaseAdr + atDown.getMemories()[cpuIdx].sharedOffs + ebd.getCtlAdr(ADRLUT_SHCTL_THR_DAT) + thrIdx * _T_TD_SIZE_ + T_TD_MSG_CNT;

  ew.va.push_back(msgCntAdr + 0);
  ew.va.push_back(msgCntAdr + _32b_SIZE_);
  ew.vb.insert(ew.vb.end(), _64b_SIZE_, 0x00);
  ew.vcs += leadingOne(2);

  return ew;

}


void CarpeDM::CarpeDMimpl::softwareReset(bool clearStatistic) {
  halt();
  clear_raw(true);
  resetAllThreads();
  if (clearStatistic) {
    clearHealth();
    clearHwDiagnostics();
  }
}

void CarpeDM::CarpeDMimpl::resetAllThreads() {
  vEbwrs ew;
  for(uint8_t cpu = 0; cpu < ebd.getCpuQty(); cpu++) { //cycle all CPUs
    for(uint8_t thr = 0; thr < ebd.getThrQty(); thr++) {
      setThrStartTime(ew, cpu, thr, 0ULL);
      setThrDeadline(ew, cpu, thr, -1ULL);
      setThrOrigin(ew, cpu, thr, DotStr::Node::Special::sIdle);
      setThrCursor(ew, cpu, thr, DotStr::Node::Special::sIdle);
    }
  }
  send(ew);
}


uint64_t CarpeDM::CarpeDMimpl::getThrMsgCnt(uint8_t cpuIdx, uint8_t thrIdx) {
  return ebd.read64b(atDown.getMemories()[cpuIdx].extBaseAdr + atDown.getMemories()[cpuIdx].sharedOffs + ebd.getCtlAdr(ADRLUT_SHCTL_THR_DAT) + thrIdx * _T_TD_SIZE_ + T_TD_MSG_CNT);
}

uint64_t CarpeDM::CarpeDMimpl::getThrDeadline(uint8_t cpuIdx, uint8_t thrIdx) {
  return ebd.read64b(atDown.getMemories()[cpuIdx].extBaseAdr + atDown.getMemories()[cpuIdx].sharedOffs + ebd.getCtlAdr(ADRLUT_SHCTL_THR_DAT) + thrIdx * _T_TD_SIZE_ + T_TD_DEADLINE);
}

vEbwrs&  CarpeDM::CarpeDMimpl::setThrDeadline(vEbwrs& ew, uint8_t cpuIdx, uint8_t thrIdx, uint64_t t) {
  uint32_t startAdr = atDown.getMemories()[cpuIdx].extBaseAdr + atDown.getMemories()[cpuIdx].sharedOffs + ebd.getCtlAdr(ADRLUT_SHCTL_THR_DAT) + thrIdx * _T_TD_SIZE_ + T_TD_DEADLINE;
  ew.va += {startAdr, startAdr + _32b_SIZE_};
  writeLeNumberToBeBytes<uint64_t>(ew.vb, t );
  ew.vcs += leadingOne(2);
  return ew;
}

vEbwrs&  CarpeDM::CarpeDMimpl::setThrStartTime(vEbwrs& ew, uint8_t cpuIdx, uint8_t thrIdx, uint64_t t) {
  uint32_t startAdr = atDown.getMemories()[cpuIdx].extBaseAdr + atDown.getMemories()[cpuIdx].sharedOffs + ebd.getCtlAdr(ADRLUT_SHCTL_THR_STA) + thrIdx * _T_TS_SIZE_ + T_TS_STARTTIME;
  ew.va += {startAdr, startAdr + _32b_SIZE_};
  writeLeNumberToBeBytes<uint64_t>(ew.vb, t );
  ew.vcs += leadingOne(2);
  return ew;
}

uint64_t CarpeDM::CarpeDMimpl::getThrStartTime(uint8_t cpuIdx, uint8_t thrIdx) {
  return ebd.read64b(atDown.getMemories()[cpuIdx].extBaseAdr + atDown.getMemories()[cpuIdx].sharedOffs + ebd.getCtlAdr(ADRLUT_SHCTL_THR_STA) + thrIdx * _T_TS_SIZE_ + T_TS_STARTTIME);
}

//FIXME wtf ... this doesnt queue anything!
vEbwrs&  CarpeDM::CarpeDMimpl::setThrPrepTime(vEbwrs& ew, uint8_t cpuIdx, uint8_t thrIdx, uint64_t t) {
  uint32_t startAdr = atDown.getMemories()[cpuIdx].extBaseAdr + atDown.getMemories()[cpuIdx].sharedOffs + ebd.getCtlAdr(ADRLUT_SHCTL_THR_STA) + thrIdx * _T_TS_SIZE_ + T_TS_PREPTIME;
  ew.va += {startAdr, startAdr + _32b_SIZE_};
  writeLeNumberToBeBytes<uint64_t>(ew.vb, t );
  ew.vcs += leadingOne(2);
  return ew;
}

uint64_t CarpeDM::CarpeDMimpl::getThrPrepTime(uint8_t cpuIdx, uint8_t thrIdx) {
  return ebd.read64b(atDown.getMemories()[cpuIdx].extBaseAdr + atDown.getMemories()[cpuIdx].sharedOffs + ebd.getCtlAdr(ADRLUT_SHCTL_THR_STA) + thrIdx * _T_TS_SIZE_ + T_TS_PREPTIME);
}

const vAdr CarpeDM::CarpeDMimpl::getCmdWrAdrs(uint32_t hash, uint8_t prio) {
  vAdr ret;

  //find the address corresponding to given name
  auto it = atDown.lookupHash(hash, carpeDMcommand::exIntro);
  auto* x = (AllocMeta*)&(*it);

  //Check if requested queue priority level exists
  uint32_t blAdr = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&x->b[BLOCK_CMDQ_PTRS + prio * _PTR_SIZE_]);
  //sLog << "Block BListAdr 0x" << std::hex << blAdr << std::endl;
  if(blAdr == LM32_NULL_PTR) {
    throw std::runtime_error( "Block node does not have requested queue of prio " + std::to_string((int)prio));
  }

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


  const uint32_t CarpeDM::CarpeDMimpl::getCmdInc(uint32_t hash, uint8_t prio) {
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


std::pair<int, int> CarpeDM::CarpeDMimpl::findRunningPattern(const std::string& sPattern) {
  std::pair<int, int> res = {-1, -1};
  vStrC members   = getPatternMembers (sPattern);
  try { res.first = getNodeCpu(firstString(members), TransferDir::DOWNLOAD); } catch (...) {res.first = -1; return res;}

  uint32_t thrds  = getThrRun(res.first);

  vStrC cursors;

  for (int i = 0; i < ebd.getThrQty(); i++ ) { cursors.push_back(getThrCursor(res.first, i)); }

  for (int i = 0; i < ebd.getThrQty(); i++ ) {
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

bool CarpeDM::CarpeDMimpl::isPatternRunning(const std::string& sPattern) {
  auto cpuAndThr = findRunningPattern(sPattern);
  return ((cpuAndThr.first >= 0) && (cpuAndThr.second >= 0));
}

//Returns the the index of the first idle thread at cpu <cpuIdx>
int CarpeDM::CarpeDMimpl::getIdleThread(uint8_t cpuIdx) {
  uint32_t thrds = getThrRun(cpuIdx);
  int i;
  for (i = 0; i < ebd.getThrQty(); i++) { if(!(bool)(thrds & (1<<i))) return i; } // aborts at free thrIdx or returns ebd.getThrQty() if no frees found
  return ebd.getThrQty();
}

//Requests Pattern to start on thread <x>
vEbwrs& CarpeDM::CarpeDMimpl::startPattern(vEbwrs& ew, const std::string& sPattern, uint8_t thrIdx, uint64_t t) { return startNodeOrigin(ew, getPatternEntryNode(sPattern), thrIdx, t);}
/*
//Requests Pattern to start
vEbwrs& CarpeDM::CarpeDMimpl::startPattern(vEbwrs& ew, const std::string& sPattern, uint64_t t) { return startNodeOrigin(ew, getPatternEntryNode(sPattern), t); }
*/
//Requests Pattern to stop
vEbwrs& CarpeDM::CarpeDMimpl::stopPattern(vEbwrs& ew, const std::string& sPattern) { return stopNodeOrigin(ew, getPatternExitNode(sPattern)); }

//Immediately aborts a Pattern
vEbwrs& CarpeDM::CarpeDMimpl::abortPattern(vEbwrs& ew, const std::string& sPattern) {
  std::pair<int, int> cpuAndThr = findRunningPattern(sPattern);
  //if we didn't find it, it's not running now. So no problem that we cannot abort it
  if ((cpuAndThr.first >= 0) && (cpuAndThr.second >= 0)) return abortThr(ew, (uint8_t)cpuAndThr.first, (uint8_t)cpuAndThr.second);
  return ew;
}


//Requests thread <thrIdx> to start at node <sNode>
vEbwrs& CarpeDM::CarpeDMimpl::startNodeOrigin(vEbwrs& ew, const std::string& sNode, uint8_t thrIdx, uint64_t t) {
  printf("StartNodeOrigin: Thr%u\n", thrIdx);
  uint8_t cpuIdx    = getNodeCpu(sNode, TransferDir::DOWNLOAD);
  setThrOrigin(ew, cpuIdx, thrIdx, sNode); //configure thread and run it
  setThrStartTime(ew, cpuIdx, thrIdx, t);
  startThr(ew, cpuIdx, thrIdx);
  //sLog << "Started thread at cpuidx " << std::dec << cpuIdx << " thrIdx " << thrIdx << " @ 0x" << std::hex << cmdTvalid << std::endl;
  return ew;
}
/*
//Requests a start at node <sNode>
vEbwrs& CarpeDM::CarpeDMimpl::startNodeOrigin(vEbwrs& ew, const std::string& sNode, uint64_t t) {
  uint8_t cpuIdx    = getNodeCpu(sNode, TransferDir::DOWNLOAD);
  int thrIdx = 0; //getIdleThread(cpuIdx); //find a free thread we can use to run our pattern
  if (thrIdx == ebd.getThrQty()) throw std::runtime_error( "Found no free thread on " + std::to_string(cpuIdx) + "'s hosting cpu");
  setThrOrigin(ew, cpuIdx, thrIdx, sNode); //configure thread and run it
  setThrStartTime(ew, cpuIdx, thrIdx, t);
  startThr(ew, cpuIdx, (uint8_t)thrIdx);
  return ew;
}
*/
//Requests stop at node <sNode> (flow to idle)
vEbwrs& CarpeDM::CarpeDMimpl::stopNodeOrigin(vEbwrs& ew, const std::string& sNode) {
  //send a command: tell patternExitNode to change the flow to Idle
  return createFlowCommand(ew, dnt::sCmdFlow, sNode, DotStr::Node::Special::sIdle, PRIO_LO, 1, true, 0, false);
}

//Immediately aborts the thread whose pattern <sNode> belongs to
vEbwrs& CarpeDM::CarpeDMimpl::abortNodeOrigin(vEbwrs& ew, const std::string& sNode) {
  std::string sPattern = getNodePattern(sNode);
  std::pair<int, int> cpuAndThr = findRunningPattern(sPattern);
  //if we didn't find it, it's not running now. So no problem that we cannot abort it
  if ((cpuAndThr.first >= 0) && (cpuAndThr.second >= 0)) return abortThr(ew, (uint8_t)cpuAndThr.first, (uint8_t)cpuAndThr.second);
  return ew;
}


  const std::string CarpeDM::CarpeDMimpl::getNodePattern (const std::string& sNode)          {return firstString(gt.getGroups<Groups::Node, &GroupMeta::pattern>(sNode));}
  const std::string CarpeDM::CarpeDMimpl::getNodeBeamproc(const std::string& sNode)          {return firstString(gt.getGroups<Groups::Node, &GroupMeta::beamproc>(sNode));}
              vStrC CarpeDM::CarpeDMimpl::getPatternMembers (const std::string& sPattern)    {return gt.getMembers<Groups::Pattern>(sPattern);}
  const std::string CarpeDM::CarpeDMimpl::getPatternEntryNode(const std::string& sPattern)   {return firstString(gt.getPatternEntryNodes(sPattern));}
  const std::string CarpeDM::CarpeDMimpl::getPatternExitNode(const std::string& sPattern)    {return firstString(gt.getPatternExitNodes(sPattern));}
              vStrC CarpeDM::CarpeDMimpl::getBeamprocMembers(const std::string& sBeamproc)   {return gt.getMembers<Groups::Pattern>(sBeamproc);}
  const std::string CarpeDM::CarpeDMimpl::getBeamprocEntryNode(const std::string& sBeamproc) {return firstString(gt.getBeamprocEntryNodes(sBeamproc));}
  const std::string CarpeDM::CarpeDMimpl::getBeamprocExitNode(const std::string& sBeamproc)  {return firstString(gt.getBeamprocExitNodes(sBeamproc));}



  vStrC CarpeDM::CarpeDMimpl::getGraphPatterns(Graph& g)  {
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




vertex_set_t CarpeDM::CarpeDMimpl::getAllCursors(bool activeOnly) {
  vertex_set_t ret;

  //TODO - this is dirty and cumbersome, make it streamlined


  for(uint8_t cpu = 0; cpu < ebd.getCpuQty(); cpu++) { //cycle all CPUs
    for(uint8_t thr = 0; thr < ebd.getThrQty(); thr++) {
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


int CarpeDM::CarpeDMimpl::staticFlushPattern(const std::string& sPattern, bool prioIl, bool prioHi, bool prioLo, bool force) {
  Graph& g = gDown;
  AllocTable& at = atDown;
  vEbwrs ew;

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

int CarpeDM::CarpeDMimpl::staticFlushBlock(const std::string& sBlock, bool prioIl, bool prioHi, bool prioLo, bool force) {
  vEbwrs ew;
  send(staticFlush(sBlock, prioIl, prioHi, prioLo, ew, force));
  return ew.va.size();
}


vEbwrs& CarpeDM::CarpeDMimpl::staticFlush(const std::string& sBlock, bool prioIl, bool prioHi, bool prioLo, vEbwrs& ew, bool force) {
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

vEbwrs& CarpeDM::CarpeDMimpl::deactivateOrphanedCommands(vEbwrs& ew, std::vector<QueueReport>& vQr) {
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
