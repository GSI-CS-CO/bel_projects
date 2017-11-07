#include "patterntable.h"

  std::string PatternTable::store() { 
    std::stringstream os;
    boost::archive::text_oarchive oa(os);
    oa << BOOST_SERIALIZATION_NVP(*this);
    return os.str();
  };


  void PatternTable::load(const std::string& s) {
    std::stringstream is;
    is.str(s);
    boost::archive::text_iarchive ia(is);
    ia >> BOOST_SERIALIZATION_NVP(*this);
  }

  pmI PatternTable::insertPattern(const std::string& pattern, bool& success) {
    PatternMeta m = PatternMeta(pattern);
    a.insert(m); 
    //success = x.second; 
    return a.end();
  }

  

  void PatternTable::debug() { 
    for (pmI x = a.begin(); x != a.end(); x++) { 
      std::cout   << "pattern: " << x->pattern << ", entry: " << x->entry <<  ", exit: " << x->exit << std::endl; 
    }
  }

