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
#include "carpeDMimpl.h"
#include "propwrite.h"

#include "node.h"
#include "block.h"
#include "dotstr.h"

namespace dnt = DotStr::Node::TypeVal;
namespace det = DotStr::Edge::TypeVal;
typedef boost::property_map< Graph, std::string myEdge::* >::type EpMap;

namespace isSafeToRemove {
  const std::string exIntro = "isSafeToRemove: ";
}

/** Check that it is safe to remove the graph gRem from current schedule.
 * Get all pattern of gRem and call isSafeToRemove for a set of pattern.
 * Warn on stderr if there are undefined pattern.
 */
bool CarpeDM::CarpeDMimpl::isSafeToRemove(Graph& gRem, std::string& report, std::vector<QueueReport>& vQr, CovenantTable& ctAdditions ) {
  std::set<std::string> patterns;
  bool warnUndefined = false;
  //Find all patterns to be removed
  for (auto& patternIt : getGraphPatterns(gRem)) {
    if (patternIt == DotStr::Misc::sUndefined) warnUndefined = true;
    else patterns.insert(patternIt);
  }
  if (warnUndefined) log<ERROR>("Warning: isSafeToRemove was handed a graph containing nodes claiming to belong to pattern '%1%'. These nodes will be ignored for analysis and removal!") % DotStr::Misc::sUndefined.c_str();
  return isSafeToRemove(patterns, report, vQr, ctAdditions);
}

/** Check that it is safe to remove a pattern from current schedule.
 */
bool CarpeDM::CarpeDMimpl::isSafeToRemove(const std::string& pattern, std::string& report, std::vector<QueueReport>& vQr, CovenantTable& ctAdditions) {
  std::set<std::string> p = {pattern};
  return isSafeToRemove(p, report, vQr, ctAdditions);
}

/** Check that it is safe to remove a set of patterns from current schedule.
 */
bool CarpeDM::CarpeDMimpl::isSafeToRemove(std::set<std::string> patterns, std::string& report, std::vector<QueueReport>& vQr, CovenantTable& ctAdditions ) {
  

  bool isSafe = true, allCovenantsUncritical = true;
  Graph& g        = gDown;
  AllocTable& at  = atDown;
  CovenantTable ctAux; //Global CovenantTable is called ct
  Graph gTmp, gEq;
  vertex_set_t blacklist, remlist, entries, cursors, covenants; //hashes of all covenants
  vertex_set_map_t covenantsPerVertex;
  std::string optmisedAnalysisReport, covenantReport;
  uint64_t currentTime = getDmWrTime();


  for (auto& patternIt : patterns) {
    // BEGIN Preparations: Entry points, Blacklist and working copy of the Graph
    //Init our blacklist of critical nodes. All vertices in the pattern to be removed need to be on it
    const std::string& exMsgMemberNode = isSafeToRemove::exIntro +  "Could not find pattern <" + patternIt + "> member node ";
    for (auto& nodeIt : getPatternMembers(patternIt)) {
      auto x = at.lookupHash(hm.lookup(nodeIt, exMsgMemberNode), exMsgMemberNode + "! ");
      remlist.insert(x->v);
      covenantsPerVertex[x->v].insert(null_vertex);
    }
    //Find and list all entry nodes of patterns 2B removed
    std::string sTmp = getPatternEntryNode(patternIt);
    const std::string& exMsgEntryNode = isSafeToRemove::exIntro +  "Could not find pattern <" + patternIt + "> entry node";
    auto x = at.lookupHash(hm.lookup(sTmp, exMsgEntryNode), exMsgEntryNode);
    entries.insert(x->v);

    log<VERBOSE>("Pattern <%1%> Entrypoint <%2%>safe removal analysis") % patternIt.c_str() % sTmp.c_str();
  }

  blacklist = remlist;

  //make a working copy of the download graph
  vertex_map_t vertexMapTmp;
  mycopy_graph<Graph>(g, gTmp, vertexMapTmp);

  for (auto& it : vertexMapTmp) { //check vertex indices
    if (it.first != it.second) {throw std::runtime_error(isSafeToRemove::exIntro +  "CpyGraph Map1 Idx Translation failed! This is beyond bad, contact Dev !");}
  }

  // End Preparations

  //BEGIN Basic Static equivalent model //
  //add static equivalent edges of all pending flow commands to working copy

  //FIXME shouldn't this also be iteratively done ???
  if (addDynamicDestinations(gTmp, at)) { log<VERBOSE>("Added dynamic equivalents.");} 

  log<VERBOSE>("Generating filtered graph view ");


  //Generate a filtered view, stripping all edges except default Destinations, resident flow destinations and dynamic flow destinations

  boost::filtered_graph <Graph, static_eq<EpMap>, boost::keep_all > fg(gTmp, make_static_eq(boost::get(&myEdge::type, gTmp)), boost::keep_all());
  //copy filtered view to normal graph to work with
  vertex_map_t vertexMapEq;
  mycopy_graph<boost::filtered_graph <Graph, static_eq<EpMap>, boost::keep_all >>(fg, gEq, vertexMapEq);
  for (auto& it : vertexMapEq) { //check vertex indices
    if (it.first != it.second) { throw std::runtime_error(isSafeToRemove::exIntro + "CpyGraph Map2 Idx Translation failed! This is beyond bad, contact Dev !");}
  }

  log<VERBOSE>("Reading Cursors ");
  //try to get consistent image of active cursors
  updateModTime();
  cursors = getAllCursors(!testmode); // Set to false for debugging system behaviour with static cursors

  //Here comes the problem: resident commands are only of consquence if they AND their target Block are active
  //Iteratively find out which cmds are executable and add equivalent edges for them. Do this until no more new edges have to be added
  if (addResidentDestinations(gEq, gTmp, cursors)) { log<VERBOSE>("Added resident equivalents.");}

  // END Basic Static Equivalent Model //

  // BEGIN Optimised Static Equivalent Model
  // Under certain conditions, (offending) default destinations can be replaced
  if (optimisedS2R) {
    log<VERBOSE>("Starting Optimiser (Update stale defDst)");
    if (updateStaleDefaultDestinations(gEq, at, ctAux, optmisedAnalysisReport)) { log<VERBOSE>("Updated stale Default Destinations to reduce wait time.");} 
  }

  // END Optimised Static Equivalent Model




  //covenant: promise not to clear/reorder a given block's queues

  // Crawl and map active areas
  // crawl all reverse trees we can reach from the given elements to be removed (used to be just the entry ndoes,
  // but that doesn't suffice for resident commands pointing into patterns to be removed) and add their nodes to the blacklist
  for (auto& vRem : remlist) {
    log<VERBOSE>("Starting Crawler from %1%") % gEq[vRem].name.c_str();
    vertex_set_t tmpTree;
    getReverseNodeTree(vRem, tmpTree, gEq, covenantsPerVertex, null_vertex, 1, 0); //start with non traversible limit of 1, count 0
    blacklist.insert(tmpTree.begin(), tmpTree.end());
  }
  log<VERBOSE>("Blacklist complete");


  log<VERBOSE>("Judging safety ");
  //calculate intersection of cursors and blacklist. If the intersection set is empty, all nodes in pattern can be safely removed
  vertex_set_t si;
  set_intersection(blacklist.begin(),blacklist.end(),cursors.begin(),cursors.end(), std::inserter(si,si.begin()));


  for (auto& it : blacklist)  {
    log<VERBOSE>("%1% --> {") % gEq[it].name.c_str();

    for (auto& itPv : covenantsPerVertex[it]) { log<VERBOSE>("%1%, ") % ((itPv != null_vertex) ? gEq[itPv].name.c_str() : "NULL"); }
    //log<VERBOSE>("\n");
  }

  //create set of all covenants which must be honoured so the prediction will hold. Because of the propagation along reverse trees, doing it for intersection members is sufficient
  for (auto& it : si)  {
    covenants.insert(covenantsPerVertex[it].begin(), covenantsPerVertex[it].end());
    log<VERBOSE>("%1% --> {") % gEq[it].name.c_str();
    for (auto& itPv : covenantsPerVertex[it]) { log<VERBOSE>("%1%, ") % ((itPv != null_vertex) ? gEq[itPv].name.c_str() : "NULL"); }
    //log<VERBOSE>("\n");
  }


  isSafe = !isSafetyCritical(covenants); // if a safety critical node (cov set contains NO_COVENANT) is on the intersection with cursor set, it's unsafe

  //Find all orphaned commands for later treatment
  //orphaned command check means: check all queues for flow commands with a destination node inside the pattern 2B removed

  if (isSafe) {
    vStrC chkNames;
    log<VERBOSE>("Checking for orphaned flow commands checks against following nodes: ");
    for (vertex_t vChk : remlist ) {
      log<VERBOSE>("%1%") % g[vChk].name.c_str();
      chkNames.push_back(g[vChk].name);
    }

    BOOST_FOREACH( vertex_t vBlock, vertices(g) ) {
      if (g[vBlock].np->isBlock()) {
        log<VERBOSE>("Checking for orphaned commands at block %1%") % g[vBlock].name.c_str();
        // for each inactive block, get qeue reports to check flow destination against all entry points we want removed
        // all flows pointing to an orphan or future orphan will be marked.
        QueueReport qr;
        getQReport(g, at, g[vBlock].name, qr, chkNames);
        vQr.push_back(qr);
      }
    }
  }


  log<VERBOSE>("Creating report ");
  //Create Debug Output File

  BOOST_FOREACH( vertex_t v, vertices(gEq) ) { gEq[v].np->clrFlags(NFLG_PAINT_LM32_SMSK); }
  for (auto& it : cursors)    { gEq[it].np->setFlags(NFLG_DEBUG1_SMSK); }
  for (auto& it : entries)    { gEq[it].np->setFlags(NFLG_DEBUG0_SMSK); }
  for (auto& it : blacklist)  {
    if(isSafetyCritical(covenantsPerVertex[it])) gEq[it].np->setFlags(NFLG_PAINT_HOST_SMSK);
  }

  report += createDot(gEq, true);
  report += optmisedAnalysisReport;

  if (optimisedS2R && isSafe) {
    covenantReport += "//Covenants to honour:\n";
    for (auto& it : covenants)  {
      allCovenantsUncritical &= !isSafetyCritical(covenantsPerVertex[it]);
      //std::cout << "Was optimised" << std::endl;
      if (it == null_vertex) {covenantReport += "//Null\n"; continue;}
      std::string covName = gEq[it].name;
      //find covname in ctAux and copy found entry to ct watchlist
      auto x = ctAux.lookup(covName);
      if (!ctAux.isOk(x)) { throw std::runtime_error(isSafeToRemove::exIntro + ": Lookup of <" + covName + "> in covenantAuxTable failed\n");}
      ctAdditions.insert(x);
      //double check if item exists
      auto y = ctAdditions.lookup(covName);
      if (!ctAdditions.isOk(y)) { throw std::runtime_error(isSafeToRemove::exIntro + ": Lookup of <" + covName + "> in covenantTable failed\n");}

      //and report
      covenantReport += "//" + covName + " p " + std::to_string(y->prio) + " s " + std::to_string(y->slot) + " chk 0x" + std::to_string(y->chkSum) + "\n";
    }
    report += covenantReport;

  }

  if (allCovenantsUncritical == false) {
    throw std::runtime_error(isSafeToRemove::exIntro + " ERROR in algorithm detected: a block listed as a covenant was safety critical itself\n\n" + report);
  }
  report += "\n//Created: " + nsTimeToDate(currentTime);
  report += "//Patterns to judge:\n";
  for (auto s : patterns) {report += "//  " + s + "\n";};
  report += "//Verdict: ";
  report += isSafe ? "SAFE\n" : "FORBIDDEN\n";

  return isSafe;
}

bool CarpeDM::CarpeDMimpl::isOptimisableEdge(edge_t e, Graph& g) {

  vertex_t toBeChecked = target(e, g);
  Graph::in_edge_iterator in_begin, in_end, in_cur;
  boost::tie(in_begin, in_end) = in_edges(toBeChecked,g);
  for (in_cur = in_begin; in_cur != in_end; ++in_cur) {
    //it's only optimisable if there are no other types of connection to the source of  the inedge we are to check!
    if ( (source(*in_cur, g) == source(e, g)) && (g[*in_cur].type != det::sBadDefDst)) return false;
  }
  return true;
}

bool CarpeDM::CarpeDMimpl::isCovenantPending(const std::string& covName) {

  cmI x = ct.lookup(covName);
  if (!ct.isOk(x)) {
  return false;} //throw std::runtime_error(isSafeToRemove::exIntro + ": Lookup of <" + covName + "> in covenantTable failed\n");
  return isCovenantPending(x);
}

bool CarpeDM::CarpeDMimpl::isCovenantPending(cmI cov) {
  QueueReport qr;
  getQReport(cov->name, qr);
  QueueElement& qe = qr.aQ[cov->prio].aQe[cov->slot];

  if (cov->chkSum == ct.genChecksum(qe))  return true;
  else                                    return false;
}

unsigned CarpeDM::CarpeDMimpl::updateCovenants() {

  unsigned cnt = 0;
  vStrC toDelete;
  for (cmI it = ct.getTable().begin(); it != ct.getTable().end(); it++ ) {
    if (!isCovenantPending(it)) {
      log<VERBOSE>("Covenant %1%  complete, removing from table") % it->name.c_str();
      toDelete.push_back(it->name);
    }
    cnt++;
  }

  for (auto it : toDelete) { ct.remove(it); }

  return cnt;
}

void CarpeDM::CarpeDMimpl::addCovenants(CovenantTable& ctAdditions) {
  for (cmI it = ctAdditions.getTable().begin(); it != ctAdditions.getTable().end(); it++ ) {
    ct.insert(it);
  }
}


bool CarpeDM::CarpeDMimpl::isSafetyCritical(vertex_set_t& c) {
  if (c.find(null_vertex) != c.end()) return true;
  else return false;
}

//FIXME Recursive function, valgrind this for cycle cost!
//recursively inserts all vertex idxs of the tree reachable (via in edges) from start vertex into the referenced set
void CarpeDM::CarpeDMimpl::getReverseNodeTree(vertex_t v, vertex_set_t& sV, Graph& g, vertex_set_map_t& covenantsPerVertex, vertex_t covenant, int32_t maxNtEdges, int32_t tNtEdges) {
  vertex_t nextCovenant;
  Graph::in_edge_iterator in_begin, in_end, in_cur;
  //Do the crawl
  boost::tie(in_begin, in_end) = in_edges(v,g);
  for (in_cur = in_begin; in_cur != in_end; ++in_cur) {
    log<VERBOSE>("%1% <-- %2% -- %3%  propcov %4%") % g[target(*in_cur, g)].name.c_str() % g[*in_cur].type.c_str() % g[source(*in_cur, g)].name.c_str() % ((covenant == null_vertex) ? "NULL" : g[covenant].name.c_str());
    vertex_set_t& cpvs = covenantsPerVertex[source(*in_cur, g)];


    if ( (g[*in_cur].type != det::sDefDst && g[*in_cur].type != det::sResFlowDst && g[*in_cur].type != det::sDynFlowDst) ) { // if its a non-traversiable edge and the traversal limit is reached, don't follow this trail
      if ( (maxNtEdges > 0) && (tNtEdges >= maxNtEdges) ) {
        log<VERBOSE>("Non-Traversible edge limit reached, not crossing %1% -> %2%") % g[source(*in_cur, g)].name.c_str() % g[target(*in_cur, g)].name.c_str();
        continue;
      } else {tNtEdges++;}
    }

    sV.insert(source(*in_cur, g));

    if (cpvs.find(covenant) != cpvs.end()) { continue; }

    if (isOptimisableEdge(*in_cur, g)) {
      log<VERBOSE>("Optimisable:  %1% -> %2%") % g[source(*in_cur, g)].name.c_str() % g[target(*in_cur, g)].name.c_str();
      nextCovenant = source(*in_cur, g);
    } else {
      nextCovenant = covenant;
    }

    cpvs.insert(nextCovenant);
    getReverseNodeTree(source(*in_cur, g), sV, g, covenantsPerVertex, nextCovenant, maxNtEdges, tNtEdges);



  }
}

bool CarpeDM::CarpeDMimpl::addResidentDestinations(Graph& gEq, Graph& gOrig, vertex_set_t cursors) {
  vertex_set_t resCmds; // prepare the set of flow commands to speed things up
  vertex_set_map_t dummy; // this doesn't need to look out for covenant sets, ignore

  //FIXME what about flush with override ???
  BOOST_FOREACH( vertex_t vChkResCmd, vertices(gEq) ) {if ((gEq[vChkResCmd].type == dnt::sCmdFlow) || (gEq[vChkResCmd].type == dnt::sSwitch)) resCmds.insert(vChkResCmd);}
  bool addEdge = (resCmds.size() > 0);
  bool didWork = false;

  while (addEdge) {
    addEdge = false;
    for(auto& vRc : resCmds) {
      vertex_set_t tmpTree, si;
      vertex_t vBlock = null_vertex, vDst = null_vertex;
      Graph::out_edge_iterator out_begin, out_end, out_cur;
      bool found = false;

      //find out if there is a path from any of the cursors to this command
      getReverseNodeTree(vRc, tmpTree, gEq, dummy);
      set_intersection(tmpTree.begin(),tmpTree.end(),cursors.begin(),cursors.end(), std::inserter(si,si.begin()));
      if ( si.size() > 0 ) {
        //found a path. now check if there already is an equivalent edge between this command's target block and its destination
        //get block and dst
        log<VERBOSE>("Path from cursor to command <%1%> found.") % gEq[vRc].name.c_str();
        //We now intentionally use the unfiltered graph again (to have target and dst edges). works cause vertex indices are equal.
        boost::tie(out_begin, out_end) = out_edges(vRc, gOrig);
        for (out_cur = out_begin; out_cur != out_end; ++out_cur) {
          if(gOrig[*out_cur].type == det::sCmdTarget || gOrig[*out_cur].type == det::sSwitchTarget)   {vBlock  = target(*out_cur, gOrig);}
          if(gOrig[*out_cur].type == det::sCmdFlowDst || gOrig[*out_cur].type == det::sSwitchDst)     {vDst    = target(*out_cur, gOrig);}
        }
        if ((vBlock  == null_vertex) || (vDst == null_vertex)) {throw std::runtime_error(isSafeToRemove::exIntro + "Could not find block and dst for resident equivalents when scanning <" + gOrig[vRc].name + ">");}

         //check for equivalent resident edges
        boost::tie(out_begin, out_end) = out_edges(vBlock, gEq);
        for (out_cur = out_begin; out_cur != out_end; ++out_cur) { if(gEq[*out_cur].type == det::sResFlowDst)  found = true;}
        if (!found) {
          log<VERBOSE>("Adding ResFlowAuxEdge: %1% -> %2%") % gEq[vBlock].name.c_str() % gEq[vDst].name.c_str();
          boost::add_edge(vBlock, vDst, myEdge(det::sResFlowDst), gEq);
          addEdge = true;
          didWork = true;
        }
      }
    }
  }
  return didWork;
}

bool CarpeDM::CarpeDMimpl::updateStaleDefaultDestinations(Graph& g, AllocTable& at, CovenantTable& covTab, std::string& qAnalysis) {
  bool didWork = false;
  edge_t edgeToDelete;

  BOOST_FOREACH( vertex_t vChkBlock, vertices(g) ) {
    //first, find blocks
    if(g[vChkBlock].np->isBlock()) {
      //second, inspect their queues and see if default dest is made stale by a dominant flow
      vertex_set_t sVflowDst = getDominantFlowDst(vChkBlock, g, at, covTab, qAnalysis);
      //log<VERBOSE>("\n");
      for (auto& it : sVflowDst) {

        //if (sVflowDst.size() > 1) {throw std::runtime_error(isSafeToRemove::exIntro + "updateStaleDefDst: found more than one dominant flow, must be 0..1");}
        if(it != null_vertex) { boost::add_edge(vChkBlock, it, myEdge(det::sDomFlowDst), g); log<VERBOSE>("updateStaleDefDst: Adding edge to %1%") % g[it].name.c_str(); }
        else { log<VERBOSE>("updateStaleDefDst: New default would be idle, skipping edge creation");}
        //find old default edge and mark for deletion
        Graph::out_edge_iterator out_begin, out_end, out_cur;
        boost::tie(out_begin, out_end) = out_edges(vChkBlock, g);
        for (out_cur = out_begin; out_cur != out_end; ++out_cur) {
          if(g[*out_cur].type == det::sDefDst) {
            log<VERBOSE>("updateStaleDefDst: Found old default dst <%1%> of block <%2%>, changing type to non traversible") % g[target(*out_cur, g)].name.c_str() % g[vChkBlock].name.c_str();
            didWork = true;
            g[*out_cur].type = det::sBadDefDst;
          }
        }
      }
    }
  }

  return didWork;
}

vertex_set_t CarpeDM::CarpeDMimpl::getDominantFlowDst(vertex_t vQ, Graph& g, AllocTable& at, CovenantTable& covTab, std::string& qAnalysis) {
  vertex_set_t ret;

  //FIXME what about flush with override ???

  qAnalysis += "//" + g[vQ].name;

  QueueReport qr;
  vStrC fo;
  qr = getQReport(g, at, g[vQ].name, qr, fo);

  for (int8_t prio = PRIO_IL; prio >= PRIO_LO; prio--) {
    qAnalysis += "#P" + std::to_string((int)prio);
    if (!qr.hasQ[prio]) {qAnalysis += "->xX->xX->xX->xX"; continue;} // if the priority doesn't exist, Ignore

    for (uint8_t i, idx = qr.aQ[prio].rdIdx; idx < qr.aQ[prio].rdIdx + 4; idx++) {
      i = idx & Q_IDX_MAX_MSK;
      QueueElement& qe = qr.aQ[prio].aQe[i];

      // if flow at read idx is not pending, this queue is empty.
      if (!qe.pending) {qAnalysis +="->eE"; continue;}

      // we're going through in order. If element has a valid time in the future (> modTime), stop right here. it and all following are possibly yet unprocessed
      if(qe.validTime > modTime) {qAnalysis += "->v" + std::to_string((int)qe.type) + "\n";
        std::stringstream auxstream;
        auxstream << "//tVal 0x" << std::setfill('0') << std::setw(10) << std::hex << qe.validTime << " tMod 0x" << std::setfill('0') << std::setw(10) << std::hex << modTime << std::endl;
        qAnalysis += auxstream.str();
        return ret;
      }

      if (qe.type != ACT_TYPE_FLOW) {
        qAnalysis += "->t" + std::to_string((int)qe.type) + "\n";
        //if the command is not a flow, we can stop here: It means the default will be used at least once, thus there is no dominant flow
        return ret;
      }

      //found a pending flow to idle, insert bogus vertex index to show that.
      if (qe.flowDst == DotStr::Node::Special::sIdle) {
        ret.insert(null_vertex);
        qAnalysis += "->i" + std::to_string((int)qe.type) + "\n";
        log<VERBOSE>("updateStaleDefDst: Found dominant flow dst idle");
        return ret;
      }
      // we ruled out that the flow leads to idle. If it's not permanent, it can't be dominant. Ignore
      if (!qe.flowPerma) {qAnalysis +=  "->p" + std::to_string((int)qe.type); continue;}
      //found a dominant flow, insert its destination
      auto x = at.lookupHash(hm.lookup(qe.flowDst, isSafeToRemove::exIntro + "updateStaleDefDst: unknown dst"), isSafeToRemove::exIntro + "updateStaleDefDst: unknown dst");
      log<VERBOSE>("updateStaleDefDst: Found dominant flow dst %1%") % g[x->v].name.c_str();
      ret.insert(x->v);
      covTab.insert(g[vQ].name, (uint8_t)prio, i, qe); //save which element in which queue of which block is eligible to save our arse
      qAnalysis +=  "->D" + std::to_string((int)qe.type);
    }
  }
  qAnalysis += "\n";
  return ret;
}



bool CarpeDM::CarpeDMimpl::addDynamicDestinations(Graph& g, AllocTable& at) {
  bool didWork = false;

  BOOST_FOREACH( vertex_t vChkBlock, vertices(g) ) {
    //first, find blocks
    if(g[vChkBlock].np->isBlock()) {
      //second, inspect their queues and add equivalent edges for pending flows
      vertex_set_t sVflowDst = getDynamicDestinations(vChkBlock, g, at);
      for (auto& it : sVflowDst) {
        log<VERBOSE>("Adding DynFlowAuxEdge: %1% -> %2%") % g[vChkBlock].name.c_str() % g[it].name.c_str();
        boost::add_edge(vChkBlock, it, myEdge(det::sDynFlowDst), g);
        didWork = true;
      }
    }
  }
  return didWork;
}


vertex_set_t CarpeDM::CarpeDMimpl::getDynamicDestinations(vertex_t vQ, Graph& g, AllocTable& at) {


  vertex_set_t ret;

  log<VERBOSE>("Searching for pending flows %1%") % g[vQ].name.c_str();

  QueueReport qr;
  vStrC fo;
  qr = getQReport(g, at, g[vQ].name, qr, fo);

  for (int8_t prio = PRIO_IL; prio >= PRIO_LO; prio--) {
    if (!qr.hasQ[prio]) {continue;}

    //find buffers of all non empty slots
    for (uint8_t i, idx = qr.aQ[prio].rdIdx; idx < qr.aQ[prio].rdIdx + 4; idx++) {
      i = idx & Q_IDX_MAX_MSK;
      QueueElement& qe = qr.aQ[prio].aQe[i];

      if (!qe.pending || qe.qty == 0) {continue;} // if command is not pending or has no charges left, it is ignored.
      if (qe.type == ACT_TYPE_FLOW) {
        if (qe.flowDst == DotStr::Node::Special::sIdle) {continue;}
        auto x = at.lookupHash(hm.lookup(qe.flowDst, isSafeToRemove::exIntro + "dyn flow dst not found"), isSafeToRemove::exIntro + "dyn flow dst not found");
        log<VERBOSE>("Found flow dst %1%") % g[x->v].name.c_str();
        ret.insert(x->v); //found a pending flow, insert its destination
      }
    }
  }

  return ret;
}
