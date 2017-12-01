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




  const ConstellationRule_set cRules = { 
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
    {n::sBlockFixed,  e::sQPrio[PRIO_IL],     {n::sQInfo},                                                                              0, 1  },
    {n::sBlockFixed,  e::sQPrio[PRIO_HI],     {n::sQInfo},                                                                              0, 1  },
    {n::sBlockFixed,  e::sQPrio[PRIO_LO],     {n::sQInfo},                                                                              0, 1  },
    {n::sBlockAlign,  e::sDefDst,     {n::sTMsg, n::sCmdNoop, n::sCmdFlow, n::sCmdFlush, n::sCmdWait, n::sBlockFixed, n::sBlockAlign},  0, 1  },
    {n::sBlockAlign,  e::sAltDst,     {n::sTMsg, n::sCmdNoop, n::sCmdFlow, n::sCmdFlush, n::sCmdWait, n::sBlockFixed, n::sBlockAlign},  0, 10 },
    {n::sBlockAlign,  e::sDstList,    {n::sDstList},                                                                                    0, 1  },
    {n::sBlockAlign,  e::sQPrio[PRIO_IL],     {n::sQInfo},                                                                              0, 1  },
    {n::sBlockAlign,  e::sQPrio[PRIO_HI],     {n::sQInfo},                                                                              0, 1  },
    {n::sBlockAlign,  e::sQPrio[PRIO_LO],     {n::sQInfo},                                                                              0, 1  },
    {n::sQInfo,       e::sMeta,       {n::sQBuf},                                                                                       2, 2  }

  };


  //check if all outedge (nodetype/edgetype/childtype) tupels are valid and occurrence count is within valid bounds
  void neighbourhoodCheck(vertex_t v, Graph& g) {
    Graph::out_edge_iterator out_begin, out_end, out_cur;
    ConstellationCnt_set cCnt;
    std::string exIntro = "Neighbourhood Validation: Node '" + g[v].name + "' of type '" + g[v].type;

    boost::tie(out_begin, out_end) = out_edges(v,g);

    if( (out_begin == out_end) && (g[v].type != n::sBlockFixed) && (g[v].type != n::sBlockAlign) ){ //found an isolated lone node. So far, only a block can exist like this
      throw std::runtime_error(exIntro + "' cannot exist in isolation\n");
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
  void eventSequenceCheck(vertex_t v, Graph& g) {
    Graph::out_edge_iterator out_begin, out_end, out_cur;
    vertex_t vcurrent = v, vnext = v;
    unsigned int infiniteLoopGuard = 0;;
    

    std::string exIntroBase = "Event Sequence Validation: Node '"; 
    std::string exIntro;

    while (infiniteLoopGuard < MaxDepth::EVENT) {
      //find the child connected to this node's defdest
      boost::tie(out_begin, out_end) = out_edges(vcurrent,g);
      for (out_cur = out_begin; out_cur != out_end; ++out_cur) {
        if(g[*out_cur].type == e::sDefDst) vnext = target(*out_cur,g); // there can only be 1 defDst. event nodes with more or less are caught by neighbourhoodcheck
        break;
      }
      exIntro = exIntroBase + g[vcurrent].name + "' of type '" + g[vcurrent].type + "' must not ";
      //check for forbidden properties:
       if ( (g[vnext].type == n::sBlockFixed) || (g[vnext].type == n::sBlockAlign) ) { //found a block
        //check for forbidden properties:

        //assumption: successors are either of class Event or Block (neighbourhoodCheck ensures this)
        if (boost::dynamic_pointer_cast<Event>(g[vcurrent].np)->getTOffs() >= boost::dynamic_pointer_cast<Block>(g[vcurrent].np)->getTPeriod() ) { // time offset greater or equal block period 
          throw std::runtime_error(exIntro  + "have a time offset greater of equal than the period of its terminating block\n");
        } 
        return; // found a valid block, we're done.
      }

      if (vcurrent == vnext) { // self reference of event-class nodes
        throw std::runtime_error(exIntro  + "loop back on itself\n");    
      }
      //assumption: successors are of class events (neighbourhoodCheck and block checks above ensure this)  
      if (boost::dynamic_pointer_cast<Event>(g[vcurrent].np)->getTOffs() > boost::dynamic_pointer_cast<Event>(g[vnext].np)->getTOffs()) { // non monotonically increasing time offsets
        throw std::runtime_error(exIntro  + "have a time offset greater than its successor's\n");   
      }  
      if (vnext == v) { // loop to original vertex without encountering block termination
        throw std::runtime_error(exIntro  + "be part of a loop without a terminating block\n"); 
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
    std::string exIntroBase = "Meta Sequence Validation: Node '" + g[v].name + "' of type '" + g[v].type + "' must not "; 
    std::string exIntro;
    
    try {Aux::metaSequenceCheckAux(v, v, g);} catch (std::runtime_error const& err) {throw std::runtime_error(exIntroBase + std::string(err.what()));} 
  }    

}