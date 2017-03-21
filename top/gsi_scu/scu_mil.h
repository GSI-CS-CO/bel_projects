#ifndef __SCU_MIL_H_
#define __SCU_MIL_H_

#include <stdint.h>
#include <board.h>
#include <mprintf.h>
#include <syscon.h>

/**************************************************************************
* MIL device bus (Canon socket)      
*   write_mil: write data to device bus
*   read_mil:  read data from device bus
*      base:   Wishbone address seen from CPUs perspective
*      fc_ifk: 1st byte: function code, 2nd byte: interface board MIL address
*      data  : 2 bytes of data
*   return value: error code
***************************************************************************/
int write_mil(volatile unsigned int *base, short data, short fc_ifk_addr);
int read_mil(volatile unsigned int *base, short *data, short fc_ifk_addr);

/*************************************************************************** 
* MIL event bus (bipolar LEMO socket)       
*- filter RAM
*-- filter for MIL events are implemented using a dedicated filter RAM
*-- the filter RAM has 4096 elements (16 virtual accelerator * 256 event codes)
*- a "status" register is used to set and get properties     
*- received MIL events are available in FIFO
****************************************************************************/

/* MIL event filter */
int clear_filter_evt_mil(volatile unsigned int *base);
int set_filter_evt_mil(volatile unsigned int *base, unsigned short evt_code, unsigned short acc_number, unsigned short filter);
int enable_filter_evt_mil(volatile unsigned int *base);
int disable_filter_evt_mil(volatile unsigned int *base);

/* MIL event status and control register */
int write_statusreg_evt_mil(volatile unsigned int *base, unsigned int data);
int read_statusreg_evt_mil(volatile unsigned int *base, unsigned int *data);

/* MIL event FIFO */
int fifo_notempty_evt_mil(volatile unsigned int *base);
int clear_fifo_evt_mil(volatile unsigned int *base);
int pop_fifo_evt_mil(volatile unsigned int *base, unsigned int *data);

/*************************************************************************** 
 * configure the mode for LEMOs to used with MIL events                 
 * lemos are numbered 1..4                                              
 * there are three modes: 
 * - disabled 
 * - single pulse, produces a 500ns pulse upon a single event 
 * - gate, produces a frame pulse using two events    
 *
 * this configuration includes setting the MIL_PULS1/2_FRAME bits
 * 
 * once a lemo is configured, the filter RAM must be configured accordingly
 * using the routine set_filter_evt_mil
 **************************************************************************/
int enable_lemo_pulse_evt_mil(volatile unsigned int *base, unsigned int lemo);
int enable_lemo_gate_evt_mil(volatile unsigned int *base, unsigned int lemo);
int disable_lemo_evt_mil(volatile unsigned int *base, unsigned int lemo);

/* routines for internal use */
extern int usleep(useconds_t usec);
int trm_free(volatile unsigned int *base);
int rcv_flag(volatile unsigned int *base);
void run_mil_test(volatile unsigned int *base, unsigned char ifk_addr);


#define  MAX_TST_CNT      10000
#define  FC_WR_IFC_ECHO   0x13
#define  FC_RD_IFC_ECHO   0x89
#define  SCU_MIL          0x35aa6b96

/*
  +---------------------------------------------+
  |   mil communication error codes             |
  +---------------------------------------------+
*/
#define   OKAY                 1
#define   TRM_NOT_FREE        -1
#define   RCV_ERROR           -2
#define   RCV_TIMEOUT         -3
#define   MIL_OUT_OF_RANGE    -4


/*
  +---------------------------------------------+
  |       mil register addresses                |
  | ATTENTION: the addresses are in uint32_t !! | 
  | This is different to typical header files,  |
  | where addresses are in uint8_t              |
  +---------------------------------------------+
*/
#define   MIL_RD_WR_DATA      0x0000    // read or write mil bus; only 32bit access alowed; data[31..16] don't care
#define   MIL_WR_CMD          0x0001    // write command to mil bus; only 32bit access alowed; data[31..16] don't care
#define   MIL_WR_RD_STATUS    0x0002    // read mil status register; only 32bit access alowed; data[31..16] don't care
                                        // write mil control register; only 32bit access alowed; data[31..16] don't care
#define   RD_CLR_NO_VW_CNT    0x0003    // use only when FPGA Manchester Endecoder is enabled;
                                        // read => valid word error counters; write => clears valid word error counters
#define   RD_WR_NOT_EQ_CNT    0x0004    // use only when FPGA Manchester Endecoder is enabled;
                                        // read => rcv pattern not equal errors; write => clears rcv pattern not equal errors
#define   RD_CLR_EV_FIFO      0x0005    // read => event fifo; write clears event fifo.
#define   RD_CLR_TIMER        0x0006    // read => event timer; write clear event timer.
#define   RD_WR_DLY_TIMER     0x0007    // read => delay timer; write set delay timer.
#define   RD_CLR_WAIT_TIMER   0x0008    // read => wait timer; write => clear wait timer.
#define   MIL_WR_RF_LEMO_CONF 0x0009    // read => mil lemo config; write ==> mil lemo config (definition of bits: see below)
#define   MIL_WR_RD_LEMO_DAT  0x000A    // read => mil lemo data; write ==> mil lemo data
#define   MIL_RD_LEMO_INP_A   0x000B    // read mil lemo input

#define   EV_FILT_FIRST       0x1000    // first event filter (ram) address.
#define   EV_FILT_LAST        0x1FFF    // last event filter (ram) addres.
                                        // the filter RAM has a size of 4096 elements
                                        // each element has a size of 4 bytes
                                        // (definition of filter bits: see below)
                                        // the filter RAM is addressed in the following way
                                        //    uint32_t *pFilter; 
                                        //    pFilter[virtAcc * 256 + evtCode]

/*
  +---------------------------------------------+
  |       mil status register layout            |
  | (definition of bits)                        |
  +---------------------------------------------+
*/
#define   MIL_INTERLOCK_INTR  0x0001    // '1' => interlock interrupt is active
#define   MIL_DATA_RDY_INTR   0x0002    // '1' => data ready interrupt is active
#define   MIL_DATA_REQ_INTR   0x0004    // '1' => data request interrupt is active
#define   MIL_EV_FIFO_NE      0x0008    // '1' => event fifo is not empty
#define   MIL_EV_FIFO_FULL    0x0010    // '1' => event fifo is full
#define   MIL_RCV_READY       0x0020    // '1' => mil received data/commands
#define   MIL_CMD_RCV         0x0040    // '1' => received pattern is a command
#define   MIL_TRM_READY       0x0080    // '1' => mil is free to transmit a pattern
#define   MIL_RCV_ERROR       0x0100    // '1' => mil received an error
#define   MIL_EV_RESET_ON     0x0200    //
#define   MIL_PULS1_FRAME     0x0400    // '1' => one event (defined in event filter) switch puls1 on, an other event switch puls1 off
                                        // '0' => one event (defined in event filter) generates a pulse on frontpannel lemo1
#define   MIL_PULS2_FRAME     0x0800    // '1' => one event (defined in event filter) switch puls2 on, an other event switch puls2 off
                                        // '0' => one event (defined in event filter) generates a pulse on frontpannel lemo2
#define   MIL_INTR_DEB_ON     0x1000    // '1' => deboune devicebus interrupts is on
#define   MIL_EV_FILTER_ON    0x2000    // '1' => event filter is generally enabled; you must programm the event filter
#define   MIL_EV_12_8B        0x4000    // '1' => event decoding 12 bit; '0' => event decoding 8 bit
#define   MIL_ENDECODER_FPGA  0x8000    // '1' => use manchester en/decoder in fpga; '0' => use external en/decoder 6408

/*
  +---------------------------------------------+
  |       event filter element layout           |
  | definition of bits 0..5                     |
  | bits 6..31 are ignored                      |
  +---------------------------------------------+
*/
#define   FI_EV_TO_FIFO       0x0001    // '1' => incoming event is written to FIFO
#define   FI_EV_TIMER_RES     0x0002    // '1' => if bit MIL_EV_RESET_ON is set, an incoming event resets the event timer
#define   FI_EV_PULS1_S       0x0004    // '1' => incoming event controls LEMO1 output
                                        //        - if MIL_PULS1_FRAME: start of frame pulse ("Rahmenpuls")
                                        //        - else:               start of event pulse counter for ev_pulse1
#define   FI_EV_PULS1_E       0x0008    // '1' => incoming event controls LEMO1 output
                                        //        - if MIL_PULS1_FRAME: stop of frame pulse ("Rahmenpuls")
#define   FI_EV_PULS2_S       0x0010    // '1' => incoming event controls LEMO2 output                            
                                        //        - if MIL_PULS1_FRAME: start of frame pulse ("Rahmenpuls")       
                                        //        - else:               start of event pulse counter for ev_pulse1
#define   FI_EV_PULS2_E       0x0020    //  1' => incoming event controls LEMO1 output                    
                                        //        - if MIL_PULS1_FRAME: stop of frame pulse ("Rahmenpuls")
/*
  +---------------------------------------------+
  |      LEMO config  element layout            |
  | definition of bits 0..7                     |
  | bits 8..31 are ignored                      |
  +---------------------------------------------+
*/
#define   LEMO_OUT_EN1        0x0001    // '1' ==> LEMO 1 configured as output (MIL Piggy)
#define   LEMO_OUT_EN2        0x0002    // '1' ==> LEMO 2 configured as output (MIL Piggy)
#define   LEMO_OUT_EN3        0x0004    // '1' ==> LEMO 3 configured as output (SIO)
#define   LEMO_OUT_EN4        0x0008    // '1' ==> LEMO 4 configured as output (SIO)
#define   LEMO_EVENT_EN1      0x0010    // '1' ==> LEMO 1 can be controlled by event (MIL Piggy)
#define   LEMO_EVENT_EN2      0x0020    // '1' ==> LEMO 2 can be controlled by event (MIL Piggy)
#define   LEMO_EVENT_EN3      0x0040    // '1' ==> LEMO 3 can be controlled by event (unused?)
#define   LEMO_EVENT_EN4      0x0080    // '1' ==> LEMO 4 can be controlled by event (unused?) 



#endif
