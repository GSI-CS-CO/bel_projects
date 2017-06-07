#include "visitoruploadcrawler.h"
#include "common.h"
#include "node.h"
#include "block.h"
#include "meta.h"
#include "event.h"

const std::string sNodeType[] = {"prioil", "priohi", "priolo"};
const std::string sQM[] = {"prioil", "priohi", "priolo"};
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
  tmpCT = getCmdTarget();
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
  tmpCT = getCmdTarget();

  vA.reserve( tmpDD.size() + tmpCT.size() ); // preallocate memory
  vA.insert( vA.end(), tmpDD.begin(), tmpDD.end() );
  vA.insert( vA.end(), tmpCT.begin(), tmpCT.end() );

  el.serialise(vA);

}

void VisitorUploadCrawler::visit(const Noop& el) const {
  vAdr vA, tmpDD, tmpCT;
  tmpDD = getDefDst();
  tmpCT = getCmdTarget();

  vA.reserve( tmpDD.size() + tmpCT.size() ); // preallocate memory
  vA.insert( vA.end(), tmpDD.begin(), tmpDD.end() );
  vA.insert( vA.end(), tmpCT.begin(), tmpCT.end() );

  el.serialise(vA);

}

void VisitorUploadCrawler::visit(const Wait& el) const {
  vAdr vA, tmpDD, tmpCT;
  tmpDD = getDefDst();
  tmpCT = getCmdTarget();

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
  vAdr vA, tmpDD, tmpDL;
  tmpDD = getDefDst();
  tmpDL = getListDst();

  vA.reserve( tmpDD.size() + tmpDL.size() ); // preallocate memory
  vA.insert( vA.end(), tmpDD.begin(), tmpDD.end() );
  vA.insert( vA.end(), tmpDL.begin(), tmpDL.end() );

  el.serialise(vA);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private helper functions

 vAdr VisitorUploadCrawler::getDefDst() const {
    bool found = false;
    Graph& g = mmu.getUpGraph();
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
            auto* x = mmu.lookupName(g[target(*out_cur,g)].name);
            if (x != NULL) {
              ret.push_back(mmu.adr2intAdr(x->adr));
              found = true; 
              //std::cout << "defDst: " << g[target(*out_cur,g)].name << " @ 0x" << std::hex << x->adr << std::endl;
            }
          }
        }
      }  
    }
    if (!(found)) ret.push_back(LM32_NULL_PTR);

    return ret;
  }

  vAdr VisitorUploadCrawler::getDynSrc() const {
    Graph& g = mmu.getUpGraph();
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
            auto* x = mmu.lookupName(g[target(*out_cur,g)].name);
            if (x != NULL) { aId = mmu.adr2extAdr(x->adr); g[v].np->setFlags(NFLG_TMSG_DYN_ID_SMSK);}
          }
        }
        if (g[*out_cur].type == sDPAR0) {
          if (aPar0 != LM32_NULL_PTR) {std::cerr << "!!! Found more than one dynamic par0 source !!!" << std::endl; break;
          } else {
            auto* x = mmu.lookupName(g[target(*out_cur,g)].name);
            if (x != NULL) { aPar0 = mmu.adr2extAdr(x->adr); g[v].np->setFlags(NFLG_TMSG_DYN_PAR0_SMSK);}             
          }
        }
        if (g[*out_cur].type == sDPAR1) {
          if (aPar1 != LM32_NULL_PTR) {std::cerr << "!!! Found more than one dynamic par1 source !!!" << std::endl; break;
          } else {
            auto* x = mmu.lookupName(g[target(*out_cur,g)].name);
            if (x != NULL) { aPar1 = mmu.adr2extAdr(x->adr); g[v].np->setFlags(NFLG_TMSG_DYN_PAR1_SMSK);}
          }
        }
        if (g[*out_cur].type == sDTEF) {
          if (aTef != LM32_NULL_PTR) {std::cerr << "!!! Found more than one dynamic tef source !!!" << std::endl; break;
          } else {
            auto* x = mmu.lookupName(g[target(*out_cur,g)].name);
            if (x != NULL) { aTef = mmu.adr2extAdr(x->adr); g[v].np->setFlags(NFLG_TMSG_DYN_TEF_SMSK);}
          }
        }
        if (g[*out_cur].type == sDRES) {
          if (aRes != LM32_NULL_PTR) {std::cerr << "!!! Found more than one dynamic res source !!!" << std::endl; break;
          } else {
            auto* x = mmu.lookupName(g[target(*out_cur,g)].name);
            if (x != NULL) { aRes = mmu.adr2extAdr(x->adr); g[v].np->setFlags(NFLG_TMSG_DYN_RES_SMSK);}
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
    Graph& g = mmu.getUpGraph();
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
            auto* x = mmu.lookupName(g[target(*out_cur,g)].name);
            if (x != NULL) {
              ret.push_back(mmu.adr2intAdr(x->adr));
              found = true;
              //std::cout << g[target(*out_cur,g)].name << " @ 0x" << std::hex << mmu.adr2intAdr(x->adr) << std::endl;
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
              auto* x = mmu.lookupName(g[target(*out_cur,g)].name);
              if (x != NULL) {
                ret.push_back(mmu.adr2intAdr(x->adr));
                found = true;
                //std::cout << "qMeta: " << g[target(*out_cur,g)].name << " @ 0x" << std::hex << mmu.adr2intAdr(x->adr) << std::endl;
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
  Graph& g = mmu.getUpGraph();
  vAdr ret;
  Graph::out_edge_iterator out_begin, out_end, out_cur;
  boost::tie(out_begin, out_end) = out_edges(v,g);
  
  found = false;
  for (out_cur = out_begin; out_cur != out_end; ++out_cur)
  {   
    if (g[target(*out_cur,g)].np == NULL) std::cerr << g[target(*out_cur,g)].name << " is UNDEFINED" << std::endl;
    else {

      if (g[target(*out_cur,g)].np->isMeta()) {
        auto* x = mmu.lookupName(g[target(*out_cur,g)].name);
        if (x != NULL) {
          ret.push_back(mmu.adr2intAdr(x->adr));
          found = true;
          //std::cout << "qBuf: " <<  g[target(*out_cur,g)].name << " @ 0x" << std::hex << mmu.adr2intAdr(x->adr) << std::endl;
        }
      }
    }  
  }

  if (!(found)) ret.push_back(LM32_NULL_PTR);

  return ret;
}

vAdr VisitorUploadCrawler::getCmdTarget() const {
  bool found;
  Graph& g = mmu.getUpGraph();
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
          auto* x = mmu.lookupName(g[target(*out_cur,g)].name);
          if (x != NULL) {
            ret.push_back(mmu.adr2intAdr(x->adr));
            found = true;
            //std::cout << "Target: " << g[target(*out_cur,g)].name << " @ 0x" << std::hex << mmu.adr2intAdr(x->adr) << std::endl;
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
  Graph& g = mmu.getUpGraph();
  vAdr ret;


  Graph::out_edge_iterator out_begin, out_end, out_cur;
  boost::tie(out_begin, out_end) = out_edges(v,g);
  
  found = false;
  for (out_cur = out_begin; out_cur != out_end; ++out_cur)
  {   
    if (g[target(*out_cur,g)].np == NULL) std::cerr << g[target(*out_cur,g)].name << " is UNDEFINED" << std::endl;
    else {

      if (!(g[target(*out_cur,g)].np->isMeta()) && g[*out_cur].type == sFD) {
        if (found) {std::cerr << "!!! Found more than one flow destination !!!" << std::endl; break;
        } else {
          auto* x = mmu.lookupName(g[target(*out_cur,g)].name);
          if (x != NULL) {
            ret.push_back(mmu.adr2intAdr(x->adr));
            found = true; 
            //std::cout << "flowDst: " << g[target(*out_cur,g)].name << " @ 0x" << std::hex << mmu.adr2intAdr(x->adr) << std::endl;
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
  Graph& g = mmu.getUpGraph();
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
          auto* x = mmu.lookupName(g[target(*out_cur,g)].name);
          if (x != NULL) {
            ret.push_back(mmu.adr2intAdr(x->adr));
            found = true;
            //std::cout << "defDst: " << g[target(*out_cur,g)].name << " @ 0x" << std::hex << mmu.adr2intAdr(x->adr) << std::endl;
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
        auto* x = mmu.lookupName(g[target(*out_cur,g)].name);
        if (x != NULL) {
          ret.push_back(mmu.adr2intAdr(x->adr));
          found = true;
          //std::cout << "altDst: " << g[target(*out_cur,g)].name << " @ 0x" << std::hex << mmu.adr2intAdr(x->adr) << std::endl;
        }
      }
    }  
  }
  if (!(found)) ret.push_back(LM32_NULL_PTR);

  
  return ret;

}