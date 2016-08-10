#ifndef _ALLOCATOR_H_
#define _ALLOCATOR_H_

#include <stdint.h>
#include "../ftm_common.h"


/*
typedef struct t_lbt_entry {
   uint32_t adr; 
   uint32_t size;
} t_lbt_entry;

typedef struct t_lbt {
  uint32_t bmp;
  t_lbt_entry lb[32];
} t_lbt;
*/

typedef struct t_block {
   uint32_t adr;
   uint32_t size;
   uint32_t idx; 
   struct t_block *next;
   struct t_block *prev;  
} t_block;

t_block* getHead(t_block* l);
t_block* getTail(t_block* l);

void dest(t_block* pRem);
t_block* rem(t_block* pRem);
t_block* addAfter(t_block* pDst, t_block* pAdd);
t_block* addBefore(t_block* pDst, t_block* pAdd);
void swap(t_block* pA, t_block* pB);

t_block* sortByAdr(t_block* pList);
t_block* sortBySize(t_block* pList);
t_block* bubblesort(t_block* pList, uint32_t (*getValue)());

int blockList2tab(uint32_t* tab, t_block* pBlockList);
int tab2BlockList(uint32_t* tab, t_block** pBlockList, t_block** pFreeList, uint32_t ramsize, uint32_t offset);
int freeBlockInTab(t_block* pBlock, t_block** pBlockList, t_block** pFreeList, uint32_t* tab);
t_block* allocateBlockInTab(uint32_t size, t_block** pFreeList, t_block** pBlockList, t_block* getFit(), uint32_t* tab);


t_block* createBlockList(uint8_t* buf, t_block* pNewList);
t_block* createFreeList(t_block* pBlockList, uint32_t ramsize,uint32_t offset);

int freeBlock(t_block* pBlock, t_block** pBlockList, t_block** pFreeList);
t_block* getBestFit(uint32_t size, t_block** pFreeList);
t_block* allocateBlock(uint32_t size, t_block** pFreeList, t_block** pBlockList, t_block* getFit() );
t_block* moveBlockBetweenLists(t_block* pSrc, t_block** pSrcList, t_block** pDstList);

void showfull(t_block* c);
void show(t_block* c);
void showTab(uint32_t* tab);
void showList(t_block* l);
int  getCont(t_block* pFreeList);
int  getUsage(t_block* pFreeList);
void showAll(t_block* pF, t_block* pB, int ramsize);
void showAllWithTab(t_block* pF, t_block* pB, int ramsize, uint32_t* tab);

uint8_t createLBT(uint8_t* buf, t_block* pBlockList);

#endif
