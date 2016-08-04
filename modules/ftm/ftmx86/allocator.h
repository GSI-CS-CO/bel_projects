#ifndef _ALLOCATOR_H_
#define _ALLOCATOR_H_

#include <stdint.h>

typedef struct t_block {
   uint32_t adr;
   uint32_t size;
   struct t_block *next;
   struct t_block *prev;  
} t_block;

t_block* getFirst(t_block* l);
t_block* getLast(t_block* l);

t_block* rem(t_block* pRem);
t_block* add_after(t_block* pDst, t_block* pAdd);
t_block* add_before(t_block* pDst, t_block* pAdd);
void swap(t_block* pA, t_block* pB);

t_block* sortByAdr(t_block* pList);
t_block* sortBySize(t_block* pList);
t_block* bubblesort(t_block* pList, uint32_t (*getValue)());

t_block* createBlockList(uint8_t* buf, t_block* pNewList);
t_block* createFreeList(t_block* pBlockList, t_block* pFreeList, uint32_t ramsize);

t_block* freeBlock(t_block* pBlock, t_block** pBlockList, t_block* pFreeList);
t_block* getBestFit(uint32_t size, t_block* pFreeList);
t_block*  allocateBlock(uint32_t size, t_block** pFreeList, t_block* pBlockList, t_block* getFit() );
t_block* moveBlockBetweenLists(t_block* pSrc, t_block** pSrcList, t_block* pDstList);

void showfull(t_block* c);
void show(t_block* c);
void showList(t_block* l);
int  showFrag(t_block* pFreeList, int ramsize);
void showAll(t_block* pF, t_block* pB, int ramsize);

uint8_t createLBT(uint8_t* buf, t_block* pBlockList);

#endif
