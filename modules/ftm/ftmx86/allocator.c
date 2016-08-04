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
    t = getFirst(pList);
    while (t != NULL) {
      if (t == c) return 1;
      t = t->next;
    }
  }
  return 0;  
} 


t_block* getFirst(t_block* l) {
  if(l != NULL) while (l->prev != NULL) l = l->prev;
  return l;
}

t_block* getLast(t_block* l) {
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

t_block* add_after(t_block* pDst, t_block* pAdd) {
  
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

t_block* add_before(t_block* pDst, t_block* pAdd)  {
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
    add_after(pB, rem(pA));
    
  }
}

t_block* sortByAdr(t_block* pList) {
  return bubblesort(pList, (void*)&getAdr);
}

t_block* sortBySize(t_block* pList) {
 return bubblesort(pList, (void*)&getSize);
}

t_block* bubblesort(t_block* pList, uint32_t (*getValue)()) {
  t_block* n = getFirst(pList);
  t_block* c = n;  
  int sorting = 1;
  if (pList != NULL) {
    while (sorting) {
      sorting = 0;
      while (c->next != NULL) {
        //showfull(c);
        if (getValue(c) > getValue(c->next)) {swap(c, c->next); sorting = 1; }
        else { c = c->next; }
        //showfull(c);
      }
      c = n; //reset to list head
    }
  }
  return pList; 
}

t_block* createBlockList(uint8_t* buf, t_block* pNewList) {return NULL;}


t_block* createFreeList(t_block* pBlockList, t_block* pFreeList, uint32_t ramsize) {
  pBlockList = sortByAdr(pBlockList);
  int32_t diff;
  t_block* c = pBlockList;
  t_block* d = pFreeList;

  //create first block
  if (c->adr > 0) {
    t_block* pAdd = (t_block*)malloc(sizeof(t_block));
    if (pAdd != NULL) {
      pAdd->adr   = 0;
      pAdd->size  = c->adr;
      add_after(d, pAdd);
      d           = d->next;
    } else {printf("CreateFreeList: Couldn't alloc new free element\n");}
  }
  while (c != NULL ) {
    if (c->next != NULL) {
      diff = c->adr + c->size - c->next->adr;
      if (diff > 0) {
        t_block* pAdd = (t_block*)malloc(sizeof(t_block));
        if (pAdd != NULL) {
          pAdd->adr   = c->adr + c->size;
          pAdd->size  = diff;
          add_after(d, pAdd);
          d           = d->next;
        } else {printf("CreateFreeList: Couldn't alloc new free element\n");}

      }
    } else {
      //create last block
      diff = ramsize - (c->adr + c->size);  
      if (diff > 0) {
        t_block* pAdd = (t_block*)malloc(sizeof(t_block));
        if (pAdd != NULL) {
          pAdd->adr   = c->adr + c->size;
          pAdd->size  = diff;
          add_after(d, pAdd);
          d           = d->next;
        } else {printf("CreateFreeList: Couldn't alloc new free element\n");}
      }
    }
    c = c->next;
  }
  
  return pBlockList;        
}

t_block* freeBlock(t_block* pBlock, t_block** pBlockList, t_block* pFreeList) {
   t_block* c;  
  if (isInList(pBlock, (t_block*)*pBlockList)) {
    //printf("Block found\n");
    pFreeList = moveBlockBetweenLists(pBlock, pBlockList, pFreeList);
    pFreeList = getFirst(sortByAdr(pFreeList));
    c = getFirst(pFreeList);
    //printf("Clear so far\n");
    while (c->next != NULL) {
      /*printf("Left\n");
      show(c);
      printf("Right\n");
      show(c->next);
      */
      if (c->adr + c->size == c->next->adr) { //Coalesce adjacent blocks
        //printf("Coalesce\n");
        c->size += c->next->size;
        /*        
        show(c);
        printf("!!! Old Free List\n");
        showList(getFirst(pFreeList));
        */
        dest(c->next);
        /*
        printf("!!! New Free List\n");
        showList(getFirst(pFreeList));
        */
      } else c = c->next; // only advance if the new, bigger block was tested
    }
  }
  return pFreeList;
}



t_block* getBestFit(uint32_t size, t_block* pFreeList) {
  t_block* c;
  if (size != 0 && pFreeList != NULL) {
    pFreeList = getFirst(sortBySize(pFreeList));
    c = pFreeList;
    //find a fitting free block
    if (c->size >= size) return c;
    while (c->next != NULL) {
      if (c->size >= size) { //best fit
        return c;
      }
      c = c->next;
    }
  } 

  return NULL;
}

t_block* allocateBlock(uint32_t size, t_block** pFreeList, t_block* pBlockList, t_block* getFit() ) {
  t_block* pF = getFit(size, *pFreeList); // fitter returns matching free block
  t_block* pFNew;
  // printf("Match\n");
  //showfull(pF);
  if (pF != NULL) {
    if (pF->size != size) { //exact size match?
      //No. Split
      pFNew = (t_block*)malloc(sizeof(t_block));
      pFNew->size = pF->size - size;
      pFNew->adr  = pF->adr + size;
      //create new free block of leftover size
     
      *pFreeList = add_after(((t_block*)*pFreeList), pFNew);
      //modify old free block before moving it to live block list
      pF->size = size;
    }
    /*
    printf("LEftover\n");
    showfull(pFNew);
    printf("Block\n");
    showfull(pF);
    */
    //move free block to live block list
    
    
    pBlockList = moveBlockBetweenLists(pF, pFreeList, pBlockList);
   
  } else printf("No space/elements in Free List!\n");
  return pBlockList;
}


t_block* moveBlockBetweenLists(t_block* pSrc, t_block** pSrcList, t_block* pDstList) {
  

  if (isInList(pSrc, (t_block*)*pSrcList)) {
    if (pSrc == (t_block*)*pSrcList) { //are we trying to remove the element pSrcList points to?
      if (((t_block*)*pSrcList)->next == NULL) *pSrcList = ((t_block*)*pSrcList)->prev;//Tail, get prev
      else if (((t_block*)*pSrcList)->prev == NULL) *pSrcList = ((t_block*)*pSrcList)->next;//Head. get next. if both are NULL, we get an empty List
    }
    pDstList = add_after(pDstList, rem(pSrc));
  } else {printf("Block not found\n");}
  return pDstList;
}

void show(t_block* c) {
  if (c == NULL) printf("NULL\n");
  else printf("0x%08x 0x%05x\n", c->adr, c->size);
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
    t_block *c = getFirst(l);
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

int  showFrag(t_block* pFreeList, int ramsize) {
  t_block* c;
  int m;

  if (ramsize != 0) { 
    c = getFirst(pFreeList);
    m = 0;
    while (c != NULL) {
      if (c->size > m) m = c->size;
      c = c->next;
    }
    return ((ramsize - m) * 100) / ramsize;
  } else return -1;
}

void showAll(t_block* pF, t_block* pB, int ramsize) {
    printf("#####################\nFree List:\n");
    pF = sortByAdr(pF);
    showList(getFirst(pF));
    printf("Block List:\n");
    pB = sortBySize(pB);
    showList(getFirst(pB));
    printf("Fragmentation: %u\n", showFrag(pF, ramsize));
    printf("+++++++++++++++++++++\n\n");
}

uint8_t createLBT(uint8_t* buf, t_block* pBlockList) {return 0;}

