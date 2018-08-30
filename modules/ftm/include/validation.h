#ifndef _VALIDATION_H_
#define _VALIDATION_H_

#include <stdint.h>
#include <string>
#include <set>
#include <boost/optional.hpp>
#include <boost/container/vector.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include "graph.h"
#include "common.h"
#include "alloctable.h"

//Multimap for node pair validation
namespace Validation {

typedef std::string edgeType_t;
typedef std::string nodeType_t;
typedef std::set<nodeType_t> children_t;


using boost::multi_index_container;
using namespace boost::multi_index;

struct ConstellationRule {
  nodeType_t parent;
  edgeType_t edge;
  children_t children;
  unsigned int min;
  unsigned int max;

  ConstellationRule(nodeType_t parent, edgeType_t edge, children_t children, unsigned int min, unsigned int max) : parent(parent), edge(edge), children(children), min(min), max(max) {}
};

struct Constellation{};
struct MinOccurrence
{

};

typedef boost::multi_index_container<
  ConstellationRule,
  indexed_by<
    hashed_unique<
      tag<Constellation>,
      composite_key<
        ConstellationRule,
        BOOST_MULTI_INDEX_MEMBER(ConstellationRule,nodeType_t,parent),
        BOOST_MULTI_INDEX_MEMBER(ConstellationRule,edgeType_t,edge)
      >
    >,
    ordered_non_unique<
      tag<MinOccurrence>,
      BOOST_MULTI_INDEX_MEMBER(ConstellationRule, unsigned int, min)
    >
  >
 > ConstellationRule_set;


typedef boost::multi_index_container<
  const ConstellationRule*,
  indexed_by<
    ordered_non_unique<
      tag<MinOccurrence>,
      BOOST_MULTI_INDEX_MEMBER(ConstellationRule, unsigned int, min)
    >
  >
 > ConstellationRule_min_view;


 struct ConstellationCnt {
  nodeType_t parent;
  edgeType_t edge;
  unsigned int cnt;

  ConstellationCnt(nodeType_t parent, edgeType_t edge) : parent(parent), edge(edge), cnt(0) {}
};


 typedef boost::multi_index_container<
  ConstellationCnt,
  indexed_by<
    hashed_unique<
      tag<Constellation>,
      composite_key<
        ConstellationCnt,
        BOOST_MULTI_INDEX_MEMBER(ConstellationCnt,nodeType_t,parent),
        BOOST_MULTI_INDEX_MEMBER(ConstellationCnt,edgeType_t,edge)
      >
    >
  >
 > ConstellationCnt_set;


namespace MaxDepth {
  const unsigned META   = 2;
  const unsigned EVENT  = 1000;
};

extern const children_t cNonMeta;
extern ConstellationRule_set cRules;


void init(); //workaround for boost v1.53 which doesn't support aggregate initalization


void eventSequenceCheck(vertex_t v, Graph& g, bool force); //check if event sequence is well behaved
void metaSequenceCheck(vertex_t v, Graph& g); //check if meta tree is well behaved
void neighbourhoodCheck(vertex_t v, Graph& g); //check if all outedge (nodetype/edgetype/childtype) tupels are valid and occurrence count is within valid bounds

  namespace Aux {
    void metaSequenceCheckAux(vertex_t v, vertex_t vcurrent, Graph& g, unsigned int recursionLvl = 0);
  }

}



#endif
