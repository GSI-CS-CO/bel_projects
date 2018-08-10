#include "validation.h"
#include "ftm_common.h"
#include "dotstr.h"
#include "node.h"
#include "block.h"
#include "meta.h"
#include "event.h"


namespace n = DotStr::Node::TypeVal;
namespace e = DotStr::Edge::TypeVal;


namespace Validation {



  const children_t cNonMeta = {n::sTMsg, n::sCmdNoop, n::sCmdFlow, n::sCmdFlush, n::sCmdWait, n::sBlockFixed, n::sBlockAlign};
ConstellationRule_set cRules;



void init() {
        cRules.insert(ConstellationRule(n::sTMsg,        e::sDefDst,     cNonMeta,  1, 1  ));
        cRules.insert(ConstellationRule(n::sTMsg,        e::sDynPar0,    cNonMeta,  0, 1  ));
        cRules.insert(ConstellationRule(n::sTMsg,        e::sDynPar1,    cNonMeta,  0, 1  ));
        cRules.insert(ConstellationRule(n::sCmdNoop,     e::sDefDst,     cNonMeta, 0, 1  ));
        cRules.insert(ConstellationRule(n::sCmdNoop,     e::sCmdTarget,  {n::sBlock, n::sBlockFixed, n::sBlockAlign},  0, 1  ));
        cRules.insert(ConstellationRule(n::sCmdFlow,     e::sDefDst,     cNonMeta, 0, 1  ));
        cRules.insert(ConstellationRule(n::sCmdFlow,     e::sCmdTarget,  {n::sBlock, n::sBlockFixed, n::sBlockAlign},  0, 1  ));
        cRules.insert(ConstellationRule(n::sCmdFlow,     e::sCmdFlowDst, cNonMeta, 0, 1  ));
        cRules.insert(ConstellationRule(n::sCmdFlush,    e::sDefDst,     cNonMeta, 1, 1  ));
        cRules.insert(ConstellationRule(n::sCmdFlush,    e::sCmdTarget,  {n::sBlock, n::sBlockFixed, n::sBlockAlign},  0, 1  ));
        cRules.insert(ConstellationRule(n::sCmdWait,     e::sDefDst,     cNonMeta, 0, 1  ));
        cRules.insert(ConstellationRule(n::sCmdWait,     e::sCmdTarget,  {n::sBlock, n::sBlockFixed, n::sBlockAlign},  0, 1  ));
        cRules.insert(ConstellationRule(n::sBlockFixed,  e::sDefDst,     cNonMeta, 0, 1  ));
        cRules.insert(ConstellationRule(n::sBlockFixed,  e::sAltDst,     cNonMeta, 0, 10 ));
        cRules.insert(ConstellationRule(n::sBlockFixed,  e::sDstList,    {n::sDstList},                                0, 1  ));
        cRules.insert(ConstellationRule(n::sBlockFixed,  e::sQPrio[PRIO_IL],     {n::sQInfo},                          0, 1  ));
        cRules.insert(ConstellationRule(n::sBlockFixed,  e::sQPrio[PRIO_HI],     {n::sQInfo},                          0, 1  ));
        cRules.insert(ConstellationRule(n::sBlockFixed,  e::sQPrio[PRIO_LO],     {n::sQInfo},                          0, 1  ));
        cRules.insert(ConstellationRule(n::sBlockAlign,  e::sDefDst,     cNonMeta,  0, 1  ));
        cRules.insert(ConstellationRule(n::sBlockAlign,  e::sAltDst,     cNonMeta,  0, 10 ));
        cRules.insert(ConstellationRule(n::sBlockAlign,  e::sDstList,    {n::sDstList},                                0, 1  ));
        cRules.insert(ConstellationRule(n::sBlockAlign,  e::sQPrio[PRIO_IL],     {n::sQInfo},                          0, 1  ));
        cRules.insert(ConstellationRule(n::sBlockAlign,  e::sQPrio[PRIO_HI],     {n::sQInfo},                          0, 1  ));
        cRules.insert(ConstellationRule(n::sBlockAlign,  e::sQPrio[PRIO_LO],     {n::sQInfo},                          0, 1  ));
        cRules.insert(ConstellationRule(n::sQInfo,       e::sMeta,       {n::sQBuf},                                   2, 2  ));

  }

  //check if all outedge (nodetype/edgetype/childtype) tupels are valid and occurrence count is within valid bounds
  void neighbourhoodCheck(vertex_t v, Graph& g) {
    Graph::out_edge_iterator out_begin, out_end, out_cur, out_chk;
    Graph::in_edge_iterator in_begin, in_end;
    ConstellationCnt_set cCnt;
    std::string exIntro = "Neighbourhood: Node '" + g[v].name + "' of type '" + g[v].type;

    if (g[v].np == nullptr) throw std::runtime_error(exIntro + "' was found unallocated\n");

    boost::tie(out_begin, out_end) = out_edges(v,g);
    if( (out_begin == out_end) && ( g[v].np->isEvent() || (g[v].type == n::sQInfo) )) { //found a childless node. Events and certain meta nodes cannot exist like this
      throw std::runtime_error(exIntro + "' cannot be childless\n");
    }

    boost::tie(in_begin, in_end) = in_edges(v,g);
    if( (in_begin == in_end) && g[v].np->isMeta() ) { //found an orphan node. meta nodes cannot exist like this
      throw std::runtime_error(exIntro + "' cannot be an orphan\n");
    }

    //Check connection duplicates
    for (out_cur = out_begin; out_cur != out_end; ++out_cur) {
      vertex_t vChk = target(*out_cur,g);
      edgeType_t et = g[*out_cur].type;
      unsigned cnt = 0;
      for (out_chk = out_begin; out_chk != out_end; ++out_chk) {
        if ( (vChk == target(*out_chk,g)) && (et == g[*out_chk].type) ) { cnt++;}
      }
      if (cnt > 1) throw std::runtime_error(exIntro + "' must not have multiple edges of type '" + et + "' to Node '" + g[vChk].name + "' of type '" + g[vChk].type + "'\n");
    }

    for (out_cur = out_begin; out_cur != out_end; ++out_cur) {
      auto it = cRules.get<Constellation>().find(boost::make_tuple(g[v].type, g[*out_cur].type));
      // if node type/edge type combo is not found, this is already invalid
      if (it == cRules.end()) {
        throw std::runtime_error(exIntro + "' must not have edge of type '" + g[*out_cur].type + "'\n");
      }
      //if necessary, add the counter for this constellation
      auto itAux = cCnt.insert(ConstellationCnt(g[v].type, g[*out_cur].type));
      auto itCnt = itAux.first; //returns iterator it, bool insertsuccess
      if (itCnt == cCnt.end() ) throw std::runtime_error(exIntro + " has malfunctioning constellation counter - possible problem with a boost multiindex container\n");
      cCnt.modify(itCnt, [](ConstellationCnt& e){e.cnt++;});

      // if the child's type is not in the set of allowable children, this is invalid
      if (it->children.count(g[target(*out_cur,g)].type) < 1) {
        throw std::runtime_error(exIntro + "' with edge of type '" + g[*out_cur].type + " must not have children of type '" + g[target(*out_cur,g)].type + "'\n");
      }
    }
    //check all exisiting constellation counts against rule min/max
    for(auto itCntChk : cCnt )  {
      auto itRules   = cRules.get<Constellation>().find(boost::make_tuple(itCntChk.parent, itCntChk.edge));


      std::string possibleChildren;
      for(auto itPCh : itRules->children) possibleChildren += (itPCh + ", ");

      if((itCntChk.cnt < itRules->min) | (itCntChk.cnt > itRules->max)) {
        throw std::runtime_error(exIntro + "' must must have between "
          + std::to_string(itRules->min) + " and " + std::to_string(itRules->max) + " edge(s) of type '" + itRules->edge
          + "'' connected to children of type(s) '" + possibleChildren + "', found " + std::to_string(itCntChk.cnt) + "\n");
      }
    }
  }

  //check if event sequence is well behaved
  void eventSequenceCheck(vertex_t v, Graph& g, bool force) {
    Graph::out_edge_iterator out_begin, out_end, out_cur;
    vertex_t vcurrent = v, vnext = v;
    unsigned int infiniteLoopGuard = 0;;


    std::string exIntroBase = "Event Sequence: Node '";
    std::string exIntro;

    while (infiniteLoopGuard < MaxDepth::EVENT) {
      //find the child connected to this node's defdest
      boost::tie(out_begin, out_end) = out_edges(vcurrent,g);
      for (out_cur = out_begin; out_cur != out_end; ++out_cur) {
        //std::cout << g[vcurrent].name << " child " << g[target(*out_cur,g)].name << " connected by " << g[*out_cur].type << std::endl;
        if(g[*out_cur].type == e::sDefDst) {
          vnext = target(*out_cur,g);
          break;
        }   // there can only be 1 defDst. event nodes with more or less are caught by neighbourhoodcheck

      }
      exIntro = exIntroBase + g[vcurrent].name + "' of type '" + g[vcurrent].type + "' must not ";
      //check for forbidden properties:
       if ( (g[vnext].type == n::sBlockFixed) || (g[vnext].type == n::sBlockAlign) ) { //found a block
        //check for forbidden properties:

        if (g[vcurrent].np == nullptr) throw std::runtime_error(g[vcurrent].name  + " is not allocated\n");
        if (g[vnext].np == nullptr) throw std::runtime_error(g[vnext].name  + " is not allocated\n");
        if (!force) { // this allows negative values for time offsets to provoke specific late events
          //assumption: successors are either of class Event or Block (neighbourhoodCheck ensures this)
          if (boost::dynamic_pointer_cast<Event>(g[vcurrent].np)->getTOffs() >= boost::dynamic_pointer_cast<Block>(g[vnext].np)->getTPeriod() ) { // time offset greater or equal block period
            throw std::runtime_error(exIntro  + "have a time offset greater of equal than the period of its terminating block\n");
          }
        }
        return; // found a valid block, we're done.
      }

      if (vcurrent == vnext) { // self reference of event-class nodes
        throw std::runtime_error(exIntro  + "loop back on itself\n");
      }
      //assumption: successors are of class events (neighbourhoodCheck and block checks above ensure this)
      if (vnext == v) { // loop to original vertex without encountering block termination
        throw std::runtime_error(exIntro  + "be part of a loop without a terminating block\n");
      }
      if (!force) { // this allows negative values for time offsets to provoke specific late events
        if (boost::dynamic_pointer_cast<Event>(g[vcurrent].np)->getTOffs() > boost::dynamic_pointer_cast<Event>(g[vnext].np)->getTOffs()) { // non monotonically increasing time offsets
          throw std::runtime_error(exIntro  + "have a time offset greater than its successor's\n");
        }
      }

      vcurrent = vnext;
      infiniteLoopGuard++;
    }
    throw std::runtime_error(exIntroBase + g[v].name + "' of type '" + g[v].type + "' is probably part of an infinite loop ( iteration cnt > " + std::to_string(MaxDepth::EVENT) + ")\n");

    //useless, but eases my mind.
    return;
  }

  namespace Aux {
    void metaSequenceCheckAux(vertex_t v, vertex_t vcurrent, Graph& g, unsigned int recursionLvl /*= 0*/) {
      Graph::out_edge_iterator out_begin, out_end, out_cur;
      vertex_t vnext;

      if (recursionLvl +1 > MaxDepth::META) throw std::runtime_error("have more than " + std::to_string(MaxDepth::META) + " levels of children\n"); // too many child levels
      boost::tie(out_begin, out_end) = out_edges(vcurrent,g);
      for (out_cur = out_begin; out_cur != out_end; ++out_cur) {
        vnext = target(*out_cur,g);
        if (vnext == vcurrent || vnext == v)  {throw std::runtime_error("be part of a loop\n");} // loop to original vertex
        metaSequenceCheckAux(v, vnext, g, recursionLvl +1);
      }

      return;

    }
  }

  void metaSequenceCheck(vertex_t v, Graph& g) {
    std::string exIntroBase = "Meta Sequence: Node '" + g[v].name + "' of type '" + g[v].type + "' must not ";
    std::string exIntro;

    try {Aux::metaSequenceCheckAux(v, v, g);} catch (std::runtime_error const& err) {throw std::runtime_error(exIntroBase + std::string(err.what()));}
  }

}
