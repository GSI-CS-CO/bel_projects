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


int CarpeDM::ebWriteCycle(Device& dev, vAdr va, vBuf& vb)
{
   //eb_status_t status;
   //FIXME What about MTU? What about returned eb status ??
   Cycle cyc;
   ebBuf veb = ebBuf(va.size());

   for(int i = 0; i < (va.end()-va.begin()); i++) {
     uint32_t data = vb[i*4 + 0] << 24 | vb[i*4 + 1] << 16 | vb[i*4 + 2] << 8 | vb[i*4 + 3];
     veb[i] = data;
   } 

   cyc.open(dev);
   for(int i = 0; i < (veb.end()-veb.begin()); i++) {
    //FIXME dirty break into cycles
    if (i && ((va[i] & (RAM_SIZE-1)) ^ (va[i-1] & (RAM_SIZE-1)))) {
      cyc.close();
      cyc.open(dev);  
    }
    cyc.write(va[i], EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)veb[i]);

   }
   cyc.close();
   
   return 0;
}

vBuf CarpeDM::ebReadCycle(Device& dev, vAdr va)
{
   //FIXME What about MTU? What about returned eb status ??
   Cycle cyc;
   uint32_t veb[va.size()];
   vBuf ret   = vBuf(va.size() * 4);
      
   //sLog << "Got Adr Vec with " << va.size() << " Adrs" << std::endl;

   cyc.open(dev);
   for(int i = 0; i < (va.end()-va.begin()); i++) {
    //FIXME dirty break into cycles
    if (i && ((va[i] & (RAM_SIZE-1)) ^ (va[i-1] & (RAM_SIZE-1)))) {
      cyc.close();
      cyc.open(dev);  
    }
    cyc.read(va[i], EB_BIG_ENDIAN | EB_DATA32, (eb_data_t*)&veb[i]);
   }
   cyc.close();

  for(unsigned int i = 0; i < va.size(); i++) { 
    ret[i * 4 + 0] = (uint8_t)(veb[i] >> 24);
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
   cyc.open(dev);
   cyc.write(adr, EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)data);
   cyc.close();
   
   return 0;
}

uint32_t CarpeDM::ebReadWord(Device& dev, uint32_t adr)
{
   //FIXME this sometimes led to memory corruption by eb's handling of &data - investigate !!!
   uint32_t data, ret;

   Cycle cyc;
   cyc.open(dev);
   cyc.read(adr, EB_BIG_ENDIAN | EB_DATA32, (eb_data_t*)&data);
   cyc.close();
   
   ret = data;

   return ret;
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
    vFw.clear();

    if(verbose) sLog << "Connecting to " << en << "... ";
    try { 
      ebs.open(0, EB_DATAX|EB_ADDRX);
      ebd.open(ebs, ebdevname.c_str(), EB_DATAX|EB_ADDRX, 3);
      ebd.sdb_find_by_identity(SDB_VENDOR_GSI,SDB_DEVICE_LM32_RAM, myDevs);
      if (myDevs.size() >= 1) { 
        cpuQty = myDevs.size();

        for(int cpuIdx = 0; cpuIdx< cpuQty; cpuIdx++) {
          //only create MemUnits for valid DM CPUs, generate Mapping so we can still use the cpuIdx supplied by User 
          foundVersion = getFwVersion(cpuIdx);

          vFw.push_back(foundVersion);
          if (expVersion <= foundVersion) {
            cpuIdxMap[cpuIdx]    = mappedIdx;
            uint32_t extBaseAdr   = myDevs[cpuIdx].sdb_component.addr_first;
            uint32_t intBaseAdr   = getIntBaseAdr(cpuIdx);
            uint32_t peerBaseAdr  = WORLD_BASE_ADR  + extBaseAdr; 
            uint32_t sharedOffs   = getSharedOffs(cpuIdx) + _SHCTL_END_; 
            uint32_t space        = getSharedSize(cpuIdx) - _SHCTL_END_;
                        
              atUp.addMemory(cpuIdx, extBaseAdr, intBaseAdr, peerBaseAdr, sharedOffs, space );
            atDown.addMemory(cpuIdx, extBaseAdr, intBaseAdr, peerBaseAdr, sharedOffs, space );
            mappedIdx++;
          }
           
        }  
        ret = true;
      }
    } catch(...) {
      //TODO report why we could not connect / find CPUs
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
    } catch(...) {
      //TODO report why we could not disconnect
    }
    if(verbose) sLog << " Done" << std::endl;
    return ret;
  }

  void CarpeDM::addDotToDict(const std::string& fn) {
    Graph g;
    std::ifstream in(fn);
    boost::dynamic_properties dp = createParser(g);
   

    if(in.good()) {
      if(verbose) sLog << "Creating Dictionary from " << fn << " ... ";
      try { boost::read_graphviz(in,g,dp,"node_id");}
      catch(...) {
        throw;
        //TODO report why parsing the dot / creating the graph failed
      }
    }  
    else {throw std::runtime_error("Could not open .dot file <" + fn + "> for reading!\n"); return;}  

    //add to dictionary
    try {
      //FIXME using a qualified node name by concatenating the graphname is nice in theory, but prevents replicable hashing from download, so it's out
      //combine node name and graph name to obtain unique replicable hash
      //BOOST_FOREACH( vertex_t v, vertices(g) ) { hm.add(boost::get_property(g, boost::graph_name) + "." + g[v].name);}


      BOOST_FOREACH( vertex_t v, vertices(g) ) { hm.add(g[v].name);}
    }  catch (...) {
      //TODO report hash collision and show which names are responsible
      throw;
    }
    in.close();

    if(verbose) sLog << " Done." << std::endl;
  }

  void CarpeDM::removeDotFromDict(const std::string& fn) {
    Graph g;
    std::ifstream in(fn);
    boost::dynamic_properties dp = createParser(g);
   

    if(in.good()) {
      if(verbose) sLog << "Removing " << fn << " from dictionary ... ";
      try { boost::read_graphviz(in,g,dp,"node_id");}
      catch(...) {
        throw;
        //TODO report why parsing the dot / creating the graph failed
      }
    }  
    else {throw std::runtime_error("Could not open .dot file <" + fn + "> for reading!\n"); return;}  

    //see addDotToDict
    //BOOST_FOREACH( vertex_t v, vertices(g) ) { hm.remove(boost::get_property(g, boost::graph_name) + "." + g[v].name);}
    BOOST_FOREACH( vertex_t v, vertices(g) ) { hm.remove(g[v].name);}
    
    in.close();

    if(verbose) sLog << " Done." << std::endl;
  }

  void CarpeDM::clearDict() {
    hm.clear();
  }


  boost::dynamic_properties CarpeDM::createParser(Graph& g) {

    boost::dynamic_properties dp(boost::ignore_other_properties);

    boost::ref_property_map<Graph *, std::string> gname(boost::get_property(g, boost::graph_name));
    dp.property("name",     gname);
    dp.property("type",     boost::get(&myEdge::type,       g));
    dp.property("node_id",  boost::get(&myVertex::name,     g));
    dp.property("cpu",      boost::get(&myVertex::cpu,      g));
    dp.property("type",     boost::get(&myVertex::type,     g));
    dp.property("flags",    boost::get(&myVertex::flags,    g));
    dp.property("tPeriod",  boost::get(&myVertex::tPeriod,  g));
    dp.property("tOffs",    boost::get(&myVertex::tOffs,    g));
    dp.property("id",       boost::get(&myVertex::id,       g));
    //ID sub fields
    dp.property("fid",      boost::get(&myVertex::id_fid,   g));
    dp.property("gid",      boost::get(&myVertex::id_gid,   g));
    dp.property("evtno",    boost::get(&myVertex::id_evtno, g));
    dp.property("sid",      boost::get(&myVertex::id_sid,   g));
    dp.property("bpid",     boost::get(&myVertex::id_bpid,  g));
    dp.property("resid",    boost::get(&myVertex::id_res,  g));
    
    dp.property("par",      boost::get(&myVertex::par,      g));
    dp.property("tef",      boost::get(&myVertex::tef,      g));
    dp.property("res",      boost::get(&myVertex::res,      g));
    dp.property("tValid",   boost::get(&myVertex::tValid,   g));
    dp.property("prio",     boost::get(&myVertex::prio,     g));
    dp.property("qty",      boost::get(&myVertex::qty,      g));
    dp.property("tWait",    boost::get(&myVertex::tWait,    g));

    dp.property("qIl",      boost::get(&myVertex::qIl,   g));
    dp.property("qHi",      boost::get(&myVertex::qHi,   g));
    dp.property("qLo",      boost::get(&myVertex::qLo,   g));

    return (const boost::dynamic_properties)dp;
  }  

    Graph& CarpeDM::parseDot(const std::string& fn, Graph& g) {
    
    std::ifstream in(fn);
    boost::dynamic_properties dp = createParser(g);
    if(in.good()) {
      if(verbose) sLog << "Parsing .dot file " << fn << "... ";
      try { boost::read_graphviz(in, g, dp, "node_id"); }
      catch(...) {
        throw;

      }
      in.close();
    }  
    else {throw std::runtime_error(" Could not read from .dot file '" + fn + "'"); return g;}  
    //format all graph labels lowercase

 

    

    boost::graph_traits<Graph>::vertex_iterator vi, vi_end;
    boost::tie(vi, vi_end) = vertices(g);   
    
    BOOST_FOREACH( edge_t e, edges(g) ) { std::transform(g[e].type.begin(), g[e].type.end(), g[e].type.begin(), ::tolower); }
    BOOST_FOREACH( vertex_t v, vertices(g) ) { std::transform(g[v].type.begin(), g[v].type.end(), g[v].type.begin(), ::tolower); } 
    

    //removed, see addDotToDict
    //if (boost::get_property(g, boost::graph_name) == "") {throw std::runtime_error(" Graph attribute 'name' must not be empty "); return g;} 
    
    //TODO automatically add necessary meta nodes
     if(verbose) sLog << "... retrieved Graph " << boost::get_property(g, boost::graph_name) << "... Done." << std::endl;
    return g;

  }


  int CarpeDM::parseFwVersionString(const std::string& s) {
    
    int verMaj, verMin, verRev;
    std::vector<std::string> x;



    try { boost::split(x, s, boost::is_any_of(".")); } catch (...) {};
    if (x.size() != 3) {return FWID_BAD_VERSION_FORMAT;}

    verMaj = std::stoi (x[VERSION_MAJOR]);
    verMin = std::stoi (x[VERSION_MINOR]);
    verRev = std::stoi (x[VERSION_REVISION]);
    
    if (verMaj < 0 || verMaj > 99 || verMin < 0 || verMin > 99 || verRev < 0 || verRev > 99) {return  FWID_BAD_VERSION_FORMAT;}
    else {return verMaj * VERSION_MAJOR_MUL + verMin * VERSION_MINOR_MUL  + verRev * VERSION_REVISION_MUL;}


  }

  //returns firmware version as int <xxyyzz> (x Major Version, y Minor Version, z Revison; negative values for error codes)
  int CarpeDM::getFwVersion(uint8_t cpuIdx) {
    const std::string tagMagic      = "UserLM32";
    const std::string tagProject    = "Project     : ";
    const std::string tagExpName    = "ftm";
    const std::string tagVersion    = "Version     : ";
    const std::string tagVersionEnd = "Platform    : ";
    std::string version;
    size_t pos, posEnd;
    struct  sdb_device& ram = myDevs.at(cpuIdx);
    vAdr fwIdAdr;

    if ((ram.sdb_component.addr_last - ram.sdb_component.addr_first + 1) < SHARED_OFFS) { return FWID_RAM_TOO_SMALL;}

    for (uint32_t adr = ram.sdb_component.addr_first + BUILDID_OFFS; adr < ram.sdb_component.addr_first + SHARED_OFFS; adr += 4) fwIdAdr.push_back(adr);
    vBuf fwIdData = ebReadCycle(ebd, fwIdAdr);
    std::string s(fwIdData.begin(),fwIdData.end());

    //check for magic word
    pos = 0;
    if(s.find(tagMagic, 0) == std::string::npos) {return FWID_BAD_MAGIC;} 
    //check for project name
    pos = s.find(tagProject, 0);
    if (pos == std::string::npos || (s.find(tagExpName, pos + tagProject.length()) != pos + tagProject.length())) {return FWID_BAD_PROJECT_NAME;} 
    //get Version string xx.yy.zz    
    pos = s.find(tagVersion, 0);
    posEnd = s.find(tagVersionEnd, pos + tagVersion.length());
    if((pos == std::string::npos) || (posEnd == std::string::npos)) {return FWID_NOT_FOUND;}
    version = s.substr(pos + tagVersion.length(), posEnd - (pos + tagVersion.length()));
    
    int ret = parseFwVersionString(version);

    return ret;
  }


  uint8_t CarpeDM::getNodeCpu(const std::string& name, bool direction) {
     
    AllocTable& at = (direction == UPLOAD ? atUp : atDown );
    uint32_t hash;
    if (!(hm.lookup(name))) {throw std::runtime_error( "Unknown Node Name"); return -1;} 
    hash = hm.lookup(name).get(); //just pass it on
    
    auto* x = at.lookupHash(hash);
    if (x == NULL)  {throw std::runtime_error( "Could not find Node in allocation table"); return -1;}
    
    return x->cpu;
  }

  uint32_t CarpeDM::getNodeAdr(const std::string& name, bool direction, bool intExt) {
     
    AllocTable& at = (direction == UPLOAD ? atUp : atDown );
    uint32_t hash;
    if (!(hm.lookup(name))) {throw std::runtime_error( "Unknown Node Name"); return LM32_NULL_PTR;} 
    hash = hm.lookup(name).get(); //just pass it on
    auto* x = at.lookupHash(hash);
    if (x == NULL)  {throw std::runtime_error( "Could not find Node in allocation table"); return LM32_NULL_PTR;}
    else            {return (intExt == INTERNAL ? at.adr2intAdr(x->cpu, x->adr) : at.adr2extAdr(x->cpu, x->adr));}
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
  bool CarpeDM::isValid(const uint32_t hash)  {

    if (atDown.lookupHash(hash) != NULL) return true;
    else return false;
  }

  bool CarpeDM::isValid(const std::string& name) {
    if (!(hm.contains(name))) return false;
    return (atDown.lookupHash(hm.lookup(name).get()) != NULL);
  }  
