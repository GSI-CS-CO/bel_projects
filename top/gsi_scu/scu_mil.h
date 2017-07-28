#ifndef __SCU_MIL_H_
#define __SCU_MIL_H_

#include <stdint.h>
#include <board.h>
#include <mprintf.h>
#include <syscon.h>
#include <aux.h>


/***********************************************************
 * MIL bus library by Wolfgang Panschow
 * 
 * This library works but has two issues:
 * 
 * 1. usage of platfrom dependent data types such as int
 *    --> conider using platfrom independent types such aa
 *    uint32_t instead
 * 2. register offsets are defined in units of integers,
 *    whereas common practice within ohwr is to use
 *    offsets in units of uint8_t
 *    --> consider using a different offset defintion
 * 
 * This file is split into two parts. The original code below 
 * is kept unchanged and is locacted directly below in the 
 * 1st part. 
 * The 2nd parts defines a new interface taking the into
 * account the suggestions above and extends the functionality
 * 
 * It shall be discussed, if the first part shall be deprecated
 * and using the definitions and routines of the 2nd part 
 * is encouraged.
 ***********************************************************/

/***********************************************************
 ***********************************************************
 *  
 * 1st part: original MIL bus library
 *
 ***********************************************************
 ***********************************************************/

extern int usleep(useconds_t usec);
int write_mil(volatile unsigned int *base, short data, short fc_ifk_addr);
int trm_free(volatile unsigned int *base);
int rcv_flag(volatile unsigned int *base);
int read_mil(volatile unsigned int *base, short *data, short fc_ifk_addr);
void clear_receive_flag(volatile unsigned int *base);
void run_mil_test(volatile unsigned int *base, unsigned char ifk_addr);
int status_mil(volatile unsigned int *base, unsigned short *status);
int write_mil_blk(volatile unsigned int *base, short *data, short fc_ifc_addr);
int scub_rcv_flag(volatile unsigned short *base, int slot);
int scub_status_mil(volatile unsigned short *base, int slot, unsigned short *status);
int scub_read_mil(volatile unsigned short *base, int slot, short *data, short fc_ifc_addr);



#define MAX_TST_CNT       10000
#define FC_WR_IFC_ECHO    0x13
#define FC_RD_IFC_ECHO    0x89
#define SCU_MIL           0x35aa6b96
#define MIL_SIO3_OFFSET   0x400
#define CALC_OFFS(SLOT)   (((SLOT) * (1 << 16)) + MIL_SIO3_OFFSET)


/*
  +---------------------------------------------+
  |   mil communication error codes             |
  +---------------------------------------------+
*/
#define   OKAY                 1
#define   TRM_NOT_FREE        -1
#define   RCV_ERROR           -2
#define   RCV_TIMEOUT         -3


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
#define   TX_TASK_0           0x0040    // write => first read command
#define   TX_TASK_1           0x0041    // write => second read command
#define   TX_TASK_2           0x0042    // write => ... read command
#define   TX_TASK_3           0x0043    // write => ... read command
#define   TX_TASK_4           0x0044    // write => ... read command
#define   TX_TASK_5           0x0045    // write => ... read command
#define   TX_TASK_6           0x0046    // write => ... read command
#define   TX_TASK_7           0x0047    // write => ... read command

#define   RX_TASK_0           0x0060    // read => first read data
#define   RX_TASK_1           0x0061    // read => ... read data
#define   RX_TASK_2           0x0062    // read => ... read data
#define   RX_TASK_3           0x0063    // read => ... read data
#define   RX_TASK_4           0x0064    // read => ... read data
#define   RX_TASK_5           0x0065    // read => ... read data
#define   RX_TASK_6           0x0066    // read => ... read data
#define   RX_TASK_7           0x0067    // read => last read data
#define   TASK_STATUS         0x0080    // bitmap of rx regs, 1: data received
#define   EV_FILT_FIRST       0x1007    // first event filter (ram) address.
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


inline int scub_write_mil_blk(volatile unsigned short *base, int slot, short *data, short fc_ifc_addr) {
  int i;
  atomic_on();
  //if (scub_trm_free(base, slot) == OKAY) {
  base[CALC_OFFS(slot) + MIL_RD_WR_DATA] = data[0];
  //} else {
    //atomic_off();
    //return TRM_NOT_FREE;
  //}
  //if (scub_trm_free(base, slot) == OKAY) {
    base[CALC_OFFS(slot) + MIL_WR_CMD] = fc_ifc_addr;
  //} else {
    //atomic_off();
    //return TRM_NOT_FREE;
  //}

  for (i = 1; i < 6; i++) {
    //if (scub_trm_free(base, slot) == OKAY) {
      base[CALC_OFFS(slot) + MIL_RD_WR_DATA] = data[i];
    //} else {
      //atomic_off();
      //return TRM_NOT_FREE;
    //}
  }
  atomic_off();
  return OKAY;
}

inline int scub_write_mil(volatile unsigned short *base, int slot, short data, short fc_ifc_addr) {
  atomic_on();
  //if (scub_trm_free(base, slot) == OKAY) {
    base[CALC_OFFS(slot) + MIL_RD_WR_DATA ] = data;
  //} else {
    //atomic_off();
    //return TRM_NOT_FREE;
  //}
  //if (scub_trm_free(base, slot) == OKAY) {
    base[CALC_OFFS(slot) + MIL_WR_CMD] = fc_ifc_addr;
    atomic_off();
    return OKAY;
  //} else {
    //atomic_off();
    //return TRM_NOT_FREE;
  //}
}

/***********************************************************
 ***********************************************************
 * 
 * 2st part:  (new) MIL bus library
 * 
 * There are two use cases for the MIL bus implemented in
 * this library.
 * 1. Device bus. There is only one bus master. Cables have
 *    Canon plugs at both ends. Devices (slaves) are address
 *    via an interface board address and a function code.
 * 2. Event bus. There is only one bus master. Cables have a
 *    Canon plug at the master side and a bipolar LEMO plug
 *    at the receiver (slave) side. Telegrams are distributed
 *    via broadcast from the master. This library implements
 *    the functionality to receive telegrams. Three types
 *    of actions are supported upon receival of an event.
 *     1. event data via FIFO
 *     2. generation of a single pulse with 500ns length
 *     3. generation of a gate; 
 *        - a 1st event starts the gate
 *        - a 2nd event stops the gate
 *    The type of action can be assigned via a filter. 
 *    A dedicated filter RAM provides 4096 filters 
 *    4096: 16 virtual accelerators x 256 event codes
 *
 ***********************************************************
 ***********************************************************/

/***********************************************************
 * 
 * mil error codes
 * 
 ***********************************************************/
#define   MIL_STAT_OK               OKAY
#define   MIL_STAT_TRM_NOT_FREE     TRM_NOT_FREE
#define   MIL_STAT_RCV_ERROR        RCV_ERROR   
#define   MIL_STAT_RCV_TIMEOUT      RCV_TIMEOUT 
#define   MIL_STAT_ERROR            -4
#define   MIL_STAT_OUT_OF_RANGE     -5

/***********************************************************
 * 
 * mil function codes for device bus
 * 
 ***********************************************************/
#define  MIL_FC_WR_IFC_ECHO FC_WR_IFC_ECHO
#define  MIL_FC_RD_IFC_ECHO FC_RD_IFC_ECHO

/***********************************************************
* routines for MIL device bus (Canon socket)      
************************************************************/

// write to MIL device bus; returns error code                                                             
int16_t writeDevMil(volatile uint32_t *base,            // Wishbone address seen from the CPUs perspective 
                    uint16_t  ifbAddr,                  // MIL address of interface board                  
                    uint16_t  fctCode,                  // function code                                   
                    uint16_t  data                      // data                                            
                    );

// read from MIL device bus; returns error code                                                            
int16_t readDevMil(volatile uint32_t *base,             // Wishbone address seen from the CPUs perspective 
                   uint16_t  ifbAddr,                   // MIL address of interface board                  
                   uint16_t  fctCode,                   // function code                                   
                   uint16_t  *data                      // data                                            
                   );

// write data to the echo register of a MIL device, then read and compare the data; returns error code                                                            
int16_t echoTestDevMil(volatile uint32_t *base,         // Wishbone address seen from the CPUs perspective 
                    uint16_t  ifbAddr,                  // MIL address of interface board                  
                    uint16_t  data                      // data                                            
                    );

/***********************************************************
* routines for MIL event bus receiver (bipolar LEMO socket)
************************************************************/

/* clear filter RAM; returns error code */
int16_t clearFilterEvtMil(volatile uint32_t *base      // Wishbone address seen from the CPUs perspective 
                          );

/* set a filter; returns error code */
int16_t setFilterEvtMil(volatile uint32_t *base,       // Wishbone address seen from the CPUs perspective 
                        uint16_t evtCode,              // event code                                      
                        uint16_t virtAcc,              // virtual accelerator                             
                        uint32_t filter                // filter value                                    
                        );

/* enable all filters, filtering is either ON or OFF for all filters; returns error code */
int16_t enableFilterEvtMil(volatile uint32_t *base     // Wishbone address seen from the CPUs perspective 
                           );

/* disble all filters, filtering is either ON or OFF for all filters; returns error code */
int16_t disableFilterEvtMil(volatile uint32_t *base    // Wishbone address seen from the CPUs perspective 
                            );

/* write to status and control register; returns error code */
int16_t writeCtrlStatRegEvtMil(volatile uint32_t *base,// Wishbone address seen from the CPUs perspective 
                               uint32_t value          // register value                                  
                               );
 
/* read from status and control register; returns error code */
int16_t readCtrlStatRegEvtMil(volatile uint32_t *base,    // Wishbone address seen from the CPUs perspective 
                              uint32_t *value             // register value                                  
                              );

/* query fill state of event FIFO; returns '1' if not empty */
uint16_t fifoNotemptyEvtMil(volatile uint32_t *base      // Wishbone address seen from the CPUs perspective 
                           );

/* remove all elements from the FIFO; returns error code */
int16_t clearFifoEvtMil(volatile uint32_t *base         // Wishbone address seen from the CPUs perspective 
                    );

/* pop on element of the FIFO; returns error code */
int16_t popFifoEvtMil(volatile uint32_t *base,          // Wishbone address seen from the CPUs perspective 
                      uint32_t *evtData                 // data of an event                                
                      );

/*configure a single ended LEMO for pulse generation, remember to set a filter too; returns error code */
int16_t configLemoPulseEvtMil(volatile uint32_t *base, // Wishbone address seen from the CPUs perspective 
                              uint32_t lemo            // select LEMO 1..4                                
                              );

/* configure a single ended LEMO for gate generation, remember to set a filter too; returns error code */
int16_t configLemoGateEvtMil(volatile uint32_t *base,   // Wishbone address seen from the CPUs perspective 
                             uint32_t lemo              // select LEMO 1..4                                
                             );

/* configure a single ended LEMO for programmable output (not controlled via event) */
int16_t configLemoOutputEvtMil(volatile uint32_t *base,   // Wishbone address seen from the CPUs perspective 
                               uint32_t lemo              // select LEMO 1..4                                
                               );


/* set the output value of a single ended LEMO programmatically (not controlled via event) */
int16_t setLemoOutputEvtMil(volatile uint32_t *base,   // Wishbone address seen from the CPUs perspective 
                            uint32_t lemo,             // select LEMO 1..4
                            uint32_t on                // 1: on, 0: off
                            );

/* disable a single ended LEMO output; returns error code */
int16_t disableLemoEvtMil(volatile uint32_t *base,      // Wishbone address seen from the CPUs perspective 
                          uint32_t lemo                 // select LEMO 1..4                                
                          );


/***********************************************************
 * 
 * mil registers available via Wishbone
 * 
 ***********************************************************/
#define   MIL_REG_RD_WR_DATA         MIL_RD_WR_DATA   << 2  // read or write mil bus; only 32bit access alowed; data[31..16] don't care
#define   MIL_REG_WR_CMD             MIL_WR_CMD       << 2  // write command to mil bus; only 32bit access alowed; data[31..16] don't care
#define   MIL_REG_WR_RD_STATUS       MIL_WR_RD_STATUS << 2  // read mil status register; only 32bit access alowed; data[31..16] don't care
                                                            // write mil control register; only 32bit access alowed; data[31..16] don't care
#define   MIL_REG_RD_CLR_NO_VW_CNT   RD_CLR_NO_VW_CNT << 2  // use only when FPGA Manchester Endecoder is enabled;
                                                            // read => valid word error counters; write => clears valid word error counters
#define   MIL_REG_RD_WR_NOT_EQ_CNT   RD_WR_NOT_EQ_CNT << 2  // use only when FPGA Manchester Endecoder is enabled;
                                                            // read => rcv pattern not equal errors; write => clears rcv pattern not equal errors
#define   MIL_REG_RD_CLR_EV_FIFO     RD_CLR_EV_FIFO << 2    // read => event fifo; write clears event fifo.
#define   MIL_REG_RD_CLR_TIMER       RD_CLR_TIMER << 2      // read => event timer; write clear event timer.
#define   MIL_REG_RD_WR_DLY_TIMER    RD_WR_DLY_TIMER << 2   // read => delay timer; write set delay timer.
#define   MIL_REG_RD_CLR_WAIT_TIMER  RD_CLR_WAIT_TIMER << 2 // read => wait timer; write => clear wait timer.
#define   MIL_REG_WR_RF_LEMO_CONF    0x0024                 // read => mil lemo config; write ==> mil lemo config (definition of bits: see below)
#define   MIL_REG_WR_RD_LEMO_DAT     0x0028                 // read => mil lemo data; write ==> mil lemo data
#define   MIL_REG_RD_LEMO_INP_A      0x002C                 // read mil lemo input
#define   MIL_REG_EV_FILT_FIRST      EV_FILT_FIRST << 2     // first event filter (ram) address.
#define   MIL_REG_EV_FILT_LAST       EV_FILT_LAST << 2      // last event filter (ram) addres.
                                                            // the filter RAM has a size of 4096 elements
                                                            // each element has a size of 4 bytes
                                                            // (definition of filter bits: see below)
                                                            // the filter RAM is addressed in the following way
                                                            //    uint32_t *pFilter; 
                                                            //    pFilter[virtAcc * 256 + evtCode]


/***********************************************************
 * 
 * mil control and status register
 * 
 * bits 0..15: see below
 * bits 16..31: unused
 *
 ***********************************************************/
#define   MIL_CTRL_STAT_INTERLOCK_INTR  MIL_INTERLOCK_INTR  // '1' => interlock interrupt is active
#define   MIL_CTRL_STAT_DATA_RDY_INTR   MIL_DATA_RDY_INTR   // '1' => data ready interrupt is active
#define   MIL_CTRL_STAT_DATA_REQ_INTR   MIL_DATA_REQ_INTR   // '1' => data request interrupt is active
#define   MIL_CTRL_STAT_EV_FIFO_NE      MIL_EV_FIFO_NE      // '1' => event fifo is not empty
#define   MIL_CTRL_STAT_EV_FIFO_FULL    MIL_EV_FIFO_FULL    // '1' => event fifo is full
#define   MIL_CTRL_STAT_RCV_READY       MIL_RCV_READY       // '1' => mil received data/commands
#define   MIL_CTRL_STAT_CMD_RCV         MIL_CMD_RCV         // '1' => received pattern is a command
#define   MIL_CTRL_STAT_TRM_READY       MIL_TRM_READY       // '1' => mil is free to transmit a pattern
#define   MIL_CTRL_STAT_RCV_ERROR       MIL_RCV_ERROR       // '1' => mil received an error
#define   MIL_CTRL_STAT_EV_RESET_ON     MIL_EV_RESET_ON     //
#define   MIL_CTRL_STAT_PULS1_FRAME     MIL_PULS1_FRAME     // '1' => one event (defined in event filter) switch puls1 on, an other event switch puls1 off
                                                            // '0' => one event (defined in event filter) generates a pulse on frontpannel lemo1
#define   MIL_CTRL_STAT_PULS2_FRAME     MIL_PULS2_FRAME     // '1' => one event (defined in event filter) switch puls2 on, an other event switch puls2 off
                                                            // '0' => one event (defined in event filter) generates a pulse on frontpannel lemo2
#define   MIL_CTRL_STAT_INTR_DEB_ON     MIL_INTR_DEB_ON     // '1' => deboune devicebus interrupts is on
#define   MIL_CTRL_STAT_EV_FILTER_ON    MIL_EV_FILTER_ON    // '1' => event filter is generally enabled; you must programm the event filter
#define   MIL_CTRL_STAT_EV_12_8B        MIL_EV_12_8B        // '1' => event decoding 12 bit; '0' => event decoding 8 bit
#define   MIL_CTRL_STAT_ENDECODER_FPGA  MIL_ENDECODER_FPGA  // '1' => use manchester en/decoder in fpga; '0' => use external en/decoder 6408


/***********************************************************
 * 
 * defintion of event filter
 * 
 * bits 0..5: see below
 * bits 6..31: unused
 *
 ***********************************************************/
#define   MIL_FILTER_EV_TO_FIFO   0x0001    // '1' => incoming event is written to FIFO
#define   MIL_FILTER_EV_TIMER_RES 0x0002    // '1' => if bit MIL_EV_RESET_ON is set, an incoming event resets the event timer
#define   MIL_FILTER_EV_PULS1_S   0x0004    // '1' => incoming event controls LEMO1 output
                                            //        - if MIL_PULS1_FRAME: start of gate ("Rahmenpuls")
                                            //        - else:               start of event pulse counter for ev_pulse1
#define   MIL_FILTER_EV_PULS1_E   0x0008    // '1' => incoming event controls LEMO1 output
                                            //        - if MIL_PULS1_FRAME: stop gate ("Rahmenpuls")
#define   MIL_FILTER_EV_PULS2_S   0x0010    // '1' => incoming event controls LEMO2 output                            
                                            //        - if MIL_PULS1_FRAME: start of gate ("Rahmenpuls")       
                                            //        - else:               start of event pulse counter for ev_pulse1
#define   MIL_FILTER_EV_PULS2_E   0x0020    //  1' => incoming event controls LEMO1 output                    
                                            //        - if MIL_PULS1_FRAME: stop of gate ("Rahmenpuls")

/***********************************************************
 * 
 * defintion of LEMO config register
 * 
 * bits 0..7: see below
 * bits 8..31: unused
 *
 ***********************************************************/
#define   MIL_LEMO_OUT_EN1    0x0001    // '1' ==> LEMO 1 configured as output (MIL Piggy)
#define   MIL_LEMO_OUT_EN2    0x0002    // '1' ==> LEMO 2 configured as output (MIL Piggy)
#define   MIL_LEMO_OUT_EN3    0x0004    // '1' ==> LEMO 3 configured as output (SIO)
#define   MIL_LEMO_OUT_EN4    0x0008    // '1' ==> LEMO 4 configured as output (SIO)
#define   MIL_LEMO_EVENT_EN1  0x0010    // '1' ==> LEMO 1 can be controlled by event (MIL Piggy)
#define   MIL_LEMO_EVENT_EN2  0x0020    // '1' ==> LEMO 2 can be controlled by event (MIL Piggy)
#define   MIL_LEMO_EVENT_EN3  0x0040    // '1' ==> LEMO 3 can be controlled by event (unused?)
#define   MIL_LEMO_EVENT_EN4  0x0080    // '1' ==> LEMO 4 can be controlled by event (unused?) 


/***********************************************************
 * 
 * defintion of LEMO data register
 * in case LEMO outputs are not controlled via events,
 * this register can be used to control them
 * 
 * bits 0..3: see below
 * bits 4..31: unused
 *
 ***********************************************************/
#define   MIL_LEMO_DAT1    0x0001    // '1' ==> LEMO 1 is switched active HIGH (MIL Piggy & SIO)
#define   MIL_LEMO_DAT2    0x0002    // '1' ==> LEMO 2 is switched active HIGH (MIL Piggy & SIO)
#define   MIL_LEMO_DAT3    0x0004    // '1' ==> LEMO 3 is switched active HIGH (SIO)
#define   MIL_LEMO_DAT4    0x0008    // '1' ==> LEMO 4 is switched active HIGH (SIO)


#endif
