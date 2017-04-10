#include "memunit.h"
#include "common.h"


  void MemUnit::updatememPoolFromBmp32(uint32_t bmp, int bmpNo) {
    for(uint32_t i=0; i < 32; i++) {
      if((bmp >> i) & 1) memPool.insert((i + bmpNo * 32) * MEM_BLOCK_SIZE);
    }
  } 

  void MemUnit::initMemPool() { 
    memPool.clear();
    uint32_t adr;
    for(uint32_t i=MEM_BLOCK_SIZE; i < poolSize; i+= MEM_BLOCK_SIZE) { //never issue <baseAddress>, as this is where Mgmt Entry vector resides
      adr =  baseAdr + i;
      std::cout << std::hex << adr << std::endl; 
      memPool.insert(adr); 
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
    bool ret = true;
    if ((adr % MEM_BLOCK_SIZE) || (memPool.count(adr) > 0))  {ret = false;} //unaligned or attempted double entry, throw exception
    else memPool.insert(adr);
    return ret;
  }        

  //Allocation functions
  bool MemUnit::allocate(const std::string& name) {
    uint32_t chunkAdr, hash;
    bool ret = true;

    insertHash(name, hash);
    if ( (allocMap.count(name) == 0) && acquireChunk(chunkAdr) ) { allocMap[name] = (chunkMeta) {chunkAdr, hash, vBuf(32)};  

    } else {ret = false;}
    return ret;
  }

  bool MemUnit::insert(const std::string& name, uint32_t adr) {}

  bool MemUnit::deallocate(const std::string& name) {
    bool ret = true;
    if ( (allocMap.count(name) > 0) && freeChunk(allocMap[name].adr) ) { allocMap.erase(name); 
    } else {ret = false;}
    return ret;
  }

  bool MemUnit::lookupName2Chunk(const std::string& name, chunkMeta*& chunk) {
    bool ret = true;
    if (allocMap.count(name) > 0) { chunk = (chunkMeta*)&(allocMap[name]); 
    } else {ret = false;}
    return ret;
  }

  //Hash functions

  bool MemUnit::insertHash(const std::string& name, uint32_t &hash) {
    hash = FnvHash::fnvHash(name.c_str());

    if (hashMap.count(hash) > 0) return false;
    else hashMap[hash] = name;
    return true;
  }

  bool MemUnit::removeHash(const uint32_t hash) {
    if (hashMap.count(hash) > 0) {hashMap.erase(hash); return true;}
    return false;
  }
  
  bool MemUnit::lookupHash2Name(const uint32_t hash, std::string& name)  {
    if (hashMap.count(hash) > 0) {name = hashMap.at(hash); return true;}
    return false;
  }


