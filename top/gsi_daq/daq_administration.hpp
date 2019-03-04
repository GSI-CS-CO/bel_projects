/*!
 *  @file daq_administration.hpp
 *  @brief DAQ administration
 *
 *  @date 04.03.2019
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
#ifndef _DAQ_ADMINISTRATION_HPP
#define _DAQ_ADMINISTRATION_HPP

#include <list>
#include <daq_interface.hpp>

namespace daq
{
class DaqAdmin;
class DaqDevice;

///////////////////////////////////////////////////////////////////////////////
class DaqChannel
{
   friend class DaqDevice;
   unsigned int m_number;
   DaqDevice*   m_pParent;

public:
   DaqChannel( unsigned int number );
   ~DaqChannel( void );

   const unsigned int getNumber( void ) const
   {
      return m_number;
   }

   DaqDevice* getParent( void )
   {
      return m_pParent;
   }
};

///////////////////////////////////////////////////////////////////////////////
class DaqDevice
{
   friend class DaqAdmin;
   std::vector<DaqChannel*> m_channelList;
   unsigned int             m_slot;
   DaqAdmin*                m_pParent;

public:
   DaqDevice( unsigned int slot );
   ~DaqDevice( void );

   const unsigned int getSlot( void ) const
   {
      return m_slot;
   }

   DaqAdmin* getParent( void )
   {
      return m_pParent;
   }

   bool registerChannel( DaqChannel* pChannel );
};

///////////////////////////////////////////////////////////////////////////////
class DaqAdmin: public DaqInterface
{
   std::list<DaqDevice*>  m_deviceList;

public:
   DaqAdmin( const std::string = DAQ_DEFAULT_WB_DEVICE );
   virtual ~DaqAdmin( void );

   bool registerDevice( DaqDevice* pDevice );
   bool unregisterDevice( DaqDevice* pDevice );
};


} //namespace daq

#endif //  ifndef _DAQ_ADMINISTRATION_HPP
//================================== EOF ======================================
