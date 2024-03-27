
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

//we ha
struct memoryLoc {
  const std::string& name;
  const uint32_t&    adr;
  const uint32_t&    size;
};  

struct memoryLoc {
  const std::string& regien;
  const uint32_t&    adr;
  const uint32_t&    size;
};

// Define the inner struct loc
struct loc {
    std::string name;
    unsigned int adr;
    unsigned int off;

    loc(const std::string& name, unsigned int adr, unsigned int offs) : name(name), adr(adr), off(off), {}

    // Operator overload for printing loc
    friend std::ostream& operator<<(std::ostream& os, const loc& l) {
        os << "Name: " << l.name << ", Address: " << l.adr;
        return os;
    }
};

// Define the outer struct aml
struct aml {
    unsigned int v;
    loc l;

    aml(unsigned int v, const std::string& name, unsigned int adr, unsigned int offs) : v(v), l(name, adr, off) {}
    aml(unsigned int v, const &loc l) : v(v), l(l) {}

    // Operator overload for printing aml
    friend std::ostream& operator<<(std::ostream& os, const aml& a) {
        os << "Value: " << a.v << ", " << a.l;
        return os;
    }
};

// Define a multi-index container with two indices: by 'v' and by 'adr'
typedef multi_index_container<
    aml,
    indexed_by<
        ordered_non_unique<member<aml, unsigned int, &aml::v>>,
        ordered_non_unique<member<loc, unsigned int, &loc::adr>>
    >
> aml_multi_index_container;



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
