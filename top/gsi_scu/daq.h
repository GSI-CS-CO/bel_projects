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

#endif

union DAQ_REGISTER_T;

/*!
 * @brief Object represents a single channel of a DAQ.
 */
struct DAQ_CANNEL_T
{
   void* p; //TODO
};

/*!
 * @brief Object represents a single SCU-Bus slave including a DAQ
 */
struct DAQ_DEVICE_T
{
   unsigned int slot;     //!< @brief Slot number of this DAQ. Range: 1..MAX_SCU_SLAVES
   unsigned int maxChannels; //!< @brief Number of DAQ-channels
   struct DAQ_CANNEL_T aChannel[DAQ_MAX_CHANNELS];
   union DAQ_REGISTER_T* pReg; //!< @brief Pointer to DAQ-registers (start of address space)
};

/*!
 * @brief Object represents all on the SCU-bus connected DAQs.
 */
struct DAQ_ALL_T
{
   unsigned int foundDevices;         //!< @brief Number of found DAQs
   struct DAQ_DEVICE_T aDaq[DAQ_MAX]; //!< @brief Array of all possible existing DAQs
};

struct DAQ_DATA_T
{
   //TODO
};

union DAQ_REGISTER_T
{
   volatile uint16_t i[SCUBUS_SLAVE_ADDR_SPACE/sizeof(uint16_t)];
   volatile struct DAQ_DATA_T s;
};

STATIC_ASSERT( sizeof( union DAQ_REGISTER_T ) == SCUBUS_SLAVE_ADDR_SPACE );

/*!
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
int daqFindAndInitializeAll( struct DAQ_ALL_T* pAllDAQ, const void* pScuBusBase );

#ifdef __cplusplus
}
#endif
#endif /* ifndef _DAQ_H */
/*================================== EOF ====================================*/
