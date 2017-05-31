#include <boost/shared_ptr.hpp>
#include <algorithm>  
#include <stdio.h>
#include <iostream>
#include <string>
#include <inttypes.h>
#include <boost/graph/graphviz.hpp>

//#include "visitor.h"
#include "common.h"
#include "propwrite.h"


#include "memunit.h"
#include <etherbone.h>

#include "ftm_shared_mmap.h"




using namespace etherbone;

const char defOutputFilename[] = "download.dot";

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

int main(int argc, char* argv[]) {



  bool doUpload = false, readBlock = false;

  int opt;
  const char *program = argv[0];
  const char *netaddress, *blockName = NULL, *inputFilename = NULL, *outputFilename = defOutputFilename;
  int32_t tmp, error=0;
  uint32_t cpuIdx = 0;

// start getopt 
   while ((opt = getopt(argc, argv, "b:c:o:w")) != -1) {
      switch (opt) {
         case 'w':
            doUpload = true;
            break;
         case 'o':
            outputFilename  = optarg;
            break;
         case 'b':
            blockName = optarg;
            readBlock = true;
            break;       
         case 'v':
            //verbose = 1;
            break;
         case 't':
            //show_time = 1;
            break;
         case 'c':
            tmp = atol(optarg);
            if (tmp < 0 || tmp > 8) {
              std::cerr << program << ": invalid cpu id -- '" << optarg << "'" << std::endl;
              error = -1;
            } else {cpuIdx = (uint32_t)tmp;}
            break;
         /*    
         case 'h':
            help();
            return 0;
         */ 
         case ':':
         
         case '?':
            error = -2;
            break;
            
         default:
            std::cerr << program << ": bad getopt result" << std::endl; 
            error = -3;
      }
   }


  if (error) return error;

   if (optind >= argc) {
   std::cerr << program << ": expecting one non-optional argument: <etherbone-device>" << std::endl;
   //help();
   return -4;
   }
   
   // process command arguments
  std::cerr << program << ": " << argc << " " << optind << " " << argv[optind] << " " << argv[optind+1] << " " << argv[optind+2] << " " << std::endl;   

   netaddress = argv[optind];
   if (optind+1 < argc) inputFilename   = argv[optind+1];
   

   /*
   if (optind+1 < argc)  command = argv[++optind];
   else                 {command = "status"; cpuId = -1;}
   if (!strcasecmp(command, "loadfw")) overrideFWcheck = 1;  
   
   if ( (!strcasecmp(command, "put")) || (!strcasecmp(command, "loadfw")))
   {
      if (optind+1 < argc) {
         strncpy(filename, argv[optind+1], FILENAME_LEN);

         readonly = 0;
      } else {
         fprintf(stderr, "%s: expecting one non-optional argument: <filename>\n", program);
         return 1;
      }
   } 
   */


  std::vector<struct sdb_device> myDevs;
  //const std::string netaddress = "dev/ttyUSB0"; 
  Socket ebs;
  Device ebd;

  // Construct an empty graph and prepare the dynamic_property_maps.
  Graph g;
  boost::dynamic_properties dp(boost::ignore_other_properties);

  ebs.open(0, EB_DATAX|EB_ADDRX);
  ebd.open(ebs, netaddress, EB_DATAX|EB_ADDRX, 3);

  ebd.sdb_find_by_identity(0x0000000000000651ULL,0x54111351, myDevs);
  if (cpuIdx >= myDevs.size()) return -1;

  std::cout << "Found " << myDevs.size() << " User-RAMs, cpu #" << cpuIdx << " is a valid choice " << std::endl;
  //create memory manager
  std::cout << "Creating Memory Unit #" << cpuIdx << "..." << std::endl;
  MemUnit mmu = MemUnit(cpuIdx, myDevs[cpuIdx].sdb_component.addr_first, INT_BASE_ADR,  SHARED_OFFS + _SHCTL_END_ , SHARED_SIZE - _SHCTL_END_, g);


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

 
  std::ifstream in(inputFilename); 
  if(inputFilename == NULL || !(boost::read_graphviz(in,g,dp,"node_id"))) {
    std::cerr << program << ": Could not open local file <" << inputFilename << ">" << std::endl;
     if(doUpload) {
      ebd.close();
      ebs.close();
      return -5;
    }
  }  
  else { 
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




    
    
   

    //analyse and serialise
    std::cout << "Processing local file <" << inputFilename << "> ..." << std::endl;  

    mmu.prepareUpload(); 

    std::cout << "... Done. " << std::endl;
    
    
    
  }
  if(doUpload) {
    //show results
    std::cout << "Generating Binary Data ... " << std::endl;
    vBuf vUlD = mmu.getUploadData();
    vAdr vUlA = mmu.getUploadAdrs(); 
    
    //for (auto& it : vUlA) { std::cout << "WR @: 0x" << std::hex << it << std::endl;}

    //vHexDump("EB to Transfer", vUlD, vUlD.size()); 
    std::cout << "... Done. " << std::endl << " Uploading ... ";
    //Upload
    ftmRamWrite(ebd, vUlA, vUlD);
   
    std::cout << "...Done. " << std::endl << "To make LM32 start, write Node Adr to 0x" << std::hex << myDevs[cpuIdx].sdb_component.addr_first + SHARED_OFFS + SHCTL_THR_STA + T_TD_NODE_PTR << " then write 1 to 0x" << myDevs[cpuIdx].sdb_component.addr_first + SHARED_OFFS + SHCTL_THR_CTL + T_TC_START << std::endl;

  } else {
    //Download Readback

    vAdr vDlBmpA = mmu.getDownloadBMPAdrs();

    //for (auto& it : vDlBmpA) { std::cout << "RD BMP @: 0x" << std::hex << it << std::endl;}

    vBuf vBmp = ftmRamRead(ebd, vDlBmpA);
    mmu.setDownloadBmp(vBmp);
    

    vAdr vDlA = mmu.getDownloadAdrs();
   
    //for (auto& it : vDlA) { std::cout << "RD @: 0x" << std::hex << it << std::endl;}
    vBuf vDlD = ftmRamRead(ebd, vDlA);
     std::cout << "Got " << std::dec << vDlA.size() << " bytes, " << vDlA.size() / (_MEM_BLOCK_SIZE / 4 )<< " Nodes " << std::endl;
    std::cout << "Download complete." << std::endl << "Parsing...";
    
    /*
    //Verify
    if(vDlD == vUlD) std::cout << "Up and Download are equal" << std::endl;
    else {std::cerr << "Verify Failed" << std::endl; }//vHexDump("Verify Failed", vDlD, vDlD.size()); }
    */
    mmu.parseDownloadData(vDlD);




    std::cout << "... Done. " << std::endl;


    if(readBlock) {
      auto* x = mmu.lookupName(std::string(blockName));
      if (x != NULL) {
        Graph& gb = mmu.getDownGraph();

        auto* q = mmu.lookupAdr(x->adr);
        if(q != NULL) {

          std::cout << "Found a ";
          if(gb[q->v].np->isBlock()) {
            std::cout << "Block " << blockName << " @ 0x" << std::hex << x->adr << std::endl;
            //read out Block info
            hexDump ("Binary:", q->b, _MEM_BLOCK_SIZE);
            auto pb = boost::dynamic_pointer_cast<Block>(gb[q->v].np);
            //# 0x" << std::hex << std::setfill('0') << std::setw(8)
            std::cout << "      IlHiLo" << std::endl;
            std::cout << "WR: 0x" << std::hex << std::setfill('0') << std::setw(6) << pb->getWrIdxs() << std::endl;
            std::cout << "RD: 0x" << std::hex << std::setfill('0') << std::setw(6) << pb->getRdIdxs() << std::endl;

            //Do the crawl
            uint8_t wrOffs = (pb->getWrIdxs() >> 16) & 0xff;

            ptrdiff_t prio  = BLOCK_CMDQ_IL_PTR;
            ptrdiff_t bufIdx   = wrOffs / (_MEM_BLOCK_SIZE / _T_CMD_SIZE  );
            ptrdiff_t elemIdx  = wrOffs % (_MEM_BLOCK_SIZE / _T_CMD_SIZE  );

            uint32_t bufListAdr, bufAdr, wrAdr;
            std::cout << blockName << " -- IL --> ";
            bufListAdr = mmu.intAdr2adr(writeBeBytesToLeNumber<uint32_t>((uint8_t*)&q->b[prio]));
            auto* qbl = mmu.lookupAdr(bufListAdr);
            if(qbl != NULL) {bufAdr = mmu.intAdr2adr(writeBeBytesToLeNumber<uint32_t>((uint8_t*)&qbl->b[(CMDQ_BUF_ARRAY + bufIdx * _PTR_SIZE_) ])); std::cout << g[qbl->v].name << " --+" << bufIdx << "b--> ";} 
            
            auto* qb = mmu.lookupAdr(bufAdr);
            if(qb != NULL) {
              wrAdr = bufAdr + CMDB_CMD_ARRAY  + elemIdx * _T_CMD_SIZE ;
              std::cout << g[qb->v].name << " --+" << elemIdx << "e--> WrAdr 0x" << std::hex << std::setfill('0') << std::setw(6) << mmu.adr2extAdr(wrAdr) << std::endl;
              hexDump ("Qbuf", qb->b, _MEM_BLOCK_SIZE);  

              std::cout << "Write Offset <" << std::hex << std::setfill('0') << std::setw(6) << pb->getWrIdxs() << ">" << " @ 0x" << std::hex << std::setfill('0') << std::setw(8) << mmu.adr2extAdr(x->adr + BLOCK_CMDQ_WR_IDXS) << std::endl;
            }  


          } else { std::cout << "Node " << blockName << " @ 0x" << std::hex << x->adr << std::endl; }
          
        } else { std::cerr << "Fuck all ptrs" << std::endl;}

      } else {

      }

    }



    std::cout << "Writing out ...";

    std::ofstream out(outputFilename); 
   

    boost::write_graphviz(out, mmu.getDownGraph(), make_vertex_writer(boost::get(&myVertex::np, mmu.getDownGraph())), make_edge_writer(boost::get(&myEdge::type, mmu.getDownGraph())), sample_graph_writer{"Demo"}, boost::get(&myVertex::name, mmu.getDownGraph()));

    std::cout << "... Done. " << std::endl;
  }
  ebd.close();
  ebs.close();

  return 0;
}
