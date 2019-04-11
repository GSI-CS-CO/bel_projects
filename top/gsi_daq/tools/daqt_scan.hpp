/*!
 *  @file daqt_scan.hpp
 *  @brief SCU-Bus scan module of Data Acquisition Tool
 *
 *  @date 11.04.2019
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
#ifndef _DAQT_SCAN_HPP
#define _DAQT_SCAN_HPP
#include <daq_interface.hpp>
#include <iostream>

namespace daqt
{

using namespace std;
using namespace daq;

class Scan: public DaqInterface
{

public:
   Scan( string wbDevice ): DaqInterface( wbDevice )
   {
   }


   void doScan( ostream& rOut );

   void operator()( ostream& rOut )
   {
      doScan( rOut );
   }

};

};

#endif // ifndef _DAQT_SCAN_HPP
//================================== EOF ======================================
