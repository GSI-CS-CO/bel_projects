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
#include "common.h"
#include "mempool.h"

#define ALLOC_OK             (0)
#define ALLOC_NO_SPACE      (-1)
#define ALLOC_ENTRY_EXISTS  (-2) 

using boost::multi_index_container;
using namespace boost::multi_index;

#define ADR_FROM_TO(from,to) ( (((uint8_t)from & 0xf) << 4) | ((uint8_t)to   & 0xf) )
 
//enum class AdrType {EXT = 0, INT = 1, PEER = 2, MGMT = 3};

enum class AllocPoolMode {WITHOUT_MGMT = 0, WITH_MGMT = 1};

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
      tag<Vertex>,  BOOST_MULTI_INDEX_MEMBER(AllocMeta,vertex_t,v)>,
    hashed_unique<
      tag<Hash>,  BOOST_MULTI_INDEX_MEMBER(AllocMeta,uint32_t,hash)>,  
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

struct MgmtMeta {
  uint8_t     cpu;
  uint32_t    adr;
  uint8_t     b[_MEM_BLOCK_SIZE];
 


  MgmtMeta(uint8_t cpu, uint32_t adr) : cpu(cpu), adr(adr) {std::memset(b, 0, sizeof b);}
  MgmtMeta(uint8_t cpu, uint32_t adr, uint8_t* src) : cpu(cpu), adr(adr) {std::copy(src, src + sizeof b, b);}
  
  // Multiindexed Elements are immutable, must use the modify function of the container to change attributes
};



typedef boost::multi_index_container<
  MgmtMeta,
  indexed_by<
    ordered_unique<
      tag<CpuAdr>,
      composite_key<
        MgmtMeta,
        BOOST_MULTI_INDEX_MEMBER(MgmtMeta,uint8_t,cpu),
        BOOST_MULTI_INDEX_MEMBER(MgmtMeta,uint32_t,adr)
      >
    >
  >    
 > MgmtMeta_set;

typedef MgmtMeta_set::iterator mmI;



class AllocTable {

  AllocMeta_set a;
  MgmtMeta_set  m;
  std::vector<MemPool> vPool;
  const size_t payloadPerChunk = _MEM_BLOCK_SIZE - 1 - _PTR_SIZE_;
  uint32_t mgmtStartAdr;
  uint32_t mgmtTotalSize;
  uint32_t mgmtGrpSize;
  uint32_t mgmtCovSize;
  



public:

  AllocTable(){};
  ~AllocTable(){};

   //deep copy
  AllocTable(AllocTable const &src);

  AllocTable &operator=(const AllocTable &src);

  void cpyWithoutMgmt(AllocTable const &src);

  std::vector<MemPool>& getMemories() {return vPool;}
  void addMemory(uint8_t cpu, uint32_t extBaseAdr, uint32_t intBaseAdr, uint32_t peerBaseAdr, uint32_t sharedOffs, uint32_t space, uint32_t rawSize) {vPool.push_back(MemPool(cpu, extBaseAdr, intBaseAdr, peerBaseAdr, sharedOffs, space, rawSize)); }
  void clearMemories() { for (unsigned int i = 0; i < vPool.size(); i++ ) vPool[i].init(); }
  void removeMemories() { vPool.clear(); }
  uint32_t getTotalSpace(uint8_t cpu)    const { return vPool[cpu].getTotalSpace(); }
  uint32_t getFreeSpace(uint8_t cpu)     const { return vPool[cpu].getFreeSpace(); }
  uint32_t getUsedSpace(uint8_t cpu)     const { return vPool[cpu].getUsedSpace(); }
  

  void syncBmpsToPools(); // generate BMP from Pool
  void recreatePools(AllocPoolMode mode);

  
  bool setBmps(vBuf bmpData);
  vBuf getBmps();

  // Multiindexed Elements are immutable, need to use the modify function of the container to change attributes
  void setStaged(amI it) { a.modify(it, [](AllocMeta& e){e.staged = true;}); }
  void clrStaged(amI it) { a.modify(it, [](AllocMeta& e){e.staged = false;}); }
  bool isStaged(amI it)  { return it->staged; }
  void modV(amI it, vertex_t vNew) { a.modify(it, [vNew](AllocMeta& e){e.v = vNew;}); } 

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

  void clear() { a.clear(); m.clear(); mgmtStartAdr = LM32_NULL_PTR; mgmtTotalSize = 0; mgmtGrpSize = 0; mgmtCovSize = 0; clearMemories(); } // clears everything including management


  //FIXME would like iterator range to a.get<Adr>() better, but no time to figure out the syntax right now
  const AllocMeta_set& getTable() const { return a; }
  const size_t getSize()          const { return a.size(); }

  amI lookupVertex(vertex_t v, const std::string& exMsg = "")   const;
  amI lookupHash(uint32_t hash, const std::string& exMsg = "")  const;
  amI lookupAdr(uint8_t cpu, uint32_t adr, const std::string& exMsg = "")    const;

  bool isOk(amI it) const {return (it != a.end()); }

  //conver addresses from one perspective to another
  const uint32_t adrConv(AdrType from, AdrType to, const uint8_t cpu, const uint32_t a) const;


  //identify an address found in downloaded binary (cpuIdx, int/peer)
  const std::pair<uint8_t, AdrType> adrClassification(const uint32_t a) const;
  const uint8_t getCpuFromExtAdr(const uint32_t a);

  void debug(std::ostream& os);

  // Management Table. Handles all nodes used in the linked list of the management binary
  int  allocateMgmt(vBuf& serialisedContainer);
  int  allocateMgmt(uint8_t cpu);
  bool insertMgmt(uint8_t cpu, uint32_t adr, uint8_t* buf);
  void deallocateAllMgmt(); //no individual deallocation, makes no sense cause we wrap an unknown binary. We always clear the whole table
  void populateMgmt(vBuf& serialisedContainer);
  vBuf recoverMgmt();

  void setMgmtLLstartAdr(uint32_t startAdr) {mgmtStartAdr = startAdr;}
  void setMgmtLLSizes(uint32_t grpSize, uint32_t covSize) {mgmtGrpSize = grpSize; mgmtCovSize = covSize;}
  uint32_t getMgmtLLstartAdr()  {return mgmtStartAdr;}
  void setMgmtTotalSize(uint32_t totSize)      {mgmtTotalSize = totSize;}
  uint32_t getMgmtTotalSize()      {return mgmtTotalSize;}
  uint32_t getMgmtGrpSize()      {return mgmtGrpSize;}
  uint32_t getMgmtCovSize()      {return mgmtCovSize;}
  void debugMgmt(std::ostream& os);
  const MgmtMeta_set& getMgmtTable() const { return m; }
  const size_t getMgmtSize()          const { return m.size(); }


};

#endif 