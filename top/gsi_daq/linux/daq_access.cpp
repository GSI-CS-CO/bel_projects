/*!
 *  @file daq_access.cpp
 *  @brief Manages the MIL and ADDAC_ DAQ data access on LM32
 *
 *  @see daq_eb_ram_buffer.hpp
 *  @see scu_shared_mem.h
 *  @date 04.06.2021
 *  @copyright (C) 2021 GSI Helmholtz Centre for Heavy Ion Research GmbH
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
 * License along with this library. If not, see <http://www.gnu.org/licenses/>
 ******************************************************************************
 */

#include "daq_access.hpp"
#include <daq_exception.hpp>

using namespace Scu;

///////////////////////////////////////////////////////////////////////////////
/*!----------------------------------------------------------------------------
 */
DaqAccess::DaqAccess( DaqEb::EtherboneConnection* poEb )
   :daq::EbRamAccess( poEb )
   ,m_addacDaqLM32Offset( INVALID_OFFSET )
#ifdef CONFIG_MIL_FG
   ,m_milDaqLm32Offset( INVALID_OFFSET )
#endif
{
   probe();
}

/*!----------------------------------------------------------------------------
 */
DaqAccess::~DaqAccess( void )
{
}

/*!----------------------------------------------------------------------------
 */
void DaqAccess::probe( void )
{
   uint32_t actMagicNumber;

   m_addacDaqLM32Offset = INVALID_OFFSET;
#ifdef CONFIG_MIL_FG
   m_milDaqLm32Offset = INVALID_OFFSET;
#endif

  /*
   * First step: Investigation whether the single ADDAC-DAQ LM32
   * application is loaded.
   */
   readLM32( &actMagicNumber, 1,
             offsetof( daq::DAQ_SHARED_IO_T, magicNumber ),
                       sizeof( actMagicNumber ) );
   if( actMagicNumber == DAQ_MAGIC_NUMBER )
   {/*
     * DAQ-LM32 single application found. But...
     */
      m_addacDaqLM32Offset = offsetof( daq::DAQ_SHARED_IO_T, magicNumber );
   }

   /*
    * Second step: Investigation whether the FG-LM32 application
    * is loaded.
    */
   readLM32( &actMagicNumber, 1,
             sizeof( FG::SCU_TEMPERATURE_T ) + offsetof( FG::FG_SHARED_DATA_T, magicNumber ),
             sizeof( actMagicNumber ) );
   if( m_addacDaqLM32Offset != INVALID_OFFSET )
   {/*
     * Check whether the DAQ-magic number is not a random number
     * of SCU_SHARED_DATA_T::board_id.
     */
      if( actMagicNumber == FG_MAGIC_NUMBER )
      {
         m_addacDaqLM32Offset = INVALID_OFFSET;
      }
   }

   if( m_addacDaqLM32Offset != INVALID_OFFSET )
   { /*
      * It seems that a real single ADDAC-DAQ application is running.
      */
      return;
   }

   if( actMagicNumber != FG_MAGIC_NUMBER )
   {
      throw daq::Exception( "Neither DAQ-application nor FG-application "
                            "in LM32 found!" );

   }

#ifdef CONFIG_MIL_FG
   m_milDaqLm32Offset = MIL_DAQ_OFFSET;
#endif

  /*
   * FG-application is loaded.
   * Third step: Investigation whether the FG+DAQ-LM32 application
   * is loaded.
   */
#ifdef CONFIG_MILDAQ_BACKWARD_COMPATIBLE

  /*
   * At first supposing a old LM32-firmware is loaded.
   */
   readLM32( &actMagicNumber, 1,
             OLD_ADDAC_DAQ_OFFSET + offsetof( daq::DAQ_SHARED_IO_T, magicNumber ),
             sizeof( actMagicNumber ) );
   if( actMagicNumber == DAQ_MAGIC_NUMBER )
   { /*
      * A old LM32-firmware has been detected.
      * MIL-DAQ-data becomes stored in LM32- shared memory area.
      */
      m_addacDaqLM32Offset = OLD_ADDAC_DAQ_OFFSET;
      return;
   }
#endif

   readLM32( &actMagicNumber, 1,
             MIL_DAQ_OFFSET + offsetof( MiLdaq::MIL_DAQ_ADMIN_T, magicNumber ),
             sizeof( actMagicNumber ) );
   if( actMagicNumber != MIL_DAQ_MAGIC_NUMBER )
   { /*
      * Old LM32-firmware without ADDAC-DAQ-support is running.
      */
      return;
   }

   readLM32( &actMagicNumber, 1,
             ADDAC_DAQ_OFFSET + offsetof( daq::DAQ_SHARED_IO_T, magicNumber ),
             sizeof( actMagicNumber ) );
   if( actMagicNumber == DAQ_MAGIC_NUMBER )
   { /*
      * LM32-firmware with MIL-DAQ-data in DDR3-RAM is running.
      */
      m_addacDaqLM32Offset = ADDAC_DAQ_OFFSET;
   }
}

//================================== EOF ======================================
