typedef struct {
   uint32_t adr;
   uint32_t size;
   t_block* next;
   t_block* prev;  
} t_block;


uint32_t getAdr(t_block* c) {
  return c->adr;
}

uint32_t getSize(t_block* c) {
  return c->size;
}

t_block* getFirst(t_block* l) {
  while (l->prev != NULL) l = l->prev;
  return l;
}

t_block* getLast(t_block* l) {
  while (l->next != NULL) l = l->next;
  return l;
}

remove(t_block* pRem) {
  pRem->prev->next = pRem->next;
  pRem->next->prev = pRem->prev;
  free(pRem);
}

add_after(t_block* pDst, t_block* pAdd) {
  if (pDst != NULL) {
    if (pAdd != NULL) {
      pAdd->next = pDst->next;
      pAdd->prev = pDst;
    }
    if (pDst->next != NULL) pDst->next->prev = pAdd;
    pDst->next = pAdd;
  }
}

add_before(t_block* pDst, t_block* pAdd)  {
  if (pDst != NULL) {
    if (pAdd != NULL) {
      pAdd->next = pDst;
      pAdd->prev = pDst->prev;
    }
    if (pDst->prev != NULL) pDst->prev->next = pAdd;
    pDst->prev = pAdd;
  }
}

swap(t_block* pA, t_block* pB) {
  t_block* tmp = NULL;
  if ((pA != NULL) && (pB != NULL)) {
    if(pA->prev != NULL) pA->prev->next = pB;
    if(pB->next != NULL) pB->next->prev = pA;  
    tmp = pA->prev;
    pA->prev = pB->prev;
    pB->prev = tmp;
    tmp = pA->next;
    pA->next = pB->next;
    pB->next = tmp;
  }
}

t_block* sortByAdr(t_block* pList) {
  return bubblesort(pList, (void*)&getAdr());
}

t_block* sortBySize(t_block* pList) {
 return bubblesort(pList, (void*)&getSize());
}

t_block* bubblesort(t_block* pList, void* getValue) {
  tBlock* c = pList;
  tBlock* n;  
  bool    sorting = true;

  if (pList != NULL) {
    while sorting {
      sorting = false;
      if (c->next != NULL) {
        n = c->next;
        if (getValue(c) > getValue(c->next)) {swap(c, c->next); sorting = true;}
        if (c->prev == NULL) pList = c; //update pointer to the first element if necessary
        c = n;
      }
    }
  }
  return pList;
}

t_block* createBlockList(uint8_t* buf, t_block* pNewList)


t_block* createFreeList(t_block* pBlockList, t_block* pFreeList, uint32_t ramsize) {
  pBlockList = sortByAdr(pBlockList);

  t_block* c = pBlockList;
  t_block* d = pFreeList;

  //create first block
  if c->adr > 0 {
    t_block* pAdd = malloc(
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
        t_block* pAdd = malloc(
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
      if diff > 0 {
        t_block* pAdd = malloc(
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

t_block* freeBlock(t_block* pBlock, t_block* pFreeList) {
  if (pBlock != NULL && pFreeList != NULL) {

    pFreeList = moveBlockBetweenLists(pBlock, pFreeList);
    pFreeList = sortByAdr(pFreeList);
    c = pFreeList;

    while (c->next != NULL)
      if (c->adr + c->size == c->next->adr) { //Join adjacent blocks
        c->size += c->next->size;
        remove(c->next);
      } else c = c->next; // only advance if the new, bigger block was tested
    }
  }
  return pFreeList;
}



t_block* getBestFit(uint32_t size, t_block* pFreeList) {
  if (size != 0 && pFreeList != NULL) {

    pFreeList = sortBySize(pFreeList);
    c = pFreeList;
    //find a fitting free block
    while (c->next != NULL)
      if (c->size => size) { //best fit
        return c;
      }
    }
  }
  return NULL;
}

t_block* allocateBlock(uint32_t size, t_block* pFreeList, t_block* pBlockList, void* getFit) {
  t_block* pF = getFit(size, pFreeList); // fitter returns matching free block
  if (pF != NULL) {
    if (pF->size != size) { //exact size match?
      //No. Split
      //create new free block of leftover size
      add_after(pFreeList, pFNew);
      //modify old free block before moving it to live block list
      pF->size = size;
    }
    //move free block to live block list
    moveBlockBetweenLists(pF, pBlockList;
  }
  
}

t_block* moveBlockBetweenLists(t_block* pSrc, t_block* pDstList) {
  if (pSrc != NULL && pDstList != NULL) {
    remove(pSrc);
    add_before(pDstList);
    return pSrc;
  }
  return pDstList;
}

void showList(t_block* l) {
 
  t_block* c = getFirst(l);
  c = sortByAdr(c);  

 while (c->next != NULL) {
  printf("0x%08x %6u\n", c->adr, c->size
 }
} 

uint8_t createLBT(uint8_t* buf, t_block* pBlockList)

