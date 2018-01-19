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
#include "idformat.h"

namespace dnt = DotStr::Node::TypeVal;


  //Generate download Bmp addresses. For downloads, this has to be two pass: get bmps first, then use them to get the node locations to read 
  const vAdr CarpeDM::getDownloadBMPAdrs() {
    AllocTable& at = atDown;
    vAdr ret;

     //add all Bmp addresses to return vector
    for(unsigned int i = 0; i < at.getMemories().size(); i++) {
      //generate addresses of Bmp's address range
      for (uint32_t adr = at.adrConv(AdrType::MGMT, AdrType::EXT,i, at.getMemories()[i].sharedOffs); adr < at.adrConv(AdrType::MGMT, AdrType::EXT,i, at.getMemories()[i].startOffs); adr += _32b_SIZE_) ret.push_back(adr);
    }
    return ret;
  }

    

  const vAdr CarpeDM::getDownloadAdrs() {
    AllocTable& at = atDown;
    vAdr ret;
    //go through Memories
    for(unsigned int i = 0; i < at.getMemories().size(); i++) {
      //go through a memory's bmp bits, starting at number of nodes the bmp itself needs (bmpSize / memblocksize). Otherwise, we'd needlessly download the bmp again
      for(unsigned int bitIdx = at.getMemories()[i].bmpSize / _MEM_BLOCK_SIZE; bitIdx < at.getMemories()[i].bmpBits; bitIdx++) {
        //if the bit says the node is used, we add the node to read addresses
        //sLog << "Bit Idx " << bitIdx << " valid " << at.getMemories()[i].getBmpBit(bitIdx) << " na 0x" << std::hex << at.getMemories()[i].sharedOffs + bitIdx * _MEM_BLOCK_SIZE << std::endl;
        if (at.getMemories()[i].getBmpBit(bitIdx)) {
          uint32_t nodeAdr = at.getMemories()[i].sharedOffs + bitIdx * _MEM_BLOCK_SIZE;
           //generate addresses of node's address range
          for (uint32_t adr = at.adrConv(AdrType::MGMT, AdrType::EXT,i, nodeAdr); adr < at.adrConv(AdrType::MGMT, AdrType::EXT,i, nodeAdr + _MEM_BLOCK_SIZE); adr += _32b_SIZE_ ) {ret.push_back(adr);}
        }
      }      
    }
    return ret;
  }  
    



  void CarpeDM::parseDownloadData(vBuf downloadData) {
    Graph& g = gDown;
    AllocTable& at = atDown;
    std::stringstream stream;

    

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //create AllocTable and Vertices
    //sLog << std::dec << "dl size " << downloadData.size() << std::endl;

    uint32_t nodeCnt = 0;
    //go through Memories
    for(unsigned int i = 0; i < at.getMemories().size(); i++) {
      //go through Bmp
      for(unsigned int bitIdx = at.getMemories()[i].bmpSize / _MEM_BLOCK_SIZE; bitIdx < at.getMemories()[i].bmpBits; bitIdx++) {
        if (at.getMemories()[i].getBmpBit(bitIdx)) {

          uint32_t    localAdr  = nodeCnt * _MEM_BLOCK_SIZE; nodeCnt++; 
          uint32_t    adr       = at.getMemories()[i].sharedOffs + bitIdx * _MEM_BLOCK_SIZE;
          uint32_t    hash      = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&downloadData[localAdr + NODE_HASH]);
          //sLog << std::dec << "Offset " << localAdr + NODE_HASH << std::endl;

          stream.str(""); stream.clear();
          stream << "0x" << std::setfill ('0') << std::setw(sizeof(uint32_t)*2) << std::hex << hash;
          std::string name      = hm.lookup(hash) ? hm.lookup(hash).get() : "#" + stream.str();
          std::string pattern   = gt.lookupOrCreateNode(name)->pattern;
          std::string beamproc  = gt.lookupOrCreateNode(name)->beamproc;
          uint32_t    flags     = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&downloadData[localAdr + NODE_FLAGS]); //FIXME what about future requests to hashmap if we improvised the name from hash? those will fail ...
          uint32_t    type      = (flags >> NFLG_TYPE_POS) & NFLG_TYPE_MSK;
          uint8_t     cpu       = i;
          
          //Vertex needs flags as a std::string. Convert to hex
          stream.str(""); stream.clear();
          stream << "0x" << std::setfill ('0') << std::setw(sizeof(uint32_t)*2) << std::hex << flags;
          std::string tmp(stream.str());

          //Add Vertex
          
          vertex_t v        = boost::add_vertex(myVertex(name, pattern, beamproc, std::to_string(cpu), hash, nullptr, "", tmp), g);
          //FIXME workaround for groupstable updates from download. not  nice ...
          
          g[v].bpEntry  = std::to_string((bool)(flags & NFLG_BP_ENTRY_LM32_SMSK));
          g[v].bpExit   = std::to_string((bool)(flags & NFLG_BP_EXIT_LM32_SMSK));
          g[v].patEntry = std::to_string((bool)(flags &  NFLG_PAT_ENTRY_LM32_SMSK));
          g[v].patExit  = std::to_string((bool)(flags &  NFLG_PAT_EXIT_LM32_SMSK));
          
          //std::cout << "atdown cpu " << (int)cpu << " Adr: 0x" << std::hex << adr <<  " Hash 0x" << hash << std::endl;
          //Add allocTable Entry
          //vBuf test(downloadData.begin() + localAdr, downloadData.begin() + localAdr + _MEM_BLOCK_SIZE);
          //vHexDump("TEST ****", test);

          if (!(at.insert(cpu, adr, hash, v, false))) {throw std::runtime_error( std::string("Hash or address collision when adding node ") + name); return;};

          // Create node object for Vertex
          auto src = downloadData.begin() + localAdr;

          auto it  = at.lookupAdr(cpu, adr);
          if (!(at.isOk(it))) {throw std::runtime_error( std::string("Node at (dec) ") + std::to_string(adr) + std::string(", hash (dec) ") + std::to_string(hash) + std::string("not found. This is weird")); return;}
          
          auto* x = (AllocMeta*)&(*it);

          std::copy(src, src + _MEM_BLOCK_SIZE, (uint8_t*)&(x->b[0]));
        
          switch(type) {
            case NODE_TYPE_TMSG         : g[v].np = (node_ptr) new  TimingMsg(g[v].name, g[v].patName, g[v].bpName, x->hash, x->cpu, x->b, flags); g[v].type = dnt::sTMsg;       g[v].np->deserialise(); break;
            case NODE_TYPE_CNOOP        : g[v].np = (node_ptr) new       Noop(g[v].name, g[v].patName, g[v].bpName, x->hash, x->cpu, x->b, flags); g[v].type = dnt::sCmdNoop;    g[v].np->deserialise(); break;
            case NODE_TYPE_CFLOW        : g[v].np = (node_ptr) new       Flow(g[v].name, g[v].patName, g[v].bpName, x->hash, x->cpu, x->b, flags); g[v].type = dnt::sCmdFlow;    g[v].np->deserialise(); break;
            case NODE_TYPE_CFLUSH       : g[v].np = (node_ptr) new      Flush(g[v].name, g[v].patName, g[v].bpName, x->hash, x->cpu, x->b, flags); g[v].type = dnt::sCmdFlush;   g[v].np->deserialise(); break;
            case NODE_TYPE_CWAIT        : g[v].np = (node_ptr) new       Wait(g[v].name, g[v].patName, g[v].bpName, x->hash, x->cpu, x->b, flags); g[v].type = dnt::sCmdWait;    g[v].np->deserialise(); break;
            case NODE_TYPE_BLOCK_FIXED  : g[v].np = (node_ptr) new BlockFixed(g[v].name, g[v].patName, g[v].bpName, x->hash, x->cpu, x->b, flags); g[v].type = dnt::sBlockFixed; g[v].np->deserialise(); break;
            case NODE_TYPE_BLOCK_ALIGN  : g[v].np = (node_ptr) new BlockAlign(g[v].name, g[v].patName, g[v].bpName, x->hash, x->cpu, x->b, flags); g[v].type = dnt::sBlockAlign; g[v].np->deserialise(); break;
            case NODE_TYPE_QUEUE        : g[v].np = (node_ptr) new   CmdQMeta(g[v].name, g[v].patName, g[v].bpName, x->hash, x->cpu, x->b, flags); g[v].type = dnt::sQInfo;    g[v].np->deserialise(); break;
            case NODE_TYPE_ALTDST       : g[v].np = (node_ptr) new   DestList(g[v].name, g[v].patName, g[v].bpName, x->hash, x->cpu, x->b, flags); g[v].type = dnt::sDstList;  g[v].np->deserialise(); break;
            case NODE_TYPE_QBUF         : g[v].np = (node_ptr) new CmdQBuffer(g[v].name, g[v].patName, g[v].bpName, x->hash, x->cpu, x->b, flags); g[v].type = dnt::sQBuf; break;
            case NODE_TYPE_UNKNOWN      : sErr << "not yet implemented " << g[v].type << std::endl; break;
            default                     : sErr << "Node type 0x" << std::hex << type << " not supported! " << std::endl;
          }
          
        }
      }
    }

  


    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // create edges

    //Two-pass for edges. First, iterate all non meta-types to establish block -> dstList parenthood
    for(auto& it : at.getTable().get<Hash>()) {
      // handled by visitor
      if (g[it.v].np == nullptr) {throw std::runtime_error( std::string("Node ") + g[it.v].name + std::string("not initialised")); return;

      } else {

        if  (!(g[it.v].np->isMeta())) g[it.v].np->accept(VisitorDownloadCrawler(g, it.v, at, sLog, sErr));
      }  
    }
    //second, iterate all meta-types
    for(auto& it : at.getTable().get<Hash>()) {
      // handled by visitor
      if (g[it.v].np == nullptr) {throw std::runtime_error( std::string("Node ") + g[it.v].name + std::string("not initialised")); return; 
      } else {
        if  (g[it.v].np->isMeta()) g[it.v].np->accept(VisitorDownloadCrawler(g, it.v, at, sLog, sErr));
      }  
    }

    
  }  

    //TODO assign to CPUs/threads


   int CarpeDM::download() {
    
    vAdr vDlBmpA, vlDlA;
    vBuf vDlBmpD, vDlD;

    atDown.clear();
    atDown.clearMemories();
    gDown.clear();
    //get all BMPs so we know which nodes to download
    if(verbose) sLog << "Downloading ...";
    vDlBmpD = ebReadCycle(ebd, getDownloadBMPAdrs());
    atDown.setBmps( vDlBmpD );
    vDlD    = ebReadCycle(ebd, getDownloadAdrs());
    // read out current time for upload mod time (seconds, but probably better to use same format as DM FW. Convert to ns)
    modTime = getDmWrTime() * 1000000000ULL;

    if(verbose) sLog << "Done." << std::endl << "Parsing ...";
    parseDownloadData(vDlD);
    if(verbose) sLog << "Done." << std::endl;
    
    freshDownload = true;

    return vDlD.size();
  }


