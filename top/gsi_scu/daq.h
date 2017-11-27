////////////////////////////////////////////////////////////////////////////////
//
// filename: daq.h
// desc: header file with defines of the daq macro
// creation date: 27.11.2017
// last modified: 27.11.2017
// author: Stefan Rauch
//
// Copyright (C) 2017 GSI Helmholtz Centre for Heavy Ion Research GmbH 
//
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//  
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.
///////////////////////////////////////////////////////////////////////////////// 
#define DAQ_MAX_CHN     16
#define DAQ_BASE        0x2000
#define DAQ_BUFFER_SIZE 512  // last 10 words are the descriptor
#define DAQ_PM_SIZE     1024 // last 10 words are the descriptor

/* all parameters start counting at 0 */
#define DAQ_CNTRL(CHN)    (DAQ_BASE + (0x0  + CHN)) /* control register */
#define DAQ_PM_FIFO(CHN)  (DAQ_BASE + (0x40 + CHN)) /* data of the post mortem fifo/ hires fifo.
                                                       fifo is filled like a circular buffer, until
                                                       the sampling is stopped. */
#define DAQ_DAT_FIFO(CHN) (DAQ_BASE + (0x50 + CHN)) /* data of the daq fifo. has two pages, one is filled with data,
                                                       the other can be read over the scu bus */
#define DAQ_DAT_CNT(CHN)  (DAQ_BASE + (0x70 + CHN)) /* remaining words in daq fifo */
#define DAQ_PM_CNT(CHN)   (DAQ_BASE + (0x80 + CHN)) /* remaining words in post mortem fifo */
#define DAQ_TRIG_LW(CHN)  (DAQ_BASE + (0x10 + CHN)) /* least significant word of the scu bus tag used as trigger */
#define DAQ_TRIG_HW(CHN)  (DAQ_BASE + (0x20 + CHN)) /* most significant word of the scu bus tag used as trigger */
#define DAQ_TRIG_DLY(CHN) (DAQ_BASE + (0x30 + CHN)) /* trigger delay in number of samples */

/* bit definitions of the control register */
#define ENA_PM         (1 << 0) /* starts post mortem sampling with 100us sample rate */
#define SAMPLE_10US    (1 << 1) /* use 10us sample rate */
#define SAMPLE_100US   (1 << 2) /* use 100us sample rate */
#define SAMPLE_1MS     (1 << 3) /* use 1ms sample rate */
#define ENA_TRIG       (1 << 4) /* start sampling when triggered */
#define EXT_TRIG       (1 << 5) /* set to 1: external trigger, set to 0: tag trigger */
#define ENA_HIRES      (1 << 6) /* high resolution with 4MHz sample rate */
#define EXT_TRIG_HIRES (1 << 7) /* trigger source in hi res mode.
                                   set to 1: external trigger, set to 0: tag trigger */

#define DAQ_INT        0x60 /* daq interrupt register
                               Bit 0: channel 0
                               ...
                               Bit 15: channel 15 */
#define DAQ_HIRES_INT  0x61 /* hires interrupt register
                               Bit 0: channel 0
                               ...
                               Bit 15: channel 15 */

/* the preset value from the four TMS registers will be latched into the timestamp count,
 * when the tag, programmed into the TAG_LW and TAG_HW regs, arrives one the scu bus */
#define DAQ_TMS_0      0x62 /* timestamp counter preset word 0 (Bit 15..0) */
#define DAQ_TMS_1      0x63 /* timestamp counter preset word 1 (Bit 31..16) */
#define DAQ_TMS_2      0x64 /* timestamp counter preset word 2 (Bit 47..32) */
#define DAQ_TMS_3      0x64 /* timestamp counter preset word 3 (Bit 63..48) */
#define DAQ_TMS_TAG_LW 0x66 /* timestamp counter tag (Bit 15..0) */
#define DAQ_TMS_TAG_HW 0x67 /* timestamp counter tag (Bit 31..16) */
