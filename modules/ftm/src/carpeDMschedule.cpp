#include <boost/shared_ptr.hpp>
#include <algorithm>  
#include <stdio.h>
#include <iostream>
#include <string>
#include <inttypes.h>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/copy.hpp>
#include <boost/algorithm/string.hpp>

#include "common.h"
#include "propwrite.h"
#include "graph.h"
#include "carpeDM.h"
/*
#include "node.h"
#include "block.h"
#include "meta.h"
#include "event.h"
*/
#include "visitoruploadcrawler.h"
#include "visitordownloadcrawler.h"





const unsigned char CarpeDM::deadbeef[4] = {0xDE, 0xAD, 0xBE, 0xEF};
const std::string CarpeDM::needle(CarpeDM::deadbeef, CarpeDM::deadbeef + 4);






     //TODO NC analysis
  vAdr CarpeDM::getUploadAdrs(){
    vAdr ret;
    uint32_t adr;

    //add all Bmp addresses to return vector
    for(unsigned int i = 0; i < atUp.getMemories().size(); i++) {
      //generate addresses of Bmp's address range
      for (adr = atUp.adr2extAdr(i, atUp.getMemories()[i].sharedOffs); adr < atUp.adr2extAdr(i, atUp.getMemories()[i].startOffs); adr += _32b_SIZE_) ret.push_back(adr);

      //std::cout << "CPU " << i << std::endl;
      //for(auto& it : ret) {  std::cout << std::hex << "0x" << it << std::endl; }
    }

    //add all Node addresses to return vector
    for (auto& it : atUp.getTable().get<CpuAdr>()) {
      //generate addresses of node's address range
      //std::cout << "cpu " << it.cpu << std::hex << "node 0x" << atUp.adr2extAdr(it.cpu, it.adr) << std::endl; 
      for (adr = atUp.adr2extAdr(it.cpu, it.adr); adr < atUp.adr2extAdr(it.cpu, it.adr + _MEM_BLOCK_SIZE); adr += _32b_SIZE_ ) ret.push_back(adr);
    }    
    return ret;
  }

  vBuf CarpeDM::getUploadData()  {
    vBuf ret;
    
    size_t bmpSum = 0;
    for(unsigned int i = 0; i < atUp.getMemories().size(); i++) { bmpSum += atUp.getMemories()[i].bmpSize; }
    //FIXME Careful, if bmpSum is not aligned this can kill!
    // std::cout << "mem reserved " << bmpSum + atUp.getSize() * _MEM_BLOCK_SIZE << " equals " << (bmpSum + atUp.getSize() * _MEM_BLOCK_SIZE) / _MEM_BLOCK_SIZE << " nodes, " << atUp.getMemories().size() << " memory" << std::endl; 
    ret.reserve( bmpSum + atUp.getSize() * _MEM_BLOCK_SIZE); // preallocate memory for BMPs and all Nodes
    
  


    for(unsigned int i = 0; i < atUp.getMemories().size(); i++) {
      auto& tmpBmp = atUp.getMemories()[i].getBmp();
      //vHexDump("bmpUpb4", tmpBmp);
      atUp.getMemories()[i].syncBmpToPool(); //sync Bmp to Pool
      //vHexDump("bmpUpafter", tmpBmp);
      
      //std::cout << "test bmp size " << tmpBmp.size() << " iterator diff " << tmpBmp.end() - tmpBmp.begin()  << " aligned size " << atUp.getMemories()[i].bmpSize << " bits " << atUp.getMemories()[i].bmpBits << std::endl;
      
      ret.insert( ret.end(), tmpBmp.begin(), tmpBmp.end() ); //add Bmp to to return vector  
    }
    //std::cout << "passed bmp" << std::endl;
    //add all node buffers to return vector  
    for (auto& it : atUp.getTable().get<CpuAdr>()) { 
      ret.insert( ret.end(), it.b, it.b + _MEM_BLOCK_SIZE );
    }
    //std::cout << "passed nodes" << std::endl;
    return ret;
  }



  void CarpeDM::generateDstLst(Graph& g, vertex_t v) {
    const std::string name = g[v].name + "_ListDst";
    hm.add(name);
    vertex_t vD = boost::add_vertex(myVertex(name, g[v].cpu, hm.lookup(name).get(), NULL, "listdst", "0x0"), g);
    boost::add_edge(v,   vD, myEdge(sDL), g);
  }  

  void CarpeDM::generateQmeta(Graph& g, vertex_t v, int prio) {
    const std::string sPrefix[] = {"Lo", "Hi", "Il"};

    const std::string nameBl = g[v].name + "_QBl_" + sPrefix[prio];
    const std::string nameB0 = g[v].name + "_Qb_"  + sPrefix[prio] + "0";
    const std::string nameB1 = g[v].name + "_Qb_"  + sPrefix[prio] + "1";
    hm.add(nameBl);
    hm.add(nameB0);
    hm.add(nameB1);
    vertex_t vBl = boost::add_vertex(myVertex(nameBl, g[v].cpu, hm.lookup(nameBl).get(), NULL, "qinfo", "0x0"), g);
    vertex_t vB0 = boost::add_vertex(myVertex(nameB0, g[v].cpu, hm.lookup(nameB0).get(), NULL, "qbuf",  "0x0"), g);
    vertex_t vB1 = boost::add_vertex(myVertex(nameB1, g[v].cpu, hm.lookup(nameB1).get(), NULL, "qbuf",  "0x0"), g);
    boost::add_edge(v,   vBl, myEdge(sQM[prio]), g);
    boost::add_edge(vBl, vB0, myEdge("meta"),    g);
    boost::add_edge(vBl, vB1, myEdge("meta"),    g);
    
  }

  void CarpeDM::generateBlockMeta() {
   Graph& g = gUp;
   Graph::out_edge_iterator out_begin, out_end, out_cur;
    
    BOOST_FOREACH( vertex_t v, vertices(g) ) {
      std::string cmp = g[v].type;
      
      if ((cmp == "blockfixed") || (cmp == "blockalign") || (cmp == "block") ) {
        boost::tie(out_begin, out_end) = out_edges(v,g);
        //check if it already has queue links / Destination List
        bool hasIl=false, hasHi=false, hasLo=false, hasMultiDst=false, hasDstLst=false;
        for (out_cur = out_begin; out_cur != out_end; ++out_cur)
        { 
          
          if (g[*out_cur].type == sQM[PRIO_IL]) hasIl       = true;
          if (g[*out_cur].type == sQM[PRIO_HI]) hasHi       = true;
          if (g[*out_cur].type == sQM[PRIO_LO]) hasLo       = true;
          if (g[*out_cur].type == sAD)          hasMultiDst = true;
          if (g[*out_cur].type == sDL)          hasDstLst   = true;
        }
        //create requested Queues / Destination List
        if (g[v].qIl != "0" && !hasIl ) { generateQmeta(g, v, PRIO_IL); }
        if (g[v].qHi != "0" && !hasHi ) { generateQmeta(g, v, PRIO_HI); }
        if (g[v].qLo != "0" && !hasLo ) { generateQmeta(g, v, PRIO_LO); }
        if(hasMultiDst & !hasDstLst)    { generateDstLst(g, v);         }

      }
    }  
  }


  void CarpeDM::prepareUpload(Graph& g) {
    std::string cmp;
    uint32_t hash;
    uint8_t cpu;
    int allocState;

    atUp.clear();
    gUp.clear();
    atUp.syncBmps(atDown);

    //save the graph we were shown into our own graph
    copy_graph(g, gUp);
    // for some reason, copy_graph does not copy the name
    boost::set_property(gUp, boost::graph_name, boost::get_property(g, boost::graph_name));
    
    //std::cout << "Graph name is: " << boost::get_property(gUp, boost::graph_name) << std::endl;
    
    //auto generate desired Block Meta Nodes first
    generateBlockMeta();
    

    //allocate and init all nodes
    BOOST_FOREACH( vertex_t v, vertices(gUp) ) {
      //std::string name = boost::get_property(gUp, boost::graph_name) + "." + gUp[v].name;
      std::string name = gUp[v].name;
      if (!(hm.lookup(name)))                   {throw std::runtime_error("Node '" + name + "' was unknown to the hashmap"); return;}
      hash = hm.lookup(name).get();
      cpu  = s2u<uint8_t>(gUp[v].cpu);
      //FIXME Careful! CPU indices in the (intermediary) .dot do not necessarily match the vector indices. Use the fucking cpuIdx map to translate!
      if (atDown.lookupHash(hash) != NULL)      {throw std::runtime_error("Node '" + name + "' already present on DM.\nThe combination <graphname.nodename> must be unique."); return; } 
      allocState = atUp.allocate(cpu, hash, v);
      if (allocState == ALLOC_NO_SPACE)         {throw std::runtime_error("Not enough space in CPU " + std::to_string(cpu) + " memory pool"); return; }
      if (allocState == ALLOC_ENTRY_EXISTS)     {throw std::runtime_error("Node '" + name + "' is duplicate in graph. .\nThe combination <graphname.nodename> must be unique."); return; }

      // this means alloc went okay
      auto* x = atUp.lookupVertex(v);
      

      cmp = gUp[v].type;
      
      if      (cmp == "tmsg")     {
        uint64_t id;
        // if ID field was not given, try to construct from subfields
        if ( gUp[v].id.find("0xD15EA5EDDEADBEEF") != std::string::npos) { 
          id =  ((s2u<uint64_t>(gUp[v].id_fid)    & ID_FID_MSK)   << ID_FID_POS)   |
                ((s2u<uint64_t>(gUp[v].id_gid)    & ID_GID_MSK)   << ID_GID_POS)   |
                ((s2u<uint64_t>(gUp[v].id_evtno)  & ID_EVTNO_MSK) << ID_EVTNO_POS) |
                ((s2u<uint64_t>(gUp[v].id_sid)    & ID_SID_MSK)   << ID_SID_POS)   |
                ((s2u<uint64_t>(gUp[v].id_bpid)   & ID_BPID_MSK)  << ID_BPID_POS)  |
                ((s2u<uint64_t>(gUp[v].id_res)    & ID_RES_MSK)   << ID_RES_POS);
        } else { id = s2u<uint64_t>(gUp[v].id); }
        gUp[v].np = (node_ptr) new       TimingMsg(gUp[v].name, x->hash, x->b, 0,  s2u<uint64_t>(gUp[v].tOffs), id, s2u<uint64_t>(gUp[v].par), s2u<uint32_t>(gUp[v].tef), s2u<uint32_t>(gUp[v].res)); 
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
      else if (cmp == "meta")     {throw std::runtime_error("Pure meta type not yet implemented"); return;}
      else                        {throw std::runtime_error("Node <" + gUp[v].name + ">'s type <" + cmp + "> is not supported!\nMost likely you forgot to set the type attribute or accidentally created the node by a typo in an edge definition."); return;} 

  
    }





    BOOST_FOREACH( vertex_t v, vertices(gUp) ) { 
      //serialise node
      gUp[v].np->accept(VisitorUploadCrawler(gUp, v, atUp)); 
   
      //Check if all mandatory fields were properly initialised
      std::string haystack(gUp[v].np->getB(), gUp[v].np->getB() + _MEM_BLOCK_SIZE);
      std::size_t n = haystack.find(needle);

      bool foundUninitialised = (n != std::string::npos);

      if(verbose || foundUninitialised) {
        sLog << std::endl;
        hexDump(gUp[v].name.c_str(), (void*)haystack.c_str(), _MEM_BLOCK_SIZE);
      }

      if(foundUninitialised) {
        throw std::runtime_error("Node '" + gUp[v].name + "'contains uninitialised elements!\nMisspelled/forgot a mandatory property in .dot file ?"); 
      } 
    }
  }
  

  int CarpeDM::upload() {
    vBuf vUlD = getUploadData();
    vAdr vUlA = getUploadAdrs();

    //Upload
    ebWriteCycle(ebd, vUlA, vUlD);
    if(verbose) sLog << "Done." << std::endl;
    return vUlD.size();
  }




  int CarpeDM::removeDot(const std::string& fn) {
   
    Graph gTmp, gEmpty;
    parseDot(fn, gTmp); 
    uint32_t hash;

    //remove all nodes in input file from download allocation table
    try {
      //combine node name and graph name to obtain unique replicable hash
      BOOST_FOREACH( vertex_t v, vertices(gTmp) ) { 
        //hash = hm.lookup(boost::get_property(gTmp, boost::graph_name) + "." + gTmp[v].name).get();
        hash = hm.lookup(gTmp[v].name).get();
        if (!(atDown.deallocate(hash))) { if(verbose) {sLog << "Node " << gTmp[v].name << " could not be removed" << std::endl;}}
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





  
  //Generate download Bmp addresses. For downloads, this has to be two pass: get bmps first, then use them to get the node locations to read 
  const vAdr CarpeDM::getDownloadBMPAdrs() {
    vAdr ret;

     //add all Bmp addresses to return vector
    for(unsigned int i = 0; i < atDown.getMemories().size(); i++) {
      //generate addresses of Bmp's address range
      for (uint32_t adr = atDown.adr2extAdr(i, atDown.getMemories()[i].sharedOffs); adr < atDown.adr2extAdr(i, atDown.getMemories()[i].startOffs); adr += _32b_SIZE_) ret.push_back(adr);
    }
    return ret;
  }

    

  const vAdr CarpeDM::getDownloadAdrs() {
    vAdr ret;
    //go through Memories
    for(unsigned int i = 0; i < atDown.getMemories().size(); i++) {
      //go through a memory's bmp bits, starting at number of nodes the bmp itself needs (bmpSize / memblocksize). Otherwise, we'd needlessly download the bmp again
      for(unsigned int bitIdx = atDown.getMemories()[i].bmpSize / _MEM_BLOCK_SIZE; bitIdx < atDown.getMemories()[i].bmpBits; bitIdx++) {
        //if the bit says the node is used, we add the node to read addresses
        //sLog << "Bit Idx " << bitIdx << " valid " << atDown.getMemories()[i].getBmpBit(bitIdx) << " na 0x" << std::hex << atDown.getMemories()[i].sharedOffs + bitIdx * _MEM_BLOCK_SIZE << std::endl;
        if (atDown.getMemories()[i].getBmpBit(bitIdx)) {
          uint32_t nodeAdr = atDown.getMemories()[i].sharedOffs + bitIdx * _MEM_BLOCK_SIZE;
           //generate addresses of node's address range
          for (uint32_t adr = atDown.adr2extAdr(i, nodeAdr); adr < atDown.adr2extAdr(i, nodeAdr + _MEM_BLOCK_SIZE); adr += _32b_SIZE_ ) {ret.push_back(adr);}
        }
      }      
    }
    return ret;
  }  
    



  void CarpeDM::parseDownloadData(vBuf downloadData) {
    atDown.clear();
    gDown.clear();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //create AllocTable and Vertices
    //sLog << std::dec << "dl size " << downloadData.size() << std::endl;

    uint32_t nodeCnt = 0;
    //go through Memories
    for(unsigned int i = 0; i < atDown.getMemories().size(); i++) {
      //go through Bmp
      for(unsigned int bitIdx = atDown.getMemories()[i].bmpSize / _MEM_BLOCK_SIZE; bitIdx < atDown.getMemories()[i].bmpBits; bitIdx++) {
        if (atDown.getMemories()[i].getBmpBit(bitIdx)) {

          uint32_t    localAdr  = nodeCnt * _MEM_BLOCK_SIZE; nodeCnt++; 
          uint32_t    adr       = atDown.getMemories()[i].sharedOffs + bitIdx * _MEM_BLOCK_SIZE;
          uint32_t    hash      = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&downloadData[localAdr + NODE_HASH]);
          //sLog << std::dec << "Offset " << localAdr + NODE_HASH << std::endl;
          std::string name      = hm.lookup(hash) ? hm.lookup(hash).get() : "#" + std::to_string(hash);
          uint32_t    flags     = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&downloadData[localAdr + NODE_FLAGS]); //FIXME what about future requests to hashmap if we improvised the name from hash? those will fail ...
          uint32_t    type      = (flags >> NFLG_TYPE_POS) & NFLG_TYPE_MSK;
          uint8_t     cpu       = i;
          


          //Vertex needs flags as a std::string. Convert to hex
          std::stringstream stream;
          stream << "0x" << std::setfill ('0') << std::setw(sizeof(uint32_t)*2) << std::hex << flags;
          std::string tmp(stream.str());

          //Add Vertex
          vertex_t v        = boost::add_vertex(myVertex(name, std::to_string(cpu), hash, NULL, "", tmp), gDown);
          //std::cout << "atdown cpu " << (int)cpu << " Adr: 0x" << std::hex << adr <<  " Hash 0x" << hash << std::endl;
          //Add allocTable Entry
          //vBuf test(downloadData.begin() + localAdr, downloadData.begin() + localAdr + _MEM_BLOCK_SIZE);
          //vHexDump("TEST ****", test);

      

          if (!(atDown.insert(cpu, adr, hash, v))) {throw std::runtime_error( std::string("Hash or address collision when adding node ") + name); return;};


          

          // Create node object for Vertex
          auto src = downloadData.begin() + localAdr;



          auto* x  = atDown.lookupAdr(cpu, adr);
          if (x == NULL) {throw std::runtime_error( std::string("Node at (dec) ") + std::to_string(adr) + std::string(", hash (dec) ") + std::to_string(hash) + std::string("not found. This is weird")); return;}
               
          std::copy(src, src + _MEM_BLOCK_SIZE, (uint8_t*)&(x->b[0]));
        
          //hexDump("buf", (uint8_t*)&(x->b[0]), _MEM_BLOCK_SIZE);

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
   
        }
      }
    }

 
    


    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // create edges

    //Two-pass for edges. First, iterate all non meta-types to establish block -> dstList parenthood
    for(auto& it : atDown.getTable().get<Hash>()) {
      // handled by visitor
      if (gDown[it.v].np == NULL) {throw std::runtime_error( std::string("Node ") + gDown[it.v].name + std::string("not initialised")); return;
      } else {
        if  (!(gDown[it.v].np->isMeta())) gDown[it.v].np->accept(VisitorDownloadCrawler(gDown, it.v, atDown));
      }  
    }
    //second, iterate all meta-types
    for(auto& it : atDown.getTable().get<Hash>()) {
      // handled by visitor
      if (gDown[it.v].np == NULL) {throw std::runtime_error( std::string("Node ") + gDown[it.v].name + std::string("not initialised")); return; 
      } else {
        if  (gDown[it.v].np->isMeta()) gDown[it.v].np->accept(VisitorDownloadCrawler(gDown, it.v, atDown));
      }  
    }

    
  }  

    //TODO assign to CPUs/threads


   int CarpeDM::download() {
    
    vAdr vDlBmpA, vlDlA;
    vBuf vDlBmpD, vDlD;

    //get all BMPs so we know which nodes to download
    if(verbose) sLog << "Downloading ...";
    vDlBmpD = ebReadCycle(ebd, getDownloadBMPAdrs());
    atDown.setBmps( vDlBmpD );
    vDlD = ebReadCycle(ebd, getDownloadAdrs());

    if(verbose) sLog << "Done." << std::endl << "Parsing ...";
    parseDownloadData(vDlD);
    if(verbose) sLog << "Done." << std::endl;
    
    return vDlD.size();
  }




 

  

  void CarpeDM::show(const std::string& title, const std::string& logDictFile, bool direction, bool filterMeta ) {

    Graph& g        = (direction == UPLOAD ? gUp  : gDown);
    AllocTable& at  = (direction == UPLOAD ? atUp : atDown);

    std::ofstream dict(logDictFile.c_str());
    std::cout << std::endl << title << std::endl;
    std::cout << std::endl << std::setfill(' ') << std::setw(4) << "Idx" << "   " << std::setfill(' ') << std::setw(4) << "Cpu" << "   " << std::setw(30) << "Name" << "   " << std::setw(10) << "Hash" << "   " << std::setw(10)  <<  "Int. Adr   "  << "   " << std::setw(10) << "Ext. Adr   " << std::endl;
    //std::cout << std::setfill('-') << std::setw(50) << std::endl;      
     std::cout << std::endl; 



    BOOST_FOREACH( vertex_t v, vertices(g) ) {
      try {
        if (g[v].np == NULL) {throw std::runtime_error( std::string("Downloaded Node ") + g[v].name + std::string(" was not initialised")); return;}

        auto* x = at.lookupHash(g[v].np->getHash());
        if (x == NULL) {throw std::runtime_error( std::string("Node ") + g[v].name + std::string(" not in AllocTable")); return; }

        if( !(filterMeta) || (filterMeta & !(g[v].np->isMeta())) ) {
          std::cout << std::setfill(' ') << std::setw(4) << std::dec << x->v
          << "   "    << std::setfill(' ') << std::setw(4) << std::dec << (int)x->cpu 
          << "   "    << std::setfill(' ') << std::setw(40) << std::left << g[v].name 
          << "   0x"  << std::hex << std::setfill('0') << std::setw(8) << x->hash
          << "   0x"  << std::hex << std::setfill('0') << std::setw(8) << at.adr2intAdr(x->cpu, x->adr) 
          << "   0x"  << std::hex << std::setfill('0') << std::setw(8) << at.adr2extAdr(x->cpu, x->adr) << std::endl;
    

          if (dict.good()) {
            dict << std::hex << "\"0x" << x->hash << "\" : \"" << g[v].name << "\"" << std::endl;
            dict << std::hex << "\"0x" << at.adr2intAdr(x->cpu, x->adr) << "\" : \"pi_" << g[v].name << "\"" << std::endl;
            dict << std::hex << "\"0x" << at.adr2extAdr(x->cpu, x->adr) << "\" : \"pe_" << g[v].name << "\"" << std::endl;
          } 
        }
      } catch(...) {
        throw std::runtime_error( std::string("Node ") + g[v].name + std::string(" not in AllocTable")); return; 
      }
    }
    std::cout << std::endl;  
  }  

  