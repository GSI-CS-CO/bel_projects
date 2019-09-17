/*!
 *  @file daq_main.h
 *  @brief Main module for daq_control (including main())
 *
 *  @date 27.02.2019
 *  @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
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
#ifndef _DAQ_MAIN_H
#define _DAQ_MAIN_H
#ifndef __lm32__
  #error Module is for target Lattice Micro 32 (LM32) only!
#endif

#if defined( CONFIG_SCU_DAQ_INTEGRATION ) && defined( CONFIG_DAQ_SINGLE_APP )
 #error Either CONFIG_SCU_DAQ_INTEGRATION or CONFIG_DAQ_SINGLE_APP !
#endif
#if !defined( CONFIG_SCU_DAQ_INTEGRATION ) && !defined( CONFIG_DAQ_SINGLE_APP )
 #error Nither CONFIG_SCU_DAQ_INTEGRATION nor CONFIG_DAQ_SINGLE_APP defined!
#endif

#include <daq.h>
#include <daq_ramBuffer.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
   DAQ_BUS_T oDaqDevs;
   RAM_SCU_T oRam;
   volatile bool isIrq;
} DAQ_ADMIN_T;

int initBuffer( RAM_SCU_T* poRam );

int scanScuBus( DAQ_BUS_T* pDaqDevices );


static inline void scuDaqInitialize( DAQ_ADMIN_T* pDaqAdmin )
{
   scanScuBus( &pDaqAdmin->oDaqDevs );
   initBuffer( &pDaqAdmin->oRam );
}

#ifndef CONFIG_DAQ_SINGLE_APP
void forEachScuDaqDevice( void );
extern DAQ_ADMIN_T g_scuDaqAdmin;
#endif

#ifdef __cplusplus
}
#endif

#endif /* ifndef _DAQ_MAIN_H */
/*================================== EOF ====================================*/
