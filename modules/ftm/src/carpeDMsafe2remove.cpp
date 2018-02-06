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

bool CarpeDM::isSafeToRemove(Graph& gRem, std::string& report) {
  bool isSafe = true;

  //Find all patterns 2B removed
  for (auto& patternIt : getGraphPatterns(gRem)) {
    isSafe &= isSafeToRemove(patternIt, report);
  }

  return isSafe;
}


bool CarpeDM::isSafeToRemove(const std::string& pattern, std::string& report) {
  Graph& g        = gDown;
  AllocTable& at  = atDown;
  Graph gTmp, gEq;
  vertex_set_t blacklist, entries, cursors;
  
  // BEGIN Preparations: Entry points, Blacklist and working copy of the Graph
  //Init our blacklist of critical nodes. All vertices in the pattern to be removed need to be on it
  for (auto& nodeIt : getPatternMembers(pattern)) {
    if (hm.lookup(nodeIt)) {
      auto x = at.lookupHash(hm.lookup(nodeIt).get());
      if (!(at.isOk(x))) {throw std::runtime_error( "Could not find member node"); return false;}
      blacklist.insert(x->v);
    }
  }
  //Find and list all entry nodes of patterns 2B removed
  std::string sTmp = getPatternEntryNode(pattern);
  if (hm.lookup(sTmp)) {
    auto x = at.lookupHash(hm.lookup(sTmp).get());
    if (!(at.isOk(x))) {throw std::runtime_error( "Could not find entry node"); return false;}
    entries.insert(x->v);
  }
  if(verbose) {sLog << "Pattern <" << pattern << "> (Entrypoint <" << sTmp << "> safe removal analysis" << std::endl;}
  //make a working copy of the download graph
  vertex_map_t vertexMapTmp;
  boost::associative_property_map<vertex_map_t> vertexMapWrapperTmp(vertexMapTmp);
  copy_graph(g, gTmp, boost::orig_to_copy(vertexMapWrapperTmp));
  for (auto& it : vertexMapTmp) { //check vertex indices
    if (it.first != it.second) {throw std::runtime_error( "CpyGraph Map1 Idx Translation failed! This is beyond bad, contact Dev !");}
  }
  
  // End Preparations

  //BEGIN Static equivalent model //
  //add static equivalent edges of all pending flow commands to working copy  
  if (addDynamicDestinations(gTmp, at)) { if(verbose) {sLog << "Added dynamic equivalents." << std::endl;} }

  //Generate a filtered view, stripping all edges except default Destinations, resident flow destinations and dynamic flow destinations
  typedef boost::property_map< Graph, std::string myEdge::* >::type EpMap;
  boost::filtered_graph <Graph, static_eq<EpMap>, boost::keep_all > fg(gTmp, make_static_eq(boost::get(&myEdge::type, gTmp)), boost::keep_all());
  //copy filtered view to normal graph to work with
  vertex_map_t vertexMapEq;
  boost::associative_property_map<vertex_map_t> vertexMapWrapperEq(vertexMapEq);
  copy_graph(fg, gEq, boost::orig_to_copy(vertexMapWrapperEq));
  for (auto& it : vertexMapEq) { //check vertex indices
    if (it.first != it.second) { throw std::runtime_error( "CpyGraph Map2 Idx Translation failed! This is beyond bad, contact Dev !");}
  }
  //try to get consistent image of active cursors
  cursors = getAllCursors(false);
  //Here comes the problem: resident commands are only of consquence if they AND their target Block are executable
  //Iteratively find out which cmds are executable and add equivalent edges for them. Do this until no more new edges have to be added
  if (addResidentDestinations(gEq, gTmp, cursors)) { if(verbose) {sLog << "Added resident equivalents." << std::endl;} }

  // END Static Equivalent Model //

  // Crawl and map active areas
  //crawl all reverse trees we can reach from the given entries and add their nodes to the blacklist
  for (auto& vEntry : entries) {
    if(verbose) { sLog << "Starting Crawler from " << gEq[vEntry].name << std::endl; }
    vertex_set_t tmpTree;
    getReverseNodeTree(vEntry, tmpTree, gEq);
    blacklist.insert(tmpTree.begin(), tmpTree.end());
  }

  //Create Debug Output File
  BOOST_FOREACH( vertex_t v, vertices(gEq) ) { gEq[v].np->clrFlags(NFLG_PAINT_LM32_SMSK); }
  for (auto& it : cursors)    { gEq[it].np->setFlags(NFLG_DEBUG1_SMSK); }
  for (auto& it : entries)    { gEq[it].np->setFlags(NFLG_DEBUG0_SMSK); }
  for (auto& it : blacklist)  { gEq[it].np->setFlags(NFLG_PAINT_HOST_SMSK); }
  report += createDot(gEq, true);

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
        if ((vBlock  == -1) || (vDst == -1)) {throw std::runtime_error( "Could not find block and dst for resident equivalents");}
        
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
 
  //check their Q counters for unprocessed commands
  uint32_t wrIdxs = boost::dynamic_pointer_cast<Block>(g[vQ].np)->getWrIdxs(); 
  uint32_t rdIdxs = boost::dynamic_pointer_cast<Block>(g[vQ].np)->getRdIdxs();
  uint32_t diff = (rdIdxs ^ wrIdxs ) & 0x00ffffff;  
      
  if(verbose) sLog << "Checking Block " << g[vQ].name << std::endl;

  for (uint8_t prio = 0; prio < 3; prio++) {
    uint32_t bufLstAdr;
    uint8_t bufLstCpu;
    AdrType bufLstAdrType; 

    if (!((diff >> (prio*8)) & Q_IDX_MAX_OVF_MSK)) {if(verbose) {sLog << "prio " << (int)prio << " is empty" << std::endl;} continue;}
    if(verbose) sLog << "Checking Prio " << prio << std::endl;  
    //get Block binary
    uint8_t* bBlock = g[vQ].np->getB();
    bufLstAdr = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&bBlock[BLOCK_CMDQ_LO_PTR + prio * _32b_SIZE_]);

    
    std::tie(bufLstCpu, bufLstAdrType) = at.adrClassification(bufLstAdr);  
    //get BufList binary
    auto bufLst = at.lookupAdr( s2u<uint8_t>(g[vQ].cpu), at.adrConv(bufLstAdrType, AdrType::MGMT, s2u<uint8_t>(g[vQ].cpu), bufLstAdr) );
    if (!(at.isOk(bufLst))) {throw std::runtime_error( "Could not find buffer list in download address table");}
    const uint8_t* bBL = bufLst->b;  
     
    //get current read cnt
    uint8_t rdIdx = (rdIdxs >> (prio*8)) & Q_IDX_MAX_MSK;
    uint8_t wrIdx = (wrIdxs >> (prio*8)) & Q_IDX_MAX_MSK;

    if(verbose) sLog << "rdCnt " << (int)rdIdx << " wrCnt " << (int)wrIdx << std::endl;

    //force wraparound
    rdIdx >= wrIdx ? wrIdx+=4 : wrIdx;

    //find buffers of all non empty slots
    for (uint8_t i = rdIdx; i < wrIdx; i++) {
      uint8_t idx = i & Q_IDX_MAX_MSK;
      uint32_t bufAdr, dstAdr;
      uint8_t bufCpu, dstCpu;
      AdrType bufAdrType, dstAdrType;
      
      bufAdr = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&bBL[(idx / 2) * _32b_SIZE_] );
      std::tie(bufCpu, bufAdrType) = at.adrClassification(bufAdr);  

      uint32_t tmpAdr = at.adrConv(bufAdrType, AdrType::MGMT, s2u<uint8_t>(g[vQ].cpu), bufAdr);

      auto buf = at.lookupAdr( s2u<uint8_t>(g[vQ].cpu), tmpAdr );
      if (!(at.isOk(buf))) {throw std::runtime_error( "Could not find buffer in download address table");}
      const uint8_t* b = buf->b;

      if(verbose) sLog << "Scanning Buffer " << (int)(i / 2) << " - " << g[buf->v].name << " at Offset " << (int)(i % 2) << std::endl;
      // scan pending command for flow to forbidden destination
      uint32_t act  = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&b[(idx % 2) * _T_CMD_SIZE_ + T_CMD_ACT]);
      uint8_t type = (act >> ACT_TYPE_POS) & ACT_TYPE_MSK;
      if(verbose) sLog << "Found Cmd type " << (int)type << std::endl; 

      if (type == ACT_TYPE_FLOW) {

        dstAdr   = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&b[(idx % 2) * _T_CMD_SIZE_ + T_CMD_FLOW_DEST]);
        if (dstAdr == LM32_NULL_PTR) continue; // pointing to idle is always okay
        std::tie(dstCpu, dstAdrType) = at.adrClassification(dstAdr);

        auto x = at.lookupAdr( (dstAdrType == AdrType::PEER ? dstCpu : s2u<uint8_t>(g[vQ].cpu)), at.adrConv(dstAdrType, AdrType::MGMT, (dstAdrType == AdrType::PEER ? dstCpu : s2u<uint8_t>(g[vQ].cpu)), dstAdr) );
        if (!(at.isOk(x))) {throw std::runtime_error( "Could not find dst in download address table");}
        if(verbose) sLog << "Found flow dst " << g[x->v].name << std::endl;
        ret.insert(x->v); //found a pending flow, insert its destination
      }
    }  
  }

  return ret;
}      