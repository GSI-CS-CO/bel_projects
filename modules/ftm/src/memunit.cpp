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


using namespace MemUnit;
  

 

  vAdr getUploadAdrs() const {
    vAdr ret;
    uint32_t adr;

    //prepare Bmps for all memories
    for(int i = 0; i < atUp.vPool.size(); i++) {
      //generate addresses for BMP (continuous)
      for (adr = atUp.adr2extAdr(i, atUp.vPool.sharedOffs); adr < atUp.adr2extAdr(i, atUp.vPool.startOffs); adr += _32b_SIZE_) ret.push_back(adr);
    }

    //generate addresses for nodes (random access)
    for (auto& it : atUp.getTable().get<Adr>()) {
      for (adr = atUp.adr2extAdr(it.cpu, it.adr); atUp.adr2extAdr(it.cpu, it.adr + _MEM_BLOCK_SIZE); adr += _32b_SIZE_ ) ret.push_back(adr);
    }    
    return ret;
  }

  vBuf getUploadData()  {
    vBuf ret;
    
    bmpSum = 0;
    for(int i = 0; i < atUp.vPool.size(); i++) { bmpSum += atUp.vPool[i].bmpSize; }

    ret.reserve( bmpSum + atUp.getSize() * _MEM_BLOCK_SIZE); // preallocate memory for BMPs and all Nodes
    
    for(int i = 0; i < atUp.vPool.size(); i++) {
      
      atUp.vPool[i].syncBmpToPool(); //sync Bmp to Pool
      ret.insert( ret.end(), atUp.vPool[i].bmp.begin(), atUp.vPool[i].bmp.end() ); //add Bmp to transfer data
    }
   
    //FIXME this is not nice ...see alloctable.h
    //TODO find out why this isn't nice ...

    for (auto& it : atUp.getTable().get<Adr>()) { 
      ret.insert( ret.end(), it.b, it.b + _MEM_BLOCK_SIZE );
    } 
    return ret;
  }


  
  //Generate download Bmp addresses. For downloads, this has to be two pass: get bmps first, then use them to get the node locations to read 
  const vAdr getDownloadBMPAdrs() const {
    //easy 1st version: read everything in shared area
    vAdr ret;
     //prepare Bmps for all memories
    for(int i = 0; i < atDown.vPool.size(); i++) {
            //generate addresses for BMP (continuous)
      for (adr = atDown.adr2extAdr(i, atDown.vPool.sharedOffs); adr < atDown.adr2extAdr(i, atDown.vPool.startOffs); adr += _32b_SIZE_) ret.push_back(adr);
    }
    return ret;
  }

    

  const vAdr getDownloadAdrs() const {
    vAdr ret;
    //go through all memories
    for(int i = 0; i < atDown.vPool.size(); i++) {
      //go through a memory's bmp bits, starting at number of nodes the bmp itself needs (bmpSize / memblocksize). Otherwise, we'd needlessly download the bmp again
      for(unsigned int bitIdx = atDown.vPool[i].bmpBits / _MEM_BLOCK_SIZE; bitIdx < atDown.vPool[i].bmpBits; bitIdx++) {
        if (atDown.vPool[i].bmp[bitIdx / 8] & (1 << (7 - bitIdx % 8))) {
          uint32_t nodeAdr = atDown.vPool[i].startOffs + bitIdx * _MEM_BLOCK_SIZE;
          //generate address vector for a node space
          for (uint32_t adr = atDown.adr2extAdr(nodeAdr); adr < atDown.adr2extAdr(nodeAdr + _MEM_BLOCK_SIZE); adr += _32b_SIZE_ ) {ret.push_back(adr);}
        }
      }      
    }
    return ret;
  }  
    



  void parseDownloadData(vBuf downloadData) {
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
        uint8_t cpu       = (flags >> NFLG_CPU_POS) & NFLG_CPU_MSK;
        std::stringstream stream;
        stream << "0x" << std::setfill ('0') << std::setw(sizeof(uint32_t)*2) << std::hex << flags;
        std::string tmp(stream.str());
        vertex_t v        = boost::add_vertex(myVertex(std::string(*std::forward<boost::optional<std::string>>(name)), hash, NULL, "", tmp), gDown);
        if (!(atDown.insert(adr, hash, v))) {throw std::runtime_error( std::string("Hash or address collision when adding node ") + std::string(*std::forward<boost::optional<std::string>>(name)) + std::string(", hash (dec) ") + std::to_string(hash) + std::string(". Check if you tried to add the same .dot more than once?")); return;};
        //std::cout << "bmpBits " << std::dec << bmpBits << " localAdr: 0x" << std::hex << localAdr << ", adr 0x" << adr << std::endl;
        auto src = downloadData.begin() + localAdr;

        auto* x  = atDown.lookupAdr(cpu, adr);
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

  const vAdr getCmdWrAdrs(uint32_t hash, uint8_t prio) const {
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
    auto* pmBl = atDown.lookupAdr(x->cpu, atDown.intAdr2adr(x->cpu, blAdr));

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


  const uint32_t getCmdInc(uint32_t hash, uint8_t prio) const {
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







  /


 

  void prepareUpload(Graph& g) {
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

  void show(const std::string& title, const std::string& logDictFile, bool direction, bool filterMeta ) {

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

  