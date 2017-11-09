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
      std::cout << " -> BeamProc: " << x->beamproc <<  ", Entry: " <<  (int)x->beamprocEntry <<  ", Exit: " <<  (int)x->beamprocExit << std::endl;
       
    }
  }

  pmI GroupTable::lookUpOrCreateNode(const std::string& sNode) {
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

  vStrC GroupTable::getPatternEntryPoints(const std::string& sPattern) {
    std::cout << "Searching Entries for Pattern " << sPattern << std::endl;
    vStrC res;
    pmRange x  = lookup<Groups::Pattern>(sPattern);
    for (auto it = x.first; it != x.second; ++it) if (it->patternEntry) res.push_back(it->node);
    return res;
  }

  vStrC GroupTable::getPatternExitPoints(const std::string& sPattern) {
    std::cout << "Searching Exits for Pattern " << sPattern << std::endl;
    vStrC res;
    pmRange x  = lookup<Groups::Pattern>(sPattern);
    for (auto it = x.first; it != x.second; ++it) if (it->patternExit) res.push_back(it->node);
    return res;
  }

  vStrC GroupTable::getBeamProcEntryPoints(const std::string& sBeamproc) {
    std::cout << "Searching Entries for beamproc " << sBeamproc << std::endl;
    vStrC res;
    pmRange x  = lookup<Groups::BeamProc>(sBeamproc);
    for (auto it = x.first; it != x.second; ++it) if (it->beamprocEntry) res.push_back(it->node);
    return res;
  }

  vStrC GroupTable::getBeamProcExitPoints(const std::string& sBeamproc) {
    std::cout << "Searching Exits for Pattern " << sBeamproc << std::endl;
    vStrC res;
    pmRange x  = lookup<Groups::BeamProc>(sBeamproc);
    for (auto it = x.first; it != x.second; ++it) if (it->beamprocExit) res.push_back(it->node);
    return res;
  }
