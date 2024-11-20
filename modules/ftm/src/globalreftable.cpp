#include "globalreftable.h"
#include "common.h"
#include "aux_boost.h"

    uint32_t GlobalRefTable::getHash(uint32_t adr) {
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

    void GlobalRefTable::debug(std::ostream& os) {
        os << "Global Reftable Entries:" << std::endl;
        for (refIt x = m.begin(); x != m.end(); x++) {
          os << "Adr 0x" << std::hex << x->first << " Hash: " << std::hex <<  x->second << std::endl;
        }
    }

    //could this be generic to all stored containers?

    // Store the object in a serialized form (to a string)
    std::string GlobalRefTable::store() {
        std::stringstream os;
        boost::archive::text_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(*this);
        return fixArchiveVersion(os.str());
    }

    // Load the object from a serialized form (from a string)
    void GlobalRefTable::load(const std::string& s) {
        std::stringstream is;
        is.str(fixArchiveVersion(s));
        boost::archive::text_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(*this);
    }