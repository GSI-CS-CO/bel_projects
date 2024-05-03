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
#include "carpeDMimpl.h"
#include "visitoruploadcrawler.h"
#include "visitordownloadcrawler.h"
#include "dotstr.h"
#include "idformat.h"

#include "log.h"

namespace dnt = DotStr::Node::TypeVal;


  //Generate download Bmp addresses. For downloads, this has to be two pass: get bmps first, then use them to get the node locations to read
  vEbrds CarpeDM::CarpeDMimpl::gatherDownloadBmpVector() {
    //sLog << "Starting download bmp address vectors" << std::endl;
    AllocTable& at = atDown;
    vEbrds er;

     //add all Bmp addresses to return vector
    for(unsigned int i = 0; i < at.getMemories().size(); i++) {
      //generate addresses of Bmp's address range
      for (uint32_t adr = at.adrConv(AdrType::MGMT, AdrType::EXT,i, at.getMemories()[i].bmpOffs); adr < at.adrConv(AdrType::MGMT, AdrType::EXT,i, at.getMemories()[i].startOffs); adr += _32b_SIZE_) {
        er.va.push_back(adr);
        er.vcs.push_back(adr == at.adrConv(AdrType::MGMT, AdrType::EXT,i, at.getMemories()[i].bmpOffs));
      }
    }
    return er;
  }



  vEbrds CarpeDM::CarpeDMimpl::gatherDownloadDataVector() {
    //sLog << "Starting download bmp data vectors" << std::endl;
    AllocTable& at = atDown;
    vEbrds er;
    //go through Memories
    for(unsigned int i = 0; i < at.getMemories().size(); i++) {
      //go through a memory's bmp bits, starting at number of nodes the bmp itself needs (bmpSize / memblocksize). Otherwise, we'd needlessly download the bmp again

      //sLog << "Bmp" << i << ":" << std::endl;
      for(unsigned int bitIdx = at.getMemories()[i].bmpSize / _MEM_BLOCK_SIZE; bitIdx < at.getMemories()[i].bmpBits; bitIdx++) {
        //if the bit says the node is used, we add the node to read addresses
        //sLog << "Bit Idx " << bitIdx << " valid " << at.getMemories()[i].getBmpBit(bitIdx) << " na 0x" << std::hex << at.getMemories()[i].sharedOffs + bitIdx * _MEM_BLOCK_SIZE << std::endl;
        if (at.getMemories()[i].getBmpBit(bitIdx)) {
          uint32_t nodeAdr = at.getMemories()[i].bmpOffs + bitIdx * _MEM_BLOCK_SIZE;
           //generate addresses of node's address range
          for (uint32_t adr = at.adrConv(AdrType::MGMT, AdrType::EXT,i, nodeAdr); adr < at.adrConv(AdrType::MGMT, AdrType::EXT,i, nodeAdr + _MEM_BLOCK_SIZE); adr += _32b_SIZE_ ) {
            er.va.push_back(adr);
            er.vcs.push_back(adr == at.adrConv(AdrType::MGMT, AdrType::EXT,i, nodeAdr));
          }
        }
      }
    }
    return er;
  }

   void CarpeDM::CarpeDMimpl::parseDownloadMgmt(const vBuf& downloadData) {
    AllocTable& at = atDown;


    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //create MgmtTable
    //sLog << std::dec << "dl size " << downloadData.size() << std::endl;
    uint32_t found = 0;
    uint32_t nodeCnt = 0;
    //go through Memories
    for(unsigned int i = 0; i < at.getMemories().size(); i++) {
      //go through Bmp
      for(unsigned int bitIdx = at.getMemories()[i].bmpSize / _MEM_BLOCK_SIZE; bitIdx < at.getMemories()[i].bmpBits; bitIdx++) {
        if (at.getMemories()[i].getBmpBit(bitIdx)) {
          uint32_t    localAdr  = nodeCnt * _MEM_BLOCK_SIZE;
          nodeCnt++;
          uint32_t    flags     = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&downloadData[localAdr + NODE_FLAGS]);
          uint32_t    type      = (flags >> NFLG_TYPE_POS) & NFLG_TYPE_MSK;
          if (type != NODE_TYPE_MGMT) continue; // skip all non mgmt nodes
          uint8_t     cpu       = i;
          uint32_t    adr       = at.getMemories()[i].bmpOffs + bitIdx * _MEM_BLOCK_SIZE;

          //we need to conform to the allocation rules and register our management nodes
          if (!(at.insertMgmt(cpu, adr, (uint8_t*)&downloadData[localAdr]))) {throw std::runtime_error( std::string("Address collision when adding mgmt node at "));};
          found++;
          //at.getMemories()[i].clrBmpBit(bitIdx); // clear bit so parseDownloadData doesnt have to deal with this node again.
          //IMPORTANT: this saves a little bit of work, but also means the bmp is out of sync until we leave this function
        }
      }
    }

    if(verbose) sLog << "Mgmt found " << std::dec << found << " data chunks. Total " << nodeCnt << " nodes scanned. Trying to recover GroupTable ..." << std::endl;

    // recover container
    vBuf aux = at.recoverMgmt();
    vBuf tmpMgmtRecovery = decompress(aux);

    if(verbose) sLog << "Bytes expected: " << std::dec << at.getMgmtTotalSize() << ", recovered: " << std::dec << aux.size() << std::endl << std::endl;
    if (tmpMgmtRecovery.size()) {
      // Rebuild Grouptable

      //FIXME: Code quality is horrible in this, why the hell did my younger self write such dross? Clean. this. up.

      auto strBegin = tmpMgmtRecovery.begin();

      GroupTable gtTmp;
      std::string tmpStrGrouptab = std::string(strBegin, strBegin + at.getMgmtGrpSize());
      if (tmpStrGrouptab.size()) { gtTmp.load(tmpStrGrouptab); gt = gtTmp;}
      // Rebuild HashMap from Grouptable
      hm.clear();
      for(auto& it : gt.getTable()) {
        hm.add(it.node);
      }
      strBegin += at.getMgmtGrpSize();

      // Rebuild Covenanttable

      CovenantTable ctTmp;
      std::string tmpStrCovtab = std::string(strBegin, strBegin + at.getMgmtCovSize());
      if (tmpStrCovtab.size()) {ctTmp.load(tmpStrCovtab); ct = ctTmp;}
      strBegin += at.getMgmtCovSize();

      // Rebuild Reftable
      GlobalRefTable rtTmp;
      std::string tmpStrReftab = std::string(strBegin, strBegin + at.getMgmtRefSize());

      if (tmpStrReftab.size()) {rtTmp.load(tmpStrReftab); rt = rtTmp; }  

      


      //rt.debug(sLog);
    } else {
      if(verbose) sLog << "Management recovery returned empty, this happens when accessing a virgin DM FW memory. Skip Grouptable and Covenant Table creation" << std::endl;
    }
    // clean up - remove now obsolete management data (we need a fresh set anyway once upload data is set)
    at.deallocateAllMgmt();
    // Tables and Pools match Bitmap again. As far as parseDownloadData is concerned, we were never here.

  }



  void CarpeDM::CarpeDMimpl::parseDownloadData(const vBuf& downloadData) {
    Graph& g = gDown;
    AllocTable& at = atDown;
    std::stringstream stream;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //create AllocTable and Vertices
    //sLog << std::dec << "dl size " << downloadData.size() << std::endl;
    if(verbose) sLog << "Analysing downloaded graph binary " << std::dec << downloadData.size() << " bytes" << std::endl;
    uint32_t nodeCnt = 0;
    
    //go through Memories
    for(unsigned int i = 0; i < at.getMemories().size(); i++) {
      //go through Bmp
      for(unsigned int bitIdx = at.getMemories()[i].bmpSize / _MEM_BLOCK_SIZE; bitIdx < at.getMemories()[i].bmpBits; bitIdx++) {
        if (at.getMemories()[i].getBmpBit(bitIdx)) {

          uint32_t    localAdr  = nodeCnt * _MEM_BLOCK_SIZE; nodeCnt++;
          uint32_t    adr       = at.getMemories()[i].bmpOffs + bitIdx * _MEM_BLOCK_SIZE;
          //sLog << "THE adr : 0x" << std::hex << adr << std::endl;
          uint32_t    hash      = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&downloadData[localAdr + NODE_HASH]);
          uint32_t    flags     = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&downloadData[localAdr + NODE_FLAGS]); //FIXME what about future requests to hashmap if we improvised the name from hash? those will fail ...
          uint32_t    type      = (flags >> NFLG_TYPE_POS) & NFLG_TYPE_MSK;
          uint8_t     cpu       = i;


          //TODO Can't management and data be handled in parallel?... unsure
          // IMPORTANT: skip all mgmt nodes
          if (type == NODE_TYPE_MGMT) {continue; }


          stream.str(""); stream.clear();
          stream << "0x" << std::setfill ('0') << std::setw(sizeof(uint32_t)*2) << std::hex << hash;
          std::string name      = hm.contains(hash) ? hm.lookup(hash) : DotStr::Misc::sHashType + stream.str();
          auto xPat  = gt.getTable().get<Groups::Node>().equal_range(name);
          std::string pattern   = (xPat.first != xPat.second ? xPat.first->pattern : DotStr::Misc::sUndefined);
          auto xBp  = gt.getTable().get<Groups::Node>().equal_range(name);
          std::string beamproc  = (xBp.first != xBp.second ? xPat.first->beamproc : DotStr::Misc::sUndefined);

          //Vertex needs flags as a std::string. Convert to hex
          stream.str(""); stream.clear();
          stream << "0x" << std::setfill ('0') << std::setw(sizeof(uint32_t)*2) << std::hex << flags;
          std::string tmp(stream.str());

          //Add Vertex
          
          

          vertex_t v = boost::add_vertex(myVertex(name, pattern, beamproc, std::to_string(cpu), hash, nullptr, "", tmp), g);
          //FIXME workaround for groupstable updates from download. not  nice ...
          //sErr << std::endl << "DEBUG_After_Suspicious_Code" << std::endl;  
          //BOOST_FOREACH( vertex_t v, vertices(g) ) { if(g[v].np != nullptr) g[v].np->show(); }

          g[v].bpEntry  = std::to_string((bool)(flags & NFLG_BP_ENTRY_LM32_SMSK));
          g[v].bpExit   = std::to_string((bool)(flags & NFLG_BP_EXIT_LM32_SMSK));
          g[v].patEntry = std::to_string((bool)(flags & NFLG_PAT_ENTRY_LM32_SMSK));
          g[v].patExit  = std::to_string((bool)(flags & NFLG_PAT_EXIT_LM32_SMSK));

          //std::cout << "atdown cpu " << (int)cpu << " Adr: 0x" << std::hex << adr <<  " Hash 0x" << hash << std::endl;
          //Add allocTable Entry
          //vBuf test(downloadData.begin() + localAdr, downloadData.begin() + localAdr + _MEM_BLOCK_SIZE);
          //vHexDump("TEST ****", test);


          if (!(at.insert(cpu, adr, hash, v, false, false))) {
            sLog << "Offending Node at: CPU " << (int)cpu << " 0x" << std::hex << adr << std::endl;
            hexDump("Dump", (char*)&downloadData[localAdr], _MEM_BLOCK_SIZE );
            throw std::runtime_error( std::string("Hash or address collision when adding node ") + name);
          };

          // Create node object for Vertex
          auto src = downloadData.begin() + localAdr;
          auto  it = at.lookupAdr(cpu, adr);
          auto*  x = (AllocMeta*)&(*it);

          std::copy(src, src + _MEM_BLOCK_SIZE, (uint8_t*)&(x->b[0]));

      

          switch(type) {
            case NODE_TYPE_TMSG         : g[v].np = (node_ptr) new  TimingMsg(g[v].name, g[v].patName, g[v].bpName, x->hash, x->cpu, flags); g[v].type = dnt::sTMsg;       g[v].np->deserialise((uint8_t*)x->b); break;
            case NODE_TYPE_CNOOP        : g[v].np = (node_ptr) new       Noop(g[v].name, g[v].patName, g[v].bpName, x->hash, x->cpu, flags); g[v].type = dnt::sCmdNoop;    g[v].np->deserialise((uint8_t*)x->b); break;
            case NODE_TYPE_CFLOW        : g[v].np = (node_ptr) new       Flow(g[v].name, g[v].patName, g[v].bpName, x->hash, x->cpu, flags); g[v].type = dnt::sCmdFlow;    g[v].np->deserialise((uint8_t*)x->b); break;
            case NODE_TYPE_CSWITCH      : g[v].np = (node_ptr) new     Switch(g[v].name, g[v].patName, g[v].bpName, x->hash, x->cpu, flags); g[v].type = dnt::sSwitch;     g[v].np->deserialise((uint8_t*)x->b); break;
            case NODE_TYPE_ORIGIN       : g[v].np = (node_ptr) new     Origin(g[v].name, g[v].patName, g[v].bpName, x->hash, x->cpu, flags); g[v].type = dnt::sOrigin;     g[v].np->deserialise((uint8_t*)x->b); break;
            case NODE_TYPE_STARTTHREAD  : g[v].np = (node_ptr) new StartThread(g[v].name, g[v].patName, g[v].bpName, x->hash, x->cpu, flags); g[v].type = dnt::sStartThread; g[v].np->deserialise((uint8_t*)x->b); break;
            case NODE_TYPE_CFLUSH       : g[v].np = (node_ptr) new      Flush(g[v].name, g[v].patName, g[v].bpName, x->hash, x->cpu, flags); g[v].type = dnt::sCmdFlush;   g[v].np->deserialise((uint8_t*)x->b); break;
            case NODE_TYPE_CWAIT        : g[v].np = (node_ptr) new       Wait(g[v].name, g[v].patName, g[v].bpName, x->hash, x->cpu, flags); g[v].type = dnt::sCmdWait;    g[v].np->deserialise((uint8_t*)x->b); break;
            case NODE_TYPE_BLOCK_FIXED  : g[v].np = (node_ptr) new BlockFixed(g[v].name, g[v].patName, g[v].bpName, x->hash, x->cpu, flags); g[v].type = dnt::sBlockFixed; g[v].np->deserialise((uint8_t*)x->b); break;
            case NODE_TYPE_BLOCK_ALIGN  : g[v].np = (node_ptr) new BlockAlign(g[v].name, g[v].patName, g[v].bpName, x->hash, x->cpu, flags); g[v].type = dnt::sBlockAlign; g[v].np->deserialise((uint8_t*)x->b); break;
            case NODE_TYPE_QUEUE        : g[v].np = (node_ptr) new   CmdQMeta(g[v].name, g[v].patName, g[v].bpName, x->hash, x->cpu, flags); g[v].type = dnt::sQInfo;      g[v].np->deserialise((uint8_t*)x->b); break;
            case NODE_TYPE_ALTDST       : g[v].np = (node_ptr) new   DestList(g[v].name, g[v].patName, g[v].bpName, x->hash, x->cpu, flags); g[v].type = dnt::sDstList;    g[v].np->deserialise((uint8_t*)x->b); break;
            case NODE_TYPE_QBUF         : g[v].np = (node_ptr) new CmdQBuffer(g[v].name, g[v].patName, g[v].bpName, x->hash, x->cpu, flags); g[v].type = dnt::sQBuf; break;
            //   NODE_TYPE_GLOBAL can never occur here, similar to management nodes. Don't list it. 
            case NODE_TYPE_UNKNOWN      : sErr << "not yet implemented " << g[v].type << std::endl; break;
            default                     : sErr << "Node type 0x" << std::hex << type << " not supported! " << std::endl;
          }
          

          
        }
        }
    }
    
    


    if(verbose) sLog << "Node creation done. Creating Edges" << std::endl;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // create edges
    //Three-pass for edges. First, iterate all non meta-types to establish block -> dstList parenthood
    for(auto& it : at.getTable().get<Hash>()) {
      // handled by visitor
      if (g[it.v].np == nullptr) {throw std::runtime_error( std::string("Node ") + g[it.v].name + std::string("not initialised")); return;

      } else {

        if  (!(g[it.v].np->isMeta())) {
          g[it.v].np->accept(VisitorDownloadCrawler(g, it.v, at, ct, sLog, sErr));
        }  
      }
    }
    //second, iterate all meta-types
    for(auto& it : at.getTable().get<Hash>()) {
      // handled by visitor
      if (g[it.v].np == nullptr) {throw std::runtime_error( std::string("Node ") + g[it.v].name + std::string("not initialised")); return;
      } else {
        if  (g[it.v].np->isMeta()) g[it.v].np->accept(VisitorDownloadCrawler(g, it.v, at, ct, sLog, sErr));
      }
    }

    if(verbose) sLog << "Done. Graph generation complete" << std::endl;


  }

  unsigned CarpeDM::CarpeDMimpl::recreateGlobalRefs() {
    Graph& g = gDown;
    AllocTable& at = atDown;
    unsigned count = 0;

    //we already recreated the reftable from the mamagement nodes we downloaded, Now, for each entry in rt, we create the global node and insert into atDown.
    refIt rtBegin, rtEnd;
    std::tie(rtBegin, rtEnd) = at.rt->getMapRange();
    
    for (auto it = rtBegin; it != rtEnd; it++) {

      uint32_t tmpAdr, hash;
      std::tie(tmpAdr, hash) = *it;
      //We do not know the CPU yet. Analyse the address to get it.
      uint8_t cpu;
      AdrType adrType;
      std::tie(cpu, adrType) = at.adrClassification(tmpAdr);
      //TODO this does not cover all possible address types
      //If it is not an adress inside a CPUs shared space, throw an ex for now
      //sLog << "Global Node at: CPU " << (int)cpu << " 0x" << std::hex << tmpAdr << std::endl;
      if (adrType != AdrType::INT) {throw std::runtime_error( std::string("Error. GlobalReftable entry contains an address not part of the AdrType::INT"));}
      
      uint32_t adr = at.adrConv(adrType, AdrType::MGMT, cpu, tmpAdr);
      /*
      sLog << "Global Node converted Adr at: CPU " << (int)cpu << " 0x" << std::hex << adr << std::endl;
      at.rl->showMemLocMap();
      at.rt->debug(sLog);
      */
      std::string section = at.rl->getLocName(adr);

      //sLog << "RLSearch: section " << section << std::endl;
      //throw std::runtime_error( std::string("HALT after lookup"));
      //To create the node, we need a few things from the group table. The hashmap has already been recreated.
      //This is the same as for the normal nodes, but since globals are not part of the occupation bitmaps, we need to do it here
      std::stringstream stream;
      stream.str(""); stream.clear();
      stream << "0x" << std::setfill ('0') << std::setw(sizeof(uint32_t)*2) << std::hex << hash;
      std::string name  = hm.contains(hash) ? hm.lookup(hash) : DotStr::Misc::sHashType + stream.str();
      auto xPat  = gt.getTable().get<Groups::Node>().equal_range(name);
      std::string pattern   = (xPat.first != xPat.second ? xPat.first->pattern : DotStr::Misc::sUndefined);
      auto xBp  = gt.getTable().get<Groups::Node>().equal_range(name);
      std::string beamproc  = (xBp.first != xBp.second ? xPat.first->beamproc : DotStr::Misc::sUndefined);

      //sLog << "Found the following from hm and gt: name " << name << " pattern " << pattern << " beamprocs " << beamproc << std::endl;

      //create node  
      vertex_t v = boost::add_vertex(myVertex(name, pattern, beamproc, std::to_string(cpu), hash, nullptr, "", DotStr::Misc::sZero), g);
      g[v].type     = dnt::sGlobal;
      g[v].section  = section;    
      g[v].bpEntry  = std::to_string(false);
      g[v].bpExit   = std::to_string(false);
      g[v].patEntry = std::to_string(false);
      g[v].patExit  = std::to_string(false);

      //allocate node
       if (!(at.insert(cpu, adr, hash, v, false, true))) {
        sLog << "Offending Global Node at: CPU " << (int)cpu << " 0x" << std::hex << adr << std::endl;
        throw std::runtime_error( std::string("Hash or address collision when adding node ") + name);
      };
      
      //add object
      g[v].np = (node_ptr) new Global(g[v].name, g[v].patName, g[v].bpName, hash, cpu, 0, g[v].section);

      //sLog << "Added Global Node " << name << " @ CPU #" << (int)cpu << " 0x" << std::hex << adr << std::endl;
      count++;
    }

    return count;
  }



  void CarpeDM::CarpeDMimpl::readMgmtLLMeta() {
    vEbrds er;
    vBuf vDl;
    uint32_t modAdrBase = atDown.getMemories()[0].extBaseAdr + atDown.getMemories()[0].sharedOffs + SHCTL_META;
    er.va.push_back(modAdrBase + T_META_START_PTR);
    er.va.push_back(modAdrBase + T_META_CON_SIZE);
    er.va.push_back(modAdrBase + T_META_GRPTAB_SIZE);
    er.va.push_back(modAdrBase + T_META_COVTAB_SIZE);
    er.va.push_back(modAdrBase + T_META_REFTAB_SIZE);
    er.vcs += leadingOne(er.va.size());

    vDl = ebd.readCycle(er.va, er.vcs);
    atDown.setMgmtLLstartAdr(writeBeBytesToLeNumber<uint32_t>((uint8_t*)&vDl[T_META_START_PTR]));

    uint32_t grp, cov, ref;
    atDown.setMgmtTotalSize(writeBeBytesToLeNumber<uint32_t>((uint8_t*)&vDl[T_META_CON_SIZE]));
    grp = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&vDl[T_META_GRPTAB_SIZE]);
    cov = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&vDl[T_META_COVTAB_SIZE]);
    ref = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&vDl[T_META_REFTAB_SIZE]);
    atDown.setMgmtLLSizes(grp, cov, ref);



  }

  int CarpeDM::CarpeDMimpl::download() {
    vBuf vDlBmpD, vDlD;
    //FIXME get mgmt linked list meta data



    vEbrds erBmp = gatherDownloadBmpVector();
    vEbrds erData;


    atDown.clear();
    atDown.clearMemories();
    gDown.clear();
    //get all BMPs so we know which nodes to download
    if(verbose) sLog << "Downloading ...";
    vDlBmpD = ebd.readCycle(erBmp.va, erBmp.vcs);
    /*
    sLog << "Tried to read " << std::dec << erBmp.va.size() << " bmp addresses " << std::endl;
    sLog << "Got back " << std::dec << vDlBmpD.size() << " bmp bytes " << std::endl;
    hexDump("bmps", vDlBmpD);
    */
    atDown.setBmps( vDlBmpD );
    erData = gatherDownloadDataVector();
    vDlD   = ebd.readCycle(erData.va, erData.vcs);
    /*
    sLog << "Tried to read " << erData.va.size() << " data addresses " << std::endl;
    sLog << "Got back " << vDlD.size() << " data bytes " << std::endl;
    */
    // read out current time for upload mod time (seconds, but probably better to use same format as DM FW. Convert to ns)
    updateModTime();

    if(verbose) sLog << "Done." << std::endl << "Calling parser for Mgmt Meta" << std::endl ;
    readMgmtLLMeta(); // we have to do this before parsing
      
    

    if(verbose) sLog << "returned." << std::endl << "Calling parser for Mgmt Data" << std::endl ;
    parseDownloadMgmt(vDlD);
    if(verbose) sLog << "returned." << std::endl << "Calling parser for Download Meta" << std::endl ;

    recreateGlobalRefs();

    parseDownloadData(vDlD);
    if(verbose) sLog << "returned." << std::endl;

    freshDownload = true;
    if(optimisedS2R) updateCovenants();

    //gt.debug(sLog);

    //ct.debug(sLog);

    return vDlD.size();
  }


