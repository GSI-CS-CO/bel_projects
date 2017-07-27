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
            cpuIdxMap[cpuIdx] = mappedIdx; 
              atUp.addMemory(cpuIdx, myDevs[cpuIdx].sdb_component.addr_first, INT_BASE_ADR, 0x80000000 + myDevs[cpuIdx].sdb_component.addr_first, SHARED_OFFS + _SHCTL_END_ , SHARED_SIZE - _SHCTL_END_ );
            atDown.addMemory(cpuIdx, myDevs[cpuIdx].sdb_component.addr_first, INT_BASE_ADR, 0x80000000 + myDevs[cpuIdx].sdb_component.addr_first, SHARED_OFFS + _SHCTL_END_ , SHARED_SIZE - _SHCTL_END_ );
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
      //combine node name and graph name to obtain unique replicable hash
      BOOST_FOREACH( vertex_t v, vertices(g) ) { hm.add(boost::get_property(g, boost::graph_name) + "." + g[v].name);}
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

    
    BOOST_FOREACH( vertex_t v, vertices(g) ) { hm.remove(boost::get_property(g, boost::graph_name) + "." + g[v].name);}
    
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
    


    //GraphNameMap gnameMap = get(boost::graph_name, g);
    //for (auto & it : gnameMap) sLog << " FOUND " << it << std::endl;
    //TODO create subgraphs as necessary
    if (boost::get_property(g, boost::graph_name) == "") {throw std::runtime_error(" Graph attribute 'name' must not be empty "); return g;} 
    //TODO automatically add necessary meta nodes
     if(verbose) sLog << "... retrieved Graph " << boost::get_property(g, boost::graph_name) << "... Done." << std::endl;
    return g;

  }


     //TODO NC analysis

  
  bool CarpeDM::prepareUploadToCpu(Graph& g, uint8_t cpuIdx, bool update) {
   
    if(verbose) sLog << "Calculating Upload Binary for CPU #" << cpuIdx << "... ";
    
    // this is the very basic version of a graph update: 
    // create completely disjunct graphs by letting the allocator know
    // which memory on the embedded system is already used
    atUp.clear();
    if (update) atUp.syncBmps(atDown);
    else        atUp.clearMemories(); 

    comManager.prepareUpload(g); 
    
    BOOST_FOREACH( vertex_t v, vertices(gUp) ) {
      if (update && isValid(cpuIdx, boost::get_property(gUp, boost::graph_name) + "." + gUp[v].name)) {
        throw std::runtime_error("Node '" + boost::get_property(gUp, boost::graph_name) + "." + gUp[v].name + "' already present on DM.\nThe combination <graphname.nodename> must be unique, duplicates are not allowed."); 
      }

      std::string haystack(gUp[v].np->getB(), gUp[v].np->getB() + _MEM_BLOCK_SIZE);
      std::size_t n = haystack.find(needle);

      bool foundUninitialised = (n != std::string::npos);

      if(verbose || foundUninitialised) {
        sLog << std::endl;
        hexDump(gUp[v].name.c_str(), (void*)haystack.c_str(), _MEM_BLOCK_SIZE);
      }

      if(foundUninitialised) {
        throw std::runtime_error("Node '" + gUp[v].name + "'contains uninitialised elements!\nMisspelled/forgot a mandatory property in .dot file ?"); 
        return false;
      }  
    }

    if(verbose) sLog << "Done." << std::endl;

    return true;
  }

  int CarpeDM::upload() {
    vBuf vUlD = comManager.getUploadData();
    vAdr vUlA = comManager.getUploadAdrs(); 
    //Upload
    ebWriteCycle(ebd, vUlA, vUlD);
    if(verbose) sLog << "Done." << std::endl;
    return vUlD.size();
  }

  int CarpeDM::removeDot(const std::string& fn) {
   
    Graph gTmp, gEmpty;
    parseUpDot(fn, gTmp); 
    uint32_t hash;

    //remove all nodes in input file from download allocation table
    try {
      //combine node name and graph name to obtain unique replicable hash
      BOOST_FOREACH( vertex_t v, vertices(gTmp) ) { 
        hash = hm.lookup(boost::get_property(gTmp, boost::graph_name) + "." + gTmp[v].name).get();
        if (!(atDown.deallocate(hash))) { if(verbose) {sLog << "Node " << boost::get_property(gTmp, boost::graph_name) + "." + gTmp[v].name << " could not be removed" << std::endl;}}
      }  
    }  catch (...) {
      //TODO report hash collision and show which names are responsible
      throw;
    }


    gUp.clear(); //create empty upload allocation table
    atUp.syncBmps(atDown); //use the bmps of the changed download allocation table for upload

    //because gUp Graph is empty, upload will only contain the reduced upload bmps, effectively deleting nodes
    return upload();

  }

  int CarpeDM::clear() {
    gUp.clear(); //Necessary?
    atUp.clear();
    atUp.clearMemories();
    return upload();

  }

  int CarpeDM::sendCmd(const std::string& targetName, uint8_t cmdPrio, mc_ptr mc) {
    if(verbose) sLog << "Preparing Command Prio " << cmdPrio << " to Block " << targetName << "... ";
    vBuf vUlD;
    vAdr vUlA;
    uint32_t cmdWrInc, hash;
    uint8_t b[_T_CMD_SIZE_ + _32b_SIZE_];

    try {
      hash      = hm.lookup(targetName).get(); 
      vUlA      = comManager.getCmdWrAdrs(hash, cmdPrio);
      cmdWrInc  = comManager.getCmdInc(hash, cmdPrio);
      mc->serialise(b);
      writeLeNumberToBeBytes(b + (ptrdiff_t)_T_CMD_SIZE_, cmdWrInc);
      vUlD.insert( vUlD.end(), b, b + _T_CMD_SIZE_ + _32b_SIZE_);
      if(verbose) sLog << "Done." << std::endl << "Sending to Q adr 0x" << std::hex << vUlA[0] << "...";
      ebWriteCycle(ebd, vUlA, vUlD);
    } catch (...) {throw;}      

    
    if(verbose) sLog << "Done." << std::endl;
    return vUlD.size();
  }


  //TODO assign to CPUs/threads


   int CarpeDM::downloadAndParse(uint8_t cpuIdx) {
    
     
    
    vAdr vDlBmpA;
    vBuf vDlD;

    //verify firmware version first
    //checkFwVersion(cpuIdx);
    if(verbose) sLog << "Downloading from CPU #" << std::dec << cpuIdx << "... ";
    vDlBmpA = comManager.getDownloadBMPAdrs();
    comManager.setDownloadBmp(ebReadCycle(ebd, vDlBmpA));
    vDlD = ebReadCycle(ebd, comManager.getDownloadAdrs());
    if(verbose) sLog << "Done." << std::endl << "Parsing ...";
    comManager.parseDownloadData(vDlD);
    if(verbose) sLog << "Done." << std::endl;
    return vDlD.size();
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


 //write out dotfile from download graph of a memunit
 void CarpeDM::writeDownDot(const std::string& fn, bool filterMeta) {
    std::ofstream out(fn);
    Graph& g = gDown;


    typedef boost::property_map< Graph, node_ptr myVertex::* >::type NpMap;

    boost::filtered_graph <Graph, boost::keep_all, non_meta<NpMap> > fg(g, boost::keep_all(), make_non_meta(boost::get(&myVertex::np, g)));

    if(out.good()) {
      if (verbose) sLog << "Writing Output File " << fn << "... ";
      try { 
            
            if (filterMeta) {
              boost::write_graphviz(out, fg, make_vertex_writer(boost::get(&myVertex::np, fg)), 
                          make_edge_writer(boost::get(&myEdge::type, fg)), sample_graph_writer{"Demo"},
                          boost::get(&myVertex::name, fg));
            }
            else {
            
              boost::write_graphviz(out, g, make_vertex_writer(boost::get(&myVertex::np, g)), 
                          make_edge_writer(boost::get(&myEdge::type, g)), sample_graph_writer{"Demo"},
                          boost::get(&myVertex::name, g));
            }
      }
      catch(...) {throw;}
      out.close();
    }  
    else {throw std::runtime_error(" Could not write to .dot file '" + fn + "'"); return;} 
    if (verbose) sLog << "Done.";
  }

 

  uint32_t CarpeDM::getNodeAdr(uint8_t cpuIdx, const std::string& name, bool direction, bool intExt) {
     
    AllocTable& at = (direction == UPLOAD ? atUp : atDown );
    Graph& g = gUp;
    uint32_t hash;

    try {hash = hm.lookup(boost::get_property(g, boost::graph_name) + name).get();} catch (...) {throw;} //just pass it on
    auto* x = at.lookupHash(hash);
    if (x == NULL)  {throw std::runtime_error( "Could not find Node in download address table"); return LM32_NULL_PTR;}
    else            {return (intExt == INTERNAL ? at.adr2intAdr(x->cpu, x->adr) : at.adr2extAdr(x->cpu, x->adr));}
  }

  //Returns the external address of a thread's command register area
  uint32_t CarpeDM::getThrCmdAdr(uint8_t cpuIdx) {
    return myDevs.at(cpuIdx).sdb_component.addr_first + SHARED_OFFS + SHCTL_THR_CTL;
  }

  //Returns the external address of a thread's initial node register 
  uint32_t CarpeDM::getThrInitialNodeAdr(uint8_t cpuIdx, uint8_t thrIdx) {
    return myDevs.at(cpuIdx).sdb_component.addr_first + SHARED_OFFS + SHCTL_THR_STA + thrIdx * _T_TS_SIZE_ + T_TS_NODE_PTR;
  }

  //Returns the external address of a thread's cursor pointer
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
     
     adr = ebReadWord(ebd, getThrInitialNodeAdr(cpuIdx, thrIdx));

     if (adr == LM32_NULL_PTR) return "Idle";

     auto* x = atDown.lookupAdr(cpuIdx, atDown.intAdr2adr(cpuIdx, adr));
     if (x != NULL) return gDown[x->v].name;
     else           return "Unknown";
  }

 

  const std::string CarpeDM::getThrCursor(uint8_t cpuIdx, uint8_t thrIdx) {
    uint32_t adr;
    
    adr = ebReadWord(ebd, getThrCurrentNodeAdr(cpuIdx, thrIdx));

    if (adr == LM32_NULL_PTR) return "Idle";

    auto* x = atDown.lookupAdr(cpuIdx, atDown.intAdr2adr(cpuIdx, adr));
    if (x != NULL) return gDown[x->v].name;
    else           return "Unknown";  
  }

  //Get bitfield showing running threads
  uint32_t CarpeDM::getThrRun(uint8_t cpuIdx) {
    return ebReadWord(ebd, getThrCmdAdr(cpuIdx) + T_TC_RUNNING); 
  }

  //Get bifield showing running threads
  uint32_t CarpeDM::getStatus(uint8_t cpuIdx) {
    return ebReadWord(ebd, myDevs.at(cpuIdx).sdb_component.addr_first + SHARED_OFFS + SHCTL_STATUS); 
  }

  void CarpeDM::inspectHeap(uint8_t cpuIdx) {
    vAdr vRa;
    vBuf heap;
    

    uint32_t baseAdr = myDevs.at(cpuIdx).sdb_component.addr_first + SHARED_OFFS;
    uint32_t heapAdr = baseAdr + SHCTL_HEAP;
    uint32_t thrAdr  = baseAdr + SHCTL_THR_DAT;

    for(int i=0; i<_THR_QTY_; i++) vRa.push_back(heapAdr + i * _PTR_SIZE_);
    heap = ebReadCycle(ebd, vRa);


    sLog << std::setfill(' ') << std::setw(4) << "Rank  " << std::setfill(' ') << std::setw(5) << "Thread  " << std::setfill(' ') << std::setw(21) 
    << "Deadline  " << std::setfill(' ') << std::setw(21) << "Origin  " << std::setfill(' ') << std::setw(21) << "Cursor" << std::endl;



    for(int i=0; i<_THR_QTY_; i++) {

      uint8_t thrIdx = (writeBeBytesToLeNumber<uint32_t>((uint8_t*)&heap[i * _PTR_SIZE_])  - atDown.extAdr2intAdr(cpuIdx, thrAdr)) / _T_TD_SIZE_;
      sLog << std::dec << std::setfill(' ') << std::setw(4) << i << std::setfill(' ') << std::setw(8) << (int)thrIdx  
      << std::setfill(' ') << std::setw(21) << getThrDeadline(cpuIdx, thrIdx)   << std::setfill(' ') << std::setw(21) 
      << getThrOrigin(cpuIdx, thrIdx)  << std::setfill(' ') << std::setw(21) << getThrCursor(cpuIdx, thrIdx) << std::endl;
    }  
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
    
    Graph& g    = gUp;

    uint64_t vTime, wTime;     
    uint32_t type, qty, prio, flPrio, flMode, act, dest;// flRngHiLo, flRngIl;
    bool abs, perm, found;
 
    const std::string sPrio[] = {"      Low", "     High", "Interlock"};
    const std::string sType[] = {"Unknown", "   Noop", "   Flow", "  Flush", "   Wait"};
    boost::optional<std::string> name; 
    
    //FIXME the safeguards for the maps are total crap. Include some decent checks, not everything is worth an exception!!!
    auto* block = atDown.lookupHash(hm.lookup(blockName).get());
    sLog << std::endl;

    sLog << "     IlHiLo" << std::endl;
    sLog << "WR 0x" << std::setfill('0') << std::setw(6) << std::hex << writeBeBytesToLeNumber<uint32_t>((uint8_t*)&block->b[BLOCK_CMDQ_WR_IDXS]) << std::endl;
    sLog << "RD 0x" << std::setfill('0') << std::setw(6) << std::hex << writeBeBytesToLeNumber<uint32_t>((uint8_t*)&block->b[BLOCK_CMDQ_RD_IDXS]) << std::endl;

    Graph::out_edge_iterator out_begin, out_end, out_cur;
    boost::tie(out_begin, out_end) = out_edges(block->v,g);
    
    //Get Buffer List of requested priority
    for (out_cur = out_begin; out_cur != out_end; ++out_cur) { if (g[target(*out_cur,g)].np->isMeta() && g[*out_cur].type == sQM[cmdPrio]) {found = true; break;} }
    if (!(found)) {throw std::runtime_error("Block " + blockName + " does not have a " + sPrio[cmdPrio] + " queue"); return;}            
    auto* bufList = atDown.lookupVertex(target(*out_cur,g));    
    if (bufList == NULL) {return;}
    

    boost::tie(out_begin, out_end) = out_edges(bufList->v,g);
    
    // Iterate Buffers
    for (out_cur = out_begin; out_cur != out_end; ++out_cur) {
      

      sLog << std::endl;

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
        sLog << std::endl << "Cmd #" << i << ": " << std::endl;
        if(type == ACT_TYPE_UNKNOWN) {sLog << "Unknown Format / not initialised" << std::endl; continue;}
        if(type == ACT_TYPE_NOOP || type == ACT_TYPE_FLOW) sLog << std::dec << qty << " x ";
        else                                               sLog << "1 x ";
        sLog << sType[type] << " @ > " << vTime << " ns, " << sPrio[prio] << " priority";
        if (((type == ACT_TYPE_FLOW) || ((type == ACT_TYPE_WAIT) && !(abs))) && perm) sLog << ", changes are permanent" << std::endl;
        else sLog << ", changes are temporary" << std::endl;

        //type specific
        switch(type) {
          case ACT_TYPE_NOOP  : break;
          case ACT_TYPE_FLOW  : sLog << "Destination: ";
                                try { 
                                  auto* y = atDown.lookupAdr(cpuIdx, atDown.intAdr2adr(cpuIdx, dest));
                                  if(y != NULL) name = hm.lookup(y->hash);
                                  else name = "INVALID"; 
                                } catch (...) {throw; name = "INVALID";}
                                sLog << name.get()  << std::endl; break;
          case ACT_TYPE_FLUSH : sLog << "Priority to Flush: " << flPrio << " Mode: " << flMode << std::endl; break;
          case ACT_TYPE_WAIT  : if (abs) {sLog << "Wait until " << wTime << std::endl;} else {sLog << "Make Block Period " << wTime << std::endl;} break;

        }

      }
    }
    sLog << std::endl;
  }      

void CarpeDM::dumpNode(uint8_t cpuIdx, const std::string& name) {
  
  Graph& g = gUp;
  try {
    auto* n = atDown.lookupHash(hm.lookup(boost::get_property(g, boost::graph_name) + name).get());  
    hexDump(gDown[n->v].name.c_str(), n->b, _MEM_BLOCK_SIZE); 
  } catch (...) {throw;}
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


uint64_t CarpeDM::getThrDeadline(uint8_t cpuIdx, uint8_t thrIdx) {
  return read64b(cpuIdx, thrIdx, myDevs.at(cpuIdx).sdb_component.addr_first + SHARED_OFFS + SHCTL_THR_DAT + thrIdx * _T_TD_SIZE_ + T_TD_DEADLINE);
}

void CarpeDM::setThrStartTime(uint8_t cpuIdx, uint8_t thrIdx, uint64_t t) {
  write64b(cpuIdx, thrIdx, myDevs.at(cpuIdx).sdb_component.addr_first + SHARED_OFFS + SHCTL_THR_STA + thrIdx * _T_TS_SIZE_ + T_TS_STARTTIME, t);
}

uint64_t CarpeDM::getThrStartTime(uint8_t cpuIdx, uint8_t thrIdx) {
  return read64b(cpuIdx, thrIdx, myDevs.at(cpuIdx).sdb_component.addr_first + SHARED_OFFS + SHCTL_THR_STA + thrIdx * _T_TS_SIZE_ + T_TS_STARTTIME);
}

void CarpeDM::setThrPrepTime(uint8_t cpuIdx, uint8_t thrIdx, uint64_t t) {
  write64b(cpuIdx, thrIdx, myDevs.at(cpuIdx).sdb_component.addr_first + SHARED_OFFS + SHCTL_THR_STA + thrIdx * _T_TS_SIZE_ + T_TS_PREPTIME, t);
}

uint64_t CarpeDM::getThrPrepTime(uint8_t cpuIdx, uint8_t thrIdx) {
  return read64b(cpuIdx, thrIdx, myDevs.at(cpuIdx).sdb_component.addr_first + SHARED_OFFS + SHCTL_THR_STA + thrIdx * _T_TS_SIZE_ + T_TS_PREPTIME);
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


//Returns if a hash / nodename is present on DM
  bool CarpeDM::isValid(const uint32_t hash)  {

    if (atDown.lookupHash(hash) != NULL) return true;
    else return false;
  }

  bool CarpeDM::isValid(const std::string& name) {
    if (!(hm.contains(name))) return false;
    return isValid(hm.lookup(name).get());
  }  
