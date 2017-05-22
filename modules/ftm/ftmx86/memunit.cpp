#include <boost/graph/graphviz.hpp>
#include "memunit.h"
#include "common.h"
#include "node.h"
#include "block.h"
#include "meta.h"
#include "event.h"
#include "visitor.h"



  void MemUnit::initMemPool() { 
    memPool.clear();
    std::cout << "extBaseAdr = 0x" << std::hex << extBaseAdr << " intBaseAdr = 0x" << intBaseAdr << ", Poolsize = " << std::dec << poolSize 
    << ", bmpLen = " << bmpLen << ", startOffs = 0x" << std::hex << startOffs << ", endOffs = 0x" << std::hex << endOffs << ", vBufSize = " 
    << std::dec << uploadBmp.size() << std::endl;
    for(uint32_t adr = startOffs; adr < endOffs; adr += _MEM_BLOCK_SIZE) { 
      //Never issue <baseAddress - (baseAddress + bmpLen -1) >, as this is where Mgmt bitmap vector resides     
      //std::cout << std::hex << adr << std::endl; 
      memPool.insert(adr); 
    }
  }

  bool MemUnit::acquireChunk(uint32_t &adr) {
    bool ret = true;
    if ( memPool.empty() ) {
      ret = false;
    } else {
      adr = *(memPool.begin());
      memPool.erase(adr);
    }
    return ret;
  }


  bool MemUnit::freeChunk(uint32_t &adr) {
    bool ret = true;
    if ((adr % _MEM_BLOCK_SIZE) || (memPool.count(adr) > 0))  {ret = false;} //unaligned or attempted double entry, throw exception
    else memPool.insert(adr);
    return ret;
  }        

  void MemUnit::createUploadBmp() {
    for (auto& it : uploadBmp) { 
      it = 0;
    }    

    //Go through allocmap and update Bmp
    for (auto& it : allocMap) {
      if( (it.second.adr >= startOffs) && (it.second.adr < endOffs)) {
        int bitIdx = (it.second.adr - startOffs) / _MEM_BLOCK_SIZE;
        uint8_t tmp = 1 << (7 - (bitIdx % 8));
        //printf("Bidx = %u, bufIdx = %u, val = %x\n", bitIdx, bitIdx / 8 , tmp);
        
        uploadBmp[bitIdx / 8] |= tmp;
      } else {//something's awfully wrong, address out of scope!
        std::cout << "Address 0x" << std::hex << it.second.adr << " is not within 0x" << std::hex << startOffs << "-" << std::hex << endOffs << std::endl;
      }
    }
    
  }

 

  vAdr MemUnit::getUploadAdrs() {
    vAdr ret;

    for (auto& it : allocMap) {
      for (uint32_t adr = adr2extAdr(it.second.adr); adr < adr2extAdr(it.second.adr) + _MEM_BLOCK_SIZE; adr += _32b_SIZE_ ) ret.push_back(adr);
    }    
    return ret;
  }

  vBuf MemUnit::getUploadData() {
    vBuf ret;
     std::cout << std::dec << uploadBmp.size() << " " <<  allocMap.size() << " " << _MEM_BLOCK_SIZE << std::endl;
    ret.reserve( uploadBmp.size() + allocMap.size() * _MEM_BLOCK_SIZE); // preallocate memory for BMP and all Nodes

    createUploadBmp();
    vHexDump ("ULBMP", uploadBmp, uploadBmp.size());

    ret.insert( ret.end(), uploadBmp.begin(), uploadBmp.end() );
    for (auto& it : allocMap) { 
      ret.insert( ret.end(), it.second.b, it.second.b + _MEM_BLOCK_SIZE );
    } 
    //ret.push_back( 0x0); 
    return ret;
  }

  vAdr MemUnit::getDownloadAdrs() {
    //easy 1st version: read everything in shared area
    vAdr ret = vAdr((poolSize + _32b_SIZE_ - 1)/ _32b_SIZE_ );
   
    for (uint32_t adr = startOffs + extBaseAdr; adr < endOffs + extBaseAdr; adr += _32b_SIZE_ ) {
      ret.push_back(adr);
    }

    return ret;
  }  
    



  void MemUnit::parseDownloadData(vBuf downloadData) {
    //extract and parse downloadBmp
    parserMap.clear();
    std::copy(downloadData.begin(), downloadData.begin() + downloadBmp.size(), downloadBmp.begin());

    //create parserMap and Vertices
    std::cout << "Got " << downloadData.size() << " bytes of data, " << downloadData.size() / _MEM_BLOCK_SIZE << " blocks." << std::endl;
    vHexDump ("DLBMP", downloadBmp, downloadBmp.size());

    for(unsigned int bitIdx = 0; bitIdx < bmpLen; bitIdx++) {
      //std::cout << std::dec << "bIdx " << bitIdx << " BIdx " << bitIdx / 8 << " Pos " << (7 - bitIdx % 8) << " is 0x" << std::hex << downloadBmp[bitIdx / 8] << " c " << (downloadBmp[bitIdx / 8] & (1 << (7 - bitIdx % 8))) << std::endl;
      if (downloadBmp[bitIdx / 8] & (1 << (7 - bitIdx % 8))) {
        uint32_t localAdr = (startOffs - sharedOffs) + bitIdx * _MEM_BLOCK_SIZE;
        uint32_t adr      = localAdr + sharedOffs;
        std::cout << "IMPORTANT: This is the key: 0x" << std::hex << adr << std::endl; 
        uint32_t hash     = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&downloadData[localAdr + NODE_HASH]);

        std::cout << hash2name(hash) << " -- L@: 0x" << std::hex << localAdr << " @: 0x" << adr << " #0x" << hash << std::endl;
        uint32_t flags    = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&downloadData[localAdr + NODE_FLAGS]);
        uint32_t type     = (flags >> NFLG_TYPE_POS) & NFLG_TYPE_MSK;
        vertex_t v        = boost::add_vertex((myVertex) {hash2name(hash), hash, NULL, "", flags}, gDown);
        parserMap[adr]    = (parserMeta){v, hash};
        auto src = downloadData.begin()+localAdr;
        std::copy(src, src + _MEM_BLOCK_SIZE, (uint8_t*)&parserMap.at(adr).b[0]);

      
        switch(type) {
          case NODE_TYPE_TMSG    : gDown[v].np =(node_ptr) new  TimingMsg(gDown[v].name, gDown[v].hash, parserMap.at(adr).b, gDown[v].flags); gDown[v].np->deserialise(); break;
          case NODE_TYPE_CNOOP   : gDown[v].np =(node_ptr) new       Noop(gDown[v].name, gDown[v].hash, parserMap.at(adr).b, gDown[v].flags); gDown[v].np->deserialise(); break;
          case NODE_TYPE_CFLOW   : gDown[v].np =(node_ptr) new       Flow(gDown[v].name, gDown[v].hash, parserMap.at(adr).b, gDown[v].flags); gDown[v].np->deserialise(); break;
          case NODE_TYPE_CFLUSH  : gDown[v].np =(node_ptr) new      Flush(gDown[v].name, gDown[v].hash, parserMap.at(adr).b, gDown[v].flags); gDown[v].np->deserialise(); break;
          case NODE_TYPE_CWAIT   : gDown[v].np =(node_ptr) new       Wait(gDown[v].name, gDown[v].hash, parserMap.at(adr).b, gDown[v].flags); gDown[v].np->deserialise(); break;
          case NODE_TYPE_BLOCK   : gDown[v].np =(node_ptr) new      Block(gDown[v].name, gDown[v].hash, parserMap.at(adr).b, gDown[v].flags); gDown[v].np->deserialise(); break;
          case NODE_TYPE_QUEUE   : gDown[v].np =(node_ptr) new   CmdQMeta(gDown[v].name, gDown[v].hash, parserMap.at(adr).b, gDown[v].flags); gDown[v].np->deserialise(); break;
          case NODE_TYPE_ALTDST  : gDown[v].np =(node_ptr) new   DestList(gDown[v].name, gDown[v].hash, parserMap.at(adr).b, gDown[v].flags); gDown[v].np->deserialise(); break;
          case NODE_TYPE_QBUF    : gDown[v].np =(node_ptr) new CmdQBuffer(gDown[v].name, gDown[v].hash, parserMap.at(adr).b, gDown[v].flags); break;
          case NODE_TYPE_UNKNOWN : std::cerr << "not yet implemented " << gDown[v].type << std::endl; break;
          default                : std::cerr << "Node type" << gDown[v].type << " not supported! " << std::endl;
        }
        
     
        
      }
    }

    boost::graph_traits<Graph>::vertex_iterator vi, vi_end;
    boost::tie(vi, vi_end) = vertices(gDown);
    std::cout << std::dec << "Size gDown: " << vi_end - vi << std::endl;

    
    // create edges

    //first, iterate all non meta-types to establish block -> dstList parenthood
    for(auto& it : parserMap) {
      // handled by visitor
      if (gDown[it.second.v].np == NULL) { std::cerr << "Node " << gDown[it.second.v].name << " is not initialised ! " << std::endl; 
      } else {
        if  (!(gDown[it.second.v].np->isMeta())) gDown[it.second.v].np->accept(VisitorNodeDownloadCrawler(it.second.v, *this));
      }  
    }
    //second, iterate all meta-types
    for(auto& it : parserMap) {
      // handled by visitor
      if (gDown[it.second.v].np == NULL) { std::cerr << "Node " << gDown[it.second.v].name << " is not initialised ! " << std::endl; 
      } else {
        if  (gDown[it.second.v].np->isMeta()) gDown[it.second.v].np->accept(VisitorNodeDownloadCrawler(it.second.v, *this));
      }  
    }
  }  

   



  vChunk MemUnit::getAllChunks() const {
    vChunk ret;
    for (auto& it : allocMap) {
      ret.push_back((chunkMeta*)(&it.second));
    }    

    return ret;
  }



  /*
void MemUnit::prepareUpload() {

      //check allocation
      //go through graph
      //call allocate    

    //update BMP

for (itBuf it = uploadBmp.begin(); it < uploadBmp.end(); it++) {
      for (int i=0; i <8; i++) {
        if ((*it) & (1<<i)) {
          adr = extBaseAdr + int(uploadBmp.end() - it)*8 + i * _MEM_BLOCK_SIZE;
          ret.push_back(adr);
          std::cout << "0x" << std::hex << adr << std::endl;
        }
      }
    }   

    //serialise
      //go through graph
      //call serialise   
  }


  void MemUnit::upload() {
      
eb_status_t ftmRamWrite()
{
   eb_status_t status;
   eb_cycle_t cycle;
   uint32_t i,j, packets, partLen, start, data;
   uint32_t* writeout = (uint32_t*)buf;   
   
   boost::container::vector<uint32_t> vpChunkMeta;chunkMeta*>

   //wrap frame buffer in EB packet
   packets = ((getUsedSpace() + PACKET_SIZE-1) / PACKET_SIZE);
   start = 0;
   
   for(j=0; j < packets; j++)
   {
      if(j == parts-1 && (len % PACKET_SIZE != 0)) partLen = len % PACKET_SIZE;
      else partLen = PACKET_SIZE;
      
      if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return die(status, "failed to create cycle"); 
      
      for(i= start>>2; i< (start + partLen) >>2;i++)  
      {
         if (bufEndian == LITTLE_ENDIAN)  data = SWAP_4(writeout[i]);
         else                             data = writeout[i];
         
         eb_cycle_write(cycle, (eb_address_t)(address+(i<<2)), EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)data); 
      }
      if ((status = eb_cycle_close(cycle)) != EB_OK) return die(status, "failed to close write cycle");
      start = start + partLen;
   }
   
   return 0;
}
    //split all allocmap elements marked for upload (transfer = true) into network packets ( div 38)

      //play each entry's buffer as eb operations
      //send cycle

    //play BMP as eb operations
    //send cycle

    
    
  }
*/


  //Allocation functions
  bool MemUnit::allocate(const std::string& name) {
    uint32_t chunkAdr, hash;
    bool ret = insertHash(name, hash);
    if ( (allocMap.count(name) == 0) && acquireChunk(chunkAdr) ) { 
      allocMap[name] = (chunkMeta) {chunkAdr, hash};  
    } else {ret = false;}
    return ret;
  }

  bool MemUnit::insert(const std::string& name, uint32_t adr) {return true;}

  bool MemUnit::deallocate(const std::string& name) {
    bool ret = true;
    if ( (allocMap.count(name) > 0) && freeChunk(allocMap.at(name).adr) ) { allocMap.erase(name); 
    } else {ret = false;}
    return ret;
  }

  chunkMeta* MemUnit::lookupName(const std::string& name) const  {
    if (allocMap.count(name) > 0) { return (chunkMeta*)&(allocMap.at(name));} 
    else {return NULL;}
  }

  parserMeta* MemUnit::lookupAdr(uint32_t adr) const {
    if (parserMap.count(adr) > 0) { return (parserMeta*)&(parserMap.at(adr));} 
    else {return NULL;}  
  }

  //Hash functions

  bool MemUnit::insertHash(const std::string& name, uint32_t &hash) {
    hash = FnvHash::fnvHash(name.c_str());

    if (hashMap.left.count(hash) > 0) return false;
    else hashMap.insert( hashValue(hash, name) );
    return true;
  }

  bool MemUnit::removeHash(const uint32_t hash) {
    if (hashMap.left.count(hash) > 0) {hashMap.left.erase(hash); return true;}
    return false;
  }
  
  void MemUnit::prepareUpload() {
    std::string cmp; 

    //allocate and init all nodes
    BOOST_FOREACH( vertex_t v, vertices(gUp) ) {
      allocate(gUp[v].name);

      auto* x = lookupName(gUp[v].name);
      if(x == NULL) {std::cerr << "ERROR: Tried to lookup unallocated node " << gUp[v].name <<  std::endl; return;}

      //init binary node data

      //TODO this should be a factory, yet the variadic part is complex ... any ideas?
      cmp = gUp[v].type;
      if      (cmp == "tmsg")     {gUp[v].np = (node_ptr) new  TimingMsg(gUp[v].name, x->hash, x->b, gUp[v].flags,  gUp[v].tOffs, gUp[v].id, gUp[v].par, gUp[v].tef, gUp[v].res); }
      else if (cmp == "noop")     {gUp[v].np = (node_ptr) new       Noop(gUp[v].name, x->hash, x->b, gUp[v].flags,  gUp[v].tOffs, gUp[v].tValid, gUp[v].qty); }
      else if (cmp == "flow")     {gUp[v].np = (node_ptr) new       Flow(gUp[v].name, x->hash, x->b, gUp[v].flags,  gUp[v].tOffs, gUp[v].tValid, gUp[v].qty); }
      else if (cmp == "flush")    {gUp[v].np = (node_ptr) new      Flush(gUp[v].name, x->hash, x->b, gUp[v].flags, gUp[v].tOffs, gUp[v].tValid, 
                                                                         gUp[v].qIl,                gUp[v].qHi,                gUp[v].qLo, 
                                                                         gUp[v].frmIl, gUp[v].toIl, gUp[v].frmHi, gUp[v].toHi, gUp[v].frmLo, gUp[v].toLo ); }
      else if (cmp == "wait")     {gUp[v].np = (node_ptr) new       Wait(gUp[v].name, x->hash, x->b, gUp[v].flags,  gUp[v].tOffs, gUp[v].tValid, gUp[v].tWait); }
      else if (cmp == "block")    {gUp[v].np = (node_ptr) new      Block(gUp[v].name, x->hash, x->b, gUp[v].flags, gUp[v].tPeriod ); }
      else if (cmp == "qinfo")    {gUp[v].np = (node_ptr) new   CmdQMeta(gUp[v].name, x->hash, x->b, gUp[v].flags);}
      else if (cmp == "listdst")  {gUp[v].np = (node_ptr) new   DestList(gUp[v].name, x->hash, x->b, gUp[v].flags);}
      else if (cmp == "qbuf")     {gUp[v].np = (node_ptr) new CmdQBuffer(gUp[v].name, x->hash, x->b, gUp[v].flags);}
      else if (cmp == "meta")     {std::cerr << "not yet implemented " << gUp[v].type << std::endl;}
      else                        {std::cerr << "Node type" << cmp << " not supported! " << std::endl;} 
    }

    //serialise all nodes
    BOOST_FOREACH( vertex_t v, vertices(gUp) ) {
        if (allocMap.count(gUp[v].name) == 0){std::cerr << " Node " << gUp[v].name << " was not allocated " << gUp[v].type << std::endl; return;} 
        if (gUp[v].np == NULL ){std::cerr << " Node " << gUp[v].name << " was not initialised! " << gUp[v].type << std::endl; return;}
        // try to serialise
        gUp[v].np->accept(VisitorNodeUploadCrawler(v, *this));
    }    
  }



