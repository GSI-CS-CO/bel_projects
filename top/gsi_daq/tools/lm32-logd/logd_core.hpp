/*!
 *  @file logd_core.hpp
 *  @brief Main functionality of LM32 log daemon.
 *
 *  @date 21.04.2022
 *  @copyright (C) 2022 GSI Helmholtz Centre for Heavy Ion Research GmbH
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
#ifndef _LOGD_CORE_HPP
#define _LOGD_CORE_HPP

#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <daqt_read_stdin.hpp>
#include <scu_mmu_fe.hpp>
#include <lm32_syslog_common.h>
#include "logd_cmdline.hpp"


namespace Scu
{

///////////////////////////////////////////////////////////////////////////////
class Lm32Logd: public std::iostream
{
   class StringBuffer: public std::stringbuf
   {
      Lm32Logd&   m_rParent;

   public:
      StringBuffer( Lm32Logd& rParent )
         :m_rParent( rParent ) {}

      int sync( void ) override;
   };

   StringBuffer         m_oStrgBuffer;
   CommandLine&         m_rCmdLine;
   mmu::Mmu             m_oMmu;
   uint                 m_lm32Base;
   uint                 m_fifoAdminBase;
   mmu::MMU_ADDR_T      m_offset;
   std::size_t          m_capacity;
   uint64_t             m_lastTimestamp;
   bool                 m_isError;
   bool                 m_isSyslogOpen;
   SYSLOG_FIFO_ADMIN_T  m_fiFoAdmin;

   SYSLOG_FIFO_ITEM_T*  m_pMiddleBuffer;
   std::ofstream        m_logfile;

   Terminal*            m_poTerminal;

public:
   Lm32Logd( mmuEb::EtherboneConnection& roEtherbone, CommandLine& rCmdLine );
   ~Lm32Logd( void );

   void operator()( const bool& rExit );

   uint64_t getLastTimestamp( void )
   {
      return m_lastTimestamp;
   }

   void setError( void )
   {
      m_isError = true;
   }

private:
   uint readLm32( char* pData, std::size_t len,
                  const std::size_t offset );

   uint readStringFromLm32( std::string& rStr, uint addr );

   void updateFiFoAdmin( SYSLOG_FIFO_ADMIN_T& );

   void setResponse( uint n );

   void readItems( SYSLOG_FIFO_ITEM_T* pData, const uint len );

   void readItems( void );

   void evaluateItem( std::string& rOutput, const SYSLOG_FIFO_ITEM_T& item );

   int readKey( void );

   static bool isPaddingChar( const char c );
   static bool isDecDigit( const char c );
};

} // namespace Scu

#endif // ifndef _LOGD_CORE_HPP
//================================== EOF ======================================
