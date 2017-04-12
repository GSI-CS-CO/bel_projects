#include <stdio.h>
#include <iostream>
#include <inttypes.h>

#include "memunit.h"


int main() {

  Graph g;

  MemUnit mmu = MemUnit(1, 0x1000, 8192, g);

  if (mmu.allocate("Matze")) {std::cout << "Allocation successful" << std::endl;}
  else {std::cout << "Allocation FAILED" << std::endl;}
  if (mmu.allocate("Er")) {std::cout << "Allocation successful" << std::endl;}
  else {std::cout << "Allocation FAILED" << std::endl;}
  if (mmu.allocate("Sie")) {std::cout << "Allocation successful" << std::endl;}
  else {std::cout << "Allocation FAILED" << std::endl;}
  if (mmu.allocate("Es")) {std::cout << "Allocation successful" << std::endl;}
  else {std::cout << "Allocation FAILED" << std::endl;}

  chunkMeta* testme;
  mmu.lookupName2Chunk("Matze", testme);
  testme->buf.push_back('a');
  testme->buf.push_back('b');
  testme->buf.push_back('c');

  mmu.lookupName2Chunk("Er", testme);
  testme->buf.push_back('d');
  testme->buf.push_back('e');

  mmu.lookupName2Chunk("Sie", testme);
  testme->buf.push_back('h');

  mmu.lookupName2Chunk("Es", testme);
  testme->buf.push_back('i');
  testme->buf.push_back('j');
  testme->buf.push_back('k');
  testme->buf.push_back('l');

  

  mmu.deallocate("Sie");



  for (itAm it = mmu.allocMap.begin(); it != mmu.allocMap.end(); it++) {
    std::cout << "Name: " <<  it->first << " Adr: 0x" << std::hex << it->second.adr << " Buf: ";
    for (itBuf itb = it->second.buf.begin(); itb < it->second.buf.end(); itb++) {
      std::cout << *itb;
    }
    
    std::cout << std::endl;
  }
  std::cout << std::endl;
 

  for (itHm it = mmu.hashMap.begin(); it != mmu.hashMap.end(); it++) {
    std::cout << "Hash: 0x" << std::hex << it->first << " Name: " << it->second << std::endl;
  }
  std::cout << std::endl;
 
  mmu.updateBmpFromAlloc();
  
  
  for (itBuf it = mmu.mgmtBmp.begin(); it < mmu.mgmtBmp.end(); it++) {
    printf(BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(*it));
  }
  printf("\n");

  mmu.showAdrsFromBmp();

  return 0;	
}





