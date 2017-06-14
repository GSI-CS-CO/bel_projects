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

const std::string sPrio[] = {"LO", "HI", "IL"};
/*
void sendFlowCmd(Device& dev, uint8_t prio, uint64_t validTime, uint32_t target, uint32_t dest, uint16_t qty) {
  Cycle cyc;

  cyc.open(dev);
  cyc.write(target + T_CMD_TIME + 0,          EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)(validTime >> 32));
  cyc.write(target + T_CMD_TIME + _32b_SIZE_, EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)(validTime & 0xffffffff));
  cyc.write(target + T_CMD_ACT,               EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)((ACT_TYPE_FLOW << ACT_TYPE_POS) | (prio << ACT_PRIO_POS) | (qty << ACT_QTY_POS )) );
  cyc.write(target + T_CMD_FLOW_DEST,         EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)(dest));
  cyc.write(target + T_CMD_RES,               EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)(0));
  cyc.close();

}
*/
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



  bool doUpload = false, readBlock = false, verbose = false;

  int opt;
  const char *program = argv[0];
  const char *netaddress, *targetName = NULL, *inputFilename = NULL, *typeName = NULL, *para = NULL;
  int32_t tmp, error=0;
  uint32_t cpuIdx = 0, thrIdx = 0, cmdPrio = PRIO_LO, cmdQty = 1;
  uint64_t cmdTvalid = 0, cmdTWait = 0, cmdFlush = PRIO_LO, longtmp;

// start getopt 
   while ((opt = getopt(argc, argv, "vc:p:t:q:")) != -1) {
      switch (opt) {

         case 'v':
            verbose = 1;
            break;
         case 't':
            longtmp = atoll(optarg);
            if (longtmp < 0) {
              std::cerr << program << ": invalid valid time -- '" << optarg << "'" << std::endl;
              error = -1;
            } else {cmdTvalid = (uint64_t)tmp;}
            break;   
         case 'p':
            tmp = atol(optarg);
            if (tmp < 0 || tmp > 2) {
              std::cerr << program << ": invalid priority -- '" << optarg << "'" << std::endl;
              error = -1;
            } else {cmdPrio = (uint32_t)tmp;}
            break;
         case 'q':
            tmp = atol(optarg);
            if (tmp < 1) {
              std::cerr << program << ": invalid qty -- '" << optarg << "'" << std::endl;
              error = -1;
            } else {cmdQty = (uint32_t)tmp;}
            break;   
         case 'c':
            tmp = atol(optarg);
            if (tmp < 0 || tmp > 8) {
              std::cerr << program << ": invalid cpu idx -- '" << optarg << "'" << std::endl;
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

   if (optind+2 >= argc) {
   std::cerr << program << ": expecting two non-optional argument: <etherbone-device> <target-block> " << std::endl;
   //help();
   return -4;
   }
   
   // process command arguments
  std::cerr << program << ": " << argc << " " << optind << " " << argv[optind] << " " << argv[optind+1] << " " << argv[optind+2] << " " << std::endl;   

   netaddress = argv[optind];
   if (optind+1 < argc) inputFilename   = argv[optind+1];
   if (optind+2 < argc) targetName      = argv[optind+2];
   if (optind+3 < argc) typeName        = argv[optind+3];
   if (optind+4 < argc) para            = argv[optind+4];

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
  
  //FIXME this is quite dangerous - misspell a property name and it will not be initialised / contain garbage
  boost::dynamic_properties dp(boost::ignore_other_properties);
  //boost::dynamic_properties dp;

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
  dp.property("prio",  boost::get(&myVertex::prio, g));
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

    const unsigned char deadbeef[4] = {0xDE, 0xAD, 0xBE, 0xEF};
    const std::string needle((const char*)deadbeef);



    BOOST_FOREACH( vertex_t v, vertices(mmu.getUpGraph()) ) {
      std::string haystack(mmu.lookupName(mmu.getUpGraph()[v].name)->b, mmu.lookupName(mmu.getUpGraph()[v].name)->b + _MEM_BLOCK_SIZE);
      std::size_t n = haystack.find(needle);
      bool foundUninitialised = (n != std::string::npos);

      if(verbose || foundUninitialised) {
        std::cout << std::endl;
        hexDump(mmu.getUpGraph()[v].name.c_str(), mmu.lookupName(mmu.getUpGraph()[v].name)->b, _MEM_BLOCK_SIZE);
      }

      if(foundUninitialised) {
        std::cout << std::endl << "Error: Node " << mmu.getUpGraph()[v].name << " contains uninitialised elements! Misspelled/forgot a mandatory property in .dot file ?" << std::endl << std::endl;  
        return -7;
      }  
    }

    std::cout << "... Done. " << std::endl;
    
    
    
  }
  
    //Download Readback

    vAdr vDlBmpA = mmu.getDownloadBMPAdrs();

    //for (auto& it : vDlBmpA) { std::cout << "RD BMP @: 0x" << std::hex << it << std::endl;}

    vBuf vBmp = ftmRamRead(ebd, vDlBmpA);
    mmu.setDownloadBmp(vBmp);
    

    vAdr vDlA = mmu.getDownloadAdrs();
   
    vBuf vDlD = ftmRamRead(ebd, vDlA);
     std::cout << "Got " << std::dec << vDlA.size() << " bytes, " << vDlA.size() / (_MEM_BLOCK_SIZE / 4 )<< " Nodes " << std::endl;
    std::cout << "Download complete." << std::endl << "Parsing...";
    
    mmu.parseDownloadData(vDlD);

 

    std::cout << "... Done. " << std::endl;


    
      auto* x = mmu.lookupName(std::string(targetName));
      if (x != NULL) {
        Graph& gb = mmu.getDownGraph();

        auto* q = mmu.lookupAdr(x->adr);
        if(q != NULL) {

          std::cout << "Found a ";
          if(gb[q->v].np->isBlock()) {
            std::cout << "Block " << targetName << " @ 0x" << std::hex << x->adr << std::endl;
            //read out Block info
            hexDump ("Binary:", q->b, _MEM_BLOCK_SIZE);
            auto pb = boost::dynamic_pointer_cast<Block>(gb[q->v].np);
            
            //Do the crawl
            uint8_t eWrIdx  = (pb->getWrIdxs() >> (cmdPrio * 8)) & Q_IDX_MAX_OVF_MSK;
            uint8_t wrIdx   = eWrIdx & Q_IDX_MAX_MSK;

            std::cout << "      IlHiLo" << std::endl;
            std::cout << "WR: 0x" << std::hex << std::setfill('0') << std::setw(6) << pb->getWrIdxs() << " -- " << sPrio[cmdPrio] << " --> w0x" << std::hex << (int)wrIdx << std::endl;
            std::cout << "RD: 0x" << std::hex << std::setfill('0') << std::setw(6) << pb->getRdIdxs() << std::endl;

            

            ptrdiff_t prio  = BLOCK_CMDQ_LO_PTR + cmdPrio * _PTR_SIZE_;
            ptrdiff_t bufIdx   = wrIdx / (_MEM_BLOCK_SIZE / _T_CMD_SIZE_  );
            ptrdiff_t elemIdx  = wrIdx % (_MEM_BLOCK_SIZE / _T_CMD_SIZE_  );

            uint32_t bufListAdr, bufAdr, wrAdr;
            std::cout << targetName << " -- " << sPrio[cmdPrio] << " --> ";
            bufListAdr = mmu.intAdr2adr(writeBeBytesToLeNumber<uint32_t>((uint8_t*)&q->b[prio]));
            auto* qbl = mmu.lookupAdr(bufListAdr);
            if(qbl != NULL) {bufAdr = mmu.intAdr2adr(writeBeBytesToLeNumber<uint32_t>((uint8_t*)&qbl->b[(CMDQ_BUF_ARRAY + bufIdx * _PTR_SIZE_) ])); std::cout << g[qbl->v].name << " --+" << bufIdx << "b--> ";} 
            
            auto* qb = mmu.lookupAdr(bufAdr);
            if(qb != NULL) {
              wrAdr = bufAdr + CMDB_CMD_ARRAY  + elemIdx * _T_CMD_SIZE_ ;
              std::cout << g[qb->v].name << " --+" << elemIdx << "e--> WrAdr 0x" << std::hex << std::setfill('0') << std::setw(6) << mmu.adr2extAdr(wrAdr) << std::endl;
              hexDump ("Qbuf", qb->b, _MEM_BLOCK_SIZE);  

              std::cout << "Write Offset <" << std::hex << std::setfill('0') << std::setw(6) << pb->getWrIdxs() << ">" << " @ 0x" << std::hex << std::setfill('0') << std::setw(8) << mmu.adr2extAdr(x->adr + BLOCK_CMDQ_WR_IDXS) << std::endl;
            }  
/*
            if(typeName != NULL) {
              auto* pDest = mmu.lookupName(std::string(para));
              if (pDest != NULL) {

                sendFlowCmd(dev, cmdPrio, cmdTvalid, mmu.adr2extAdr(wrAdr), dest, cmdQty);
                Cycle cyc;
                cyc.open(dev);
                cyc.write(mmu.adr2extAdr(x->adr + BLOCK_CMDQ_WR_IDXS), EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)(pb->getWrIdxs() & ~(0xff << (cmdPrio * 8)) | ((++eWrIdx & Q_IDX_MAX_OVF_MSK) << (cmdPrio * 8))));
                cyc.close();
              } else {std::cout "Destination is invalid" << std::endl;}
            }    
*/

          } else { std::cout << "Node " << targetName << " @ 0x" << std::hex << x->adr << std::endl; }
          
        } else { std::cerr << "Fuck all ptrs" << std::endl;}

      } else { }

    



    
  ebd.close();
  ebs.close();

  return 0;
}
