/*!
 * @file daq.h
 * @brief Control module for ADDAC Data Acquisition Unit (DAQ)
 * @see
 * <a href="https://www-acc.gsi.de/wiki/bin/viewauth/Hardware/Intern/DataAquisitionMacrof%C3%BCrSCUSlaveBaugruppen">
 *    Data Aquisition Macro fuer SCU Slave Baugruppen</a>
 * @see
 * <a href="https://www-acc.gsi.de/wiki/Hardware/Intern/StdRegScuBusSlave">
 *    Registersatz SCU-Bus-Slaves</a>
 *
 * @date 13.11.2018
 * @copyright (C) 2018 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 * @author Ulrich Becker <u.becker@gsi.de>
 *
 * @todo Synchronization with SCU-Bus. It could be there that further devices
 *       which have traffic via this SCU-Bus!
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
#ifndef _DAQ_H
#define _DAQ_H

#if !defined(__lm32__) && !defined(__DOCFSM__)
  #error This module is for the target LM32 only!
#endif

//#define CONFIG_DEBUG_DAQ_ENABLE

#ifdef CONFIG_DEBUG_DAQ_ENABLE
#include <mprintf.h>
#endif

#ifndef __DOCFSM__
 #include <stdbool.h>
 #include <scu_bus.h>
 #include <daq_descriptor.h>
 #ifndef CONFIG_DAQ_SINGLE_APP
  #include <scu_function_generator.h>
  #include <sw_queue.h>
 #endif
 #include <scu_syslog.h>
#endif

#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
#warning CAUTION: Module daq will compiled in simulation mode!!!
#endif

#ifdef CONFIG_DAQ_PEDANTIC_CHECK
   /* CAUTION:
    * Assert-macros could be expensive in memory consuming and the
    * latency time can increase as well!
    * Especially in embedded systems with small resources.
    * Therefore use them for bug-fixing or developing purposes only!
    */
   #include <scu_assert.h>
   #define DAQ_ASSERT SCU_ASSERT
#else
   #define DAQ_ASSERT(__e) ((void)0)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @ingroup SCU_BUS
 * @defgroup DAQ
 * Data AQisition unit
 */
/*!
 * @ingroup DAQ
 * @defgroup DAQ_SCU_BUS
 * @brief Objects and functions for a DAQ device container
 *                       that means the SCU bus.
 */
/*!
 * @ingroup DAQ_SCU_BUS
 * @defgroup DAQ_DEVICE
 * @brief Objects and functions for a DAQ device respectively \n
 *        a DAQ SCU bus slave.
 */
/*!
 * @ingroup DAQ_DEVICE
 * @defgroup DAQ_CHANNEL
 * @brief Objects and functions for a single DAQ channel
 */
/*!
 * @ingroup SCU_BUS INTERRUPT
 * @defgroup DAQ_INTERRUPT
 * @brief Interrupt functions concerning the DAQ.
 */

#ifndef DAQ_CID_SYS
/*!
 * @ingroup DAQ_DEVICE SCU_BUS
 * @brief DAQ device system ID
 */
 #define DAQ_CID_SYS   0x37
#endif

#ifndef DAQ_CID_GROUP
/*!
 * @ingroup DAQ_DEVICE SCU_BUS
 * @brief DAQ device group ID
 */
 #define DAQ_CID_GROUP 0x26
#endif

/*!
 * @ingroup DAQ_DEVICE DAQ_INTERRUPT
 * @brief Interrupt number for DAQ fifo full.
 */
#define DAQ_IRQ_DAQ_FIFO_FULL   12

/*!
 * @ingroup DAQ_DEVICE DAQ_INTERRUPT
 * @brief Interrupt number for High-Resolution finished
 */
#define DAQ_IRQ_HIRES_FINISHED  11

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL DAQ_DEVICE
 * @brief Access indexes for writing and reading the DAQ-registers
 * @see daqChannelGetReg
 * @see daqChannelSetReg
 */
typedef enum
{
   CtrlReg         = 0x00, //!< @see DAQ_CTRL_REG_T
   TRIG_LW         = 0x10, //!< @brief Least significant word for SCU-bus event
                           //!<        tag trigger condition.
   TRIG_HW         = 0x20, //!< @brief Most significant word for SCU-bus event
                           //!<        tag trigger condition.
   TRIG_DLY        = 0x30, //!< @brief Trigger delay in samples.
   PM_DAT          = 0x40, //!< @brief Data of PostMortem respectively HiRes-FiFos.
   DAQ_DAT         = 0x50, //!< @brief Data of DAQ-FiFo.
   DAQ_INTS        = 0x60, //!< @brief DAQ interrupt collect register.
   HIRES_INTS      = 0x61, //!< @brief HiRes interrupt collect register.
   TS_COUNTER_WD1  = 0x62, //!< @brief Timestamp counter preset word 1 bits [15:0]
   TS_COUNTER_WD2  = 0x63, //!< @brief Timestamp counter preset word 2 bits [31:16]
   TS_COUNTER_WD3  = 0x64, //!< @brief Timestamp counter preset word 3 bits [47:32]
   TS_COUNTER_WD4  = 0x65, //!< @brief Timestamp counter preset word 2 bits [63:48]
   TS_CNTR_TAG_LW  = 0x66, //!< @brief Timestamp counter tag bits [15:0]
   TS_CNTR_TAG_HW  = 0x67, //!< @brief Timestamp counter tag bits [31:16]
   DAQ_FIFO_WORDS  = 0x70, //!< @brief Remaining FiFo word of DaqDat FiFo.
   PM_FIFO_WORDS   = 0x80  //!< @brief Remaining FiFo word of PmDat FiFo.
} DAQ_REGISTER_INDEX_T;

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL DAQ_DEVICE
 * @brief Values for single bit respectively flag in bit-field structure.
 * @see DAQ_CTRL_REG_T
 */
typedef enum
{
   OFF =  0, //!< @brief Value for switch on.
   ON  =  1  //!< @brief Value for switch off.
} DAQ_SWITCH_STATE_T;

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL DAQ_DEVICE
 * @brief Data type of the control register for each channel.
 * @note Do not change the order of attributes! Its a Hardware image!
 * @see CtrlReg
 * @see DAQ_SWITCH_STATE_T
 *
 * CAUTION: Don't use a smaller type for a bit field element like
 *          uint16_t!
 */
typedef volatile struct
{
#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) || defined(__DOXYGEN__) || defined(__DOCFSM__)
   /*!
    *  @brief Bit [15:12] slot number, shall be initialized by software,
    *         will used for the DAQ-Descriptor-Word.
    */
   DAQ_REGISTER_T slot:        4;
   DAQ_REGISTER_T __unused__:  4; //!< @brief Bit [11:8] till now unused.
   __DAQ_BF_CONTROL_REGISTER_BITS
#else
   #error Big endian is requested for this bit- field structure!
#endif
} DAQ_CTRL_REG_T;
#ifndef __DOXYGEN__
STATIC_ASSERT( sizeof(DAQ_CTRL_REG_T) == sizeof(DAQ_REGISTER_T) );
#endif

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL DAQ_DEVICE
 * @brief Data type of DAQ register "Daq_Fifo_Words"
 * @see DAQ_FIFO_WORDS
 */
typedef volatile struct
{
#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) || defined(__DOXYGEN__) || defined(__DOCFSM__)
   DAQ_REGISTER_T version:     7; //!<@brief Version number of DAQ macro.
   DAQ_REGISTER_T fifoWords:   9; //!<@brief Remaining data words in PmDat Fifo
#else
   #error Big endian is requested for this bit- field structure!
#endif
} DAQ_DAQ_FIFO_WORDS_T;
#ifndef __DOXYGEN__
STATIC_ASSERT( sizeof(DAQ_DAQ_FIFO_WORDS_T) == sizeof(DAQ_REGISTER_T) );
#endif

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL DAQ_DEVICE
 * @brief Data type of DAQ register "PM_Fifo_Words"
 * @see PM_FIFO_WORDS
 */
typedef volatile struct
{
#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) || defined(__DOXYGEN__) || defined(__DOCFSM__)
   DAQ_REGISTER_T maxChannels: 6; //!< @brief Maximum number of used channels
   DAQ_REGISTER_T fifoWords:  10; //!< @brief Remaining data words in PmDat Fifo
#else
   #error Big endian is requested for this bit- field structure!
#endif
} DAQ_PM_FIFO_WORDS_T;
#ifndef __DOXYGEN__
STATIC_ASSERT( sizeof(DAQ_PM_FIFO_WORDS_T) == sizeof(DAQ_REGISTER_T) );
#endif

/*!
 * @brief Relative start address of the SCU registers
 *
 * Accessing to a SCU register will made as followed:
 * Absolute-register-address = SCU-bus-slave base_address + DAQ_REGISTER_OFFSET
 *                           + canal_number * sizeof(DAQ_REGISTER_T)
 */
#define DAQ_REGISTER_OFFSET 0x4000

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL DAQ_DEVICE
 * @brief Memory mapped IO-space of a DAQ macro
 */
typedef volatile union
{
   volatile DAQ_REGISTER_T
      i[(SCUBUS_SLAVE_ADDR_SPACE-DAQ_REGISTER_OFFSET)/sizeof(DAQ_REGISTER_T)];
   //volatile struct DAQ_DATA_NAME_T s;
} DAQ_REGISTER_ACCESS_T;
#ifndef __DOXYGEN__
STATIC_ASSERT( sizeof( DAQ_REGISTER_ACCESS_T ) ==
               (SCUBUS_SLAVE_ADDR_SPACE-DAQ_REGISTER_OFFSET) );
#endif

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Flag pool of channel properties
 */
typedef struct PACKED_SIZE
{
   /*!
    * @brief When true than this channel will not used.
    */
   bool notUsed:          1;

   /*!
    * @brief Help flag will go to true when
    *        post-mortem has bell switched off.
    *
    * This redundance is necessary to reduce the SCU-Buss accesses.
    * Unfortunately the post-mortem mode doesn't supports the interrupt line.
    *
    */
   bool postMortemEvent:  1;

   /*!
    * @brief Restarts the post mortem, after a copy cycle of post mortem data.
    */
   bool restart:          1;

} DAQ_CHANNEL_BF_PROPERTY_T;
#ifndef __DOXYGEN__
STATIC_ASSERT( sizeof( DAQ_CHANNEL_BF_PROPERTY_T ) == sizeof( uint8_t ) );
#endif

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Object represents a single channel of a DAQ.
 */
typedef struct PACKED_SIZE
{
   /*!
    * @brief Channel number [0..DAQ_MAX_CHANNELS-1]
    */
   uint8_t n;

   /*!
    * @brief Interrupt mask.
    *
    * In principle not necessary, but it accelerates the
    * concerning interrupt routine a bit.
    * @see DAQ_INT_PENDING_T
    */
   DAQ_REGISTER_T intMask;

   /*!
    * @brief Down counter to limit the receiving blocks in continuous mode.
    *
    * When the counter is zero, the endless receiving mode is
    * active until the continuous mode becomes disabled, else
    * the counter becomes decremented with each received block and
    * the channel becomes automatically disabled when the counter
    * reaches zero.
    */
   uint16_t blockDownCounter;

#ifdef _CONFIG_PATCH_DAQ_TIMESTAMP
   /*!
    * @brief White rabbit time-stamp of the corresponding interrupt of this channel.
    * @note This is a patch! Because of a perhaps erroneous DAQ-VHDL-macro.
    *       This shall be removed ASAP.
    */
    uint64_t timestamp;
#endif

#ifdef CONFIG_DAQ_SW_SEQUENCE
   /*!
    * @brief Sequence number respectively modulo 256 block counter for
    *        blocks in continuous mode.
    *
    * Becomes incremented for each received continuous data block.
    */
   uint8_t  sequenceContinuous;

   /*!
    * @brief Sequence number respectively modulo 256 block counter for
    *        blocks in high resolution or post mortem mode.
    *
    * Becomes incremented for each received hiRes or PM data block.
    */
   uint8_t  sequencePmHires;
#endif
   DAQ_CHANNEL_BF_PROPERTY_T properties; //!<@see DAQ_CHANNEL_PROPERTY_T
#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
   uint16_t          callCount;   //!<@brief For simulation purposes only!
   DAQ_DESCRIPTOR_T  simulatedDescriptor; //!<@brief For simulation purposes only!
#endif
} DAQ_CANNEL_T;

#ifndef CONFIG_DAQ_SINGLE_APP
/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_DEVICE
 * @brief Task type for the DAQ channels for the ADDAC function generator
 *        feedback of set- and actual values.
 */
typedef enum
{
   FB_OFF = 0, /*!<@brief Switch DAQ channels for FG-feedback on */
   FB_ON  = 1  /*!<@brief Switch DAQ channels for FG-feedback off */
} DAQ_FEEDBACK_ACTION_T;

#define FSM_DECLARE_STATE( state, attr... ) state

/*!
 * @ingroup DAQ_DEVICE
 * @brief Declaration of the states for the feedback switch FSM.
 */
typedef enum
{
   FSM_DECLARE_STATE( FB_READY, label='Wait for message.', color='green' ),
   FSM_DECLARE_STATE( FB_FIRST_ON, label='First DAQ channel is on', color='red' ),
   FSM_DECLARE_STATE( FB_BOTH_ON, label='Second DAQ channel is on', color='red' )
} DAQ_FEEDBACK_STATUS_T;

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_DEVICE
 * @brief Item type for action buffer.
 */
typedef struct PACKED_SIZE
{
  /*!
   * @brief Operation code.
   * @see DAQ_FEEDBACK_ACTION_T
   */
   DAQ_FEEDBACK_ACTION_T action;

  /*!
   * @brief Number of function generator.
   */
   unsigned int fgNumber;
} DAQ_ACTION_ITEM_T;

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_DEVICE
 * @brief Main data type for feedback on/off-switch FSM.
 * @see daqDeviceDoFeedbackSwitchOnOffFSM
 */
typedef struct
{ /*!
   * @brief Holds the earliest time point for the next
   *        switch event.
   */
   uint64_t              waitingTime;

  /*!
   * @brief Number of the concerning function generator
   *        for DAQ feedback.
   */
   unsigned int          fgNumber;

  /*!
   * @brief Holds the current status of the FSM.
   * @see daqDeviceDoFeedbackSwitchOnOffFSM
   */
   DAQ_FEEDBACK_STATUS_T status;

  /*!
   * @brief Waiting queue for the next switch actions coming from SAFT-LIB.
   * 
   * It is supposed the SAFT-LIB doesn't send more than MAX_FG_PER_SLAVE
   * commands to the same slave in a short time, therefore the queue has a
   * maximum capacity of MAX_FG_PER_SLAVE.
   * @var SW_QUEUE_T aktionBuffer
   * @see MAX_FG_PER_SLAVE
   * @see daqDeviceFeedBackReset
   * @see daqDevicePutFeedbackSwitchCommand
   * @see daqDeviceDoFeedbackSwitchOnOffFSM
   */
   QUEUE_IMPLEMENT( aktionBuffer, 2 * MAX_FG_PER_SLAVE, DAQ_ACTION_ITEM_T );
} DAQ_FEEDBACK_T;

#endif /* ifndef CONFIG_DAQ_SINGLE_APP */

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_DEVICE
 * @brief Object represents a single SCU-Bus slave including a DAQ
 */
typedef struct
{  /*!
    * @brief Keeps the information about the type of this device.
    */
   DAQ_DEVICE_TYP_T type;

   /*!
    * @brief Slot number residing this device in SCU-bus. (1 - 12)
    */
   unsigned int slot;

   /*!
    * @brief Number of DAQ-channels
    */
   unsigned int maxChannels;

   /*!
    * @brief Device number becomes valid after daqBusFindAndInitializeAll
    */
   unsigned int n;

   /*!
    * @brief Array of channel objects
    */
   DAQ_CANNEL_T aChannel[DAQ_MAX_CHANNELS];

   /*!
    * @brief Pointer to DAQ-registers (start of address space)
    */
   DAQ_REGISTER_ACCESS_T* volatile pReg;

#ifndef CONFIG_DAQ_SINGLE_APP
   /*!
    * @brief Administration of feedback channels for
    * ADDAC- function generators
    */
   DAQ_FEEDBACK_T feedback;
#endif
} DAQ_DEVICE_T;

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_SCU_BUS
 * @brief Object represents all on the SCU-bus connected DAQs.
 */
typedef struct
{
   //! @brief Mirror-flags of the used slots by DAQ-slaves
   SCUBUS_SLAVE_FLAGS_T  slotDaqUsedFlags;
   //! @brief Number of found DAQs
   unsigned int          foundDevices;
   //! @brief Holding of the last error for the Linux host.
   DAQ_LAST_STATUS_T     lastErrorState;
   //! @brief Array of all possible existing DAQs
   DAQ_DEVICE_T          aDaq[DAQ_MAX];
} DAQ_BUS_T;

/*======================== DAQ channel functions ============================*/

#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * CAUTION: Following prototypes are for simulation purposes only!
 *          In this mode some functions becomes bend to the following functions
 *          and not to a real hardware access!
 */
DAQ_DATA_T daqChannelPopPmFifoSimulate( register DAQ_CANNEL_T* pThis );
DAQ_DATA_T daqChannelPopDaqFifoSimulate( register DAQ_CANNEL_T* pThis );
unsigned int daqChannelGetPmFifoWordsSimulate( register DAQ_CANNEL_T* pThis );
unsigned int daqChannelGetDaqFifoWordsSimulate( register DAQ_CANNEL_T* pThis );
/*
 * End of prototypes for simulation!
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */
#endif /* ifdef CONFIG_DAQ_SIMULATE_CHANNEL */

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL DAQ_DEVICE
 * @brief Macro returns the pointer to the DAQ device object of type
 *        DAQ_DEVICE_T* belonging the given DAQ channel object.
 * @note <b>CAUTION</b>\n
 *       This macro works only if the channel object (pThis) is
 *       real content of the container DAQ_DEVICE_T. \n
 *       That means, the address range of the concerned object DAQ_CANNEL_T
 *       has to be within the address range of its container-object
 *       DAQ_DEVICE_T!\n
 *       But that's just the case in this library.
 *
 * @note <b>CAUTION</b>\n
 *       This macro will only work after the successful call of
 *       function "daqBusFindAndInitializeAll"
 *
 * This is a usual technique using often within the Linux kernel.
 *
 * @see CONTAINER_OF defined in helper_macros.h
 * @param pThis Pointer to the channel object
 * @return Pointer to the parent object of the given channel object of
 *         type DAQ_DEVICE_T*
 */
#define DAQ_CHANNEL_GET_PARENT_OF( pThis ) \
   CONTAINER_OF( pThis, DAQ_DEVICE_T, aChannel[pThis->n] )

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL DAQ_SCU_BUS
 * @brief Macro returns the pointer to the DAQ SCU bus object belonging
 *        the DAQ device with this channel object.
 *
 * @note <b>CAUTION</b>\n
 *       This macro will only work after the successful call of
 *       function "daqBusFindAndInitializeAll"
 *
 * @see DAQ_CHANNEL_GET_PARENT_OF
 * @see DAQ_DEVICE_GET_PARENT_OF
 * @param pThis Pointer to the channel object
 * @return Pointer to the grandparent object of the given DAQ channel object of
 *         type DAQ_BUS_T*
 */
#define DAQ_CHANNEL_GET_GRANDPARENT_OF( pThis ) \
   DAQ_DEVICE_GET_PARENT_OF( DAQ_CHANNEL_GET_PARENT_OF( pThis ) )

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Returns the pointer to the control registers of the DAQ-device
 *        containing this given DAQ-channel.
 * @note <b>CAUTION!</b>\n
 *       This function works only if the channel object (pThis) is
 *       real content of the container DAQ_DEVICE_T. \n
 *       That means, the address range of the concerned object DAQ_CANNEL_T
 *       has to be within the address range of its container-object
 *       DAQ_DEVICE_T!\n
 *       But that's just the case in this library.
 * @see DAQ_CHANNEL_GET_PARENT_OF
 * @see CONTAINER_OF defined in helper_macros.h
 * @param pThis Pointer to the channel object
 * @return Pointer to the control register.
 */
STATIC inline
DAQ_REGISTER_ACCESS_T* volatile daqChannelGetRegPtr( register DAQ_CANNEL_T* pThis )
{
   DAQ_ASSERT( pThis->n < DAQ_MAX_CHANNELS );
   return DAQ_CHANNEL_GET_PARENT_OF( pThis )->pReg;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Returns the SCU- bus address of the DAQ device belonging this
 *        channel.
 * @param pThis Pointer to the channel object
 * @return Slave base address of the DAQ device of this channel.
 */
STATIC inline
void* daqChannelGetScuBusSlaveBaseAddress( register DAQ_CANNEL_T* pThis )
{
   DAQ_ASSERT( (unsigned int)daqChannelGetRegPtr( pThis ) > DAQ_REGISTER_OFFSET );
  /*
   * Because the register access to the DAQ device is more frequent than
   * to the registers of the SCU slave, therefore the base address of the
   * DAQ registers are noted rather than the SCU bus slave address.
   */
   return ((void*)daqChannelGetRegPtr( pThis )) - DAQ_REGISTER_OFFSET;
}

/*! --------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Macro makes the index for memory mapped IO of the DQQ register space
 * @note For internal use only!
 * @see DAQ_REGISTER_ACCESS_T
 */
#define __DAQ_MAKE_INDEX(index) (index | pThis->n)

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief DAQ-register access helper macro for get- and set- functions
 *        of the DAQ- registers.
 * @param index Register index name @see DAQ_REGISTER_INDEX_T
 * @note For internal use only!
 */
#define __DAQ_GET_CHANNEL_REG( index ) \
   (daqChannelGetRegPtr(pThis)->i[__DAQ_MAKE_INDEX(index)])

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Macro verifies whether the access is within the allowed IO- range
 */
#if defined( CONFIG_DAQ_PEDANTIC_CHECK ) || defined(__DOXYGEN__)
  #define __DAQ_VERIFY_CHANNEL_REG_ACCESS( index ) \
      DAQ_ASSERT( __DAQ_MAKE_INDEX(index) < sizeof(DAQ_REGISTER_ACCESS_T) )
#else
  #define __DAQ_VERIFY_CHANNEL_REG_ACCESS( index )
#endif

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Returns a pointer to the control-register of the given channel-
 *        object.
 * @see daqChannelGetRegPtr
 * @see DAQ_CTRL_REG_T
 * @param pThis Pointer to the channel object
 * @return Pointer to control register bit field structure.
 */
ALWAYS_INLINE
STATIC inline
DAQ_CTRL_REG_T* daqChannelGetCtrlRegPtr( register DAQ_CANNEL_T* restrict pThis )
{
   DAQ_ASSERT( pThis != NULL );
   __DAQ_VERIFY_CHANNEL_REG_ACCESS( CtrlReg );
   return (DAQ_CTRL_REG_T*) &__DAQ_GET_CHANNEL_REG( CtrlReg );
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Get the slot number of the given DAQ-channel.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 * @return Slot number of the DAQ device where belonging this channel
 */
ALWAYS_INLINE
STATIC inline int daqChannelGetSlot( register DAQ_CANNEL_T* pThis )
{
   return daqChannelGetCtrlRegPtr( pThis )->slot;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Get the channel number in a range of 0 to 15 of this channel object.
 * @param pThis Pointer to the channel object.
 * @return Channel number.
 */
ALWAYS_INLINE
STATIC inline int daqChannelGetNumber( const register DAQ_CANNEL_T* pThis )
{
   return pThis->n;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Set a error state which can happen during reading the DAQ-FiFo.
 * @see DAQ_LAST_STATUS_T
 * @param pThis Pointer to the channel object.
 * @param state Error status.
 */
void daqChannelSetStatus( register DAQ_CANNEL_T* pThis, DAQ_REC_STAT_T state );

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Turns the 10 us sample mode on for the concerned channel.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 */
STATIC inline void daqChannelSample10usOn( register DAQ_CANNEL_T* pThis )
{
#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
   pThis->callCount = 0;
   pThis->simulatedDescriptor.name.cControl.daqMode   = true;
   pThis->simulatedDescriptor.name.cControl.hiResMode = false;
   pThis->simulatedDescriptor.name.cControl.pmMode    = false;
#else
   DAQ_CTRL_REG_T* pCtrl = daqChannelGetCtrlRegPtr( pThis );
   pCtrl->Sample1ms = OFF;
   pCtrl->Sample100us = OFF;
   pCtrl->Sample10us = ON;
#endif
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Turns the 10 us sample mode off for the channel-object.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 * @param channel Channel number.
 */
STATIC inline void daqChannelSample10usOff( register DAQ_CANNEL_T* pThis )
{
#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
   pThis->simulatedDescriptor.name.cControl.daqMode   = false;
   pThis->simulatedDescriptor.name.cControl.hiResMode = false;
   pThis->simulatedDescriptor.name.cControl.pmMode    = false;
#else
   daqChannelGetCtrlRegPtr( pThis )->Sample10us = OFF;
#endif
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Queries whether 10 us sample mode is active
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 * @retval true is active
 * @retval false is not active
 */
STATIC inline bool daqChannelIsSample10usActive( register DAQ_CANNEL_T* pThis )
{
#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
   return pThis->simulatedDescriptor.name.cControl.daqMode;
#else
   return (daqChannelGetCtrlRegPtr( pThis )->Sample10us == ON);
#endif
}


/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Turns the 100 us sample mode on for the concerned channel.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 */
STATIC inline void daqChannelSample100usOn( register DAQ_CANNEL_T* pThis )
{
#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
   daqChannelSample10usOn( pThis );
#else
   DAQ_CTRL_REG_T* pCtrl = daqChannelGetCtrlRegPtr( pThis );
   pCtrl->Sample10us = OFF;
   pCtrl->Sample1ms = OFF;
   pCtrl->Sample100us = ON;
#endif
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Turns the 100 us sample mode off for the concerned channel.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 */
STATIC inline
void daqChannelSample100usOff( register DAQ_CANNEL_T* pThis )
{
#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
   daqChannelSample10usOff( pThis );
#else
   daqChannelGetCtrlRegPtr( pThis )->Sample100us = OFF;
#endif
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Queries whether 100 us sample mode is active
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 * @retval true is active
 * @retval false is not active
 */
STATIC inline
bool daqChannelIsSample100usActive( register DAQ_CANNEL_T* pThis )
{
#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
   return pThis->simulatedDescriptor.name.cControl.daqMode != 0;
#else
   return (daqChannelGetCtrlRegPtr( pThis )->Sample100us == ON);
#endif
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Turns the 1 ms us sample mode on for the concerned channel.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 */
STATIC inline void daqChannelSample1msOn( register DAQ_CANNEL_T* pThis )
{
#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
   daqChannelSample10usOn( pThis );
#else
 #ifdef CONFIG_DEBUG_DAQ_ENABLE
   mprintf( "%s( 0x%08x )\n", __func__, (unsigned int)pThis );
 #endif
   DAQ_CTRL_REG_T* pCtrl = daqChannelGetCtrlRegPtr( pThis );
   pCtrl->Sample10us = OFF;
   pCtrl->Sample100us = OFF;
   pCtrl->Sample1ms = ON;
#endif
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Turns the 1 ms sample mode off for the concerned channel.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 */
STATIC inline void daqChannelSample1msOff( register DAQ_CANNEL_T* pThis )
{
#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
   daqChannelSample1msOff( pThis );
#else
 #ifdef CONFIG_DEBUG_DAQ_ENABLE
   mprintf( "%s( 0x%08x )\n", __func__, (unsigned int)pThis );
 #endif
   daqChannelGetCtrlRegPtr( pThis )->Sample1ms = OFF;
#endif
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Queries whether 1 ms sample mode is active
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 * @retval true is active
 * @retval false is not active
 */
STATIC inline bool daqChannelIsSample1msActive( register DAQ_CANNEL_T* pThis )
{
#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
   return daqChannelIsSample100usActive( pThis );
#else
   return (daqChannelGetCtrlRegPtr( pThis )->Sample1ms == ON);
#endif
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Decrements the channel down counter and deactivates all possible
 *        continue modes when the counter reaches zero.
 * @param pThis Pointer to the channel object
 */
STATIC inline
bool daqChannelDecrementBlockCounter( register DAQ_CANNEL_T* pThis )
{
   if( pThis->blockDownCounter == 0 )
      return false;

   pThis->blockDownCounter--;
   if( pThis->blockDownCounter > 0 )
      return false;

   DAQ_CTRL_REG_T* pCtrl = daqChannelGetCtrlRegPtr( pThis );
   pCtrl->Sample10us = OFF;
   pCtrl->Sample100us = OFF;
   pCtrl->Sample1ms = OFF;
   return true;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Enables the trigger mode of the concerning channel.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 */
STATIC inline void daqChannelEnableTriggerMode( register DAQ_CANNEL_T* pThis )
{
   daqChannelGetCtrlRegPtr( pThis )->Ena_TrigMod = ON;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Disables the trigger mode of the concerning channel.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 */
STATIC inline void daqChannelDisableTriggerMode( register DAQ_CANNEL_T* pThis )
{
   daqChannelGetCtrlRegPtr( pThis )->Ena_TrigMod = OFF;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Queries whether the trigger mode of the concerning channel is active.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 * @retval true is active
 * @retval false is not active
 */
STATIC inline
bool daqChannelIsTriggerModeEnabled( register DAQ_CANNEL_T* pThis )
{
   return (daqChannelGetCtrlRegPtr( pThis )->Ena_TrigMod == ON);
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Enabling external trigger source
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 */
STATIC inline
void daqChannelEnableExtrenTrigger( register DAQ_CANNEL_T* pThis )
{
   daqChannelGetCtrlRegPtr( pThis )->ExtTrig_nEvTrig = ON;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Enable event trigger
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 */
STATIC inline void daqChannelEnableEventTrigger( register DAQ_CANNEL_T* pThis )
{
   daqChannelGetCtrlRegPtr( pThis )->ExtTrig_nEvTrig = OFF;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Returns the trigger source of the given channel object
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 * @retval true Channel is in external trigger mode
 * @retval false Channel in in event trigger mode
 */
STATIC inline bool daqChannelGetTriggerSource( register DAQ_CANNEL_T* pThis )
{
   return (daqChannelGetCtrlRegPtr( pThis )->ExtTrig_nEvTrig == ON);
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Enables the post mortem mode (PM) of the given channel.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 */
STATIC inline void daqChannelEnablePostMortem( register DAQ_CANNEL_T* pThis )
{
#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
   DAQ_ASSERT( pThis->simulatedDescriptor.name.cControl.hiResMode == false );
   pThis->simulatedDescriptor.name.cControl.pmMode = true;
   pThis->callCount = 0;
#else
   DAQ_ASSERT( daqChannelGetCtrlRegPtr( pThis )->Ena_HiRes == OFF );
   daqChannelGetCtrlRegPtr( pThis )->Ena_PM = ON;
#endif
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Queries whether the post mortem mode (PM) is enabled.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 * @retval true PM is enabled
 * @retval false PM is disabled
 */
STATIC inline bool daqChannelIsPostMortemActive( register DAQ_CANNEL_T* pThis )
{
#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
   return pThis->simulatedDescriptor.name.cControl.pmMode;
#else
   return (daqChannelGetCtrlRegPtr( pThis )->Ena_PM == ON);
#endif
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Disables the post mortem mode (PM) of the given channel.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 */
STATIC inline void daqChannelDisablePostMortem( register DAQ_CANNEL_T* pThis )
{
   pThis->properties.postMortemEvent = daqChannelIsPostMortemActive( pThis );
#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
   pThis->simulatedDescriptor.name.cControl.pmMode = false;
#else
   daqChannelGetCtrlRegPtr( pThis )->Ena_PM = OFF;
#endif
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Tests whether post mortem mode has been switched off, and the data
 *        are ready to copy.
 *
 * Hint: Of course it's also possible to ask the Ena_PM bit respectively
 * the function daqChannelIsPostMortemActive()
 * but in these manner the scu-bus access  will reduced. \n
 * Unfortunately at the moment the post mortem mode doesn't supports
 * the interrupt.
 * @see daqChannelDisablePostMortem
 * @param pThis Pointer to the channel object
 * @retval true Post-Mortem data ready to copy
 * @retval false No post mortem event
 */
STATIC inline bool daqWasPostMortemEvent( register DAQ_CANNEL_T* pThis )
{
   bool ret = pThis->properties.postMortemEvent;
   pThis->properties.postMortemEvent = false;
   return ret;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Enables the high resolution sampling
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 */
STATIC inline
void daqChannelEnableHighResolution( register DAQ_CANNEL_T* pThis )
{
#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
   DAQ_ASSERT( pThis->simulatedDescriptor.name.cControl.pmMode == false );
   pThis->simulatedDescriptor.name.cControl.hiResMode = true;
   pThis->callCount = 0;
#else
   DAQ_ASSERT( daqChannelGetCtrlRegPtr( pThis )->Ena_PM == OFF );
   daqChannelGetCtrlRegPtr( pThis )->Ena_HiRes = ON;
#endif
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Disables the high resolution sampling
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 */
STATIC inline
void daqChannelDisableHighResolution( register DAQ_CANNEL_T* pThis )
{
#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
   pThis->simulatedDescriptor.name.cControl.hiResMode = false;
#else
   daqChannelGetCtrlRegPtr( pThis )->Ena_HiRes = OFF;
#endif
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Queries whether the high resolution mod is enabled
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 * @retval true high resolution mode is enabled
 * @retval false high resolution mode is disabled
 */
STATIC inline
bool daqChannelIsHighResolutionEnabled( register DAQ_CANNEL_T* pThis )
{
#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
   return pThis->simulatedDescriptor.name.cControl.hiResMode;
#else
   return (daqChannelGetCtrlRegPtr( pThis )->Ena_HiRes == ON);
#endif
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Set the trigger source for high resolution mode to external
 *        trigger source.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 */
STATIC inline
void daqChannelEnableExternTriggerHighRes( register DAQ_CANNEL_T* pThis )
{
   daqChannelGetCtrlRegPtr( pThis )->ExtTrig_nEvTrig_HiRes = ON;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Enables the event trigger mode in high resolution mode.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 */
STATIC inline
void daqChannelEnableEventTriggerHighRes( register DAQ_CANNEL_T* pThis )
{
   daqChannelGetCtrlRegPtr( pThis )->ExtTrig_nEvTrig_HiRes = OFF;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Returns the trigger source of high resolution of the given
 *        channel object
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 * @retval true Channel is in external trigger mode
 * @retval false Channel in in event trigger mode
 */
STATIC inline
bool daqChannelGetTriggerSourceHighRes( register DAQ_CANNEL_T* pThis )
{
   return (daqChannelGetCtrlRegPtr( pThis )->ExtTrig_nEvTrig_HiRes == ON);
}

/*! --------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Gets the least significant word of the bus tag event
 *        trigger condition.
 * @see daqChannelGetRegPtr
 * @see daqDescriptorGetTriggerConditionLW in daq_descriptor.h
 * @param pThis Pointer to the channel object
 * @return least significant wort of bus tag event condition
 */
STATIC inline
DAQ_REGISTER_T daqChannelGetTriggerConditionLW( register DAQ_CANNEL_T* pThis )
{
   DAQ_ASSERT( pThis != NULL );
   __DAQ_VERIFY_CHANNEL_REG_ACCESS(TRIG_LW);
   return __DAQ_GET_CHANNEL_REG( TRIG_LW );
}

/*! --------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Sets the least significant word of the bus tag event
 *        trigger condition.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 * @param value least significant wort of bus tag event condition
 */
STATIC inline
void daqChannelSetTriggerConditionLW( register DAQ_CANNEL_T* pThis,
                                      const DAQ_REGISTER_T value )
{
   DAQ_ASSERT( pThis != NULL );
   __DAQ_VERIFY_CHANNEL_REG_ACCESS(TRIG_LW);
   __DAQ_GET_CHANNEL_REG( TRIG_LW ) = value;
}

/*! --------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Gets the most significant word of the bus tag event
 *        trigger condition.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 * @return most significant wort of bus tag event condition
 */
STATIC inline
DAQ_REGISTER_T daqChannelGetTriggerConditionHW( register DAQ_CANNEL_T* pThis )
{
   DAQ_ASSERT( pThis != NULL );
   __DAQ_VERIFY_CHANNEL_REG_ACCESS( TRIG_HW );
   return __DAQ_GET_CHANNEL_REG( TRIG_HW );
}

/*! --------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Sets the most significant word of the bus tag event
 *        trigger condition.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 * @param value most significant wort of bus tag event condition
 */
STATIC inline
void daqChannelSetTriggerConditionHW( register DAQ_CANNEL_T* pThis,
                                      const DAQ_REGISTER_T value )
{
   DAQ_ASSERT( pThis != NULL );
   __DAQ_VERIFY_CHANNEL_REG_ACCESS( TRIG_HW );
   __DAQ_GET_CHANNEL_REG( TRIG_HW ) = value;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Gets the trigger delay of this channel.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 * @return trigger delay
 */
STATIC inline
DAQ_REGISTER_T daqChannelGetTriggerDelay( register DAQ_CANNEL_T* pThis )
{
   DAQ_ASSERT( pThis != NULL );
   __DAQ_VERIFY_CHANNEL_REG_ACCESS( TRIG_DLY );
   return __DAQ_GET_CHANNEL_REG( TRIG_DLY );
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Set the trigger delay of this channel.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 * @param value trigger delay
 * @bug It doesn't work yet!
 */
STATIC inline
void daqChannelSetTriggerDelay( register DAQ_CANNEL_T* pThis,
                                const DAQ_REGISTER_T value )
{
   DAQ_ASSERT( pThis != NULL );
   __DAQ_VERIFY_CHANNEL_REG_ACCESS( TRIG_DLY );
   __DAQ_GET_CHANNEL_REG( TRIG_DLY ) = value;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL DAQ_INTERRUPT
 * @brief Returns the pointer to the 16 bit DAQ interrupt pending register
 * @param pThis Pointer to the channel object
 * @return Pointer to the 16 bit DAQ interrupt pending register
 */
STATIC inline volatile
DAQ_REGISTER_T* daqChannelGetDaqIntPendingPtr( register DAQ_CANNEL_T* pThis )
{
   DAQ_ASSERT( pThis != NULL );
   return &daqChannelGetRegPtr(pThis)->i[DAQ_INTS];
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL DAQ_INTERRUPT
 * @brief Tests and clears the DAQ interrupt pending flag of this channel.
 * @param pThis Pointer to the channel object
 * @retval true DAQ Interrupt was pending.
 * @retval false No DAQ interrupt was pending.
 */
STATIC inline
bool daqChannelTestAndClearDaqIntPending( register DAQ_CANNEL_T* pThis )
{
   if( ((*daqChannelGetDaqIntPendingPtr( pThis )) & pThis->intMask) != 0 )
   { /*
      * Bit becomes cleared by writing a one in the concerning
      * register position.
      */
      (*daqChannelGetDaqIntPendingPtr( pThis )) |= pThis->intMask;
      return true;
   }
   return false;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL DAQ_INTERRUPT
 * @brief Returns the pointer to the 16 bit HiRes interrupt pending register
 * @param pThis Pointer to the channel object
 * @return Pointer to the 16 bit HiRes interrupt pending register
 */
STATIC inline volatile
DAQ_REGISTER_T* daqChannelGetHiResIntPendingPtr( register DAQ_CANNEL_T* pThis )
{
   DAQ_ASSERT( pThis != NULL );
   return &daqChannelGetRegPtr(pThis)->i[HIRES_INTS];
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL DAQ_INTERRUPT
 * @brief Tests and clears the HiRes interrupt pending flag of this channel.
 * @param pThis Pointer to the channel object
 * @retval true HiRes Interrupt was pending.
 * @retval false No HiRes interrupt was pending.
 */
STATIC inline
bool daqChannelTestAndClearHiResIntPending( register DAQ_CANNEL_T* pThis )
{
   if( ((*daqChannelGetHiResIntPendingPtr( pThis )) & pThis->intMask) != 0 )
   { /*
      * Bit becomes cleared by writing a one in the concerning
      * register position.
      */
      (*daqChannelGetHiResIntPendingPtr( pThis )) |= pThis->intMask;
      return true;
   }
   return false;
}

/*! --------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Get the pointer to the data of the post mortem respectively to
 *        the high resolution FiFo.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 */
STATIC inline
DAQ_DATA_T volatile * daqChannelGetPmDatPtr( register DAQ_CANNEL_T* pThis )
{
   DAQ_ASSERT( pThis != NULL );
   __DAQ_VERIFY_CHANNEL_REG_ACCESS( PM_DAT );
   return &__DAQ_GET_CHANNEL_REG( PM_DAT );
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Moves the oldest 16 bit data word from the Post Mortem / HiRes FiFo
 *        of this channel and returns it.
 * @param pThis Pointer to the channel object
 * @return 16 bit data word moved out from the Post Mortem / HiRes FiFo.
 */
ALWAYS_INLINE
STATIC inline volatile
DAQ_DATA_T daqChannelPopPmFifo( register DAQ_CANNEL_T* pThis )
{
#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
   return daqChannelPopPmFifoSimulate( pThis );
#else
   return *daqChannelGetPmDatPtr( pThis );
#endif
}

/*! --------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Get the pointer to the data of the DAQ FiFo.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 */
STATIC inline
DAQ_DATA_T volatile * daqChannelGetDaqDatPtr( register DAQ_CANNEL_T* pThis )
{
   DAQ_ASSERT( pThis != NULL );
   __DAQ_VERIFY_CHANNEL_REG_ACCESS( DAQ_DAT );
   return &__DAQ_GET_CHANNEL_REG( DAQ_DAT );
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Moves the oldest 16 bit data word from the DAQ FiFo of this channel
 *        and returns it.
 * @param pThis Pointer to the channel object
 * @return 16 bit data word moved out from the DAQ FiFo.
 */
ALWAYS_INLINE
STATIC inline volatile
DAQ_DATA_T daqChannelPopDaqFifo( register DAQ_CANNEL_T* pThis )
{
#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
   return daqChannelPopDaqFifoSimulate( pThis );
#else
   return *daqChannelGetDaqDatPtr( pThis );
#endif
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL DAQ_DEVICE
 * @brief Get version of the DAQ VHDL Macros [0..126]
 * @param pThis Pointer to the channel object
 * @return version number
 */
STATIC inline int daqChannelGetMacroVersion( register DAQ_CANNEL_T* pThis )
{
   DAQ_ASSERT( pThis != NULL );
   __DAQ_VERIFY_CHANNEL_REG_ACCESS( DAQ_FIFO_WORDS );
   return ((DAQ_DAQ_FIFO_WORDS_T*)
                          &__DAQ_GET_CHANNEL_REG( DAQ_FIFO_WORDS ))->version;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Get remaining number of words in DaquDat Fifo.
 * @see DAQ_FIFO_DAQ_WORD_SIZE
 * @param pThis Pointer to the channel object
 * @return Remaining number of words during read out.
 */
STATIC inline
DAQ_REGISTER_T daqChannelGetDaqFifoWords( register DAQ_CANNEL_T* pThis )
{
   DAQ_ASSERT( pThis != NULL );
#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
   return daqChannelGetDaqFifoWordsSimulate( pThis );
#else
   __DAQ_VERIFY_CHANNEL_REG_ACCESS( DAQ_FIFO_WORDS );
   return ((DAQ_DAQ_FIFO_WORDS_T*)
          &__DAQ_GET_CHANNEL_REG( DAQ_FIFO_WORDS ))->fifoWords;
#endif
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL DAQ_DEVICE
 * @brief Gets the maximum number of all used channels form the DAQ device
 *        where this channel belongs.
 * @param pThis Pointer to the channel object
 * @return Number of used channels
 */
STATIC inline
DAQ_REGISTER_T daqChannelGetMaxCannels( register DAQ_CANNEL_T* pThis )
{
   DAQ_ASSERT( pThis != NULL );
   __DAQ_VERIFY_CHANNEL_REG_ACCESS( PM_FIFO_WORDS );
   return ((DAQ_PM_FIFO_WORDS_T*)
          &__DAQ_GET_CHANNEL_REG( PM_FIFO_WORDS ))->maxChannels;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Get remaining number of words in PmDat Fifo.
 * @see DAQ_FIFO_PM_HIRES_WORD_SIZE
 * @param pThis Pointer to the channel object
 * @return Remaining number of words during read out.
 */
STATIC inline
DAQ_REGISTER_T daqChannelGetPmFifoWords( register DAQ_CANNEL_T* pThis )
{
   DAQ_ASSERT( pThis != NULL );
#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
   return daqChannelGetPmFifoWordsSimulate( pThis );
#else
   __DAQ_VERIFY_CHANNEL_REG_ACCESS( PM_FIFO_WORDS );
   return ((DAQ_PM_FIFO_WORDS_T*)
          &__DAQ_GET_CHANNEL_REG( PM_FIFO_WORDS ))->fifoWords;
#endif
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Returns "true" when the Post-Mortem/High Resolution FiFo is full.
 * @param pThis Pointer to the channel object
 * @return true: Fifo is full; false: Fofo is not full.
 */
bool daqChannelIsPmHiResFiFoFull( register DAQ_CANNEL_T* pThis );

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Resets the channel registers to its default values except the slot
 *        number.
 * @param pThis Pointer to the channel object
 */
void daqChannelReset( register DAQ_CANNEL_T* pThis );

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL
 * @brief Prints the actual channel information to the console.
 * @note This function becomes implemented only if the compiler switch
 *       CONFIG_DAQ_DEBUG has been defined!
 * @param pThis Pointer to the channel object
 */
#if defined( CONFIG_DAQ_DEBUG ) || defined(__DOXYGEN__)
   void daqChannelPrintInfo( register DAQ_CANNEL_T* pThis );
#else
   #define daqChannelPrintInfo( pThis ) (void)0
#endif

/*======================== DAQ- Device Functions ============================*/
/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_DEVICE DAQ_SCU_BUS
 * @brief Macro returns the pointer to the SCU-bus slave device object of type
 *        DAQ_BUS_T* belonging the given DAQ channel object.
 * @note <b>CAUTION</b>\n
 *       This macro works only if the DAQ device object (pThis) is
 *       real content of the container DAQ_BUS_T. \n
 *       That means, the address range of the concerned object DAQ_DEVICE_T
 *       has to be within the address range of its container-object
 *       DAQ_BUS_T!\n
 *       But that's just the case in this library.
 *
 * @note <b>CAUTION</b>\n
 *       This macro will only work after the successful call of
 *       function "daqBusFindAndInitializeAll"
 *
 * This is a usual technique using often within the Linux kernel.
 *
 * @see CONTAINER_OF defined in helper_macros.h
 * @param pThis Pointer to the DAQ device object
 * @return Pointer to the parent object of the given DAQ device object of
 *         type DAQ_BUS_T*
 */
#define DAQ_DEVICE_GET_PARENT_OF( pThis ) \
   CONTAINER_OF( pThis, DAQ_BUS_T, aDaq[pThis->n] )

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_DEVICE
 * @brief Returns the SCU bus slave address of this DAQ device
 * @param pThis Pointer to the DAQ-device object
 * @return Slave base address of this DAQ device
 */
STATIC inline
void* daqDeviceGetScuBusSlaveBaseAddress( register DAQ_DEVICE_T* pThis )
{
   DAQ_ASSERT( pThis != NULL );
   DAQ_ASSERT( pThis->pReg != NULL );
   DAQ_ASSERT( (unsigned int)pThis->pReg > DAQ_REGISTER_OFFSET );
  /*
   * Because the register access to the DAQ device is more frequent than
   * to the registers of the SCU slave, therefore the base address of the
   * DAQ register is noted rather than the SCU bus slave address.
   */
   return ((void*)pThis->pReg) - DAQ_REGISTER_OFFSET;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_DEVICE DAQ_INTERRUPT
 * @brief Enables the HiRes finished and DAQ-FiFo interrupts.
 * @see daqDeviceDisableScuSlaveInterrupt
 * @param pThis Pointer to the DAQ-device objects
 */
STATIC inline
void daqDeviceEnableScuSlaveInterrupt( register DAQ_DEVICE_T* pThis )
{
   scuBusSetRegisterFalgs( daqDeviceGetScuBusSlaveBaseAddress( pThis ),
                           Intr_Ena,
                           (1 << DAQ_IRQ_DAQ_FIFO_FULL) |
                           (1 << DAQ_IRQ_HIRES_FINISHED)
                         );
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_DEVICE DAQ_INTERRUPT
 * @brief Disnables the HiRes finished and DAQ-FiFo interrupts.
 * @see daqDeviceEnableScuSlaveInterrupt
 * @param pThis Pointer to the DAQ-device objects
 */
STATIC inline
void daqDeviceDisableScuSlaveInterrupt( register DAQ_DEVICE_T* pThis )
{
   scuBusClearRegisterFalgs( daqDeviceGetScuBusSlaveBaseAddress( pThis ),
                             Intr_Ena,
                             (1 << DAQ_IRQ_DAQ_FIFO_FULL) |
                             (1 << DAQ_IRQ_HIRES_FINISHED)
                           );
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_DEVICE DAQ_INTERRUPT
 * @brief Returns the pointer to the interrupt active flag register of
 *        the SCU slave macro to which belongs this DAQ device.
 * @param pThis Pointer to the DAQ-device object.
 * @return Pointer to the interrupt active flag register.
 */
STATIC inline
DAQ_REGISTER_T* volatile daqDeviceGetInterruptFlags
                                               ( register DAQ_DEVICE_T* pThis )
{
   return &((DAQ_REGISTER_T*)daqDeviceGetScuBusSlaveBaseAddress( pThis ))
                                                                 [Intr_Active];
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_DEVICE DAQ_INTERRUPT
 * @brief Tests and clears the DAQ interrupt pending flag of this DAQ device.
 * @param pFlags Pointer to the DAQ- interrupt flag register.
 * @retval true DAQ Interrupt was pending.
 * @retval false No DAQ interrupt was pending.
 */
STATIC inline
bool _daqDeviceTestAndClearDaqInt( DAQ_REGISTER_T* volatile pFlags )
{
   if( (*pFlags & (1 << DAQ_IRQ_DAQ_FIFO_FULL)) != 0 )
   { /*
      * Bit becomes cleared by writing a one in the concerning
      * register position.
      */
      *pFlags |= (1 << DAQ_IRQ_DAQ_FIFO_FULL);
      return true;
   }
   return false;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_DEVICE DAQ_INTERRUPT
 * @brief Tests and clears the DAQ interrupt pending flag of this DAQ device.
 * @param pThis Pointer to the DAQ-device object
 * @retval true DAQ Interrupt was pending.
 * @retval false No DAQ interrupt was pending.
 */
STATIC inline
bool daqDeviceTestAndClearDaqInt( register DAQ_DEVICE_T* pThis )
{
   return _daqDeviceTestAndClearDaqInt( daqDeviceGetInterruptFlags( pThis ) );
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_DEVICE DAQ_INTERRUPT
 * @brief Tests and clears the HiRes interrupt pending flag of this DAQ device.
 * @param pFlags Pointer to the DAQ- interrupt flag register.
 * @retval true HiRes Interrupt was pending.
 * @retval false No HiRes interrupt was pending.
 */
STATIC inline
bool _daqDeviceTestAndClearHiResInt( uint16_t* volatile pFlags )
{
   if( (*pFlags & (1 << DAQ_IRQ_HIRES_FINISHED)) != 0 )
   { /*
      * Bit becomes cleared by writing a one in the concerning
      * register position.
      */
      *pFlags |= (1 << DAQ_IRQ_HIRES_FINISHED);
      return true;
   }
   return false;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_DEVICE DAQ_INTERRUPT
 * @brief Tests and clears the HiRes interrupt pending flag of this DAQ device.
 * @param pThis Pointer to the DAQ-device object
 * @retval true HiRes Interrupt was pending.
 * @retval false No HiRes interrupt was pending.
 */
STATIC inline
bool daqDeviceTestAndClearHiResInt( register DAQ_DEVICE_T* pThis )
{
   return _daqDeviceTestAndClearHiResInt( daqDeviceGetInterruptFlags( pThis ));
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_DEVICE DAQ_INTERRUPT
 * @brief Returns true when at least one of both DAQ interrupts
 *      (HiRes or Continuous)  is active.
 * @param pFlags Pointer to the DAQ- flag register
 * @retval true  DAQ interrupt request is active.
 * @retval false No DAQ interrupt
 */
STATIC inline
bool _daqDeviceIsInterrupt( uint16_t* volatile pFlags )
{
   return ((*pFlags & ((1 << DAQ_IRQ_HIRES_FINISHED) |
                       (1 << DAQ_IRQ_DAQ_FIFO_FULL)) ) != 0);
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_DEVICE DAQ_INTERRUPT
 * @brief Returns true when at least one of both DAQ interrupts
 *      (HiRes or Continuous)  is active.
 * @param pThis Pointer to the DAQ-device objects
 * @retval true  DAQ interrupt request is active.
 * @retval false No DAQ interrupt
 */
STATIC inline
bool daqDeviceIsInterrupt( register DAQ_DEVICE_T* pThis )
{
   return _daqDeviceIsInterrupt( daqDeviceGetInterruptFlags( pThis ));
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_DEVICE DAQ_INTERRUPT
 * @brief Gets the pointer to the DAQ interrupt pending register.
 * @param pThis Pointer to the DAQ-device object
 * @return Pointer to the DAQ interrupt pending Register.
 */
STATIC inline volatile
uint16_t* daqDeviceGetDaqIntPendingPtr( register DAQ_DEVICE_T* pThis )
{
   DAQ_ASSERT( pThis != NULL );
   DAQ_ASSERT( pThis->pReg != NULL );
   return &pThis->pReg->i[DAQ_INTS];
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_DEVICE DAQ_INTERRUPT
 * @brief Clears all possible pending  DAQ interrupt flags.
 * @param pThis Pointer to the DAQ-device objects
 */
STATIC inline
void daqDeviceClearDaqChannelInterrupts( register DAQ_DEVICE_T* pThis )
{
   *daqDeviceGetDaqIntPendingPtr( pThis ) = (uint16_t)~0;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_DEVICE
 * @brief Gets the pointer to the HiRes interrupt pending register.
 * @param pThis Pointer to the DAQ-device objects
 * @return Pointer to the HiRes interrupt pending Register.
 */
STATIC inline volatile
uint16_t* daqDeviceGetHiResIntPendingPtr( register DAQ_DEVICE_T* pThis )
{
   DAQ_ASSERT( pThis != NULL );
   DAQ_ASSERT( pThis->pReg != NULL );
   return &pThis->pReg->i[HIRES_INTS];
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_DEVICE
 * @brief Clears all possible pending HiRes interrupt flags.
 * @param pThis Pointer to the DAQ-device objects
 */
STATIC inline
void daqDeviceClearHiResChannelInterrupts( register DAQ_DEVICE_T* pThis )
{
   *daqDeviceGetHiResIntPendingPtr( pThis ) = (uint16_t)~0;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_DEVICE
 * @brief Get the slot number of a DAQ device respectively the
 *        DAQ-SCU-bis slave.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the DAQ-device object
 * @return Slot number.
 */
STATIC inline int daqDeviceGetSlot( register DAQ_DEVICE_T* pThis )
{
   DAQ_ASSERT( pThis != NULL );
#if 0
   DAQ_ASSERT( pThis->pReg != NULL );
   /*
    * All existing channels of a DAQ have the same slot number
    * therefore its enough to choose the channel 0.
    */
   return daqChannelGetSlot( &pThis->aChannel[0] );
#else
   return pThis->slot;
#endif
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_DEVICE
 * @brief Get version of the DAQ VHDL Macros [0..126]
 * @param pThis Pointer to the DAQ-device object
 * @return version number
 */
STATIC inline int daqDeviceGetMacroVersion( register DAQ_DEVICE_T* pThis )
{
   DAQ_ASSERT( pThis != NULL );
   DAQ_ASSERT( pThis->pReg != NULL );
   /*
    * All existing channels of a DAQ have the same version number
    * therefore its enough to choose the channel 0.
    */
   return daqChannelGetMacroVersion( &pThis->aChannel[0] );
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_DEVICE
 * @brief Gets the maximum number of existing channels of this device.
 * @param pThis Pointer to the DAQ-device objects
 * @return Number of existing channels of this device.
 */
STATIC inline
unsigned int daqDeviceGetMaxChannels( register DAQ_DEVICE_T* pThis )
{
   DAQ_ASSERT( pThis != NULL );
#if 0
   DAQ_ASSERT( pThis->pReg != NULL );
   /*
    * All existing channels of a DAQ have the same information
    * of the maximum number of used channels,
    * therefore its enough to choose the channel 0.
    */
   return daqChannelGetMaxCannels( &pThis->aChannel[0] );
#else
   return pThis->maxChannels;
#endif
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_DEVICE
 * @brief Gets the number of used channels.
 *
 * They must be equal or smaller than the maximum of existing channels.
 *
 * @see daqDeviceGetMaxChannels
 * @see DAQ_CHANNEL_BF_PROPERTY_T ::notUsed
 * @param pThis Pointer to the DAQ-device objectsremainingWords
 * @return Number of used channels of this device.
 */
unsigned int daqDeviceGetUsedChannels( register DAQ_DEVICE_T* pThis );


/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_DEVICE
 * @brief Gets the pointer to the device object remaining to the given number.
 * @param pThis Pointer to the DAQ-device objects
 * @param n Channel number in a range of 0 to max found channels minus one.
 * @see daqDeviceGetMaxChannels
 */
STATIC inline
DAQ_CANNEL_T* daqDeviceGetChannelObject( register DAQ_DEVICE_T* pThis,
                                         const unsigned int n )
{
   DAQ_ASSERT( n < ARRAY_SIZE(pThis->aChannel) );
   DAQ_ASSERT( n < pThis->maxChannels );
   return &pThis->aChannel[n];
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_DEVICE
 * @brief Becomes invoked from the interrupt routine.
 * @param pThis Pointer to the DAQ-device objects
 */
//void daqDeviceOnInterrupt( register DAQ_DEVICE_T* pThis );

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_DEVICE
 * @brief Presets the time stamp counter of this DAQ device;
 * @see daqDeviceGetTimeStampCounter
 * @param pThis Pointer to the DAQ-device object
 * @param timeOffset Offset time in milliseconds presetting the
 *        timestamp registers. Within this time the the timing-ECA has to be
 *        occur!
 */
void daqDevicePresetTimeStampCounter( register DAQ_DEVICE_T* pThis,
                                      const uint32_t timeOffset  );


/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_DEVICE
 * @brief Gets the 64 bit preseted time stamp counter
 * @see daqDeviceSetTimeStampCounter
 * @param pThis Pointer to the DAQ-device object
 * @return 64 bit time stamp value.
 */
uint64_t daqDeviceGetTimeStampCounter( register DAQ_DEVICE_T* pThis );

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_DEVICE
 * @brief Set the time stamp counter ECA- tag for this DAQ device.
 * @see daqDeviceGetTimeStampTag
 * @param pThis Pointer to the DAQ-device object
 * @param Value of time stamp tag.
 */
void daqDeviceSetTimeStampCounterEcaTag( register DAQ_DEVICE_T* pThis,
                                         const uint32_t tsTag );

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_DEVICE
 * @brief Gets the adjusted time stamp tag for this device.
 * @see daqDeviceSetTimeStampTag
 * @param pThis Pointer to the DAQ-device object
 * @return Value of time stamp tag.
 */
uint32_t daqDeviceGetTimeStampTag( register DAQ_DEVICE_T* pThis );

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_DEVICE
 * @brief Resets the DAQ device in its default values except the slot number.
 * @param pThis Pointer to the DAQ-device object
 */
void daqDeviceReset( register DAQ_DEVICE_T* pThis );

#ifndef CONFIG_DAQ_SINGLE_APP

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_DEVICE
 * @brief Puts a command for the feedback switch FSM in the waiting queue
 *        this command will executed ASAP.
 * @param pThis Pointer to the DAQ-device object
 * @param what Command: FB_ON or FB_OFF.
 * @param fgNumber Number of the function generator: 0 or 1.
 */
void daqDevicePutFeedbackSwitchCommand( register DAQ_DEVICE_T* pThis,
                                        const DAQ_FEEDBACK_ACTION_T what,
                                        const unsigned int fgNumber
                                      );

#endif /* ifndef CONFIG_DAQ_SINGLE_APP */

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_DEVICE
 * @brief Prints the actual DAQ-device information to the console.
 * @note This function becomes implemented only if the compiler switch
 *       CONFIG_DAQ_DEBUG has been defined!
 * @param pThis Pointer to the DAQ-device object
 */
#if defined( CONFIG_DAQ_DEBUG ) || defined(__DOXYGEN__)
   void daqDevicePrintInfo( register DAQ_DEVICE_T* pThis );
#else
   #define daqDevicePrintInfo( pThis ) (void)0
#endif

#if defined( CONFIG_DAQ_DEBUG ) || defined(__DOXYGEN__)
   void daqDevicePrintInterruptStatus( register DAQ_DEVICE_T* pThis );
#else
   #define daqDevicePrintInterruptStatus( pThis ) (void)0
#endif

/*============================ DAQ Bus Functions ============================*/
/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_SCU_BUS
 * @brief Preinitialized the DAQ_BUS_T by zero and try to find all
 *        existing ADDAC-DAQ (and ADDAC-FG's) connected to SCU-bus.
 *
 * For each found DAQ the a element of DAQ_BUS_T::DAQ_DEVICE_T becomes
 * initialized.
 *
 * @param pAllDAQ Pointer object of DAQ_BUS_T including a list of all DAQ.
 * @param pScuBusBase Base address of SCU bus.
 *                    Obtained by find_device_adr(GSI, SCU_BUS_MASTER);
 * @param pFgList Pointer to function generator list. When this module \n
 *                is a part of the ADDAC function generators so the
 *                function generators of this device will registered in \n
 *                the function generator list as well.\n
 *                This parameter isn't present when the compiler-switch
 *                CONFIG_DAQ_SINGLE_APP has been defined.
 *
 * @retval -1 Error occurred.
 * @retval  0 No DAQ found.
 * @retval >0 Number of connected DAQ in SCU-bus.
 */
int daqBusFindAndInitializeAll( register DAQ_BUS_T* pAllDAQ,
                                const void* pScuBusBase
                             #ifndef CONFIG_DAQ_SINGLE_APP
                                ,FG_MACRO_T* pFgList
                             #endif
                              );

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_SCU_BUS
 * @brief Returns the total number of DAQ input channels from all DAQ-slaves
 *        of the whole SCU-bus.
 * @param pAllDAQ Pointer object of DAQ_BUS_T including a list of all DAQ.
 * @note Function daqBusFindAndInitializeAll has to be invoked before!
 *
 */
int daqBusGetNumberOfAllFoundChannels( register DAQ_BUS_T* pAllDAQ );

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_SCU_BUS
 * @brief Returns true when a ACU device has been detected in slot 1.
 */
bool daqBusIsAcuDeviceOnly( register DAQ_BUS_T* pAllDAQ );

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_SCU_BUS
 * @brief Gets the number of found DAQ devices.
 * @param pThis Pointer to the DAQ bus object.
 * @return Number of found DAQ - devices
 */
ALWAYS_INLINE
STATIC inline
unsigned int daqBusGetFoundDevices( const register DAQ_BUS_T* pThis )
{
   return pThis->foundDevices;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_SCU_BUS
 * @brief Get the total number of used DAQ channels of this SCU bus.
 *
 * This number has to be smaller or equal than the number of
 * all found channels.
 * @see daqBusGetNumberOfAllFoundChannels
 * @see daqDeviceGetUsedChannels
 * @see DAQ_CHANNEL_BF_PROPERTY_T ::notUsed
 * @param pThis Pointer to the DAQ bus object.
 * @return Total number of all used channels from this bus.
 */
unsigned int daqBusGetUsedChannels( register DAQ_BUS_T* pThis );

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_SCU_BUS
 * @brief Gets the pointer to a device object by its device number.
 * @note Do not confuse the device number with the slot number!
 * @param pThis Pointer to the DAQ bus object.
 * @param n Device number in the range of 0 to number of found devices minus one.
 * @see daqBusGetFoundDevices
 * @return Pointer of type DAQ_DEVICE_T
 */
STATIC inline DAQ_DEVICE_T* daqBusGetDeviceObject( register DAQ_BUS_T* pThis,
                                                   const unsigned int n )
{
   DAQ_ASSERT( n < ARRAY_SIZE(pThis->aDaq) );
   DAQ_ASSERT( n < pThis->foundDevices );
   return &pThis->aDaq[n];
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_SCU_BUS
 * @brief Gets the pointer of a DAQ device object by the slot number
 * @param pThis Pointer to the DAQ bus object.
 * @param slot Slot number range: [1..12]
 * @retval ==NULL No DAQ device in the given slot.
 * @retval !=NULL Pointer to the DAQ device connected in the given slot.
 */
DAQ_DEVICE_T* daqBusGetDeviceBySlotNumber( register DAQ_BUS_T* pThis,
                                           const unsigned int slot );

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_SCU_BUS
 * @brief Gets the pointer to the channel object related to the absolute
 *        channel number.
 *
 * That means the absolute channel number independent of the DAQ device.\n
 * E.g.: Supposing all DAQ devices connected to the SCU-bus have four channels.\n
 *       Thus has the second channel of the second DAQ device the
 *       absolute number 5 from a maximum of 8 channels.
 * @note The counting starts at the left side of the SCU bus with zero.\n
 *       The first channel of the first device has the number 0.
 * @see daqBusGetNumberOfAllFoundChannels
 * @param pThis Pointer to the DAQ bus object.
 * @param n Absolute channel number beginning at the left side with zero.
 * @retval ==NULL Channel not present. (Maybe the given number is to high.)
 * @retval !=NULL Pointer to the channel object.
 */
DAQ_CANNEL_T* daqBusGetChannelObjectByAbsoluteNumber( register DAQ_BUS_T* pThis,
                                                      const unsigned int n );


/*! ---------------------------------------------------------------------------
 */
void daqBusEnableSlaveInterrupts( register DAQ_BUS_T* pThis );

/*! ---------------------------------------------------------------------------
 */
void daqBusDisablSlaveInterrupts( register DAQ_BUS_T* pThis );

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_SCU_BUS
 * @brief Clears all possible pending interrupts (DAQ and HiRes) of
 *        all DAQ devices
 * @param pThis Pointer to the DAQ bus object.
 */
void daqBusClearAllPendingInterrupts( register DAQ_BUS_T* pThis );

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_SCU_BUS
 * @brief Presets the time stamp counters of all found DAQ devices on
 *        this SCU bus.
 * @param pThis Pointer to the DAQ bus object.
 * @param timeOffset Offset time in milliseconds presetting the
 *        timestamp registers. Within this time the the timing-ECA has to be
 *        occur!
 */
void daqBusPresetAllTimeStampCounters( register DAQ_BUS_T* pThis,
                                       const uint32_t timeOffset );

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_SCU_BUS
 * @brief Sets the time stamp counter ECA- tags of all found DAQ devices on
 *        this SCU bus.
 * @param pThis Pointer to the DAQ bus object.
 * @param tsTag 32 bit value for all time stamp counter tags.
 */
void daqBusSetAllTimeStampCounterEcaTags( register DAQ_BUS_T* pThis,
                                          const uint32_t tsTag );


#ifndef CONFIG_DAQ_SINGLE_APP
/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_SCU_BUS
 * @brief Task function for cooperative multitasking for the main-loop.
 * @param pThis Pointer to the DAQ bus object.
 */
void daqBusDoFeedbackTask( register DAQ_BUS_T* pThis );
#endif

/*! ---------------------------------------------------------------------------
 * @todo All!
 */
unsigned int daqBusDistributeMemory( register DAQ_BUS_T* pThis );

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_SCU_BUS
 * @brief Resets all to this SCU bus connected DAQ devices in its default
 *        values. Except the slot number.
 * @param pThis Pointer to the DAQ bus object.
 */
void daqBusReset( register DAQ_BUS_T* pThis );

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_SCU_BUS
 * @brief Prints the information of all found DAQ-devices to the console.
 * @note This function becomes implemented only if the compiler switch
 *       CONFIG_DAQ_DEBUG has been defined!
 * @param pThis Pointer to the DAQ bus object.
 */
#if defined( CONFIG_DAQ_DEBUG ) || defined(__DOXYGEN__)
   void daqBusPrintInfo( register DAQ_BUS_T* pThis );
#else
   #define daqBusPrintInfo( pThis ) (void)0
#endif

/*======================== DAQ- Descriptor functions ========================*/
/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL DAQ_DEVICE
 * @brief Prints the information of the device descriptor.
 * @note This function becomes implemented only if the compiler switch
 *       CONFIG_DAQ_DEBUG has been defined!
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type uint16_t) of the received record.
 *              @see DAQ_DESCRIPTOR_WORD_SIZE
 */
#if defined( CONFIG_DAQ_DEBUG ) || defined(__DOXYGEN__)
   void daqDescriptorPrintInfo( register DAQ_DESCRIPTOR_T* pThis );
#else
   #define daqDescriptorPrintInfo( pThis ) (void)0
#endif

#if defined( CONFIG_DAQ_PEDANTIC_CHECK ) || defined(__DOXYGEN__)
/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_CHANNEL DAQ_DEVICE
 * @brief Verifies the descriptor with the concerning DAQ channel.
 * @note This macro becomes only implemented if CONFIG_DAQ_PEDANTIC_CHECK
 *       has been defined.
 * @param pThis Pointer to start address of the descriptor
 * @param pChannel Pointer to the concerning channel object which causes
 *                 this descriptor.
 */
   #define DAQ_DESCRIPTOR_VERIFY_MY( pThis, pChannel )                        \
   {                                                                          \
      DAQ_ASSERT( daqDescriptorGetSlot( pThis ) ==                            \
                  daqChannelGetSlot( pChannel ) );                            \
      DAQ_ASSERT( daqDescriptorGetChannel( pThis ) ==                         \
                  daqChannelGetNumber( pChannel ) );                          \
      DAQ_ASSERT( daqDescriptorGetTriggerConditionLW( pThis ) ==              \
                  daqChannelGetTriggerConditionLW( pChannel ) );              \
      DAQ_ASSERT( daqDescriptorGetTriggerConditionHW( pThis ) ==              \
                  daqChannelGetTriggerConditionHW( pChannel ) );              \
      DAQ_ASSERT( daqDescriptorGetTriggerDelay( pThis ) ==                    \
                  daqChannelGetTriggerDelay( pChannel ) );                    \
   }
#else
   #define DAQ_DESCRIPTOR_VERIFY_MY( pThis, pChannel ) (void)0
#endif // / if defined( CONFIG_DAQ_PEDANTIC_CHECK ) || defined(__DOXYGEN__)

#ifdef __cplusplus
}
#endif
#endif /* ifndef _DAQ_H */
/*================================== EOF ====================================*/
