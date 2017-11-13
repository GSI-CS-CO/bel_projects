#include "grouptable.h"

  std::string GroupTable::store() { 
    std::stringstream os;
    boost::archive::text_oarchive oa(os);
    oa << BOOST_SERIALIZATION_NVP(*this);
    return os.str();
  };


  void GroupTable::load(const std::string& s) {
    std::stringstream is;
    is.str(s);
    boost::archive::text_iarchive ia(is);
    ia >> BOOST_SERIALIZATION_NVP(*this);
  }

  bool GroupTable::insert(const std::string& sNode) {
    GroupMeta m = GroupMeta(sNode);
    auto x = a.insert(m); 
    return x.second; 
  }

  void GroupTable::debug() { 
    for (pmI x = a.begin(); x != a.end(); x++) { 
      std::cout << x->node << " -> Pattern: " << x->pattern <<  ", Entry: " <<  (int)x->patternEntry <<  ", Exit: " <<  (int)x->patternExit;
      std::cout << " -> Beamproc: " << x->beamproc <<  ", Entry: " <<  (int)x->beamprocEntry <<  ", Exit: " <<  (int)x->beamprocExit << std::endl;
       
    }
  }

  pmI GroupTable::lookupOrCreateNode(const std::string& sNode) {
    //std::cout << "Looking for node..." << sNode;
    pmRange x = lookup<Groups::Node>(sNode);
    if (!isOk(x.first)) { //node was not found, try to create
      //std::cout << "not found. Creating..." << sNode;
      GroupMeta m = GroupMeta(sNode);
      auto y = a.insert(m); 
      if (y.second) {x.first = y.first;} else {throw std::runtime_error( "Failed to manage group memberships of Node '" + sNode + "', node does not exist and could not be created");}
    }
    //std::cout << " done" << std::endl;
    return x.first;
  }


  