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
#include <scu_circular_buffer.h>
#include <eb_console_helper.h>

//#define CONFIG_PRINT_DAQ_BUFFER_OVERFLOW

#ifdef CONFIG_MIL_DAQ_USE_RAM
 #error "Obsolete module when CONFIG_MIL_DAQ_USE_RAM is defined!"
#endif

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
