#include <boost/shared_ptr.hpp>
#include <algorithm>  
#include <stdio.h>
#include <iostream>
#include <string>
#include <inttypes.h>
#include <boost/graph/graphviz.hpp>

#include "common.h"
#include "propwrite.h"
#include "memunit.h"
#include "ftm_shared_mmap.h"
#include "graph.h"
#include "carpeDM.h"
#include "minicommand.h"




const unsigned char CarpeDM::deadbeef[4] = {0xDE, 0xAD, 0xBE, 0xEF};
const std::string CarpeDM::needle(CarpeDM::deadbeef, CarpeDM::deadbeef + 4);

int CarpeDM::ftmRamWrite(Device& dev, vAdr va, vBuf& vb)
{
   //eb_status_t status;
   //sLog << "Sizes: Va * 4 " << (va.size()*4) << " Vb " << vb.size() << std::endl; 
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

vBuf CarpeDM::ftmRamRead(Device& dev, vAdr va)
{
   //eb_status_t status;
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

bool CarpeDM::connect(const std::string& en) {
    ebdevname = std::string(en); //copy to avoid mem trouble later
    bool  ret = false;

    vM.clear();

    try { 
      ebs.open(0, EB_DATAX|EB_ADDRX);
      ebd.open(ebs, ebdevname.c_str(), EB_DATAX|EB_ADDRX, 3);
      ebd.sdb_find_by_identity(SDB_VENDOR_GSI,SDB_DEVICE_LM32_RAM, myDevs);
      if (myDevs.size() >= 1) { 
        cpuQty = myDevs.size();
        for(int i = 0; i< cpuQty; i++) vM.push_back(MemUnit(i, myDevs[i].sdb_component.addr_first, INT_BASE_ADR,  SHARED_OFFS + _SHCTL_END_ , SHARED_SIZE - _SHCTL_END_, hm));
        ret = true;
      }
    } catch(...) {
      //TODO report why we could not connect / find CPUs
    }
    return ret;

  }

  bool CarpeDM::disconnect() {
    bool ret = false;
    try { 
      ebd.close();
      ebs.close();
      cpuQty = -1;
      ret = true;
    } catch(...) {
      //TODO report why we could not disconnect
    }
    return ret;
  }

  void CarpeDM::addDotToDict(const std::string& fn) {
    Graph g;
    std::ifstream in(fn);
    boost::dynamic_properties dp = createParser(g);

    if(in.good()) {
  
      try { boost::read_graphviz(in,g,dp,"node_id");}
      catch(...) {
        throw;
        //TODO report why parsing the dot / creating the graph failed
      }
      
      
    }  
    else {std::cout << "Could not open .dot file <" << fn << "> for reading!" << std::endl; return;}  

    //add to dictionary
    try {
      BOOST_FOREACH( vertex_t v, vertices(g) ) { hm.add(g[v].name);}
    }  catch (...) {
      //TODO report hash collision and show which names are responsible
      throw;
    }
    in.close();
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
      try { boost::read_graphviz(in, g, dp, "node_id"); }
      catch(...) {
        throw;

      }
      in.close();
    }  
    else {sErr << " Could not open .dot file <" << fn << "> for reading!" << std::endl; return g;}  
    //format all graph labels lowercase
    boost::graph_traits<Graph>::vertex_iterator vi, vi_end;
    boost::tie(vi, vi_end) = vertices(g);   
    BOOST_FOREACH( edge_t e, edges(g) ) { std::transform(g[e].type.begin(), g[e].type.end(), g[e].type.begin(), ::tolower); }
    BOOST_FOREACH( vertex_t v, vertices(g) ) { std::transform(g[v].type.begin(), g[v].type.end(), g[v].type.begin(), ::tolower); } 
    
    //TODO create subgraphs as necessary

    //TODO automatically add necessary meta nodes

    return g;

  }


     //TODO NC analysis

  
  bool CarpeDM::prepareUploadToCpu(Graph& g, uint8_t cpuIdx) {
    MemUnit& m = vM.at(cpuIdx);
    
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
        sErr << std::endl << "Node " << m.getUpGraph()[v].name << " contains uninitialised elements! Misspelled/forgot a mandatory property in .dot file ?" << std::endl << std::endl;  
        return false;
      }  
    }



    return true;
  }

  int CarpeDM::upload(uint8_t cpuIdx) {
    MemUnit& m = vM.at(cpuIdx);
    vBuf vUlD = m.getUploadData();
    vAdr vUlA = m.getUploadAdrs(); 
    //Upload
    ftmRamWrite(ebd, vUlA, vUlD);

    return vUlD.size();
  }


  //TODO assign to CPUs/threads


   int CarpeDM::downloadAndParse(uint8_t cpuIdx) {
    
    MemUnit& m = vM.at(cpuIdx); 
    
    vAdr vDlBmpA;
    vBuf vDlD;

    //verify firmware version first
    //checkFwVersion(cpuIdx);

    vDlBmpA = m.getDownloadBMPAdrs();
    m.setDownloadBmp(ftmRamRead(ebd, vDlBmpA));
    vDlD = ftmRamRead(ebd, m.getDownloadAdrs());
    m.parseDownloadData(vDlD);

    return vDlD.size();
  }
    
 


 //write out dotfile from download graph of a memunit
 void CarpeDM::writeDownDot(const std::string& fn, MemUnit& m) {
    std::ofstream out(fn); 
    if(out.good()) {
      try { boost::write_graphviz(out, m.getDownGraph(), make_vertex_writer(boost::get(&myVertex::np, m.getDownGraph())), 
                          make_edge_writer(boost::get(&myEdge::type, m.getDownGraph())), sample_graph_writer{"Demo"},
                          boost::get(&myVertex::name, m.getDownGraph()));
      }
      catch(...) {
        throw;

      }
      out.close();
    }  
    else {sErr << " Could not write to .dot file <" << fn << "> !" << std::endl; return;} 

    
  }

  vAdr CarpeDM::getCmdWrAdrs(uint8_t cpuIdx, const std::string& targetName, uint8_t prio) {
    return vM.at(cpuIdx).getCmdWrAdrs(hm.lookup(targetName).get(), prio);
  }

  vBuf CarpeDM::getCmdData(uint8_t cpuIdx, const std::string& targetName, uint8_t prio, mc_ptr mc) {
    uint8_t b[_T_CMD_SIZE_ + _32b_SIZE_];
    vBuf ret(_T_CMD_SIZE_ + _32b_SIZE_);

    mc->serialise(b);
    writeLeNumberToBeBytes(b + (ptrdiff_t)_T_CMD_SIZE_, vM.at(cpuIdx).getCmdInc(hm.lookup(targetName).get(), prio));
    
    ret.insert( ret.end(), b, b + _MEM_BLOCK_SIZE + _32b_SIZE_);

    return ret;
  }