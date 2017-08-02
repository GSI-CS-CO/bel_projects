#include "alloctable.h"

  

  bool AllocTable::insert(uint8_t cpu, uint32_t adr, uint32_t hash, vertex_t v) {
    /*
    std::cout << "Problem: " << std::endl; 
    if (lookupAdr(cpu, adr) != NULL) std::cout << (int)cpu << " Adr 0x" << std::hex << adr << " exists already" << std::endl;
    if (lookupHash(hash) != NULL) std::cout << "Hash 0x" << std::hex << hash << " exists already" << std::endl;
    if (lookupVertex(v) != NULL) std::cout << "V 0x" << std::dec << v << " exists already" << std::endl;
    */
    auto x = a.insert({cpu, adr, hash, v});
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

  AllocMeta* AllocTable::lookupVertex(vertex_t v) const  {
    auto it = a.get<Vertex>().find(v);
    if (it != a.get<Vertex>().end()) return (AllocMeta*)&(*it);
    else return NULL;
  }

  AllocMeta* AllocTable::lookupHash(uint32_t hash) const  {
    auto it = a.get<Hash>().find(hash);
    if (it != a.get<Hash>().end()) return (AllocMeta*)&(*it);
    else return NULL;
  }

  AllocMeta* AllocTable::lookupAdr(uint8_t cpu, uint32_t adr) const {
    auto it = a.get<CpuAdr>().find(boost::make_tuple( cpu, adr ));
    if (it != a.get<CpuAdr>().end()) return (AllocMeta*)&(*it);
    else return NULL;
    
  }

  //Allocation functions
  int AllocTable::allocate(uint8_t cpu, uint32_t hash, vertex_t v) {
    uint32_t chunkAdr;
    //std::cout << "Cpu " << (int)cpu << " mempools " << vPool.size() << std::endl;
    if (cpu >= vPool.size()) {
      //std::cout << "cpu idx out of range" << std::endl;
      return ALLOC_NO_SPACE;}

    if (!(vPool[cpu].acquireChunk(chunkAdr))) return ALLOC_NO_SPACE;
    if (!(insert(cpu, chunkAdr, hash, v)))    return ALLOC_ENTRY_EXISTS;

    return ALLOC_OK;
  }

  bool AllocTable::deallocate(uint32_t hash) {

    auto* x = lookupHash(hash);
    
    if (x == NULL) {
      //std::cout << "NULL" << std::endl;
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

  bool AllocTable::syncBmps(AllocTable const &src) {
    //check of the number of memories is identical
    if(vPool.size() != src.vPool.size()) {return false;}
    for (unsigned int i = 0; i < vPool.size(); i++ ) vPool[i].setBmp(src.vPool[i].getBmp());
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
    for(unsigned int i = 0; i < vPool.size(); i++) {ret.insert( ret.end(), vPool[i].getBmp().begin(), vPool[i].getBmp().end() );}
    
    return ret;
  }

  AllocTable::AllocTable(AllocTable const &src) {
    this->a = src.a;
    this->syncBmps(src);
  }  
