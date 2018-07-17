#include "grouptable.h"

  std::string GroupTable::store() { 
    std::stringstream os;
    boost::archive::text_oarchive oa(os);
    oa << BOOST_SERIALIZATION_NVP(*this);
    return os.str();
  };


  void GroupTable::load(const std::string& s) {
    std::stringstream is;
    is.str(fixArchiveVersion(s));
    boost::archive::text_iarchive ia(is);
    ia >> BOOST_SERIALIZATION_NVP(*this);
  }

  bool GroupTable::insert(const std::string& sNode) {
    GroupMeta m = GroupMeta(sNode);
    auto x = a.insert(m); 
    return x.second; 
  }

  void GroupTable::debug(std::ostream& os) { 
    for (pmI x = a.begin(); x != a.end(); x++) { 
      os << x->node << " -> Pattern: " << x->pattern <<  ", Entry: " <<  (int)x->patternEntry <<  ", Exit: " <<  (int)x->patternExit;
      os << " -> Beamproc: " << x->beamproc <<  ", Entry: " <<  (int)x->beamprocEntry <<  ", Exit: " <<  (int)x->beamprocExit << std::endl;
       
    }
  }

  pmI GroupTable::lookupOrCreateNode(const std::string& sNode) {
    auto x  = a.get<Groups::Node>().equal_range(sNode); 
    if (x.first == x.second) { //node was not found, try to create
      GroupMeta m = GroupMeta(sNode);
      auto y = a.insert(m);  // returns pair < iterator it, bool success>
      if (y.second) {x.first = y.first;} else {throw std::runtime_error( "Failed to manage group memberships of Node '" + sNode + "', node does not exist and could not be created");}
      
    }
    return x.first;
  }

  vStrC GroupTable::getAllPatterns() {
    std::set<std::string> st;
    vStrC ret; // bad type really, but a little workaround with a set does it
    for (auto& it : a.get<Groups::Pattern>()) {
      st.insert(it.pattern);
    }
    for (auto& it : st) ret.push_back(it);

    return ret;

  }

  