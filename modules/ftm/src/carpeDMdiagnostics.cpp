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



namespace coverage {
  std::map<uint8_t, std::string> patName;
  std::map<std::string, vertex_t> patEntry;
  std::map<std::string, vertex_t> patExit;
  const uint64_t cursorPos  = 0;  
  const uint64_t cursorBits = 3;
  const uint64_t cursorMsk  = (1<<cursorBits) -1;
  
  const uint64_t staticPos  = cursorPos + cursorBits;  
  const uint64_t staticBits = 6;
  const uint64_t staticMsk  = (1<<staticBits) -1;
  const uint64_t staticDigitBits = 2;
  const uint64_t staticDigitMsk  = (1<<staticDigitBits) -1;
  
  const uint64_t dynPos  = staticPos + staticBits;  
  const uint64_t dynBits = 18;
  const uint64_t dynMsk  = (1<<dynBits) -1;
  const uint64_t dynDigitBits = 2;
  const uint64_t dynDigitMsk  = (1<<dynDigitBits) -1;

  const uint64_t maxSeed = (1 << (cursorBits + staticBits + dynBits));

  struct config3 {
    uint8_t cursor;
    uint8_t def[3];
    uint8_t dyn[9];
  };

}


  //check if all tables are in sync
  bool CarpeDM::tableCheck(std::string& report) {
    Graph& g        = gDown;
    AllocTable& at  = atDown;
    bool qtyIsOk  = true, allocIsOk = true, hashIsOk = true, groupsIsOk = true, isOk;

    std::string    intro = "*** Table Status:  ",
                qtyIntro = "*** Element Count: ", 
              allocIntro = "*** Alloctable:    ",
             groupsIntro = "*** GroupTable:    ",
             hashIntro   = "*** Hashtable:     ";
    std::string qtyReport, allocReport, groupsReport, hashReport;         
    const std::string sMiss   = "Missing ";
    const std::string sSurp   = "Surplus ";
    const std::string sFirst  = "element: ";
    const std::string sOK     = "OK\n";
    const std::string sERR    = "ERROR\n";

    // check if  graph node count equals ... 
    auto nQty = boost::vertices(g);
    size_t nodeQty  = nQty.second - nQty.first;

    // ... alloctable entry count
    size_t allocEntryQty     = at.getSize();
    // ... groupstable entry count
    size_t groupsEntryQty    = gt.getSize();
    // ... hashtable entry count
    size_t hashEntryQty      = hm.size();

    qtyIsOk &= ((nodeQty == allocEntryQty) & (nodeQty == allocEntryQty) & (nodeQty == groupsEntryQty) & (nodeQty == hashEntryQty));

  
      qtyReport += "Nodes:        " + std::to_string(nodeQty) + "\nAllocEntries: " + std::to_string(allocEntryQty)
                +  "\nHashEntries:  " + std::to_string(hashEntryQty) +  "\nGroupEntries: " + std::to_string(groupsEntryQty) + "\n";
   
    // check if all graph nodes are known to all tables
    BOOST_FOREACH( vertex_t v, vertices(g) ) {
      //Check Hashtable
      if (!hm.contains(g[v].name)) {hashIsOk = false; hashReport += sMiss + sFirst + g[v].name + "\n";}
    }
    BOOST_FOREACH( vertex_t v, vertices(g) ) {
      //Check Alloctable
      try {
        auto dummy = at.lookupVertex(v);
        (void) dummy; struct dummy; // suppress gcc warning about unused variable
      } catch (...) {allocIsOk = false; allocReport += sMiss + sFirst + g[v].name + "\n";}
    }
    BOOST_FOREACH( vertex_t v, vertices(g) ) {
      //Check Groupstable
      auto x  = gt.getTable().get<Groups::Node>().equal_range(g[v].name);
      if (x.first == x.second)   {groupsIsOk = false; groupsReport += sMiss + sFirst + g[v].name + "\n";} 
    } 

    // Let's assume hashmap is okay if all node names are accounted for

    // Let's assume alloctable is okay if all nodes are accounted for

    // check if all groupstable entries are present in graph
    bool notFound;
    for (auto& patternIt : gt.getAllPatterns()) { //NOTE: use the pattern list in GroupTable, not the list in the Graph!
      //std::cout << "Pattern " << patternIt << std::endl;
      for (auto& nodeIt : getPatternMembers(patternIt)) {
        notFound = true; 
        BOOST_FOREACH( vertex_t v, vertices(g) ) {
          if (g[v].name == nodeIt) { notFound = false; break; }
        }
        if (notFound) {
          //std::cout << nodeIt << " was not found " << std::endl; 
          groupsIsOk = false; groupsReport += sSurp + sFirst + nodeIt + "\n"; 
        }
      }
      
    }
    isOk = qtyIsOk & allocIsOk & hashIsOk & groupsIsOk;
    
    intro       += (isOk       ? sOK : sERR);
    qtyIntro    += (qtyIsOk    ? sOK : sERR);
    allocIntro  += (allocIsOk  ? sOK : sERR);
    hashIntro   += (hashIsOk   ? sOK : sERR);
    groupsIntro += (groupsIsOk ? sOK : sERR);

    
    report = intro
           + qtyIntro     + qtyReport     + "\n" 
           + allocIntro   + allocReport   + "\n"
           + hashIntro    + hashReport    + "\n"
           + groupsIntro  + groupsReport  + "\n";
    

    return isOk;      
  }


  std::string& CarpeDM::inspectQueues(const std::string& blockName, std::string& report) {

    QueueReport qr;
    qr = getQReport(blockName, qr);
        
    report += "Inspecting Queues of Block " + blockName + "\n";

    for (int8_t prio = PRIO_IL; prio >= PRIO_LO; prio--) {
      
      report += "Priority " + std::to_string((int)prio) + " (" + dnt::sQPrio[prio] + ") ";
      if (!qr.hasQ[prio]) {report += " Not instantiated\n"; continue;}

      report += " RdIdx: " + std::to_string((int)qr.aQ[prio].rdIdx) 
              + " WrIdx: " + std::to_string((int)qr.aQ[prio].wrIdx) 
              + "    Pending: " + std::to_string((int)qr.aQ[prio].pendingCnt) + "\n";
      
      //find buffers of all non empty slots
      for (uint8_t i, idx = qr.aQ[prio].rdIdx; idx < qr.aQ[prio].rdIdx + 4; idx++) {
        i = idx & Q_IDX_MAX_MSK;
        QueueElement& qe = qr.aQ[prio].aQe[i];

        report += "#" + std::to_string(i) + " ";
        //if it is pending, say so
        report += (qe.pending ? "pending " : "empty   ");

        if (!(verbose | qe.pending)) { report += "-\n"; continue;}

        std::stringstream auxstream;
        auxstream << std::setfill('0') << std::setw(16) << std::hex << qe.validTime;

        report += "Valid Time: 0x" + auxstream.str() + "    CmdType: ";
        //type specific
        std::string const sYes  = "YES";
        std::string const sNo   = "NO ";
        switch(qe.type) {
          case ACT_TYPE_NOOP  : { report += qe.sType + "    Qty: " + std::to_string(qe.qty) + "\n";
                                  break;
                                }
          case ACT_TYPE_FLOW  : { report += qe.sType + "    Permanent: " + (qe.flowPerma ? sYes : sNo) + "    Qty: " + std::to_string(qe.qty) + "    " + blockName + " --> " + qe.flowDst + " \n";
                                  break;
                                }
          case ACT_TYPE_FLUSH : { report += qe.sType + "    Flushing:"  
                                                                 + "   2 (" + dnt::sQPrio[2] + "): " + (qe.flushIl ? sYes : sNo)
                                                                 + "   1 (" + dnt::sQPrio[1] + "): " + (qe.flushHi ? sYes : sNo)
                                                                 + "   0 (" + dnt::sQPrio[0] + "): " + (qe.flushLo ? sYes : sNo) 
                                                                 + "\n"; 
                                  break;
                                }
          case ACT_TYPE_WAIT  : { report += qe.sType + (qe.waitAbs ? " - until " : " - make block period ") + std::to_string(qe.waitTime) + "ns\n";
                                  break;
                                }
          default             : { report += "Unknown Format / Not initialised\n";
                                  break;
                                }
        }  

      }
      
    }

    return report;  
  }


  QueueReport& CarpeDM::getQReport(const std::string& blockName, QueueReport& qr) {
    Graph& g = gDown;
    AllocTable& at = atDown;
    vStrC futureOrphan;
    return getQReport(g, at, blockName, qr, futureOrphan);
  }  

  QueueReport& CarpeDM::getQReport(Graph& g, AllocTable& at, const std::string& blockName, QueueReport& qr, const vStrC& futureOrphan) {
    
    const std::string exIntro = " getQReport: ";
    
    

    auto x = at.lookupHash(hm.lookup(blockName, exIntro)); // x is the blocks alloctable entry
    qr.name = blockName;

    //check their Q counters for unprocessed commands
    uint32_t wrIdxs = boost::dynamic_pointer_cast<Block>(g[x->v].np)->getWrIdxs(); 
    uint32_t rdIdxs = boost::dynamic_pointer_cast<Block>(g[x->v].np)->getRdIdxs();

    if (verbose) sLog << "Check for orphaned commands is scanning Queue @ " << blockName << std::endl;

    for (uint8_t prio = PRIO_LO; prio <= PRIO_IL; prio++) {
      
      uint32_t bufLstAdr;
      uint8_t bufLstCpu;
      AdrType bufLstAdrType; 
      
      //get Block binary
      uint8_t* bBlock = (uint8_t*)&(x->b);
      bufLstAdr = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&bBlock[BLOCK_CMDQ_LO_PTR + prio * _32b_SIZE_]);
      std::tie(bufLstCpu, bufLstAdrType) = at.adrClassification(bufLstAdr);  
      //get BufList binary
      const uint8_t* bBL;
      try {
        auto bufLst = at.lookupAdr( x->cpu, at.adrConv(bufLstAdrType, AdrType::MGMT, x->cpu, bufLstAdr) ); //buffer list cpu is the same as block cpu
        qr.hasQ[prio] = true;
        bBL = bufLst->b;
      } catch (...) { continue; }

      
      //get current read cnt
      uint8_t auxRd = (rdIdxs >> (prio*8)) & Q_IDX_MAX_OVF_MSK;
      uint8_t auxWr = (wrIdxs >> (prio*8)) & Q_IDX_MAX_OVF_MSK;

      int8_t rdIdx = auxRd & Q_IDX_MAX_MSK;
      int8_t wrIdx = auxWr & Q_IDX_MAX_MSK;
      int8_t diff  = (wrIdx >= rdIdx) ? wrIdx - rdIdx : wrIdx - rdIdx + 4;
      bool          full = ((auxRd != auxWr) & (rdIdx == wrIdx));
      uint8_t pendingCnt = full ? 4 : diff;
      
      qr.aQ[prio].wrIdx       = auxWr;
      qr.aQ[prio].rdIdx       = auxRd;
      qr.aQ[prio].pendingCnt  = pendingCnt;

      //set of all pending command indices
      std::set<uint8_t> pendingIdx;
      if (pendingCnt) {for(uint8_t pidx = rdIdx; pidx < (rdIdx + pendingCnt); pidx++) {pendingIdx.insert( pidx & Q_IDX_MAX_MSK);}}

      
      if (verbose) sLog << "Prio " << (int)prio << std::endl;

      //find buffers of all non empty slots
      for (uint8_t i = 0; i <= Q_IDX_MAX_MSK; i++) {
        uint8_t bufCpu;
        AdrType bufAdrType;
        
        uint32_t bufAdr = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&bBL[(i / 2) * _32b_SIZE_] );
        std::tie(bufCpu, bufAdrType) = at.adrClassification(bufAdr);
        try {  
          auto buf = at.lookupAdr( x->cpu, at.adrConv(bufAdrType, AdrType::MGMT, x->cpu, bufAdr) ); //buffer cpu is the same as block cpu
          qr.aQ[prio].aQe[i].pending = (bool)pendingIdx.count(i);
          getQelement(g, at, i, buf, qr.aQ[prio].aQe[i], futureOrphan );
        } catch (...) {continue;}
      }
    }
      
    return qr;
  }

  QueueElement& CarpeDM::getQelement(Graph& g, AllocTable& at, uint8_t idx, amI allocIt, QueueElement& qe, const vStrC& futureOrphan) {
    //TODO might cleaner as deserialisers for MiniCommand Class
    uint8_t*  bAux  = (uint8_t*)&(allocIt->b);
    uint8_t*     b  = (uint8_t*)&bAux[(idx % 2) * _T_CMD_SIZE_];
    uint8_t    cpu  = allocIt->cpu;
    uint32_t   act  = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&b[T_CMD_ACT]);
    uint8_t   type  = (act >> ACT_TYPE_POS) & ACT_TYPE_MSK;
    qe.validTime    = writeBeBytesToLeNumber<uint64_t>((uint8_t*)&b[T_CMD_TIME]);
    qe.qty          = (act >> ACT_QTY_POS) & ACT_QTY_MSK;
    qe.type         = type; //for conevenient use of case statements
    // calculate ext address for direct surgical modification
    qe.extAdr       = at.adrConv(AdrType::MGMT, AdrType::EXT, allocIt->cpu, allocIt->adr) + (idx % 2) * _T_CMD_SIZE_; 

    //type specific
    switch(type) {
      case ACT_TYPE_NOOP  : { qe.sType = dnt::sCmdNoop;
                              break;
                            }
      case ACT_TYPE_FLOW  : {
                              uint32_t dstAdr = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&b[T_CMD_FLOW_DEST]);
                              std::string sDst;
                              
                              if (dstAdr == LM32_NULL_PTR) { sDst = DotStr::Node::Special::sIdle; }// pointing to idle is always okay
                              else {
                                //Find out if Destination is on this core ('cpu') or on a peer core ('dstCpuAux')
                                uint8_t dstCpuAux, dstCpu;
                                AdrType dstAdrType;
                                std::tie(dstCpuAux, dstAdrType) = at.adrClassification(dstAdr);
                                dstCpu = (dstAdrType == AdrType::PEER ? dstCpuAux : cpu);
                                //get allocentry of the destination by its cpu idx and memory address
                                try {
                                  auto dst = at.lookupAdr( dstCpu, at.adrConv(dstAdrType, AdrType::MGMT, dstCpu, dstAdr) );
                                  sDst = g[dst->v].name;
                                  for (auto& itOrphan : futureOrphan) {
                                    if (sDst == itOrphan) {
                                      if (verbose) sLog << "found orphaned command pointing to " << itOrphan << " in slot " << (int)idx << std::endl;
                                      qe.orphaned = true;
                                      break;} 
                                  }
                                } catch (...) {
                                  if (verbose) sLog << "found orphaned command pointing to unknown destination (#" << std::dec << (int)dstCpu << " 0x" << std::hex << dstAdr << std::dec << " in slot " << (int)idx << std::endl; 
                                  sDst = DotStr::Misc::sUndefined;
                                  qe.orphaned = true;
                                }
                                
                              }    
                              qe.sType          = dnt::sCmdFlow;
                              qe.flowPerma      = (act >> ACT_CHP_POS) & ACT_CHP_MSK;
                              qe.flowDst        = sDst;
                              qe.flowDstPattern = getNodePattern(sDst);
                              break;
                            }
      case ACT_TYPE_FLUSH : {
                              uint8_t flPrio = (act >> ACT_FLUSH_PRIO_POS) & ACT_FLUSH_PRIO_MSK;
                              qe.sType   = dnt::sCmdFlush;
                              qe.flushIl = flPrio & (1<<PRIO_IL);
                              qe.flushHi = flPrio & (1<<PRIO_HI);
                              qe.flushLo = flPrio & (1<<PRIO_LO);
                              break;
                            }
      case ACT_TYPE_WAIT  : {
                              qe.sType    = dnt::sCmdWait;
                              qe.waitAbs  = (act >> ACT_WAIT_ABS_POS) & ACT_WAIT_ABS_MSK;
                              qe.waitTime = writeBeBytesToLeNumber<uint64_t>((uint8_t*)&b[T_CMD_WAIT_TIME]);
                              break;
                            }
      default             : {
                              break;
                            }
    }

    return qe;
  }



void CarpeDM::dumpNode(uint8_t cpuIdx, const std::string& name) {
  
  Graph& g = gDown;
  if (hm.contains(name)) {   
    auto it = atDown.lookupHash(hm.lookup(name));
    auto* x = (AllocMeta*)&(*it);  
    hexDump(g[x->v].name.c_str(), (const char*)x->b, _MEM_BLOCK_SIZE); 
  }  
}

void CarpeDM::inspectHeap(uint8_t cpuIdx) {
  vAdr vRa;
  vBuf heap;
  

  uint32_t baseAdr = atDown.getMemories()[cpuIdx].extBaseAdr + atDown.getMemories()[cpuIdx].sharedOffs;
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


void CarpeDM::clearHealth(uint8_t cpuIdx) {
  vEbwrs ew;
  clearHealth(cpuIdx, ew);
  ebWriteCycle(ebd, ew.va, ew.vb, ew.vcs);
}

void CarpeDM::clearHealth() {
  vEbwrs ew;
  for(int cpuIdx = 0; cpuIdx < getCpuQty(); cpuIdx++) { clearHealth(cpuIdx, ew); }
  ebWriteCycle(ebd, ew.va, ew.vb, ew.vcs);
}

vEbwrs& CarpeDM::clearHealth(uint8_t cpuIdx, vEbwrs& ew) {
  uint32_t const baseAdr = atDown.getMemories()[cpuIdx].extBaseAdr + atDown.getMemories()[cpuIdx].sharedOffs;
  
  uint8_t buf[8];
  
  
  

  const vBuf basicState = {0x00, 0x00, 0x00, 0x0f};



  //reset thread message counters
  //printf("VA size before %u, VCS size \n", ew.va.size(), ew.vcs.size());
  for (uint8_t thrIdx = 0; thrIdx < _THR_QTY_; thrIdx++) {

    resetThrMsgCnt(cpuIdx, thrIdx, ew);
    //printf("VA size %u, VCS size \n", ew.va.size(), ew.vcs.size());
  }

  size_t oldContent = ew.va.size();

  // reset diagnostic values aggregate

  ew.va.push_back(baseAdr + SHCTL_DIAG + T_DIAG_MSG_CNT + 0);  // 64b counter
  ew.va.push_back(baseAdr + SHCTL_DIAG + T_DIAG_MSG_CNT + _32b_SIZE_);
  ew.vb.insert(ew.vb.end(), _64b_SIZE_, 0x00);

  // skip boot timestamp, we did not reboot

  // iterate over fields of the aggregate
  for (uint32_t offs = T_DIAG_SCH_MOD; offs < T_DIAG_DIF_MIN; offs += _32b_SIZE_) {
    ew.va.push_back(baseAdr + SHCTL_DIAG + offs);
    //min diff value must be initialised with max, max diff with min
    ew.vb.insert(ew.vb.end(), _32b_SIZE_, 0x00);
  }

  //Min dif
  ew.va.push_back(baseAdr + SHCTL_DIAG + T_DIAG_DIF_MIN + 0);  // 64b counter
  ew.va.push_back(baseAdr + SHCTL_DIAG + T_DIAG_DIF_MIN + _32b_SIZE_);
  writeLeNumberToBeBytes(buf, std::numeric_limits<int64_t>::max());
  ew.vb.insert(ew.vb.end(), buf, buf +_64b_SIZE_);

  //Max dif
  ew.va.push_back(baseAdr + SHCTL_DIAG + T_DIAG_DIF_MAX + 0);  // 64b counter
  ew.va.push_back(baseAdr + SHCTL_DIAG + T_DIAG_DIF_MAX + _32b_SIZE_);
  writeLeNumberToBeBytes(buf, std::numeric_limits<int64_t>::min());
  ew.vb.insert(ew.vb.end(), buf, buf +_64b_SIZE_);

  //Running Sum
  ew.va.push_back(baseAdr + SHCTL_DIAG + T_DIAG_DIF_SUM + 0);  // 64b counter
  ew.va.push_back(baseAdr + SHCTL_DIAG + T_DIAG_DIF_SUM + _32b_SIZE_);
  ew.vb.insert(ew.vb.end(), _64b_SIZE_, 0x00);

  

  //skip Dif Warning Threshold, that stays as it is

  // iterate over fields of the aggregate
  for (uint32_t offs = T_DIAG_WAR_CNT; offs < _T_DIAG_SIZE_; offs += _32b_SIZE_) {
    ew.va.push_back(baseAdr + SHCTL_DIAG + offs);
    ew.vb.insert(ew.vb.end(), _32b_SIZE_, 0x00);
  }

  // clear status value
  ew.va.push_back(baseAdr + SHCTL_STATUS);
  ew.vb += basicState;

  //insert EB flow control vector
  ew.vcs += leadingOne(ew.va.size() - oldContent);

  return ew;

}


HealthReport& CarpeDM::getHealth(uint8_t cpuIdx, HealthReport &hr) {
  uint32_t const baseAdr = atDown.getMemories()[cpuIdx].extBaseAdr + atDown.getMemories()[cpuIdx].sharedOffs;




  vAdr diagAdr;
  vBuf diagBuf;
  uint8_t* b;

  // this is possible because T_DIAG offsets start at 0, see ftm_common.h for definition
  for (uint32_t offs = 0; offs < _T_DIAG_SIZE_; offs += _32b_SIZE_) diagAdr.push_back(baseAdr + SHCTL_DIAG + offs); 
  diagAdr.push_back(baseAdr + SHCTL_STATUS);  
  diagBuf = ebReadCycle(ebd, diagAdr);
  b = (uint8_t*)&diagBuf[0];

  //hexDump("TEST", diagBuf );


  hr.cpu              = cpuIdx;
  hr.msgCnt           = writeBeBytesToLeNumber<uint64_t>(b + T_DIAG_MSG_CNT); 
  hr.bootTime         = writeBeBytesToLeNumber<uint64_t>(b + T_DIAG_BOOT_TS); 


  hr.smodTime         = writeBeBytesToLeNumber<uint64_t>(b + T_DIAG_SCH_MOD + T_MOD_INFO_TS);
  for (int i = 0; i<8; i++) { //copy and sanitize issuer name
    char c = *(char*)(b + T_DIAG_SCH_MOD + T_MOD_INFO_IID + i);
    hr.smodIssuer[i] = (((c > 32) && (c < 126)) ? c : '\00');
  }
  hr.smodIssuer[8] = '\00';
  for (int i = 0; i<8; i++) { //copy and sanitize issuer machine name
    char c = *(char*)(b + T_DIAG_SCH_MOD + T_MOD_INFO_MID + i);
    hr.smodHost[i] = (((c > 32) && (c < 126)) ? c : '\00');
  }
  hr.smodHost[8] = '\00';

  uint8_t schOpType = writeBeBytesToLeNumber<uint32_t>(b + T_DIAG_SCH_MOD + T_MOD_INFO_TYPE);   //there may be more info here later, so don't use byte offsets, just mask (by cast now)
  
  //printf("Schedule Optype 0x%02x @ 0x%08x\n", schOpType, T_DIAG_SCH_MOD + T_MOD_INFO_TYPE);

  switch(schOpType) {
    case OP_TYPE_SCH_CLEAR      : hr.smodOpType = "Clear"; break;
    case OP_TYPE_SCH_ADD        : hr.smodOpType = "Add"; break;
    case OP_TYPE_SCH_OVERWRITE  : hr.smodOpType = "Overwrite"; break;
    case OP_TYPE_SCH_REMOVE     : hr.smodOpType = "Remove"; break;
    case OP_TYPE_SCH_KEEP       : hr.smodOpType = "Keep"; break;
    default                     : hr.smodOpType = "    ?"; break;
  }

   
  hr.smodCnt     =  writeBeBytesToLeNumber<uint32_t>(b + T_DIAG_SCH_MOD + T_MOD_INFO_CNT);


  hr.cmodTime         = writeBeBytesToLeNumber<uint64_t>(b + T_DIAG_CMD_MOD + + T_MOD_INFO_TS);
  for (int i = 0; i<8; i++) { //copy and sanitize issuer name
    char c = *(char*)(b + T_DIAG_CMD_MOD + T_MOD_INFO_IID + i);
    hr.cmodIssuer[i] = (((c > 32) && (c < 126)) ? c : '\00');
  }
  hr.cmodIssuer[8] = '\00';
  for (int i = 0; i<8; i++) { //copy and sanitize issuer machine name
    char c = *(char*)(b + T_DIAG_CMD_MOD + T_MOD_INFO_MID + i);
    hr.cmodHost[i] = (((c > 32) && (c < 126)) ? c : '\00');
  }
  hr.cmodHost[8] = '\00';  
  


  uint8_t cmdOpType = writeBeBytesToLeNumber<uint32_t>(b + T_DIAG_CMD_MOD + T_MOD_INFO_TYPE);   //there may be more info here later, so don't use byte offsets, just mask (by cast now)
  //printf("Cmd Optype %02x @ 0x%08x, Flow would be %02x\n", cmdOpType, T_DIAG_CMD_MOD + T_MOD_INFO_TYPE, OP_TYPE_CMD_FLOW);

  switch(cmdOpType) {
    case OP_TYPE_CMD_FLOW  : hr.cmodOpType = "Flow"; break;
    case OP_TYPE_CMD_NOP   : hr.cmodOpType = "No Op"; break;
    case OP_TYPE_CMD_WAIT  : hr.cmodOpType = "Wait"; break;
    case OP_TYPE_CMD_FLUSH : hr.cmodOpType = "Flush"; break;
    case OP_TYPE_CMD_START : hr.cmodOpType = "Start"; break;
    case OP_TYPE_CMD_STOP  : hr.cmodOpType = "Stop"; break;
    case OP_TYPE_CMD_CEASE : hr.cmodOpType = "Cease"; break;
    case OP_TYPE_CMD_ABORT : hr.cmodOpType = "Abort"; break;
    default                : hr.cmodOpType = "    ?";
  }


  hr.cmodCnt          = (uint8_t)writeBeBytesToLeNumber<uint32_t>(b + T_DIAG_CMD_MOD + T_MOD_INFO_CNT);

  hr.minTimeDiff      =  writeBeBytesToLeNumber<int64_t>(b + T_DIAG_DIF_MIN);  
  hr.maxTimeDiff      =  writeBeBytesToLeNumber<int64_t>(b + T_DIAG_DIF_MAX);
  hr.avgTimeDiff      = (hr.msgCnt ? writeBeBytesToLeNumber<int64_t>(b + T_DIAG_DIF_SUM) / (int64_t)hr.msgCnt : 0);   
  hr.warningThreshold =  writeBeBytesToLeNumber<int64_t>(b + T_DIAG_DIF_WTH);
  hr.warningCnt       = writeBeBytesToLeNumber<uint64_t>(b + T_DIAG_WAR_CNT);
  uint32_t warningHash = writeBeBytesToLeNumber<uint32_t>(b + T_DIAG_WAR_1ST_HASH);
  hr.warningNode      = hm.contains(warningHash) ? hm.lookup(warningHash) : "?";
  hr.warningTime      = writeBeBytesToLeNumber<uint64_t>(b + T_DIAG_WAR_1ST_TS);
  hr.maxBacklog       = writeBeBytesToLeNumber<uint32_t>(b + T_DIAG_BCKLOG_STRK);
  hr.stat             = writeBeBytesToLeNumber<uint32_t>(b + _T_DIAG_SIZE_); // stat comes after last element of T_DIAG
  
  return hr;
  
}

void CarpeDM::show(const std::string& title, const std::string& logDictFile, TransferDir dir, bool filterMeta ) {

  Graph& g        = (dir == TransferDir::UPLOAD ? gUp  : gDown);
  AllocTable& at  = (dir == TransferDir::UPLOAD ? atUp : atDown);

  sLog << std::endl << title << std::endl;
  sLog << std::endl << "Patterns:" << std::endl;

  for (auto& it : gt.getAllPatterns()) sLog <<  it << std::endl;

  sLog << std::endl << std::setfill(' ') << std::setw(4) << "Idx" << "   " << std::setfill(' ') << std::setw(4) << "S/R" << "   " 
                    << std::setfill(' ') << std::setw(4) << "Cpu" << "   " << std::setw(30) << "Name" << "   " 
                    << std::setw(10) << "Hash" << "   " << std::setw(10)  <<  "Int. Adr   "  << "   " << std::setw(10) << "Ext. Adr   " << std::endl;
  //sLog << " " << std::endl; 

  BOOST_FOREACH( vertex_t v, vertices(g) ) {
    auto x = at.lookupVertex(v);
    
    if( !(filterMeta) || (filterMeta & !(g[v].np->isMeta())) ) {
      sLog   << std::setfill(' ') << std::setw(4) << std::dec << v 
      << "   "    << std::setfill(' ') << std::setw(2) << std::dec << (int)(at.isStaged(x))  
      << "   "    << std::setfill(' ') << std::setw(4) << std::dec << (int)x->cpu  
      << "   "    << std::setfill(' ') << std::setw(40) << std::left << g[v].name 
      << "   0x"  << std::hex << std::setfill('0') << std::setw(8) << x->hash
      << "   0x"  << std::hex << std::setfill('0') << std::setw(8) << at.adrConv(AdrType::MGMT, AdrType::INT, x->cpu, x->adr) 
      << "   0x"  << std::hex << std::setfill('0') << std::setw(8) << at.adrConv(AdrType::MGMT, AdrType::EXT, x->cpu, x->adr)  << std::endl;
    }
  }  

  sLog << std::endl;  

  if(debug) {
    BOOST_FOREACH( vertex_t v, vertices(g) ) {
      auto x = at.lookupVertex(v);
      dumpNode(x->cpu, g[x->v].name);
    }
  }  
}


  
  
using namespace coverage;


std::vector<std::vector<uint64_t>> CarpeDM::coverage3TestData(uint64_t seedStart, uint64_t cases, uint8_t parts, uint8_t percentage ) {

  patName.clear();
  patName[0] = "A";
  patName[1] = "B";
  patName[2] = "C";
  patName[3] = "idle";

  const int range_from  = 0;
  const int range_to    = 99;
  std::random_device                  rand_dev;
  std::mt19937                        generator(rand_dev());
  std::uniform_int_distribution<int>  distr(range_from, range_to);

  

  uint64_t seed = seedStart;
  std::vector<std::vector<uint64_t>> ret(parts);
  std::vector<uint64_t> tmp;
  //generate
  while( (tmp.size() < (( cases * percentage) / 100 ) ) && (seed < maxSeed) ) {
    if( coverage3IsSeedValid(seed) ) {
      int rnd = distr(generator);
      //std::cout << "rnd " << rnd << " thr " << (int)percentage << " " << ((rnd < (int)percentage) ? "YES" : "NO" ) <<  std::endl; 
      if (rnd < (int)percentage) tmp.push_back(seed); 
    }
    seed++;
  }

  //split
  uint64_t partSize = tmp.size() / parts;
  std::cout << " cnt " << tmp.size() << " parts " << (int)parts << " partsize " << partSize << std::endl;

  for(unsigned i=0; i < parts; i++) {
    size_t thisOffs = (size_t)((i+0)*partSize);
    size_t nextOffs = (size_t)((i+1)*partSize);
    ret[i].insert( ret[i].end(), tmp.begin() + thisOffs, (((tmp.begin() + nextOffs) <= tmp.end()) ? (tmp.begin() + nextOffs) : tmp.end()) );
  }

  return ret;

}





void CarpeDM::coverage3Upload(uint64_t seed ) {

  //std::cout << "F 0x" << std::setfill('0') << std::setw(10) <<  std::hex << seed << std::endl;

  Graph g, gCmd;
  vEbwrs tmpWr;


  coverage3GenerateBase(g);
  coverage3GenerateStatic(g, seed );
  //nullify();
  BOOST_FOREACH( vertex_t v, vertices(g) ) { g[v].hash = hm.hash(g[v].name); } //generate hash to complete vertex information
  updateModTime();
  overwrite(g, true);
  download();
  //writeDownDotFile("coverage.dot", false);

  coverage3GenerateDynamic(gCmd, seed );
  send(createCommandBurst(gCmd, tmpWr));
  setThrOrigin(0, 0, coverage3GenerateCursor(g, seed));
  forceThrCursor(0, 0);
  download();


  
}



bool CarpeDM::coverage3IsSeedValid(uint64_t seed) {
  uint32_t curInit  = (seed >> cursorPos) & cursorMsk;
  //uint32_t defInit  = (seed >> staticPos) & staticMsk;
  uint32_t qInit    = (seed >> dynPos)    & dynMsk;

  //std::cout << "curInit 0x" << std::hex << curInit << std::endl; 
  //std::cout << "defInit 0x" << std::hex << defInit << std::endl;

  //check cursor is valid
  if (curInit > 6) return false;
  //cfg.cursor = curInit; 

  //defInit is power of 2 and thus always valid
  //std::cout << "defInit " << std::dec;
  /*
  for (unsigned i=0; i < (staticBits / staticDigitBits); i++) {
    unsigned digit = (defInit >> (i * staticDigitBits)) & staticDigitMsk;
    //cfg.def[i] = digit;
    //std::cout << digit << ", ";
  }
  */
  //std::cout << std::endl;

  //check qInit is valid
  //std::cout << "qInit " << std::dec;
  for (unsigned i=0; i< (dynBits / dynDigitBits); i++) {
    unsigned digit = (qInit >> (i * dynDigitBits)) & dynDigitMsk;
    //std::cout << digit << ", ";
    //cfg.dyn[i] = digit;
    if (digit > 2) return false;
  }  
  //std::cout << std::endl;
  return true;
}

std::string CarpeDM::coverage3GenerateCursor(Graph& g, uint64_t seed ) {
  //cursor position 3 bit 
  uint32_t curInit  = (seed >> cursorPos) & cursorMsk;
  std::string cursor;
  if (curInit == 6) { cursor = patName[3]; }
  else {              cursor = (curInit & 1) ? g[patExit[patName[curInit >> 1]]].name : g[patEntry[patName[curInit >> 1]]].name; }
  
  return cursor;
}

Graph& CarpeDM::coverage3GenerateBase(Graph& g) {
  patEntry.clear();
  patExit.clear();

  for (unsigned i = 0; i < 3; i++) {
    vertex_t v = boost::add_vertex(g);
    patEntry[patName[i]] = v;
    g[v].name      = patName[i] + "_M";
    g[v].cpu       = "0";
    g[v].patEntry  = "true";
    g[v].type      = dnt::sTMsg;
    g[v].patName   = patName[i];
    g[v].tOffs     = "0";
    g[v].id_fid    = "1";
    g[v].id_gid    = "4048";
    g[v].id_sid    = "0";
    g[v].id_bpid   = "0";
    g[v].id_evtno  = std::to_string(i);
    g[v].par       = "0";

    
  }

  for (unsigned i = 0; i < 3; i++) {
    vertex_t v = boost::add_vertex(g);
    patExit[patName[i]] = v;
    g[v].name      = patName[i] + "_B";
    g[v].cpu       = "0";
    g[v].patExit   = "true";
    g[v].type      = dnt::sBlockFixed;
    g[v].patName   = patName[i];
    g[v].tPeriod   = "50000"; 
    g[v].qLo       = "true"; 
    
  }

  return g;
}



Graph& CarpeDM::coverage3GenerateStatic(Graph& g, uint64_t seed ) {
  //cursor position 3 bit (4b)
  //default Link onehot A -> x ( A | B | C | idle), B -> y, C -> z,   3 tri -> 6 bit (12b)

  

  uint32_t defInit  = (seed >> staticPos) & staticMsk;
  


  //since only one default successor is allowed, we can onehot encode successors, reducing permuations from 2^9 to 4^3 

  // symbols 0-3 -> (0 | A | B | C)
  //bit indices
  //from A to: 0, from B to: 4, from C to: 8 


  //generate default links
  for (unsigned auxFrom = 0; auxFrom < 3; auxFrom++) {
    vertex_t vMsg  = patEntry[patName[auxFrom]];  //link from msg to its block
    vertex_t vFrom  = patExit[patName[auxFrom]];  //from is always a pattern exit
    boost::add_edge(vMsg, vFrom, myEdge(det::sDefDst), g);
    unsigned auxTo = (defInit >> (auxFrom * staticDigitBits)) & staticDigitMsk;
    if (auxTo < 3) { // do nothing if it's zero (idle)
      vertex_t vTo = patEntry[patName[auxTo]]; //to is always a pattern entry
      boost::add_edge(vFrom, vTo, myEdge(det::sDefDst), g);
    }  
  }

  return g;
}


Graph& CarpeDM::coverage3GenerateDynamic(Graph& g, uint64_t seed) {
  //cursor position 3 bit (4b)
  //queue Link   matrix ABC x ABC -> ABC0ABC1ABC2 9 tri -> 18 bit 
  uint32_t qInit  = (seed >> dynPos) & dynMsk;
   // symbols 0-2 -> (0 | tempLink | permaLink)
  // convert index positions into vertex_descriptors
  // bitpos = indpexpos * 2
  //      to A:  to B:  to C:
  //from A:  0      2      4
  //from B:  6      8     10
  //from C: 12     14     16
  
  boost::set_property(g, boost::graph_name, DotStr::Graph::Special::sCmd);

  for (unsigned auxFrom = 0; auxFrom < 3; auxFrom++) {
    for (unsigned auxTo = 0; auxTo < 3; auxTo++) {
      uint8_t type = (qInit >> ((auxFrom * 3 + auxTo) * dynDigitBits)) & dynDigitMsk;
      if(type) {
        vertex_t v = boost::add_vertex(g);
        g[v].name       = "Flow_" + std::to_string(auxFrom * 3);
        g[v].type       = dnt::sCmdFlow;
        g[v].patName    = patName[auxFrom];
        g[v].cmdDestPat = patName[auxTo];
        g[v].prio       = "0";
        g[v].tValid     = "0"; 
        g[v].qty        = "1";
        g[v].perma      = (type == 2) ? "true" : "false";
      }  
    }
  }


  return g;
}


