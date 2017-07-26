#include "alloctable.h"

  using namespace AllocTable;

  bool insert(uint8_t cpu, uint32_t adr, uint32_t hash, vertex_t v) {
    /*
    std::cout << "Problem: " << std::endl; 
    if (lookupAdr(adr) != NULL) std::cout << "Adr 0x" << std::hex << adr << " exists already" << std::endl;
    if (lookupHash(hash) != NULL) std::cout << "Hash 0x" << std::hex << hash << " exists already" << std::endl;
    if (lookupVertex(v) != NULL) std::cout << "V 0x" << std::dec << v << " exists already" << std::endl;
    */
    auto x = a.insert({cpu, adr, hash, v});
    return x.second;
  }

  bool removeByVertex(vertex_t v) {
    auto it = a.get<Vertex>().erase(v);
    if (it != 0) { return true;  }
    else         { return false; }
  }

  bool removeByAdr(uint8_t cpu, uint32_t adr) {
    auto it = a.get<CpuAdr>().erase(boost::make_tuple( cpu, adr ));
    if (it != 0) { return true;  }
    else         { return false; }
    
  }

  bool removeByHash(uint32_t hash) {
    auto it = a.get<Hash>().erase(hash);
    if (it != 0) { return true;  }
    else         { return false; }

  }

  AllocMeta* lookupVertex(vertex_t v) const  {
    auto it = a.get<Vertex>().find(v);
    if (it != a.get<Vertex>().end()) return (AllocMeta*)&(*it);
    else return NULL;
  }

  AllocMeta* lookupHash(uint32_t hash) const  {
    auto it = a.get<Hash>().find(hash);
    if (it != a.get<Hash>().end()) return (AllocMeta*)&(*it);
    else return NULL;
  }

  AllocMeta* lookupAdr(uint8_t cpu, uint32_t adr) const {
    auto it = a.get<CpuAdr>().find(boost::make_tuple( cpu, adr ));
    if (it != a.get<CpuAdr>().end()) return (AllocMeta*)&(*it);
    else return NULL;
  }

  //Allocation functions
  int allocate(uint8_t cpu, uint32_t hash, vertex_t v) {
    uint32_t chunkAdr;
    if (vPool.size() <= cpu) {
      //std::cout << "cpu idx out of range" << std::endl;
      return false;}

    if (!(vPool[cpu].acquireChunk(chunkAdr))) return ALLOC_NO_SPACE;
    if (!(insert(cpu, chunkAdr, hash, v)))         return ALLOC_ENTRY_EXISTS;

    return ALLOC_OK;
  }

  bool deallocate(uint32_t hash) {

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

  bool syncBmps(AllocTable const &src) {
    //check of the number of memories is identical
    if(vPool.size() != aSource.vPool.size()) {return false;}
    for (int i = 0; i < vPool.size(); i++ ) vPool[i].setBmp(aSource.vPool[i].getBmp());
    return true;
  }

  AllocTable(AllocTable const &src) {
    this->a = src.a;
    this->syncBmps(src);
  }  
