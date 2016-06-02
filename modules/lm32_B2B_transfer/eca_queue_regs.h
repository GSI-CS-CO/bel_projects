/** Copyright (C) 2011-2016 GSI Helmholtz Centre for Heavy Ion Research GmbH 
 *
 *  @author Wesley W. Terpstra <w.terpstra@gsi.de>
 *
 *******************************************************************************
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************
 */
/** @file        eca_queue_regs.h
  * DesignUnit   eca_queue
  * @author      Wesley W. Terpstra <w.terpstra@gsi.de>
  * @date        08/04/2016
  * @version     2.0
  * @copyright   2016 GSI Helmholtz Centre for Heavy Ion Research GmbH
  *
  * @brief       Register map for Wishbone interface of VHDL entity <eca_queue_auto>
  */

#ifndef _ECA_QUEUE_H_
#define _ECA_QUEUE_H_

  #define ECA_QUEUE_SDB_VENDOR_ID 0x00000651
  #define ECA_QUEUE_SDB_DEVICE_ID 0xd5a3faea

  #define ECA_QUEUE_QUEUE_ID_GET     0x00  //ro,  8 b, The index of a_channel_o from the ECA to which this queue is connected (set channel_select=queue_id+1)
  #define ECA_QUEUE_POP_OWR          0x04  //wo,  1 b, Pop action from the channel's queue
  #define ECA_QUEUE_FLAGS_GET        0x08  //ro,  5 b, Error flags for this action(0=late, 1=early, 2=conflict, 3=delayed, 4=valid)
  #define ECA_QUEUE_NUM_GET          0x0c  //ro,  8 b, Subchannel target
  #define ECA_QUEUE_EVENT_ID_HI_GET  0x10  //ro, 32 b, Event ID (high word)
  #define ECA_QUEUE_EVENT_ID_LO_GET  0x14  //ro, 32 b, Event ID (low word)
  #define ECA_QUEUE_PARAM_HI_GET     0x18  //ro, 32 b, Parameter (high word)
  #define ECA_QUEUE_PARAM_LO_GET     0x1c  //ro, 32 b, Parameter (low word)
  #define ECA_QUEUE_TAG_GET          0x20  //ro, 32 b, Tag from the condition
  #define ECA_QUEUE_TEF_GET          0x24  //ro, 32 b, Timing extension field
  #define ECA_QUEUE_DEADLINE_HI_GET  0x28  //ro, 32 b, Deadline (high word)
  #define ECA_QUEUE_DEADLINE_LO_GET  0x2c  //ro, 32 b, Deadline (low word)
  #define ECA_QUEUE_EXECUTED_HI_GET  0x30  //ro, 32 b, Actual execution time (high word)
  #define ECA_QUEUE_EXECUTED_LO_GET  0x34  //ro, 32 b, Actual execution time (low word)

#endif
