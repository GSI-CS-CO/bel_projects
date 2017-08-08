#include "visitoruploadcrawler.h"
#include "common.h"
#include "node.h"
#include "block.h"
#include "meta.h"
#include "event.h"

const std::string sNodeType[] = {"priolo", "priohi", "prioil"};
const std::string sQM[] = {"priolo", "priohi", "prioil"};
const std::string sDL  = "listdst";
const std::string sDD  = "defdst";
const std::string sAD  = "altdst";
const std::string sTG  = "target";
const std::string sFD  = "flowdst";
const std::string sDID  = "dynid";
const std::string sDPAR0  = "dynpar0";
const std::string sDPAR1  = "dynpar1";
const std::string sDTEF  = "dyntef";
const std::string sDRES  = "dynres";



void VisitorUploadCrawler::visit(const Block& el) const {
  vAdr vA, tmpDD, tmpQM;
  tmpDD = getDefDst();
  tmpQM = getQInfo();

  vA.reserve( tmpDD.size() + tmpQM.size() ); // preallocate memory
  vA.insert( vA.end(), tmpDD.begin(), tmpDD.end() );
  vA.insert( vA.end(), tmpQM.begin(), tmpQM.end() );

  el.serialise(vA);
}

void VisitorUploadCrawler::visit(const TimingMsg& el) const  {
  vAdr vA, tmpDD, tmpDS;
  tmpDD = getDefDst();
  tmpDS = getDynSrc();

  vA.reserve( tmpDD.size() + tmpDS.size() ); // preallocate memory
  vA.insert( vA.end(), tmpDD.begin(), tmpDD.end() );
  vA.insert( vA.end(), tmpDS.begin(), tmpDS.end() );


  el.serialise(vA);
}

void VisitorUploadCrawler::visit(const Flow& el) const  {
  vAdr vA, tmpDD, tmpCT, tmpFD;
  tmpDD = getDefDst();
  tmpCT = getCmdTarget((Command&)el);
  tmpFD = getFlowDst();

  vA.reserve( tmpDD.size() + tmpCT.size() + tmpFD.size() ); // preallocate memory
  vA.insert( vA.end(), tmpDD.begin(), tmpDD.end() );
  vA.insert( vA.end(), tmpCT.begin(), tmpCT.end() );
  vA.insert( vA.end(), tmpFD.begin(), tmpFD.end() );

  el.serialise(vA);

}

void VisitorUploadCrawler::visit(const Flush& el) const {
  vAdr vA, tmpDD, tmpCT;
  tmpDD = getDefDst();
  tmpCT = getCmdTarget((Command&)el);

  vA.reserve( tmpDD.size() + tmpCT.size() ); // preallocate memory
  vA.insert( vA.end(), tmpDD.begin(), tmpDD.end() );
  vA.insert( vA.end(), tmpCT.begin(), tmpCT.end() );

  el.serialise(vA);

}

void VisitorUploadCrawler::visit(const Noop& el) const {
  vAdr vA, tmpDD, tmpCT;
  tmpDD = getDefDst();
  tmpCT = getCmdTarget((Command&)el);

  vA.reserve( tmpDD.size() + tmpCT.size() ); // preallocate memory
  vA.insert( vA.end(), tmpDD.begin(), tmpDD.end() );
  vA.insert( vA.end(), tmpCT.begin(), tmpCT.end() );

  el.serialise(vA);

}

void VisitorUploadCrawler::visit(const Wait& el) const {
  vAdr vA, tmpDD, tmpCT;
  tmpDD = getDefDst();
  tmpCT = getCmdTarget((Command&)el);

  vA.reserve( tmpDD.size() + tmpCT.size() ); // preallocate memory
  vA.insert( vA.end(), tmpDD.begin(), tmpDD.end() );
  vA.insert( vA.end(), tmpCT.begin(), tmpCT.end() );

  el.serialise(vA);

}

void VisitorUploadCrawler::visit(const CmdQMeta& el) const {
  vAdr vA, tmpDD, tmpQB;
  tmpDD = getDefDst();
  tmpQB = getQBuf();

  vA.reserve( tmpDD.size() + tmpQB.size() ); // preallocate memory
  vA.insert( vA.end(), tmpDD.begin(), tmpDD.end() );
  vA.insert( vA.end(), tmpQB.begin(), tmpQB.end() );

  el.serialise(vA);
}

void VisitorUploadCrawler::visit(const CmdQBuffer& el) const {
  el.serialise(getDefDst());
}

void VisitorUploadCrawler::visit(const DestList& el) const {
  
  vAdr vA, tmpDL;
  tmpDL = getListDst();
  el.serialise(tmpDL);
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
      if (g[target(*out_cur,g)].np == NULL) std::cerr << g[target(*out_cur,g)].name << " is UNDEFINED" << std::endl;
      else {

        if (!(g[target(*out_cur,g)].np->isMeta()) && g[*out_cur].type == sDD) {
          if (found) {std::cerr << "!!! Found more than one default destination !!!" << std::endl; break;
          } else {
            
            auto* x = at.lookupVertex(target(*out_cur,g));
            // Destination MUST NOT lie outside own memory! (well, technically, it'd' work, but it'd be race condition galore ...)
            if (x != NULL && x->cpu == cpu) {
              ret.push_back(at.adr2intAdr(x->cpu, x->adr));
              found = true; 
            }
          }
        }
      }  
    }
    if (!(found)) ret.push_back(LM32_NULL_PTR);

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
      if (g[target(*out_cur,g)].np == NULL) std::cerr << g[target(*out_cur,g)].name << " is UNDEFINED" << std::endl;
      else {

        if (g[*out_cur].type == sDID) {
          if (aId != LM32_NULL_PTR) {std::cerr << "!!! Found more than one dynamic id source !!!" << std::endl; break;
          } else {
            auto* x = at.lookupVertex(target(*out_cur,g));
            if (x != NULL) { aId = at.adr2extAdr(x->cpu, x->adr); g[v].np->setFlags(NFLG_TMSG_DYN_ID_SMSK);}
          }
        }
        if (g[*out_cur].type == sDPAR0) {
          if (aPar0 != LM32_NULL_PTR) {std::cerr << "!!! Found more than one dynamic par0 source !!!" << std::endl; break;
          } else {
            auto* x = at.lookupVertex(target(*out_cur,g));
            if (x != NULL) { aPar0 = at.adr2extAdr(x->cpu, x->adr); g[v].np->setFlags(NFLG_TMSG_DYN_PAR0_SMSK);}
            //std::cout << "DynAdr 0 0x" << std::hex << aPar0 << std::endl;
          }
        }
        if (g[*out_cur].type == sDPAR1) {
          if (aPar1 != LM32_NULL_PTR) {std::cerr << "!!! Found more than one dynamic par1 source !!!" << std::endl; break;
          } else {
            auto* x = at.lookupVertex(target(*out_cur,g));
            if (x != NULL) { aPar1 = at.adr2extAdr(x->cpu, x->adr); g[v].np->setFlags(NFLG_TMSG_DYN_PAR1_SMSK);}
            //std::cout << "DynAdr 1 0x" << std::hex << aPar1 << std::endl;
          }
        }
        if (g[*out_cur].type == sDTEF) {
          if (aTef != LM32_NULL_PTR) {std::cerr << "!!! Found more than one dynamic tef source !!!" << std::endl; break;
          } else {
            auto* x = at.lookupVertex(target(*out_cur,g));
            if (x != NULL) { aTef = at.adr2extAdr(x->cpu, x->adr); g[v].np->setFlags(NFLG_TMSG_DYN_TEF_SMSK);}
          }
        }
        if (g[*out_cur].type == sDRES) {
          if (aRes != LM32_NULL_PTR) {std::cerr << "!!! Found more than one dynamic res source !!!" << std::endl; break;
          } else {
            auto* x = at.lookupVertex(target(*out_cur,g));
            if (x != NULL) { aRes = at.adr2extAdr(x->cpu, x->adr); g[v].np->setFlags(NFLG_TMSG_DYN_RES_SMSK);}
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
      if (g[target(*out_cur,g)].np == NULL) std::cerr << g[target(*out_cur,g)].name << " is UNDEFINED" << std::endl;
      else {
        if (g[target(*out_cur,g)].np->isMeta() && g[target(*out_cur,g)].type == sDL) {
          if (found) {std::cerr << "!!! Found more than one Destination List !!!" << std::endl; break;
          } else {
            auto* x = at.lookupVertex(target(*out_cur,g));
            // Queue nodes MUST NOT lie outside own memory!
            if (x != NULL && x->cpu == cpu) {
              ret.push_back(at.adr2intAdr(x->cpu, x->adr));
              found = true;
            }
          }
        }
      }  
    }
    if (!(found)) ret.push_back(LM32_NULL_PTR);

    for (idx=0; idx < 3; idx++) {
      found = false;
      for (out_cur = out_begin; out_cur != out_end; ++out_cur)
      {   
        if (g[target(*out_cur,g)].np == NULL) std::cerr << g[target(*out_cur,g)].name << " is UNDEFINED" << std::endl;
        else {

          if (g[target(*out_cur,g)].np->isMeta() && g[*out_cur].type == sQM[idx]) {
            if (found) {std::cerr << "!!! Found more than one queue info of type " << sQM[idx] << " !!!" << std::endl; break;}
            else {
              auto* x = at.lookupVertex(target(*out_cur,g));
              // Queue nodes MUST NOT lie outside own memory!
              if (x != NULL && x->cpu == cpu) {
                ret.push_back(at.adr2intAdr(x->cpu, x->adr));
                found = true;
              }
            }  
          }
        }  
      }
      if (!(found)) ret.push_back(LM32_NULL_PTR);
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
    if (g[target(*out_cur,g)].np == NULL) std::cerr << g[target(*out_cur,g)].name << " is UNDEFINED" << std::endl;
    else {

      if (g[target(*out_cur,g)].np->isMeta()) {
        auto* x = at.lookupVertex(target(*out_cur,g));
        // Queue nodes MUST NOT lie outside own memory!
        if (x != NULL && x->cpu == cpu) {
          ret.push_back(at.adr2intAdr(x->cpu, x->adr));
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
    if (g[target(*out_cur,g)].np == NULL) std::cerr << g[target(*out_cur,g)].name << " is UNDEFINED" << std::endl;
    else {

      if (!(g[target(*out_cur,g)].np->isMeta()) && g[*out_cur].type == sTG) {
        if (found) {std::cerr << "!!! Found more than one target !!!" << std::endl; break;
        } else {
          auto* x = at.lookupVertex(target(*out_cur,g));
          if (x != NULL) {
            //command cross over to other CPUs is okay, handle by checking if caller cpu idx is different to found child cpu idx
            //std::cout << "Caller CPU " << int(cpu) << " Callee CPU " << (int)x->cpu << std::endl;
            ret.push_back(x->cpu == cpu ? at.adr2intAdr(x->cpu, x->adr) : at.adr2peerAdr(x->cpu, x->adr));
            //now we have a problem: we need to reach out to set the peer flag on the calling node. not strictly nice ...
            el.clrAct(ACT_TCPU_SMSK);
            el.setAct((x->cpu & ACT_TCPU_MSK) << ACT_TCPU_POS);
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
    if (g[target(*out_cur,g)].np == NULL) std::cerr << g[target(*out_cur,g)].name << " is UNDEFINED" << std::endl;
    else {

      if (!(g[target(*out_cur,g)].np->isMeta()) && g[*out_cur].type == sTG) {
        if (found) {std::cerr << "!!! Found more than one target !!!" << std::endl; break;
        } else {
          auto* x = at.lookupVertex(target(*out_cur,g));
          //command cross over to other CPUs is okay. Find out what Cpu the command target is on
          if (x != NULL) { targetCpu = x->cpu; }
        }
      }  
    } 
  }
  found = false;
  for (out_cur = out_begin; out_cur != out_end; ++out_cur)
  {   
    if (g[target(*out_cur,g)].np == NULL) std::cerr << g[target(*out_cur,g)].name << " is UNDEFINED" << std::endl;
    else {

      if (!(g[target(*out_cur,g)].np->isMeta()) && g[*out_cur].type == sFD) {
        if (found) {std::cerr << "!!! Found more than one flow destination !!!" << std::endl; break;
        } else {
          auto* x = at.lookupVertex(target(*out_cur,g));
          // Flow Destination must be in the same memory the command target is in
          if (x != NULL && x->cpu == targetCpu) {
            ret.push_back(at.adr2intAdr(x->cpu, x->adr));
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
  boost::tie(in_begin, in_end) = in_edges(v,g);
  vp = source(*in_begin,g);
  
  
  boost::tie(out_begin, out_end) = out_edges(vp,g);
  
  //search parent blocks default destination
  
  found = false;
  for (out_cur = out_begin; out_cur != out_end; ++out_cur)
  {   
    if (g[target(*out_cur,g)].np == NULL) std::cerr << g[target(*out_cur,g)].name << " is UNDEFINED" << std::endl;
    else {

      if (!(g[target(*out_cur,g)].np->isMeta()) && g[*out_cur].type == sDD) {
        if (found) {std::cerr << "!!! Found more than one default destination !!!" << std::endl; break;
        } else {
          auto* x = at.lookupVertex(target(*out_cur,g));
          // Destination MUST NOT lie outside own memory! (well, technically, it'd work, but it'd be race condition galore ...)
          if (x != NULL && x->cpu == cpu) {
            ret.push_back(at.adr2intAdr(x->cpu, x->adr));
            found = true;
          }
        }
      }
    }  
  }
  if (!(found)) ret.push_back(LM32_NULL_PTR);

  //search parent blocks alternative destinations
  found = false;
  for (out_cur = out_begin; out_cur != out_end; ++out_cur)
  {   
    if (g[target(*out_cur,g)].np == NULL) std::cerr << g[target(*out_cur,g)].name << " is UNDEFINED" << std::endl;
    else {

      if (!(g[target(*out_cur,g)].np->isMeta()) && g[*out_cur].type == sAD) {
        auto* x = at.lookupVertex(target(*out_cur,g));
        // Destination MUST NOT lie outside own memory! (well, technically, it'd work, but it'd be race condition galore ...)
        if (x != NULL && x->cpu == cpu) {
          ret.push_back(at.adr2intAdr(x->cpu, x->adr));
          found = true;
          //std::cout << "altDst: " << g[target(*out_cur,g)].name << " @ 0x" << std::hex << at.adr2intAdr(cpu, x->adr) << std::endl;
        }
      }
    }  
  }
  if (!(found)) ret.push_back(LM32_NULL_PTR);

  
  return ret;

}