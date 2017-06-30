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
#include "memunit.h"
#include "ftm_shared_mmap.h"
#include "graph.h"
#include "carpeDM.h"
#include "minicommand.h"




const unsigned char CarpeDM::deadbeef[4] = {0xDE, 0xAD, 0xBE, 0xEF};
const std::string CarpeDM::needle(CarpeDM::deadbeef, CarpeDM::deadbeef + 4);

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


bool CarpeDM::connect(const std::string& en) {
    ebdevname = std::string(en); //copy to avoid mem trouble later
    bool  ret = false;
    uint8_t mappedIdx = 0;
    int expVersion = parseFwVersionString(EXP_VER), foundVersion;

    if (expVersion < 0) {throw("Bad minimum firmware version string received from Makefile"); return false;}

    vM.clear();
    cpuIdxMap.clear();

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
          if (expVersion <= foundVersion) {
            cpuIdxMap[cpuIdx] = mappedIdx; 
            vM.push_back(MemUnit(cpuIdx, myDevs[cpuIdx].sdb_component.addr_first, INT_BASE_ADR,  SHARED_OFFS + _SHCTL_END_ , SHARED_SIZE - _SHCTL_END_, hm));
            mappedIdx++;
          } else {if(verbose) sErr  << std::endl << "CPU " << cpuIdx << " has no/incompatible firmware (found " << getFwVersion(cpuIdx) << ", expected " << expVersion << ")" << std::endl; }
           
        }  
        ret = true;
      }
    } catch(...) {
      //TODO report why we could not connect / find CPUs
    }

    if(verbose) sLog << " Done."  << std::endl << "Found " << getCpuQty() << " Cores, " << cpuIdxMap.size() << " of them run a valid DM firmware." << std::endl;

    if (cpuIdxMap.size() == 0) {throw std::runtime_error("No valid CPUs running a valid DM firmware found"); return false;}


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
      BOOST_FOREACH( vertex_t v, vertices(g) ) { hm.add(g[v].name);}
    }  catch (...) {
      //TODO report hash collision and show which names are responsible
      throw;
    }
    in.close();

    if(verbose) sLog << " Done." << std::endl;
  }

  void CarpeDM::clearDict() {
    hm.clear();
  }


  boost::dynamic_properties CarpeDM::createParser(Graph& g) {

    boost::dynamic_properties dp(boost::ignore_other_properties);
    
    dp.property("type",     boost::get(&myEdge::type,       g));
    dp.property("node_id",  boost::get(&myVertex::name,     g));
    dp.property("type",     boost::get(&myVertex::type,     g));
    dp.property("flags",    boost::get(&myVertex::flags,    g));
    dp.property("tPeriod",  boost::get(&myVertex::tPeriod,  g));
    dp.property("tOffs",    boost::get(&myVertex::tOffs,    g));
    dp.property("id",       boost::get(&myVertex::id,       g));
    dp.property("par",      boost::get(&myVertex::par,      g));
    dp.property("tef",      boost::get(&myVertex::tef,      g));
    dp.property("res",      boost::get(&myVertex::res,      g));
    dp.property("tValid",   boost::get(&myVertex::tValid,   g));
    dp.property("prio",     boost::get(&myVertex::prio,     g));
    dp.property("qty",      boost::get(&myVertex::qty,      g));
    dp.property("tWait",    boost::get(&myVertex::tWait,    g));

    return (const boost::dynamic_properties)dp;
  }  

  Graph& CarpeDM::parseUpDot(const std::string& fn, Graph& g) {
    
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
    
    //TODO create subgraphs as necessary

    //TODO automatically add necessary meta nodes
    if(verbose) sLog << "Done." << std::endl;
    return g;

  }


     //TODO NC analysis

  
  bool CarpeDM::prepareUploadToCpu(Graph& g, uint8_t cpuIdx) {
    MemUnit& m = vM.at(cpuIdx);
    if(verbose) sLog << "Calculating Upload Binary for CPU #" << cpuIdx << "... ";
    m.prepareUpload(g); 
    
    BOOST_FOREACH( vertex_t v, vertices(m.getUpGraph()) ) {
 

      std::string haystack(m.getUpGraph()[v].np->getB(), m.getUpGraph()[v].np->getB() + _MEM_BLOCK_SIZE);
      std::size_t n = haystack.find(needle);

      bool foundUninitialised = (n != std::string::npos);

      if(verbose || foundUninitialised) {
        sLog << std::endl;
        hexDump(m.getUpGraph()[v].name.c_str(), (void*)haystack.c_str(), _MEM_BLOCK_SIZE);
      }

      if(foundUninitialised) {
        throw std::runtime_error("Node '" + m.getUpGraph()[v].name + "'contains uninitialised elements!\nMisspelled/forgot a mandatory property in .dot file ?"); 
        return false;
      }  
    }

    if(verbose) sLog << "Done." << std::endl;

    return true;
  }

  int CarpeDM::upload(uint8_t cpuIdx) {
    if(verbose) sLog << "Uploading to CPU #" << cpuIdx << "... ";
    MemUnit& m = vM.at(cpuIdxMap.at(cpuIdx));
    vBuf vUlD = m.getUploadData();
    vAdr vUlA = m.getUploadAdrs(); 
    //Upload
    ebWriteCycle(ebd, vUlA, vUlD);
    if(verbose) sLog << "Done." << std::endl;
    return vUlD.size();
  }

  int CarpeDM::sendCmd(uint8_t cpuIdx, const std::string& targetName, uint8_t cmdPrio, mc_ptr mc) {
    MemUnit& m = vM.at(cpuIdxMap.at(cpuIdx));
    if(verbose) sLog << "Preparing Command to Block " << targetName << "on CPU #" << cpuIdx << "... ";
    vBuf vUlD;
    vAdr vUlA;
    uint32_t cmdWrInc, hash;
    uint8_t b[_T_CMD_SIZE_ + _32b_SIZE_];

    try {
      hash      = hm.lookup(targetName).get(); 
      vUlA      = m.getCmdWrAdrs(hash, cmdPrio);
      cmdWrInc  = m.getCmdInc(hash, cmdPrio);
      mc->serialise(b);
      writeLeNumberToBeBytes(b + (ptrdiff_t)_T_CMD_SIZE_, cmdWrInc);
      vUlD.insert( vUlD.end(), b, b + _T_CMD_SIZE_ + _32b_SIZE_);
      if(verbose) sLog << "Done." << std::endl << "Sending...";
      ebWriteCycle(ebd, vUlA, vUlD);
    } catch (...) {throw;}      

    
    if(verbose) sLog << "Done." << std::endl;
    return vUlD.size();
  }


  //TODO assign to CPUs/threads


   int CarpeDM::downloadAndParse(uint8_t cpuIdx) {
    
    MemUnit& m = vM.at(cpuIdxMap.at(cpuIdx)); 
    
    vAdr vDlBmpA;
    vBuf vDlD;

    //verify firmware version first
    //checkFwVersion(cpuIdx);
    if(verbose) sLog << "Downloading from CPU #" << std::dec << cpuIdx << "... ";
    vDlBmpA = m.getDownloadBMPAdrs();
    m.setDownloadBmp(ebReadCycle(ebd, vDlBmpA));
    vDlD = ebReadCycle(ebd, m.getDownloadAdrs());
    if(verbose) sLog << "Done." << std::endl << "Parsing ...";
    m.parseDownloadData(vDlD);
    if(verbose) sLog << "Done." << std::endl;
    return vDlD.size();
  }

  int CarpeDM::parseFwVersionString(const std::string& s) {
    
    int verMaj, verMin, verRev;
    std::vector<std::string> x;

    boost::split(x, s, boost::is_any_of("."));
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


 //write out dotfile from download graph of a memunit
 void CarpeDM::writeDownDot(const std::string& fn, MemUnit& m) {
    std::ofstream out(fn); 
    if(out.good()) {
      if (verbose) sLog << "Writing Output File " << fn << "... ";
      try { boost::write_graphviz(out, m.getDownGraph(), make_vertex_writer(boost::get(&myVertex::np, m.getDownGraph())), 
                          make_edge_writer(boost::get(&myEdge::type, m.getDownGraph())), sample_graph_writer{"Demo"},
                          boost::get(&myVertex::name, m.getDownGraph()));
      }
      catch(...) {throw;}
      out.close();
    }  
    else {throw std::runtime_error(" Could not write to .dot file '" + fn + "'"); return;} 
    if (verbose) sLog << "Done.";
  }

 

  uint32_t CarpeDM::getNodeAdr(uint8_t cpuIdx, const std::string& name, bool direction, bool intExt) {
    MemUnit& m = vM.at(cpuIdxMap.at(cpuIdx)); 
    AllocTable& at = (direction == UPLOAD ? m.getUpAllocTable() : m.getDownAllocTable() );
    uint32_t hash;

    try {hash = hm.lookup(name).get();} catch (...) {throw;} //just pass it on
    auto* x = at.lookupHash(hash);
    if (x == NULL)  {throw std::runtime_error( "Could not find Node in download address table"); return LM32_NULL_PTR;}
    else            {return (intExt == INTERNAL ? m.adr2intAdr(x->adr) : m.adr2extAdr(x->adr));}
  }

  //Returns the external address of a thread's command register area
  uint32_t CarpeDM::getThrCmdAdr(uint8_t cpuIdx) {
    return myDevs.at(cpuIdx).sdb_component.addr_first + SHARED_OFFS + SHCTL_THR_CTL;
  }

  //Returns the external address of a thread's initial node register 
  uint32_t CarpeDM::getThrInitialNodeAdr(uint8_t cpuIdx, uint8_t thrIdx) {
    return myDevs.at(cpuIdx).sdb_component.addr_first + SHARED_OFFS + SHCTL_THR_STA + thrIdx * _T_TS_SIZE_ + T_TS_NODE_PTR;
  }

  //Returns the external address of a thread's command register area
  uint32_t CarpeDM::getThrCurrentNodeAdr(uint8_t cpuIdx, uint8_t thrIdx) {
    return myDevs.at(cpuIdx).sdb_component.addr_first + SHARED_OFFS + SHCTL_THR_DAT + thrIdx * _T_TD_SIZE_ + T_TD_NODE_PTR;
  }
  

  //Sets the Node the Thread will start from
  void CarpeDM::setThrOrigin(uint8_t cpuIdx, uint8_t thrIdx, const std::string& name) {
     ebWriteWord(ebd, getThrInitialNodeAdr(cpuIdx, thrIdx), getNodeAdr(cpuIdx, name, DOWNLOAD, INTERNAL)) ;
  }

  //Returns the Node the Thread will start from
  const std::string CarpeDM::getThrOrigin(uint8_t cpuIdx, uint8_t thrIdx) {
     uint32_t adr;
     MemUnit& m = vM.at(cpuIdxMap.at(cpuIdx));
     adr = ebReadWord(ebd, getThrInitialNodeAdr(cpuIdx, thrIdx));

     if (adr == LM32_NULL_PTR) return "Idle";

     auto* x = m.getDownAllocTable().lookupAdr(m.intAdr2adr(adr));
     if (x != NULL) return m.getDownGraph()[x->v].name;
     else           return "Unknown";
  }

  const std::string CarpeDM::getThrCursor(uint8_t cpuIdx, uint8_t thrIdx) {
    uint32_t adr;
    MemUnit& m = vM.at(cpuIdxMap.at(cpuIdx));
    adr = ebReadWord(ebd, getThrCurrentNodeAdr(cpuIdx, thrIdx));

    if (adr == LM32_NULL_PTR) return "Idle";

    auto* x = m.getDownAllocTable().lookupAdr(m.intAdr2adr(adr));
    if (x != NULL) return m.getDownGraph()[x->v].name;
    else           return "Unknown";  
  }

  //Get bifield showing running threads
  uint32_t CarpeDM::getThrRun(uint8_t cpuIdx) {
    return ebReadWord(ebd, getThrCmdAdr(cpuIdx) + T_TC_RUNNING); 
  }


  //Requests Threads to start
  void CarpeDM::setThrStart(uint8_t cpuIdx, uint32_t bits) {
    ebWriteWord(ebd, getThrCmdAdr(cpuIdx) + T_TC_START, bits) ;
  }


  //Requests Threads to stop
  void CarpeDM::setThrStop(uint8_t cpuIdx, uint32_t bits) {
    ebWriteWord(ebd, getThrCmdAdr(cpuIdx) + T_TC_STOP, bits);
  }

  //hard abort, emergency only
  void CarpeDM::clrThrRun(uint8_t cpuIdx, uint32_t bits) {
    uint32_t state = ebReadWord(ebd, getThrCmdAdr(cpuIdx) + T_TC_RUNNING);
    ebWriteWord(ebd, getThrCmdAdr(cpuIdx) + T_TC_RUNNING, state & ~bits);
  }

  bool CarpeDM::isThrRunning(uint8_t cpuIdx, uint8_t thrIdx) {
    return (bool)(getThrRun(cpuIdx) & (1<< thrIdx));
  }

  //Requests Thread to start
  void CarpeDM::startThr(uint8_t cpuIdx, uint8_t thrIdx) {
    setThrStart(cpuIdx, (1<<thrIdx));    
  }

  //Requests Thread to stop
  void CarpeDM::stopThr(uint8_t cpuIdx, uint8_t thrIdx) {
    setThrStop(cpuIdx, (1<<thrIdx));    
  }

  //Immediately aborts a Thread
  void CarpeDM::abortThr(uint8_t cpuIdx, uint8_t thrIdx) {
    clrThrRun(cpuIdx, (1<<thrIdx));    
  }

  void CarpeDM::dumpQueue(uint8_t cpuIdx, const std::string& blockName, uint8_t cmdPrio) {
    MemUnit& m  = vM.at(cpuIdxMap.at(cpuIdx));
    Graph& g    = m.getDownGraph();

    uint64_t vTime, wTime;     
    uint32_t type, qty, prio, flPrio, flMode, act, dest, flRngHiLo, flRngIl;
    bool abs, perm, found;
 
    const std::string sPrio[] = {"      Low", "     High", "Interlock"};
    const std::string sType[] = {"Unknown", "   Noop", "   Flow", "  Flush", "   Wait"};
    boost::optional<std::string> name; 
    
    auto* block = m.getDownAllocTable().lookupHash(hm.lookup(blockName).get());

    Graph::out_edge_iterator out_begin, out_end, out_cur;
    boost::tie(out_begin, out_end) = out_edges(block->v,g);
    
    //Get Buffer List of requested priority
    for (out_cur = out_begin; out_cur != out_end; ++out_cur) { if (g[target(*out_cur,g)].np->isMeta() && g[*out_cur].type == sQM[cmdPrio]) {found = true; break;} }
    if (!(found)) {throw std::runtime_error("Block " + blockName + " does not have a " + sPrio[cmdPrio] + " queue"); return;}            
    auto* bufList = m.getDownAllocTable().lookupVertex(target(*out_cur,g));    
    if (bufList == NULL) {return;}
    

    boost::tie(out_begin, out_end) = out_edges(bufList->v,g);
    
    // Iterate Buffers
    for (out_cur = out_begin; out_cur != out_end; ++out_cur) {
      

      sLog << "Buffer " << g[target(*out_cur,g)].name << ": " << std::endl;

      hexDump(g[target(*out_cur,g)].name.c_str(), g[target(*out_cur,g)].np->getB(), _MEM_BLOCK_SIZE);

      //output commands
      for(int i=0; i< _MEM_BLOCK_SIZE / _T_CMD_SIZE_; i ++ ) {
        uint8_t* b = g[target(*out_cur,g)].np->getB();

        vTime = writeBeBytesToLeNumber<uint64_t>((uint8_t*)&b[i * _T_CMD_SIZE_ + T_CMD_TIME]);
        act   = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&b[i * _T_CMD_SIZE_ + T_CMD_ACT]);
        type = ( ((act >> ACT_TYPE_POS)  & ACT_TYPE_MSK) < _ACT_TYPE_END_ ? ((act >> ACT_TYPE_POS)  & ACT_TYPE_MSK) : ACT_TYPE_UNKNOWN);
        prio = ( ((act >> ACT_PRIO_POS)  & ACT_PRIO_MSK) < 3 ? ((act >> ACT_PRIO_POS)  & ACT_PRIO_MSK) : PRIO_LO);
        qty  = (act >> ACT_QTY_POS) & ACT_QTY_MSK;
        perm = (act >> ACT_CHP_POS) & ACT_CHP_MSK;
        //type specific
        abs = (act >> ACT_WAIT_ABS_POS) & ACT_WAIT_ABS_MSK;
        flPrio = (act >> ACT_FLUSH_PRIO_POS) & ACT_FLUSH_PRIO_MSK;
        flMode = (act >> ACT_FLUSH_MODE_POS) & ACT_FLUSH_MODE_MSK;
        wTime  = writeBeBytesToLeNumber<uint64_t>((uint8_t*)&b[i * _T_CMD_SIZE_ + T_CMD_WAIT_TIME]);
        dest   = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&b[i * _T_CMD_SIZE_ + T_CMD_FLOW_DEST]);

        //write output
        sLog << "Cmd #" << i << ": " << std::endl;
        if(type == ACT_TYPE_UNKNOWN) {sLog << "Unknown Format / not initialised" << std::endl; continue;}
        if(type == ACT_TYPE_NOOP || type == ACT_TYPE_FLOW) sLog << qty << " x ";
        else                                               sLog << "1 x ";
        sLog << sType[type] << " @ >" << vTime << " ns, " << sPrio[prio] << " priority";
        if (((type == ACT_TYPE_FLOW) || ((type == ACT_TYPE_WAIT) && !(abs))) && perm) sLog << ", changes are permanent" << std::endl;
        else sLog << ", changes are temporary" << std::endl;

        //type specific
        switch(type) {
          case ACT_TYPE_NOOP  : break;
          case ACT_TYPE_FLOW  : sLog << "Destination: ";
                                try { 
                                  auto* y = m.getDownAllocTable().lookupAdr(m.intAdr2adr(dest));
                                  name = hm.lookup(y->hash); 
                                } catch (...) {throw; name = "INVALID";}
                                sLog << name.get()  << std::endl; break;
          case ACT_TYPE_FLUSH : sLog << "Priority to Flush: " << flPrio << " Mode: " << flMode << std::endl; break;
          case ACT_TYPE_WAIT  : if (abs) {sLog << "Wait until " << wTime << std::endl;} else {sLog << "Make Block Period " << wTime << std::endl;} break;

        }
      }
    }
  }      

void CarpeDM::dumpNode(uint8_t cpuIdx, const std::string& name) {
  MemUnit& m  = vM.at(cpuIdxMap.at(cpuIdx));
  try {
    auto* n = m.getDownAllocTable().lookupHash(hm.lookup(name).get());  
    hexDump(m.getDownGraph()[n->v].name.c_str(), n->b, _MEM_BLOCK_SIZE); 
  } catch (...) {throw;}
}