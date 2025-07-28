#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "globalreftable.h"
#include "common.h"
#include "aux_boost.h"


  void GlobalRefTable::debug(std::ostream& os) {
      os << "Global Reftable Entries:" << std::endl;
      for (refIt x = m.begin(); x != m.end(); x++) {
        os << "Cpu " << x->cpu << "Adr 0x" << std::hex << x->adr << " Hash: " << std::hex <<  x->hash << std::endl;
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

  



  bool GlobalRefTable::removeByAdr(uint8_t cpu, uint32_t adr) {

    auto it = m.get<CpuAdr>().find(boost::make_tuple( cpu, adr ));
    m.get<CpuAdr>().erase(it, it);
    /*
    if (it != m.get<Vertex>().end())) { return true;  }
    else         { return false; }
    */
    return true;
  }

  bool GlobalRefTable::removeByHash(uint32_t hash) {
    auto it = m.get<Hash>().erase(hash);
    if (it != 0) { return true;  }
    else         { return false; }

  }

  refIt GlobalRefTable::lookupHash(uint32_t hash, const std::string& exMsg) const  {
    refIt ret;
    auto it = m.get<Hash>().find(hash);
    ret = m.iterator_to( *it );
    if (!isOk(ret)) {
        std::stringstream auxstream;
        auxstream << "GlobalRefTable: Unknown Hash " << " 0x" << std::setfill('0') << std::setw(8) << std::hex << (int)hash << std::endl;
        throw std::runtime_error(auxstream.str());
    }
    return ret;
  }

  refIt GlobalRefTable::lookupHashNoEx(uint32_t hash) const  {
    refIt ret;
    auto it = m.get<Hash>().find(hash);
    ret = m.iterator_to( *it );
    return ret;
  }

  refIt GlobalRefTable::lookupAdr(uint8_t cpu, uint32_t adr, const std::string& exMsg) const {

    refIt ret;
    auto it = m.get<CpuAdr>().find(boost::make_tuple( cpu, adr ));
    ret = m.iterator_to( *it );

    std::stringstream auxstream;
    auxstream << exMsg << "GlobalRefTable: Unknown cpu/adr combo " << (int)cpu << " 0x" << std::setfill('0') << std::setw(8) << std::hex << (int)adr << std::endl;

    if (!isOk(ret)) {throw std::runtime_error(auxstream.str());}
    return ret;

  }
