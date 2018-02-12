#include <boost/shared_ptr.hpp>
#include <algorithm>  
#include <stdio.h>
#include <iostream>
#include <string>
#include <inttypes.h>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/copy.hpp>
#include <boost/algorithm/string.hpp>

#include "common.h"
#include "ftm_shared_mmap.h"
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


  if ( (cpu < 0) || (cpu >= getCpuQty()) || (thr < 0) || (thr >= _THR_QTY_  ) ) { 
    //sLog << "OOR, returning none" << std::endl; 
    return boost::optional<std::pair<int, int>>(); 
  }


  //sLog << "Valid, returning " << res.first << " " << res.second << std::endl;
  return res;
}


/*
throw std::runtime_error("Node '" + g[v].name + "'s has non numeric value properties '" + DotStr::Node::Prop::Base::sCpu + " or " + DotStr::Node::Prop::Base::sThread + "\n");
throw std::runtime_error("Node '" + g[v].name + "'s value for property '" + DotStr::Node::Prop::Base::sCpu + "' (" + std::to_string(cpu) + " is out of range (0-" + std::to_string(getCpuQty()-1) + "\n");
throw std::runtime_error("Node '" + g[v].name + "'s value for property '" + DotStr::Node::Prop::Base::sThread + "' (" + std::to_string(cpu) + " is out of range (0-" + std::to_string(_THR_QTY_-1) + "\n");
*/

vEbwrs& CarpeDM::createCommandBurst(Graph& g, vEbwrs& ew) {

  
  mc_ptr mc;

  if ((boost::get_property(g, boost::graph_name)).find(DotStr::Graph::Special::sCmd) == std::string::npos) {throw std::runtime_error("Expected a series of commands, but this appears to be a schedule (Tag '" + DotStr::Graph::Special::sCmd + "' not found in graphname)");}
  
  //FIXME This is bad, global commands (start/abort, currently also stop) are always sent first ! Queue them normally!!!!
 

  BOOST_FOREACH( vertex_t v, vertices(g) ) {

    std::string target, destination;
    
    

    //use the pattern and beamprocess tags to determine the target. Pattern overrides Beamprocess overrides cpu/thread
          if (g[v].patName  != DotStr::Misc::sUndefined)    { target = getPatternExitNode(g[v].patName); }
    else  if (g[v].bpName != DotStr::Misc::sUndefined)      { target = getBeamprocExitNode(g[v].bpName); }
    else  {target = g[v].cmdTarget;}

  

    //use the destPattern and destBeamprocess tags to determine the destination
    if (g[v].cmdDestPat  != DotStr::Misc::sUndefined)      { destination = getPatternEntryNode(g[v].cmdDestPat); }
    else  if (g[v].cmdDestBp != DotStr::Misc::sUndefined)  { destination = getBeamprocEntryNode(g[v].cmdDestBp); }
    else                                                    {destination = g[v].cmdDest;}
     

    uint64_t cmdTvalid  = s2u<uint64_t>(g[v].tValid);
    uint8_t  cmdPrio    = s2u<uint8_t>(g[v].prio);
    uint8_t cpu, thr;

   sLog << "Command <" << g[v].name << ">, type <" << g[v].type << "> pat <" << g[v].patName << "> destPat <" << g[v].cmdDestPat << "> target <" << g[v].cmdTarget << "> dest <" << g[v].cmdDest << ">" << std::endl;
    

    // Commands with optional target

    if (g[v].type == dnt::sCmdStart)   {
      if(parseCpuAndThr(v, g)) {
        std::tie(cpu, thr) = parseCpuAndThr(v, g).get();
        sLog << " Starting cpu=" << (int)cpu << ", thr=" << (int)thr << std::endl;  startThr(cpu, thr, ew); 
      } else {
        target = getPatternEntryNode(g[v].patName); 
        if (hm.lookup(target)) {sLog << " Starting at <" << target << ">" << std::endl; startNodeOrigin(target, ew);  }
        else throw std::runtime_error("Cannot execute command '" + g[v].type + "' No valid cpu/thr provided and '" + target + "' is not a valid node name\n"); 
      }
      continue;
    }
    if (g[v].type == dnt::sCmdStop)    {
      if(parseCpuAndThr(v, g)) {
        std::tie(cpu, thr) = parseCpuAndThr(v, g).get();
        sLog << " Stopping (trying) cpu=" << (int)cpu << ", thr=" << (int)thr << std::endl;  stopPattern(getNodePattern(getThrCursor(cpu, thr)), ew); 
      } else {
        if (hm.lookup(target)) { sLog << " Stopping at <" << target << ">" << std::endl; stopNodeOrigin(target, ew); }
        else throw std::runtime_error("Cannot execute command '" + g[v].type + "' No valid cpu/thr provided and '" + target + "' is not a valid node name\n");  
      }
      continue;
    }  
    else if (g[v].type == dnt::sCmdAbort)   {
      if(parseCpuAndThr(v, g)) {
        std::tie(cpu, thr) = parseCpuAndThr(v, g).get();
        sLog << " Aborting cpu=" << (int)cpu << ", thr=" << (int)thr << std::endl; abortThr(cpu, thr, ew); 
      } else {
        if (hm.lookup(target)) {sLog << " Aborting (trying) at <" << target << ">" << std::endl; abortNodeOrigin(target, ew); }
        else throw std::runtime_error("Cannot execute command '" + g[v].type + "'. No valid cpu/thr provided and '" + target + "' is not a valid node name\n"); 
      }
      continue;  
    }

 
    if (g[v].type == dnt::sCmdOrigin)   { 
      //Leave out for now and autocorrect cpu
      //if (getNodeCpu(target, DOWNLOAD) != cpu) throw std::runtime_error("Command '" + g[v].name + "'s value for property '" + DotStr::Node::Prop::Base::sCpu + "' is invalid\n");try { adr = getNodeAdr(destination, TransferDir::DOWNLOAD, AdrType::INT); } catch (std::runtime_error const& err) {
      try { setThrOrigin(getNodeCpu(target, TransferDir::DOWNLOAD), thr, target, ew); } catch (std::runtime_error const& err) {
        throw std::runtime_error("Cannot execute command '" + g[v].type + "', " + std::string(err.what())); 
      } 
      continue;
    }

    // Commands targeted at cmd queue of individual blocks, using miniCommand (mc) class
         if (g[v].type == dnt::sCmdNoop)    { uint32_t cmdQty = s2u<uint32_t>(g[v].qty);
                                              mc = (mc_ptr) new MiniNoop(cmdTvalid, cmdPrio, cmdQty );
                                            }
    else if (g[v].type == dnt::sCmdFlow)    { uint32_t cmdQty = s2u<uint32_t>(g[v].qty);
                                              sLog << " Flowing from <" << target << "> to <" << destination << ">, permanent defDest change='" << s2u<bool>(g[v].perma) << "'" << std::endl;
                                              uint32_t adr = LM32_NULL_PTR;
                                              try { adr = getNodeAdr(destination, TransferDir::DOWNLOAD, AdrType::INT); } catch (std::runtime_error const& err) {
                                                throw std::runtime_error("Destination '" + destination + "'' invalid: " + std::string(err.what()));
                                              }

                                              mc = (mc_ptr) new MiniFlow(cmdTvalid, cmdPrio, cmdQty, adr, s2u<bool>(g[v].perma) );
                                            }
    else if (g[v].type == dnt::sCmdFlush)   { mc = (mc_ptr) new MiniFlush(cmdTvalid, cmdPrio, s2u<bool>(g[v].qIl), s2u<bool>(g[v].qHi), s2u<bool>(g[v].qLo));}
    else if (g[v].type == dnt::sCmdWait)    { uint64_t cmdTwait  = s2u<uint64_t>(g[v].tWait);
                                              mc = (mc_ptr) new MiniWait(cmdTvalid, cmdPrio, cmdTwait, false, false );
                                            }
    else                                    { throw std::runtime_error("Command <" + g[v].name + ">'s type <" + g[v].type + "> is not supported!\n");} 
    
    sLog << std::endl;
    //send miniCommand
    createCommand(target, cmdPrio, mc, ew);

  }

  

  return ew;

}  
  
  int CarpeDM::send(vEbwrs& ew) {
    ebWriteCycle(ebd, ew.va, ew.vb);
    return ew.vb.size();
  }


  vEbwrs& CarpeDM::createCommand(const std::string& targetName, uint8_t cmdPrio, mc_ptr mc, vEbwrs& ew) {
    

    uint32_t cmdWrInc, hash;
    uint8_t b[_T_CMD_SIZE_ + _32b_SIZE_];
    
    if (!hm.lookup(targetName)) throw std::runtime_error("Command target <" + targetName + "> is not valid\n");
    
 
    hash        = hm.lookup(targetName).get(); 
    ew.va += getCmdWrAdrs(hash, cmdPrio);
    
    cmdWrInc    = getCmdInc(hash, cmdPrio);
    mc->serialise(b);
    writeLeNumberToBeBytes(b + (ptrdiff_t)_T_CMD_SIZE_, cmdWrInc);
    ew.vb.insert( ew.vb.end(), b, b + _T_CMD_SIZE_ + _32b_SIZE_);
    
    
    return ew;
  }


  //Returns the external address of a thread's command register area
  uint32_t CarpeDM::getThrCmdAdr(uint8_t cpuIdx) {
    return cpuDevs.at(cpuIdx).sdb_component.addr_first + SHARED_OFFS + SHCTL_THR_CTL;
  }

  //Returns the external address of a thread's initial node register 
  uint32_t CarpeDM::getThrInitialNodeAdr(uint8_t cpuIdx, uint8_t thrIdx) {
    return cpuDevs.at(cpuIdx).sdb_component.addr_first + SHARED_OFFS + SHCTL_THR_STA + thrIdx * _T_TS_SIZE_ + T_TS_NODE_PTR;
  }

  //Returns the external address of a thread's cursor pointer
  uint32_t CarpeDM::getThrCurrentNodeAdr(uint8_t cpuIdx, uint8_t thrIdx) {
    return cpuDevs.at(cpuIdx).sdb_component.addr_first + SHARED_OFFS + SHCTL_THR_DAT + thrIdx * _T_TD_SIZE_ + T_TD_NODE_PTR;
  }
  

  //Sets the Node the Thread will start from
  vEbwrs& CarpeDM::setThrOrigin(uint8_t cpuIdx, uint8_t thrIdx, const std::string& name, vEbwrs& ew) {
    uint8_t b[4];

    ew.va.push_back(getThrInitialNodeAdr(cpuIdx, thrIdx));
    writeLeNumberToBeBytes<uint32_t>(b, getNodeAdr(name, TransferDir::DOWNLOAD, AdrType::INT));
    ew.vb.insert( ew.vb.end(), b, b + sizeof(b));
    return ew;
  }

  //Returns the Node the Thread will start from
  const std::string CarpeDM::getThrOrigin(uint8_t cpuIdx, uint8_t thrIdx) {
     uint32_t adr;
     
     adr = ebReadWord(ebd, getThrInitialNodeAdr(cpuIdx, thrIdx));

     if (adr == LM32_NULL_PTR) return DotStr::Node::Special::sIdle;

     auto x = atDown.lookupAdr(cpuIdx, atDown.adrConv(AdrType::INT, AdrType::MGMT,cpuIdx, adr));
     if (atDown.isOk(x))  return gDown[x->v].name;
     else                 return DotStr::Misc::sUndefined;
  }

 

  const std::string CarpeDM::getThrCursor(uint8_t cpuIdx, uint8_t thrIdx) {
    uint32_t adr;
    
    adr = ebReadWord(ebd, getThrCurrentNodeAdr(cpuIdx, thrIdx));

    if (adr == LM32_NULL_PTR) return DotStr::Node::Special::sIdle;

    auto x = atDown.lookupAdr(cpuIdx, atDown.adrConv(AdrType::INT, AdrType::MGMT,cpuIdx, adr));
    if (atDown.isOk(x)) return gDown[x->v].name;
    else                return DotStr::Misc::sUndefined;  
  }

  //DEBUG ONLY !!! force thread cursor to the value of the corresponding origin
  void CarpeDM::forceThrCursor(uint8_t cpuIdx, uint8_t thrIdx) {
    uint32_t cursor = ebReadWord(ebd, getThrInitialNodeAdr(cpuIdx, thrIdx));
    ebWriteWord(ebd, getThrCurrentNodeAdr(cpuIdx, thrIdx), cursor);
  } 

  //Get bitfield showing running threads
  uint32_t CarpeDM::getThrRun(uint8_t cpuIdx) {
    return ebReadWord(ebd, getThrCmdAdr(cpuIdx) + T_TC_RUNNING); 
  }

  //Get bifield showing running threads
  uint32_t CarpeDM::getStatus(uint8_t cpuIdx) {
    return ebReadWord(ebd, cpuDevs.at(cpuIdx).sdb_component.addr_first + SHARED_OFFS + SHCTL_STATUS); 
  }

  void CarpeDM::inspectHeap(uint8_t cpuIdx) {
    vAdr vRa;
    vBuf heap;
    

    uint32_t baseAdr = cpuDevs.at(cpuIdx).sdb_component.addr_first + SHARED_OFFS;
    uint32_t heapAdr = baseAdr + SHCTL_HEAP;
    uint32_t thrAdr  = baseAdr + SHCTL_THR_DAT;

    for(int i=0; i<_THR_QTY_; i++) vRa.push_back(heapAdr + i * _PTR_SIZE_);
    heap = ebReadCycle(ebd, vRa);


    sLog << std::setfill(' ') << std::setw(4) << "Rank  " << std::setfill(' ') << std::setw(5) << "Thread  " << std::setfill(' ') << std::setw(21) 
    << "Deadline  " << std::setfill(' ') << std::setw(21) << "Origin  " << std::setfill(' ') << std::setw(21) << "Cursor" << std::endl;



    for(int i=0; i<_THR_QTY_; i++) {

      uint8_t thrIdx = (writeBeBytesToLeNumber<uint32_t>((uint8_t*)&heap[i * _PTR_SIZE_])  - atDown.adrConv(AdrType::EXT, AdrType::INT,cpuIdx, thrAdr)) / _T_TD_SIZE_;
      sLog << std::dec << std::setfill(' ') << std::setw(4) << i << std::setfill(' ') << std::setw(8) << (int)thrIdx  
      << std::setfill(' ') << std::setw(21) << getThrDeadline(cpuIdx, thrIdx)   << std::setfill(' ') << std::setw(21) 
      << getThrOrigin(cpuIdx, thrIdx)  << std::setfill(' ') << std::setw(21) << getThrCursor(cpuIdx, thrIdx) << std::endl;
    }  
  }

  //Requests Threads to start
  vEbwrs& CarpeDM::setThrStart(uint8_t cpuIdx, uint32_t bits, vEbwrs& ew) {
    uint8_t b[4];

    ew.va.push_back(getThrCmdAdr(cpuIdx) + T_TC_START);
    writeLeNumberToBeBytes<uint32_t>(b, bits);
    ew.vb.insert( ew.vb.end(), b, b + sizeof(b));
    return ew;
  }


  //Requests Threads to stop
  vEbwrs& CarpeDM::setThrAbort(uint8_t cpuIdx, uint32_t bits, vEbwrs& ew) {
    uint8_t b[4];

    ew.va.push_back(getThrCmdAdr(cpuIdx) + T_TC_ABORT);
    writeLeNumberToBeBytes<uint32_t>(b, bits);
    ew.vb.insert( ew.vb.end(), b, b + sizeof(b));
    return ew;
  }
/*
  //hard abort, emergency only
  void CarpeDM::clrThrRun(uint8_t cpuIdx, uint32_t bits) {
    uint32_t state = ebReadWord(ebd, getThrCmdAdr(cpuIdx) + T_TC_RUNNING);
    ebWriteWord(ebd, getThrCmdAdr(cpuIdx) + T_TC_RUNNING, state & ~bits);
  }
*/
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

  void CarpeDM::dumpQueue(uint8_t cpuIdx, const std::string& blockName, uint8_t cmdPrio) {
    
    Graph& g    = gUp;

    uint64_t vTime, wTime;     
    uint32_t type, qty, prio, flPrio, flMode, act, dest;// flRngHiLo, flRngIl;
    bool abs, perm, found;
 
    const std::string sPrio[] = {"      Low", "     High", "Interlock"};
    const std::string sType[] = {"Unknown", "   Noop", "   Flow", "  Flush", "   Wait"};
    boost::optional<std::string> name; 
    
    //FIXME the safeguards for the maps are total crap. Include some decent checks, not everything is worth an exception!!!
    auto block = atDown.lookupHash(hm.lookup(blockName).get());
    sLog << std::endl;

    sLog << "     IlHiLo" << std::endl;
    sLog << "WR 0x" << std::setfill('0') << std::setw(6) << std::hex << writeBeBytesToLeNumber<uint32_t>((uint8_t*)&block->b[BLOCK_CMDQ_WR_IDXS]) << std::endl;
    sLog << "RD 0x" << std::setfill('0') << std::setw(6) << std::hex << writeBeBytesToLeNumber<uint32_t>((uint8_t*)&block->b[BLOCK_CMDQ_RD_IDXS]) << std::endl;

    Graph::out_edge_iterator out_begin, out_end, out_cur;
    boost::tie(out_begin, out_end) = out_edges(block->v,g);
    
    //Get Buffer List of requested priority
    for (out_cur = out_begin; out_cur != out_end; ++out_cur) { if (g[target(*out_cur,g)].np->isMeta() && g[*out_cur].type == det::sQPrio[cmdPrio]) {found = true; break;} }
    if (!(found)) {throw std::runtime_error("Block " + blockName + " does not have a " + det::sQPrio[cmdPrio] + " queue"); return;}            
    auto bufList = atDown.lookupVertex(target(*out_cur,g));    
    if (!(atDown.isOk(bufList))) {return;}
    

    boost::tie(out_begin, out_end) = out_edges(bufList->v,g);
    
    // Iterate Buffers
    for (out_cur = out_begin; out_cur != out_end; ++out_cur) {
      

      sLog << std::endl;

      hexDump(g[target(*out_cur,g)].name.c_str(), (const char*)g[target(*out_cur,g)].np->getB(), _MEM_BLOCK_SIZE);

      //output commands
      for(int i=0; i< _MEM_BLOCK_SIZE / _T_CMD_SIZE_; i ++ ) {
        uint8_t* b = g[target(*out_cur,g)].np->getB();

        vTime = writeBeBytesToLeNumber<uint64_t>((uint8_t*)&b[i * _T_CMD_SIZE_ + T_CMD_TIME]);
        act   = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&b[i * _T_CMD_SIZE_ + T_CMD_ACT]);
        type = ( ((act >> ACT_TYPE_POS)  & ACT_TYPE_MSK) < _ACT_TYPE_END_ ? ((act >> ACT_TYPE_POS)  & ACT_TYPE_MSK) : ACT_TYPE_UNKNOWN);
        prio = ( ((act >> ACT_PRIO_POS)  & ACT_PRIO_MSK) < 3 ? ((act >> ACT_PRIO_POS)  & ACT_PRIO_MSK) : PRIO_LO);
        qty  = (act >> ACT_QTY_POS) & ACT_QTY_MSK;
        perm = (act >> ACT_CHP_POS) & ACT_CHP_MSK;
        //type specific
        abs = (act >> ACT_WAIT_ABS_POS) & ACT_WAIT_ABS_MSK;
        flPrio = (act >> ACT_FLUSH_PRIO_POS) & ACT_FLUSH_PRIO_MSK;
        flMode = (act >> ACT_FLUSH_MODE_POS) & ACT_FLUSH_MODE_MSK;
        wTime  = writeBeBytesToLeNumber<uint64_t>((uint8_t*)&b[i * _T_CMD_SIZE_ + T_CMD_WAIT_TIME]);
        dest   = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&b[i * _T_CMD_SIZE_ + T_CMD_FLOW_DEST]);
        
        //write output
        sLog << std::endl << "Cmd #" << i << ": " << std::endl;
        if(type == ACT_TYPE_UNKNOWN) {sLog << "Unknown Format / not initialised" << std::endl; continue;}
        if(type == ACT_TYPE_NOOP || type == ACT_TYPE_FLOW) sLog << std::dec << qty << " x ";
        else                                               sLog << "1 x ";
        sLog << sType[type] << " @ > " << vTime << " ns, " << sPrio[prio] << " priority";
        if (((type == ACT_TYPE_FLOW) || ((type == ACT_TYPE_WAIT) && !(abs))) && perm) sLog << ", changes are permanent" << std::endl;
        else sLog << ", changes are temporary" << std::endl;

        //type specific
        switch(type) {
          case ACT_TYPE_NOOP  : break;
          case ACT_TYPE_FLOW  : sLog << "Destination: ";
                                try { 
                                  auto y = atDown.lookupAdr(cpuIdx, atDown.adrConv(AdrType::INT, AdrType::MGMT,cpuIdx, dest));
                                  if(atDown.isOk(y)) name = hm.lookup(y->hash);
                                  else name = "INVALID"; 
                                } catch (...) {throw; name = "INVALID";}
                                sLog << name.get()  << std::endl; break;
          case ACT_TYPE_FLUSH : sLog << "Priority to Flush: " << flPrio << " Mode: " << flMode << std::endl; break;
          case ACT_TYPE_WAIT  : if (abs) {sLog << "Wait until " << wTime << std::endl;} else {sLog << "Make Block Period " << wTime << std::endl;} break;

        }

      }
    }
    sLog << std::endl;
  }      

void CarpeDM::dumpNode(uint8_t cpuIdx, const std::string& name) {
  
  Graph& g = gDown;
 
    auto it = atDown.lookupHash(hm.lookup(name).get());
    if (atDown.isOk(it)) {
      auto* x = (AllocMeta*)&(*it);  
      hexDump(g[x->v].name.c_str(), (const char*)x->b, _MEM_BLOCK_SIZE); 
    }
}


uint64_t CarpeDM::getThrMsgCnt(uint8_t cpuIdx, uint8_t thrIdx) {
  return read64b(cpuDevs.at(cpuIdx).sdb_component.addr_first + SHARED_OFFS + SHCTL_THR_DAT + thrIdx * _T_TD_SIZE_ + T_TD_MSG_CNT);   
} 

uint64_t CarpeDM::getThrDeadline(uint8_t cpuIdx, uint8_t thrIdx) {
  return read64b(cpuDevs.at(cpuIdx).sdb_component.addr_first + SHARED_OFFS + SHCTL_THR_DAT + thrIdx * _T_TD_SIZE_ + T_TD_DEADLINE);
}

vEbwrs&  CarpeDM::setThrStartTime(uint8_t cpuIdx, uint8_t thrIdx, uint64_t t, vEbwrs& ew) {
  write64b(cpuDevs.at(cpuIdx).sdb_component.addr_first + SHARED_OFFS + SHCTL_THR_STA + thrIdx * _T_TS_SIZE_ + T_TS_STARTTIME, t);
  return ew;
}

uint64_t CarpeDM::getThrStartTime(uint8_t cpuIdx, uint8_t thrIdx) {
  return read64b(cpuDevs.at(cpuIdx).sdb_component.addr_first + SHARED_OFFS + SHCTL_THR_STA + thrIdx * _T_TS_SIZE_ + T_TS_STARTTIME);
}

vEbwrs&  CarpeDM::setThrPrepTime(uint8_t cpuIdx, uint8_t thrIdx, uint64_t t, vEbwrs& ew) {
  write64b(cpuDevs.at(cpuIdx).sdb_component.addr_first + SHARED_OFFS + SHCTL_THR_STA + thrIdx * _T_TS_SIZE_ + T_TS_PREPTIME, t);
  return ew;
}

uint64_t CarpeDM::getThrPrepTime(uint8_t cpuIdx, uint8_t thrIdx) {
  return read64b(cpuDevs.at(cpuIdx).sdb_component.addr_first + SHARED_OFFS + SHCTL_THR_STA + thrIdx * _T_TS_SIZE_ + T_TS_PREPTIME);
}

  const vAdr CarpeDM::getCmdWrAdrs(uint32_t hash, uint8_t prio) {
    vAdr ret;  

    //find the address corresponding to given name
    auto it = atDown.lookupHash(hash);

    if (!(atDown.isOk(it))) {throw std::runtime_error( "Could not find target block in download address table"); return ret;}
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
    if ((wrIdx == rdIdx) && (eWrIdx != eRdIdx)) {throw std::runtime_error( "Block queue is full, can't write. "); return ret; }
    //lookup Buffer List                                                        
    it = atDown.lookupAdr(x->cpu, atDown.adrConv(AdrType::INT, AdrType::MGMT, x->cpu, blAdr));
    if (!(atDown.isOk(it))) {throw std::runtime_error( "Could not find target queue in download address table"); return ret;}
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
    auto it = atDown.lookupHash(hash);

    if (!(atDown.isOk(it))) {throw std::runtime_error( "Could not find target block in download address table"); return 0;}
    auto* x = (AllocMeta*)&(*it);
        //sLog << "indices: 0x" << std::hex << writeBeBytesToLeNumber<uint32_t>((uint8_t*)&x->b[BLOCK_CMDQ_WR_IDXS]) << std::endl;
    //get incremented Write index of requested prio
    eWrIdx = ( writeBeBytesToLeNumber<uint32_t>((uint8_t*)&x->b[BLOCK_CMDQ_WR_IDXS]) >> (prio * 8)) & Q_IDX_MAX_OVF_MSK;
    //assign to index vector
    newIdxs = ( writeBeBytesToLeNumber<uint32_t>((uint8_t*)&x->b[BLOCK_CMDQ_WR_IDXS]) & ~(0xff << prio * 8)) | (((eWrIdx +1)  & Q_IDX_MAX_OVF_MSK) << (prio * 8));
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
  mc_ptr mc = (mc_ptr) new MiniFlow(0, PRIO_LO, 1, getNodeAdr(DotStr::Node::Special::sIdle, TransferDir::DOWNLOAD, AdrType::INT), false );
  //send a command: tell patternExitNode to change the flow to Idle
  return createCommand(sNode, PRIO_LO, mc, ew);

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
    std::set<std::string> sP;
    vStrC ret;

    BOOST_FOREACH( vertex_t v, vertices(g) ) {sP.insert(getNodePattern(g[v].name));}

    for(auto& itP : sP) ret.push_back(itP);

    return ret;  


  }  

  

 
vertex_set_t CarpeDM::getAllCursors(bool activeOnly) {
  vertex_set_t ret;

  //TODO - this is dirty and cumbersome, make it streamlined


  for(uint8_t cpu=0; cpu < getCpuQty(); cpu++) { //cycle all CPUs
    for(uint8_t thr=0; thr < _THR_QTY_; thr++) {
      uint32_t  adr = ebReadWord(ebd, getThrCurrentNodeAdr(cpu, thr));
      uint64_t dl = getThrDeadline(cpu, thr); 
      if (adr == LM32_NULL_PTR || (activeOnly && ((int64_t)dl == -1))) continue; // only active cursors: no dead end idles, no aborted threads
      auto x = atDown.lookupAdr(cpu, atDown.adrConv(AdrType::INT, AdrType::MGMT,cpu, adr));
      if (atDown.isOk(x)) ret.insert(x->v);
    }
      //add all thread cursors addresses of CPU <i>
    
      //create triplicate version to allow majority vote on all values
    
      //pass to ebReadCycle
    
      //majority vote on results
    
      //parse pointers. each pointer successfully translated to a vertex idx goes into the result set

  }

  return ret;

}



HealthReport& CarpeDM::getHealth(uint8_t cpuIdx, HealthReport &hr) {
  uint32_t const baseAdr = cpuDevs.at(cpuIdx).sdb_component.addr_first + SHARED_OFFS;

  vAdr diagAdr;
  vBuf diagBuf;
  uint8_t* b;

  // this is possible because T_DIAG offsets start at 0, see ftm_common.h for definition
  for (uint32_t offs = 0; offs < _T_DIAG_SIZE_; offs += _32b_SIZE_) diagAdr.push_back(baseAdr + SHCTL_DIAG + offs); 
  diagAdr.push_back(baseAdr + SHCTL_STATUS);  
  diagBuf = ebReadCycle(ebd, diagAdr);
  b = (uint8_t*)&diagBuf[0];

  //hexDump("TEST", diagBuf );

  //hexDump("boot", (const char*)(b + T_DIAG_BOOT_TS), 8 );
  //hexDump("smod", (const char*)(b + T_DIAG_SMOD_TS), 8 );

  hr.cpu              = cpuIdx;
  hr.msgCnt           = writeBeBytesToLeNumber<uint64_t>(b + T_DIAG_MSG_CNT); 
  hr.bootTime         = writeBeBytesToLeNumber<uint64_t>(b + T_DIAG_BOOT_TS); 
  //TODO Schedule modfication issuer, hash ...
  //TODO Command time, modfication issuer, hash ...
  //printf("bootnum, 0x%016x \n", hr.bootTime);
  hr.smodTime         = writeBeBytesToLeNumber<uint64_t>(b + T_DIAG_SMOD_TS);
  for (int i = 0; i<8; i++) { //copy and sanitize issuer name
    char c = *(char*)(b + T_DIAG_SMOD_IID + i);
    hr.smodIssuer[i] = (((c > 32) && (c < 126)) ? c : '\00');
  }
  hr.smodIssuer[8] = '\00';  
  hr.minTimeDiff      =  writeBeBytesToLeNumber<int64_t>(b + T_DIAG_DIF_MIN);  
  hr.maxTimeDiff      =  writeBeBytesToLeNumber<int64_t>(b + T_DIAG_DIF_MAX);
  hr.avgTimeDiff      = (hr.msgCnt ? writeBeBytesToLeNumber<int64_t>(b + T_DIAG_DIF_SUM) / (int64_t)hr.msgCnt : 0);   
  hr.warningThreshold =  writeBeBytesToLeNumber<int64_t>(b + T_DIAG_DIF_WTH);
  hr.warningCnt       = writeBeBytesToLeNumber<uint64_t>(b + T_DIAG_WAR_CNT);
  hr.stat             = writeBeBytesToLeNumber<uint32_t>(b + _T_DIAG_SIZE_); // stat comes after last element of T_DIAG
  
  return hr;
  
} 
