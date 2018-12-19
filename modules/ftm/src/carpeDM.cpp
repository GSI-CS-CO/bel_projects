#include <boost/shared_ptr.hpp>
#include <algorithm>
#include <stdio.h>
#include <iostream>
#include <string>
#include <inttypes.h>
#include <boost/graph/graphviz.hpp>
//#include <boost/algorithm/string.hpp>
#include <sys/time.h>

#include "common.h"
#include "propwrite.h"
#include "graph.h"
#include "carpeDM.h"
#include "minicommand.h"
#include "dotstr.h"
#include "idformat.h"
#include "lzmaCompression.h"
#include "ebwrapper.h"


  namespace dgp = DotStr::Graph::Prop;
  namespace dnp = DotStr::Node::Prop;
  namespace dep = DotStr::Edge::Prop;


  Graph& CarpeDM::getUpGraph()   {return gUp;}   //Returns the Upload Graph for CPU <cpuIdx>
  Graph& CarpeDM::getDownGraph() {return gDown;} //Returns the Download Graph for CPU <cpuIdx>

vBuf CarpeDM::compress(const vBuf& in) {return lzmaCompress(in);}
vBuf CarpeDM::decompress(const vBuf& in) {return lzmaDecompress(in);}




  void CarpeDM::completeId(vertex_t v, Graph& g) { // deduce SubID fields from ID or vice versa, depending on whether ID is defined



    std::stringstream ss;
    uint64_t id;
    uint8_t fid;
    boost::dynamic_properties dp = createParser(g); //create current property map

    if (g[v].id == DotStr::Misc::sUndefined64) { // from SubID fields to ID
      //sLog << "Input Node  " << g[v].name;
      fid = (s2u<uint8_t>(g[v].id_fid) & ID_FID_MSK); //get fid
      if (fid >= idFormats.size()) throw std::runtime_error("bad format id (FID) field in Node '" + g[v].name + "'");
      vPf& vTmp = idFormats[fid]; //choose conversion vector by fid
      id = 0;
      for(auto& it : vTmp) {  //for each format vector element
        //use dot property tag string as key to dp map (map of tags to (maps of vertex_indices to values))
        uint64_t val = s2u<uint64_t>(boost::get(it.s, dp, v)); // use vertex index v as key in this property map to obtain value
        //sLog << ", " << std::dec << it.s << " = " << (val & ((1 << it.bits ) - 1) ) << ", (" << (int)it.pos << ",0x" << std::hex << ((1 << it.bits ) - 1) << ")";
        id |= ((val & ((1 << it.bits ) - 1) ) << it.pos); // OR the masked and shifted value to id
      }

      ss.flush();
      ss << "0x" << std::hex << id;
      g[v].id = ss.str();
      //sLog << "ID = " << g[v].id << std::endl;
    } else { //from ID to SubID fields
      id = s2u<uint8_t>(g[v].id);
      fid = ((id >> ID_FID_POS) & ID_FID_MSK);
      if (fid >= idFormats.size()) throw std::runtime_error("bad format id (FID) within ID field of Node '" + g[v].name + "'");
      vPf& vTmp = idFormats[fid];

      for(auto& it : vTmp) {
        ss.flush();
        ss << std::dec << ((id >> it.pos) &  ((1 << it.bits ) - 1) );
        boost::put(it.s, dp, v, ss.str());
      }
    }

  }

  const std::string& CarpeDM::firstString(const vStrC& v) {return ((v.size() > 0) ? *(v.begin()) : DotStr::Misc::sUndefined);}


  boost::dynamic_properties CarpeDM::createParser(Graph& g) {

    boost::dynamic_properties dp(boost::ignore_other_properties);
    boost::ref_property_map<Graph *, std::string> gname( boost::get_property(g, boost::graph_name));
    dp.property(dgp::sName,     gname);
    dp.property(dep::Base::sType,               boost::get(&myEdge::type,         g));
    dp.property(dnp::Base::sName,               boost::get(&myVertex::name,       g));
    dp.property(dnp::Base::sCpu,                boost::get(&myVertex::cpu,        g));

    dp.property(dnp::Base::sType,               boost::get(&myVertex::type,       g));
    dp.property(dnp::Base::sFlags,              boost::get(&myVertex::flags,      g));
    dp.property(dnp::Base::sPatName,            boost::get(&myVertex::patName,    g));
    dp.property(dnp::Base::sPatEntry,           boost::get(&myVertex::patEntry,   g));
    dp.property(dnp::Base::sPatExit,            boost::get(&myVertex::patExit,    g));
    dp.property(dnp::Base::sBpName,             boost::get(&myVertex::bpName,     g));
    dp.property(dnp::Base::sBpEntry,            boost::get(&myVertex::bpEntry,    g));
    dp.property(dnp::Base::sBpExit,             boost::get(&myVertex::bpExit,     g));
    //Block
    dp.property(dnp::Block::sTimePeriod,        boost::get(&myVertex::tPeriod,    g));
    dp.property(dnp::Block::sGenQPrioHi,        boost::get(&myVertex::qIl,        g));
    dp.property(dnp::Block::sGenQPrioMd,        boost::get(&myVertex::qHi,        g));
    dp.property(dnp::Block::sGenQPrioLo,        boost::get(&myVertex::qLo,        g));
    //Timing Message
    dp.property(dnp::TMsg::sTimeOffs,           boost::get(&myVertex::tOffs,      g));
    dp.property(dnp::TMsg::sId,                 boost::get(&myVertex::id,         g));
      //ID sub fields
    dp.property(dnp::TMsg::SubId::sFid,         boost::get(&myVertex::id_fid,     g));
    dp.property(dnp::TMsg::SubId::sGid,         boost::get(&myVertex::id_gid,     g));
    dp.property(dnp::TMsg::SubId::sEno,         boost::get(&myVertex::id_evtno,   g));
    dp.property(dnp::TMsg::SubId::sSid,         boost::get(&myVertex::id_sid,     g));
    dp.property(dnp::TMsg::SubId::sBpid,        boost::get(&myVertex::id_bpid,    g));
    dp.property(dnp::TMsg::SubId::sBin,         boost::get(&myVertex::id_bin,     g));
    dp.property(dnp::TMsg::SubId::sReqNoB,      boost::get(&myVertex::id_reqnob,  g));
    dp.property(dnp::TMsg::SubId::sVacc,        boost::get(&myVertex::id_vacc,    g));
    dp.property(dnp::TMsg::sPar,                boost::get(&myVertex::par,        g));
    dp.property(dnp::TMsg::sTef,                boost::get(&myVertex::tef,        g));
    //Command
    dp.property(dnp::Cmd::sTimeValid,           boost::get(&myVertex::tValid,     g));
    dp.property(dnp::Cmd::sVabs,                boost::get(&myVertex::vabs,       g));
    dp.property(dnp::Cmd::sPrio,                boost::get(&myVertex::prio,       g));
    dp.property(dnp::Cmd::sQty,                 boost::get(&myVertex::qty,        g));
    dp.property(dnp::Cmd::sTimeWait,            boost::get(&myVertex::tWait,      g));
    dp.property(dnp::Cmd::sPermanent,           boost::get(&myVertex::perma,      g));

    //for .dot-cmd abuse
    dp.property(dnp::Cmd::sTarget,              boost::get(&myVertex::cmdTarget,  g));
    dp.property(dnp::Cmd::sDst,                 boost::get(&myVertex::cmdDest,    g));
    dp.property(dnp::Cmd::sDstPattern,          boost::get(&myVertex::cmdDestPat, g));
    dp.property(dnp::Cmd::sDstBeamproc,         boost::get(&myVertex::cmdDestBp,  g));
    dp.property(dnp::Base::sThread,             boost::get(&myVertex::thread,     g));

    return (const boost::dynamic_properties)dp;
  }


  std::string CarpeDM::readTextFile(const std::string& fn) {
    std::string ret;
    std::ifstream in(fn);
    if(in.good()) {
      std::stringstream buffer;
      buffer << in.rdbuf();
      ret = buffer.str();
    }
    else {throw std::runtime_error(" Could not read from file '" + fn + "'");}

    return ret;
  }

  Graph& CarpeDM::parseDot(const std::string& s, Graph& g) {
    boost::dynamic_properties dp = createParser(g);

    try { boost::read_graphviz(s, g, dp, dnp::Base::sName); }
    catch(...) { throw; }

    BOOST_FOREACH( vertex_t v, vertices(g) ) { g[v].hash = hm.hash(g[v].name); } //generate hash to complete vertex information

    return g;
  }



  void CarpeDM::showMemSpace() {
    sLog << "Space" << std::setw(11) << "Free" << std::endl;
    for (uint8_t x = 0; x < ebd.getCpuQty(); x++) {
      sLog << std::dec << std::setfill(' ') << std::setw(11) << atDown.getTotalSpace(x) << std::setw(10) << atDown.getFreeSpace(x) * 100 / atDown.getTotalSpace(x) << "%";
      sLog << std::endl;
    }
  }





  uint8_t CarpeDM::getNodeCpu(const std::string& name, TransferDir dir) {

    AllocTable& at = (dir == TransferDir::UPLOAD ? atUp : atDown );
    uint32_t hash;
    hash = hm.lookup(name); //just pass it on

    auto x = at.lookupHash(hash);
    return x->cpu;
  }

  uint32_t CarpeDM::getNodeAdr(const std::string& name, TransferDir dir, AdrType adrT) {
    if (verbose) sLog << "Looking up Adr of " << name << std::endl;
    if(name == DotStr::Node::Special::sIdle) return LM32_NULL_PTR; //idle node is resolved as a null ptr without comment

    AllocTable& at = (dir == TransferDir::UPLOAD ? atUp : atDown );
    uint32_t hash;

    hash = hm.lookup(name); //just pass it on
    auto x = at.lookupHash(hash);

    switch (adrT) {
      case AdrType::MGMT : return x->adr; break;
      case AdrType::INT  : return at.adrConv(AdrType::MGMT, AdrType::INT, x->cpu, x->adr); break;
      case AdrType::EXT  : return at.adrConv(AdrType::MGMT, AdrType::EXT, x->cpu, x->adr); break;
      case AdrType::PEER : return at.adrConv(AdrType::MGMT, AdrType::PEER, x->cpu, x->adr); break;
      default            : throw std::runtime_error( "Unknown Adr Type conversion"); return LM32_NULL_PTR;
    }

  }


//Returns if a hash / nodename is present on DM
  bool CarpeDM::isInHashDict(const uint32_t hash)  {

    if (atDown.isOk(atDown.lookupHash(hash))) return true;
    else return false;
  }

  bool CarpeDM::isInHashDict(const std::string& name) {
    if (!(hm.contains(name))) return false;
    return (atDown.isOk(atDown.lookupHash(hm.lookup(name))));
  }

  // Name/Hash Dict ///////////////////////////////////////////////////////////////////////////////
  //Add all nodes in .dot file to name/hash dictionary
  void CarpeDM::clearHashDict() {hm.clear();}; //Clear the dictionary
  std::string CarpeDM::storeHashDict() {return hm.store();};
  void CarpeDM::loadHashDict(const std::string& s) {hm.load(s);}
  void CarpeDM::storeHashDictFile(const std::string& fn) {writeTextFile(fn, storeHashDict());};
  void CarpeDM::loadHashDictFile(const std::string& fn) {loadHashDict(readTextFile(fn));};
  bool CarpeDM::isHashDictEmpty() {return (bool)(hm.size() == 0);};
  int  CarpeDM::getHashDictSize() {return hm.size();};
  void CarpeDM::showHashDict() {hm.debug(sLog);};

  // Group/Entry/Exit Table ///////////////////////////////////////////////////////////////////////////////
  std::string CarpeDM::storeGroupsDict() {return gt.store();};
  void CarpeDM::loadGroupsDict(const std::string& s) {gt.load(s);}
  void CarpeDM::storeGroupsDictFile(const std::string& fn) {writeTextFile(fn, storeGroupsDict());};
  void CarpeDM::loadGroupsDictFile(const std::string& fn) {loadGroupsDict(readTextFile(fn));};
  void CarpeDM::clearGroupsDict() {gt.clear();}; //Clear pattern table
   int CarpeDM::getGroupsSize() {return gt.getSize();};
  void CarpeDM::showGroupsDict() {gt.debug(sLog);};


  void CarpeDM::writeDotFile(const std::string& fn, Graph& g, bool filterMeta) { writeTextFile(fn, createDot(g, filterMeta)); }
  void CarpeDM::writeDownDotFile(const std::string& fn, bool filterMeta)       { writeTextFile(fn, createDot(gDown, filterMeta)); }
  void CarpeDM::writeUpDotFile(const std::string& fn, bool filterMeta)         { writeTextFile(fn, createDot(gUp, filterMeta)); }

  // Schedule Manipulation and Dispatch ///////////////////////////////////////////////////////////
  //TODO assign a cpu to each node object. Currently taken from input .dot
  int CarpeDM::assignNodesToCpus() {return 0;};
  //get all nodes from DM
  std::string CarpeDM::downloadDot(bool filterMeta) {download(); return createDot( gDown, filterMeta);};
  void CarpeDM::downloadDotFile(const std::string& fn, bool filterMeta) {download(); writeDownDotFile(fn, filterMeta);};
  //add all nodes and/or edges in dot file
  int CarpeDM::addDot(const std::string& s, bool force) {Graph gTmp; return safeguardTransaction(&CarpeDM::add, parseDot(s, gTmp), force);};
  int CarpeDM::addDotFile(const std::string& fn, bool force) {return addDot(readTextFile(fn), force);};
  //add all nodes and/or edges in dot file
  int CarpeDM::overwriteDot(const std::string& s, bool force) {Graph gTmp; return safeguardTransaction(&CarpeDM::overwrite, parseDot(s, gTmp), force);};
  int CarpeDM::overwriteDotFile(const std::string& fn, bool force) {return overwriteDot(readTextFile(fn), force);};
  //removes all nodes NOT in input file
  int CarpeDM::keepDot(const std::string& s, bool force) {Graph gTmp; return safeguardTransaction(&CarpeDM::keep, parseDot(s, gTmp), force);};
  int CarpeDM::keepDotFile(const std::string& fn, bool force) {return keepDot(readTextFile(fn), force);};
  //removes all nodes in input file
  int CarpeDM::removeDot(const std::string& s, bool force) {Graph gTmp; return safeguardTransaction(&CarpeDM::remove, parseDot(s, gTmp), force);};
  int CarpeDM::removeDotFile(const std::string& fn, bool force) {return removeDot(readTextFile(fn), force);};
  // Safe removal check
  //bool isSafe2RemoveDotFile(const std::string& fn) {Graph gTmp; return isSafeToRemove(parseDot(readTextFile(fn), gTmp));};
  //clears all nodes from DM
  int CarpeDM::clear(bool force) {return safeguardTransaction(&CarpeDM::clear_raw, force);};


  // Command Generation and Dispatch //////////////////////////////////////////////////////////////
  int CarpeDM::sendCommandsDot(const std::string& s) {Graph gTmp; vEbwrs ew; return send(createCommandBurst(ew, parseDot(s, gTmp)));}; //Sends a dotfile of commands to the DM
  int CarpeDM::sendCommandsDotFile(const std::string& fn) {Graph gTmp; vEbwrs ew; return send(createCommandBurst(ew, parseDot(readTextFile(fn), gTmp)));};
  //Send a command to Block <targetName> on CPU <cpuIdx> via Etherbone
  //int CarpeDM::sendCommand(const std::string& targetName, uint8_t cmdPrio, mc_ptr mc) {vEbwrs ew; return send(createCommand(targetName, cmdPrio, mc, ew));};


   //write out dotstringfrom download graph
  std::string CarpeDM::createDot(Graph& g, bool filterMeta) {
    std::ostringstream out;
    typedef boost::property_map< Graph, node_ptr myVertex::* >::type NpMap;

    boost::filtered_graph <Graph, boost::keep_all, non_meta<NpMap> > fg(g, boost::keep_all(), make_non_meta(boost::get(&myVertex::np, g)));
    try {

        if (filterMeta) {
          boost::write_graphviz(out, fg, make_vertex_writer(boost::get(&myVertex::np, fg)),
                      make_edge_writer(boost::get(&myEdge::type, fg)), sample_graph_writer{DotStr::Graph::sDefName},
                      boost::get(&myVertex::name, fg));
        }
        else {

          boost::write_graphviz(out, g, make_vertex_writer(boost::get(&myVertex::np, g)),
                      make_edge_writer(boost::get(&myEdge::type, g)), sample_graph_writer{DotStr::Graph::sDefName},
                      boost::get(&myVertex::name, g));
        }
      }
      catch(...) {throw;}

    return out.str();
  }

  //write out dotfile from download graph of a memunit
  void CarpeDM::writeTextFile(const std::string& fn, const std::string& s) {
    std::ofstream out(fn);

    if (verbose) sLog << "Writing Output File " << fn << "... ";
    if(out.good()) { out << s; }
    else {throw std::runtime_error(" Could not write to .dot file '" + fn + "'"); return;}
    if (verbose) sLog << "Done.";
  }

  bool CarpeDM::validate(Graph& g, AllocTable& at, bool force) {
    try {
          BOOST_FOREACH( vertex_t v, vertices(g) ) { Validation::neighbourhoodCheck(v, g);  }

          BOOST_FOREACH( vertex_t v, vertices(g) ) {
            if (g[v].np == nullptr) throw std::runtime_error("Validation of Sequence: Node '" + g[v].name + "' was not allocated" );
            g[v].np->accept(VisitorValidation(g, v, at, force));
          }
    } catch (std::runtime_error const& err) { throw std::runtime_error("Validation of " + std::string(err.what()) ); }
    return true;
  }



  //Improvised Transaction Management: If an upload preparation operation fails for any reason, we roll back the meta tables
  int CarpeDM::safeguardTransaction(int (CarpeDM::*func)(Graph&, bool), Graph& g, bool force) {
    HashMap hmBak     = hm;
    GroupTable gtBak  = gt;
    CovenantTable ctBak = ct;
    int ret;

    try {
      ret = (*this.*func)(g, force);
    } catch(...) {
      hm = hmBak;
      gt = gtBak;
      ct = ctBak;
      sLog << "Operation FAILED, executing roll back\n" << std::endl;
      throw;
    }

    return ret;
  }

  //Improvised Transaction Management: If an upload operation fails for any reason, we roll back the meta tables
  int CarpeDM::safeguardTransaction(int (CarpeDM::*func)(bool), bool force) {
    HashMap hmBak     = hm;
    GroupTable gtBak  = gt;
    CovenantTable ctBak = ct;
    int ret;

    try {
      ret = (*this.*func)(force);
    } catch(...) {
      hm = hmBak;
      gt = gtBak;
      ct = ctBak;
      sLog << "Operation FAILED, executing roll back\n" << std::endl;
      throw;
    }

    return ret;

  }


  vEbwrs& CarpeDM::createModInfo(uint8_t cpu, uint32_t modCnt, uint8_t opType, vEbwrs& ew, uint32_t adrOffs) {
    // modification time address (lo/hi)
    uint32_t modAdrBase = atUp.getMemories()[cpu].extBaseAdr + atUp.getMemories()[cpu].sharedOffs + SHCTL_DIAG + adrOffs;
    // save modification time, issuer

    char username[LOGIN_NAME_MAX];
    getlogin_r(username, LOGIN_NAME_MAX);
    char machinename[HOST_NAME_MAX];
    gethostname(machinename, HOST_NAME_MAX);


    uint8_t b[8];


    ew.vcs += leadingOne(8); // add 8 words
    ew.va.push_back(modAdrBase + T_MOD_INFO_TS    + 0);
    ew.va.push_back(modAdrBase + T_MOD_INFO_TS    + _32b_SIZE_);
    ew.va.push_back(modAdrBase + T_MOD_INFO_IID   + 0);
    ew.va.push_back(modAdrBase + T_MOD_INFO_IID   + _32b_SIZE_);
    ew.va.push_back(modAdrBase + T_MOD_INFO_MID   + 0);
    ew.va.push_back(modAdrBase + T_MOD_INFO_MID   + _32b_SIZE_);
    ew.va.push_back(modAdrBase + T_MOD_INFO_TYPE  );
    ew.va.push_back(modAdrBase + T_MOD_INFO_CNT   );
    writeLeNumberToBeBytes<uint64_t>((uint8_t*)&b[0], modTime);
    ew.vb.insert( ew.vb.end(), b, b +  _TS_SIZE_  );
    ew.vb.insert( ew.vb.end(), username, username +  _64b_SIZE_  );
    ew.vb.insert( ew.vb.end(), machinename, machinename +  _64b_SIZE_  );
    writeLeNumberToBeBytes<uint32_t>((uint8_t*)&b[0], opType);
    ew.vb.insert( ew.vb.end(), b, b +  _32b_SIZE_  );
    writeLeNumberToBeBytes<uint32_t>((uint8_t*)&b[0], modCnt);
    ew.vb.insert( ew.vb.end(), b, b +  _32b_SIZE_  );

    return ew;
  }

  vEbwrs& CarpeDM::createSchedModInfo(uint8_t cpu, uint32_t modCnt, uint8_t opType, vEbwrs& ew) { return createModInfo(cpu, modCnt, opType, ew, T_DIAG_SCH_MOD); };
  vEbwrs& CarpeDM::createCmdModInfo  (uint8_t cpu, uint32_t modCnt, uint8_t opType, vEbwrs& ew) { return createModInfo(cpu, modCnt, opType, ew, T_DIAG_CMD_MOD); };
/*
  int CarpeDM::startThr(uint8_t cpuIdx, uint8_t thrIdx)                              { vEbwrs ew; return send(startThr(cpuIdx, thrIdx, ew));} //Requests Thread to start
  int CarpeDM::startPattern(const std::string& sPattern, uint8_t thrIdx)             { vEbwrs ew; return send(startPattern(sPattern, thrIdx, ew));}//Requests Pattern to start
  int CarpeDM::startPattern(const std::string& sPattern)                             { vEbwrs ew; return send(startPattern(sPattern, ew));}//Requests Pattern to start on first free thread
  int CarpeDM::startNodeOrigin(const std::string& sNode, uint8_t thrIdx)             { vEbwrs ew; return send(startNodeOrigin(sNode, thrIdx, ew));}//Requests thread <thrIdx> to start at node <sNode>
  int CarpeDM::startNodeOrigin(const std::string& sNode)                             { vEbwrs ew; return send(startNodeOrigin(sNode, ew));}//Requests a start at node <sNode>
  int CarpeDM::stopPattern(const std::string& sPattern)                              { vEbwrs ew; return send(stopPattern(sPattern, ew));}//Requests Pattern to stop
  int CarpeDM::stopNodeOrigin(const std::string& sNode)                              { vEbwrs ew; return send(stopNodeOrigin(sNode, ew));}//Requests stop at node <sNode> (flow to idle)
  int CarpeDM::abortPattern(const std::string& sPattern)                             { vEbwrs ew; return send(abortPattern(sPattern, ew));}//Immediately aborts a Pattern
  int CarpeDM::abortNodeOrigin(const std::string& sNode)                             { vEbwrs ew; return send(abortNodeOrigin(sNode, ew));}//Immediately aborts the thread whose pattern <sNode> belongs to
  int CarpeDM::abortThr(uint8_t cpuIdx, uint8_t thrIdx)                              { vEbwrs ew; return send(abortThr(cpuIdx, thrIdx, ew));} //Immediately aborts a Thread
  int CarpeDM::setThrStart(uint8_t cpuIdx, uint32_t bits)                            { vEbwrs ew; return send(setThrStart(cpuIdx, bits, ew));} //Requests Threads to start
  int CarpeDM::setThrAbort(uint8_t cpuIdx, uint32_t bits)                            { vEbwrs ew; return send(setThrAbort(cpuIdx, bits, ew));}//Immediately aborts Threads
  int CarpeDM::setThrOrigin(uint8_t cpuIdx, uint8_t thrIdx, const std::string& name) { vEbwrs ew; return send(setThrOrigin(cpuIdx, thrIdx, name, ew));}//Sets the Node the Thread will start from
  int CarpeDM::setThrStartTime(uint8_t cpuIdx, uint8_t thrIdx, uint64_t t)           { vEbwrs ew; return send(setThrStartTime(cpuIdx, thrIdx, t, ew));}
  int CarpeDM::setThrPrepTime(uint8_t cpuIdx, uint8_t thrIdx, uint64_t t)            { vEbwrs ew; return send(setThrPrepTime(cpuIdx, thrIdx, t, ew));}
*/

  void CarpeDM::showUp(bool filterMeta) {show("Upload Table", "upload_dict.txt", TransferDir::UPLOAD, false);} //show a CPU's Upload address table
  void CarpeDM::showDown(bool filterMeta) {  //show a CPU's Download address table
    show("Download Table" + (filterMeta ? std::string(" (noMeta)") : std::string("")), "download_dict.txt", TransferDir::DOWNLOAD, filterMeta);
  }

  void CarpeDM::updateModTime() { modTime = getDmWrTime(); }
