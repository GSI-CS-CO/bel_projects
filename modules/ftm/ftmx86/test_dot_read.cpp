#include <boost/shared_ptr.hpp>
#include <algorithm>  
#include <stdio.h>
#include <iostream>
#include <string>
#include <inttypes.h>
#include <boost/graph/graphviz.hpp>

#include "visitor.h"
#include "common.h"
#include "propwrite.h"


#include "memunit.h"
#include <etherbone.h>

using namespace etherbone;

int ftmRamWrite(Device& dev, vAdr va, vBuf& vb)
{
   //eb_status_t status;
   //std::cout << "Sizes: Va * 4 " << (va.size()*4) << " Vb " << vb.size() << std::endl; 
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

vBuf ftmRamRead(Device& dev, vAdr va)
{
   //eb_status_t status;
   Cycle cyc;
   uint32_t veb[va.size()];
   vBuf ret   = vBuf(va.size() * 4);
      
   //std::cout << "Got Adr Vec with " << va.size() << " Adrs" << std::endl;

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

int main() {

  const uint32_t cpuIdx = 3;

  std::vector<struct sdb_device> myDevs;
  //const std::string netaddress = "dev/ttyUSB0"; 
  Socket ebs;
  Device ebd;

  // Construct an empty graph and prepare the dynamic_property_maps.
  Graph g;
  boost::dynamic_properties dp(boost::ignore_other_properties);


  dp.property("type",  boost::get(&myEdge::type, g));

  dp.property("node_id", boost::get(&myVertex::name, g));
  dp.property("type",  boost::get(&myVertex::type, g));

  // not sure about this ... this should be several meaningful properties which are then grouped into "flags" field
  dp.property("flags", boost::get(&myVertex::flags, g));

  //Block
  dp.property("tPeriod", boost::get(&myVertex::tPeriod, g));

  // leave out prefilled block cmdq indices and cmdq buffers and for now
  //Events
  dp.property("tOffs",  boost::get(&myVertex::tOffs, g));
  //Timing Message
  dp.property("id",   boost::get(&myVertex::id, g));
  dp.property("par",  boost::get(&myVertex::par, g));
  dp.property("tef",  boost::get(&myVertex::tef, g));
  dp.property("res",  boost::get(&myVertex::res, g));
  //Commands
  dp.property("tValid",  boost::get(&myVertex::tValid, g));

  // Leave out Flush for now
 
  //Flow, Noop
  dp.property("qty",  boost::get(&myVertex::qty, g));

  //Wait
  dp.property("tWait",  boost::get(&myVertex::tWait, g));

 
  std::ifstream in("./try.dot"); 
  bool status = boost::read_graphviz(in,g,dp,"node_id");
  boost::graph_traits<Graph>::vertex_iterator vi, vi_end;
  boost::tie(vi, vi_end) = vertices(g);
  std::cout << "Size " << vi_end - vi << std::endl;

 
  //format all graph labels lowercase
    BOOST_FOREACH( edge_t e, edges(g) ) {
    std::transform(g[e].type.begin(), g[e].type.end(), g[e].type.begin(), ::tolower);
  }

  BOOST_FOREACH( vertex_t v, vertices(g) ) {
    std::transform(g[v].type.begin(), g[v].type.end(), g[v].type.begin(), ::tolower);
  }




  ebs.open(0, EB_DATAX|EB_ADDRX);
  ebd.open(ebs, "dev/ttyUSB0", EB_DATAX|EB_ADDRX, 3);

  ebd.sdb_find_by_identity(0x0000000000000651ULL,0x54111351, myDevs);
  if (cpuIdx >= myDevs.size()) return -1;

  std::cout << "Found " << myDevs.size() << " User-RAMs, cpu #" << cpuIdx << " is a valid choice " << std::endl;
  //create memory manager
  std::cout << "Creating Memory Unit #" << cpuIdx << "..." << std::endl;
  MemUnit mmu = MemUnit(cpuIdx, myDevs[cpuIdx].sdb_component.addr_first, 0x1000000, 0x500, 8192, g);
  uint32_t ban;
  mmu.acquireChunk(ban);
  mmu.acquireChunk(ban);
  mmu.acquireChunk(ban);

  //analyse and serialise
  
  mmu.prepareUpload(); 
  /*
  //show structure
  for (auto& it : mmu.getAllChunks()) {
    std::cout << "E@: 0x" << std::hex << mmu.adr2extAdr(it->adr) << " #: 0x" << it->hash << " -> Name: " << mmu.hash2name(it->hash) << std::endl;
    hexDump("", it->b, _MEM_BLOCK_SIZE);
  }
  */

  //show results
  
  vBuf vUlD = mmu.getUploadData();
  vAdr vUlA   = mmu.getUploadAdrs(); 
  
  //for (auto& it : vUlA) { std::cout << "WR @: 0x" << std::hex << it << std::endl;}

  //vHexDump("EB to Transfer", vUlD, vUlD.size()); 

  //Upload
  ftmRamWrite(ebd, vUlA, vUlD);
 

  //Download Readback
  vAdr vDlBmpA = mmu.getDownloadBMPAdrs();

  //for (auto& it : vDlBmpA) { std::cout << "RD BMP @: 0x" << std::hex << it << std::endl;}

  vBuf vBmp = ftmRamRead(ebd, vDlBmpA);
  mmu.setDownloadBmp(vBmp);
  

  vAdr vDlA = mmu.getDownloadAdrs();
  std::cout << "Got " << std::dec << vDlA.size() << " bytes, " << vDlA.size() / (_MEM_BLOCK_SIZE / 4 )<< " Nodes " << std::endl;
  //for (auto& it : vDlA) { std::cout << "RD @: 0x" << std::hex << it << std::endl;}
  vBuf vDlD = ftmRamRead(ebd, vDlA);

  
  ebd.close();
  ebs.close();
  /*
  //Verify
  if(vDlD == vUlD) std::cout << "Up and Download are equal" << std::endl;
  else {std::cerr << "Verify Failed" << std::endl; }//vHexDump("Verify Failed", vDlD, vDlD.size()); }
  */
  mmu.parseDownloadData(vDlD);

  std::ofstream out("./download.dot"); 
  boost::default_writer dw;

  

  boost::write_graphviz(out, mmu.getDownGraph(), make_vertex_writer(boost::get(&myVertex::np, mmu.getDownGraph())), dw, sample_graph_writer{"Demo"}, boost::get(&myVertex::name, mmu.getDownGraph()));

  return 0;
}
