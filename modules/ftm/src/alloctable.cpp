#include "alloctable.h"


  bool AllocTable::insert(uint32_t adr, uint32_t hash, vertex_t v) {
    /*
    std::cout << "Problem: " << std::endl; 
    if (lookupAdr(adr) != NULL) std::cout << "Adr 0x" << std::hex << adr << " exists already" << std::endl;
    if (lookupHash(hash) != NULL) std::cout << "Hash 0x" << std::hex << hash << " exists already" << std::endl;
    if (lookupVertex(v) != NULL) std::cout << "V 0x" << std::dec << v << " exists already" << std::endl;
    */
    auto x = a.insert({adr, hash, v});
    return x.second;
  }

  bool AllocTable::removeByVertex(vertex_t v) {
    auto it = a.get<Vertex>().erase(v);
    if (it != 0) { return true;  }
    else         { return false; }
  }

  bool AllocTable::removeByAdr(uint32_t adr) {
    auto it = a.get<Adr>().erase(adr);
    if (it != 0) { return true;  }
    else         { return false; }
    
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

  AllocMeta* AllocTable::lookupAdr(uint32_t adr) const {
    auto it = a.get<Adr>().find(adr);
    if (it != a.get<Adr>().end()) return (AllocMeta*)&(*it);
    else return NULL;
  }


