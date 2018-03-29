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
      if (!hm.lookup(g[v].name)) {hashIsOk = false; hashReport += sMiss + sFirst + g[v].name + "\n";}
    }
    BOOST_FOREACH( vertex_t v, vertices(g) ) {
      //Check Alloctable
      auto x = at.lookupVertex(v);
      if (!at.isOk(x))           {allocIsOk = false; allocReport += sMiss + sFirst + g[v].name + "\n";}
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

      report += " RdCnt: " + std::to_string((int)qr.aQ[prio].rdIdx) 
              + " WrCnt: " + std::to_string((int)qr.aQ[prio].wrIdx) 
              + "    Pending: " + std::to_string((int)qr.aQ[prio].pendingCnt) + "\n";
      
      //find buffers of all non empty slots
      for (uint8_t i = 0; i <= Q_IDX_MAX_MSK; i++) {
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
    
    const std::string exIntro = " getQReport: ";
    const std::string nodeNotFound = " Node could not be found: ";

    if (!(hm.lookup(blockName))) throw std::runtime_error(exIntro + nodeNotFound + blockName); 
    auto x = at.lookupHash(hm.lookup(blockName).get()); // x is the blocks alloctable entry
    if (!(at.isOk(x))) throw std::runtime_error(" allocTable: " + nodeNotFound + blockName);

    //check their Q counters for unprocessed commands
    uint32_t wrIdxs = boost::dynamic_pointer_cast<Block>(g[x->v].np)->getWrIdxs(); 
    uint32_t rdIdxs = boost::dynamic_pointer_cast<Block>(g[x->v].np)->getRdIdxs();

    for (uint8_t prio = PRIO_LO; prio <= PRIO_IL; prio++) {
      
      uint32_t bufLstAdr;
      uint8_t bufLstCpu;
      AdrType bufLstAdrType; 
      
      //get Block binary
      uint8_t* bBlock = (uint8_t*)&(x->b);
      bufLstAdr = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&bBlock[BLOCK_CMDQ_LO_PTR + prio * _32b_SIZE_]);
      std::tie(bufLstCpu, bufLstAdrType) = at.adrClassification(bufLstAdr);  
      //get BufList binary
      auto bufLst = at.lookupAdr( x->cpu, at.adrConv(bufLstAdrType, AdrType::MGMT, x->cpu, bufLstAdr) ); //buffer list cpu is the same as block cpu
      if (!(at.isOk(bufLst))) { continue; }
      else                    { qr.hasQ[prio] = true; }

      const uint8_t* bBL = bufLst->b;  
       
      //get current read cnt
      uint8_t auxRd = (rdIdxs >> (prio*8)) & Q_IDX_MAX_OVF_MSK;
      uint8_t auxWr = (wrIdxs >> (prio*8)) & Q_IDX_MAX_OVF_MSK;

      uint8_t rdIdx = auxRd & Q_IDX_MAX_MSK;
      //uint8_t wrIdx = auxWr & Q_IDX_MAX_MSK; no use for it right now
      uint8_t pendingCnt = (auxWr > auxRd) ? auxWr - auxRd : auxRd - auxWr;
         
      //set of all pending command indices
      std::set<uint8_t> pendingIdx;
      if (auxRd != auxWr) {for(uint8_t pidx = rdIdx; pidx < (rdIdx + pendingCnt); pidx++) {pendingIdx.insert( pidx & Q_IDX_MAX_MSK);}}

      qr.aQ[prio].wrIdx   = auxWr;
      qr.aQ[prio].rdIdx   = auxRd;
      qr.aQ[prio].pendingCnt = pendingIdx.size();

      //find buffers of all non empty slots
      for (uint8_t i = 0; i <= Q_IDX_MAX_MSK; i++) {
        uint8_t bufCpu;
        AdrType bufAdrType;
        
        uint32_t bufAdr = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&bBL[(i / 2) * _32b_SIZE_] );
        std::tie(bufCpu, bufAdrType) = at.adrClassification(bufAdr);  
        auto buf = at.lookupAdr( x->cpu, at.adrConv(bufAdrType, AdrType::MGMT, x->cpu, bufAdr) ); //buffer cpu is the same as block cpu
        if (!(at.isOk(buf))) {continue;}

        qr.aQ[prio].aQe[i] = getQelement(pendingIdx.count(i), buf, qr.aQ[prio].aQe[i]);
      }
    }
      
    return qr;
  }

  QueueElement& CarpeDM::getQelement(bool pending, amI allocIt, QueueElement& qe) {
    //TODO might cleaner as deserialisers for MiniCommand Class
    Graph&       g  = gDown;
    AllocTable& at  = atDown;
    uint8_t*     b  = (uint8_t*)&(allocIt->b);
    uint8_t    cpu  = allocIt->cpu;
    uint32_t   act  = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&b[T_CMD_ACT]);
    uint8_t   type  = (act >> ACT_TYPE_POS) & ACT_TYPE_MSK;

    qe.pending      = pending;
    qe.validTime    = writeBeBytesToLeNumber<uint64_t>((uint8_t*)&b[T_CMD_TIME]);
    qe.qty          = (act >> ACT_QTY_POS) & ACT_QTY_MSK;
    qe.type         = type; //for conevenient use of case statements

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
                                auto dst = at.lookupAdr( dstCpu, at.adrConv(dstAdrType, AdrType::MGMT, dstCpu, dstAdr) );
                                if (!(at.isOk(dst)))  {sDst = DotStr::Misc::sUndefined;}
                                else                  {sDst = g[dst->v].name;}
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
 
    auto it = atDown.lookupHash(hm.lookup(name).get());
    if (atDown.isOk(it)) {
      auto* x = (AllocMeta*)&(*it);  
      hexDump(g[x->v].name.c_str(), (const char*)x->b, _MEM_BLOCK_SIZE); 
    }
}

void CarpeDM::inspectHeap(uint8_t cpuIdx) {
  vAdr vRa;
  vBuf heap;
  

  uint32_t baseAdr = cpuDevs.at(cpuIdx).sdb_component.addr_first + atDown.getMemories()[cpuIdx].sharedOffs;
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


HealthReport& CarpeDM::getHealth(uint8_t cpuIdx, HealthReport &hr) {
  uint32_t const baseAdr = cpuDevs.at(cpuIdx).sdb_component.addr_first + atDown.getMemories()[cpuIdx].sharedOffs;

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
  hr.stat             = writeBeBytesToLeNumber<uint32_t>(b + _T_DIAG_SIZE_); // stat comes after last element of T_DIAG
  
  return hr;
  
} 
