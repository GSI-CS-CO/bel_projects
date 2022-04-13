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
#include <daqt_messages.hpp>
#include "mem_browser.hpp"

namespace Scu
{
namespace mmu
{

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

   do
   {
      readNextItem( currentItem );
      if( level > 0 )
      {
         out << "tag: " << currentItem.tag  << endl;
      }
      level += MMU_ITEMSIZE + currentItem.length;
   }
   while( currentItem.iNext != 0 );

   return 0;
}

} // namespace Scu
} // namespace mmu

//================================== EOF ======================================
