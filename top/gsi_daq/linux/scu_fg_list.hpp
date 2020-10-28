/*!
 *  @file scu_fg_list.hpp
 *  @brief Administration of found function generators by LM32 firmware
 *
 *  @date 22.10.2019
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
 * License along with this library. If not, see <http://www.gnu.org/licenses/>
 ******************************************************************************
 */
#ifndef _SCU_FG_LIST_HPP
#define _SCU_FG_LIST_HPP

#include <scu_control_config.h>
#include <scu_lm32_mailbox.hpp>
#include <vector>

namespace Scu
{
using namespace std;

///////////////////////////////////////////////////////////////////////////////
/*!
 * @brief Administrator for the list of all found function generators.
 */
class FgList
{
public:
   /*!
    * @brief Descriptor of a SCU function generator.
    */
   class FgListItem: protected FG_MACRO_T
   {
   public:
      /*!
       * @brief Returns the socket number including the SCU-bus slot number
       *        and the MIL-flags.
       */
      uint getSocket( void ) const
      {
         return socket;
      }

      /*!
       * @brief Returns the SCU-bus slot number (range 1..12).
       *        If the return value 0, so it is a MIL-device
       *        connected in the DEV-bus socket of the SCU-host device.
       */
      uint getSlot( void ) const
      {
         return socket & SCU_BUS_SLOT_MASK;
      }

      /*!
       * @brief Returns the MIL bus address of the device
       *        or the device number when it's a non MIL function generator.
       */
      uint getDevice( void ) const
      {
         return device;
      }

      /*!
       * @brief Returns the version number of the function generator.
       */
      uint getVersion( void ) const
      {
         return version;
      }

      /*!
       * @brief Returns the data wide in bits of the function generator.
       */
      uint getOutputBits( void ) const
      {
         return outputBits & OUTPUT_BIT_MASK;
      }

      /*!
       * @brief Returns "true" in the case of gap reading when the set-value
       *        is invalid.
       */
      bool isSetValueInvalid( void ) const
      {
         return (outputBits & SET_VALUE_NOT_VALID_MASK) != 0;
      }

      /*!
       * @brief Returns true when the function generator is a MIL- device
       *        otherwise it's a ADDAC device.
       */
      bool isMIL( void ) const
      {
         return getSocket() != getSlot();
      }

      /*!
       * @brief Returns true when the function generator is a ADDAC-device
       *        otherwhise it's a MIL device.
       */
      bool isAddac( void ) const
      {
         return getSocket() == getSlot();
      }
   };
private:
   using FG_LIST_T = vector<FgListItem>;
   FG_LIST_T         m_list;
   uint              m_lm32SoftwareVersion;

public:
   constexpr static uint c_maxFgMacros = MAX_FG_MACROS;

   FgList( void );

   virtual ~FgList( void );

   /*!
    * @brief Returns the iterator object for list items of type
    *        FgList::FgListItem of list begin.
    */
   const FG_LIST_T::iterator begin( void )
   {
      return m_list.begin();
   }

   /*!
    * @brief Returns the iterator object for list items of type
    *        FgList::FgListItem of list end.
    */
   const FG_LIST_T::iterator end( void )
   {
      return m_list.end();
   }

   /*!
    * @brief Returns true when the list is empty.
    */
   const bool empty( void )
   {
      return m_list.empty();
   }

   /*!
    * @brief Returns the major version number of the
    *        LM32 firmware after a scan has been made.
    */
   uint getLm32SoftwareVersion( void ) const
   {
      return m_lm32SoftwareVersion;
   }

   /*!
    * @brief Performs a scan respectively rescan by the LM32 for
    *        all function generators including MIL and non MIL.
    * @param pEbAccess Pointer to the object of type daq::EbRamAccess
    *        for communicating with LM32.
    */
   void scan( daq::EbRamAccess* pEbAccess );

   /*!
    * @brief Performs a scan respectively rescan by the LM32 for
    *        all function generators including MIL and non MIL.
    * @param poSwi Pointer to the object of type Scu::Lm32Swi
    *        for communicating with LM32 and performing a
    *        software interrupt.
    */
   void scan( Lm32Swi* poSwi );

   /*!
    * @brief Synchronizing by the LM32 found function generators
    *        with internal device list.
    * @param pEbAccess Pointer to the object of type daq::EbRamAccess
    *        for communicating with LM32.
    */
   void sync( daq::EbRamAccess* pEbAccess );

   /*!
    * @brief Returns true if function generator with
    *        the given socket and given device number present.
    * @note A scan of function generators before assumed!
    */
   bool isPresent( const uint socket, const uint device );

   /*!
    * @brief Returns true if the given socket number is used by a
    *        function generator.
    * @note A scan of function generators before assumed!
    */
   bool isSocketUsed( const uint socket );


   /*!
    * @brief Returns the number of found MIL function generators after
    *        a scan has been made.
    */
   uint getNumOfFoundMilFg( void );

   /*!
    * @brief Returns the number of ADDAC and/or ACU function generators after
    *        a scan has been made.
    */
   uint getNumOfFoundNonMilFg( void );

   /*!
    * @brief Returns the total number of found function generators after
    *        a scan has been made.
    */
   uint getNumOfFoundFg( void ) const
   {
      return m_list.size();
   }
};

} // nemespace Scu

#endif // ifndef _SCU_FG_LIST_HPP
//================================== EOF ======================================
