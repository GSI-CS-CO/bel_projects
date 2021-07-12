#ifndef __SCU_MIL_H_
#define __SCU_MIL_H_

#include <stdint.h>
#include <board.h>
#include <mprintf.h>
#include <syscon.h>
#ifdef _CONFIG_OLD_IRQ
 #include <aux.h>
#else
 #include <lm32Interrupts.h>
#endif

/*!***********************************************************
 * @file scu_mil.h MIL bus library
 * @author  Wolfgang Panschow
 * @see https://www-acc.gsi.de/wiki/bin/viewauth/Hardware/Intern/PerfOpt
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


//extern int usleep(useconds_t usec);

/*!
 * @defgroup MIL_INTERFACE Functions and constants for MIL-bus.
 * @{
 */

int write_mil(volatile unsigned int *base, short data, short fc_ifk_addr);
int read_mil(volatile unsigned int *base, short *data, short fc_ifk_addr);
int status_mil(volatile unsigned int *base, unsigned short *status);

/*!
 * @brief Writs a data block of 16 bit data words to the mil device.
 * @see MIL_BLOCK_SIZE
 */
int write_mil_blk(volatile unsigned int *base, short *data, short fc_ifc_addr);
int scub_status_mil(volatile unsigned short *base, int slot, unsigned short *status);
int scub_read_mil(volatile unsigned short *base, int slot, short *data, short fc_ifc_addr);
int set_task_mil(volatile unsigned int *base, unsigned char task, short fc_ifc_addr);
int get_task_mil(volatile unsigned int *base, unsigned char task, short *data);
int scub_set_task_mil(volatile unsigned short int *base, int slot, unsigned char task, short fc_ifc_addr);
int scub_get_task_mil(volatile unsigned short int *base, int slot, unsigned char task, short *data);
int scub_reset_mil(volatile unsigned short *base, int slot);
int reset_mil(volatile unsigned *base);



#define MAX_TST_CNT       10000
#define FC_WR_IFC_ECHO    0x13
#define FC_RD_IFC_ECHO    0x89
#define SCU_MIL           0x35aa6b96
#define MIL_SIO3_OFFSET   0x400
#define MIL_SIO3_TX_DATA  0x400
#define MIL_SIO3_TX_CMD   0x401
#define MIL_SIO3_STAT     0x402
#define MIL_SIO3_RST      0x412
#define MIL_SIO3_RX_TASK1 0xd01
#define MIL_SIO3_TX_TASK1 0xc01
#define MIL_SIO3_RX_TASK2 0xd02
#define MIL_SIO3_TX_TASK2 0xc02
#define MIL_SIO3_D_RCVD   0xe00
#define MIL_SIO3_D_ERR    0xe10
#define MIL_SIO3_TX_REQ   0xe20
#define CALC_OFFS(SLOT)   (((SLOT) * (1 << 16))) // from slot 1 to slot 12
#define TASKMIN           1
#define TASKMAX           254
#define TASK_TIMEOUT 150
#define BLOCK_TIMEOUT 150
/*
  +---------------------------------------------+
  |   mil communication error codes             |
  +---------------------------------------------+
*/
#define   OKAY                 1
#define   TRM_NOT_FREE        -1
#define   RCV_ERROR           -2
#define   RCV_TIMEOUT         -3
#define   RCV_TASK_ERR        -4
#define   RCV_PARITY          -5
#define   ERROR               -6
#define   RCV_TASK_BSY        -7


/*
  +---------------------------------------------+
  |       mil register addresses                |
  | ATTENTION: the addresses are in uint32_t !! | 
  | This is different to typical header files,  |
  | where addresses are in uint8_t              |
  +---------------------------------------------+
*/
#define   MIL_RD_WR_DATA      0x0000    //!<@brief read or write mil bus; only 32bit access alowed; data[31..16] don't care
#define   MIL_WR_CMD          0x0001    //!<@brief write command to mil bus; only 32bit access alowed; data[31..16] don't care
#define   MIL_WR_RD_STATUS    0x0002    //!<@brief read mil status register; only 32bit access alowed; data[31..16] don't care
                                        //!<@brief write mil control register; only 32bit access alowed; data[31..16] don't care
#define   RD_CLR_NO_VW_CNT    0x0003    //!<@brief use only when FPGA Manchester Endecoder is enabled;
                                        //!<@brief read => valid word error counters; write => clears valid word error counters
#define   RD_WR_NOT_EQ_CNT    0x0004    //!<@brief use only when FPGA Manchester Endecoder is enabled;
                                        //!<@brief read => rcv pattern not equal errors; write => clears rcv pattern not equal errors
#define   RD_CLR_EV_FIFO      0x0005    //!<@brief read => event fifo; write clears event fifo.
#define   RD_CLR_TIMER        0x0006    //!<@brief read => event timer; write clear event timer.
#define   RD_WR_DLY_TIMER     0x0007    //!<@brief read => delay timer; write set delay timer.
#define   RD_CLR_WAIT_TIMER   0x0008    //!<@brief read => wait timer; write => clear wait timer.
#define   MIL_WR_RD_LEMO_CONF 0x0009    //!<@brief read/write lemo config register
#define   MIL_WR_RD_LEMO_DAT  0x000A    //!<@brief read/write lemo output data register
#define   MIL_RD_LEMO_INP     0x000B    //!<@brief read pin status at lemo pins
#define   EV_FILT_FIRST       0x1000    //!<@brief first event filter (ram) address.
#define   EV_FILT_LAST        0x1FFF    //!<@brief last event filter (ram) addres.

/*
  +---------------------------------------------+
  |       mil status register layout            |
  +---------------------------------------------+
*/
#define   MIL_INTERLOCK_INTR  0x0001    //!<@brief '1' => interlock interrupt is active
#define   MIL_DATA_RDY_INTR   0x0002    //!<@brief '1' => data ready interrupt is active
#define   MIL_DATA_REQ_INTR   0x0004    //!<@brief '1' => data request interrupt is active
#define   MIL_EV_FIFO_NE      0x0008    //!<@brief '1' => event fifo is not empty
#define   MIL_EV_FIFO_FULL    0x0010    //!<@brief '1' => event fifo is full
#define   MIL_RCV_READY       0x0020    //!<@brief '1' => mil received data/commands
#define   MIL_CMD_RCV         0x0040    //!<@brief '1' => received pattern is a command
#define   MIL_TRM_READY       0x0080    //!<@brief '1' => mil is free to transmit a pattern
#define   MIL_RCV_ERROR       0x0100    //!<@brief '1' => mil received an error
#define   MIL_EV_RESET_ON     0x0200    //
#define   MIL_PULS1_FRAME     0x0400    //!<@brief '1' => one event (defined in event filter) switch puls1 on, an other event switch puls1 off
                                        //! '0' => one event (defined in event filter) generates a pulse on frontpannel lemo1
#define   MIL_PULS2_FRAME     0x0800    //!<@brief '1' => one event (defined in event filter) switch puls2 on, an other event switch puls2 off
                                        //! '0' => one event (defined in event filter) generates a pulse on frontpannel lemo2
#define   MIL_INTR_DEB_ON     0x1000    //!<@brief '1' => deboune devicebus interrupts is on
#define   MIL_EV_FILTER_ON    0x2000    //!<@brief '1' => event filter is generally enabled; you must programm the event filter
#define   MIL_EV_12_8B        0x4000    //!<@brief '1' => event decoding 12 bit; '0' => event decoding 8 bit
#define   MIL_ENDECODER_FPGA  0x8000    //!<@brief '1' => use manchester en/decoder in fpga; '0' => use external en/decoder 6408

#define MIL_BLOCK_SIZE 6 //!<@brief MIL data block length in 16-bit words (uint16_t).

/*!
 * @brief Writes a data-block of 16-bit words to the MIL device via SCU bus.
 * @see MIL_BLOCK_SIZE
 */
static
inline int scub_write_mil_blk(volatile unsigned short *base, int slot, short *data, short fc_ifc_addr) {
  int i;
  atomic_on();
  base[CALC_OFFS(slot) + MIL_SIO3_TX_DATA] = data[0];
  base[CALC_OFFS(slot) + MIL_SIO3_TX_CMD] = fc_ifc_addr;

  for (i = 1; i < MIL_BLOCK_SIZE; i++) {
      base[CALC_OFFS(slot) + MIL_SIO3_TX_DATA] = data[i];
  }
  atomic_off();
  return OKAY;
}

static
inline int scub_write_mil(volatile unsigned short *base, int slot, short data, short fc_ifc_addr) {
    atomic_on();
    base[CALC_OFFS(slot) + MIL_SIO3_TX_DATA ] = data;
    base[CALC_OFFS(slot) + MIL_SIO3_TX_CMD] = fc_ifc_addr;
    atomic_off();
    return OKAY;
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

//!@brief write to MIL device bus; returns error code
int16_t writeDevMil(volatile uint32_t *base,            //!<@brief Wishbone address seen from the CPUs perspective
                    uint16_t  ifbAddr,                  //!<@brief MIL address of interface board
                    uint16_t  fctCode,                  //!<@brief function code
                    uint16_t  data                      //!<@brief data
                    );

//!@brief read from MIL device bus; returns error code
int16_t readDevMil(volatile uint32_t *base,             //!<@brief Wishbone address seen from the CPUs perspective
                   uint16_t  ifbAddr,                   //!<@brief MIL address of interface board
                   uint16_t  fctCode,                   //!<@brief function code
                   uint16_t  *data                      //!<@brief data
                   );

//!@brief write data to the echo register of a MIL device, then read and compare the data; returns error code
int16_t echoTestDevMil(volatile uint32_t *base,         //!<@brief Wishbone address seen from the CPUs perspective
                    uint16_t  ifbAddr,                  //!<@brief MIL address of interface board
                    uint16_t  data                      //!<@brief data
                    );

//!@brief reset device bus part of MIL piggy
int16_t resetPiggyDevMil(volatile uint32_t *base);      //!<@brief Wishbone address seen from the CPUs perspective

/***********************************************************
* routines for MIL event bus receiver (bipolar LEMO socket)
************************************************************/

/*!@brief clear filter RAM; returns error code */
int16_t clearFilterEvtMil(volatile uint32_t *base      //!<@brief Wishbone address seen from the CPUs perspective
                          );

/*!@brief set a filter; returns error code */
int16_t setFilterEvtMil(volatile uint32_t *base,       //!<@brief Wishbone address seen from the CPUs perspective
                        uint16_t evtCode,              //!<@brief event code
                        uint16_t virtAcc,              //!<@brief virtual accelerator
                        uint32_t filter                //!<@brief filter value
                        );

/*!@brief enable all filters, filtering is either ON or OFF for all filters; returns error code */
int16_t enableFilterEvtMil(volatile uint32_t *base     //!<@brief Wishbone address seen from the CPUs perspective
                           );

/*!@brief disble all filters, filtering is either ON or OFF for all filters; returns error code */
int16_t disableFilterEvtMil(volatile uint32_t *base    //!<@brief Wishbone address seen from the CPUs perspective
                            );

/*!@brief write to status and control register; returns error code */
int16_t writeCtrlStatRegEvtMil(volatile uint32_t *base,//!<@brief Wishbone address seen from the CPUs perspective
                               uint32_t value          //!<@brief register value
                               );
 
/*!@brief read from status and control register; returns error code */
int16_t readCtrlStatRegEvtMil(volatile uint32_t *base,    //!<@brief Wishbone address seen from the CPUs perspective
                              uint32_t *value             //!<@brief register value
                              );

/*!@brief query fill state of event FIFO; returns '1' if not empty */
uint16_t fifoNotemptyEvtMil(volatile uint32_t *base      //!<@brief Wishbone address seen from the CPUs perspective
                           );

/*!@brief remove all elements from the FIFO; returns error code */
int16_t clearFifoEvtMil(volatile uint32_t *base         //!<@brief Wishbone address seen from the CPUs perspective
                    );

/*!@brief pop on element of the FIFO; returns error code */
int16_t popFifoEvtMil(volatile uint32_t *base,          //!<@brief Wishbone address seen from the CPUs perspective
                      uint32_t *evtData                 //!<@brief data of an event
                      );

/*!@brief configure a single ended LEMO for pulse generation, remember to set a filter too; returns error code */
int16_t configLemoPulseEvtMil(volatile uint32_t *base, // Wishbone address seen from the CPUs perspective 
                              uint32_t lemo            // select LEMO 1..4                                
                              );

/*!@brief configure a single ended LEMO for gate generation, remember to set a filter too; returns error code */
int16_t configLemoGateEvtMil(volatile uint32_t *base,   //!<@brief Wishbone address seen from the CPUs perspective
                             uint32_t lemo              //!<@brief select LEMO 1..4
                             );

/*!@brief configure a single ended LEMO for programmable output (not controlled via event) */
int16_t configLemoOutputEvtMil(volatile uint32_t *base,   //!<@brief Wishbone address seen from the CPUs perspective
                               uint32_t lemo              //!<@brief select LEMO 1..4
                               );


/*!@brief set the output value of a single ended LEMO programmatically (not controlled via event) */
int16_t setLemoOutputEvtMil(volatile uint32_t *base,   //!<@brief Wishbone address seen from the CPUs perspective
                            uint32_t lemo,             //!<@brief select LEMO 1..4
                            uint32_t on                //!<@brief 1: on, 0: off
                            );

/*!@brief disable a single ended LEMO output; returns error code */
int16_t disableLemoEvtMil(volatile uint32_t *base,      //!<@brief Wishbone address seen from the CPUs perspective
                          uint32_t lemo                 //!<@brief select LEMO 1..4
                          );


/***********************************************************
 * 
 * mil registers available via Wishbone
 * 
 ***********************************************************/
#define   MIL_REG_RD_WR_DATA         (MIL_SIO3_OFFSET + MIL_RD_WR_DATA)   << 2    //!<@brief read or write mil bus; only 32bit access alowed; data[31..16] don't care
#define   MIL_REG_WR_CMD             (MIL_SIO3_OFFSET + MIL_WR_CMD)       << 2    //!<@brief write command to mil bus; only 32bit access alowed; data[31..16] don't care
#define   MIL_REG_WR_RD_STATUS       (MIL_SIO3_OFFSET + MIL_WR_RD_STATUS) << 2    //!<@brief read mil status register; only 32bit access alowed; data[31..16] don't care
                                                                                  //! write mil control register; only 32bit access alowed; data[31..16] don't care
#define   MIL_REG_RD_CLR_NO_VW_CNT   (MIL_SIO3_OFFSET + RD_CLR_NO_VW_CNT) << 2    //!<@brief use only when FPGA Manchester Endecoder is enabled;
                                                                                  //! read => valid word error counters; write => clears valid word error counters
#define   MIL_REG_RD_WR_NOT_EQ_CNT   (MIL_SIO3_OFFSET + RD_WR_NOT_EQ_CNT) << 2    //!<@brief use only when FPGA Manchester Endecoder is enabled;
                                                                                  //! read => rcv pattern not equal errors; write => clears rcv pattern not equal errors
#define   MIL_REG_RD_CLR_EV_FIFO     (MIL_SIO3_OFFSET + RD_CLR_EV_FIFO) << 2      //!<@brief read => event fifo; write clears event fifo.
#define   MIL_REG_RD_CLR_TIMER       (MIL_SIO3_OFFSET + RD_CLR_TIMER)   << 2      //!<@brief read => event timer; write clear event timer.
#define   MIL_REG_RD_WR_DLY_TIMER    (MIL_SIO3_OFFSET + RD_WR_DLY_TIMER) << 2     //!<@brief read => delay timer; write set delay timer.
#define   MIL_REG_RD_CLR_WAIT_TIMER  (MIL_SIO3_OFFSET + RD_CLR_WAIT_TIMER) << 2   //!<@brief read => wait timer; write => clear wait timer.
#define   MIL_REG_WR_RF_LEMO_CONF    (MIL_SIO3_OFFSET + MIL_WR_RD_LEMO_CONF) << 2 //!<@brief read/write lemo config register
#define   MIL_REG_WR_RD_LEMO_DAT     (MIL_SIO3_OFFSET + MIL_WR_RD_LEMO_DAT) << 2  //!<@brief read/write lemo output data register
#define   MIL_REG_RD_LEMO_INP_A      (MIL_SIO3_OFFSET + MIL_RD_LEMO_INP) << 2     //!<@brief read pin status at lemo pins
#define   MIL_REG_EV_FILT_FIRST      EV_FILT_FIRST << 2                           //!<@brief first event filter (ram) address.
#define   MIL_REG_EV_FILT_LAST       EV_FILT_LAST  << 2                           //!<@brief last event filter (ram) addres.
                                                                                  //! the filter RAM has a size of 4096 elements
                                                                                  //! each element has a size of 4 bytes
                                                                                  //! (definition of filter bits: see below)
                                                                                  //! the filter RAM is addressed in the following way
                                                                                  //!    uint32_t *pFilter;
                                                                                  //!    pFilter[virtAcc * 256 + evtCode]


/***********************************************************
 * 
 * mil control and status register
 * 
 * bits 0..15: see below
 * bits 16..31: unused
 *
 ***********************************************************/
#define   MIL_CTRL_STAT_INTERLOCK_INTR  MIL_INTERLOCK_INTR  //!<@brief '1' => interlock interrupt is active
#define   MIL_CTRL_STAT_DATA_RDY_INTR   MIL_DATA_RDY_INTR   //!<@brief '1' => data ready interrupt is active
#define   MIL_CTRL_STAT_DATA_REQ_INTR   MIL_DATA_REQ_INTR   //!<@brief '1' => data request interrupt is active
#define   MIL_CTRL_STAT_EV_FIFO_NE      MIL_EV_FIFO_NE      //!<@brief '1' => event fifo is not empty
#define   MIL_CTRL_STAT_EV_FIFO_FULL    MIL_EV_FIFO_FULL    //!<@brief '1' => event fifo is full
#define   MIL_CTRL_STAT_RCV_READY       MIL_RCV_READY       //!<@brief '1' => mil received data/commands
#define   MIL_CTRL_STAT_CMD_RCV         MIL_CMD_RCV         //!<@brief '1' => received pattern is a command
#define   MIL_CTRL_STAT_TRM_READY       MIL_TRM_READY       //!<@brief '1' => mil is free to transmit a pattern
#define   MIL_CTRL_STAT_RCV_ERROR       MIL_RCV_ERROR       //!<@brief '1' => mil received an error
#define   MIL_CTRL_STAT_EV_RESET_ON     MIL_EV_RESET_ON     //
#define   MIL_CTRL_STAT_PULS1_FRAME     MIL_PULS1_FRAME     //!<@brief '1' => one event (defined in event filter) switch puls1 on, an other event switch puls1 off
                                                            //! '0' => one event (defined in event filter) generates a pulse on frontpannel lemo1
#define   MIL_CTRL_STAT_PULS2_FRAME     MIL_PULS2_FRAME     //!<@brief '1' => one event (defined in event filter) switch puls2 on, an other event switch puls2 off \n
                                                            //! '0' => one event (defined in event filter) generates a pulse on frontpannel lemo2
#define   MIL_CTRL_STAT_INTR_DEB_ON     MIL_INTR_DEB_ON     //!<@brief '1' => deboune devicebus interrupts is on
#define   MIL_CTRL_STAT_EV_FILTER_ON    MIL_EV_FILTER_ON    //!<@brief '1' => event filter is generally enabled; you must programm the event filter
#define   MIL_CTRL_STAT_EV_12_8B        MIL_EV_12_8B        //!<@brief '1' => event decoding 12 bit; '0' => event decoding 8 bit
#define   MIL_CTRL_STAT_ENDECODER_FPGA  MIL_ENDECODER_FPGA  //!<@brief '1' => use manchester en/decoder in fpga; '0' => use external en/decoder 6408


/***********************************************************
 * 
 * defintion of event filter
 * 
 * bits 0..5: see below
 * bits 6..31: unused
 *
 ***********************************************************/
#define   MIL_FILTER_EV_TO_FIFO   0x0001    //!<@brief '1' => incoming event is written to FIFO
#define   MIL_FILTER_EV_TIMER_RES 0x0002    //!<@brief '1' => if bit MIL_EV_RESET_ON is set, an incoming event resets the event timer
#define   MIL_FILTER_EV_PULS1_S   0x0004    //!<@brief '1' => incoming event controls LEMO1 output \n
                                            //!        - if MIL_PULS1_FRAME: start of gate ("Rahmenpuls") \n
                                            //!        - else:               start of event pulse counter for ev_pulse1
#define   MIL_FILTER_EV_PULS1_E   0x0008    //!<@brief '1' => incoming event controls LEMO1 output \n
                                            //!        - if MIL_PULS1_FRAME: stop gate ("Rahmenpuls") \n
#define   MIL_FILTER_EV_PULS2_S   0x0010    //!<@brief '1' => incoming event controls LEMO2 output \n
                                            //!        - if MIL_PULS1_FRAME: start of gate ("Rahmenpuls") \n
                                            //!        - else:               start of event pulse counter for ev_pulse1
#define   MIL_FILTER_EV_PULS2_E   0x0020    //!<@brief  1' => incoming event controls LEMO1 output \n
                                            //!        - if MIL_PULS1_FRAME: stop of gate ("Rahmenpuls")

/***********************************************************
 * 
 * defintion of LEMO config register
 * 
 * bits 0..7: see below
 * bits 8..31: unused
 * chk: use cases for MIL and SIO
 ***********************************************************/
#define   MIL_LEMO_OUT_EN1    0x0001    //!<@brief '1' ==> LEMO 1 configured as output (MIL Piggy)
#define   MIL_LEMO_OUT_EN2    0x0002    //!<@brief '1' ==> LEMO 2 configured as output (MIL Piggy)
#define   MIL_LEMO_OUT_EN3    0x0004    //!<@brief '1' ==> LEMO 3 configured as output (SIO)
#define   MIL_LEMO_OUT_EN4    0x0008    //!<@brief '1' ==> LEMO 4 configured as output (SIO)
#define   MIL_LEMO_EVENT_EN1  0x0010    //!<@brief '1' ==> LEMO 1 can be controlled by event (MIL Piggy)
#define   MIL_LEMO_EVENT_EN2  0x0020    //!<@brief '1' ==> LEMO 2 can be controlled by event (MIL Piggy)
#define   MIL_LEMO_EVENT_EN3  0x0040    //!<@brief '1' ==> LEMO 3 can be controlled by event (unused?)
#define   MIL_LEMO_EVENT_EN4  0x0080    //!<@brief '1' ==> LEMO 4 can be controlled by event (unused?)


/***********************************************************
 * 
 * defintion of LEMO data register
 * in case LEMO outputs are not controlled via events,
 * this register can be used to control them
 * 
 * bits 0..3: see below
 * bits 4..31: unused
 * chk: use cases for MIL and SIO
 ***********************************************************/
#define   MIL_LEMO_DAT1    0x0001    //!<@brief '1' ==> LEMO 1 is switched active HIGH (MIL Piggy & SIO)
#define   MIL_LEMO_DAT2    0x0002    //!<@brief '1' ==> LEMO 2 is switched active HIGH (MIL Piggy & SIO)
#define   MIL_LEMO_DAT3    0x0004    //!<@brief '1' ==> LEMO 3 is switched active HIGH (SIO)
#define   MIL_LEMO_DAT4    0x0008    //!<@brief '1' ==> LEMO 4 is switched active HIGH (SIO)

//@}

#endif
