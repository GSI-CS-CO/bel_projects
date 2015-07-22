#include <stdlib.h>
#include <cb.h>
#include <fg.h>
#include <aux.h>
#include <mprintf.h>

int cbisEmpty(volatile struct circ_buffer* cb, int num) {
  return cb[num].wr_ptr == cb[num].rd_ptr;
}

int cbisFull(volatile struct circ_buffer* cb, int num) {
  int ret = 0;
  ret = (cb[num].wr_ptr + 1) % cb[num].size == cb[num].rd_ptr;
  return ret; 
}

int cbgetCount(volatile struct circ_buffer* cb, int num) {
  return abs(cb[num].wr_ptr - cb[num].rd_ptr);
}

void cbWrite(volatile struct circ_buffer* cb, int num, struct param_set *pset) {
  int rptr = cb[num].rd_ptr;
  int wptr = cb[num].wr_ptr;
  int size = cb[num].size;
  /* write element to free slot */
  cb[num].pset[wptr] = *pset;
  /* move write pointer forward */
  cb[num].wr_ptr = (wptr + 1) % size;
  /* overwrite */
  if (cb[num].wr_ptr == cb[num].rd_ptr)
    cb[num].rd_ptr = (cb[num].rd_ptr + 1) % cb[num].size;
}

void cbRead(volatile struct circ_buffer *cb, int num, struct param_set *pset) {
  int rptr = cb[num].rd_ptr;
  int wptr = cb[num].wr_ptr;
  int size = cb[num].size;
  /* check empty */
  if (wptr == rptr) {
    return;
  }
  /* read element */
  *pset = cb[num].pset[rptr];
  //mprintf("%x(%d) ", cb[num].pset[rptr].coeff_c, rptr);
  //if (!(cb[num].pset[rptr].coeff_c % 10))
  //  mprintf("\n");
  /* move read pointer forward */
  cb[num].rd_ptr = (rptr + 1) % size;    
}

void cbDump(volatile struct circ_buffer *cb, int num) {
  int i = 0, col;
  struct param_set *pset;
  mprintf("dumped cb[%d]: \n", num);  
  mprintf ("wr_ptr: %d rd_ptr: %d size: %d\n", cb[num].wr_ptr, cb[num].rd_ptr, cb[num].size);
  while(i < cb[num].size) {
    mprintf("%d ", i);
    for(col = 0; (col < 8) && (i < cb[num].size); col++) {
      *pset = cb[num].pset[i++];
      mprintf("0x%x ", pset->coeff_c);
    }
    mprintf("\n");
  }
}
