#include "memunit.h"
#include "common.h"



  void MemUnit::initMemPool() { 
    memPool.clear();
    std::cout << "extBaseAdr = 0x" << std::hex << extBaseAdr << " intBaseAdr = 0x" << intBaseAdr << ", Poolsize = " << std::dec << poolSize 
    << ", bmpLen = " << bmpLen << ", startOffs = 0x" << std::hex << startOffs << ", endOffs = 0x" << std::hex << endOffs << ", vBufSize = " 
    << std::dec << uploadBmp.size() << std::endl;
    for(uint32_t adr = startOffs; adr < endOffs; adr += _MEM_BLOCK_SIZE) { 
      //Never issue <baseAddress - (baseAddress + bmpLen -1) >, as this is where Mgmt bitmap vector resides     
      //std::cout << std::hex << adr << std::endl; 
      memPool.insert(adr); 
    }
  }

  bool MemUnit::acquireChunk(uint32_t &adr) {
    bool ret = true;
    if ( memPool.empty() ) {
      ret = false;
    } else {
      adr = *(memPool.begin());
      memPool.erase(adr);
    }
    return ret;
  }


  bool MemUnit::freeChunk(uint32_t &adr) {
    bool ret = true;
    if ((adr % _MEM_BLOCK_SIZE) || (memPool.count(adr) > 0))  {ret = false;} //unaligned or attempted double entry, throw exception
    else memPool.insert(adr);
    return ret;
  }        

  void MemUnit::createUploadBmp() {
    for (auto& it : uploadBmp) { 
      it = 0;
    }    

    //Go through allocmap and update Bmp
    for (auto& it : allocMap) {
      if( (it.second.adr >= startOffs) && (it.second.adr < endOffs)) {
        int bitIdx = (it.second.adr) / _MEM_BLOCK_SIZE;
        uint8_t tmp = 1 << (bitIdx % 8);
        printf("Bidx = %u, bufIdx = %u, val = %x\n", bitIdx, bitIdx / 8 , tmp);
        
        uploadBmp[bitIdx / 8] |= tmp;
      } else {//something's awfully wrong, address out of scope!
        std::cout << "Address 0x" << std::hex << it.second.adr << " is not within 0x" << std::hex << startOffs << "-" << std::hex << endOffs << std::endl;
      }
    }
    
  }

 

  vAdr MemUnit::getUploadAdrs() {
    vAdr ret;

    for (auto& it : allocMap) {
      for (uint32_t adr = adr2extAdr(it.second.adr); adr < adr2extAdr(it.second.adr) + _MEM_BLOCK_SIZE; adr += _32b_SIZE_ ) ret.push_back(adr);
    }    
    return ret;
  }

  vBuf MemUnit::getUploadData() {
    vBuf ret;
    ret.reserve( uploadBmp.size() + allocMap.size() * _MEM_BLOCK_SIZE ); // preallocate memory for BMP and all Nodes
    
    ret.insert( ret.end(), uploadBmp.begin(), uploadBmp.end() );
    for (auto& it : allocMap) { 
      ret.insert( ret.end(), it.second.b, it.second.b + _MEM_BLOCK_SIZE );
    }  
    return ret;
  }

  vAdr getDownloadAdrs();


  void parseDownloadData();
  //dlAlloc: adr -> vertex_desc, hash, buffer

  // creating nodes
  // iterate over (big) eb download buffer (data blocks):
    //obtain key by converting from extAdr (download Address) to adr value, create dlAlloc entry 
    //copy data block to dlAlloc entry buffer
    //convert all intAdr values to adr values
    //create node according to type field, assign all parsed data and name for given hash
    //add vertex descriptor to dlAlloc entry
  //creating edges
    //iterate over dlAlloc map
    //get vertex descriptor, this will be the parent.
    //call parser function matching node type on dlAlloc entry buffer
    //for each address ...
      //lookup vertex descriptor, this will be the child.
      //create edge to child. Edge type property must match link use within node (eg. defDst, altDst, target, etc)
    



  vChunk MemUnit::getAllChunks() const {
    vChunk ret;
    for (auto& it : allocMap) {
      ret.push_back((chunkMeta*)(&it.second));
    }    

    return ret;
  }



  /*
void MemUnit::prepareUpload() {

      //check allocation
      //go through graph
      //call allocate    

    //update BMP

for (itBuf it = uploadBmp.begin(); it < uploadBmp.end(); it++) {
      for (int i=0; i <8; i++) {
        if ((*it) & (1<<i)) {
          adr = extBaseAdr + int(uploadBmp.end() - it)*8 + i * _MEM_BLOCK_SIZE;
          ret.push_back(adr);
          std::cout << "0x" << std::hex << adr << std::endl;
        }
      }
    }   

    //serialise
      //go through graph
      //call serialise   
  }


  void MemUnit::upload() {
      
eb_status_t ftmRamWrite()
{
   eb_status_t status;
   eb_cycle_t cycle;
   uint32_t i,j, packets, partLen, start, data;
   uint32_t* writeout = (uint32_t*)buf;   
   
   boost::container::vector<uint32_t> vpChunkMeta;chunkMeta*>

   //wrap frame buffer in EB packet
   packets = ((getUsedSpace() + PACKET_SIZE-1) / PACKET_SIZE);
   start = 0;
   
   for(j=0; j < packets; j++)
   {
      if(j == parts-1 && (len % PACKET_SIZE != 0)) partLen = len % PACKET_SIZE;
      else partLen = PACKET_SIZE;
      
      if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return die(status, "failed to create cycle"); 
      
      for(i= start>>2; i< (start + partLen) >>2;i++)  
      {
         if (bufEndian == LITTLE_ENDIAN)  data = SWAP_4(writeout[i]);
         else                             data = writeout[i];
         
         eb_cycle_write(cycle, (eb_address_t)(address+(i<<2)), EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)data); 
      }
      if ((status = eb_cycle_close(cycle)) != EB_OK) return die(status, "failed to close write cycle");
      start = start + partLen;
   }
   
   return 0;
}
    //split all allocmap elements marked for upload (transfer = true) into network packets ( div 38)

      //play each entry's buffer as eb operations
      //send cycle

    //play BMP as eb operations
    //send cycle

    
    
  }
*/


  //Allocation functions
  bool MemUnit::allocate(const std::string& name) {
    uint32_t chunkAdr, hash;
    bool ret = insertHash(name, hash);
    if ( (allocMap.count(name) == 0) && acquireChunk(chunkAdr) ) { 
      allocMap[name] = (chunkMeta) {chunkAdr, hash};  
    } else {ret = false;}
    return ret;
  }

  bool MemUnit::insert(const std::string& name, uint32_t adr) {return true;}

  bool MemUnit::deallocate(const std::string& name) {
    bool ret = true;
    if ( (allocMap.count(name) > 0) && freeChunk(allocMap.at(name).adr) ) { allocMap.erase(name); 
    } else {ret = false;}
    return ret;
  }

  chunkMeta* MemUnit::lookupName(const std::string& name) const  {
    if (allocMap.count(name) > 0) { return (chunkMeta*)&(allocMap.at(name));} 
    else {return NULL;}
  }

  //Hash functions

  bool MemUnit::insertHash(const std::string& name, uint32_t &hash) {
    hash = FnvHash::fnvHash(name.c_str());

    if (hashMap.left.count(hash) > 0) return false;
    else hashMap.insert( hashValue(hash, name) );
    return true;
  }

  bool MemUnit::removeHash(const uint32_t hash) {
    if (hashMap.left.count(hash) > 0) {hashMap.left.erase(hash); return true;}
    return false;
  }
  
  



