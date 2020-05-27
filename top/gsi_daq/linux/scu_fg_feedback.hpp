/*!
 * @file scu_fg_feedback.hpp
 * @brief Administration of data aquesition units for function generator
 *        feedback.
 *
 * @date 25.05.2020
 * @copyright (C) 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 * @author Ulrich Becker <u.becker@gsi.de>
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
#ifndef _SCU_FG_FEEDBACK_HPP
#define _SCU_FG_FEEDBACK_HPP

#include <list>
#include <daq_calculations.hpp>
#include <daq_administration.hpp>
#ifdef CONFIG_MIL_FG
 #include <mdaq_administration.hpp>
#endif
#include <scu_fg_list.hpp>
#include <daq_base_interface.hpp>

using namespace Scu;
using namespace gsi;

///////////////////////////////////////////////////////////////////////////////
class FgFeedbackChannel
{
};

///////////////////////////////////////////////////////////////////////////////
class FgFeedbackBaseDevice: public DaqBaseDevice
{

   ~FgFeedbackBaseDevice( void ) override {}
};

class FgFeedbackDevice
{

   ~FgFeedbackDevice( void ) {}
};


///////////////////////////////////////////////////////////////////////////////
class FgFeedbackAdministration
{
   using DAQ_POLL_T = std::vector<DaqBaseInterface*>;
   /*!
    * @brief List of function generators found by the LM32 application.
    */
   FgList m_oFoundFgs;

   /*!
    * @brief Object for ADDAC DAQ administration.
    */
   daq::DaqAdministration     m_oAddacDaqAdmin;

#ifdef CONFIG_MIL_FG
   MiLdaq::DaqAdministration  m_oMilDaqAdmin;
#endif

   DAQ_POLL_T                 m_vPollList;

protected:
   #define DEVICE_LIST_BASE std::list
   using DEVICE_LIST_T = DEVICE_LIST_BASE<FgFeedbackDevice*>;
   DEVICE_LIST_T m_devicePtrList;

public:
   FgFeedbackAdministration( DaqEb::EtherboneConnection* poEtherbone );
   FgFeedbackAdministration( daq::EbRamAccess* poEbAccess );
   virtual ~FgFeedbackAdministration( void );

   /*!
    * @brief Scanning and synchronizing of the function-generator list found
    *        by the LM32 application.
    * @note This function performances a re-scan by the LM32!
    */
   void scan( void )
   {
      m_oFoundFgs.scan( m_oAddacDaqAdmin.getEbAccess() );
   }

   /*!
    * @brief Synchronizing of the function-generator list found by the
    *        LM32 application.
    */
   void sync( void )
   {
      m_oFoundFgs.sync( m_oAddacDaqAdmin.getEbAccess() );
   }

   uint distributeData( void );
};



#endif // ifndef _SCU_FG_FEEDBACK_HPP
//================================== EOF ======================================
