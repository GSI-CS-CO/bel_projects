#ifndef __SCU_MIL_H_
#define __SCU_MIL_H_

#include <stdint.h>
#include <board.h>
#include <mprintf.h>
#include <syscon.h>

extern int usleep(useconds_t usec);
int trm_free(volatile unsigned int *base);
int write_mil(volatile unsigned int *base, short data, short fc_ifk_addr);
int rcv_flag(volatile unsigned int *base);
int read_mil(volatile unsigned int *base, short *data, short fc_ifk_addr);
void clear_receive_flag(volatile unsigned int *base);
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
#define   OKAY                1
#define   TRM_NOT_FREE        -1
#define   RCV_ERROR           -2
#define   RCV_TIMEOUT         -3


/*
  +---------------------------------------------+
  |       mil register addresses                |
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
#define   EV_FILT_FIRST       0x1000    // first event filter (ram) address.
#define   EV_FILT_LAST        0x1FFF    // last event filter (ram) addres.

/*
  +---------------------------------------------+
  |       mil status register layout            |
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


#endif
