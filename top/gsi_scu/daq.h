/*!
 *  @file daq.h
 *  @brief Control module for Data Acquisition Unit (DAQ)
 *  @date 13.11.2018
 *  @copyright (C) 2018 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *
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
#ifndef _DAQ_H
#define _DAQ_H

#include "scu_bus.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * For the reason LM32 RAM consume can be reduced here is the possibility
 * to overwrite DAQ_MAX by the Makefile.
 */
#ifndef DAQ_MAX
   #define DAQ_MAX MAX_SCU_SLAVES //!< @brief Maximum number of DAQ's
#else
  #if DAQ_MAX > MAX_SCU_SLAVES
    #error Macro DAQ_MAX can not be greater than MAX_SCU_SLAVES !
  #endif
#endif

#ifndef DAQ_MAX_CHANNELS
  #define DAQ_MAX_CHANNELS 16 //!< @brief Maximum number of DAQ-channels
#else
  #if DAQ_MAX_CHANNELS > 16
    #error Not more than 16 channels per DAQ
  #endif
#endif


/*!
 * @brief Access indexes for writing and reading the DAQ-registers
 * @see daqChannelGetReg
 * @see daqChannelSetReg
 */
typedef enum
{
   CtrlReg         = 0x00, //!< @see DAQ_CTRL_REG_T
   TRIG_LW         = 0x10, //!< @brief Least significant word for SCU-bus event
                           //!<        tag tregger condition.
   TRIG_HW         = 0x20, //!< @brief Most significant word for SCU-bus event
                           //!<        tag tregger condition.
   TRIG_DLY        = 0x30, //!< @brief Trigger delay in samples.
   PM_DAT          = 0x40, //!< @brief Data of PostMortem respectively HiRes-FiFos.
   DAQ_DAT         = 0x50, //!< @brief Data of DAQ-FiFo.
   DAQU_FIFO_WORDS = 0x70, //!< @brief Remaining FiFo word of DaqDat FiFo.
   PM_FIFO_WORDS   = 0x80  //!< @brief Remaining FiFo word of PmDat FiFo.
} DAQ_REGISTER_INDEX;

/*!
 * @brief Values for single bit respectively flag in bit-field structure.
 * @see DAQ_CTRL_REG_T
 */
typedef enum
{
   OFF =  0, //!< @brief Value for switch on.
   ON  =  1  //!< @brief Value for switch off.
} DAQ_SWITCH_STATE_T;

/*!
 * @brief Data type of the control register for each channel.
 * @note Do not change the order of attributes! Its a Hardware image!
 * @see CtrlReg
 * @see DAQ_SWITCH_STATE_T
 */
typedef volatile struct
{
#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) || defined(__DOXYGEN__)
   uint16_t slot:                  4; //!< @brief Bit [15:12] slot number,
                                      //!<        shall be initialized by software,
                                      //!<        will used for the DAQ-Descriptor-Word.
   uint16_t __unused__:            4; //!< @brief Bit [11:8] till now unused.
   uint16_t ExtTrig_nEvTrig_HiRes: 1; //!< @brief Bit [7] trigger source in high resolution mode
                                      //!<        1= external trigger, event trigger.
   uint16_t Ena_HiRes:             1; //!< @brief Bit [6] high resolution sampling with 4 MHz.
                                      //!< @note Ena_PM shall not be active at the same time!
   uint16_t ExtTrig_nEvTrig:       1; //!< @brief Bit [5] trigger source in DAQ mode:
                                      //!<        1=ext trigger, 0= event trigger.
   uint16_t Ena_TrigMod:           1; //!< @brief Bit [4] prevents sampling till triggering.
   uint16_t Sample1ms:             1; //!< @brief Bit [3] use 1 ms sample.
   uint16_t Sample100us:           1; //!< @brief Bit [2] use 100 us sample.
   uint16_t Sample10us:            1; //!< @brief Bit [1] use 10 us sample.
   uint16_t Ena_PM:                1; //!< @brief Bit [0] starts PM sampling with 100 us.
#else
   #error Big endian is requested for this bit- field structure!
#endif
} DAQ_CTRL_REG_T;
STATIC_ASSERT( sizeof(DAQ_CTRL_REG_T) == sizeof(uint16_t) );

/*!
 * @brief Relative start address of the SCU registers
 *
 * Accessing to a SCU register will made as followed:
 * Absolute-register-address = SCU-bus-slave base_address + DAQ_REGISTER_OFFSET
 *                           + canal_number * sizeof(uint16_t)
 */
#define DAQ_REGISTER_OFFSET 0x4000

struct DAQ_DATA_T
{
   //TODO
};


typedef volatile union
{
   volatile uint16_t i[SCUBUS_SLAVE_ADDR_SPACE/sizeof(uint16_t)];
   volatile struct DAQ_DATA_T s;
} DAQ_REGISTER_T;
STATIC_ASSERT( sizeof( DAQ_REGISTER_T ) == SCUBUS_SLAVE_ADDR_SPACE );


/*!
 * @brief Object represents a single channel of a DAQ.
 */
typedef struct
{
   uint8_t n; //!< @brief Channel number [0..DAQ_MAX_CHANNELS-1]
   void* p; //TODO pointer ti FoFo
} DAQ_CANNEL_T;

/*!
 * @brief Object represents a single SCU-Bus slave including a DAQ
 */
typedef struct
{
   unsigned int maxChannels; //!< @brief Number of DAQ-channels
   DAQ_CANNEL_T aChannel[DAQ_MAX_CHANNELS]; //!< @brief Array of channel objects
   DAQ_REGISTER_T* volatile pReg; //!< @brief Pointer to DAQ-registers (start of address space)
} DAQ_DEVICE_T;

/*!
 * @brief Object represents all on the SCU-bus connected DAQs.
 */
typedef struct
{
   unsigned int foundDevices;  //!< @brief Number of found DAQs
   DAQ_DEVICE_T aDaq[DAQ_MAX]; //!< @brief Array of all possible existing DAQs
} DAQ_ALL_T;


/*! ------------------------------------------------------------------------
 * @brief Preinitialized the DAQ_ALL_T by zero and try to find all
 *        existing DAQs connected to SCU-bus.
 *
 * For each found DAQ the a element of DAQ_ALL_T::DAQ_DEVICE_T becomes initialized.
 *
 * @param pAllDAQ Pointer object of DAQ_ALL_T including a list of all DAQ.
 * @param pScuBusBase Base address of SCU bus.
 *                    Obtained by find_device_adr(GSI, SCU_BUS_MASTER);
 * @retval -1 Error occurred.
 * @retval  0 No DAQ found.
 * @retval >0 Number of connected DAQ in SCU-bus.
 */
int daqFindAndInitializeAll( DAQ_ALL_T* pAllDAQ, const void* pScuBusBase );

/*! ---------------------------------------------------------------------------
 * @brief Returns the total number of DAQ input channels from all DAQ-slaves
 *        of the whole SCU-bus.
 * @param pAllDAQ Pointer object of DAQ_ALL_T including a list of all DAQ.
 * @note Function daqFindAndInitializeAll has to be invoked before!
 *
 */
int daqGetNumberOfAllFoundChannels( DAQ_ALL_T* pAllDAQ );

/*! --------------------------------------------------------------------------
 * @brief Writes the given value in addressed register
 * @param pReg Start address of DAQ-macro.
 * @param index Offset address of register @see
 * @param channel Channel number.
 * @param value Value for writing into register.
 */
static inline void daqChannelSetReg( DAQ_REGISTER_T* volatile pReg,
                                     const DAQ_REGISTER_INDEX index,
                                     const unsigned int channel,
                                     const uint16_t value )
{
   LM32_ASSERT( channel < DAQ_MAX_CHANNELS );
   LM32_ASSERT( (index & 0x0F) == 0x00 );
   pReg->i[index+channel] = value;
}

/*! ---------------------------------------------------------------------------
 * @brief Reads a value from a addressed register
 * @param pReg Start address of DAQ-macro.
 * @param index Offset address of register @see
 * @param channel Channel number.
 * @return Register value.
 */
static inline uint16_t daqChannelGetReg( DAQ_REGISTER_T* volatile pReg,
                                         const DAQ_REGISTER_INDEX index,
                                         const unsigned int channel )
{
   LM32_ASSERT( channel < DAQ_MAX_CHANNELS );
   LM32_ASSERT( (index & 0x0F) == 0x00 );
   return pReg->i[index+channel];
}

/*! ---------------------------------------------------------------------------
 * @brief Returns the pointer to the control register of a given DAQ-channel
 * @note <p>CAUTION!</p>\n
 *       This function works only if the channel object (pThis) is
 *       real content of the container DAQ_DEVICE_T. \n
 *       That means, the address range of the concerned object DAQ_CANNEL_T
 *       has to be within the address range of its container-object
 *       DAQ_DEVICE_T!
 * @see CONTAINER_OF defined in helper_macros.h
 * @param pThis Pointer to the channel object
 * @return Pointer to the control register.
 */
static inline DAQ_REGISTER_T* volatile daqChannelGetRegPtr( DAQ_CANNEL_T* pThis )
{
   LM32_ASSERT( pThis->n < DAQ_MAX_CHANNELS );
   return CONTAINER_OF( pThis, DAQ_DEVICE_T, aChannel[pThis->n] )->pReg;
}

/*! ---------------------------------------------------------------------------
 * @brief Returns a pointer to the control-register of the given channel-
 *        object.
 * @see daqChannelGetRegPtr
 * @see DAQ_CTRL_REG_T
 * @param pThis Pointer to the channel object
 * @return Pointer to control register bit field structure.
 */
static inline DAQ_CTRL_REG_T* daqChannelGetCtrlRegPtr( DAQ_CANNEL_T* pThis )
{
   return (DAQ_CTRL_REG_T*) &daqChannelGetRegPtr(pThis)->i[CtrlReg+pThis->n];
}

/*! ---------------------------------------------------------------------------
 * @brief Get the slot number of the given DAQ-channel.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 * @return Slot number of the DAQ device where belonging this channel
 */
static inline int daqChannelGetSlot( DAQ_CANNEL_T* pThis )
{
   return daqChannelGetCtrlRegPtr( pThis )->slot;
}

/*! ---------------------------------------------------------------------------
 * @brief Turns the 10 us sample mode on for the concerned channel.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 */
static inline void daqChannelSample10usOn( DAQ_CANNEL_T* pThis )
{
   DAQ_CTRL_REG_T* pCtrl = daqChannelGetCtrlRegPtr( pThis );
   pCtrl->Sample1ms = OFF;
   pCtrl->Sample100us = OFF;
   pCtrl->Sample10us = ON;
}

/*! ---------------------------------------------------------------------------
 * @brief Turns the 10 us sample mode off for the channel-object.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 * @param channel Channel number.
 */
static inline void daqChannelSample10usOff( DAQ_CANNEL_T* pThis )
{
   daqChannelGetCtrlRegPtr( pThis )->Sample10us = OFF;
}

/*! ---------------------------------------------------------------------------
 * @brief Queries whether 10 us sample mode is active
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 * @retval true is active
 * @retval false is not active
 */
static inline bool daqChannelIsSample10usActive( DAQ_CANNEL_T* pThis )
{
   return (daqChannelGetCtrlRegPtr( pThis )->Sample10us == ON);
}


/*! ---------------------------------------------------------------------------
 * @brief Turns the 100 us sample mode on for the concerned channel.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 */
static inline void daqChannelSample100usOn( DAQ_CANNEL_T* pThis )
{
   DAQ_CTRL_REG_T* pCtrl = daqChannelGetCtrlRegPtr( pThis );
   pCtrl->Sample10us = OFF;
   pCtrl->Sample1ms = OFF;
   pCtrl->Sample100us = ON;
}

/*! ---------------------------------------------------------------------------
 * @brief Turns the 100 us sample mode off for the concerned channel.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 */
static inline void daqChannelSample100usOff( DAQ_CANNEL_T* pThis )
{
   daqChannelGetCtrlRegPtr( pThis )->Sample100us = OFF;
}

/*! ---------------------------------------------------------------------------
 * @brief Queries whether 100 us sample mode is active
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 * @retval true is active
 * @retval false is not active
 */
static inline bool daqChannelIsSample100usActive( DAQ_CANNEL_T* pThis )
{
   return (daqChannelGetCtrlRegPtr( pThis )->Sample100us == ON);
}

/*! ---------------------------------------------------------------------------
 * @brief Turns the 1 ms us sample mode on for the concerned channel.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 */
static inline void daqChannelSample1msOn( DAQ_CANNEL_T* pThis )
{
   DAQ_CTRL_REG_T* pCtrl = daqChannelGetCtrlRegPtr( pThis );
   pCtrl->Sample10us = OFF;
   pCtrl->Sample100us = OFF;
   pCtrl->Sample1ms = ON;
}

/*! ---------------------------------------------------------------------------
 * @brief Turns the 1 ms sample mode off for the concerned channel.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 */
static inline void daqChannelSample1msOff( DAQ_CANNEL_T* pThis )
{
   daqChannelGetCtrlRegPtr( pThis )->Sample1ms = OFF;
}

/*! ---------------------------------------------------------------------------
 * @brief Queries whether 1 ms sample mode is active
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 * @retval true is active
 * @retval false is not active
 */
static inline bool daqChannelIsSample1msActive( DAQ_CANNEL_T* pThis )
{
   return (daqChannelGetCtrlRegPtr( pThis )->Sample1ms == ON);
}

/*! ---------------------------------------------------------------------------
 * @brief Enables the trigger mode of the concerning channel.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 */
static inline void daqChannelEnableTriggerMode( DAQ_CANNEL_T* pThis )
{
   daqChannelGetCtrlRegPtr( pThis )->Ena_TrigMod = ON;
}

/*! ---------------------------------------------------------------------------
 * @brief Disables the trigger mode of the concerning channel.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 */
static inline void daqChannelDisableTriggerMode( DAQ_CANNEL_T* pThis )
{
   daqChannelGetCtrlRegPtr( pThis )->Ena_TrigMod = OFF;
}

/*! ---------------------------------------------------------------------------
 * @brief Queries whether the trigger mode of the concerning channel is active.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 * @retval true is active
 * @retval false is not active
 */
static inline bool daqChannelIsTriggerModeEnabled( DAQ_CANNEL_T* pThis )
{
   return (daqChannelGetCtrlRegPtr( pThis )->Ena_TrigMod == ON);
}

/*! ---------------------------------------------------------------------------
 * @brief Enabling external trigger source
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 */
static inline void daqChannelEnableExtrenTrigger( DAQ_CANNEL_T* pThis )
{
   daqChannelGetCtrlRegPtr( pThis )->ExtTrig_nEvTrig = ON;
}

/*! ---------------------------------------------------------------------------
 * @brief Enable event trigger
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 */
static inline void daqChannelEnableEventTrigger( DAQ_CANNEL_T* pThis )
{
   daqChannelGetCtrlRegPtr( pThis )->ExtTrig_nEvTrig = OFF;
}

/*! ---------------------------------------------------------------------------
 * @brief Returns the trigger source of the given channel object
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 * @retval true Channel is in external trigger mode
 * @retval false Channel in in event trigger mode
 */
static inline bool daqChannelGetTriggerSource( DAQ_CANNEL_T* pThis )
{
   return (daqChannelGetCtrlRegPtr( pThis )->ExtTrig_nEvTrig == ON);
}

/*! ---------------------------------------------------------------------------
 * @brief Enables the post mortem mode (PM) of the given channel.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 */
static inline bool daqChannelEnablePostMortem( DAQ_CANNEL_T* pThis )
{
   LM32_ASSERT( daqChannelGetCtrlRegPtr( pThis )->Ena_HiRes == OFF );
   daqChannelGetCtrlRegPtr( pThis )->Ena_PM = ON;
}

/*! ---------------------------------------------------------------------------
 * @brief Queries whether the post mortem mode (PM) is enabled.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 * @retval true PM is enabled
 * @retval false PM is disabled
 */
static inline bool daqChannelIsPostMortemActive( DAQ_CANNEL_T* pThis )
{
   return (daqChannelGetCtrlRegPtr( pThis )->Ena_PM == ON);
}

/*! ---------------------------------------------------------------------------
 * @brief Disables the post mortem mode (PM) of the given channel.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 */
static inline bool daqChannelDisablePostMortem( DAQ_CANNEL_T* pThis )
{
   daqChannelGetCtrlRegPtr( pThis )->Ena_PM = OFF;
}

/*! ---------------------------------------------------------------------------
 * @brief Enables the high resolution sampling
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 */
static inline void daqChannelEnableHighResolution( DAQ_CANNEL_T* pThis )
{
   LM32_ASSERT( daqChannelGetCtrlRegPtr( pThis )->Ena_PM == OFF );
   daqChannelGetCtrlRegPtr( pThis )->Ena_HiRes = ON;
}

/*! ---------------------------------------------------------------------------
 * @brief Disables the high resolution sampling
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 */
static inline void daqChannelDisableHighResolution( DAQ_CANNEL_T* pThis )
{
   daqChannelGetCtrlRegPtr( pThis )->Ena_HiRes = OFF;
}

/*! ---------------------------------------------------------------------------
 * @brief Queries whether the high resolution mod is enabled
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 * @retval true high resolution mode is enabled
 * @retval false high resolution mode is disabled
 */
static inline bool daqChannelIsHighResolutionEnabled( DAQ_CANNEL_T* pThis )
{
   return (daqChannelGetCtrlRegPtr( pThis )->Ena_HiRes == ON);
}

/*! ---------------------------------------------------------------------------
 * @brief Set the trigger source for high resolution mode to external
 *        trigger source.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 */
static inline void daqChannelEnableExternTriggerHighRes( DAQ_CANNEL_T* pThis )
{
   daqChannelGetCtrlRegPtr( pThis )->ExtTrig_nEvTrig_HiRes = ON;
}

/*! ---------------------------------------------------------------------------
 * @brief Enables the event trigger mode in high resolution mode.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 */
static inline void daqChannelEnableEventTriggerHighRes( DAQ_CANNEL_T* pThis )
{
   daqChannelGetCtrlRegPtr( pThis )->ExtTrig_nEvTrig_HiRes = OFF;
}

/*! ---------------------------------------------------------------------------
 * @brief Returns the trigger source of high resolution of the given
 *        channel object
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 * @retval true Channel is in external trigger mode
 * @retval false Channel in in event trigger mode
 */
static inline bool daqChannelGetTriggerSourceHighRes( DAQ_CANNEL_T* pThis )
{
   return (daqChannelGetCtrlRegPtr( pThis )->ExtTrig_nEvTrig_HiRes == ON);
}

/*! --------------------------------------------------------------------------
 * @brief Gets the least significant word of the bus tag event
 *        trigger condition.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 * @return least significant wort of bus tag event condition
 */
static inline uint16_t daqChannelGetTriggerConditionLW( DAQ_CANNEL_T* pThis )
{
   return daqChannelGetRegPtr( pThis )->i[TRIG_LW+pThis->n];
}

/*! --------------------------------------------------------------------------
 * @brief sets the least significant word of the bus tag event
 *        trigger condition.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 * @param value least significant wort of bus tag event condition
 */
static inline void daqChannelSetTriggerConditionLW( DAQ_CANNEL_T* pThis, uint16_t value )
{
   daqChannelGetRegPtr( pThis )->i[TRIG_LW+pThis->n] = value;
}

/*! --------------------------------------------------------------------------
 * @brief Gets the most significant word of the bus tag event
 *        trigger condition.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 * @return most significant wort of bus tag event condition
 */
static inline uint16_t daqChannelGetTriggerConditionHW( DAQ_CANNEL_T* pThis )
{
   return daqChannelGetRegPtr( pThis )->i[TRIG_HW+pThis->n];
}

/*! --------------------------------------------------------------------------
 * @brief sets the most significant word of the bus tag event
 *        trigger condition.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 * @param value most significant wort of bus tag event condition
 */
static inline void daqChannelSetTriggerConditionHW( DAQ_CANNEL_T* pThis, uint16_t value )
{
   daqChannelGetRegPtr( pThis )->i[TRIG_HW+pThis->n] = value;
}

/*! ---------------------------------------------------------------------------
 * @brief Gets the trigger delay of this channel.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 * @return trigger delay
 */
static inline uint16_t daqChannelGetTriggerDelay( DAQ_CANNEL_T* pThis )
{
   return daqChannelGetRegPtr( pThis )->i[TRIG_DLY+pThis->n];
}

/*! ---------------------------------------------------------------------------
 * @brief Set the trigger delay of this channel.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 * @param value trigger delay
 */
static inline uint16_t daqChannelSetTriggerDelay( DAQ_CANNEL_T* pThis, uint16_t value )
{
   daqChannelGetRegPtr( pThis )->i[TRIG_DLY+pThis->n] = value;
}

/*! --------------------------------------------------------------------------
 * @brief Get the pointer to the data of the post mortem respectively to
 *        the high resolution FiFo.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 */
static inline uint16_t volatile * daqChannelGetPmDatPtr( DAQ_CANNEL_T* pThis )
{ //TODO ?
   return &daqChannelGetRegPtr( pThis )->i[PM_DAT+pThis->n];
}

/*! --------------------------------------------------------------------------
 * @brief Get the pointer to the data of the DAQ FiFo.
 * @see daqChannelGetRegPtr
 * @param pThis Pointer to the channel object
 */
static inline uint16_t volatile * daqChannelGetDaqDatPtr( DAQ_CANNEL_T* pThis )
{  //TODO ?
   return &daqChannelGetRegPtr( pThis )->i[DAQ_DAT+pThis->n];
}


/*! ---------------------------------------------------------------------------
 * @brief Get the slot number of a DAQ device respectively the
 *        DAQ-SCU-bis slave.
 * @see daqChannelGetRegPtr
 * @param pDaqDev Start address of the concerned DAQ-device
 * @return Slot number.
 */
static inline int daqDeviceGetSlot( DAQ_DEVICE_T* pDaqDev )
{
   LM32_ASSERT( pDaqDev != NULL );
   LM32_ASSERT( pDaqDev->pReg != NULL );
   /*
    * All existing channels of a DAQ have the same slot number
    * therefore its enough to choose the channel 0.
    */
   return daqChannelGetSlot( &pDaqDev->aChannel[0] );
}

#ifdef __cplusplus
}
#endif
#endif /* ifndef _DAQ_H */
/*================================== EOF ====================================*/
