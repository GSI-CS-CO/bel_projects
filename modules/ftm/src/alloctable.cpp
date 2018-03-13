#include "alloctable.h"

  

  bool AllocTable::insert(uint8_t cpu, uint32_t adr, uint32_t hash, vertex_t v, bool staged) {
   /*
    std::cout << "Problem: " << std::endl; 
    if (lookupAdr(cpu, adr) != a.end()) std::cout << (int)cpu << " Adr 0x" << std::hex << adr << " exists already" << std::endl;
    if (lookupHash(hash) != a.end()) std::cout << "Hash 0x" << std::hex << hash << " exists already" << std::endl;
    if (lookupVertex(v) != a.end()) std::cout << "V 0x" << std::dec << v << " (hash 0x" << hash << ") exists already" << std::endl;
    */
    vPool[cpu].occupyChunk(adr);
    auto x = a.insert({cpu, adr, hash, v, staged});

    return x.second;
  }

  bool AllocTable::removeByVertex(vertex_t v) {
    auto it = a.get<Vertex>().erase(v);
    if (it != 0) { return true;  }
    else         { return false; }
  }

  bool AllocTable::removeByAdr(uint8_t cpu, uint32_t adr) {

    auto it = a.get<CpuAdr>().find(boost::make_tuple( cpu, adr ));
    a.get<CpuAdr>().erase(it, it);
    /*
    if (it != a.get<Vertex>().end())) { return true;  }
    else         { return false; }
    */
    return true;
  }

  bool AllocTable::removeByHash(uint32_t hash) {
    auto it = a.get<Hash>().erase(hash);
    if (it != 0) { return true;  }
    else         { return false; }

  }

  amI AllocTable::lookupVertex(vertex_t v) const  {
    auto it = a.get<Vertex>().find(v);
    return a.iterator_to( *it );
  }

  amI AllocTable::lookupHash(uint32_t hash) const  {
    auto it = a.get<Hash>().find(hash);
    return a.iterator_to( *it );
  }

  amI AllocTable::lookupAdr(uint8_t cpu, uint32_t adr) const {
    auto it = a.get<CpuAdr>().find(boost::make_tuple( cpu, adr ));
    return a.iterator_to( *it );
    
  }

  //Allocation functions
  int AllocTable::allocate(uint8_t cpu, uint32_t hash, vertex_t v, bool staged) {
    uint32_t chunkAdr;
    //std::cout << "Cpu " << (int)cpu << " mempools " << vPool.size() << std::endl;
    if (cpu >= vPool.size()) {
      //std::cout << "cpu idx out of range" << std::endl;
      return ALLOC_NO_SPACE;}

    if (!(vPool[cpu].acquireChunk(chunkAdr))) return ALLOC_NO_SPACE;
    if (!(insert(cpu, chunkAdr, hash, v, staged)))    return ALLOC_ENTRY_EXISTS;

    return ALLOC_OK;
  }


  int AllocTable::allocateMgmt(vBuf& serialisedContainer) {
    vAdr ret;
    
    //get the size of serialised container and round up to needed mem blocks
    unsigned neededChunks = (serialisedContainer.size() + payloadPerChunk -1) / payloadPerChunk * payloadPerChunk; //integer round up 

    //while we need mem blocks ...
    for(unsigned chunk=0; chunk < neededChunks; chunk++) { 
      //find the cpu with the most free space
      std::pair<uint8_t, size_t> chosen;
      chosen.second = 0;
      for (uint8_t cpuIdx=0; cpuIdx < cpuQty; cpuIdx++) {
        size_t tmpSize = getFreeSpace(cpuIdx); 
        if (chosen.second < tmpSize) {
          chosen.second = tmpSize;  
          chosen.first  = cpuIdx;  
        }
      }

      allocateMgmt(chosen.first);
    }
  }    

  int AllocTable::allocateMgmt(uint8_t cpu) {


    uint32_t chunkAdr;
    //std::cout << "Cpu " << (int)cpu << " mempools " << vPool.size() << std::endl;
    if (cpu >= vPool.size()) {
      //std::cout << "cpu idx out of range" << std::endl;
      return ALLOC_NO_SPACE;}

    if (!(vPool[cpu].acquireChunk(chunkAdr))) return ALLOC_NO_SPACE;
    if (!(insertMgmt(cpu, chunkAdr)))         return ALLOC_ENTRY_EXISTS;

    return ALLOC_OK;
  
  }


  bool AllocTable::insertMgmt(uint8_t cpu, uint32_t adr) {
    vPool[cpu].occupyChunk(adr);
    auto x = m.insert({cpu, adr});

    return x.second;
  }

  //populate buffers of the management table with payload from serialised container and linked list metadata
  void AllocTable::populateMgmt(vBuf& serialisedContainer) {
    const uint32_t nodeType = NODE_TYPE_MGMT; 
    size_t        bytesLeft = serialisedContainer.size();

    //iterate management table: mark chunk as management node type, add linked list metadata and fill with serialised payload chunks
    for(auto& it : m) {
      size_t bytesToCopy = min(bytesLeft, payloadPerChunk);
      memcpy( (uint8_t*)&it.b[0], (uint8_t*)&serialisedContainer[0], bytesToCopy )                          //copy slice into buffer
      it.b[NODE_FLAGS + 3] = (uint8_t)NODE_TYPE_MGMT;                                                       //Node Type. +3 is bit 0..7 of NODE_FLAGS word
      if (bytesToCopy == payloadPerChunk) {
        writeLeNumberToBeBytes((uint8_t*)&it.b[NODE_DEF_DEST_PTR], adrConv(AdrType::MGMT, AdrType::EXT, it.cpu, it.adr)); //Link to next Element
      } else {
        writeLeNumberToBeBytes((uint8_t*)&it.b[NODE_DEF_DEST_PTR], LM32_NULL_PTR); //Last element, null link to next element
      }
      bytesLeft -= bytesToCopy;
    }
  } 

  //recover payload (serialised container) from buffers of management table
  vBuf AllocTable::recoverMgmt() {
    vBuf ret;
    uint32_t chunkLinkPtr;
    size_t bytesLeft                    = mgmtSize;
    std::pair<uint8_t, AdrType> adrDesc = adrClassification(mgmtStartAdr);
    uint32_t                        adr = adrConv(adrDesc.second, AdrType::MGMT, adrDesc.first, mgmtStartAdr);

    //traverse the linked list by looking up elements in the management table. Copy payload of found elements to return vector
    while(adr != LM32_NULL_PTR) {
      size_t bytesToCopy = min(bytesLeft, payloadPerChunk);
      //lookup entry and fetch buffer content
      auto aux      = m.get<CpuAdr>().find(boost::make_tuple( adrDesc.first, adr ));
      auto it       = m.iterator_to( *aux );
      if (it == m.end()) throw std::runtime_error("MgmtTable: Cannot find entry for CPU " + (int)adrDesc.first + " Adr 0x" +  +"\n"); 
      //add payload to return vector
      ret.insert( ret.end(), it->b, it->b + bytesToCopy);
      //get next entry
      chunkLinkPtr  = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&it->b[NODE_DEF_DEST_PTR]);
      adrDesc       = adrClassification(chunkLinkPtr);
      adr           = adrConv(adrDesc.second, AdrType::MGMT, adrDesc.first, chunkLinkPtr);
      bytesLeft    -= bytesToCopy;
    } 

    //recovery of management binary complete, return
    return ret;

  }

  void AllocTable::deallocateAllMgmt() {
    for(auto& it : m) { vPool[x.cpu].freeChunk(x.adr); }
  }

  bool AllocTable::deallocate(uint32_t hash) {

    auto x = lookupHash(hash);
    
    if (x == a.end()) {
      //std::cout << "nullptr" << std::endl;
      return false;}
    if (vPool.size() <= x->cpu) {
      //std::cout << "cpu idx out of range" << std::endl;
      return false;}

    if (!(vPool[x->cpu].freeChunk(x->adr))) {
      //std::cout << "Chunk" << std::endl;
      return false;}
    if (!(removeByHash(hash))) {
      //std::cout << "AT Hash" << std::endl; 
      return false;}
    return true;
    

  }

  bool AllocTable::syncToAtBmps(AllocTable const &src) {
    //check of the number of memories is identical
    if(vPool.size() != src.vPool.size()) {return false;}
    for (unsigned int i = 0; i < vPool.size(); i++ ) { vPool[i].setBmp(src.vPool[i].getBmp());}
    return true;
  }

  bool AllocTable::setBmps(vBuf bmpData) {
    size_t bmpSum = 0;
    for(unsigned int i = 0; i < vPool.size(); i++) {bmpSum += vPool[i].bmpSize;}
    if (bmpSum != bmpData.size()) return false;

    //iterate the bmps, take their sizes to split input vector
    vBuf::iterator bmpBegin = bmpData.begin();

    for(unsigned int i = 0; i < vPool.size(); i++) {
      vBuf vTmp(bmpBegin, bmpBegin + vPool[i].bmpSize);
      vPool[i].setBmp(vTmp);
      bmpBegin += vPool[i].bmpSize;
    }  

    return true;

  }

  vBuf AllocTable::getBmps() {
    vBuf ret;
    size_t bmpSum = 0;
    for(unsigned int i = 0; i < vPool.size(); i++) {bmpSum += vPool[i].bmpSize;}
    ret.reserve( bmpSum ); // preallocate memory
    //apend all bmps
    for(unsigned int i = 0; i < vPool.size(); i++) {ret += vPool[i].getBmp();}
    
    return ret;
  }

  AllocTable::AllocTable(AllocTable const &src) {
    this->a = src.a;
    this->syncToAtBmps(src);
    this->updatePools();
  }

  void AllocTable::debug(std::ostream& os) {
    for (amI x = a.begin(); x != a.end(); x++) {

    os   << std::setfill(' ') << std::setw(4) << std::dec << x->v 
        << "   "    << std::setfill(' ') << std::setw(2) << std::dec << (int)x->staged
        << "   "    << std::setfill(' ') << std::setw(4) << std::dec << (int)x->cpu
        << "   0x"  << std::hex << std::setfill('0') << std::setw(8) << x->hash
        << "   0x"  << std::hex << std::setfill('0') << std::setw(8) << adrConv(AdrType::MGMT, AdrType::INT, x->cpu, x->adr) 
        << "   0x"  << std::hex << std::setfill('0') << std::setw(8) << adrConv(AdrType::MGMT, AdrType::EXT, x->cpu, x->adr)  << std::endl;
    }    
  }

  const uint32_t AllocTable::adrConv(AdrType from, AdrType to, const uint8_t cpu, const uint32_t a) const {
    if (a == LM32_NULL_PTR) return a;
    
    //std::cerr << "atypes " << debugAdrType(from) << ", " << debugAdrType(to) << std::endl;

    switch (ADR_FROM_TO(from,to)) {
      case ADR_FROM_TO(AdrType::EXT,  AdrType::MGMT) : return a - vPool[cpu].extBaseAdr;
      case ADR_FROM_TO(AdrType::EXT,  AdrType::PEER) : return a - vPool[cpu].extBaseAdr  + vPool[cpu].peerBaseAdr;
      case ADR_FROM_TO(AdrType::EXT,  AdrType::INT)  : return a - vPool[cpu].extBaseAdr  + vPool[cpu].intBaseAdr;
      case ADR_FROM_TO(AdrType::INT,  AdrType::MGMT) : return a - vPool[cpu].intBaseAdr;
      case ADR_FROM_TO(AdrType::INT,  AdrType::EXT)  : return a - vPool[cpu].intBaseAdr  + vPool[cpu].extBaseAdr;
      case ADR_FROM_TO(AdrType::INT,  AdrType::PEER) : return a - vPool[cpu].intBaseAdr  + vPool[cpu].peerBaseAdr;
      case ADR_FROM_TO(AdrType::PEER, AdrType::MGMT) : return a - vPool[cpu].peerBaseAdr;
      case ADR_FROM_TO(AdrType::PEER, AdrType::EXT)  : return a - vPool[cpu].peerBaseAdr + vPool[cpu].extBaseAdr;
      case ADR_FROM_TO(AdrType::PEER, AdrType::INT)  : return a - vPool[cpu].peerBaseAdr + vPool[cpu].intBaseAdr;
      case ADR_FROM_TO(AdrType::MGMT, AdrType::EXT)  : return a + vPool[cpu].extBaseAdr;
      case ADR_FROM_TO(AdrType::MGMT, AdrType::INT)  : return a + vPool[cpu].intBaseAdr;
      case ADR_FROM_TO(AdrType::MGMT, AdrType::PEER) : return a + vPool[cpu].peerBaseAdr;
      default : throw std::runtime_error("bad address conversion perspective"); return 0;
    }

  }


  //classify an address found in downloaded binary
  const std::pair<uint8_t, AdrType> AllocTable::adrClassification(const uint32_t a) const {
    uint8_t cpu  = -1;
    AdrType adrT = AdrType::UNKNOWN;

    //we can rule out 'external' address class, because all nodes use addresses from some CPU's perspective to ease real time handling for DM Firmware
    //we can also rule out the 'management' address class, because this only exists inside the allocation tables
    //this leaves 'peer' and 'internal'. 'peer' has a very high base offset because the world crossbar is located at 0x80000000,
    //which makes the address class easy to discern from internal

    //printf("a 0x%08x, world 0x%08x\n", a, WORLD_BASE_ADR);

    if (a >= WORLD_BASE_ADR) { //peer address class
      
      for (uint8_t i = 0; i < vPool.size(); i++) { //no iterator used because we need the cpu idx for later
        //printf("a 0x%08x, peer %u start 0x%08x end 0x%08x\n", a, i, vPool[cpu].peerBaseAdr, vPool[i].peerBaseAdr + vPool[i].rawSize);
        if ( (a >= vPool[i].peerBaseAdr) && (a <= vPool[i].peerBaseAdr + vPool[i].rawSize) ) {cpu = i; adrT = AdrType::PEER; break;}
      } 
    } else { //internal address class
      
      for (uint8_t i = 0; i < vPool.size(); i++) { //no iterator used because we need the cpu idx for later
        //printf("a 0x%08x, int %u start 0x%08x end 0x%08x\n", a, i, vPool[i].intBaseAdr, vPool[i].intBaseAdr + vPool[i].rawSize);
        if ( (a >= vPool[i].intBaseAdr) && (a <= vPool[i].intBaseAdr + vPool[i].rawSize) ) {cpu = i; adrT = AdrType::INT; break;}
      } 
    }
    
    return  std::make_pair(cpu, adrT);
  }
