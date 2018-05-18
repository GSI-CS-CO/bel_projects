#include <boost/shared_ptr.hpp>
#include <algorithm>  
#include <stdio.h>
#include <iostream>
#include <string>
#include <inttypes.h>
#include <boost/graph/graphviz.hpp>
#include <boost/algorithm/string.hpp>
#include <sys/time.h>

#include "common.h"
#include "propwrite.h"
#include "graph.h"
#include "carpeDM.h"
#include "minicommand.h"
#include "dotstr.h"
#include "idformat.h"
#include "lzmaCompression.h"


  namespace dgp = DotStr::Graph::Prop;
  namespace dnp = DotStr::Node::Prop;
  namespace dep = DotStr::Edge::Prop;


  Graph& CarpeDM::getUpGraph()   {return gUp;}   //Returns the Upload Graph for CPU <cpuIdx>
  Graph& CarpeDM::getDownGraph() {return gDown;} //Returns the Download Graph for CPU <cpuIdx>

vBuf CarpeDM::compress(const vBuf& in) {return lzmaCompress(in);}
vBuf CarpeDM::decompress(const vBuf& in) {return lzmaDecompress(in);}

void CarpeDM::simAdrTranslation (uint32_t a, uint8_t& cpu, uint32_t& arIdx) {
  //get cpu
  /*
   if (debug) sLog << "cpuQty "  << getCpuQty() << std::endl;
  for (uint8_t cpuIdx = 0; cpuIdx < getCpuQty(); cpuIdx++) {
    if (debug) sLog << "a : " << std::hex << a << ", cmpA " << simRamAdrMap[cpuIdx] << " cpu " << (int)cpuIdx << " arIdx "  << arIdx << std::endl;
    if (simRamAdrMap[cpuIdx] > a) break;
    cpu = cpuIdx;
  }
  */
  cpu = ((a >> 17) & 0x7) -1;
  arIdx = atDown.adrConv(AdrType::EXT, AdrType::MGMT, cpu, a) >> 2;
}

void CarpeDM::simRamWrite (uint32_t a, eb_data_t d) {
  uint8_t cpu = -1;
  uint32_t arIdx;
  if (debug) sLog << "cpu : " << (int)cpu << " arIdx " << std::hex << arIdx << std::endl;
  simAdrTranslation (a, cpu, arIdx);
  simRam[cpu][arIdx] = d;
}


void CarpeDM::simRamRead (uint32_t a, eb_data_t* d) {
  uint8_t cpu;
  uint32_t arIdx;
  simAdrTranslation (a, cpu, arIdx);
  *d = simRam[cpu][arIdx];
}

int CarpeDM::simWriteCycle(vAdr va, vBuf& vb) {
  if (debug) sLog << "Starting Write Cycle" << std::endl;
  eb_data_t veb[va.size()];

  for(int i = 0; i < (va.end()-va.begin()); i++) {
   uint32_t data = vb[i*4 + 0] << 24 | vb[i*4 + 1] << 16 | vb[i*4 + 2] << 8 | vb[i*4 + 3];
   veb[i] = (eb_data_t)data;
  } 
  
  
  for(int i = 0; i < (va.end()-va.begin()); i++) {
  
    if (debug) sLog << " Writing @ 0x" << std::hex << std::setfill('0') << std::setw(8) << va[i] << " : 0x" << std::hex << std::setfill('0') << std::setw(8) << veb[i] << std::endl;
    simRamWrite(va[i], veb[i]);
  }
  

  return 0;

}





vBuf CarpeDM::simReadCycle(vAdr va)
{
  

  eb_data_t veb[va.size()];
  vBuf ret = vBuf(va.size() * 4);
  if (debug) sLog << "Starting Read Cycle" << std::endl; 
  //sLog << "Got Adr Vec with " << va.size() << " Adrs" << std::endl;

  
  for(int i = 0; i < (va.end()-va.begin()); i++) {
    if (debug) sLog << " Reading @ 0x" << std::hex << std::setfill('0') << std::setw(8) << va[i] << std::endl;
    simRamRead(va[i], (eb_data_t*)&veb[i]);
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



int CarpeDM::ebWriteCycle(Device& dev, vAdr va, vBuf& vb, vBl vcs)
{
  if (sim) {return simWriteCycle(va, vb); }
  //eb_status_t status;
  //FIXME What about MTU? What about returned eb status ??

  if (debug) sLog << "Starting Write Cycle" << std::endl;
  Cycle cyc;
  eb_data_t veb[va.size()];

  for(int i = 0; i < (va.end()-va.begin()); i++) {
   uint32_t data = vb[i*4 + 0] << 24 | vb[i*4 + 1] << 16 | vb[i*4 + 2] << 8 | vb[i*4 + 3];
   veb[i] = (eb_data_t)data;
  } 
  try {
    cyc.open(dev);
    for(int i = 0; i < (va.end()-va.begin()); i++) {
    if (i && vcs.at(i)) {
      cyc.close();
      if (debug) sLog << "Close and open next Write Cycle" << std::endl;
      cyc.open(dev);  
    }
    
    if (debug) sLog << " Writing @ 0x" << std::hex << std::setfill('0') << std::setw(8) << va[i] << " : 0x" << std::hex << std::setfill('0') << std::setw(8) << veb[i] << std::endl;
    cyc.write(va[i], EB_BIG_ENDIAN | EB_DATA32, veb[i]);

    }
    cyc.close();
  } catch (etherbone::exception_t const& ex) {
    throw std::runtime_error("Etherbone " + std::string(ex.method) + " returned " + std::string(eb_status(ex.status)) + "\n" );
  }

   return 0;
}

int   CarpeDM::ebWriteCycle(Device& dev, vAdr va, vBuf& vb) {return  ebWriteCycle(dev, va, vb, leadingOne(va.size()));}



vBuf CarpeDM::ebReadCycle(Device& dev, vAdr va, vBl vcs)
{
  if (sim) {return simReadCycle(va); }
  //FIXME What about MTU? What about returned eb status ??


  Cycle cyc;
  eb_data_t veb[va.size()];
  vBuf ret = vBuf(va.size() * 4);
  if (debug) sLog << "Starting Read Cycle" << std::endl; 
  //sLog << "Got Adr Vec with " << va.size() << " Adrs" << std::endl;

  try {
    cyc.open(dev);
    for(int i = 0; i < (va.end()-va.begin()); i++) {
    //FIXME dirty break into cycles
    if (i && vcs.at(i)) {
      cyc.close();
      if (debug) sLog << "Close and open next Read Cycle" << std::endl; 
      cyc.open(dev);  
    }
    if (debug) sLog << " Reading @ 0x" << std::hex << std::setfill('0') << std::setw(8) << va[i] << std::endl;
    cyc.read(va[i], EB_BIG_ENDIAN | EB_DATA32, (eb_data_t*)&veb[i]);
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

vBuf CarpeDM::ebReadCycle(Device& dev, vAdr va) {return  ebReadCycle(dev, va, leadingOne(va.size()));}

int CarpeDM::ebWriteWord(Device& dev, uint32_t adr, uint32_t data)
{
  uint8_t b[_32b_SIZE_];
  writeLeNumberToBeBytes(b, data);
  vAdr vA({adr});
  vBuf vD(std::begin(b), std::end(b) );

  return ebWriteCycle(ebd, vA, vD);
}

uint32_t CarpeDM::ebReadWord(Device& dev, uint32_t adr)
{
  vAdr vA({adr});
  vBuf vD = ebReadCycle(ebd, vA);
  uint8_t* b = &vD[0];

  return writeBeBytesToLeNumber<uint32_t>(b); 
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

bool CarpeDM::simConnect() {
    ebdevname = "simDummy"; //copy to avoid mem trouble later
    uint8_t mappedIdx = 0;
    uint32_t const intBaseAdr   = 0x1000000;
    uint32_t const sharedSize   = 98304;
    uint32_t const rawSize      = 131072;
    uint32_t const sharedOffs   = 0x500; 
    uint32_t const devBaseAdr   = 0x4120000;
    cpuQty = 4;

    atUp.clear();
    atUp.removeMemories();
    gUp.clear();
    atDown.clear();
    atDown.removeMemories();
    gDown.clear();
    cpuIdxMap.clear();
    cpuDevs.clear();


    

    sLog << "Connecting to Sim... ";
    simRam.reserve(cpuQty); 
       
    for(int cpuIdx = 0; cpuIdx< cpuQty; cpuIdx++) {
      simRam[cpuIdx]        = new uint32_t [(rawSize + _32b_SIZE_ -1) >> 2];
      cpuIdxMap[cpuIdx]     = mappedIdx;
      uint32_t extBaseAdr   = devBaseAdr + cpuIdx * rawSize;
      simRamAdrMap[cpuIdx]  = extBaseAdr;
      uint32_t peerBaseAdr  = WORLD_BASE_ADR  + extBaseAdr;
      uint32_t space        = sharedSize - _SHCTL_END_;
                  
      atUp.addMemory(cpuIdx, extBaseAdr, intBaseAdr, peerBaseAdr, sharedOffs, space, rawSize );
      atDown.addMemory(cpuIdx, extBaseAdr, intBaseAdr, peerBaseAdr, sharedOffs, space, rawSize );
      mappedIdx++;
    }  
    sLog << "done" << std::endl;
    return true;

}


bool CarpeDM::connect(const std::string& en, bool simulation, bool test) {
    sim = simulation;
    testmode = test;
    simRam.clear();
    simRamAdrMap.clear();
    if (sim) {return simConnect(); }

    ebdevname = std::string(en); //copy to avoid mem trouble later
    bool  ret = false;
    uint8_t mappedIdx = 0;
    int expVersion = parseFwVersionString(EXP_VER), foundVersion;
    int foundVersionMax = -1;

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
      ebd.sdb_find_by_identity(ECA::vendID, ECA::devID, ecaDevs);
      if (ecaDevs.size() < 1) throw std::runtime_error("Could not find ECA on DM (needed for WR time). Something is wrong\n");
 


      ebd.sdb_find_by_identity(SDB_VENDOR_GSI,SDB_DEVICE_LM32_RAM, cpuDevs);
      if (cpuDevs.size() >= 1) { 
        cpuQty = cpuDevs.size();

        
        for(int cpuIdx = 0; cpuIdx< cpuQty; cpuIdx++) {
          //only create MemUnits for valid DM CPUs, generate Mapping so we can still use the cpuIdx supplied by User
          const std::string fwIdROM = getFwIdROM(cpuIdx);
          foundVersion = getFwVersion(fwIdROM);
          foundVersionMax = foundVersionMax < foundVersion ? foundVersion : foundVersionMax; 
          vFw.push_back(foundVersion);
          int expVersionMin = expVersion;
          int expVersionMax = (expVersion / (int)FwId::VERSION_MAJOR_MUL) * (int)FwId::VERSION_MAJOR_MUL 
                             + 99 * (int)FwId::VERSION_MINOR_MUL
                             + 99 * (int)FwId::VERSION_REVISION_MUL;
                         
          if ( (foundVersion >= expVersionMin) && (foundVersion <= expVersionMax) ) {
            //FIXME check for consequent use of cpu index map!!! I'm sure there'll be absolute chaos throughout the lib if CPUs indices were not continuous
            cpuIdxMap[cpuIdx]    = mappedIdx;
            
            uint32_t extBaseAdr   = cpuDevs[cpuIdx].sdb_component.addr_first;
            uint32_t intBaseAdr   = getIntBaseAdr(fwIdROM);
            uint32_t peerBaseAdr  = WORLD_BASE_ADR + extBaseAdr;
            uint32_t rawSize      = cpuDevs[cpuIdx].sdb_component.addr_last - cpuDevs[cpuIdx].sdb_component.addr_first;
            uint32_t sharedOffs   = getSharedOffs(fwIdROM); 
            uint32_t space        = getSharedSize(fwIdROM) - _SHCTL_END_;
                        
              atUp.addMemory(cpuIdx, extBaseAdr, intBaseAdr, peerBaseAdr, sharedOffs, space, rawSize );
            atDown.addMemory(cpuIdx, extBaseAdr, intBaseAdr, peerBaseAdr, sharedOffs, space, rawSize );
            mappedIdx++;
          }
          /*
          sLog << "#" << (int)cpuIdx << " Shared Offset 0x" << std::hex <<  atDown.getMemories()[cpuIdx].sharedOffs << std::endl;
          sLog << "#" << (int)cpuIdx << " BmpSize 0x" << std::hex <<  atDown.getMemories()[cpuIdx].bmpSize << std::endl;
          sLog << "#" << (int)cpuIdx << " SHCTL 0x" << std::hex <<  _SHCTL_END_ << std::endl;
          sLog << "#" << (int)cpuIdx << " Start Offset 0x" << std::hex <<  atDown.getMemories()[cpuIdx].startOffs << std::endl;
          */
        }  
        ret = true;
      }
    } catch (etherbone::exception_t const& ex) {
      throw std::runtime_error("Etherbone " + std::string(ex.method) + " returned " + std::string(eb_status(ex.status)) + "\n" );
    } catch(...) {
      throw std::runtime_error("Could not find CPUs running valid DM Firmware\n" );
    }

    if(verbose) {
      sLog << " Done."  << std::endl << "Found " << getCpuQty() << " Cores, " << cpuIdxMap.size() << " of them run a valid DM firmware." << std::endl;
    }
    std::string fwCause = foundVersionMax == -1 ? "" : "Requires FW v" + createFwVersionString(expVersion) + ", found " + createFwVersionString(foundVersionMax);
    if (cpuIdxMap.size() == 0) {throw std::runtime_error("No CPUs running a valid DM firmware found. " + fwCause);}


    return ret;

  }

  bool CarpeDM::disconnect() {


    bool ret = false;

    if(verbose) sLog << "Disconnecting ... ";
    if (sim) {simRam.clear(); ret = true;}
    else {
      try { 
        ebd.close();
        ebs.close();
        cpuQty = -1;
        ret = true;
      } catch (etherbone::exception_t const& ex) {
        throw std::runtime_error("Etherbone " + std::string(ex.method) + " returned " + std::string(eb_status(ex.status)) + "\n" );
        //TODO report why we could not disconnect
      }
    }
    if(verbose) sLog << " Done" << std::endl;
    sim = false;
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

  const std::string CarpeDM::createFwVersionString(const int fwVer) {
    
    unsigned int fwv = (unsigned int)fwVer;
    std::string ret;

    unsigned int verMaj = fwv / (unsigned int)FwId::VERSION_MAJOR_MUL; fwv %= (unsigned int)FwId::VERSION_MAJOR_MUL;
    unsigned int verMin = fwv / (unsigned int)FwId::VERSION_MINOR_MUL; fwv %= (unsigned int)FwId::VERSION_MINOR_MUL;
    unsigned int verRev = fwv;
    
    ret = std::to_string(verMaj) + "." + std::to_string(verMin) + "." + std::to_string(verRev);
    return ret;

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
  const std::string CarpeDM::getFwIdROM(uint8_t cpuIdx) {
    //FIXME replace with FW ID string constants
    const std::string tagMagic      = "UserLM32";
    const std::string tagProject    = "Project     : ";
    const std::string tagExpName    = "ftm";
    std::string version;
    size_t pos;
    struct  sdb_device& ram = cpuDevs.at(cpuIdx);
    vAdr fwIdAdr;
    //FIXME get rid of SHARED_OFFS somehow and replace with an end tag and max limit  
    for (uint32_t adr = ram.sdb_component.addr_first + BUILDID_OFFS; adr < ram.sdb_component.addr_first + BUILDID_OFFS + BUILDID_SIZE; adr += 4) fwIdAdr.push_back(adr);
    vBuf fwIdData = ebReadCycle(ebd, fwIdAdr);
    std::string s(fwIdData.begin(),fwIdData.end());

    //check for magic word
    pos = 0;
    if(s.find(tagMagic, 0) == std::string::npos) {throw std::runtime_error( "Bad Firmware Info ROM: Magic word not found\n");} 
    //check for project name
    pos = s.find(tagProject, 0);
    if (pos == std::string::npos || (s.find(tagExpName, pos + tagProject.length()) != pos + tagProject.length())) {throw std::runtime_error( "Bad Firmware Info ROM: Not a DM project\n");} 

    return s;
  }

  //returns firmware version as int <xxyyzz> (x Major Version, y Minor Version, z Revison; negative values for error codes)
  int CarpeDM::getFwVersion(const std::string& fwIdROM) {
    //FIXME replace with FW ID string constants
    //get Version string xx.yy.zz    

    std::string version = readFwIdROMTag(fwIdROM, "Version     : ", 10, true);
    
    int ret = parseFwVersionString(version);

    return ret;
  }


  const std::string CarpeDM::readFwIdROMTag(const std::string& fwIdROM, const std::string& tag, size_t maxlen, bool stopAtCr ) {
    size_t pos, posEnd, tmp;
    std::string s = fwIdROM;
  
    tmp = s.find(tag, 0);
    if(tmp == std::string::npos) throw std::runtime_error( "Could not find tag <" + tag + ">in FW ID ROM\n");
    pos = tmp + tag.length();  

    tmp = s.find("\n", pos);
    if( (tmp == std::string::npos) || (tmp > (pos + maxlen)) ) posEnd = (pos + maxlen);
    else posEnd = tmp;
    
    return s.substr(pos, posEnd - pos);


  }

  // SDB Functions
  bool CarpeDM::isValidDMCpu(uint8_t cpuIdx) {return (cpuIdxMap.count(cpuIdx) > 0);}; //Check if CPU is registered as running a valid firmware
  int CarpeDM::getCpuQty()   const {return cpuQty;} //Return number of found CPUs (not necessarily valid ones!)
  bool CarpeDM::isCpuIdxValid(uint8_t cpuIdx) { if ( cpuIdxMap.find(cpuIdx) != cpuIdxMap.end() ) return true; else return false;}  



  uint32_t CarpeDM::getIntBaseAdr(const std::string& fwIdROM) {
    //FIXME replace with FW ID string constants
    //CAREFUL: Get the EXACT position. If you miss out on leading spaces, the parsed number gets truncated!
    std::string value = readFwIdROMTag(fwIdROM, "IntAdrOffs  : ", 10, true);
    //sLog << "IntAdrOffs : " << value << " parsed: 0x" << std::hex << s2u<uint32_t>(value) << std::endl;
    return s2u<uint32_t>(value);

  }

  uint32_t CarpeDM::getSharedOffs(const std::string& fwIdROM) {
    //FIXME replace with FW ID string constants
    std::string value = readFwIdROMTag(fwIdROM, "SharedOffs  : ", 10, true);
    //sLog << "Parsing SharedOffs : " << value << " parsed: 0x" << std::hex << s2u<uint32_t>(value) << std::endl;
    return s2u<uint32_t>(value);

  }

  uint32_t CarpeDM::getSharedSize(const std::string& fwIdROM){
    std::string value = readFwIdROMTag(fwIdROM, "SharedSize  : ", 10, true);
    //sLog << "SharedSize : " << value << " parsed: "  << std::dec << s2u<uint32_t>(value) << std::endl;
    return s2u<uint32_t>(value);

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



 
void CarpeDM::showCpuList() {
  int expVersionMin = parseFwVersionString(EXP_VER);
  int expVersionMax = (expVersionMin / (int)FwId::VERSION_MAJOR_MUL) * (int)FwId::VERSION_MAJOR_MUL 
                   + 99 * (int)FwId::VERSION_MINOR_MUL
                   + 99 * (int)FwId::VERSION_REVISION_MUL;

  sLog << std::endl << std::setfill(' ') << std::setw(5) << "CPU" << std::setfill(' ') << std::setw(11) << "FW found" 
       << std::setfill(' ') << std::setw(11) << "Min" << std::setw(11) << "Max" << std::setw(11) << "Space" << std::setw(11) << "Free" << std::endl;
  for (int x = 0; x < cpuQty; x++) {
    
    sLog << std::dec << std::setfill(' ') << std::setw(5) << x << std::setfill(' ') << std::setw(11) << (sim ? "Sim" : createFwVersionString(vFw[x])) 
                                                                       << std::setfill(' ') << std::setw(11) << (sim ? "Sim" : createFwVersionString(expVersionMin))
                                                                       << std::setfill(' ') << std::setw(11) << (sim ? "Sim" : createFwVersionString(expVersionMax));
                                                                           
    sLog << std::dec << std::setfill(' ') << std::setw(11) << atDown.getTotalSpace(x) << std::setw(10) << atDown.getFreeSpace(x) * 100 / atDown.getTotalSpace(x) << "%";                                                                       
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
  int CarpeDM::addDot(const std::string& s) {Graph gTmp; return safeguardTransaction(&CarpeDM::add, parseDot(s, gTmp), false);};            
  int CarpeDM::addDotFile(const std::string& fn) {return addDot(readTextFile(fn));};                 
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
  int CarpeDM::sendCommandsDot(const std::string& s) {Graph gTmp; vEbwrs ew; return send(createCommandBurst(parseDot(s, gTmp), ew));}; //Sends a dotfile of commands to the DM
  int CarpeDM::sendCommandsDotFile(const std::string& fn) {Graph gTmp; vEbwrs ew; return send(createCommandBurst(parseDot(readTextFile(fn), gTmp), ew));};
  //Send a command to Block <targetName> on CPU <cpuIdx> via Etherbone
  int CarpeDM::sendCommand(const std::string& targetName, uint8_t cmdPrio, mc_ptr mc) {vEbwrs ew; return send(createCommand(targetName, cmdPrio, mc, ew));}; 


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
    /* get time from ECA */
    uint64_t wrTime;

    if (sim) {
      timeval ts;   
      gettimeofday(&ts, NULL);

      wrTime = (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_usec * 1000ULL;
      
      return wrTime;

    }
  
    eb_data_t    nsHi0, nsLo, nsHi1;
    Cycle cyc;


    uint32_t ecaAddr = ecaDevs[0].sdb_component.addr_first;
    
    do {
      try {
        cyc.open(ebd);
        cyc.read( ecaAddr + ECA::timeHiW, EB_BIG_ENDIAN|EB_DATA32, &nsHi0);
        cyc.read( ecaAddr + ECA::timeLoW, EB_BIG_ENDIAN|EB_DATA32, &nsLo);
        cyc.read( ecaAddr + ECA::timeHiW, EB_BIG_ENDIAN|EB_DATA32, &nsHi1);
        cyc.close();
      } catch (etherbone::exception_t const& ex) {
        throw std::runtime_error("Etherbone " + std::string(ex.method) + " returned " + std::string(eb_status(ex.status)) + "\n" );
      }
    } while (nsHi0 != nsHi1);
  
    /* time */
    wrTime = (uint64_t)nsHi0 << 32;
    wrTime = wrTime + (uint64_t)nsLo;
  


    return wrTime;
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


  void CarpeDM::showUp(bool filterMeta) {show("Upload Table", "upload_dict.txt", TransferDir::UPLOAD, false);} //show a CPU's Upload address table
  void CarpeDM::showDown(bool filterMeta) {  //show a CPU's Download address table
    show("Download Table" + (filterMeta ? std::string(" (noMeta)") : std::string("")), "download_dict.txt", TransferDir::DOWNLOAD, filterMeta);
  }

  void CarpeDM::updateModTime() { modTime = getDmWrTime(); } 
  