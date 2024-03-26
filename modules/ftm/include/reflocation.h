
#include <stdint.h>
#include <string>
#include <iostream>
#include <set>
#include <boost/optional.hpp>
#include <boost/bimap.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/composite_key.hpp>

using boost::multi_index_container;
using namespace boost::multi_index;

typedef boost::bimap< const std::string, uint32_t > memLocMap;
typedef boost::bimap< const std::string, uint32_t > memFieldMap;


struct memoryArea {
  const std::string& name;
  const uint32_t&    adr;
  const uint32_t&    size;
  vertex_t    v;
};  


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


int main() {
    MemoryManager manager;

    // Registering base addresses with sizes
    manager.registerBaseAddress("Data1", 1000, 10);
    manager.registerBaseAddress("Data2", 2000, 20);
    manager.registerBaseAddress("Data3", 3000, 30);

    // Registering field offsets
    manager.registerFieldOffset(0);
    manager.registerFieldOffset(100);
    manager.registerFieldOffset(200);

    // Simulating lookup using cumulative address sum
    size_t cumulativeAddressSum = 2305;

 

    // Lookup the base address corresponding to the cumulative address sum
    size_t baseAddress = 0;
    for (const auto& entry : manager.baseAddressMap) {
        size_t addr = entry.first.first;
        size_t size = entry.first.second;
        if ((cumulativeAddressSum >= addr) && (cumulativeAddressSum < (addr + size))) {
            baseAddress = addr;
            break;
        }
    }

       // Lookup the name string corresponding to the cumulative address sum
    std::string name = manager.lookupName(baseAddress);
    std::cout << "Corresponding name: " << name << std::endl;

    // Lookup the field offset corresponding to the cumulative address sum
    size_t fieldOffset = manager.lookupFieldOffset(cumulativeAddressSum, baseAddress);
    std::cout << "Corresponding field offset: " << fieldOffset << std::endl;

    return 0;
}
