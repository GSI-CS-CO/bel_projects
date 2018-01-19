#include "visitoruploadcrawler.h"
#include "common.h"
#include "node.h"
#include "block.h"
#include "meta.h"
#include "event.h"
#include "dotstr.h"

namespace dnp = DotStr::Node::Prop;
namespace dnt = DotStr::Node::TypeVal;
namespace dep = DotStr::Edge::Prop;
namespace det = DotStr::Edge::TypeVal;

const std::string VisitorUploadCrawler::exIntro = "VisitorUploadCrawler: ";

//FIXME Dear future self, the code duplication in here is appalling. Create some proper helper functions for crying out loud !

void VisitorUploadCrawler::visit(const Block& el) const {
  el.serialise(getDefDst() + getQInfo());
}

void VisitorUploadCrawler::visit(const TimingMsg& el) const  {
  el.serialise(getDefDst() + getDynSrc() );
}

void VisitorUploadCrawler::visit(const Flow& el) const  {
  el.serialise( getDefDst() + getCmdTarget((Command&)el) + getFlowDst() );
}

void VisitorUploadCrawler::visit(const Flush& el) const {
  el.serialise( getDefDst() + getCmdTarget((Command&)el) );
}

void VisitorUploadCrawler::visit(const Noop& el) const {
  el.serialise( getDefDst() + getCmdTarget((Command&)el) );
}

void VisitorUploadCrawler::visit(const Wait& el) const {
  el.serialise( getDefDst() + getCmdTarget((Command&)el) );
}

void VisitorUploadCrawler::visit(const CmdQMeta& el) const {
  el.serialise( getDefDst() + getQBuf() );
}

void VisitorUploadCrawler::visit(const CmdQBuffer& el) const {
  el.serialise(getDefDst());
}

void VisitorUploadCrawler::visit(const DestList& el) const {
  el.serialise(getListDst());
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private helper functions

 vAdr VisitorUploadCrawler::getDefDst() const {
    bool found = false;
    
    vAdr ret;
    Graph::out_edge_iterator out_begin, out_end, out_cur;
    boost::tie(out_begin, out_end) = out_edges(v,g);
 
    for (out_cur = out_begin; out_cur != out_end; ++out_cur)
    {   
      if (g[target(*out_cur,g)].np == nullptr) throw std::runtime_error( exIntro + "Node " + g[target(*out_cur,g)].name + " of type " + g[target(*out_cur,g)].type + " was found unallocated\n");

      if (!(g[target(*out_cur,g)].np->isMeta()) && g[*out_cur].type == det::sDefDst) {
        auto x = at.lookupVertex(target(*out_cur,g));
        // Destination MUST NOT lie outside own memory! (well, technically, it'd' work, but it'd be race condition galore ...)
        if (at.isOk(x) && x->cpu == cpu) {
          ret.push_back(at.adrConv(AdrType::MGMT, AdrType::INT, x->cpu, x->adr));
          found = true; 
        }
      }
    }
    if (!(found)) { ret.push_back(LM32_NULL_PTR); }

    return ret;
  }

  vAdr VisitorUploadCrawler::getDynSrc() const {
    
    vAdr ret;
    Graph::out_edge_iterator out_begin, out_end, out_cur;
    boost::tie(out_begin, out_end) = out_edges(v,g);
 
    uint32_t aId   = LM32_NULL_PTR;
    uint32_t aPar0 = LM32_NULL_PTR;
    uint32_t aPar1 = LM32_NULL_PTR;
    uint32_t aTef  = LM32_NULL_PTR;
    uint32_t aRes  = LM32_NULL_PTR;

    
    for (out_cur = out_begin; out_cur != out_end; ++out_cur)
    {   
      if (g[target(*out_cur,g)].np == nullptr) sErr << g[target(*out_cur,g)].name << " is UNDEFINED" << std::endl;
      else {

        if (g[*out_cur].type == det::sDynId) {
          if (aId != LM32_NULL_PTR) {sErr << "Found more than one dynamic id source" << std::endl; break;
          } else {
            auto x = at.lookupVertex(target(*out_cur,g));
            if (at.isOk(x)) { aId = at.adrConv(AdrType::MGMT, AdrType::EXT, x->cpu, x->adr); g[v].np->setFlags(NFLG_TMSG_DYN_ID_SMSK);}
          }
        }
        if (g[*out_cur].type == det::sDynPar0) {
          if (aPar0 != LM32_NULL_PTR) {sErr << "Found more than one dynamic par0 source" << std::endl; break;
          } else {
            auto x = at.lookupVertex(target(*out_cur,g));
            if (at.isOk(x)) { aPar0 = at.adrConv(AdrType::MGMT, AdrType::EXT, x->cpu, x->adr); g[v].np->setFlags(NFLG_TMSG_DYN_PAR0_SMSK);}
            //sLog << "DynAdr 0 0x" << std::hex << aPar0 << std::endl;
          }
        }
        if (g[*out_cur].type == det::sDynPar1) {
          if (aPar1 != LM32_NULL_PTR) {sErr << "Found more than one dynamic par1 source" << std::endl; break;
          } else {
            auto x = at.lookupVertex(target(*out_cur,g));
            if (at.isOk(x)) { aPar1 = at.adrConv(AdrType::MGMT, AdrType::EXT, x->cpu, x->adr); g[v].np->setFlags(NFLG_TMSG_DYN_PAR1_SMSK);}
            //sLog << "DynAdr 1 0x" << std::hex << aPar1 << std::endl;
          }
        }
        if (g[*out_cur].type == det::sDynTef) {
          if (aTef != LM32_NULL_PTR) {sErr << "Found more than one dynamic tef source" << std::endl; break;
          } else {
            auto x = at.lookupVertex(target(*out_cur,g));
            if (at.isOk(x)) { aTef = at.adrConv(AdrType::MGMT, AdrType::EXT, x->cpu, x->adr); g[v].np->setFlags(NFLG_TMSG_DYN_TEF_SMSK);}
          }
        }
        if (g[*out_cur].type == det::sDynRes) {
          if (aRes != LM32_NULL_PTR) {sErr << "Found more than one dynamic res source" << std::endl; break;
          } else {
            auto x = at.lookupVertex(target(*out_cur,g));
            if (at.isOk(x)) { aRes = at.adrConv(AdrType::MGMT, AdrType::EXT, x->cpu, x->adr); g[v].np->setFlags(NFLG_TMSG_DYN_RES_SMSK);}
          }
        }
      }
    }

    ret.push_back(aId);  
    ret.push_back(aPar0);
    ret.push_back(aPar1);
    ret.push_back(aTef);
    ret.push_back(aRes);       

    return ret;
  }

  vAdr VisitorUploadCrawler::getQInfo() const {
    int idx;
    bool found;
    
    vAdr ret;
    Graph::out_edge_iterator out_begin, out_end, out_cur;
    boost::tie(out_begin, out_end) = out_edges(v,g);
    
    found = false;
    for (out_cur = out_begin; out_cur != out_end; ++out_cur)
    { 
     // sErr << "Scanning " << g[target(*out_cur,g)].name << "(classMeta=" << (int)g[target(*out_cur,g)].np->isMeta() << ")," << g[target(*out_cur,g)].type << " connected by " << g[*out_cur].type << " edge against " << det::sDstList << "," << nDstList << std::endl;  

      if (g[target(*out_cur,g)].np == nullptr) sErr << g[target(*out_cur,g)].name << " is UNDEFINED" << std::endl;
      else {
        if (g[target(*out_cur,g)].np->isMeta() && g[*out_cur].type == det::sDstList) {

          if (found) {//sErr << "Found more than one Destination List" << std::endl; 
            break;
          } else {
            auto x = at.lookupVertex(target(*out_cur,g));
            // Queue nodes MUST NOT lie outside own memory!
            //sErr << "Got a DstList at " << g[target(*out_cur,g)].name << std::endl;  
            if (at.isOk(x) && x->cpu == cpu) {
              ret.push_back(at.adrConv(AdrType::MGMT, AdrType::INT, x->cpu, x->adr));
              found = true;
            } else {
              sErr << "unallocated or wrong cpu " << std::endl;  
            }
          }
        }
      }  
    }
    if (!(found)) { ret.push_back(LM32_NULL_PTR); 
      //sErr << "Found no Destination List" << std::endl; 
    }

    for (idx=0; idx < 3; idx++) {
      found = false;
      for (out_cur = out_begin; out_cur != out_end; ++out_cur)
      { 
       // sErr << "Scanning " << g[target(*out_cur,g)].name << "(classMeta=" << (int)g[target(*out_cur,g)].np->isMeta() << ") connected by " << g[*out_cur].type << " edge against " << det::sQPrio[idx] << std::endl;   
        if (g[target(*out_cur,g)].np == nullptr) sErr << g[target(*out_cur,g)].name << " is UNDEFINED" << std::endl;
        else {

          if (g[target(*out_cur,g)].np->isMeta() && g[*out_cur].type == det::sQPrio[idx]) {
            if (found) {sErr << "Found more than one queue info of type " << det::sQPrio[idx] << "" << std::endl; break;}
            else {
              auto x = at.lookupVertex(target(*out_cur,g));
              // Queue nodes MUST NOT lie outside own memory!
              if (at.isOk(x) && x->cpu == cpu) {
                ret.push_back(at.adrConv(AdrType::MGMT, AdrType::INT, x->cpu, x->adr));
                found = true;
              } else {
              sErr << "unallocated or wrong cpu " << std::endl;  
            }
            }  
          }
        }  
      }
      if (!(found)) { ret.push_back(LM32_NULL_PTR);
       //sErr << "Found no Q Buffers" << std::endl; 
     }
    }

    return ret;
  }


vAdr VisitorUploadCrawler::getQBuf() const {
  bool found;
  
  vAdr ret;
  Graph::out_edge_iterator out_begin, out_end, out_cur;
  boost::tie(out_begin, out_end) = out_edges(v,g);
  
  found = false;
  for (out_cur = out_begin; out_cur != out_end; ++out_cur)
  {   
    if (g[target(*out_cur,g)].np == nullptr) sErr << g[target(*out_cur,g)].name << " is UNDEFINED" << std::endl;
    else {

      if (g[target(*out_cur,g)].np->isMeta()) {
        auto x = at.lookupVertex(target(*out_cur,g));
        // Queue nodes MUST NOT lie outside own memory!
        if (at.isOk(x) && x->cpu == cpu) {
          ret.push_back(at.adrConv(AdrType::MGMT, AdrType::INT, x->cpu, x->adr));
          found = true;
        }
      }
    }  
  }

  if (!(found)) ret.push_back(LM32_NULL_PTR);

  return ret;
}

vAdr VisitorUploadCrawler::getCmdTarget(Command& el) const {
  bool found;
  
  vAdr ret;
  Graph::out_edge_iterator out_begin, out_end, out_cur;
  boost::tie(out_begin, out_end) = out_edges(v,g);
  
  found = false;
  for (out_cur = out_begin; out_cur != out_end; ++out_cur)
  {   
    if (g[target(*out_cur,g)].np == nullptr) sErr << g[target(*out_cur,g)].name << " is UNDEFINED" << std::endl;
    else {

      if (!(g[target(*out_cur,g)].np->isMeta()) && g[*out_cur].type == det::sCmdTarget) {
        if (found) {sErr << "Found more than one target" << std::endl; break;
        } else {
          auto x = at.lookupVertex(target(*out_cur,g));
          if (at.isOk(x)) {
            //command cross over to other CPUs is okay, handle by checking if caller cpu idx is different to found child cpu idx
            //sLog << "Caller CPU " << int(cpu) << " Callee CPU " << (int)x->cpu << std::endl;
            ret.push_back(x->cpu == cpu ? at.adrConv(AdrType::MGMT, AdrType::INT, x->cpu, x->adr) : at.adrConv(AdrType::MGMT, AdrType::PEER, x->cpu, x->adr));
            found = true;
          }
        }
      }
    }  
  }

  if (!(found)) ret.push_back(LM32_NULL_PTR);

  return ret;
}

vAdr VisitorUploadCrawler::getFlowDst() const {
  bool found;
  uint8_t targetCpu;
  
  vAdr ret;


  Graph::out_edge_iterator out_begin, out_end, out_cur;
  boost::tie(out_begin, out_end) = out_edges(v,g);
  
  // find the command target (again) and check if it is internal or at a peer
  found = false;
  for (out_cur = out_begin; out_cur != out_end; ++out_cur)
  {   
    if (g[target(*out_cur,g)].np == nullptr) sErr << g[target(*out_cur,g)].name << " is UNDEFINED" << std::endl;
    else {

      if (!(g[target(*out_cur,g)].np->isMeta()) && g[*out_cur].type == det::sCmdTarget) {
        if (found) {sErr << "Found more than one target" << std::endl; break;
        } else {
          auto x = at.lookupVertex(target(*out_cur,g));
          //command cross over to other CPUs is okay. Find out what Cpu the command target is on
          if (at.isOk(x)) { targetCpu = x->cpu; }
        }
      }  
    } 
  }
  found = false;
  for (out_cur = out_begin; out_cur != out_end; ++out_cur)
  {   
    if (g[target(*out_cur,g)].np == nullptr) sErr << g[target(*out_cur,g)].name << " is UNDEFINED" << std::endl;
    else {

      if (!(g[target(*out_cur,g)].np->isMeta()) && g[*out_cur].type == det::sCmdFlowDst) {
        if (found) {sErr << "Found more than one flow destination" << std::endl; break;
        } else {
          auto x = at.lookupVertex(target(*out_cur,g));
          // Flow Destination must be in the same memory the command target is in
          if (at.isOk(x) && x->cpu == targetCpu) {
            ret.push_back(at.adrConv(AdrType::MGMT, AdrType::INT, x->cpu, x->adr));
            found = true;
          }
        }
      }
    }  
  }

  if (!(found)) ret.push_back(LM32_NULL_PTR);

  return ret;
}

vAdr VisitorUploadCrawler::getListDst() const {
  bool found;

  
  vAdr ret;
  Graph::out_edge_iterator out_begin, out_end, out_cur;
  Graph::in_edge_iterator in_begin, in_end;
  vertex_t vp;

  //get the parent. there shall be only one, a block (no check for that right now, sorry)

  //sErr << " crawling edges for " << g[v].name << "'s destlist" << std::endl;

  boost::tie(in_begin, in_end) = in_edges(v,g);
  vp = source(*in_begin,g);
  
  
  boost::tie(out_begin, out_end) = out_edges(vp,g);
  
  //search parent blocks default destination
  
  found = false;
  for (out_cur = out_begin; out_cur != out_end; ++out_cur)
  {   
    
    if (g[target(*out_cur,g)].np == nullptr) sErr << g[target(*out_cur,g)].name << " is UNDEFINED" << std::endl;
    else {

      if (!(g[target(*out_cur,g)].np->isMeta()) && g[*out_cur].type == det::sDefDst) {
        if (found) {sErr << "Found more than one default destination" << std::endl; break;
        } else {
          auto x = at.lookupVertex(target(*out_cur,g));
          // Destination MUST NOT lie outside own memory! (well, technically, it'd work, but it'd be race condition galore ...)
          if (at.isOk(x) && x->cpu == cpu) {
            ret.push_back(at.adrConv(AdrType::MGMT, AdrType::INT, x->cpu, x->adr));
            found = true;
            //sLog << "defDst: " << g[target(*out_cur,g)].name << " @ 0x" << std::hex << at.adrConv(AdrType::MGMT, AdrType::INT,cpu, x->adr) << std::endl;
          } else { sErr << "default destination was found unallocated or on different CPU" << std::endl; }
        }
      }
    }  
  }
  if (!(found)) { ret.push_back(LM32_NULL_PTR); 
    //sLog << "No def dest found" << std::endl; 
  }

  //search parent blocks alternative destinations
  found = false;
  for (out_cur = out_begin; out_cur != out_end; ++out_cur)
  {   
    if (g[target(*out_cur,g)].np == nullptr) sErr << g[target(*out_cur,g)].name << " is UNDEFINED" << std::endl;
    else {

      if (g[*out_cur].type == det::sAltDst) {
        auto x = at.lookupVertex(target(*out_cur,g));
        // Destination MUST NOT lie outside own memory! (well, technically, it'd work, but it'd be race condition galore ...)
        if (at.isOk(x)) {
          if (  x->cpu == cpu) {
            ret.push_back(at.adrConv(AdrType::MGMT, AdrType::INT, x->cpu, x->adr));
            found = true;
            //sLog << "altDst: #" << target(*out_cur,g) << " " << g[target(*out_cur,g)].name << " @ 0x" << std::hex << at.adrConv(AdrType::MGMT, AdrType::INT,cpu, x->adr) << std::endl;
          } else { sLog << "altDst: #" << target(*out_cur,g) << " " << g[target(*out_cur,g)].name << " @ 0x" << std::hex << at.adrConv(AdrType::MGMT, AdrType::INT,cpu, x->adr) << " expected at CPU" << cpu << ", found on " << (int)x->cpu << "" << std::endl;  at.debug(sLog);}
        } else { 
          //sErr << "alt destination was found unallocated" << std::endl; 
        }
      }
    }  
  }
  /*
  if (!(found)) { //ret.push_back(LM32_NULL_PTR); 
    sLog << "No alt dest found" << std::endl; }
  */


  
  return ret;

}

/*
// starting on helper functions for cleanup
vAdr getChildrenByEdgeType(vertex_t vStart, const std::string edgeType, const unsigned int minResultLen, const unsigned int maxResultLen, const uint32_t resultPadData ) {
  Graph::out_edge_iterator out_begin, out_end, out_cur;
  vAdr ret;
  boost::tie(out_begin, out_end) = out_edges(vStart,g);

  for (out_cur = out_begin; out_cur != out_end; ++out_cur)
  {   
    if (g[target(*out_cur,g)].np == nullptr) throw std::runtime_error( exIntro + "Node " + g[target(*out_cur,g)].name + " of type " + g[target(*out_cur,g)].type + " has not data object\n");
    if (g[*out_cur].type == edgeType) {
      auto x = at.lookupVertex(target(*out_cur,g));
      if (!at.isOk(x)) throw std::runtime_error( exIntro + "Node " + g[target(*out_cur,g)].name + " of type " + g[target(*out_cur,g)].type + " was found unallocated\n");
      ret.push_back(x->cpu == cpu ? at.adrConv(AdrType::MGMT, AdrType::INT, x->cpu, x->adr) : at.adrConv(AdrType::MGMT, AdrType::PEER, x->cpu, x->adr));
      if (ret.size() >= maxResultLen) break;
    }
  }
  for (int i = ret.size(); i <= minResultLen; i++ ) ret.push_back(resultPadData);

  return ret;  
}  



vAdr findNodeAdrByEdgeType(vertex_t vStart, const std::string edgeType, const unsigned int minResultLen, const unsigned int maxResultLen, const uint32_t resultPadData ) {
  Graph::out_edge_iterator out_begin, out_end, out_cur;
  vAdr ret;
  boost::tie(out_begin, out_end) = out_edges(vStart,g);

  for (out_cur = out_begin; out_cur != out_end; ++out_cur)
  {   
    if (g[target(*out_cur,g)].np == nullptr) throw std::runtime_error( exIntro + "Node " + g[target(*out_cur,g)].name + " of type " + g[target(*out_cur,g)].type + " has not data object\n");
    if (g[*out_cur].type == edgeType) {
      auto x = at.lookupVertex(target(*out_cur,g));
      if (!at.isOk(x)) throw std::runtime_error( exIntro + "Node " + g[target(*out_cur,g)].name + " of type " + g[target(*out_cur,g)].type + " was found unallocated\n");
      ret.push_back(x->cpu == cpu ? at.adrConv(AdrType::MGMT, AdrType::INT, x->cpu, x->adr) : at.adrConv(AdrType::MGMT, AdrType::PEER, x->cpu, x->adr));
      if (ret.size() >= maxResultLen) break;
    }
  }
  for (int i = ret.size(); i <= minResultLen; i++ ) ret.push_back(resultPadData);

  return ret;  
}  

*/
