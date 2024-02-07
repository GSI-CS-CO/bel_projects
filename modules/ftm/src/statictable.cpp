#include "statictable.h"

  std::string StaticTable::store() {
    std::stringstream os;
    boost::archive::text_oarchive oa(os);
    oa << BOOST_SERIALIZATION_NVP(*this);
    return fixArchiveVersion(os.str());
  };


  void StaticTable::load(const std::string& s) {
    std::stringstream is;
    is.str(fixArchiveVersion(s));
    boost::archive::text_iarchive ia(is);
    ia >> BOOST_SERIALIZATION_NVP(*this);
  }
