/*!
 *  @file scu_circular_buffer.c
 *  @brief SCU- circular buffer resp. ring buffer administration in
 *         shared memory of LM32.
 *
 *  @date 21.10.2019
 *  @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Stefan Rauch perhaps...
 *  @revision Ulrich Becker <u.becker@gsi.de>
 *
 ******************************************************************************
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */
#include <stdlib.h>
#include <scu_function_generator.h>
#include <scu_circular_buffer.h>
#include <eb_console_helper.h>

#ifdef _CONFIG_USE_OLD_CB

/** @brief write parameter set to circular buffer
 *  @param cb pointer to the channel buffer
 *  @param cr pointer to the channel register
 *  @param channel number of the channel
 *  @param pset pointer to parameter set
 */
void cbWrite(volatile FG_CHANNEL_BUFFER_T* cb, volatile FG_CHANNEL_REG_T* cr, const int channel, FG_PARAM_SET_T* pset )
{
   unsigned int wptr = cr[channel].wr_ptr;
   /* write element to free slot */
   cb[channel].pset[wptr] = *pset;
   /* move write pointer forward */
   cr[channel].wr_ptr = (wptr + 1) % (BUFFER_SIZE);
   /* overwrite */
   if( cr[channel].wr_ptr == cr[channel].rd_ptr )
      cr[channel].rd_ptr = (cr[channel].rd_ptr + 1) % (BUFFER_SIZE);
}


/** @brief debug method for dumping a circular buffer
 *  @param cb pointer to the channel buffer
 *  @param cr pointer to the channel register
 *  @param channel number of the channel
 */
void cbDump(volatile FG_CHANNEL_BUFFER_T* cb, volatile FG_CHANNEL_REG_T* cr, const int channel )
{
   int i = 0, col;
   volatile FG_PARAM_SET_T *pset;
   mprintf("dumped cb[%d]: \n", channel);
   mprintf ("wr_ptr: %d rd_ptr: %d size: %d\n", cr[channel].wr_ptr, cr[channel].rd_ptr, BUFFER_SIZE);
   while(i < BUFFER_SIZE)
   {
      mprintf("%d ", i);
      for(col = 0; (col < 8) && (i < BUFFER_SIZE); col++)
      {
         pset = &cb[channel].pset[i++];
         mprintf("0x%x ", pset->coeff_c);
      }
   }
}

/** @brief add a message to a message buffer
 *  @param mb pointer to the first message buffer
 *  @param queue number of the queue
 *  @param m message which will be added to the queue
 */
int add_msg(volatile FG_MESSAGE_BUFFER_T* mb, int queue, const MSI_T* pm )
{
   const RING_POS_T next_head = (mb[queue].ring_head + 1) % RING_SIZE;
   if (next_head != mb[queue].ring_tail)
   {
      /* there is room */
      mb[queue].ring_data[mb[queue].ring_head] = *pm;
      mb[queue].ring_head = next_head;
      return 0;
   }
   /* no room left in the buffer */
   mprintf(ESC_WARNING"msg buffer %d full!\n"ESC_NORMAL, queue);
   return -1;
}

/** @brief remove a message from a message buffer
 *  @param mb pointer to the first message buffer
 *  @param queue number of the queue
 */
MSI_T remove_msg(volatile FG_MESSAGE_BUFFER_T* mb, int queue )
{
   MSI_T m;
   if( mb[queue].ring_head != mb[queue].ring_tail )
   {
      m = mb[queue].ring_data[mb[queue].ring_tail];
      mb[queue].ring_tail = (mb[queue].ring_tail + 1) % RING_SIZE;
      return m;
   }
   m.msg = -1;
   m.adr = -1;
   return m;
}
#endif //_CONFIG_USE_OLD_CB

//#define CONFIG_PRINT_DAQ_BUFFER_OVERFLOW

void add_daq_msg(volatile MIL_DAQ_BUFFER_T* mb, MIL_DAQ_OBJ_T m )
{
  const RING_POS_T next_head = (mb->ring_head + 1) % DAQ_RING_SIZE;
#ifdef CONFIG_PRINT_DAQ_BUFFER_OVERFLOW
  if( next_head == mb->ring_tail )
     mprintf( ESC_WARNING "DAQ buffer overflow!\n" ESC_NORMAL );
#endif
  mb->ring_data[mb->ring_head] = m;
  mb->ring_head = next_head;
}

/*================================== EOF ====================================*/
