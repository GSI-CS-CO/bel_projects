#include <stdlib.h>
#include <cb.h>
#include <fg.h>
#include <aux.h>
#include <mprintf.h>

/** @brief write parameter set to circular buffer
 *  @param cb pointer to the channel buffer
 *  @param cr pointer to the channel register
 *  @param channel number of the channel
 *  @param pset pointer to parameter set
 */
void cbWrite(volatile struct channel_buffer* cb, volatile struct channel_regs* cr, int channel, struct param_set *pset) {
  unsigned int wptr = cr[channel].wr_ptr;
  /* write element to free slot */
  cb[channel].pset[wptr] = *pset;
  /* move write pointer forward */
  cr[channel].wr_ptr = (wptr + 1) % (BUFFER_SIZE);
  /* overwrite */
  if (cr[channel].wr_ptr == cr[channel].rd_ptr)
    cr[channel].rd_ptr = (cr[channel].rd_ptr + 1) % (BUFFER_SIZE);
}


/** @brief debug method for dumping a circular buffer
 *  @param cb pointer to the channel buffer
 *  @param cr pointer to the channel register
 *  @param channel number of the channel
 */
void cbDump(volatile struct channel_buffer *cb, volatile struct channel_regs* cr, int channel) {
  int i = 0, col;
  struct param_set *pset;
  mprintf("dumped cb[%d]: \n", channel);  
  mprintf ("wr_ptr: %d rd_ptr: %d size: %d\n", cr[channel].wr_ptr, cr[channel].rd_ptr, BUFFER_SIZE);
  while(i < BUFFER_SIZE) {
    mprintf("%d ", i);
    for(col = 0; (col < 8) && (i < BUFFER_SIZE); col++) {
      *pset = cb[channel].pset[i++];
      mprintf("0x%x ", pset->coeff_c);
    }
  }
}


void add_daq_msg(volatile struct daq_buffer *mb, struct daq m) {
  ring_pos_t next_head = (mb->ring_head + 1) % DAQ_RING_SIZE;
  mb->ring_data[mb->ring_head] = m;
  mb->ring_head = next_head;
}

/** @brief add a message to a message buffer
 *  @param mb pointer to the first message buffer
 *  @param queue number of the queue
 *  @param m message which will be added to the queue
 */
int add_msg(volatile struct message_buffer *mb, int queue, struct msi m) {
  ring_pos_t next_head = (mb[queue].ring_head + 1) % RING_SIZE;
  if (next_head != mb[queue].ring_tail) {
      /* there is room */
      mb[queue].ring_data[mb[queue].ring_head] = m;
      mb[queue].ring_head = next_head;
      return 0;
  } else {
      /* no room left in the buffer */
      mprintf("msg buffer %d full!\n", queue);
      return -1;
  }
}

/** @brief remove a message from a message buffer
 *  @param mb pointer to the first message buffer
 *  @param queue number of the queue
 */
struct msi remove_msg(volatile struct message_buffer *mb, int queue) {
  struct msi m;
  if (mb[queue].ring_head != mb[queue].ring_tail) {
      m = mb[queue].ring_data[mb[queue].ring_tail];
      mb[queue].ring_tail = (mb[queue].ring_tail + 1) % RING_SIZE;
      return m;
  } else {
      m.msg = -1;
      m.adr = -1;
      return m;
  }
}

/** @brief test if a queue has any messages
 *  @param mb pointer to the first message buffer
 *  @param queue number of the queue
 */
int has_msg(volatile struct message_buffer *mb, int queue) {
    if (mb[queue].ring_head != mb[queue].ring_tail)
      return 1;
    else
      return 0;
}
