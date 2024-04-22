#ifndef _GLOBAL_REF_TABLE_H_
#define _GLOBAL_REF_TABLE_H_

#include <stdint.h>


#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <sstream>
#include <string>
#include <iostream>
#include <map>

typedef std::map<uint32_t, uint32_t>::iterator refIt;

class GlobalRefTable {
    std::map<uint32_t, uint32_t> m;
public:
    

    GlobalRefTable() = default;  // Default constructor

    bool insert(uint32_t adr, uint32_t hash) {auto x = m.insert(std::make_pair(adr, hash)); return x.second; }

    uint32_t getHash(uint32_t adr) {
        refIt it = m.find(adr);
        uint32_t ret;

        if( it != m.end()) ret = it->second;
        else  {
            std::stringstream auxstream;
            auxstream << "GlobalRefTable: Unknown Adr " << " 0x" << std::setfill('0') << std::setw(8) << std::hex << (int)adr << std::endl;
            throw std::runtime_error(auxstream.str());
        }
            
        return ret;
    }

    refIt lookupAdr(uint32_t adr) {return m.find(adr);}

    void clear() { m.clear(); }

    bool isOk(refIt it) const {return (it != m.end()); }

    void debug(std::ostream& os) {
        os << "Global Reftable Entries:" << std::endl;
        for (refIt x = m.begin(); x != m.end(); x++) {
          os << "Adr 0x" << std::hex << x->first << " Hash: " << std::hex <<  x->second << std::endl;
        }
    }

    // Store the object in a serialized form (to a string)
    std::string store() {
        std::stringstream os;
        boost::archive::text_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(m);
        return os.str();
    }

    // Load the object from a serialized form (from a string)
    void load(const std::string& s) {
        std::stringstream is(s);
        boost::archive::text_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(m);
    }
};


#endif
