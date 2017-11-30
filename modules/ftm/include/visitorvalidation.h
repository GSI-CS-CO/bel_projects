#include "visitoruploadcrawler.h"
#include "common.h"
#include "node.h"
#include "block.h"
#include "meta.h"
#include "event.h"
#include "dotstr.h"


namespace n = DotStr::Node::TypeVal;
namespace e = DotStr::Edge::TypeVal;

typedef int edgeType_t;
typedef int nodeType_t;


using boost::multi_index_container;
using namespace boost::multi_index;


struct Constellation{};
struct MinOccurrence
{
  bool operator()(unsigned int x,const ConstellationRule& e1)const{return e1.min >= x;}
  bool operator()(const ConstellationRule& e2,unsigned int x)const{return e2.min >= x;}
};


struct ConstellationRule {
  nodeType_t parent;
  edgeType_t edge;
  std::set<nodeType_t> children;
  unsigned int min;
  unsigned int max;

  ConstellationRule(nodeType_t parent, uint32_t edge, nodeType_t child, unsigned int min, unsigned int max) : parent(parent), edge(edge), child(child), min(min), max(max) {}
};

typedef boost::multi_index_container<
  ConstellationRule,
  indexed_by<
    hashed_unique<
      tag<Constellation>,
      composite_key<
        ConstellationRule,
        BOOST_MULTI_INDEX_MEMBER(ConstellationRule,nodeType_t,parent),
        BOOST_MULTI_INDEX_MEMBER(ConstellationRule,edgeType_t,edge)
      >
    >    
    ordered_non_unique <
        tag<Groups::Beamproc>,
        BOOST_MULTI_INDEX_MEMBER(ConstellationRule, unsigned int, min)> 
      >
    >
  >    
 > ConstellationRule_set;

 struct ConstellationCnt {
  nodeType_t parent;
  edgeType_t edge;
  unsigned int cnt;
  
  ConstellationCnt(nodeType_t parent, uint32_t edge, unsigned int cnt) : parent(parent), edge(edge), cnt(0) {}
};


 typedef boost::multi_index_container<
  ConstellationCnt,
  indexed_by<
    hashed_unique<
      tag<Constellation>,
      composite_key<
        ConstellationCnt,
        BOOST_MULTI_INDEX_MEMBER(ConstellationCnt,nodeType_t,parent),
        BOOST_MULTI_INDEX_MEMBER(ConstellationCnt,edgeType_t,edge)
      >
    >
  >    
 > ConstellationCnt_set;


extern const ConstellationRule_set cRules; 

const ConstellationRule_set validConstellations = { 
  {n::sTMsg,        e::sDefDst,     {n::sTMsg, n::sCmdNoop, n::sCmdFlow, n::sCmdFlush, n::sCmdWait, n::sBlockFixed, n::sBlockAlign},  1, 1  },
  {n::sTMsg,        e::sDynPar0,    {n::sBlockFixed, n::sBlockAlign },                                                                0, 1  },
  {n::sTMsg,        e::sDynPar1,    {n::sBlockFixed, n::sBlockAlign },                                                                0, 1  },
  {n::sCmdNoop,     e::sDefDst,     {n::sTMsg,  n::sCmdNoop, n::sCmdFlow, n::sCmdFlush, n::sCmdWait, n::sBlockFixed, n::sBlockAlign}, 0, 1  },
  {n::sCmdNoop,     e::sCmdTarget,  {n::sBlock, n::sBlockFixed, n::sBlockAlign},                                                      1, 1  },
  {n::sCmdFlow,     e::sDefDst,     {n::sTMsg,  n::sCmdNoop, n::sCmdFlow, n::sCmdFlush, n::sCmdWait, n::sBlockFixed, n::sBlockAlign}, 0, 1  },
  {n::sCmdFlow,     e::sCmdTarget,  {n::sBlock, n::sBlockFixed, n::sBlockAlign},                                                      1, 1  },
  {n::sCmdFlow,     e::sCmdFlowDst, {n::sBlock, n::sBlockFixed, n::sBlockAlign},                                                      1, 1  },
  {n::sCmdFlush,    e::sDefDst,     {n::sTMsg,  n::sCmdNoop, n::sCmdFlow, n::sCmdFlush, n::sCmdWait, n::sBlockFixed, n::sBlockAlign}, 1, 1  },
  {n::sCmdFlush,    e::sCmdTarget,  {n::sBlock, n::sBlockFixed, n::sBlockAlign},                                                      1, 1  },
  {n::sCmdWait,     e::sDefDst,     {n::sTMsg,  n::sCmdNoop, n::sCmdFlow, n::sCmdFlush, n::sCmdWait, n::sBlockFixed, n::sBlockAlign}, 0, 1  },
  {n::sCmdWait,     e::sCmdTarget,  {n::sBlock, n::sBlockFixed, n::sBlockAlign},                                                      1, 1  },
  {n::sBlockFixed,  e::sDefDst,     {n::sTMsg,  n::sCmdNoop, n::sCmdFlow, n::sCmdFlush, n::sCmdWait, n::sBlockFixed, n::sBlockAlign}, 0, 1  },
  {n::sBlockFixed,  e::sAltDst,     {n::sTMsg,  n::sCmdNoop, n::sCmdFlow, n::sCmdFlush, n::sCmdWait, n::sBlockFixed, n::sBlockAlign}, 0, 10 },
  {n::sBlockFixed,  e::sDstList,    {n::sDstList},                                                                                    0, 1  },
  {n::sBlockFixed,  e::sPrioHi,     {n::sQInfo},                                                                                      0, 1  },
  {n::sBlockFixed,  e::sPrioMd,     {n::sQInfo},                                                                                      0, 1  },
  {n::sBlockFixed,  e::sPrioLo,     {n::sQInfo},                                                                                      0, 1  },
  {n::sBlockAlign,  e::sDefDst,     {n::sTMsg, n::sCmdNoop, n::sCmdFlow, n::sCmdFlush, n::sCmdWait, n::sBlockFixed, n::sBlockAlign},  0, 1  },
  {n::sBlockAlign,  e::sAltDst,     {n::sTMsg, n::sCmdNoop, n::sCmdFlow, n::sCmdFlush, n::sCmdWait, n::sBlockFixed, n::sBlockAlign},  0, 10 },
  {n::sBlockAlign,  e::sDstList,    {n::sDstList},                                                                                    0, 1  },
  {n::sBlockAlign,  e::sPrioHi,     {n::sQInfo},                                                                                      0, 1  },
  {n::sBlockAlign,  e::sPrioMd,     {n::sQInfo},                                                                                      0, 1  },
  {n::sBlockAlign,  e::sPrioLo,     {n::sQInfo},                                                                                      0, 1  },
  {n::sBlockAlign,  e::sPrioLo,     {n::sQInfo},                                                                                      0, 1  },
  {n::sQInfo,       e::sMeta,       {n::sQBuf},                                                                                       2, 2  }
}


//check if all outedge (nodetype/edgetype/childtype) tupels are valid and occurrence count is within valid bounds
void VisitorValidation::neighbourhoodCheck(vertex_t v, Graph& g) const {
  Graph::out_edge_iterator out_begin, out_end, out_cur;
  ConstellationCnt_set cCnt;
  std::string exIntro = "Neighbourhood Validation: Node '" + g[v].name + "' of type '" + g[v].type;

  boost::tie(out_begin, out_end) = out_edges(v,g);

  if( (out_cur == out_end) && (g[v].type != n::sBlockFixed) && (g[v].type != n::sBlockAlign) ){ //found an isolated lone node. So far, only a block can exist like this
    throw std::runtime_error(exIntro + "' with edge of type '" + g[*out_cur].type + " must not have children of type '" + g[target(*out_cur,g)].type + "'\n");
  }

  for (out_cur = out_begin; out_cur != out_end; ++out_cur) { 
    auto it = cRules<Constellation>.get().find(g[v].type, g[*out_cur].type);
    // if node type/edge type combo is not found, this is already invalid
    if (it == cRules.end()) {
      throw std::runtime_error(exIntro + "' may not have edge of type '" + g[*out_cur].type + "'\n");
    }
    //if necessary, add the counter for this constellation
    boost::tie(auto itCnt, bool newCntCreated) = cCnt.insert({g[v].type, g[*out_cur].type});
    if (itCnt == cCnt.end() ) throw std::runtime_error(exIntro + " has malfunctioning constellation counter - possible problem with a boost multiindex container\n"); 
    cCnt.modify(itCnt, [](ConstellationCnt& e){e.cnt++;}) 

    // if the child's type is not in the set of allowable children, this is invalid
    if (it->children.count(g[target(*out_cur,g)].type) < 1) {
      throw std::runtime_error(exIntro + "' with edge of type '" + g[*out_cur].type + " must not have children of type '" + g[target(*out_cur,g)].type + "'\n");
    }
  }
  //check all exisiting cnt against rule min/max
  auto const& by_min = cRules.get<MinOccurrence>();

  for(auto itRng : boost::make_iterator_range(by_min.lower_bound(1, MinOccurrence), by_min.end() ) ) {
    auto itCntChk   = cCnt<Constellation>.get().find({itRng->parent, itRng->edge});
    unsigned found  = 0;
    if(itCntChk    != cCnt.end()) {found = itCntChk->cnt;}

    std::string possibleChildren;
    for(auto itPCh : itRng->children) possibleChildren += (*itPCh + ", ");

    if(found < itRng->min | found > itRng->max) {
      throw std::runtime_error(exIntro + "' must must have between " 
        + itRng->min + " and " + itRng->max + "' edge(s) of type '" + itRng->edge 
                            + " connected to children of type(s) '" + possibleChildren
                                                     + "', found' " + found + "\n");
    }  
  }
}  


  

//check if event sequence is well behaved
void VisitorValidation::eventSequenceCheck(const Event& el) const {
  Graph::out_edge_iterator out_begin, out_end, out_cur;
  vertex_t vcurrent = vnext = v;
  int infiniteLoopGuard = infiniteLoopGuardStart = 1000;


  std::string exIntroBase = "Event Sequence Validation: Node '"; 
  std::string exIntro;

  while (infiniteLoopGuard > 0) {
    //find the child connected to this node's defdest
    boost::tie(out_begin, out_end) = out_edges(vcurrent,g);
    for (out_cur = out_begin; out_cur != out_end; ++out_cur) {
      if(g[*out_cur].type == e::sDefDst) vnext = target(*out_cur,g);
      break;
    }
    exIntro = exIntroBase + g[vcurrent].name + "' of type '" + g[vcurrent].type + "' ";
    //check for forbidden properties:
    if (vcurrent == vnext)                        {throw std::runtime_error(exIntro  + "loops back on itself\n");}    // self reference of event-class nodes
    if (g[vcurrent].toffs > g[vcnext].toffs)      {throw std::runtime_error(exIntro  + "must have a time offset less or equal than its successor's\n");}   // non monotonically increasing time offsets
    if (vnext == v)                               {throw std::runtime_error(exIntro  + "is part of loop without a terminating block\n");} // loop to original vertex without encountering block termination
    if ( (g[vnext].type == n::sBlock) 
      || (g[vnext].type == n::sBlockFixed)
      || (g[vnext].type == n::sBlockAlign) )
      { if (g[vcurrent].toff >= g[vnext].tperiod) { throw std::runtime_error(exIntro  + "must have a time offset less than the period of its terminating block\n");} // time offset greater or equal block period
      return; // found a valid block, we're done.
    }
    
    vcurrent = vnext;
    infiniteLoopGuard--;
  }
  throw std::runtime_error(exIntroBase + g[v].name + "' of type '" + g[v].type + "' is probably part of an undetected infinite loop ( iteration cnt > " + std::to_string(infiniteLoopGuardStart) + ")\n"); 

  //useless, but eases my mind.
  return;
}

void VisitorValidation::metaSequenceCheck(const Meta& el) const {
  /*
  Graph::out_edge_iterator out_begin, out_end, out_cur;
  vertex_t vcurrent = vnext = v;
  int infiniteLoopGuard = infiniteLoopGuardStart = 1000;


  std::string exIntroBase = "Meta Sequence Validation: Node '"; 
  std::string exIntro;

  while (infiniteLoopGuard > 0) {
    //find the child connected to this node's defdest
    boost::tie(out_begin, out_end) = out_edges(vcurrent,g);
    for (out_cur = out_begin; out_cur != out_end; ++out_cur) {
      vnext = target(*out_cur,g);
      exIntro = exIntroBase + g[vcurrent].name + "' of type '" + g[vcurrent].type + "' ";
      //check for forbidden properties:
      if (vcurrent == vnext)                        {throw std::runtime_error(exIntro  + "loops back on itself\n");}    // self reference of meta-class nodes
      if (vnext == v)                               {throw std::runtime_error(exIntro  + "is part of loop\n");} // loop to original vertex without encountering block termination
      
    }
    
    
    
    infiniteLoopGuard--;
  }
  throw std::runtime_error(exIntroBase + g[v].name + "' of type '" + g[v].type + "' is probably part of an undetected infinite loop ( iteration cnt > " + std::to_string(infiniteLoopGuardStart) + ")\n"); 

  //useless, but eases my mind.
  return;  
  */
}  



//FIXME Dear future self, the code duplication in here is appalling. Create some proper helper functions for crying out loud !

void VisitorValidation::visit(const Block& el) const {
  neighbourhoodCheck(v, g);
  
}

void VisitorValidation::visit(const TimingMsg& el) const  {
  neighbourhoodCheck(v, g);  
  eventSequenceCheck((Event&)el);}

void VisitorValidation::visit(const Flow& el) const  {
  neighbourhoodCheck(v, g);
  eventSequenceCheck((Event&)el);
}

void VisitorValidation::visit(const Flush& el) const {
  neighbourhoodCheck(v, g);
  eventSequenceCheck((Event&)el);  
}

void VisitorValidation::visit(const Noop& el) const {
  neighbourhoodCheck(v, g);
  eventSequenceCheck((Event&)el);  
}

void VisitorValidation::visit(const Wait& el) const {
  neighbourhoodCheck(v, g);
  eventSequenceCheck((Event&)el);  
}

void VisitorValidation::visit(const CmdQMeta& el) const {
  neighbourhoodCheck(v, g);  
}

void VisitorValidation::visit(const CmdQBuffer& el) const {
  neighbourhoodCheck(v, g);
}

void VisitorValidation::visit(const DestList& el) const {
  neighbourhoodCheck(v, g);
}


  //search parent blocks alternative destinations
  found = false;
  for (out_cur = out_begin; out_cur != out_end; ++out_cur)
  {   
    if (g[target(*out_cur,g)].np == nullptr) std::cerr << g[target(*out_cur,g)].name << " is UNDEFINED" << std::endl;
    else {

      if (!(g[target(*out_cur,g)].np->isMeta()) && g[*out_cur].type == e::sAltDst) {
        auto x = at.lookupVertex(target(*out_cur,g));
        // Destination MUST NOT lie outside own memory! (well, technically, it'd work, but it'd be race condition galore ...)
        if (at.isOk(x)) {
          if (  x->cpu == cpu) {
            ret.push_back(at.adr2intAdr(x->cpu, x->adr));
            found = true;
            //std::cout << "altDst: #" << target(*out_cur,g) << " " << g[target(*out_cur,g)].name << " @ 0x" << std::hex << at.adr2intAdr(cpu, x->adr) << std::endl;
          } else { std::cout << "altDst: #" << target(*out_cur,g) << " " << g[target(*out_cur,g)].name << " @ 0x" << std::hex << at.adr2intAdr(cpu, x->adr) << " expected at CPU" << cpu << ", found on " << (int)x->cpu << "" << std::endl;  at.debug();}
        } else { 
          //std::cerr << "alt destination was found unallocated" << std::endl; 
        }
      }
    }  
  }
  /*
  if (!(found)) { //ret.push_back(LM32_NULL_PTR); 
    std::cout << "No alt dest found" << std::endl; }
  */


  
  return ret;

}
