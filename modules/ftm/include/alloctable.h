#ifndef _ALLOC_TABLE_H_
#define _ALLOC_TABLE_H_

#include <stdint.h>
#include <string>
#include <iostream>
#include <set>
#include <boost/optional.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include "graph.h"
#include "mempool.h"

#define ALLOC_OK             (0)
#define ALLOC_NO_SPACE      (-1)
#define ALLOC_ENTRY_EXISTS  (-2) 

using boost::multi_index_container;
using namespace boost::multi_index;



struct AllocMeta {
  uint8_t     cpu;
  uint32_t    adr;
  uint32_t    hash;
  vertex_t    v;
  uint8_t     b[_MEM_BLOCK_SIZE];
  bool        staged;


  AllocMeta(uint8_t cpu, uint32_t adr, uint32_t hash) : cpu(cpu), adr(adr), hash(hash) {std::memset(b, 0, sizeof b);}
  AllocMeta(uint8_t cpu, uint32_t adr, uint32_t hash, vertex_t v) : cpu(cpu), adr(adr), hash(hash), v(v), staged(false) {std::memset(b, 0, sizeof b);}
  AllocMeta(uint8_t cpu, uint32_t adr, uint32_t hash, vertex_t v, bool staged) : cpu(cpu), adr(adr), hash(hash), v(v), staged(staged) {std::memset(b, 0, sizeof b);}
  
  // Multiindexed Elements are immutable, must use the modify function of the container to change attributes
};


struct Hash{};
struct Vertex{};
struct CpuAdr{};


typedef boost::multi_index_container<
  AllocMeta,
  indexed_by<
    hashed_unique<
      tag<Hash>,  BOOST_MULTI_INDEX_MEMBER(AllocMeta,uint32_t,hash)>,
    hashed_unique<
      tag<Vertex>,  BOOST_MULTI_INDEX_MEMBER(AllocMeta,vertex_t,v)>,
    ordered_unique<
      tag<CpuAdr>,
      composite_key<
        AllocMeta,
        BOOST_MULTI_INDEX_MEMBER(AllocMeta,uint8_t,cpu),
        BOOST_MULTI_INDEX_MEMBER(AllocMeta,uint32_t,adr)
      >
    >
  >    
 > AllocMeta_set;

typedef AllocMeta_set::iterator amI;

class AllocTable {

  AllocMeta_set a;
  std::vector<MemPool> vPool;
  

public:

  AllocTable(){};
  ~AllocTable(){};

   //deep copy
  AllocTable(AllocTable const &src);

  std::vector<MemPool>& getMemories() {return vPool;}
  void addMemory(uint8_t cpu, uint32_t extBaseAdr, uint32_t intBaseAdr, uint32_t peerBaseAdr, uint32_t sharedOffs, uint32_t space) {vPool.push_back(MemPool(cpu, extBaseAdr, intBaseAdr, peerBaseAdr, sharedOffs, space)); }
  void clearMemories() { for (unsigned int i = 0; i < vPool.size(); i++ ) vPool[i].init(); }
  void removeMemories() { vPool.clear(); }

  void updateBmps()  {for (unsigned int i = 0; i < vPool.size(); i++ ) vPool[i].syncBmpToPool();}
  void updatePools() {for (unsigned int i = 0; i < vPool.size(); i++ ) vPool[i].syncPoolToBmp();}

  bool syncToAtBmps(AllocTable const &src);
  bool setBmps(vBuf bmpData);
  vBuf getBmps();

  // Multiindexed Elements are immutable, need to use the modify function of the container to change attributes
  void setStaged(amI it) { a.modify(it, [](AllocMeta& e){e.staged = true;}); }
  void clrStaged(amI it) { a.modify(it, [](AllocMeta& e){e.staged = false;}); }
  bool isStaged(amI it)  { return it->staged; }

  //FIXME
        ////// WORKAROUND - not time to figure out fucking modify_key lambda syntax now /////
  //void setV(amI it, vertex_t vNew) { a.get<Vertex>().modify_key(it, [vNew](vertex_t& v){v = vNew;}); } 

  //Allocation functions
// TODO - Maybe better with pair <iterator, bool> to get a direct handle on the inserted/allocated element?
  int allocate(uint8_t cpu, uint32_t hash, vertex_t v, bool staged);
  int allocate(uint8_t cpu, uint32_t hash, vertex_t v) {return allocate(cpu, hash, v, true); }
  

  bool deallocate(uint32_t hash);

  bool insert(uint8_t cpu, uint32_t adr, uint32_t hash, vertex_t v, bool staged);
  bool insert(uint8_t cpu, uint32_t adr, uint32_t hash, vertex_t v) {return insert(cpu, adr, hash, v, true); }

  bool removeByVertex(vertex_t v);
  bool removeByAdr(uint8_t cpu, uint32_t adr);
  bool removeByHash(uint32_t hash);
  void stageAll()   {for (amI it = a.begin(); it != a.end(); it++) setStaged(it);  }
  void unstageAll() {for (amI it = a.begin(); it != a.end(); it++) clrStaged(it); }

  void clear() { a.clear(); }

  //FIXME would like iterator range to a.get<Adr>() better, but no time to figure out the syntax right now
  const AllocMeta_set& getTable() const { return a; }
  const size_t getSize()          const { return a.size(); }

  amI  lookupVertex(vertex_t v)   const;
  amI lookupHash(uint32_t hash)  const;
  amI lookupAdr(uint8_t cpu, uint32_t adr)    const;

  bool isOk(amI it) const {return (it != a.end()); }

  const uint32_t extAdr2adr(const uint8_t cpu, const uint32_t ea)      const  { return (ea == LM32_NULL_PTR ? LM32_NULL_PTR : ea - vPool[cpu].extBaseAdr); }
  const uint32_t intAdr2adr(const uint8_t cpu, const uint32_t ia)      const  { return (ia == LM32_NULL_PTR ? LM32_NULL_PTR : ia - vPool[cpu].intBaseAdr); }
  const uint32_t peerAdr2adr(const uint8_t cpu, const uint32_t pa)     const  { return (pa == LM32_NULL_PTR ? LM32_NULL_PTR : pa - vPool[cpu].peerBaseAdr); }

  const uint32_t extAdr2intAdr(const uint8_t cpu, const uint32_t ea)   const  { return (ea == LM32_NULL_PTR ? LM32_NULL_PTR : ea - vPool[cpu].extBaseAdr + vPool[cpu].intBaseAdr); }
  const uint32_t intAdr2extAdr(const uint8_t cpu, const uint32_t ia)   const  { return (ia == LM32_NULL_PTR ? LM32_NULL_PTR : ia - vPool[cpu].intBaseAdr + vPool[cpu].extBaseAdr); }
  const uint32_t extAdr2peerAdr(const uint8_t cpu, const uint32_t ea)  const  { return (ea == LM32_NULL_PTR ? LM32_NULL_PTR : ea - vPool[cpu].extBaseAdr + vPool[cpu].peerBaseAdr); }
  const uint32_t peerAdr2extAdr(const uint8_t cpu, const uint32_t pa)  const  { return (pa == LM32_NULL_PTR ? LM32_NULL_PTR : pa - vPool[cpu].peerBaseAdr + vPool[cpu].extBaseAdr); }

  const uint32_t adr2extAdr(const uint8_t cpu, const uint32_t a)       const  { return a + vPool[cpu].extBaseAdr; }
  const uint32_t adr2intAdr(const uint8_t cpu, const uint32_t a)       const  { return a + vPool[cpu].intBaseAdr; }
  const uint32_t adr2peerAdr(const uint8_t cpu, const uint32_t a)      const  { return a + vPool[cpu].peerBaseAdr; }

  void debug();

};

#endif 