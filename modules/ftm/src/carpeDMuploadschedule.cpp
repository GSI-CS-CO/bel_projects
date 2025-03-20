#include <boost/shared_ptr.hpp>
#include <algorithm>
#include <stdio.h>
#include <iostream>
#include <string>
#include <inttypes.h>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/copy.hpp>
#include <boost/algorithm/string.hpp>
#include <unistd.h>
#include <limits.h>


#include "common.h"
#include "propwrite.h"
#include "graph.h"
#include "carpeDMimpl.h"
#include "visitoruploadcrawler.h"
#include "visitordownloadcrawler.h"
#include "dotstr.h"
#include "idformat.h"
#include "log.h"

namespace dnp = DotStr::Node::Prop;
namespace dnt = DotStr::Node::TypeVal;
namespace dep = DotStr::Edge::Prop;
namespace det = DotStr::Edge::TypeVal;;
namespace dnm = DotStr::Node::MetaGen;
using namespace DotStr::Misc;


  //TODO NC Traffic Verification


  //TODO CPU Load Balancer
  vEbwrs CarpeDM::CarpeDMimpl::gatherUploadVector(std::set<uint8_t> moddedCpus, uint32_t modCnt, uint8_t opType) {
    //sLog << "Starting Upload address & data vectors" << std::endl;
    vEbwrs ew;
    uint32_t adr, modAdrBase;


    //TODO if this was sorted by CPU, it would be way more efficient!!

    //add all Bmp addresses to return vector
    for(unsigned int i = 0; i < atUp.getMemories().size(); i++) {
      //generate addresses of Bmp's address range
      for (adr = atUp.adrConv(AdrType::MGMT, AdrType::EXT,i, atUp.getMemories()[i].bmpOffs); adr < atUp.adrConv(AdrType::MGMT, AdrType::EXT,i, atUp.getMemories()[i].startOffs); adr += _32b_SIZE_) {
        ew.vcs.push_back(adr == atUp.adrConv(AdrType::MGMT, AdrType::EXT,i, atUp.getMemories()[i].bmpOffs));
        ew.va.push_back(adr);
      }
      //add Bmp to to return vector
      ew.vb += atUp.getMemories()[i].getBmp();
    }



    //add all Nodes to return vector
    for (auto& it : atUp.getTable().get<CpuAdr>()) {
      //generate address range for all nodes staged for upload
      if(it.staged) {
        moddedCpus.insert(it.cpu); // mark cpu as modified if a node is staged
        //Address and cycle start
        for (adr = atUp.adrConv(AdrType::MGMT, AdrType::EXT,it.cpu, it.adr); adr < atUp.adrConv(AdrType::MGMT, AdrType::EXT,it.cpu, it.adr + _MEM_BLOCK_SIZE); adr += _32b_SIZE_ ) {
          ew.vcs.push_back(adr == atUp.adrConv(AdrType::MGMT, AdrType::EXT,it.cpu, it.adr));
          ew.va.push_back(adr);
        }
        //Data
        ew.vb.insert( ew.vb.end(), it.b, it.b + _MEM_BLOCK_SIZE );
      }
    }

    //add all Mgmt Nodes to return vector
    for (auto& it : atUp.getMgmtTable().get<CpuAdr>()) {
      //generate address range for all nodes
        //Address and cycle start
        for (adr = atUp.adrConv(AdrType::MGMT, AdrType::EXT,it.cpu, it.adr); adr < atUp.adrConv(AdrType::MGMT, AdrType::EXT,it.cpu, it.adr + _MEM_BLOCK_SIZE); adr += _32b_SIZE_ ) {
          ew.vcs.push_back(adr == atUp.adrConv(AdrType::MGMT, AdrType::EXT, it.cpu, it.adr));
          ew.va.push_back(adr);
        }
        //Data
        ew.vb.insert( ew.vb.end(), it.b, it.b + _MEM_BLOCK_SIZE );

    }


    // save modification infos
    for (auto& itMod : moddedCpus) { createSchedModInfo(ew, itMod, modCnt, opType); }

    //FIXME This should not be hardcoded to the number of elements and not repeated. Loop it,be clean  

    // save global meta info for management linked list
    uint8_t b[5 * _32b_SIZE_];
    //enough to write it to cpu 0
    modAdrBase = atUp.getMemories()[0].extBaseAdr + atUp.getMemories()[0].sharedOffs + SHCTL_META;
    writeLeNumberToBeBytes<uint32_t>((uint8_t*)&b[T_META_START_PTR], atUp.getMgmtLLstartAdr());
    writeLeNumberToBeBytes<uint32_t>((uint8_t*)&b[T_META_CON_SIZE],  atUp.getMgmtTotalSize());
    writeLeNumberToBeBytes<uint32_t>((uint8_t*)&b[T_META_GRPTAB_SIZE],  atUp.getMgmtGrpSize());
    writeLeNumberToBeBytes<uint32_t>((uint8_t*)&b[T_META_COVTAB_SIZE],  atUp.getMgmtCovSize());
    writeLeNumberToBeBytes<uint32_t>((uint8_t*)&b[T_META_REFTAB_SIZE],  atUp.getMgmtRefSize());
    
    ew.va.push_back(modAdrBase + T_META_START_PTR);
    ew.va.push_back(modAdrBase + T_META_CON_SIZE);
    ew.va.push_back(modAdrBase + T_META_GRPTAB_SIZE);
    ew.va.push_back(modAdrBase + T_META_COVTAB_SIZE);
    ew.va.push_back(modAdrBase + T_META_REFTAB_SIZE);
    ew.vcs += leadingOne(ew.va.size());
    ew.vb.insert( ew.vb.end(), b, b + 5 * _32b_SIZE_ );



    return ew;
  }



  void CarpeDM::CarpeDMimpl::generateDstLst(Graph& g, vertex_t v, unsigned multiDst) {

    const std::string prefix = g[v].name;
    unsigned loops = (multiDst + 1 + DST_MAX -1) / DST_MAX; //add 1 to multidst. its the altdst count, but we need 1 more for a defdst.
    for (unsigned i=0; i<loops;i++) {
      const std::string name = prefix + dnm::sDstListSuffix + "_" + std::to_string(i);
      log<DEBUG_LVL0>(L"generateDstLst: Accomodating %1% + 1 destinations. Loop %2%/%3%, generating %4% ") % multiDst % i % loops % name.c_str();
      hm.add(name);
      vertex_t vD = boost::add_vertex(myVertex(name, g[v].cpu, hm.lookup(name), nullptr, dnt::sDstList, DotStr::Misc::sHexZero), g);
      //FIXME add to grouptable
      g[vD].patName = g[v].patName;
      gt.setPattern(g[vD].name, g[vD].patName, false, false);
      edge_t thisEdge = (boost::add_edge(vD, v, myEdge(det::sDefDst), g)).first;
      log<DEBUG_LVL1>(L"generateDstLst: Adding Edge from %1% to %2%. Setting pattern name to %3%") % g[source(thisEdge, g)].name.c_str() % g[target(thisEdge, g)].name.c_str() % g[vD].patName.c_str();
    }

  }

  void CarpeDM::CarpeDMimpl::generateQmeta(Graph& g, vertex_t v, int prio) {
    //std::cout << "generating " << g[v].name << ", patname " << g[v].patName << " prio " << (int)prio << std::endl;

    const std::string nameBl = g[v].name + dnm::sQBufListTag + dnm::sQPrioPrefix[prio];
    const std::string nameB0 = g[v].name + dnm::sQBufTag     + dnm::sQPrioPrefix[prio] + dnm::s1stQBufSuffix;
    const std::string nameB1 = g[v].name + dnm::sQBufTag     + dnm::sQPrioPrefix[prio] + dnm::s2ndQBufSuffix;
    hm.add(nameBl);
    hm.add(nameB0);
    hm.add(nameB1);

    vertex_t vBl = boost::add_vertex(myVertex(nameBl, g[v].cpu, hm.lookup(nameBl), nullptr, dnt::sQInfo, DotStr::Misc::sHexZero), g);
    vertex_t vB0 = boost::add_vertex(myVertex(nameB0, g[v].cpu, hm.lookup(nameB0), nullptr, dnt::sQBuf,  DotStr::Misc::sHexZero), g);
    vertex_t vB1 = boost::add_vertex(myVertex(nameB1, g[v].cpu, hm.lookup(nameB1), nullptr, dnt::sQBuf,  DotStr::Misc::sHexZero), g);

    g[vBl].patName = g[v].patName;
    gt.setPattern(g[vBl].name, g[vBl].patName, false, false);
    g[vB0].patName = g[v].patName;
    gt.setPattern(g[vB0].name, g[vB0].patName, false, false);
    g[vB1].patName = g[v].patName;
    gt.setPattern(g[vB1].name, g[vB1].patName, false, false);

    boost::add_edge(v,   vBl, myEdge(det::sQPrio[prio]), g);
    boost::add_edge(vBl, vB0, myEdge(det::sMeta),    g);
    boost::add_edge(vBl, vB1, myEdge(det::sMeta),    g);

  }

  /** For the given graph generate the meta vertices for blocks, fixed
   * blocks and aligned blocks. For each block check the attributes and
   * the outgoing edges for the types 'priolo', 'priohi', 'prioil',
   * 'listdst', 'altdst'. If the required meta vertices do not exist,
   * generate these.
   */
  void CarpeDM::CarpeDMimpl::generateBlockMeta(Graph& g, bool doGenerateDstLst) {
   Graph::out_edge_iterator out_begin, out_end, out_cur;

    BOOST_FOREACH( vertex_t v, vertices(g) ) {
      std::string cmp = g[v].type;
      if ((cmp == dnt::sBlockFixed) || (cmp == dnt::sBlockAlign) || (cmp == dnt::sBlock) ) {
        //std::cout << "Scanning Block " << g[v].name << std::endl;
        boost::tie(out_begin, out_end) = out_edges(v,g);
        //check if it already has queue links / Destination List

        bool  genIl, genHi, genLo;
        bool  hasIl=false, hasHi=false, hasLo=false, hasDstLst=false;
        unsigned multiDst=0;
        try{
              genIl = s2u<bool>(g[v].qIl);
              genHi = s2u<bool>(g[v].qHi);
              genLo = s2u<bool>(g[v].qLo);
        } catch (std::runtime_error const& err) {
          throw std::runtime_error( "Parser error when processing node <" + g[v].name + ">. Cause: " + err.what());
        }

        for (out_cur = out_begin; out_cur != out_end; ++out_cur)
        {
          if (g[*out_cur].type == det::sQPrio[PRIO_IL]) hasIl       = true;
          if (g[*out_cur].type == det::sQPrio[PRIO_HI]) hasHi       = true;
          if (g[*out_cur].type == det::sQPrio[PRIO_LO]) hasLo       = true;
          if (g[*out_cur].type == det::sAltDst)         multiDst++;
          if (g[*out_cur].type == det::sDstList)        hasDstLst   = true;
        }
        log<DEBUG_LVL1>(L"generateBlockMeta: Checking Block %1%. MultiDst=%2% hasDstLst=%3%") % g[v].name.c_str() % multiDst % hasDstLst;

        //create requested Queues / Destination List
        if (genIl && !hasIl ) { generateQmeta(g, v, PRIO_IL); }
        if (genHi && !hasHi ) { generateQmeta(g, v, PRIO_HI); }
        if (genLo && !hasLo ) { generateQmeta(g, v, PRIO_LO); }

        if((multiDst || genIl || hasIl || genHi || hasHi || genLo || hasLo) && doGenerateDstLst)   { generateDstLst(g, v, multiDst); }
      }
    }
  }

  void CarpeDM::CarpeDMimpl::updateListDstStaging(vertex_t v) {
    Graph::out_edge_iterator out_begin, out_end, out_cur;
    Graph& g = gUp;
    AllocTable& at = atUp;
    // the changed edge leads to Alternative Dst, find and stage the block's dstList
    boost::tie(out_begin, out_end) = out_edges(v,g);
    for (out_cur = out_begin; out_cur != out_end; ++out_cur) {
      if (g[*out_cur].type == det::sDstList) {
        auto dst = at.lookupVertex(target(*out_cur, g));
        at.setStaged(dst);
        //std::cout << "staged " << g[dst->v].name  << std::endl;
        // if we found a Dst List, stage it
        break;
      }
    }
  }

  void CarpeDM::CarpeDMimpl::updateStaging(vertex_t v, edge_t e)  {
    // staging changes
    Graph& g = gUp;
    AllocTable& at = atUp;

    /*
    if (g[e].type == det::sDefDst || g[e].type == det::sAltDst ) {
      updateListDstStaging(v); // stage source block's Destination List
    }
    */
    if (g[e].type != det::sAltDst ) {
      auto x = at.lookupVertex(v);

      // !!! only stage if there is no covenant for this node, otherwise we run into a race condition:
      // A covenant means the DM will change the default dst to a safe value in the near future.
      // If we'd also change def dst, doing it before DM does has no effect, and doing it after the DM did would overwrite the safe def dst
      if(!isCovenantPending(g[x->v].name)) at.setStaged(x);
      else { log<INFO>(L"updateStaging: Node %1% has an active covenant. Skipping staging to avoid race condition.") % g[v].name.c_str(); }

    }

  }


  void CarpeDM::CarpeDMimpl::mergeUploadDuplicates(vertex_t borg, vertex_t victim) {
    Graph& g = gUp;
    // add all of node 'victim's edges to node 'borg'. Resistance is futile.
    std::vector<edge_t> vEdges2remove;

    // out edges
    Graph::out_edge_iterator out_begin, out_end, out_cur;
    boost::tie(out_begin, out_end) = out_edges(victim, g);
    for (out_cur = out_begin; out_cur != out_end; ++out_cur) {
      vEdges2remove.push_back(*out_cur);
      boost::add_edge(borg, target(*out_cur,g), myEdge(g[*out_cur]), g);
      updateStaging(borg, *out_cur);
    }

    // in egdes
    Graph::in_edge_iterator in_begin, in_end, in_cur;
    boost::tie(in_begin, in_end) = in_edges(victim, g);
    for (in_cur = in_begin; in_cur != in_end; ++in_cur) {
      // if source and target of the edge are equal (edge connects the node to itself),
      // this edge is inserted into vEdges2remove above in the out edges section.
      // removing an edge twice throws an exception in boost.
      if (source(*in_cur, g) != target(*in_cur, g)) {
        vEdges2remove.push_back(*in_cur);
      }

      boost::add_edge(source(*in_cur,g), borg, myEdge(g[*in_cur]), g);
    }

    for (auto eRm : vEdges2remove) {
      boost::remove_edge(eRm, g);
    }
  }

  void CarpeDM::CarpeDMimpl::prepareUpload() {

    std::string cmp;
    uint32_t hash, flags;
    uint8_t cpu;
    int allocState;



    //allocate and init all new vertices
    BOOST_FOREACH( vertex_t v, vertices(gUp) ) {

      myVertex* vt = (myVertex*)&gUp[v];

      //sLog << "Testing " << vt->name << std::endl;

      std::string name = gUp[v].name;
      //try{
        //if (!(hm.lookup(name)))                   {throw std::runtime_error("Node '" + name + "' was unknown to the hashmap"); return;}
        hash = gUp[v].hash;
        try {
        cpu  = s2u<uint8_t>(gUp[v].cpu);
        } catch (std::runtime_error const& err) {
          throw std::runtime_error( "Parser error when processing cpu tags of node <" + name + ">. Cause: " + err.what());
        }
        //add flags for beam process and pattern entry and exit points
        try {
        flags = ((s2u<bool>(gUp[v].bpEntry))  << NFLG_BP_ENTRY_LM32_POS)
              | ((s2u<bool>(gUp[v].bpExit))   << NFLG_BP_EXIT_LM32_POS)
              | ((s2u<bool>(gUp[v].patEntry)) << NFLG_PAT_ENTRY_LM32_POS)
              | ((s2u<bool>(gUp[v].patExit))  << NFLG_PAT_EXIT_LM32_POS);

        } catch (std::runtime_error const& err) {
          throw std::runtime_error( "Parser error when processing pattern/BP entry/exit tags of node <" + name + ">. Cause: " + err.what());
        }

        amI it = atUp.lookupHashNoEx(hash); //if we already have a download entry, keep allocation, but update vertex index
        if (!atUp.isOk(it)) {
          //sLog << "Adding " << name << std::endl;
          allocState = atUp.allocate(cpu, hash, v, gUp, true);
          if (allocState == ALLOC_NO_SPACE)         {throw std::runtime_error("Not enough space in CPU " + std::to_string(cpu) + " memory pool"); return; }
          if (allocState == ALLOC_ENTRY_EXISTS)     {throw std::runtime_error("Node '" + name + "' would be duplicate in graph."); return; }
          // getting here means alloc went okay
          it = atUp.lookupHash(hash);
        }

        //TODO Find something better than stupic cast to ptr
        //Ugly as hell. But otherwise the bloody iterator will only allow access to MY alloc buffers (not their pointers!) as const!
        auto* x = (AllocMeta*)&(*it);

        try{
        // add timing node data objects to vertices
        if(gUp[v].np == nullptr) {

          cmp = gUp[v].type;

              // TODO most of this shit should be in constructor
               if (cmp == dnt::sTMsg)        {completeId(v, gUp); // create ID from SubId fields or vice versa
                                              gUp[v].np = (node_ptr) new  TimingMsg(gUp[v].name, gUp[v].patName, gUp[v].bpName, x->hash, x->cpu, flags, s2u<uint64_t>(gUp[v].tOffs), s2u<uint64_t>(gUp[v].id), s2u<uint64_t>(gUp[v].par), s2u<uint32_t>(gUp[v].tef), s2u<uint32_t>(gUp[v].res)); }
          else if (cmp == dnt::sCmdNoop)     {gUp[v].np = (node_ptr) new       Noop(gUp[v].name, gUp[v].patName, gUp[v].bpName, x->hash, x->cpu, flags, s2u<uint64_t>(gUp[v].tOffs), s2u<uint64_t>(gUp[v].tValid), s2u<uint8_t>(gUp[v].prio), s2u<uint32_t>(gUp[v].qty), s2u<bool>(gUp[v].vabs)); }
          else if (cmp == dnt::sCmdFlow)     {gUp[v].np = (node_ptr) new       Flow(gUp[v].name, gUp[v].patName, gUp[v].bpName, x->hash, x->cpu, flags, s2u<uint64_t>(gUp[v].tOffs), s2u<uint64_t>(gUp[v].tValid), s2u<uint8_t>(gUp[v].prio), s2u<uint32_t>(gUp[v].qty), s2u<bool>(gUp[v].vabs), s2u<bool>(gUp[v].perma)); }
          else if (cmp == dnt::sSwitch)      {gUp[v].np = (node_ptr) new     Switch(gUp[v].name, gUp[v].patName, gUp[v].bpName, x->hash, x->cpu, flags, s2u<uint64_t>(gUp[v].tOffs) ); }
          else if (cmp == dnt::sOrigin)      {gUp[v].np = (node_ptr) new     Origin(gUp[v].name, gUp[v].patName, gUp[v].bpName, x->hash, x->cpu, flags, s2u<uint64_t>(gUp[v].tOffs), s2u<uint8_t>(gUp[v].thread)); }
          else if (cmp == dnt::sStartThread) {gUp[v].np = (node_ptr) new     StartThread(gUp[v].name, gUp[v].patName, gUp[v].bpName, x->hash, x->cpu, flags, s2u<uint64_t>(gUp[v].tOffs), s2u<uint64_t>(gUp[v].startOffs), s2u<uint32_t>(gUp[v].thread)); }
          else if (cmp == dnt::sCmdFlush)    {gUp[v].np = (node_ptr) new      Flush(gUp[v].name, gUp[v].patName, gUp[v].bpName, x->hash, x->cpu, flags, s2u<uint64_t>(gUp[v].tOffs), s2u<uint64_t>(gUp[v].tValid), s2u<uint8_t>(gUp[v].prio),
                                                                                s2u<bool>(gUp[v].qIl), s2u<bool>(gUp[v].qHi), s2u<bool>(gUp[v].qLo), s2u<bool>(gUp[v].vabs), s2u<bool>(gUp[v].perma), s2u<uint8_t>(gUp[v].frmIl), s2u<uint8_t>(gUp[v].toIl), s2u<uint8_t>(gUp[v].frmHi),
                                                                                s2u<uint8_t>(gUp[v].toHi), s2u<uint8_t>(gUp[v].frmLo), s2u<uint8_t>(gUp[v].toLo) ); }
          else if (cmp == dnt::sCmdWait)     {gUp[v].np = (node_ptr) new       Wait(gUp[v].name, gUp[v].patName, gUp[v].bpName, x->hash, x->cpu, flags, s2u<uint64_t>(gUp[v].tOffs), s2u<uint64_t>(gUp[v].tValid), s2u<uint8_t>(gUp[v].prio), s2u<uint64_t>(gUp[v].tWait), s2u<bool>(gUp[v].vabs)); }
          else if (cmp == dnt::sBlock)       {gUp[v].np = (node_ptr) new BlockFixed(gUp[v].name, gUp[v].patName, gUp[v].bpName, x->hash, x->cpu, flags, s2u<uint64_t>(gUp[v].tPeriod) ); }
          else if (cmp == dnt::sBlockFixed)  {gUp[v].np = (node_ptr) new BlockFixed(gUp[v].name, gUp[v].patName, gUp[v].bpName, x->hash, x->cpu, flags, s2u<uint64_t>(gUp[v].tPeriod) ); }
          else if (cmp == dnt::sBlockAlign)  {gUp[v].np = (node_ptr) new BlockAlign(gUp[v].name, gUp[v].patName, gUp[v].bpName, x->hash, x->cpu, flags, s2u<uint64_t>(gUp[v].tPeriod) ); }
          else if (cmp == dnt::sQInfo)       {gUp[v].np = (node_ptr) new   CmdQMeta(gUp[v].name, gUp[v].patName, gUp[v].bpName, x->hash, x->cpu, flags);}
          else if (cmp == dnt::sDstList)     {gUp[v].np = (node_ptr) new   DestList(gUp[v].name, gUp[v].patName, gUp[v].bpName, x->hash, x->cpu, flags);}
          else if (cmp == dnt::sQBuf)        {gUp[v].np = (node_ptr) new CmdQBuffer(gUp[v].name, gUp[v].patName, gUp[v].bpName, x->hash, x->cpu, flags);}
          else if (cmp == dnt::sGlobal)      {gUp[v].np = (node_ptr) new     Global(gUp[v].name, gUp[v].patName, gUp[v].bpName, x->hash, x->cpu, flags, gUp[v].section);}
          else if (cmp == dnt::sMeta)        {throw std::runtime_error("Pure meta type not yet implemented"); return;}
          //FIXME try to get info from download
          else                        {throw std::runtime_error("Node <" + gUp[v].name + ">'s type <" + cmp + "> is not supported!\nMost likely you forgot to set the type attribute or accidentally created the node by a typo in an edge definition."); return;}
        }
        } catch (std::runtime_error const& err) {
          throw std::runtime_error( "Failed to create data object for node <" + name + "> of type <" + cmp + ">. Cause: " + err.what());
        }
    }

    // Crawl vertices and serialise their data objects for upload
    BOOST_FOREACH( vertex_t v, vertices(gUp) ) {

      gUp[v].np->accept(VisitorUploadCrawler(gUp, v, atUp, sLog, sErr));

      //Check if all mandatory fields were properly initialised
      auto x = atUp.lookupVertex(v);
      std::string haystack(x->b, x->b + _MEM_BLOCK_SIZE);
      std::size_t n = haystack.find(DotStr::Misc::needle);

      bool foundUninitialised = (n != std::string::npos);

      if(debug || foundUninitialised) {
        log<DEBUG_LVL1>(L"prepareUpload: @ %1$#08x \n %2%") % atUp.adrConv(AdrType::MGMT, AdrType::INT, x->cpu, x->adr) % hexDump(gUp[v].name.c_str(), haystack.c_str(), _MEM_BLOCK_SIZE).c_str();
      }

      if(foundUninitialised) {
        throw std::runtime_error("Node '" + gUp[v].name + "'contains uninitialised elements!\nMisspelled/forgot a mandatory property in .dot file ?");
      }
    }

    atUp.rl->showMemLocMap();
    //atUp.rl->showMemFieldMap();

  }


  int CarpeDM::CarpeDMimpl::upload( uint8_t opType, std::vector<QueueReport>& vQr) {
    updateModTime();
    //we only regard modifications by order as modded, so we need to check for changed content before we generate the management data
    std::set<uint8_t> moddedCpus;
    for(unsigned int i = 0; i < atUp.getMemories().size(); i++) {
      if (!freshDownload || (atUp.getMemories()[i].getBmp() != atDown.getMemories()[i].getBmp()) ) moddedCpus.insert(i); // mark cpu as modified if alloctable empty or Bmp Changed
    }
    vEbwrs ew, ewChg, ewOrphans;
    generateMgmtData();
    ewChg = gatherUploadVector(moddedCpus, 0, opType); //TODO not using modCnt right now, maybe implement later
    deactivateOrphanedCommands(ewOrphans, vQr);
  /*
    //Simulate orphan cleanup memory corruption bug
    const  int dummy = 14;
    ewOrphans.va.push_back(ewChg.va[dummy]);
    ewOrphans.vb.push_back(0xDE);
    ewOrphans.vb.push_back(0xAD);
    ewOrphans.vb.push_back(0xBE);
    ewOrphans.vb.push_back(0xEF);
    ewOrphans.vcs.push_back(ewChg.vcs[dummy]);
   /
    std::string sDebug;
    auto adri = ewOrphans.va.begin();
    auto dati = ewOrphans.vb.begin();
    while (adri != ewOrphans.va.end() and dati != ewOrphans.vb.end())
    {
      auto adr = adri;
      auto dat = dati;
      uint8_t b[4] = {*(dat+0), *(dat+1), *(dat+2), *(dat+3)};
      auto iHit = std::find(ewChg.va.begin(), ewChg.va.end(), *adr);

      if(iHit != ewChg.va.end()) {
        std::stringstream auxstream;
        uint32_t val = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&b[0]);
        auto idx = (iHit - ewChg.va.begin()) * 4;

        ewChg.vb[idx+0] = 0xca;
        ewChg.vb[idx+1] = 0xfe;
        ewChg.vb[idx+2] = 0xba;
        ewChg.vb[idx+3] = 0xbe;

        uint8_t b1[4] = {ewChg.vb[idx+0], ewChg.vb[idx+1], ewChg.vb[idx+2], ewChg.vb[idx+3]};

        uint32_t hitval = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&b1[0]);

        auxstream << " A 0x" << std::setfill('0') << std::setw(8) << std::hex << *adr << " D 0x" << std::setfill('0') << std::setw(8) << std::hex << val << " Destroys: 0x" << std::setfill('0') << std::setw(8) << std::hex << hitval << std::endl;
        sDebug += auxstream.str();
      }
      adri++;
      dati+=4;
    }
    if (sDebug.size()  > 0) {
      sErr << "Possible access violation: Orphaned command cleanup routine tried to overwrite otherwise modified nodes. List of conflicting EB write ops:\n" << sDebug << std::endl;
      //throw std::runtime_error("Possible access violation: Orphaned command cleanup routine tried to overwrite otherwise modified nodes. List of conflicting EB write ops:\n" + sDebug);

    }
*/
    ew = ewOrphans + ewChg; //order of writing is critical !!! Else the above described memory corruption will happen

    //Upload
    ebd.writeCycle(ew.va, ew.vb, ew.vcs);
    log<INFO>(L"upload: Done");
    freshDownload = false;
    return ew.va.size();

  }

  /** Use the current download graph and allocation table as base for the
   * upload. Clear upload graph and allocation table. Download schedule from
   * firmware and copy this to the upload graph and allocation table.
   */
  void CarpeDM::CarpeDMimpl::baseUploadOnDownload() {
    //init up graph from down
    gUp.clear(); //Necessary?
    atUp.clear();
    download();
    atUp.cpyWithoutMgmt(atDown);
    // for some reason, copy_graph does not copy the name
    //boost::set_property(gTmp, boost::graph_name, boost::get_property(g, boost::graph_name));
    vertex_map_t vmap;
    updown_copy_graph(gDown, gUp, vmap, atUp, hm, gt);
    //copy_graph<Graph>(gDown, gUp, vmap);

  }

  void CarpeDM::CarpeDMimpl::addition(Graph& gTmp) {

    vertex_map_t vertexMap, duplicates;
    log<VERBOSE>(L"addition: Generating Metadata");


    //find and list all duplicates i.e. docking points between trees and Update hash dict
    BOOST_FOREACH( vertex_t w, vertices(gTmp) ) {
      //add to hash dict
      hm.add(gTmp[w].name);
      amI x = atUp.lookupHashNoEx(gTmp[w].hash);

      if (atUp.isOk(x)) {
        //Check how the duplicate is defined. Implicit (by edge) is okay, explicit is not. necessary to avoid unintentional merge of graph nodes of the same name
        //Check the type: if it's undefined, node definition was implicit
        if (gTmp[w].type != sUndefined) throw std::runtime_error( "Node " + gTmp[w].name + " already exists. You can only use the name again in an edge descriptor (implicit definition)");
        duplicates[x->v] = w;
      }
    }

    //merge graphs (will lead to disjunct trees with duplicates at overlaps), but keep the mapping for vertex merge
    //boost::associative_property_map<vertex_map_t> vertexMapWrapper(vertexMap);
    //copy_graph(gTmp, gUp, boost::orig_to_copy(vertexMapWrapper));
    log<VERBOSE>(L"addition: Merging graphs");
    mycopy_graph<Graph>(gTmp, gUp, vertexMap);
    //for(auto& it : vertexMap ) {sLog <<  "gTmp " << gTmp[it.first].name << " @ " << it.first << " gUp " << it.second << std::endl; }
    //merge duplicate nodes
    for(auto& it : duplicates ) {
      //sLog <<  it.first << " <- " << it.second << "(" << vertexMap[it.second] << ")" << std::endl;
      mergeUploadDuplicates(it.first, vertexMap[it.second]);
    }
    log<VERBOSE>(L"addition: Removing duplicates");
    //now remove duplicates
    for(auto& itDup : duplicates ) {
      boost::clear_vertex(vertexMap[itDup.second], gUp);
      boost::remove_vertex(vertexMap[itDup.second], gUp);
      //remove_vertex() changes the vertex vector, as it must stay contignuous. descriptors higher than he removed one therefore need to be decremented by 1
      for( auto& updateMapIt : vertexMap) {if (updateMapIt.first > itDup.second) updateMapIt.second--; }
    }
    generateBlockMeta(gUp); //auto generate desired Block Meta Nodes

    //FIXME this also adds/changes the known nodes based on the download. Do we really want that?
    //add whats left to groups dict
    log<VERBOSE>(L"addition: Updating Group dict");
    BOOST_FOREACH( vertex_t v, vertices(gUp) ) {
      try {
        gt.setBeamproc(gUp[v].name, gUp[v].bpName, (s2u<bool>(gUp[v].bpEntry)), (s2u<bool>(gUp[v].bpExit)));
        gt.setPattern(gUp[v].name, gUp[v].patName, (s2u<bool>(gUp[v].patEntry)), (s2u<bool>(gUp[v].patExit)));
      } catch (std::runtime_error const& err) {
        throw std::runtime_error( "Parser error when processing entry/exit tags of node <" + gUp[v].name + ">. Cause: " + err.what());
      }
    }
    //writeUpDotFile("inspect.dot", false);
    log<VERBOSE>(L"addition: creating binary for upload");
    prepareUpload();
    atUp.syncBmpsToPools();
    log<INFO>(L"addition: Done");
  }

  void CarpeDM::CarpeDMimpl::pushMetaNeighbours(vertex_t v, Graph& g, vertex_set_t& s) {

    //recursively find all adjacent meta type vertices
    BOOST_FOREACH( vertex_t w, adjacent_vertices(v, g)) {
      log<DEBUG_LVL0>(L"Checking adjacent vertex %1%") % g[w].name.c_str();
      if (g[w].np == nullptr) {throw std::runtime_error("Node " + g[w].name + " does not have a data object, this is bad");}
      if (g[w].np->isMeta()) {
        s.insert(w);
        log<DEBUG_LVL0>(L"Added adjacent vertex %1% to del map, %2% vertex idx") % g[w].name.c_str() % w;
        pushMetaNeighbours(w, g, s);
      } else {
        log<DEBUG_LVL0>(L"Skipping adjacent vertex %1%, %2% vertex idx") % g[w].name.c_str() % w;
      }
    }
    log<INFO>(L"pushMetaNeighbours: Done");
  }

  void CarpeDM::CarpeDMimpl::subtraction(Graph& gTmp) {

    vertex_map_t vertexMap;
    vertex_set_t toDelete;
    //typedef boost::bimap< uint32_t, vertex_t v > hashVertexMap;
    //typedef hBiMap::value_type hashVertexTuple;
//
    //hashVertexMap hvm;

    //TODO probably a more elegant solution out there, but I don't have the time for trial and error on boost property maps.
    //create 1:1 vertex map for all vertices in gUp initially marked for deletion. Also add all their meta children to leave no loose ends
    log<VERBOSE>(L"subtraction: Searching nodes to remove");
    BOOST_FOREACH( vertex_t v, vertices(gUp) ) {
      vertexMap[v] = v;
      //hvm.insert(hashVertexTuple(gUp[v].hash, v))
    }


    //TODO Test this approach to remove square complexity by lookup
    BOOST_FOREACH( vertex_t w, vertices(gTmp) ) {
      if (verbose) sLog <<  "Searching " << std::hex << " 0x" << gTmp[w].hash << std::endl;

      if (gTmp[w].type == DotStr::Misc::sUndefined) continue;

      auto x = atUp.lookupHashNoEx(gTmp[w].hash);
      if (atUp.isOk(x)) {
        toDelete.insert(x->v);
        log<DEBUG_LVL0>(L"subtraction: Checking adjacent vertices of node %1%") % gUp[x->v].name.c_str();                   // add the node
        pushMetaNeighbours(x->v, gUp, toDelete); // add all of its meta children as well
      }
    }
    log<VERBOSE>(L"subtraction: updating staging");
    //FIXME Square complexity, but unsure if inner loop can be replaced
    //check staging, vertices might have lost children
    for(auto& vd : toDelete ) {
      //check out all parents (sources) of this to be deleted node, update their staging
      Graph::in_edge_iterator in_begin, in_end, in_cur;

      boost::tie(in_begin, in_end) = in_edges(vertexMap[vd], gUp);
      for (in_cur = in_begin; in_cur != in_end; ++in_cur) {
        updateStaging(source(*in_cur, gUp), *in_cur);

      }
    }


    //FIXME Square complexity, but unsure if inner loop can be replaced
    //remove designated vertices
    log<VERBOSE>(L"subtraction: removing designated nodes");
    for(auto& vd : toDelete ) {
      //sLog <<  "Removing Node " << gUp[vertexMap[vd]].name << std::endl;
      atUp.deallocate(gUp[vertexMap[vd]].hash); //using the hash is independent of vertex descriptors, so no remapping necessary yet
      //remove node from hash and groups dict
      log<DEBUG_LVL0>(L"subtraction: removing %1%") % gUp[vertexMap[vd]].name.c_str();
      hm.remove(gUp[vertexMap[vd]].name);
      gt.remove<Groups::Node>(gUp[vertexMap[vd]].name);
      boost::clear_vertex(vertexMap[vd], gUp);
      boost::remove_vertex(vertexMap[vd], gUp);

      //FIXME this removal scheme is crap ! Graph vertices and edges ought to be kept in setS or listS so iterators stay valid on removal of other vertices
      //However, the cure is worse than the disease. graphviz_write, copy_graph all fuck around if we do, as they require vertex descriptors wich setS and listS don't provide ... leave for now


      //remove_vertex() changes the vertex vector, as it must stay contignuous. descriptors higher than he removed one therefore need to be decremented by 1
      for( auto& updateMapIt : vertexMap) {if (updateMapIt.first > vd) updateMapIt.second--; }
    }

    //now we have a problem: all vertex descriptors in the alloctable just got invalidated by the removal ... repair them
    std::vector<amI> itAtVec; //because elements change order during repair loop, we need to store iterators first
    for( amI it = atUp.getTable().begin(); it != atUp.getTable().end(); it++) { itAtVec.push_back(it); }



    for( auto itIt : itAtVec ) {  //now we can safely iterate over the alloctable iterators
      atUp.modV(itIt, vertexMap[itIt->v]);
    }
    generateBlockMeta(gUp); //auto generate desired Block Meta Nodes
    prepareUpload();
    atUp.syncBmpsToPools();
    log<INFO>(L"subtraction: Done");
  }


  void CarpeDM::CarpeDMimpl::generateMgmtData() {
    std::string tmpStrBufGrp = gt.store();
    std::string tmpStrBufCov = ct.store();
    std::string tmpStrBufRef = rt.store();

    atUp.setMgmtLLSizes(tmpStrBufGrp.size(), tmpStrBufCov.size(), tmpStrBufRef.size());
    std::string tmpStrBuf = tmpStrBufGrp + tmpStrBufCov + tmpStrBufRef;

    vBuf tmpBuf(tmpStrBuf.begin(), tmpStrBuf.end());
    vBuf mgmtBinary = compress(tmpBuf);
    atUp.allocateMgmt(mgmtBinary);
    atUp.populateMgmt(mgmtBinary);
    //atUp.debugMgmt(sLog);
    atUp.syncBmpsToPools();
  }

  void CarpeDM::CarpeDMimpl::nullify() {
    gUp.clear();
    gDown.clear();
    atUp.clear();
    atDown.clear();
    gt.clear();
    ct.clear();
    rt.clear();
    hm.clear();
  }

  /** Check that all vertex names of g are in the current HashMap hm and
   * in the current GroupTable gt. This is used for remove and keep operations.
   */
  void CarpeDM::CarpeDMimpl::checkTablesForSubgraph(Graph& g) {
    BOOST_FOREACH( vertex_t v, vertices(g) ) {
      //Check Hashtable
      if (!hm.lookup(g[v].name)) {throw std::runtime_error("Node <" + g[v].name + "> was explicitly named for keep/remove, but is unknown to Hashtable!\n");}
      //Check Groupstable
      auto x  = gt.getTable().get<Groups::Node>().equal_range(g[v].name);
      if (x.first == x.second)   {throw std::runtime_error("Node <" + g[v].name + "> was explicitly named for keep/remove, but is unknown to Grouptable!\n");}
    }

  }

  //high level functions for external interface
  int CarpeDM::CarpeDMimpl::add(Graph& g, bool force) {

    if ((boost::get_property(g, boost::graph_name)).find(DotStr::Graph::Special::sCmd) != std::string::npos) {throw std::runtime_error("Expected a schedule, but these appear to be commands (Tag '" + DotStr::Graph::Special::sCmd + "' found in graphname)"); return -1;}
    if(verbose) sLog << "Download binary as base for addition" << std::endl;
    baseUploadOnDownload();
    if(verbose) sLog << "Add new subgraph" << std::endl;
    addition(g);
    //writeUpDotFile("upload.dot", false);
    validate(gUp, atUp, force);
    if(verbose) sLog << "Upload" << std::endl;
    return upload(OP_TYPE_SCH_ADD);
  }

  int CarpeDM::CarpeDMimpl::remove(Graph& g, bool force) {
    if ((boost::get_property(g, boost::graph_name)).find(DotStr::Graph::Special::sCmd) != std::string::npos) {throw std::runtime_error("Expected a schedule, but these appear to be commands (Tag '" + DotStr::Graph::Special::sCmd + "' found in graphname)"); return -1;}
    checkTablesForSubgraph(g); //all explicitly named nodes must be known to hash and grouptable. let's check first

    //FIXME fuckin dangerous stuff, both change the global grouptable. For this to work, it must be 1st baseUploadOnDownload, generateBlockMeta must be 2nd. This is horrible
    baseUploadOnDownload();
    //generateBlockMeta(g);

    std::string report;
    std::vector<QueueReport> vQr;
    CovenantTable ctAdditions;
    bool isSafe =  isSafeToRemove(g, report, vQr, ctAdditions);
    //writeTextFile("safetyReportNormal.dot", report);
    if (!(force | (isSafe))) {throw std::runtime_error("//Subgraph cannot safely be removed!\n\n" + report);}

    subtraction(g);
    //writeUpDotFile("upload.dot", false);
    validate(gUp, atUp, force);
    addCovenants(ctAdditions);
    return upload(OP_TYPE_SCH_REMOVE, vQr);
  }


  int CarpeDM::CarpeDMimpl::keep(Graph& g, bool force) {
    if ((boost::get_property(g, boost::graph_name)).find(DotStr::Graph::Special::sCmd) != std::string::npos) {throw std::runtime_error("Expected a schedule, but these appear to be commands (Tag '" + DotStr::Graph::Special::sCmd + "' found in graphname)"); return -1;}
    Graph gTmpRemove;
    Graph& gTmpKeep = g;
    checkTablesForSubgraph(g); //all explicitly named nodes must be known to hash and grouptable. let's check first
    //writeDotFile("inspect.dot", gTmpKeep, false);

    //FIXME fuckin dangerous stuff, both change the global grouptable. For this to work, it must be 1st baseUploadOnDownload, generateBlockMeta must be 2nd. This is horrible
    baseUploadOnDownload();
    generateBlockMeta(gTmpKeep, false);


    //FIXME Resource hog with square complexity. Replace inner loop by lookup
    bool found;
    BOOST_FOREACH( vertex_t w, vertices(gUp) ) {
      //sLog <<  "Scanning " << gUp[w].name << std::endl;
      found = false;
      BOOST_FOREACH( vertex_t v, vertices(gTmpKeep) ) {
        if ((gTmpKeep[v].name == gUp[w].name)) {
          found = true;
          break;

        }
      }
      if (!found) { boost::add_vertex(myVertex(gUp[w]), gTmpRemove);
      }
    }

    std::string report;
    std::vector<QueueReport> vQr;
    CovenantTable ctAdditions;
    bool isSafe =  isSafeToRemove(gTmpRemove, report, vQr, ctAdditions);
    //writeTextFile("safetyReportNormal.dot", report);
    if (!(force | (isSafe))) {throw std::runtime_error("//Subgraph cannot safely be removed!\n\n" + report);}

    subtraction(gTmpRemove);
    //writeUpDotFile("upload.dot", false);
    validate(gUp, atUp, force);
    addCovenants(ctAdditions);
    return upload(OP_TYPE_SCH_KEEP, vQr);
  }

  int CarpeDM::CarpeDMimpl::clear_raw(bool force) {
    nullify(); // read out current time for upload mod time (seconds, but probably better to use same format as DM FW. Convert to ns)
    // check if there are any threads still running first
    uint32_t activity = 0;
    for(uint8_t cpuIdx=0; cpuIdx < ebd.getCpuQty(); cpuIdx++) {
      uint32_t s = getThrStart(cpuIdx);
      uint32_t r = getThrRun(cpuIdx);
      //printf("#%u ThrStartBits: 0x%08x, ThrRunBits: 0x%08x, force=%u\n", cpuIdx, s, r, (int)force );
      activity |= s | r;
    }


    if (!force && activity)  {throw std::runtime_error("Cannot clear, threads are still running. Call stop/abort/halt first\n");}
    return upload(OP_TYPE_SCH_CLEAR);
  }

  int CarpeDM::CarpeDMimpl::overwrite(Graph& g, bool force) {
    if ((boost::get_property(g, boost::graph_name)).find(DotStr::Graph::Special::sCmd) != std::string::npos) {throw std::runtime_error("Expected a schedule, but these appear to be commands (Tag '" + DotStr::Graph::Special::sCmd + "' found in graphname)"); return -1;}
    nullify();
    addition(g);
    //writeUpDotFile("upload.dot", false);
    validate(gUp, atUp, force);
    // check if there are any threads still running first
    uint32_t activity = 0;
    for(uint8_t cpuIdx=0; cpuIdx < ebd.getCpuQty(); cpuIdx++) {
      activity |= getThrRun(cpuIdx);
    }
    if (!force && activity)  {throw std::runtime_error("Cannot overwrite, threads are still running. Call stop/abort/halt first\n");}

    return upload(OP_TYPE_SCH_OVERWRITE);

  }
