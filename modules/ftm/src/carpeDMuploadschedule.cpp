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
#include "visitoruploadcrawler.h"
#include "visitordownloadcrawler.h"
#include "dotstr.h"

using namespace DotStr;


     //TODO NC analysis
  vAdr CarpeDM::getUploadAdrs(){
    vAdr ret;
    uint32_t adr;

    //add all Bmp addresses to return vector
    for(unsigned int i = 0; i < atUp.getMemories().size(); i++) {
      //generate addresses of Bmp's address range
      for (adr = atUp.adr2extAdr(i, atUp.getMemories()[i].sharedOffs); adr < atUp.adr2extAdr(i, atUp.getMemories()[i].startOffs); adr += _32b_SIZE_) ret.push_back(adr);

      
    }

    //add all Node addresses to return vector
    for (auto& it : atUp.getTable().get<CpuAdr>()) {
      //generate address range for all nodes staged for upload
      if(it.staged) {
       //std::cout << std::hex << "Adding Node @ 0x" << atUp.adr2extAdr(it.cpu, it.adr) << std::endl;

       for (adr = atUp.adr2extAdr(it.cpu, it.adr); adr < atUp.adr2extAdr(it.cpu, it.adr + _MEM_BLOCK_SIZE); adr += _32b_SIZE_ ) ret.push_back(adr); 
     }
    }    
    return ret;
  }

  vBuf CarpeDM::getUploadData()  {
    vBuf ret;
    ret.clear();
    
    size_t bmpSum = 0;
    for(unsigned int i = 0; i < atUp.getMemories().size(); i++) { bmpSum += atUp.getMemories()[i].bmpSize; }
    //FIXME Careful, if bmpSum is not aligned this can kill!
    // std::cout << "mem reserved " << bmpSum + atUp.getSize() * _MEM_BLOCK_SIZE << " equals " << (bmpSum + atUp.getSize() * _MEM_BLOCK_SIZE) / _MEM_BLOCK_SIZE << " nodes, " << atUp.getMemories().size() << " memory" << std::endl; 
    ret.reserve( bmpSum + atUp.getSize() * _MEM_BLOCK_SIZE); // preallocate memory for BMPs and all Nodes
    
    for(unsigned int i = 0; i < atUp.getMemories().size(); i++) {
      auto& tmpBmp = atUp.getMemories()[i].getBmp();
      //vHexDump("bmpUpb4", tmpBmp);
      //atUp.getMemories()[i].syncBmpToPool(); //sync Bmp to Pool
      //vHexDump("bmpUpafter", tmpBmp);
      
      //std::cout << "test bmp size " << tmpBmp.size() << " iterator diff " << tmpBmp.end() - tmpBmp.begin()  << " aligned size " << atUp.getMemories()[i].bmpSize << " bits " << atUp.getMemories()[i].bmpBits << std::endl;
      
      ret.insert( ret.end(), tmpBmp.begin(), tmpBmp.end() ); //add Bmp to to return vector  
    }
    //std::cout << "passed bmp" << std::endl;
    //add all node buffers to return vector
    atUp.debug();

    for (auto& it : atUp.getTable().get<CpuAdr>()) {
      if(it.staged) {
        //hexDump(gUp[it.v].name.c_str(), (const uint8_t*)it.b, _MEM_BLOCK_SIZE); 
        ret.insert( ret.end(), it.b, it.b + _MEM_BLOCK_SIZE );
      } // add all nodes staged for upload
    }
    //std::cout << "passed nodes" << std::endl;
    return ret;
  }



  void CarpeDM::generateDstLst(Graph& g, vertex_t v) {
    const std::string name = g[v].name + tDstListSuffix;
    hm.add(name);
    vertex_t vD = boost::add_vertex(myVertex(name, g[v].cpu, hm.lookup(name).get(), NULL, nDstList, tHexZero), g);
    boost::add_edge(v,   vD, myEdge(eDstList), g);
  }  

  void CarpeDM::generateQmeta(Graph& g, vertex_t v, int prio) {


    const std::string nameBl = g[v].name + tQBufListTag + tQPrioPrefix[prio];
    const std::string nameB0 = g[v].name + tQBufTag     + tQPrioPrefix[prio] + t1stQBufSuffix;
    const std::string nameB1 = g[v].name + tQBufTag     + tQPrioPrefix[prio] + t2ndQBufSuffix;
    hm.add(nameBl);
    hm.add(nameB0);
    hm.add(nameB1);
    vertex_t vBl = boost::add_vertex(myVertex(nameBl, g[v].cpu, hm.lookup(nameBl).get(), NULL, nQInfo, tHexZero), g);
    vertex_t vB0 = boost::add_vertex(myVertex(nameB0, g[v].cpu, hm.lookup(nameB0).get(), NULL, nQBuf,  tHexZero), g);
    vertex_t vB1 = boost::add_vertex(myVertex(nameB1, g[v].cpu, hm.lookup(nameB1).get(), NULL, nQBuf,  tHexZero), g);
    boost::add_edge(v,   vBl, myEdge(eQPrio[prio]), g);
    boost::add_edge(vBl, vB0, myEdge(nMeta),    g);
    boost::add_edge(vBl, vB1, myEdge(nMeta),    g);
    
  }

  void CarpeDM::generateBlockMeta(Graph& g) {
   Graph::out_edge_iterator out_begin, out_end, out_cur;
    
    BOOST_FOREACH( vertex_t v, vertices(g) ) {
      std::string cmp = g[v].type;
      
      if ((cmp == nBlockFixed) || (cmp == nBlockAlign) || (cmp == nBlock) ) {
        boost::tie(out_begin, out_end) = out_edges(v,g);
        //check if it already has queue links / Destination List
        bool hasIl=false, hasHi=false, hasLo=false, hasMultiDst=false, hasDstLst=false;
        for (out_cur = out_begin; out_cur != out_end; ++out_cur)
        { 
          
          if (g[*out_cur].type == eQPrio[PRIO_IL]) hasIl       = true;
          if (g[*out_cur].type == eQPrio[PRIO_HI]) hasHi       = true;
          if (g[*out_cur].type == eQPrio[PRIO_LO]) hasLo       = true;
          if (g[*out_cur].type == eAltDst)          hasMultiDst = true;
          if (g[*out_cur].type == eDstList)          hasDstLst   = true;
        }
        //create requested Queues / Destination List
        if (g[v].qIl != "0" && !hasIl ) { generateQmeta(g, v, PRIO_IL); }
        if (g[v].qHi != "0" && !hasHi ) { generateQmeta(g, v, PRIO_HI); }
        if (g[v].qLo != "0" && !hasLo ) { generateQmeta(g, v, PRIO_LO); }
        if((hasMultiDst | ((g[v].qIl != "0") | hasIl) | ((g[v].qHi != "0") | hasHi) |  ((g[v].qLo != "0") | hasLo)) & !hasDstLst)    { generateDstLst(g, v);         }

      }
    }  
  }

  


  void CarpeDM::updateListDstStaging(vertex_t v) {
    Graph::out_edge_iterator out_begin, out_end, out_cur;
    Graph& g = gUp;
    AllocTable& at = atUp;
    //std::cout << g[v].name << "'s alt destination list changed" << std::endl;
    // the changed edge leads to Alternative Dst, find and stage the block's dstList 
    boost::tie(out_begin, out_end) = out_edges(v,g);  
    for (out_cur = out_begin; out_cur != out_end; ++out_cur) {
      if (g[*out_cur].type == eDstList) {
        auto dst = at.lookupVertex(target(*out_cur, g));
        if (at.isOk(dst)) { at.setStaged(dst); }// if we found a Dst List, stage it
        else throw std::runtime_error("Dst List '" + g[dst->v].name + "' was not allocated, this is very bad");
        break;
      }
    }
  }

  void CarpeDM::updateStaging(vertex_t v, edge_t e)  {
    // staging changes
    Graph& g = gUp;
    AllocTable& at = atUp;
    //std::cout << g[v].name << ", now parent of " << g[target(e, g)].name  << std::endl;

    if (g[e].type == eAltDst) {
      
      updateListDstStaging(v); // stage source block's Destination List
    } else {
      auto x = at.lookupVertex(v);
      if(at.isOk(x)) {at.setStaged(x); }
      else throw std::runtime_error("Node '" + g[v].name + "' was not allocated, this is very bad");
      
    }

  }


  void CarpeDM::mergeUploadDuplicates(vertex_t borg, vertex_t victim) {
    Graph& g = gUp;

    // add all of nod 'victim's edges to node 'borg'. Resistance is futile.
    sLog << g[borg].name << " @ " << borg << " consumes " << g[victim].name << " @ " << victim << std::endl;

    Graph::out_edge_iterator out_begin, out_end, out_cur;
    boost::tie(out_begin, out_end) = out_edges(victim, g);
    
    // out edges
    for (out_cur = out_begin; out_cur != out_end; ++out_cur) {
      //sLog << " moving out edge " << std::endl;
      boost::add_edge(borg, target(*out_cur,g), (myEdge){boost::get(&myEdge::type, g, *out_cur)}, g);
      boost::remove_edge(*out_cur, g);
      updateStaging(borg, *out_cur); 
    }  
    
    // in egdes
    Graph::in_edge_iterator in_begin, in_end, in_cur;
    boost::tie(in_begin, in_end) = in_edges(victim, g);
    for (in_cur = in_begin; in_cur != in_end; ++in_cur) {
      //sLog << " moving in edge " << std::endl;
      boost::add_edge(source(*in_cur,g), borg, (myEdge){boost::get(&myEdge::type, g, *in_cur)}, g);
      boost::remove_edge(*in_cur, g);
       
    }
    

    
  }

  void CarpeDM::prepareUpload() {
    
    std::string cmp;
    uint32_t hash;
    uint8_t cpu;
    int allocState;
    

    //allocate and init all new vertices
    BOOST_FOREACH( vertex_t v, vertices(gUp) ) {
      //std::string name = boost::get_property(gUp, boost::graph_name) + "." + gUp[v].name;
      std::string name = gUp[v].name;
      if (!(hm.lookup(name)))                   {throw std::runtime_error("Node '" + name + "' was unknown to the hashmap"); return;}
      hash = hm.lookup(name).get();
      cpu  = s2u<uint8_t>(gUp[v].cpu);
      //FIXME Careful! CPU indices in the (intermediary) .dot do not necessarily match the vector indices. Use the fucking cpuIdx map to translate!


      amI it = atUp.lookupHash(hash); //if we already have a download entry, keep allocation, but update vertex index
      if (!(atUp.isOk(it))) {
        //  
        //sLog << "Adding " << name << std::endl;
        allocState = atUp.allocate(cpu, hash, v, true);
        if (allocState == ALLOC_NO_SPACE)         {throw std::runtime_error("Not enough space in CPU " + std::to_string(cpu) + " memory pool"); return; }
        if (allocState == ALLOC_ENTRY_EXISTS)     {throw std::runtime_error("Node '" + name + "' would be duplicate in graph."); return; }
        // getting here means alloc went okay
        it = atUp.lookupHash(hash);
      }  
      
      //TODO Find something better than stupic cast to ptr
      //Ugly as hell. But otherwise the bloody iterator will only allow access to MY alloc buffers (not their pointers!) as const!
      auto* x = (AllocMeta*)&(*it);

      // add timing node data objects to vertices
      if(gUp[v].np == NULL) {

        cmp = gUp[v].type;
      
        if      (cmp == nTMsg)     {
          uint64_t id;
          // if ID field was not given, try to construct from subfields
          if ( gUp[v].id.find(tUndefined64) != std::string::npos) { 
            id =  ((s2u<uint64_t>(gUp[v].id_fid)    & ID_FID_MSK)   << ID_FID_POS)   |
                  ((s2u<uint64_t>(gUp[v].id_gid)    & ID_GID_MSK)   << ID_GID_POS)   |
                  ((s2u<uint64_t>(gUp[v].id_evtno)  & ID_EVTNO_MSK) << ID_EVTNO_POS) |
                  ((s2u<uint64_t>(gUp[v].id_sid)    & ID_SID_MSK)   << ID_SID_POS)   |
                  ((s2u<uint64_t>(gUp[v].id_bpid)   & ID_BPID_MSK)  << ID_BPID_POS)  |
                  ((s2u<uint64_t>(gUp[v].id_res)    & ID_RES_MSK)   << ID_RES_POS);
          } else { id = s2u<uint64_t>(gUp[v].id); }
          gUp[v].np = (node_ptr) new       TimingMsg(gUp[v].name, x->hash, x->cpu, x->b, 0,  s2u<uint64_t>(gUp[v].tOffs), id, s2u<uint64_t>(gUp[v].par), s2u<uint32_t>(gUp[v].tef), s2u<uint32_t>(gUp[v].res)); 
        }
        else if (cmp == nCmdNoop)     {gUp[v].np = (node_ptr) new            Noop(gUp[v].name, x->hash, x->cpu, x->b, 0,  s2u<uint64_t>(gUp[v].tOffs), s2u<uint64_t>(gUp[v].tValid), s2u<uint8_t>(gUp[v].prio), s2u<uint8_t>(gUp[v].qty)); }
        else if (cmp == nCmdFlow)     {gUp[v].np = (node_ptr) new            Flow(gUp[v].name, x->hash, x->cpu, x->b, 0,  s2u<uint64_t>(gUp[v].tOffs), s2u<uint64_t>(gUp[v].tValid), s2u<uint8_t>(gUp[v].prio), s2u<uint8_t>(gUp[v].qty)); }
        else if (cmp == nCmdFlush)    {gUp[v].np = (node_ptr) new           Flush(gUp[v].name, x->hash, x->cpu, x->b, 0,  s2u<uint64_t>(gUp[v].tOffs), s2u<uint64_t>(gUp[v].tValid), s2u<uint8_t>(gUp[v].prio),
                                                                                s2u<bool>(gUp[v].qIl), s2u<bool>(gUp[v].qHi), s2u<bool>(gUp[v].qLo), s2u<uint8_t>(gUp[v].frmIl), s2u<uint8_t>(gUp[v].toIl), s2u<uint8_t>(gUp[v].frmHi),
                                                                                s2u<uint8_t>(gUp[v].toHi), s2u<uint8_t>(gUp[v].frmLo), s2u<uint8_t>(gUp[v].toLo) ); }
        else if (cmp == nCmdWait)     {gUp[v].np = (node_ptr) new            Wait(gUp[v].name, x->hash, x->cpu, x->b, 0,  s2u<uint64_t>(gUp[v].tOffs), s2u<uint64_t>(gUp[v].tValid), s2u<uint8_t>(gUp[v].prio), s2u<uint64_t>(gUp[v].tWait)); }
        else if (cmp == nBlock)    {gUp[v].np = (node_ptr) new      BlockFixed(gUp[v].name, x->hash, x->cpu, x->b, 0, s2u<uint64_t>(gUp[v].tPeriod) ); }
        else if (cmp == nBlockFixed)    {gUp[v].np = (node_ptr) new BlockFixed(gUp[v].name, x->hash, x->cpu, x->b, 0, s2u<uint64_t>(gUp[v].tPeriod) ); }
        else if (cmp == nBlockAlign)    {gUp[v].np = (node_ptr) new BlockAlign(gUp[v].name, x->hash, x->cpu, x->b, 0, s2u<uint64_t>(gUp[v].tPeriod) ); }
        else if (cmp == nQInfo)    {gUp[v].np = (node_ptr) new        CmdQMeta(gUp[v].name, x->hash, x->cpu, x->b, 0);}
        else if (cmp == nDstList)  {gUp[v].np = (node_ptr) new        DestList(gUp[v].name, x->hash, x->cpu, x->b, 0);}
        else if (cmp == nQBuf)     {gUp[v].np = (node_ptr) new      CmdQBuffer(gUp[v].name, x->hash, x->cpu, x->b, 0);}
        else if (cmp == nMeta)     {throw std::runtime_error("Pure meta type not yet implemented"); return;}
        //FIXME try to get info from download
        else                        {throw std::runtime_error("Node <" + gUp[v].name + ">'s type <" + cmp + "> is not supported!\nMost likely you forgot to set the type attribute or accidentally created the node by a typo in an edge definition."); return;}
      }  

    }
   
    // Crawl vertices and serialise their data objects for upload
    BOOST_FOREACH( vertex_t v, vertices(gUp) ) {
     
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

  void CarpeDM::baseUploadOnDownload() {
    gUp.clear();
    //init up graph from down
    download();
    atUp = atDown;
    // for some reason, copy_graph does not copy the name
    //boost::set_property(gTmp, boost::graph_name, boost::get_property(g, boost::graph_name));
    copy_graph(gDown, gUp);
    //now, we need to change the buffer pointers in the copied nodes, as they still point to buffers in upload allocation table

    BOOST_FOREACH( vertex_t v, vertices(gUp) ) {
      auto* x = (AllocMeta*)&(*atUp.lookupVertex(v));
      gUp[v].np->setB(x->b);
    } 
  }

  int CarpeDM::add(const std::string& fn) {

    Graph gTmp;
    vertex_map_t vertexMap, duplicates;
    
    baseUploadOnDownload(); 

    parseDot(fn, gTmp);
    if ((boost::get_property(gTmp, boost::graph_name)).find("!CMD") != std::string::npos) {throw std::runtime_error("Cannot treat a series of commands as a schedule"); return -1;}
        typedef std::map<vertex_t, vertex_t> vertex_map_t;
    
    generateBlockMeta(gTmp); //auto generate desired Block Meta Nodes

    //find and list all duplicates i.e. docking points between trees
 
    //probably a more elegant solution out there, but I don't have the time for trial and error on boost property maps.
    BOOST_FOREACH( vertex_t v, vertices(gUp) ) { 
      BOOST_FOREACH( vertex_t w, vertices(gTmp) ) { 
        if (gTmp[w].name == gUp[v].name) {sLog << gTmp[w].name << " gTmp " << w << " <-> " << gUp[v].name << " gUp " << v << std::endl; duplicates[v] = w;} 
      }  
    }

    //merge graphs (will lead to disjunct trees with duplicates at overlaps), but keep the mapping for vertex merge
    boost::associative_property_map<vertex_map_t> vertexMapWrapper(vertexMap);
    copy_graph(gTmp, gUp, boost::orig_to_copy(vertexMapWrapper));
    for(auto& it : vertexMap ) {sLog <<  "gTmp " << gTmp[it.first].name << " @ " << it.first << " gUp " << it.second << std::endl; }
    //merge duplicate nodes
    for(auto& it : duplicates ) { 
      sLog <<  it.first << " <- " << it.second << "(" << vertexMap[it.second] << ")" << std::endl; 
      mergeUploadDuplicates(it.first, vertexMap[it.second]); 
    }

    //now remove duplicates
    for(auto& it : duplicates ) { 
      boost::clear_vertex(vertexMap[it.second], gUp); 
      boost::remove_vertex(vertexMap[it.second], gUp);
      //remove_vertex() changes the vertex vector, as it must stay contignuous. descriptors higher than he removed one therefore need to be decremented by 1
      for( auto& updateIt : vertexMap) {if (updateIt.second > it.second) updateIt.second--; }
    }

    writeUpDot("inspect.dot", false);


    prepareUpload();
    atUp.updateBmps();
    writeUpDot("upload.dot", false);

    return upload();

  }


int CarpeDM::overwrite(const std::string& fn) {
  Graph gTmp; 
  parseDot(fn, gTmp);
  if ((boost::get_property(gTmp, boost::graph_name)).find("!CMD") != std::string::npos) {throw std::runtime_error("Cannot treat a series of commands as a schedule"); return -1;}
  prepareUpload();
  atUp.updateBmps();

  return upload();

}


  int CarpeDM::keep(const std::string& fn) {
    prepareKeep(fn);
    download();
    return execKeep();
  }


  void CarpeDM::prepareKeep(const std::string& fn) {
    Graph gTmp;
    atUp.clear();
    gUp.clear();
    parseDot(fn, gTmp);
    if ((boost::get_property(gTmp, boost::graph_name)).find("!CMD") != std::string::npos) {throw std::runtime_error("Cannot treat a series of commands as a schedule"); return;}
    prepareUpload(); 
  }  

  //removes all nodes NOT in input file
  int CarpeDM::execKeep() {
    uint32_t hash;
    std::set<uint32_t> vHashes;

    //Get all downloaded Hashes
    for (auto& it : atDown.getTable().get<Hash>()) vHashes.insert(it.hash);

    
      
    //Strike all also present in the input file
    BOOST_FOREACH( vertex_t v, vertices(gUp) ) {   
      hash = hm.lookup(gUp[v].name).get();
      if (vHashes.count(hash) > 0) vHashes.erase(hash);
    }  

    //remove all nodes NOT in input file from download allocation table
    for (auto& itHash : vHashes) {
      if (!(atDown.isOk(atDown.lookupHash(itHash)))) { if(verbose) {sLog << "Node " << hm.lookup(itHash).get() << " was not present on DM" << std::endl;}}
      if (!(atDown.deallocate(itHash))) { if(verbose) {sLog << "Node " << hm.lookup(itHash).get() << " could not be removed" << std::endl;}}  
    }
    atDown.updateBmps();
    //show("After Removal", "", DOWNLOAD, false );
    atUp.syncToAtBmps(atDown); //use the bmps of the changed download allocation table for upload
    atUp.unstageAll(); // node nodes will be uploaded, only the bmp
    //because gUp Graph is empty, upload will only contain the reduced upload bmps, effectively deleting nodes
    return upload();

  }

  //removes all nodes in input file
  int CarpeDM::remove(const std::string& fn) {
    Graph gTmp;
    atUp.clear();
    gUp.clear();
    
    parseDot(fn, gTmp);
    if ((boost::get_property(gTmp, boost::graph_name)).find("!CMD") != std::string::npos) {throw std::runtime_error("Cannot treat a series of commands as a schedule"); return -1;}
    prepareUpload(); 
    download();

    uint32_t hash;

    //remove all nodes in input file from download allocation table
    try {
      
      BOOST_FOREACH( vertex_t v, vertices(gUp) ) { 
        hash = hm.lookup(gUp[v].name).get();
        if (!(atDown.isOk(atDown.lookupHash(hash)))) { if(verbose) {sLog << "Node " << hm.lookup(hash).get() << " was not present on DM" << std::endl;}}
        if (!(atDown.deallocate(hash))) { if(verbose) {sLog << "Node " << gUp[v].name << "(0x" << std::hex << hash << " could not be removed" << std::endl;}}
      }  
    }  catch (...) {
      //TODO report hash collision and show which names are responsible
      throw;
    }
    atDown.updateBmps();
    //show("After Removal", "", DOWNLOAD, false );

    atUp.syncToAtBmps(atDown); //use the bmps of the changed download allocation table for upload
    atUp.unstageAll(); // node nodes will be uploaded, only the bmp
    

    //because gUp Graph is empty, upload will only contain the reduced upload bmps, effectively deleting nodes
    return upload();

  }

  int CarpeDM::clear() {
    gUp.clear(); //Necessary?
    atUp.clear();
    atUp.clearMemories();
    return upload();

  }