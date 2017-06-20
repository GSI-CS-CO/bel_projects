class CarpeDM {


protected:

  std::string ebdevname;
  std::string outputfilename;
  
  std::string  inputfilename;
  boost::dynamic_properties dp(boost::ignore_other_properties);
  std::vector<MemUnit> vM;
  std::vector<Graph> vUp;

  Socket ebs;
  Device ebd;  
  std::vector<struct sdb_device> myDevs;  

  int cpuQty;

  
  


public:
  CarpeDM() cpuQty(-1),  {
    dp.property("type",     boost::get(&myEdge::type,       g));
    dp.property("node_id",  boost::get(&myVertex::name,     g));
    dp.property("type",     boost::get(&myVertex::type,     g));
    dp.property("flags",    boost::get(&myVertex::flags,    g));
    dp.property("tPeriod",  boost::get(&myVertex::tPeriod,  g));
    dp.property("tOffs",    boost::get(&myVertex::tOffs,    g));
    dp.property("id",       boost::get(&myVertex::id,       g));
    dp.property("par",      boost::get(&myVertex::par,      g));
    dp.property("tef",      boost::get(&myVertex::tef,      g));
    dp.property("res",      boost::get(&myVertex::res,      g));
    dp.property("tValid",   boost::get(&myVertex::tValid,   g));
    dp.property("prio",     boost::get(&myVertex::prio,     g));
    dp.property("qty",      boost::get(&myVertex::qty,      g));
    dp.property("tWait",    boost::get(&myVertex::tWait,    g));

  } 

   ~CarpeDM() {};


  bool connect(const std::string& en) {
    ebdevname = std::string(en); //copy to avoid mem trouble later
    bool  ret = false;

    try { 
      ebs.open(0, EB_DATAX|EB_ADDRX);
      ebd.open(ebs, netaddress, EB_DATAX|EB_ADDRX, 3);
      ebd.sdb_find_by_identity(0x0000000000000651ULL,0x54111351, myDevs);
      if (myDevs.size() >= 1) { cpuQty = myDevs.size(); ret = true;}
    } catch(...) {
      //TODO report why we could not connect / find CPUs
    }
    return ret;

  }

  bool disconnect() {
    bool ret = false;
    try { 
      ebd.close();
      ebs.close();
      cpuQty = -1;
      ret = true;
    } catch(...) {
      //TODO report why we could not disconnect
    }
    return ret;
  }

  void addDotToDict(const std::string& fn) {
    Graph g;
    std::ifstream in(fn);

    if(in.good()) {
      try { boost::read_graphviz(in,g,dp,"node_id") }
      catch(...) {
        throw;
        //TODO report why parsing the dot / creating the graph failed
      }
      in.close();
    }  
    else {std::cerr << program << " Could not open .dot file <" << fn << ">!" << std::endl; return;}  
    
  
    //add to dictionary
    try {
      BOOST_FOREACH( vertex_t v, vertices(g) ) { hm.add(g[v].name); }
    }  catch (...) {
      //TODO report hash collision and show which names are responsible
      throw;
    }
  }

  void clearDict() {
    hm.clear();
  }

  Graph& g parseUpDot(const std::string& fn, Graph& g) {
 
    std::ifstream in(fn);

    if(in.good()) {
      try { boost::read_graphviz(in,g,dp,"node_id") }
      catch(...) {
        throw;

      }
      in.close();
    }  
    else {std::cerr << program << " Could not open .dot file <" << fn << "> for reading!" << std::endl; return g;}  
    //format all graph labels lowercase
    boost::graph_traits<Graph>::vertex_iterator vi, vi_end;
    boost::tie(vi, vi_end) = vertices(g);   
    BOOST_FOREACH( edge_t e, edges(g) ) { std::transform(g[e].type.begin(), g[e].type.end(), g[e].type.begin(), ::tolower); }
    BOOST_FOREACH( vertex_t v, vertices(g) ) { std::transform(g[v].type.begin(), g[v].type.end(), g[v].type.begin(), ::tolower); } 


    //TODO create subgraphs as necessary

    //TODO automatically add necessary meta nodes

    return g;

  }

     //TODO NC analysis

    //TODO assign to CPUs/threads






  const void writeDownDot(const std::string& fn, MemUnit& m) {
    std::ofstream out(fn); 
    if(out.good()) {
      try { boost::write_graphviz(out, m.getDownGraph(), make_vertex_writer(boost::get(&myVertex::np, m.getDownGraph())), 
                          make_edge_writer(boost::get(&myEdge::type, m.getDownGraph())), sample_graph_writer{"Demo"},
                          boost::get(&myVertex::name, m.getDownGraph()));
      }
      catch(...) {
        throw;

      }
      out.close();
    }  
    else {std::cerr << program << " Could not write to .dot file <" << fn << "> !" << std::endl; return;} 

    
  }




  
};
