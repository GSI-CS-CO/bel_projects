#include <stdio.h>
#include <iostream>
#include <inttypes.h>
#include <boost/bimap.hpp>
#include <set>
#include <boost/lockfree/queue.hpp>


#define min(a, b) (((a) < (b)) ? (a) : (b))



struct name  {};
struct adr       {};

 using namespace boost::bimaps;

    // Soccer World cup.

    typedef bimap
    <
        tagged< std::string, name >,
        tagged< uint32_t        , adr   >

    > alloc_bimap;

    typedef alloc_bimap::value_type elem;



    alloc_bimap abm;
    abm.insert( elem("Argentina"    ,1) );
    abm.insert( elem("Spain"        ,2) );

    std::cout << "Countries names ordered by their final position:"
                << std::endl;

    for( results_bimap::map_by<name>::const_iterator
            i    = results.by<name>().begin(),
            iend = results.by<name>().end() ;
            i != iend; ++i )
    {
        std::cout << i->get<name  >() << ") "
                  << i->get<adr>() << std::endl;
    }

    std::cout << std::endl
              << "Countries names ordered alfabetically along with"
                 "their final position:"
              << std::endl;

    for( results_bimap::map_by<adr>::const_iterator
            i    = results.by<adr>().begin(),
            iend = results.by<adr>().end() ;
            i != iend; ++i )
    {
        std::cout << i->get<adr>() << " ends "
                  << i->get<name  >() << "º"
                  << std::endl;
    }



  #define MEM_BLOCK_SIZE  32
  #define MEM_TOTAL_SIZE  8192
  #define MAX_IDX         (MEM_TOTAL_SIZE / MEM_BLOCK_SIZE)

  std::set<uint32_t> memPool;
  std::map<std::string, myData> allocMap;
typedef std::map< std::string, myData >::iterator itMap ;


  void updatememPoolFromBmp32(uint32_t bmp, int bmpNo) {
    for(uint32_t i=0; i < 32; i++) {
      if((bmp >> i) & 1) memPool.insert((i + bmpNo * 32) * MEM_BLOCK_SIZE);
    }
  }

 
  void initMemPool() { 
    memPool.clear();
    for(uint32_t i=0; i < MAX_IDX; i++) {std::cout << i * MEM_BLOCK_SIZE << std::endl; memPool.insert(i * MEM_BLOCK_SIZE); }
  }

    
   bool acquireChunk(uint32_t &adr) {
    bool ret = true;
    if ( memPool.empty() ) {
      ret = false;
    } else {
      adr = *(memPool.begin());
      memPool.erase(adr);
    }
    return ret;
  }

  bool freeChunk(uint32_t adr) {
    bool ret = true;
    if ((adr % MEM_BLOCK_SIZE) || (memPool.count(adr) > 0))  {ret = false;} //unaligned or attempted double entry, throw exception
    else memPool.insert(adr);
    return ret;
  }        

  bool allocate(const std::string& name) {
    uint32_t chunkAdr;
    bool ret = true;
    if ( (allocMap.count(name) == 0) && acquireChunk(chunkAdr) ) { allocMap[name] = (myData) {chunkAdr}; 
    } else {ret = false;}
    return ret;
  }

  bool deallocate(const std::string& name) {
    bool ret = true;
    if ( (allocMap.count(name) > 0) && freeChunk(allocMap[name].adr) ) { allocMap.erase(name); 
    } else {ret = false;}
    return ret;
  }

  bool lookup(const std::string& name, myData& m) {
    bool ret = true;
    if (allocMap.count(name) > 0) { m = allocMap[name]; 
    } else {ret = false;}
    return ret;
  }

  bool lookupAdr(const std::string& name, uint32_t& adr) {
    myData m;
    bool ret = lookup(name, m);
    adr = m.adr;
    return ret;
  }

int main() {

  std::map<std::string, std::string> mm;
    uint32_t testIdx0, testIdx1, testIdx2, testIdx;

  mm["Matze"] = "Held";
  
  std::cout << mm["Matze"] << std::endl;


  if (acquireChunk(testIdx0)) std::cout << "Got Idx  " << testIdx0 << " from emtpy pool" << std::endl;
  else std::cout << "Could not get idx, pool empty " << std::endl;

  initMemPool();



  acquireChunk(testIdx0);
  acquireChunk(testIdx1);

  std::cout << "Got Idx " << testIdx0 << " and " << testIdx1 << std::endl;

  acquireChunk(testIdx2);

  std::cout << "Also got Idx " << testIdx2 << std::endl; 

  if (freeChunk(testIdx1)) std::cout << "Returned " << testIdx1 << std::endl;
  else std::cout << "Failed to return " << testIdx1 << std::endl;

  if (freeChunk(testIdx1)) std::cout << "Returned " << testIdx1 << " 2x times !!!" << std::endl;
  else std::cout << "Failed to return " << testIdx1 << " the second time" << std::endl;

  acquireChunk(testIdx1);

  std::cout << "Got New Idx " << testIdx1 << std::endl;
/*
  memPool.clear();

  updatememPoolFromBmp32(0b0110101, 0);
  updatememPoolFromBmp32(0b0110101, 1);
*/
  acquireChunk(testIdx0);
  acquireChunk(testIdx1);

  std::cout << "Prepped Pool - Got Idx " << testIdx0 << " and " << testIdx1 << std::endl;

  acquireChunk(testIdx2);

  std::cout << "Also got Idx " << testIdx2 << std::endl; 

  if (freeChunk(testIdx1)) std::cout << "Returned " << testIdx1 << std::endl;
  else std::cout << "Failed to return " << testIdx1 << std::endl;

  if (freeChunk(testIdx1)) std::cout << "Returned " << testIdx1 << " 2x times !!!" << std::endl;
  else std::cout << "Failed to return " << testIdx1 << " the second time" << std::endl;

  acquireChunk(testIdx1);

  std::cout << "Got New Idx " << testIdx1 << std::endl;

  allocate("Matze");
  allocate("DerDa");
  allocate("DieDa");

  //memPool.clear();

  allocate("DasDa");



  for( itMap iM = allocMap.begin() ; iM != allocMap.end() ; ++iM )
  {
    std::cout << "Allocated " <<  (*iM).first <<  " at adr " << (*iM).second.adr << std::endl;
  }

  deallocate("Matze");

  uint32_t adr;

  if (lookupAdr("Matze", adr)) std::cout << "Got Adr " << adr  << std::endl;
  else std::cout << "Lookup failed " << std::endl;

  if (lookupAdr("DieDa", adr)) std::cout << "Got Adr " << adr  << std::endl;
  else std::cout << "Lookup failed " << std::endl;


  return 0;	
}





