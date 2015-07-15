#include <stdlib.h>
#include <cb.h>
#include <fg.h>
#include <aux.h>
#include <mprintf.h>

int cbisEmpty(volatile struct channel_regs* cr, int channel) {
  return cr[channel].wr_ptr == cr[channel].rd_ptr;
}

int cbisFull(volatile struct channel_regs* cr, int channel) {
  int ret = 0;
  ret = (cr[channel].wr_ptr + 1) % (BUFFER_SIZE+1) == cr[channel].rd_ptr;
  return ret; 
}

int cbgetCount(volatile struct channel_regs* cr, int channel) {
  return abs(cr[channel].wr_ptr - cr[channel].rd_ptr);
}

void cbWrite(volatile struct channel_buffer* cb, volatile struct channel_regs* cr, int channel, struct param_set *pset) {
  unsigned int wptr = cr[channel].wr_ptr;
  /* write element to free slot */
  cb[channel].pset[wptr] = *pset;
  /* move write pointer forward */
  cr[channel].wr_ptr = (wptr + 1) % (BUFFER_SIZE+1);
  /* overwrite */
  if (cr[channel].wr_ptr == cr[channel].rd_ptr)
    cr[channel].rd_ptr = (cr[channel].rd_ptr + 1) % (BUFFER_SIZE+1);
}

void cbRead(volatile struct channel_buffer *cb, volatile struct channel_regs* cr, int channel, struct param_set *pset) {
  unsigned int rptr = cr[channel].rd_ptr;
  unsigned int wptr = cr[channel].wr_ptr;
  /* check empty */
  if (wptr == rptr) {
    return;
  }
  /* read element */
  *pset = cb[channel].pset[rptr];
  /* move read pointer forward */
  cr[channel].rd_ptr = (rptr + 1) % (BUFFER_SIZE+1);    
}

void cbDump(volatile struct channel_buffer *cb, volatile struct channel_regs* cr, int channel) {
  int i = 0, col;
  struct param_set *pset;
  mprintf("dumped cb[%d]: \n", channel);  
  mprintf ("wr_ptr: %d rd_ptr: %d size: %d\n", cr[channel].wr_ptr, cr[channel].rd_ptr, BUFFER_SIZE+1);
  while(i < BUFFER_SIZE+1) {
    mprintf("%d ", i);
    for(col = 0; (col < 8) && (i < BUFFER_SIZE+1); col++) {
      *pset = cb[channel].pset[i++];
      mprintf("0x%x ", pset->coeff_c);
    }
  }
}
