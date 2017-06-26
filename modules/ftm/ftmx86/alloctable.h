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
#include "graph.h"

using boost::multi_index_container;
using namespace boost::multi_index;

struct AllocMeta {
  uint32_t    adr;
  uint32_t    hash;
  vertex_t    v;
  uint8_t     b[_MEM_BLOCK_SIZE];

  AllocMeta(uint32_t adr, uint32_t hash) : adr(adr), hash(hash) {};
  AllocMeta(uint32_t adr, uint32_t hash, vertex_t v) : adr(adr), hash(hash), v(v) {};

};

struct Adr{};
struct Hash{};
struct Vertex{};

typedef boost::multi_index_container<
  AllocMeta,
  indexed_by<
    ordered_unique<
      tag<Adr>,  BOOST_MULTI_INDEX_MEMBER(AllocMeta,uint32_t,adr)>,
    hashed_unique<
      tag<Hash>,  BOOST_MULTI_INDEX_MEMBER(AllocMeta,uint32_t,hash)>,
    hashed_unique<
      tag<Vertex>,  BOOST_MULTI_INDEX_MEMBER(AllocMeta,vertex_t,v)> >  
 > AllocMeta_set;


class AllocTable {

  AllocMeta_set a;

public:

  AllocTable(){};
  ~AllocTable(){};

  bool insert(uint32_t adr, uint32_t hash, vertex_t v);

  bool removeByVertex(vertex_t v);
  bool removeByAdr(uint32_t adr);
  bool removeByHash(uint32_t hash);

  void clear() { a.clear(); }

  //FIXME would like iterator range to a.get<Adr>() better, but no time to figure out the syntax right now
  const AllocMeta_set& getTable() const { return a; }
  const size_t getSize() const { return a.size(); }

  AllocMeta* lookupVertex(vertex_t v) const;
  AllocMeta* lookupHash(uint32_t hash) const;
  AllocMeta* lookupAdr(uint32_t adr) const;

};

#endif 