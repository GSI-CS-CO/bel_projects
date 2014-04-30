#include <stdlib.h>
#include <cb.h>
#include <fg.h>
#include <aux.h>

int cbisEmpty(struct circ_buffer* cb, int num) {
  int ret = 0;
  atomic_on();
  ret = cb[num].wr_ptr == cb[num].rd_ptr;
  atomic_off();
  return ret;
}

int cbisFull(struct circ_buffer* cb, int num) {
  int ret = 0;
  atomic_on();
  ret = (cb[num].wr_ptr + 1) % cb[num].size == cb[num].rd_ptr;
  atomic_off();
  return ret; 
}

int cbgetCount(struct circ_buffer* cb, int num) {
  int ret = 0;
  atomic_on();
  ret = abs(cb[num].wr_ptr - cb[num].rd_ptr);
  atomic_off();
  return ret;
}

void cbWrite(struct circ_buffer* cb, int num, struct param_set *pset) {
  atomic_on();
  /* write element to free slot */
  cb[num].pset[cb[num].wr_ptr] = *pset;
  /* move write pointer forward */
  cb[num].wr_ptr = (cb[num].wr_ptr + 1) % cb[num].size;
  /* overwrite */
  if (cb[num].wr_ptr == cb[num].rd_ptr)
    cb[num].rd_ptr = (cb[num].rd_ptr + 1) % cb[num].size;
  atomic_off();
}

void cbRead(struct circ_buffer *cb, int num, struct param_set *pset) {
  atomic_on();
  /* check empty */
  if (cb[num].wr_ptr == cb[num].rd_ptr) {
    atomic_off();
    return;
  }
  /* read element */
  *pset = cb[num].pset[cb[num].rd_ptr];
  /* move read pointer forward */
  cb[num].rd_ptr = (cb[num].rd_ptr + 1) % cb[num].size;    
  atomic_off();
}


