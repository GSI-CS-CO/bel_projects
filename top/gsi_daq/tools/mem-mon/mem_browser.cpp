/*!
 *  @file mem_browser.cpp
 *  @brief Browser module of memory monitor.
 *
 *  @date 12.04.2022
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
#include <iomanip>
#include <daqt_messages.hpp>
#include "mem_browser.hpp"

using namespace Scu::mmu;
using namespace std;

/*!----------------------------------------------------------------------------
 */
Browser::Browser( mmuEb::EtherboneConnection& roEtherbone, CommandLine& rCmdLine )
   :Mmu( &roEtherbone )
   ,m_rCmdLine( rCmdLine )
{
}

/*!----------------------------------------------------------------------------
 */
Browser::~Browser( void )
{
}

/*!----------------------------------------------------------------------------
 */
void Browser::checkMmuPresent( void )
{
   if( isPresent() )
      return;
   ERROR_MESSAGE( "No MMU found on this SCU!" );
   ::exit( EXIT_FAILURE );
}

/*!----------------------------------------------------------------------------
 */
int Browser::operator()( std::ostream& out )
{
   checkMmuPresent();

   MMU_ITEM_T currentItem;
   currentItem.iNext = 0;
   uint level = 0;

   const uint factor = m_rCmdLine.isInBytes()? sizeof(RAM_PAYLOAD_T) : 1;
   const uint wide   = m_rCmdLine.isInBytes()? 9 : 8;
   do
   {
      readNextItem( currentItem );
      if( level > 0 )
      {
         if( m_rCmdLine.isVerbose() )
            out << "tag: ";
         if( m_rCmdLine.isTagInDecimal() )
             out << setw( 5 );
         else
             out << "0x" << hex << uppercase << setw( 4 ) << setfill('0');

         out << currentItem.tag << ",  " << dec;

         if( m_rCmdLine.isVerbose() )
            out << "begin: ";

         out << setfill( ' ' ) << setw( wide ) << currentItem.iStart * factor  << ",  ";

         if( m_rCmdLine.isVerbose() )
            out << "end: ";
         out << setfill( ' ' ) << setw( wide ) << (currentItem.iStart + currentItem.length) * factor << ",  ";

         if( m_rCmdLine.isVerbose() )
            out << "size: ";
         out << setfill( ' ' ) << setw( wide ) << currentItem.length * factor << ",  ";

         if( m_rCmdLine.isVerbose() )
            out << "consumption: ";
         float size = (static_cast<float>( MMU_ITEMSIZE + currentItem.length) * 100.0)
                      / static_cast<float>(MMU_MAX_INDEX);
         out << fixed << setprecision(6) << setw( 9 ) << size << '%';

         out << endl;
      }
      level += MMU_ITEMSIZE + currentItem.length;
   }
   while( (currentItem.iNext != 0) && (level <= MMU_MAX_INDEX) );

   if( currentItem.iNext != 0 )
   {
      ERROR_MESSAGE( "No end of list found. MMU could be corrupt!" );
      ::exit( EXIT_FAILURE );
   }

   float size = (static_cast<float>(level+MMU_ITEMSIZE) * 100.0)
                      / static_cast<float>(MMU_MAX_INDEX);

   constexpr uint NETTO_MAX = MMU_MAX_INDEX-MMU_ITEMSIZE;
   if( m_rCmdLine.isVerbose() )
   {
      out << "total: "
          << level*factor << " of " << NETTO_MAX*factor << ", "
          << "free: " << (NETTO_MAX - level)*factor << ", "
          << "capacity: " << MMU_MAX_INDEX*factor << ", "
          << "consumption: " << fixed << setprecision(6) << setw( 10 ) << size << '%' << endl;
   }
   else
   {
      out << level*factor << "/" << NETTO_MAX*factor << ", "
          << (NETTO_MAX - level)*factor << ", "
          << MMU_MAX_INDEX*factor << ", "
          << fixed << setprecision(6) << setw( 10 ) << size << '%' << endl;
   }


   return 0;
}

//================================== EOF ======================================
