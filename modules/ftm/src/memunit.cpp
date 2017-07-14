#include <boost/graph/graphviz.hpp>
#include <boost/graph/copy.hpp>
#include "memunit.h"
#include "common.h"
#include "node.h"
#include "block.h"
#include "meta.h"
#include "event.h"
#include "visitoruploadcrawler.h"
#include "visitordownloadcrawler.h"



  void MemUnit::initMemPool() { 
    memPool.clear();
    /*
    std::cout << "extBaseAdr = 0x" << std::hex << extBaseAdr << " intBaseAdr = 0x" << intBaseAdr << ", nodeQty = " << std::dec << nodeQty 
    << ", bmpBits = " << bmpBits << ", startOffs = 0x" << std::hex << startOffs << ", endOffs = 0x" << std::hex << endOffs << ", vBufSize = " 
    << std::dec << uploadBmp.size();
    */
    for(uint32_t adr = startOffs; adr < endOffs; adr += _MEM_BLOCK_SIZE) { 
      //Never issue <baseAddress - (baseAddress + bmpBits -1) >, as this is where Mgmt bitmap vector resides     
      //std::cout << std::hex << adr << std::endl; 
      memPool.insert(adr); 
    }

    //std::cout << " Poolsize " << std::dec << memPool.size() << std::endl;
  }

  void MemUnit::initMemPoolFromDownloadBMP() {
    uint32_t nodeAdr;
    initMemPool(); //init normally, then remove everything already used in download
    for(unsigned int bitIdx = 0; bitIdx < bmpBits; bitIdx++) {
      if (downloadBmp[bitIdx / 8] & (1 << (7 - bitIdx % 8))) {
        nodeAdr = startOffs + bitIdx * _MEM_BLOCK_SIZE;
        //std::cout << "Removing 0x" << std::hex << nodeAdr << " from pool" << std::endl;
        removeChunk(nodeAdr);
      }
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
    //bool ret = true;
    uint32_t a = adr - startOffs;



    if ((a >= endOffs - startOffs) || (a % _MEM_BLOCK_SIZE)) {return false;}
    if (memPool.count(adr) > 0)  {return false;} //unaligned or attempted double entry, throw exception
    memPool.insert(adr);
    return true;
  }        

  void MemUnit::createUploadBmp() {
    //awkward initialistion because bmp in bytes can be  > bmpBits / 8
    for (auto& it : uploadBmp) { 
      it = 0;
    }
    for(uint32_t i=0; i< bmpBits/8; i++) uploadBmp[i] = 0xff;
    for(uint32_t i=0; i< (bmpBits % 8); i++) {
      //std::cout << "adding bit " << bmpBits/8 + i << " address 0x" << std::hex << (bmpBits/8 + i) * _MEM_BLOCK_SIZE + startOffs << std::endl; 
      uploadBmp[bmpBits/8] |= (1 << (7 - (i % 8)));  
    }
    /*  
    std::cout << std::endl << "BMP B4 (size " << uploadBmp.size() << ", bmpSize " << bmpSize << ", bmpBits " << bmpBits << " : ";
    for(unsigned int bitIdx = 0; bitIdx < bmpSize *8; bitIdx++) {
      
      if (uploadBmp[bitIdx / 8] & (1 << (7 - bitIdx % 8))) {std::cout << "1";}
      else {std::cout << "0";}
    }
    std::cout << std::endl;  
    */

    //Go through pool and update Bmp
    for (auto& adr : memPool ) {
      if( (adr >= startOffs) && (adr <= endOffs)) {
        int bitIdx = (adr - startOffs) / _MEM_BLOCK_SIZE;
        uint8_t tmp = ~(1 << (7 - (bitIdx % 8)));
        //printf("Bidx = %u, bufIdx = %u, val = %x, adr 0x%08x \n", bitIdx, bitIdx / 8 , tmp, adr);
        
        uploadBmp[bitIdx / 8] &= tmp;
      } else {//something's awfully wrong, address out of scope!
        throw std::runtime_error( std::string("Address ") + std::to_string(adr) + std::string(" is out of range")); return;
      }
    }
    /*
    std::cout << std::endl << "BMP Up (size " << uploadBmp.size() << ", bmpSize " << bmpSize << ", bmpBits " << bmpBits << " : ";
    for(unsigned int bitIdx = 0; bitIdx < bmpSize *8; bitIdx++) {
      
      if (uploadBmp[bitIdx / 8] & (1 << (7 - bitIdx % 8))) {std::cout << "1";}
      else {std::cout << "0";}
    }
    std::cout << std::endl;
    */
    //vHexDump ("ULBMP", uploadBmp);
    
  }

 

  vAdr MemUnit::getUploadAdrs() const {
    vAdr ret;
    uint32_t adr;
    //generate addresses for BMP (continuous)
    for (adr = adr2extAdr(sharedOffs); adr < adr2extAdr(startOffs); adr += _32b_SIZE_) ret.push_back(adr);
    //generate addresses for nodes (random access)
    for (auto& it : atUp.getTable().get<Adr>()) {
      for (adr = adr2extAdr(it.adr); adr < adr2extAdr(it.adr + _MEM_BLOCK_SIZE); adr += _32b_SIZE_ ) ret.push_back(adr);
    }    
    return ret;
  }

  vBuf MemUnit::getUploadData()  {
    vBuf ret;
    
    ret.reserve( uploadBmp.size() + atUp.getSize() * _MEM_BLOCK_SIZE); // preallocate memory for BMP and all Nodes

    createUploadBmp();
    //vHexDump ("ULBMP", uploadBmp, uploadBmp.size());

    ret.insert( ret.end(), uploadBmp.begin(), uploadBmp.end() );
    //FIXME this is not nice ...see alloctable.h
    for (auto& it : atUp.getTable().get<Adr>()) { 
      ret.insert( ret.end(), it.b, it.b + _MEM_BLOCK_SIZE );
    } 
    return ret;
  }


  

  const vAdr MemUnit::getDownloadBMPAdrs() const {
    //easy 1st version: read everything in shared area
    vAdr ret;

    for (uint32_t adr = adr2extAdr(sharedOffs); adr < adr2extAdr(startOffs); adr += _32b_SIZE_) ret.push_back(adr);

    return ret;
  }

    

  const vAdr MemUnit::getDownloadAdrs() const {
    vAdr ret;

    for(unsigned int bitIdx = 0; bitIdx < bmpBits; bitIdx++) {
      if (downloadBmp[bitIdx / 8] & (1 << (7 - bitIdx % 8))) {
        uint32_t nodeAdr = startOffs + bitIdx * _MEM_BLOCK_SIZE;
        //std::cout << "BitIdx " << std::dec << bitIdx << " -> 0x" << std::hex << nodeAdr << std::endl;
        for (uint32_t adr = adr2extAdr(nodeAdr); adr < adr2extAdr(nodeAdr + _MEM_BLOCK_SIZE); adr += _32b_SIZE_ ) {ret.push_back(adr);}
      }
    }      
    
    return ret;
  }  
    



  void MemUnit::parseDownloadData(vBuf downloadData) {
    //extract and parse downloadBmp
    atDown.clear();
    gDown.clear();
    /*
    std::cout << std::endl << "BMP Down: ";
    for(unsigned int bitIdx = 0; bitIdx < bmpBits; bitIdx++) {
      
      if (downloadBmp[bitIdx / 8] & (1 << (7 - bitIdx % 8))) {std::cout << "1";}
      else {std::cout << "0";}
    }
    std::cout << std::endl;
    
    std::cout << "TableSize: " << atDown.getSize() << std::endl;
  */
    //create AllocTable and Vertices
    uint32_t nodeCnt = 0;
    for(unsigned int bitIdx = 0; bitIdx < bmpBits; bitIdx++) {
      
      if (downloadBmp[bitIdx / 8] & (1 << (7 - bitIdx % 8))) {

        uint32_t localAdr = nodeCnt * _MEM_BLOCK_SIZE; nodeCnt++;
        uint32_t adr      = startOffs + bitIdx * _MEM_BLOCK_SIZE;
        //std::cout <<  " BitIdx " << bitIdx << " 0x" << std::hex << localAdr << std::endl;
        uint32_t hash     = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&downloadData[localAdr + NODE_HASH]);
        boost::optional<std::string> name;
        // we don't know if the hash map knows about this node. If not, we'll return the hash as a string.
        try {
          name = hashMap.lookup(hash);
        } catch (...) {
          name = "#" + std::to_string(hash);
        }
        //FIXME what about future requests to hashmap if we improvised the name from hash? those will fail ...

        uint32_t flags    = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&downloadData[localAdr + NODE_FLAGS]);
        //std::cout << "DL Flags 0x" << std::hex << flags << " # 0x"  << hash << std::endl;
        uint32_t type     = (flags >> NFLG_TYPE_POS) & NFLG_TYPE_MSK;
        std::stringstream stream;
        stream << "0x" << std::setfill ('0') << std::setw(sizeof(uint32_t)*2) << std::hex << flags;
        std::string tmp(stream.str());
        vertex_t v        = boost::add_vertex(myVertex(std::string(*std::forward<boost::optional<std::string>>(name)), hash, NULL, "", tmp), gDown);
        if (!(atDown.insert(adr, hash, v))) {throw std::runtime_error( std::string("Hash or address collision when adding node ") + std::string(*std::forward<boost::optional<std::string>>(name)) + std::string(", hash (dec) ") + std::to_string(hash) + std::string(". Check if you tried to add the same .dot more than once?")); return;};
        //std::cout << "bmpBits " << std::dec << bmpBits << " localAdr: 0x" << std::hex << localAdr << ", adr 0x" << adr << std::endl;
        auto src = downloadData.begin() + localAdr;

        auto* x  = atDown.lookupAdr(adr);
        //FIXME check for NULL ?
        if (x == NULL) {throw std::runtime_error( std::string("Node at (dec) ") + std::to_string(adr) + std::string(", hash (dec) ") + std::to_string(hash) + std::string("not found. This is weird")); return;}
        std::copy(src, src + _MEM_BLOCK_SIZE, (uint8_t*)&(x->b[0]));
      
        switch(type) {
          case NODE_TYPE_TMSG         : gDown[v].np =(node_ptr) new  TimingMsg(gDown[v].name, x->hash, x->b, flags); gDown[v].np->deserialise(); break;
          case NODE_TYPE_CNOOP        : gDown[v].np =(node_ptr) new       Noop(gDown[v].name, x->hash, x->b, flags); gDown[v].np->deserialise(); break;
          case NODE_TYPE_CFLOW        : gDown[v].np =(node_ptr) new       Flow(gDown[v].name, x->hash, x->b, flags); gDown[v].np->deserialise(); break;
          case NODE_TYPE_CFLUSH       : gDown[v].np =(node_ptr) new      Flush(gDown[v].name, x->hash, x->b, flags); gDown[v].np->deserialise(); break;
          case NODE_TYPE_CWAIT        : gDown[v].np =(node_ptr) new       Wait(gDown[v].name, x->hash, x->b, flags); gDown[v].np->deserialise(); break;
          case NODE_TYPE_BLOCK_FIXED  : gDown[v].np =(node_ptr) new BlockFixed(gDown[v].name, x->hash, x->b, flags); gDown[v].np->deserialise(); break;
          case NODE_TYPE_BLOCK_ALIGN  : gDown[v].np =(node_ptr) new BlockAlign(gDown[v].name, x->hash, x->b, flags); gDown[v].np->deserialise(); break;
          case NODE_TYPE_QUEUE        : gDown[v].np =(node_ptr) new   CmdQMeta(gDown[v].name, x->hash, x->b, flags); gDown[v].np->deserialise(); break;
          case NODE_TYPE_ALTDST       : gDown[v].np =(node_ptr) new   DestList(gDown[v].name, x->hash, x->b, flags); gDown[v].np->deserialise(); break;
          case NODE_TYPE_QBUF         : gDown[v].np =(node_ptr) new CmdQBuffer(gDown[v].name, x->hash, x->b, flags); break;
          case NODE_TYPE_UNKNOWN      : std::cerr << "not yet implemented " << gDown[v].type << std::endl; break;
          default                     : std::cerr << "Node type 0x" << std::hex << type << " not supported! " << std::endl;
        }
        /*
        std::cout << gDown[v].name;
        if (gDown[v].np == NULL) std::cout <<  " has no node !" << std::endl;
        else std::cout <<  " has is ok." << std::endl;
        */
        
      }
    }

    boost::graph_traits<Graph>::vertex_iterator vi, vi_end;
    boost::tie(vi, vi_end) = vertices(gDown);
    


    
    // create edges

    //first, iterate all non meta-types to establish block -> dstList parenthood
    for(auto& it : atDown.getTable().get<Adr>()) {
      // handled by visitor
      if (gDown[it.v].np == NULL) {throw std::runtime_error( std::string("Node ") + gDown[it.v].name + std::string("not initialised")); return;
      } else {
        if  (!(gDown[it.v].np->isMeta())) gDown[it.v].np->accept(VisitorDownloadCrawler(it.v, *this));
      }  
    }
    //second, iterate all meta-types
    for(auto& it : atDown.getTable().get<Adr>()) {
      // handled by visitor
      if (gDown[it.v].np == NULL) {throw std::runtime_error( std::string("Node ") + gDown[it.v].name + std::string("not initialised")); return; 
      } else {
        if  (gDown[it.v].np->isMeta()) gDown[it.v].np->accept(VisitorDownloadCrawler(it.v, *this));
      }  
    }

    
  }  

  const vAdr MemUnit::getCmdWrAdrs(uint32_t hash, uint8_t prio) const {
    vAdr ret;  

    //find the address corresponding to given name
    auto* x = atDown.lookupHash(hash);

    if (x == NULL) {throw std::runtime_error( "Could not find target block in download address table"); return ret;}


    //Check if requested queue priority level exists
    uint32_t blAdr = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&x->b[BLOCK_CMDQ_PTRS + prio * _PTR_SIZE_]); 
    //std::cout << "Block BListAdr 0x" << std::hex << blAdr << std::endl;
    if(blAdr == LM32_NULL_PTR) {throw std::runtime_error( "Block Node does not have requested queue"); return ret; }
    
      //get Write and Read indices
    uint8_t eWrIdx = ( writeBeBytesToLeNumber<uint32_t>((uint8_t*)&x->b[BLOCK_CMDQ_WR_IDXS]) >> (prio * 8)) & Q_IDX_MAX_OVF_MSK;
    uint8_t wrIdx  = eWrIdx & Q_IDX_MAX_MSK;
    uint8_t eRdIdx = ( writeBeBytesToLeNumber<uint32_t>((uint8_t*)&x->b[BLOCK_CMDQ_RD_IDXS]) >> (prio * 8)) & Q_IDX_MAX_OVF_MSK;
    uint8_t rdIdx  = eRdIdx & Q_IDX_MAX_MSK;
    
    //Check if queue is not full
    //std::cout << "wrIdx " << (int)wrIdx << " rdIdx " << (int)rdIdx << " ewrIdx " << (int)eWrIdx << " rdIdx " << (int)rdIdx << " eRdIdx " << eRdIdx << std::endl;
    if ((wrIdx == rdIdx) && (eWrIdx != eRdIdx)) {throw std::runtime_error( "Block queue is full, can't write. "); return ret; }
    //lookup Buffer List                                                        
    auto* pmBl = atDown.lookupAdr(intAdr2adr(blAdr));

    if (pmBl == NULL) {throw std::runtime_error( "Could not find target queue in download address table"); return ret;}

    //calculate write offset                                                     

    ptrdiff_t bufIdx   = wrIdx / (_MEM_BLOCK_SIZE / _T_CMD_SIZE_  );
    ptrdiff_t elemIdx  = wrIdx % (_MEM_BLOCK_SIZE / _T_CMD_SIZE_  );

    //std::cout << "bIdx " << bufIdx << " eIdx " << elemIdx << " @ 0x" << std::hex << pmBl->adr << std::endl;
    uint32_t  startAdr = intAdr2extAdr(writeBeBytesToLeNumber<uint32_t>((uint8_t*)&pmBl->b[bufIdx * _PTR_SIZE_])) + elemIdx * _T_CMD_SIZE_;

    //std::cout << "Current BufAdr 0x" << std::hex << startAdr << std::endl;

    //generate command address range
    for(uint32_t adr = startAdr; adr < startAdr + _T_CMD_SIZE_; adr += _32b_SIZE_) ret.push_back(adr);

    //and insert address for wr idx increment
    ret.push_back(adr2extAdr(x->adr) + BLOCK_CMDQ_WR_IDXS);
    return ret;


  } 


  const uint32_t MemUnit::getCmdInc(uint32_t hash, uint8_t prio) const {
    uint32_t newIdxs;
    uint8_t  eWrIdx;

    //find the address corresponding to given name
    auto* x = atDown.lookupHash(hash);

    if (x == NULL) {throw std::runtime_error( "Could not find target block in download address table"); return 0;}
        //std::cout << "indices: 0x" << std::hex << writeBeBytesToLeNumber<uint32_t>((uint8_t*)&x->b[BLOCK_CMDQ_WR_IDXS]) << std::endl;
    //get incremented Write index of requested prio
    eWrIdx = ( writeBeBytesToLeNumber<uint32_t>((uint8_t*)&x->b[BLOCK_CMDQ_WR_IDXS]) >> (prio * 8)) & Q_IDX_MAX_OVF_MSK;
    //assign to index vector
    newIdxs = ( writeBeBytesToLeNumber<uint32_t>((uint8_t*)&x->b[BLOCK_CMDQ_WR_IDXS]) & ~(0xff << prio * 8)) | (((eWrIdx +1)  & Q_IDX_MAX_OVF_MSK) << (prio * 8));

    return newIdxs;
  }







  //Allocation functions
  int MemUnit::allocate(uint32_t hash, vertex_t v) {
    uint32_t chunkAdr;
    if (!(acquireChunk(chunkAdr)))         return ALLOC_NO_SPACE;
    if (!(atUp.insert(chunkAdr, hash, v))) return ALLOC_ENTRY_EXISTS;

    return ALLOC_OK;
  }

  bool MemUnit::deallocate(uint32_t hash) {
     /*
     std::cout << "TableSize3: " << atDown.getSize() << std::endl;
     std::cout << "Hash 0x" << std::hex << hash << std::endl;

     auto& l = atDown.getTable().get<Hash>();
    for (auto& it : l)  std::cout << "#Hash 0x" << std::hex << it.hash << std::endl;

    auto it = l.find(hash);
    if (it != l.end()) std::cout << "Found at 0x" << std::hex << it->adr << std::endl;  
    */
    auto* x = atDown.lookupHash(hash);

    if (x == NULL) {
      //std::cout << "NULL" << std::endl;
      return false;}
    if (!(freeChunk(x->adr))) {
      //std::cout << "Chunk" << std::endl;
      return false;}
    if (!(atDown.removeByHash(hash))) {
      //std::cout << "AT Hash" << std::endl; 
      return false;}
    return true;
    

  }


 

  void MemUnit::prepareUpload(Graph& g) {
    std::string cmp;
    uint32_t hash;

    gUp.clear();

    //save the graph we were shown into our own graph
    copy_graph(g, gUp);
    // for some reason, copy_graph does not copy the name
    boost::set_property(gUp, boost::graph_name, boost::get_property(g, boost::graph_name));
    
    //std::cout << "Graph name is: " << boost::get_property(gUp, boost::graph_name) << std::endl;



    //allocate and init all nodes
    BOOST_FOREACH( vertex_t v, vertices(gUp) ) {
      try { hash = hashMap.lookup(boost::get_property(gUp, boost::graph_name) + "." + gUp[v].name).get(); } catch(...) { throw; }    
      if (allocate(hash, v) == ALLOC_NO_SPACE) {throw std::runtime_error( "Memory pool for this CPU is empty"); return; }
      
      auto* x = atUp.lookupVertex(v);
      if(x == NULL) {throw std::runtime_error( std::string("Node ") + gUp[v].name + std::string(" not initialised")); return; }

      //init binary node data
      cmp = gUp[v].type;
      
      //TODO add the individual ID component representation
      /*
      std::string::size_type sz;   // alias of size_t

  int i_dec = std::stoi (gUp[v].prio,&sz);
      */

       

      if      (cmp == "tmsg")     {
        /*
        if ( gUp[v].id.find("0xD15EA5EDDEADBEEF") != std::string::npos) { // ID field was undefined. Try to construct from subfields
          id_fid id_gid;
  id_evtno;
  id_sid;
  id_bpid;
  id_res;

        }

*/

        gUp[v].np = (node_ptr) new       TimingMsg(gUp[v].name, x->hash, x->b, 0,  s2u<uint64_t>(gUp[v].tOffs), s2u<uint64_t>(gUp[v].id), s2u<uint64_t>(gUp[v].par), s2u<uint32_t>(gUp[v].tef), s2u<uint32_t>(gUp[v].res)); 


      }


      else if (cmp == "noop")     {gUp[v].np = (node_ptr) new            Noop(gUp[v].name, x->hash, x->b, 0,  s2u<uint64_t>(gUp[v].tOffs), s2u<uint64_t>(gUp[v].tValid), s2u<uint8_t>(gUp[v].prio), s2u<uint8_t>(gUp[v].qty)); }
      else if (cmp == "flow")     {gUp[v].np = (node_ptr) new            Flow(gUp[v].name, x->hash, x->b, 0,  s2u<uint64_t>(gUp[v].tOffs), s2u<uint64_t>(gUp[v].tValid), s2u<uint8_t>(gUp[v].prio), s2u<uint8_t>(gUp[v].qty)); }
      else if (cmp == "flush")    {gUp[v].np = (node_ptr) new           Flush(gUp[v].name, x->hash, x->b, 0,  s2u<uint64_t>(gUp[v].tOffs), s2u<uint64_t>(gUp[v].tValid), s2u<uint8_t>(gUp[v].prio),
                                                                              s2u<bool>(gUp[v].qIl), s2u<bool>(gUp[v].qHi), s2u<bool>(gUp[v].qLo), s2u<uint8_t>(gUp[v].frmIl), s2u<uint8_t>(gUp[v].toIl), s2u<uint8_t>(gUp[v].frmHi),
                                                                              s2u<uint8_t>(gUp[v].toHi), s2u<uint8_t>(gUp[v].frmLo), s2u<uint8_t>(gUp[v].toLo) ); }
      else if (cmp == "wait")     {gUp[v].np = (node_ptr) new            Wait(gUp[v].name, x->hash, x->b, 0,  s2u<uint64_t>(gUp[v].tOffs), s2u<uint64_t>(gUp[v].tValid), s2u<uint8_t>(gUp[v].prio), s2u<uint64_t>(gUp[v].tWait)); }
      else if (cmp == "block")    {gUp[v].np = (node_ptr) new      BlockFixed(gUp[v].name, x->hash, x->b, 0, s2u<uint64_t>(gUp[v].tPeriod) ); }
      else if (cmp == "blockfixed")    {gUp[v].np = (node_ptr) new BlockFixed(gUp[v].name, x->hash, x->b, 0, s2u<uint64_t>(gUp[v].tPeriod) ); }
      else if (cmp == "blockalign")    {gUp[v].np = (node_ptr) new BlockAlign(gUp[v].name, x->hash, x->b, 0, s2u<uint64_t>(gUp[v].tPeriod) ); }
      else if (cmp == "qinfo")    {gUp[v].np = (node_ptr) new        CmdQMeta(gUp[v].name, x->hash, x->b, 0);}
      else if (cmp == "listdst")  {gUp[v].np = (node_ptr) new        DestList(gUp[v].name, x->hash, x->b, 0);}
      else if (cmp == "qbuf")     {gUp[v].np = (node_ptr) new      CmdQBuffer(gUp[v].name, x->hash, x->b, 0);}
      else if (cmp == "meta")     {std::cerr << "Pure meta not yet implemented " << gUp[v].type << std::endl;}
      else                        {std::cerr << "Node type <" << cmp << "> not supported! " << std::endl;} 

  
    }
    BOOST_FOREACH( vertex_t v, vertices(gUp) ) { gUp[v].np->accept(VisitorUploadCrawler(v, *this)); }
        
  }

  void MemUnit::show(const std::string& title, const std::string& logDictFile, bool direction, bool filterMeta ) {

    Graph& g        = (direction == UPLOAD ? gUp  : gDown);
    AllocTable& at  = (direction == UPLOAD ? atUp : atDown);

    std::ofstream dict(logDictFile.c_str());
    std::cout << std::endl << title << std::endl;
    std::cout << std::endl << std::setfill(' ') << std::setw(4) << "Idx" << "   " << std::setw(30) << "Name" << "   " << std::setw(10) << "Hash" << "   " << std::setw(10)  <<  "Int. Adr   "  << "   " << std::setw(10) << "Ext. Adr   " << std::endl;
    //std::cout << std::setfill('-') << std::setw(50) << std::endl;      
     std::cout << std::endl; 



    BOOST_FOREACH( vertex_t v, vertices(g) ) {
      try {
        if (g[v].np == NULL) {throw std::runtime_error( std::string("Downloaded Node ") + g[v].name + std::string(" was not initialised")); return;}

        auto* x = at.lookupHash(g[v].np->getHash());
        if (x == NULL) {throw std::runtime_error( std::string("Node ") + g[v].name + std::string(" not in AllocTable")); return; }

        if( !(filterMeta) || (filterMeta & !(g[v].np->isMeta())) ) {
          std::cout << std::setfill(' ') << std::setw(4) << std::dec << x->v 
          << "   "    << std::setfill(' ') << std::setw(40) << std::left << g[v].name 
          << "   0x"  << std::hex << std::setfill('0') << std::setw(8) << x->hash
          << "   0x"  << std::hex << std::setfill('0') << std::setw(8) << adr2intAdr(x->adr) 
          << "   0x"  << std::hex << std::setfill('0') << std::setw(8) << adr2extAdr(x->adr) << std::endl;
    

          if (dict.good()) {
            dict << std::hex << "\"0x" << x->hash << "\" : \"" << g[v].name << "\"" << std::endl;
            dict << std::hex << "\"0x" << adr2intAdr(x->adr) << "\" : \"pi_" << g[v].name << "\"" << std::endl;
            dict << std::hex << "\"0x" << adr2extAdr(x->adr) << "\" : \"pe_" << g[v].name << "\"" << std::endl;
          } 
        }
      } catch(...) {
        throw std::runtime_error( std::string("Node ") + g[v].name + std::string(" not in AllocTable")); return; 
      }
    }
    std::cout << std::endl;  
  }  

  