#ifndef _BMI_CONTAINERS_H_
#define _BMI_CONTAINERS_H_

struct Hash{};
struct Vertex{};
struct CpuAdr{};

/*
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


using boost::multi_index_container;
using namespace boost::multi_index;


struct AllocMeta {
  uint8_t     cpu;
  uint32_t    adr;
  uint32_t    hash;
  vertex_t    v;
  uint8_t     b[_MEM_BLOCK_SIZE];
  bool        staged;
  bool        global;

  AllocMeta(uint8_t cpu, uint32_t adr, uint32_t hash) : cpu(cpu), adr(adr), hash(hash) {std::memset(b, 0, sizeof b);}
  AllocMeta(uint8_t cpu, uint32_t adr, uint32_t hash, vertex_t v) : cpu(cpu), adr(adr), hash(hash), v(v), staged(false) {std::memset(b, 0, sizeof b);}
  AllocMeta(uint8_t cpu, uint32_t adr, uint32_t hash, vertex_t v, bool staged) : cpu(cpu), adr(adr), hash(hash), v(v), staged(staged) {std::memset(b, 0, sizeof b);}
  AllocMeta(uint8_t cpu, uint32_t adr, uint32_t hash, vertex_t v, bool staged, bool global) : cpu(cpu), adr(adr), hash(hash), v(v), staged(staged), global(global) {std::memset(b, 0, sizeof b);}
  // Multiindexed Elements are immutable, must use the modify function of the container to change attributes
};




struct AllocMeta_set {
  using type = boost::multi_index_container<
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
 >;
};

typedef AllocMeta_set::iterator amI;

struct MgmtMeta {
  uint8_t     cpu;
  uint32_t    adr;
  uint8_t     b[_MEM_BLOCK_SIZE];



  MgmtMeta(uint8_t cpu, uint32_t adr) : cpu(cpu), adr(adr) {std::memset(b, 0, sizeof b);}
  MgmtMeta(uint8_t cpu, uint32_t adr, uint8_t* src) : cpu(cpu), adr(adr) {std::copy(src, src + sizeof b, b);}

  // Multiindexed Elements are immutable, must use the modify function of the container to change attributes
};


struct MgmtMeta_set {
  using type = boost::multi_index_container<
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
 >;
}

typedef MgmtMeta_set::iterator mmI;

struct RefMeta {
  uint8_t     cpu;
  uint32_t    adr;
  uint32_t    hash;
  
  RefMeta(uint8_t cpu, uint32_t adr, uint32_t hash) : cpu(cpu), adr(adr) hash(hash) {}

  // Multiindexed Elements are immutable, must use the modify function of the container to change attributes
};

struct RefMeta_set {
  using type = boost::multi_index_container<
  RefMeta,
  indexed_by<
    ordered_unique<
      tag<CpuAdr>,
      composite_key<
        RefMeta,
        BOOST_MULTI_INDEX_MEMBER(MgmtMeta,uint8_t,cpu),
        BOOST_MULTI_INDEX_MEMBER(MgmtMeta,uint32_t,adr)
      >
    >
  >
 >;
};

typedef RefMeta_set::iterator refIt;
*/
#endif