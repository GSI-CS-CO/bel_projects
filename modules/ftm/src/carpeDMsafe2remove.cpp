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
#include "propwrite.h"

#include "node.h"
#include "block.h"
#include "dotstr.h"

namespace dnt = DotStr::Node::TypeVal;
namespace det = DotStr::Edge::TypeVal;

namespace isSafeToRemove {
  const std::string exIntro = "isSafeToRemove: ";
}


bool CarpeDM::isSafeToRemove(Graph& gRem, std::string& report) {
  std::set<std::string> patterns;
  //Find all patterns 2B removed
  for (auto& patternIt : getGraphPatterns(gRem)) { patterns.insert(patternIt); }
    
  return isSafeToRemove(patterns, report);
}

bool CarpeDM::isSafeToRemove(const std::string& pattern, std::string& report) {
  std::set<std::string> p = {pattern};
  return isSafeToRemove(p, report);
}  


bool CarpeDM::isSafeToRemove(std::set<std::string> patterns, std::string& report) {
  Graph& g        = gDown;
  AllocTable& at  = atDown;
  Graph gTmp, gEq;
  vertex_set_t blacklist, entries, cursors;

  //if(verbose) {sLog << "Pattern <" << pattern << "> (Entrypoint <" << sTmp << "> safe removal analysis" << std::endl;}

  for (auto& patternIt : patterns) {
    // BEGIN Preparations: Entry points, Blacklist and working copy of the Graph
    //Init our blacklist of critical nodes. All vertices in the pattern to be removed need to be on it
    for (auto& nodeIt : getPatternMembers(patternIt)) {
      if (hm.lookup(nodeIt)) {
        auto x = at.lookupHash(hm.lookup(nodeIt).get());
        if (!(at.isOk(x))) {throw std::runtime_error(isSafeToRemove::exIntro +  "Could not find pattern <" + patternIt + "> member node <" + nodeIt + ">"); return false;}
        blacklist.insert(x->v);
      }
    }
    //Find and list all entry nodes of patterns 2B removed
    std::string sTmp = getPatternEntryNode(patternIt);
    if (hm.lookup(sTmp)) {
      auto x = at.lookupHash(hm.lookup(sTmp).get());
      if (!(at.isOk(x))) {throw std::runtime_error(isSafeToRemove::exIntro + "Could not find pattern <" + patternIt + "> entry node <" + sTmp + ">"); return false;}
      entries.insert(x->v);
    }
  

  }

  //make a working copy of the download graph
  vertex_map_t vertexMapTmp;
  boost::associative_property_map<vertex_map_t> vertexMapWrapperTmp(vertexMapTmp);
  copy_graph(g, gTmp, boost::orig_to_copy(vertexMapWrapperTmp));
  for (auto& it : vertexMapTmp) { //check vertex indices
    if (it.first != it.second) {throw std::runtime_error(isSafeToRemove::exIntro +  "CpyGraph Map1 Idx Translation failed! This is beyond bad, contact Dev !");}
  }
  
  // End Preparations

  //BEGIN Basic Static equivalent model //
  //add static equivalent edges of all pending flow commands to working copy  
  if (addDynamicDestinations(gTmp, at)) { if(verbose) {sLog << "Added dynamic equivalents." << std::endl;} }

  if(verbose) sLog << "Generating filtered graph view " << std::endl;


  //Generate a filtered view, stripping all edges except default Destinations, resident flow destinations and dynamic flow destinations
  typedef boost::property_map< Graph, std::string myEdge::* >::type EpMap;
  boost::filtered_graph <Graph, static_eq<EpMap>, boost::keep_all > fg(gTmp, make_static_eq(boost::get(&myEdge::type, gTmp)), boost::keep_all());
  //copy filtered view to normal graph to work with
  vertex_map_t vertexMapEq;
  boost::associative_property_map<vertex_map_t> vertexMapWrapperEq(vertexMapEq);
  copy_graph(fg, gEq, boost::orig_to_copy(vertexMapWrapperEq));
  for (auto& it : vertexMapEq) { //check vertex indices
    if (it.first != it.second) { throw std::runtime_error(isSafeToRemove::exIntro + "CpyGraph Map2 Idx Translation failed! This is beyond bad, contact Dev !");}
  }

  if(verbose) sLog << "Reading Cursors " << std::endl;
  //try to get consistent image of active cursors
  updateModTime();
  cursors = getAllCursors(true);
  //Here comes the problem: resident commands are only of consquence if they AND their target Block are active
  //Iteratively find out which cmds are executable and add equivalent edges for them. Do this until no more new edges have to be added
  if (addResidentDestinations(gEq, gTmp, cursors)) { if(verbose) {sLog << "Added resident equivalents." << std::endl;} }

  // END Basic Static Equivalent Model //

  // BEGIN Optimised Static Equivalent Model
  // Under certain conditions, (offending) default destinations can be replaced. These are:
  // 1. The block has Queue(s)
  // 2. The Queue(s) contains at least one permanent flow to a different destination than the blocks current default
  // 3. The default destination will never be called again. This is the case if:
  // 3.1 The permanent flow is pending
  // 3.2 The permanent flow is valid; its valid time is in the past compared to the modtime read before the cursor read
  // 3.3 Queue always overrides default until permanent flow sets new default. This happens if:
  // 3.3.1  The permanent flow is the next due element over all queues (pole position)
  // 3.3.2  The permanent flow is preceded by an unbroken streak of flows over all queues (all pending and valid)
  // 3.3.2.1  Any other type of command before the permanent flow signifies a broken streak
  // 3.3.2.2  Flushes are considered to have no effect except being a break in the streak

  if (updateStaleDefaultDestinations(gEq, at)) { if(verbose) {sLog << "Updated stale Default Destinations to reduce wait time." << std::endl;} }
  

  // END Optimised Static Equivalent Model

  // Crawl and map active areas
  //crawl all reverse trees we can reach from the given entries and add their nodes to the blacklist
  for (auto& vEntry : entries) {
    if(verbose) { sLog << "Starting Crawler from " << gEq[vEntry].name << std::endl; }
    vertex_set_t tmpTree;
    getReverseNodeTree(vEntry, tmpTree, gEq);
    blacklist.insert(tmpTree.begin(), tmpTree.end());
  }
  if(verbose) sLog << "Creating report " << std::endl;
  //Create Debug Output File
  BOOST_FOREACH( vertex_t v, vertices(gEq) ) { gEq[v].np->clrFlags(NFLG_PAINT_LM32_SMSK); }
  for (auto& it : cursors)    { gEq[it].np->setFlags(NFLG_DEBUG1_SMSK); }
  for (auto& it : entries)    { gEq[it].np->setFlags(NFLG_DEBUG0_SMSK); }
  for (auto& it : blacklist)  { gEq[it].np->setFlags(NFLG_PAINT_HOST_SMSK); }
  report += createDot(gEq, true);

  if(verbose) sLog << "Judging safety " << std::endl;
  //calculate intersection of cursors and blacklist. If the intersection set is empty, all nodes in pattern can be safely removed
  vertex_set_t si;
  set_intersection(blacklist.begin(),blacklist.end(),cursors.begin(),cursors.end(), std::inserter(si,si.begin()));
  



  return ( 0 == si.size() );
}


//recursively inserts all vertex idxs of the tree reachable (via in edges) from start vertex into the referenced set
void CarpeDM::getReverseNodeTree(vertex_t v, vertex_set_t& sV, Graph& g) {

  Graph::in_edge_iterator in_begin, in_end, in_cur;
  //Do the crawl       
  boost::tie(in_begin, in_end) = in_edges(v,g);
  for (in_cur = in_begin; in_cur != in_end; ++in_cur) {
    if (sV.find(source(*in_cur, g)) != sV.end()) break;
    sV.insert(source(*in_cur, g));
    //sLog << "Adding Tree Node " << g[source(*in_cur, g)].name << std::endl;
    getReverseNodeTree(source(*in_cur, g), sV, g);
  }
}



bool CarpeDM::addResidentDestinations(Graph& gEq, Graph& gOrig, vertex_set_t cursors) {
  vertex_set_t resCmds; // prepare the set of flow commands to speed things up
  BOOST_FOREACH( vertex_t vChkResCmd, vertices(gEq) ) {if (gEq[vChkResCmd].type == dnt::sCmdFlow) resCmds.insert(vChkResCmd);}
  bool addEdge = (resCmds.size() > 0);
  bool didWork = false;

  while (addEdge) {
    addEdge = false;
    for(auto& vRc : resCmds) {
      vertex_set_t tmpTree, si;
      vertex_t vBlock = -1, vDst = -1; 
      Graph::out_edge_iterator out_begin, out_end, out_cur;
      bool found = false;

      //find out if there is a path from any of the cursors to this command
      getReverseNodeTree(vRc, tmpTree, gEq);
      set_intersection(tmpTree.begin(),tmpTree.end(),cursors.begin(),cursors.end(), std::inserter(si,si.begin()));
      if ( si.size() > 0 ) {
        //found a path. now check if there already is an equivalent edge between this command's target block and its destination
        //get block and dst
        
        //We now intentionally use the unfiltered graph again (to have target and dst edges). works cause vertex indices are equal.
        boost::tie(out_begin, out_end) = out_edges(vRc, gOrig);
        for (out_cur = out_begin; out_cur != out_end; ++out_cur) {
          if(gOrig[*out_cur].type == det::sCmdTarget)  {vBlock  = target(*out_cur, gOrig);}
          if(gOrig[*out_cur].type == det::sCmdFlowDst) {vDst    = target(*out_cur, gOrig);}
        }
        if (((signed)vBlock  == -1) || ((signed)vDst == -1)) {throw std::runtime_error(isSafeToRemove::exIntro + "Could not find block and dst for resident equivalents");}
        
         //check for equivalent resident edges
        boost::tie(out_begin, out_end) = out_edges(vBlock, gEq);
        for (out_cur = out_begin; out_cur != out_end; ++out_cur) { if(gEq[*out_cur].type == det::sResFlowDst)  found = true;}
        if (!found) {
          if (verbose) { sLog << "Adding ResFlowAuxEdge: " << gEq[vBlock].name << " -> " << gEq[vDst].name << std::endl; }
          boost::add_edge(vBlock, vDst, myEdge(det::sResFlowDst), gEq);
          addEdge = true;
          didWork = true;
        }
      }
    }  
  }
  return didWork;
}

bool CarpeDM::updateStaleDefaultDestinations(Graph& g, AllocTable& at) {
  bool didWork = false;
  edge_t edgeToDelete;

  BOOST_FOREACH( vertex_t vChkBlock, vertices(g) ) {
    //first, find blocks
    if(g[vChkBlock].np->isBlock()) {
      //second, inspect their queues and see if default dest is made stale by a dominant flow
      vertex_set_t sVflowDst = getDominantFlowDst(vChkBlock, g, at);
      for (auto& it : sVflowDst) {
        
        if (sVflowDst.size() > 1) {throw std::runtime_error(isSafeToRemove::exIntro + "updateStaleDefDst: found more than one dominant flow, must be 0..1");}
        if(it != -1) { boost::add_edge(vChkBlock, it, myEdge(det::sDefDst), g); }
        else { if (verbose)  sLog << "updateStaleDefDst: New default would be idle, skipping edge creation" << std::endl; }
        //find old default edge and mark for deletion
        Graph::out_edge_iterator out_begin, out_end, out_cur;
        boost::tie(out_begin, out_end) = out_edges(vChkBlock, g);
        for (out_cur = out_begin; out_cur != out_end; ++out_cur) { 
          if(g[*out_cur].type == det::sDefDst) {
            if (verbose) sLog << "updateStaleDefDst: Found old default dst <" << g[target(*out_cur, g)].name << "> of block <" << g[vChkBlock].name << std::endl;
            didWork = true;
            edgeToDelete = *out_cur;
          } 
        }

      }
    }
  }
  if (didWork) boost::remove_edge(edgeToDelete, g);
  return didWork;
}

vertex_set_t CarpeDM::getDominantFlowDst(vertex_t vQ, Graph& g, AllocTable& at) {
  vertex_set_t ret;

  if(verbose) sLog << "Searching for dominant flows " << g[vQ].name << std::endl;

  QueueReport qr;
  qr = getQReport(g, at, g[vQ].name, qr);
        
  for (int8_t prio = PRIO_IL; prio >= PRIO_LO; prio--) {
    if (!qr.hasQ[prio]) {continue;} // if the priority doesn't exist, Ignore

    for (uint8_t i, idx = qr.aQ[prio].rdIdx; idx < qr.aQ[prio].rdIdx + 4; idx++) {
      i = idx & Q_IDX_MAX_MSK;
      QueueElement& qe = qr.aQ[prio].aQe[i];

      // we're going through in order. If element has a valid time in the future (> modTime), stop right here. it and all following are possibly yet unprocessed
      if(qe.validTime > modTime) {return ret;} 

      if (qe.type != ACT_TYPE_FLOW) { 
        //if the command is not a flow, we can stop here: It means the default will be used at least once, thus there is no dominant flow
        return ret;
      }  
      // if flow is not pending, it can't be dominant. Ignore
      if (!qe.pending) {continue;} 
      //found a pending flow to idle, insert bogus vertex index to show that.
      if (qe.flowDst == DotStr::Node::Special::sIdle) {ret.insert(-1); if(verbose) sLog << "updateStaleDefDst: Found dominant flow dst idle" << std::endl; continue;} 
      // we ruled out that the flow leads to idle. If it's not permanent, it can't be dominant. Ignore
      if (!qe.flowPerma) {continue;} 
      //found a dominant flow, insert its destination
      auto x = at.lookupHash(hm.lookup(qe.flowDst).get());
      if (!(at.isOk(x))) {throw std::runtime_error(isSafeToRemove::exIntro + "updateStaleDefDst: Could not find dst in download allocation table");}
      if(verbose) sLog << "updateStaleDefDst: Found dominant flow dst " << g[x->v].name << std::endl;
      ret.insert(x->v); 
      
    }
  }    
  return ret;
}      



bool CarpeDM::addDynamicDestinations(Graph& g, AllocTable& at) {
  bool didWork = false;

  BOOST_FOREACH( vertex_t vChkBlock, vertices(g) ) {
    //first, find blocks
    if(g[vChkBlock].np->isBlock()) {
      //second, inspect their queues and add equivalent edges for pending flows
      vertex_set_t sVflowDst = getDynamicDestinations(vChkBlock, g, at);
      for (auto& it : sVflowDst) {
        if(verbose) {sLog << "Adding DynFlowAuxEdge: " << g[vChkBlock].name << " -> " << g[it].name << std::endl;}
        boost::add_edge(vChkBlock, it, myEdge(det::sDynFlowDst), g);
        didWork = true;
      }
    }
  }
  return didWork;
}


vertex_set_t CarpeDM::getDynamicDestinations(vertex_t vQ, Graph& g, AllocTable& at) {


  vertex_set_t ret;

  if(verbose) sLog << "Searching for pending flows " << g[vQ].name << std::endl;

  QueueReport qr;
  qr = getQReport(g, at, g[vQ].name, qr);
        
  for (int8_t prio = PRIO_IL; prio >= PRIO_LO; prio--) {
    if (!qr.hasQ[prio]) {continue;}

    //find buffers of all non empty slots
    for (uint8_t i, idx = qr.aQ[prio].rdIdx; idx < qr.aQ[prio].rdIdx + 4; idx++) {
      i = idx & Q_IDX_MAX_MSK;
      QueueElement& qe = qr.aQ[prio].aQe[i];

      if (!qe.pending) {continue;}
      if (qe.type == ACT_TYPE_FLOW) {
        if (qe.flowDst == DotStr::Node::Special::sIdle) {continue;}
        auto x = at.lookupHash(hm.lookup(qe.flowDst).get());
        if (!(at.isOk(x))) {throw std::runtime_error(isSafeToRemove::exIntro + "Could not find dst in download allocation table");}
        if(verbose) sLog << "Found flow dst " << g[x->v].name << std::endl;
        ret.insert(x->v); //found a pending flow, insert its destination
      }
    }  
  }

  return ret;
}      