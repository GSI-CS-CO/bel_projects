/*!
 * @file daq_fg_allocator.c
 * @brief Allocation of set- and actual- DAQ-channel for a given
 *        function generator.
 *
 * @note This module is suitable for LM32 and Linux
 *
 * @see
 * <a href="https://www-acc.gsi.de/wiki/bin/viewauth/Hardware/Intern/DataAquisitionMacrofürSCUSlaveBaugruppen">
 * Data Aquisition Macro fuer SCU Slave Baugruppen</a>
 *
 * @date 21.10.2020
 * @copyright (C) 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 * @author Ulrich Becker <u.becker@gsi.de>
 *
 * @todo Allocation of DIOB devices
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
#include <daq_fg_allocator.h>
#include <scu_function_generator.h>

#if defined( CONFIG_DAQ_FG_ALLOCATOR_PEDANTIC_CHECK ) || !defined(__lm32__)
   /* CAUTION:
    * Assert-macros could be expensive in memory consuming and the
    * latency time can increase as well!
    * Especially in embedded systems with small resources.
    * Therefore use them for bug-fixing or developing purposes only!
    */
   #include <scu_assert.h>
   #define DAQ_FG_ASSERT SCU_ASSERT
#else
   #define DAQ_FG_ASSERT(__e) ((void)0)
#endif

/*! ---------------------------------------------------------------------------
 * @see daq_fg_allocator.h
 */
const char* daqDeviceTypeToString( const DAQ_DEVICE_TYP_T type )
{
   #define CASE_ITEM( c ) case c: return #c

   switch( type )
   {
      CASE_ITEM( UNKNOWN );
      CASE_ITEM( ADDAC );
      CASE_ITEM( ACU );
      CASE_ITEM( DIOB );
   #ifdef CONFIG_MIL_FG
      CASE_ITEM( MIL );
   #endif
   }

   #undef CASE_ITEM

   return "undefined";
}

/*! ---------------------------------------------------------------------------
 * @see daq_fg_allocator.h
 */
unsigned int daqGetSetDaqNumberOfFg( const unsigned int fgNum,
                                     const DAQ_DEVICE_TYP_T type )
{
   DAQ_FG_ASSERT( fgNum < MAX_FG_PER_SLAVE );
   switch( type )
   {
      case DIOB: /* CAUTION: DIOB is a workaround! No break here. */
      case ADDAC:
      { /*
         * Returning of DAQ-channel number of ADDAC function generators
         * for set-values.
         */
         return fgNum + MAX_FG_PER_SLAVE;
      }
      case ACU:
      { /*
         * Returning of DAQ-channel number of ACU function generators
         * for set-values.
         */
         return fgNum * 2;
      }
      //TODO  DIOB here!
      default:
      {
         DAQ_FG_ASSERT( false );
         break;
      }
   }
   /*
    * Shall never be reached!
    */
   return 0;
}

/*! ---------------------------------------------------------------------------
 * @see daq_fg_allocator.h
 */
unsigned int daqGetActualDaqNumberOfFg( const unsigned int fgNum,
                                        const DAQ_DEVICE_TYP_T type )
{
   DAQ_FG_ASSERT( fgNum < MAX_FG_PER_SLAVE );
   switch( type )
   {
      case DIOB: /* CAUTION: DIOB is a workaround! No break here. */
      case ADDAC:
      { /*
         * Returning of DAQ-channel number of ADDAC function generators
         * for actual-values.
         */
         return fgNum;
      }
      case ACU:
      { /*
         * Returning of DAQ-channel number of ACU function generators
         * for actual-values.
         */
         return fgNum * 2 + 1;
      }
      //TODO  DIOB here!
      default:
      {
         DAQ_FG_ASSERT( false );
         break;
      }
   }
   /*
    * Shall never be reached!
    */
   return 0;
}

/*================================== EOF ====================================*/
