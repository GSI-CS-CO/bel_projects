#include <boost/shared_ptr.hpp>
#include <algorithm>  
#include <stdio.h>
#include <iostream>
#include <string>
#include <inttypes.h>
#include <boost/graph/graphviz.hpp>
#include <boost/algorithm/string.hpp>

#include "common.h"
#include "propwrite.h"
#include "graph.h"
#include "carpeDM.h"
#include "minicommand.h"
#include "dotstr.h"
#include "idformat.h"

  namespace dgp = DotStr::Graph::Prop;
  namespace dnp = DotStr::Node::Prop;
  namespace dep = DotStr::Edge::Prop;





int CarpeDM::ebWriteCycle(Device& dev, vAdr va, vBuf& vb)
{
  //eb_status_t status;
  //FIXME What about MTU? What about returned eb status ??
  Cycle cyc;
  eb_data_t veb[va.size()];

  for(int i = 0; i < (va.end()-va.begin()); i++) {
   uint32_t data = vb[i*4 + 0] << 24 | vb[i*4 + 1] << 16 | vb[i*4 + 2] << 8 | vb[i*4 + 3];
   veb[i] = (eb_data_t)data;
  } 
  try {
    cyc.open(dev);
    for(int i = 0; i < (va.end()-va.begin()); i++) {
    //FIXME dirty break into cycles
    if (i && ((va[i] & (RAM_SIZE-1)) ^ (va[i-1] & (RAM_SIZE-1)))) {
      cyc.close();
      cyc.open(dev);  
    }
    sLog << "Writing @ 0x" << std::hex << std::setfill('0') << std::setw(8) << va[i] << " : 0x" << std::hex << std::setfill('0') << std::setw(8) << veb[i] << std::endl;
    cyc.write(va[i], EB_BIG_ENDIAN | EB_DATA32, veb[i]);

    }
    cyc.close();
  } catch (etherbone::exception_t const& ex) {
    throw std::runtime_error("Etherbone " + std::string(ex.method) + " returned " + std::string(eb_status(ex.status)) + "\n" );
  }

   return 0;
}


vBuf CarpeDM::ebReadCycle(Device& dev, vAdr va)
{
  //FIXME What about MTU? What about returned eb status ??
  Cycle cyc;
  eb_data_t veb[va.size()];
  vBuf ret = vBuf(va.size() * 4);
    
  //sLog << "Got Adr Vec with " << va.size() << " Adrs" << std::endl;

  try {
    cyc.open(dev);
    for(int i = 0; i < (va.end()-va.begin()); i++) {
    //FIXME dirty break into cycles
    if (i && ((va[i] & (RAM_SIZE-1)) ^ (va[i-1] & (RAM_SIZE-1)))) {
      cyc.close();
      cyc.open(dev);  
    }
    cyc.read(va[i], EB_BIG_ENDIAN | EB_DATA32, veb + i);
    }
    cyc.close();

  } catch (etherbone::exception_t const& ex) {
    throw std::runtime_error("Etherbone " + std::string(ex.method) + " returned " + std::string(eb_status(ex.status)) + "\n" );
  }
  //FIXME use endian functions
  for(unsigned int i = 0; i < va.size(); i++) { 
    ret[i * 4]     = (uint8_t)(veb[i] >> 24);
    ret[i * 4 + 1] = (uint8_t)(veb[i] >> 16);
    ret[i * 4 + 2] = (uint8_t)(veb[i] >> 8);
    ret[i * 4 + 3] = (uint8_t)(veb[i] >> 0);
  } 

  return ret;
}

int CarpeDM::ebWriteWord(Device& dev, uint32_t adr, uint32_t data)
{
   Cycle cyc;
   //FIXME What about returned eb status ??
   sLog << "Writing @ 0x" << std::hex << std::setfill('0') << std::setw(8) << adr << " : 0x" << std::hex << std::setfill('0') << std::setw(8) << data << std::endl;
   try { 
     cyc.open(dev);
     cyc.write(adr, EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)data);
     cyc.close();
   } catch (etherbone::exception_t const& ex) {
     throw std::runtime_error("Etherbone " + std::string(ex.method) + " returned " + std::string(eb_status(ex.status)) + "\n" );
   }

   return 0;
}

uint32_t CarpeDM::ebReadWord(Device& dev, uint32_t adr)
{
  eb_data_t data;
  Cycle cyc;
  try {
    cyc.open(dev);
    cyc.read(adr, EB_BIG_ENDIAN | EB_DATA32, (eb_data_t*)&data);
    cyc.close();
  } catch (etherbone::exception_t const& ex) {
    throw std::runtime_error("Etherbone " + std::string(ex.method) + " returned " + std::string(eb_status(ex.status)) + "\n" );
  }
  return (uint32_t)data;
}

 //Reads and returns a 64 bit word from DM
uint64_t CarpeDM::read64b(uint32_t startAdr) {
  vAdr vA({startAdr + 0, startAdr + _32b_SIZE_});
  vBuf vD = ebReadCycle(ebd, vA);
  uint8_t* b = &vD[0];

  return writeBeBytesToLeNumber<uint64_t>(b); 
}

int CarpeDM::write64b(uint32_t startAdr, uint64_t d) {
  uint8_t b[_TS_SIZE_];
  writeLeNumberToBeBytes(b, d);
  vAdr vA({startAdr + 0, startAdr + _32b_SIZE_});
  vBuf vD(std::begin(b), std::end(b) );

  return ebWriteCycle(ebd, vA, vD);

}


bool CarpeDM::connect(const std::string& en) {
    ebdevname = std::string(en); //copy to avoid mem trouble later
    bool  ret = false;
    uint8_t mappedIdx = 0;
    int expVersion = parseFwVersionString(EXP_VER), foundVersion;

    if (expVersion <= 0) {throw std::runtime_error("Bad required minimum firmware version string received from Makefile"); return false;}

    atUp.clear();
    atUp.removeMemories();
    gUp.clear();
    atDown.clear();
    atDown.removeMemories();
    gDown.clear();
    cpuIdxMap.clear();
    cpuDevs.clear();

    vFw.clear();

    if(verbose) sLog << "Connecting to " << en << "... ";
    try { 
      ebs.open(0, EB_DATAX|EB_ADDRX);
      ebd.open(ebs, ebdevname.c_str(), EB_DATAX|EB_ADDRX, 3);
      ebd.sdb_find_by_identity(PPS::vendID, PPS::devID, ppsDev);
      if (ppsDev.size() < 1) throw std::runtime_error("Could not find a WR PPS Generator device. Something is wrong\n");
 


      ebd.sdb_find_by_identity(SDB_VENDOR_GSI,SDB_DEVICE_LM32_RAM, cpuDevs);
      if (cpuDevs.size() >= 1) { 
        cpuQty = cpuDevs.size();

        for(int cpuIdx = 0; cpuIdx< cpuQty; cpuIdx++) {
          //only create MemUnits for valid DM CPUs, generate Mapping so we can still use the cpuIdx supplied by User 
          foundVersion = getFwVersion(cpuIdx);

          vFw.push_back(foundVersion);
          if (expVersion <= foundVersion) {
            cpuIdxMap[cpuIdx]    = mappedIdx;
            uint32_t extBaseAdr   = cpuDevs[cpuIdx].sdb_component.addr_first;
            uint32_t intBaseAdr   = getIntBaseAdr(cpuIdx);
            uint32_t peerBaseAdr  = WORLD_BASE_ADR  + extBaseAdr;
            uint32_t rawSize      = cpuDevs[cpuIdx].sdb_component.addr_last - cpuDevs[cpuIdx].sdb_component.addr_first;
            uint32_t sharedOffs   = getSharedOffs(cpuIdx) + _SHCTL_END_; 
            uint32_t space        = getSharedSize(cpuIdx) - _SHCTL_END_;
                        
              atUp.addMemory(cpuIdx, extBaseAdr, intBaseAdr, peerBaseAdr, sharedOffs, space, rawSize );
            atDown.addMemory(cpuIdx, extBaseAdr, intBaseAdr, peerBaseAdr, sharedOffs, space, rawSize );
            mappedIdx++;
          }
           
        }  
        ret = true;
      }
    } catch (etherbone::exception_t const& ex) {
      throw std::runtime_error("Etherbone " + std::string(ex.method) + " returned " + std::string(eb_status(ex.status)) + "\n" );
    } catch(...) {
      throw std::runtime_error("Could not find CPUs running valid DM Firmware\n" );
    }

    if(verbose) {
      showCpuList();
      sLog << " Done."  << std::endl << "Found " << getCpuQty() << " Cores, " << cpuIdxMap.size() << " of them run a valid DM firmware." << std::endl;
    }  
    if (cpuIdxMap.size() == 0) {throw std::runtime_error("No CPUs running a valid DM firmware found"); return false;}


    return ret;

  }

  bool CarpeDM::disconnect() {
    bool ret = false;

    if(verbose) sLog << "Disconnecting ... ";
    try { 
      ebd.close();
      ebs.close();
      cpuQty = -1;
      ret = true;
    } catch (etherbone::exception_t const& ex) {
      throw std::runtime_error("Etherbone " + std::string(ex.method) + " returned " + std::string(eb_status(ex.status)) + "\n" );
      //TODO report why we could not disconnect
    }
    if(verbose) sLog << " Done" << std::endl;
    return ret;
  }




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


  int CarpeDM::parseFwVersionString(const std::string& s) {
    
    int verMaj, verMin, verRev;
    std::vector<std::string> x;



    try { boost::split(x, s, boost::is_any_of(".")); } catch (...) {};
    if (x.size() != 3) {return (int)FwId::FWID_BAD_VERSION_FORMAT;}

    verMaj = std::stoi (x[(int)FwId::VERSION_MAJOR]);
    verMin = std::stoi (x[(int)FwId::VERSION_MINOR]);
    verRev = std::stoi (x[(int)FwId::VERSION_REVISION]);
    
    if (verMaj < 0 || verMaj > 99 || verMin < 0 || verMin > 99 || verRev < 0 || verRev > 99) {return (int)FwId::FWID_BAD_VERSION_FORMAT;}
    else {return verMaj * (int)FwId::VERSION_MAJOR_MUL + verMin * (int)FwId::VERSION_MINOR_MUL  + verRev * (int)FwId::VERSION_REVISION_MUL;}


  }

  //returns firmware version as int <xxyyzz> (x Major Version, y Minor Version, z Revison; negative values for error codes)
  int CarpeDM::getFwVersion(uint8_t cpuIdx) {
    //FIXME replace with FW ID string constants
    const std::string tagMagic      = "UserLM32";
    const std::string tagProject    = "Project     : ";
    const std::string tagExpName    = "ftm";
    const std::string tagVersion    = "Version     : ";
    const std::string tagVersionEnd = "Platform    : ";
    std::string version;
    size_t pos, posEnd;
    struct  sdb_device& ram = cpuDevs.at(cpuIdx);
    vAdr fwIdAdr;

    if ((ram.sdb_component.addr_last - ram.sdb_component.addr_first + 1) < SHARED_OFFS) { return (int)FwId::FWID_RAM_TOO_SMALL;}

    for (uint32_t adr = ram.sdb_component.addr_first + BUILDID_OFFS; adr < ram.sdb_component.addr_first + SHARED_OFFS; adr += 4) fwIdAdr.push_back(adr);
    vBuf fwIdData = ebReadCycle(ebd, fwIdAdr);
    std::string s(fwIdData.begin(),fwIdData.end());

    //check for magic word
    pos = 0;
    if(s.find(tagMagic, 0) == std::string::npos) {return (int)FwId::FWID_BAD_MAGIC;} 
    //check for project name
    pos = s.find(tagProject, 0);
    if (pos == std::string::npos || (s.find(tagExpName, pos + tagProject.length()) != pos + tagProject.length())) {return (int)FwId::FWID_BAD_PROJECT_NAME;} 
    //get Version string xx.yy.zz    
    pos = s.find(tagVersion, 0);
    posEnd = s.find(tagVersionEnd, pos + tagVersion.length());
    if((pos == std::string::npos) || (posEnd == std::string::npos)) {return (int)FwId::FWID_NOT_FOUND;}
    version = s.substr(pos + tagVersion.length(), posEnd - (pos + tagVersion.length()));
    
    int ret = parseFwVersionString(version);

    return ret;
  }


  uint8_t CarpeDM::getNodeCpu(const std::string& name, TransferDir dir) {
     
    AllocTable& at = (dir == TransferDir::UPLOAD ? atUp : atDown );
    uint32_t hash;
    if (!(hm.lookup(name))) {throw std::runtime_error( "Unknown Node Name '" + name + "' when lookup up hosting cpu"); return -1;} 
    hash = hm.lookup(name).get(); //just pass it on
    
    auto x = at.lookupHash(hash);
    if (!(at.isOk(x)))  {throw std::runtime_error( "Could not find Node '" + name + "' in allocation table"); return -1;}
    
    return x->cpu;
  }

  uint32_t CarpeDM::getNodeAdr(const std::string& name, TransferDir dir, AdrType adrT) {
    sLog << "Looking up Adr of " << name << std::endl;
    if(name == DotStr::Node::Special::sIdle) return LM32_NULL_PTR; //idle node is resolved as a null ptr without comment

    AllocTable& at = (dir == TransferDir::UPLOAD ? atUp : atDown );
    uint32_t hash;
    if (!(hm.lookup(name))) {throw std::runtime_error( "Unknown Node Name '" + name + "' when lookup up address"); return LM32_NULL_PTR;} 
    hash = hm.lookup(name).get(); //just pass it on
    auto x = at.lookupHash(hash);
    if (!(at.isOk(x)))  {throw std::runtime_error( "Could not find Node in allocation table"); return LM32_NULL_PTR;}
    else {
      switch (adrT) {
        case AdrType::MGMT      : return x->adr; break;
        case AdrType::INT  : return at.adrConv(AdrType::MGMT, AdrType::INT, x->cpu, x->adr); break;
        case AdrType::EXT  : return at.adrConv(AdrType::MGMT, AdrType::EXT, x->cpu, x->adr); break;
        case AdrType::PEER      : return at.adrConv(AdrType::MGMT, AdrType::PEER, x->cpu, x->adr); break;
        default                 : throw std::runtime_error( "Unknown Adr Type conversion"); return LM32_NULL_PTR;
      }
    }  
  }

 
void CarpeDM::showCpuList() {
  int expVersion = parseFwVersionString(EXP_VER);

  sLog << std::setfill(' ') << std::setw(7) << "CPU" << std::setfill(' ') << std::setw(11) << "FW found" << std::setfill(' ') << std::setw(10) << "FW exp." << std::endl;
  for (int x = 0; x < cpuQty; x++) {
    if (vFw[x] > expVersion) sLog << " ! ";
    else if (vFw[x] < expVersion) sLog << " X ";
    else sLog << "   ";
    sLog << "  " << std::dec << std::setfill(' ') << std::setw(2) << x << "   " << std::setfill('0') << std::setw(6) << vFw[x] << "   " << std::setfill('0') << std::setw(6) << expVersion;
    sLog << std::endl;
  }

}

//Returns if a hash / nodename is present on DM
  bool CarpeDM::isInHashDict(const uint32_t hash)  {

    if (atDown.isOk(atDown.lookupHash(hash))) return true;
    else return false;
  }

  bool CarpeDM::isInHashDict(const std::string& name) {
    if (!(hm.contains(name))) return false;
    return (atDown.isOk(atDown.lookupHash(hm.lookup(name).get())));
  }  

  void CarpeDM::show(const std::string& title, const std::string& logDictFile, TransferDir dir, bool filterMeta ) {

    Graph& g        = (dir == TransferDir::UPLOAD ? gUp  : gDown);
    AllocTable& at  = (dir == TransferDir::UPLOAD ? atUp : atDown);

    sLog << std::endl << title << std::endl;
    sLog << std::endl << std::setfill(' ') << std::setw(4) << "Idx" << "   " << std::setfill(' ') << std::setw(4) << "S/R" << "   " << std::setfill(' ') << std::setw(4) << "Cpu" << "   " << std::setw(30) << "Name" << "   " << std::setw(10) << "Hash" << "   " << std::setw(10)  <<  "Int. Adr   "  << "   " << std::setw(10) << "Ext. Adr   " << std::endl;
    sLog << std::endl; 

    BOOST_FOREACH( vertex_t v, vertices(g) ) {
      auto x = at.lookupVertex(v);
      
      if( !(filterMeta) || (filterMeta & !(g[v].np->isMeta())) ) {
        sLog   << std::setfill(' ') << std::setw(4) << std::dec << v 
        << "   "    << std::setfill(' ') << std::setw(2) << std::dec << (int)(at.isOk(x) && (int)(at.isStaged(x)))  
        << " "      << std::setfill(' ') << std::setw(1) << std::dec << (int)(!(at.isOk(x)))
        << "   "    << std::setfill(' ') << std::setw(4) << std::dec << (at.isOk(x) ? (int)x->cpu : -1 )  
        << "   "    << std::setfill(' ') << std::setw(40) << std::left << g[v].name 
        << "   0x"  << std::hex << std::setfill('0') << std::setw(8) << (at.isOk(x) ? x->hash  : 0 )
        << "   0x"  << std::hex << std::setfill('0') << std::setw(8) << (at.isOk(x) ? at.adrConv(AdrType::MGMT, AdrType::INT, x->cpu, x->adr)  : 0 ) 
        << "   0x"  << std::hex << std::setfill('0') << std::setw(8) << (at.isOk(x) ? at.adrConv(AdrType::MGMT, AdrType::EXT, x->cpu, x->adr)  : 0 )  << std::endl;
      }
    }  

    sLog << std::endl;  
  }

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

  bool CarpeDM::validate(Graph& g, AllocTable& at) {
    try { 
          BOOST_FOREACH( vertex_t v, vertices(g) ) { Validation::neighbourhoodCheck(v, g);  }
          
          BOOST_FOREACH( vertex_t v, vertices(g) ) { 
            if (g[v].np == nullptr) throw std::runtime_error("Validation of Sequence: Node '" + g[v].name + "' was not allocated" );
            g[v].np->accept(VisitorValidation(g, v, at)); 
          }
    } catch (std::runtime_error const& err) { throw std::runtime_error("Validation of " + std::string(err.what()) ); }
    return true;
  }


  uint64_t CarpeDM::getDmWrTime() {
    uint32_t ppsAdr = ppsDev.at(0).sdb_component.addr_first;
    //uint32_t state;
    uint64_t wr_time;
    vAdr va;
    vBuf vb;
    uint8_t* b;
    uint8_t tmp;
  
    va.push_back(ppsAdr + PPS::CNTR_UTCLO_REG);
    va.push_back(ppsAdr + PPS::CNTR_UTCHI_REG);
    vb = ebReadCycle(ebd, va);
    //awkward: module excepts read on low word first, then high, which leaves us with words in wrong order. swap words
    
    b = (uint8_t*)&vb[0];
    //hexDump("b4", (const char*)b, 8);
    for(int i = 0; i<4; i++) {tmp = vb[4+i]; vb[4+i] = vb[i]; vb[i] = tmp;}
    for(int i = 0; i<3; i++) {vb[i] = 0;} // equal HiWord & 0xff. That wr pps gen is bloody awkward...

    //state   = writeBeBytesToLeNumber<uint32_t>(b + 0) & PPS::STATE_MSK;
    wr_time = writeBeBytesToLeNumber<uint64_t>(b);

    return wr_time;
  }


  