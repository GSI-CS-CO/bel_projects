#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "allocator.h"

static uint32_t getAdr(t_block* c) {
  return c->adr;
}

static uint32_t getSize(t_block* c) {
  return c->size;
}

static int isInList(t_block* c, t_block* pList) {
  t_block* t; 
  if ((c != NULL) && (pList != NULL)) {
    t = getHead(pList);
    while (t != NULL) {
      if (t == c) return 1;
      t = t->next;
    }
  }
  return 0;  
} 


t_block* getHead(t_block* l) {
  if(l != NULL) while (l->prev != NULL) l = l->prev;
  return l;
}

t_block* getTail(t_block* l) {
  if(l != NULL) while (l->next != NULL) l = l->next;
  return l;
}

t_block* rem(t_block* pRem) {
       
  if (pRem != NULL) {
    if (pRem->next != NULL) pRem->next->prev = pRem->prev;
    if (pRem->prev != NULL) pRem->prev->next = pRem->next;
    pRem->next = NULL;
    pRem->prev = NULL;
  }
  return pRem;
}

void dest(t_block* pRem) {
  free(rem(pRem));
}

t_block* addAfter(t_block* pDst, t_block* pAdd) {
  
  if (pDst != NULL) {
    if (pAdd != NULL) {
       
      pAdd->next = pDst->next;
      pAdd->prev = pDst;
    }
    if (pDst->next != NULL) pDst->next->prev = pAdd;
    pDst->next = pAdd;
  } else {pDst = pAdd; printf("Created new list\n");}
  return pDst;
}

t_block* addBefore(t_block* pDst, t_block* pAdd)  {
  if (pDst != NULL) {
    if (pAdd != NULL) {
      pAdd->next = pDst;
      pAdd->prev = pDst->prev;
    }
    if (pDst->prev != NULL) pDst->prev->next = pAdd;
    pDst->prev = pAdd;
  } else {pDst = pAdd; printf("Created new list\n");}
  return pDst;
}

void swap(t_block* pA, t_block* pB) {
  if ((pA != NULL) && (pB != NULL)) {
    addAfter(pB, rem(pA));
    
  }
}

t_block* sortByAdr(t_block* pList) {
  return bubblesort(pList, (void*)&getAdr);
}

t_block* sortBySize(t_block* pList) {
 return bubblesort(pList, (void*)&getSize);
}

t_block* bubblesort(t_block* pList, uint32_t (*getValue)()) {
  t_block* c = getHead(pList);  
  int sorting = 1;
  if (pList != NULL) {
    while (sorting) {
      sorting = 0;
      while (c->next != NULL) {
        if (getValue(c) > getValue(c->next)) {swap(c, c->next); sorting = 1; }
        else { c = c->next; }
      }
      c = getHead(c); //reset to list head the ugly way
    }
  }
  return pList; 
}

int tab2BlockList(uint32_t* tab, t_block** pBlockList,  t_block** pFreeList, uint32_t ramsize, uint32_t offset) {

  //int e = 0;
  uint32_t cnt;

  //init free list

  for (cnt=0; cnt < 32; cnt++) {
     
    printf("#%u  b: %u a: %u s: %u\n", cnt, ((tab[(LBT_BMP)>>2] >> cnt) & 1), tab[(LBT_TAB + cnt * _LB_SIZE_ + LB_PTR)>>2], tab[(LBT_TAB + cnt * _LB_SIZE_ + LB_SIZE)>>2]);
    
    if ((tab[(LBT_BMP)>>2] >> cnt) & 1) {
      t_block* pAdd = (t_block*)malloc(sizeof(t_block));
      if (pAdd != NULL) {
        pAdd->adr   = tab[(LBT_TAB + cnt * _LB_SIZE_ + LB_PTR)>>2];
        pAdd->size  = tab[(LBT_TAB + cnt * _LB_SIZE_ + LB_SIZE)>>2];
        pAdd->idx   = cnt;
        *pBlockList = addAfter(*pBlockList, pAdd);
      } else return -1;
    }

  }
  
  *pFreeList = createFreeList(*pBlockList, ramsize, offset);

  return 0;

}



t_block* createFreeList(t_block* pBlockList, uint32_t ramsize, uint32_t offset) {
  t_block* pF = NULL;  

  printf("Creating Free List\n"); 

  //if Block list is empty, create 1 element free list containing whole memory
  if(pBlockList == NULL) {
    t_block* pAdd = (t_block*)malloc(sizeof(t_block));
    if (pAdd != NULL) {
      pAdd->adr   = 0;
      pAdd->size  = ramsize;
      addAfter(pF, pAdd);
    }
  }
  
  pBlockList = getHead(sortByAdr(pBlockList));
  int32_t diff;
  t_block* c = pBlockList;


  

  //create first block
  if (c->adr > offset) {
    printf("Gap at start\n"); 
    t_block* pAdd = (t_block*)malloc(sizeof(t_block));
    if (pAdd != NULL) {
      pAdd->adr   = offset;
      pAdd->size  = c->adr;
      pF = addAfter(pF, pAdd);
      pF = getTail(pF);
    } else {printf("CreateFreeList: Couldn't alloc new free element\n");}
  }
  

  showList(c);

  while (c != NULL ) {
    if (c->next != NULL) {
      show(c);
      show(c->next);
      diff = c->next->adr - (c->adr + c->size);
      printf("diff: %d", diff);
      if (diff > 0) {
        printf("Gap mid\n"); 
        t_block* pAdd = (t_block*)malloc(sizeof(t_block));
        if (pAdd != NULL) {
          pAdd->adr   = c->adr + c->size;
          pAdd->size  = diff;
          pF = addAfter(pF, pAdd);
          pF = getTail(pF);
          printf(".*..\n");
        } else {printf("CreateFreeList: Couldn't alloc new free element\n");}
      }
    } else {
      //create last block
      printf("Doing Last Free\n"); 
      diff = ramsize - (c->adr + c->size);  
      if (diff > 0) {
        printf("Gap end\n"); 
        t_block* pAdd = (t_block*)malloc(sizeof(t_block));
        if (pAdd != NULL) {
          pAdd->adr   = c->adr + c->size;
          pAdd->size  = diff;
          printf("..\n"); 
          pF = addAfter(pF, pAdd);
          printf("...\n"); 
          pF = getTail(pF);
          printf(".!..\n");
        } else {printf("CreateFreeList: Couldn't alloc new free element\n");}
      }
    }
    c = c->next;
    printf(".....\n");
  }
  printf("Done\n"); 
  return getHead(pF);        
}



int blockList2tab(uint32_t* tab, t_block* pBlockList) {
  
  uint32_t i, cntS = 0, cntB = 0;
  t_block* pB = getHead(pBlockList);
  
  //check space in tab bitmap (the dumb way)
  for (i=0;i<32;i++) cntS += (~(tab[(LBT_BMP)>>2] >> i) & 1);
  //check number of blocks
  while (pB != NULL) {cntB++; pB = pB->next;}

  if (cntB > cntS) {printf("Not enough space in Tab, needed %u, got %u\n", cntB, cntS); return -1;}

  
  //find space

  pB = getHead(pBlockList);
  for (i=0;i<32;i++) {
    if (pB == NULL) break;
    printf("+# %2u %u\n", i, (!((tab[(LBT_BMP)>>2] >> i) & 1)));
    if (!((tab[LBT_BMP] >> i) & 1)) {
      tab[(LBT_BMP)>>2]                          |= 1<<i;
      tab[(LBT_TAB + i * _LB_SIZE_ + LB_PTR)>>2]  = pB->adr;
      tab[(LBT_TAB + i * _LB_SIZE_ + LB_SIZE)>>2] = pB->size;
      pB = pB->next;    
    }
  }
  return cntB;
}


t_block* allocateBlockInTab(uint32_t size, t_block** pFreeList, t_block** pBlockList, t_block* getFit(), uint32_t* tab) {

  int i;
  printf("WTF %p, %08x\n", tab, tab[0]);
  t_block* pBlock = allocateBlock(size, pFreeList, pBlockList, getFit );
  
  if (pBlock != NULL) {
  

    //find space
    for (i=0;i<32;i++) {
      if (!((tab[(LBT_BMP)>>2] >> i) & 1)) {
        printf("Idx: %u, tab bmp: %p, tab b: %p, tab c: %p\n", i, (uint32_t*)&tab[LBT_BMP], (uint32_t*)&tab[LBT_TAB], (uint32_t*)&tab[LBT_TAB + i * _LB_SIZE_ + LB_PTR]);
        tab[(LBT_BMP)>>2]                          |= (1 << i);
        tab[(LBT_TAB + i * _LB_SIZE_ + LB_PTR)>>2]  = pBlock->adr;
        tab[(LBT_TAB + i * _LB_SIZE_ + LB_SIZE)>>2] = pBlock->size;
        pBlock->idx     = i;  
        break;
      }
    }
    if (pBlock->idx == -1) {printf("No Idx for block found\n");}
  }

  return pBlock;

}


int freeBlockInTab(t_block* pBlock, t_block** pBlockList, t_block** pFreeList, uint32_t* tab) {

  //clear entry in table
  if (pBlock != NULL) {
    if (pBlock->idx < 32) {
      tab[(LBT_BMP)>>2]                          &= ~(1 << pBlock->idx);
      //unnecesssary but easier to debug
      tab[(LBT_TAB + pBlock->idx * _LB_SIZE_ + LB_PTR)>>2]  = 0;
      tab[(LBT_TAB + pBlock->idx * _LB_SIZE_ + LB_SIZE)>>2] = 0;
    }
  }
  return freeBlock(pBlock, pBlockList, pFreeList);
}

int freeBlock(t_block* pBlock, t_block** pBlockList, t_block** pFreeList) {
   t_block* c;  
  if (isInList(pBlock, (t_block*)*pBlockList)) {
 
    moveBlockBetweenLists(pBlock, pBlockList, pFreeList);
    *pFreeList = getHead(sortByAdr(*pFreeList));
    c = *pFreeList;
    while (c->next != NULL) {
      if (c->adr + c->size == c->next->adr) { //Coalesce adjacent blocks
        c->size += c->next->size;
        dest(c->next);
        
      } else c = c->next; // only advance if the new, bigger block was tested
    }
  } else {return -1;}
  return 0;
}



t_block* getBestFit(uint32_t size, t_block** pFreeList) {
  t_block* c;
  if (size != 0 && *pFreeList != NULL) {
    *pFreeList = getHead(sortBySize(*pFreeList));
    c = *pFreeList;
    //find a fitting free block
    if (c->size >= size) return c;
    while (c != NULL) {
      if (c->size >= size) { //best fit
        return c;
      }
      c = c->next;
    }
  } 

  return NULL;
}

t_block* allocateBlock(uint32_t size, t_block** pFreeList, t_block** pBlockList, t_block* getFit() ) {
  t_block* pF = getFit(size, pFreeList); // fitter returns matching free block, list is sorted
  t_block* pFNew;

  printf("Allocating 0x%05x...", size);
  if (pF != NULL) {
    printf(" allocator found 0x%05x 0x%05x\n", pF->adr, pF->size);
    if (pF->size != size) { //exact size match?
      //No. Split
      //create new free block of leftover size
      pFNew = (t_block*)malloc(sizeof(t_block));
      pFNew->size = pF->size - size;
      pFNew->adr  = pF->adr  + size;
      pFNew->idx  = -1;
      //Add to free list
      addAfter((getTail((t_block*)*pFreeList)), pFNew);
      //modify old free block before moving it to live block list
      pF->size = size;
    } else {printf("Size Match\n");}

    //move old free block to live block list
    moveBlockBetweenLists(pF, pFreeList, pBlockList);

  } else printf(" No space/elements in Free List!\n");

  return pF;
}


t_block* moveBlockBetweenLists(t_block* pSrc, t_block** pSrcList, t_block** pDstList) {
  if (isInList(pSrc, (t_block*)*pSrcList)) {
    if (pSrc == (t_block*)*pSrcList) { //are we trying to remove the element pSrcList points to?
      if      (((t_block*)*pSrcList)->next == NULL) *pSrcList = ((t_block*)*pSrcList)->prev;//Tail, get prev
      else if (((t_block*)*pSrcList)->prev == NULL) *pSrcList = ((t_block*)*pSrcList)->next;//Head. get next. if both are NULL, we get an empty List
    }
    *pDstList = addAfter(getTail(*pDstList), rem(pSrc));
  } else {printf("Block not found\n"); return NULL;}
  return pSrc;
}

void show(t_block* c) {
  if (c == NULL) printf("NULL\n");
  else printf("A 0x%08x S 0x%05x I #%2d\n", c->adr, c->size, c->idx);
}

void showfull(t_block* c) {
  if (c == NULL) {printf(" T NULL\n");}
  else { 
          if (c->prev != NULL)  printf(" P 0x%05x 0x%05x\n", c->prev->adr, c->prev->size); else  printf(" P NULL\n");
          printf(" T 0x%05x 0x%05x\n", c->adr, c->size);       
          if (c->next != NULL)  printf(" N 0x%05x 0x%05x\n", c->next->adr, c->next->size); else  printf(" N NULL\n");   
  }      
}

void showList(t_block* l) {
  uint32_t cnt = 0;
  if (l != NULL) {
    t_block *c = getHead(l);
    //c = sortByAdr(c);  
    show(c);
    while ((c->next != NULL)){
      c = c->next;      
      show(c);
      if (cnt > 99) {printf("Too many elements or link loop\n");break;}
      cnt++;
    }
  } else {printf("NULL\n");}
} 

int  getCont(t_block* pFreeList) {
  t_block* c;
  int m;


  c = getHead(pFreeList);
  if (c == NULL) return -1;
  m = 0;
  while (c != NULL) {
    if (c->size > m) m = c->size;
    c = c->next;
  }
  return m;

}

int  getUsage(t_block* pBlockList) {
  t_block* c;
  int m;


  c = getHead(pBlockList);
  if (c == NULL) return 0;
  m = 0;
  while (c != NULL) {
    m += c->size;
    c = c->next;
  }
  return m;

}

void showAll(t_block* pF, t_block* pB, int ramsize) {
    printf("#####################\nFree List:\n");
    pF = sortByAdr(pF);
    showList(getHead(pF));
    printf("Block List:\n");
    pB = sortByAdr(pB);
    showList(getHead(pB));
    printf("Memory use:     %u/%u, %u\n", getUsage(pB), ramsize, getUsage(pB) *100 / ramsize);
    printf("Max Continuous: %u\n", getCont(pF));
    printf("Fragmentation:  %u\n",  (ramsize - getCont(pF))*100 / ramsize );
    
    printf("+++++++++++++++++++++\n\n");
 
}

void showTab(uint32_t* tab) {
  int cnt;
  
  printf("**************************************\n** ");
  for (cnt=0; cnt < 32; cnt++) {if ((tab[(LBT_BMP)>>2] >> cnt) & 1) printf("1"); else printf("_");}
  printf(" **\n**************************************\n");
  for (cnt=0; cnt < 32; cnt++) {if ((tab[(LBT_BMP)>>2] >> cnt) | 1) { printf("#%2u  b: %u a: 0x%05x s: 0x%05x\n", cnt,
  ((tab[(LBT_BMP)>>2] >> cnt) & 1), tab[(LBT_TAB + cnt * _LB_SIZE_ + LB_PTR)>>2], tab[(LBT_TAB + cnt * _LB_SIZE_ + LB_SIZE)>>2]); }}
}

void showAllWithTab(t_block* pF, t_block* pB, int ramsize, uint32_t* tab) {
  showTab(tab);
  showAll(pF, pB, ramsize);
}

uint8_t createLBT(uint8_t* buf, t_block* pBlockList) {return 0;}

