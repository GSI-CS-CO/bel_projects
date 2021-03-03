/*!
 * @file fgw_parser.hpp
 * @brief Module for parsing polynom input file-streams for
 *        SCU function generators
 *
 * @date 09.12.2020
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
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */
#ifndef _FGW_PARSER_HPP
#define _FGW_PARSER_HPP

#ifndef __DOCFSM__
 #include <stdlib.h>
 #include <iostream>
 #include <fstream>
 #include <string>
 #include <vector>
 #include <limits.h>
 #include <daq_calculations.hpp>
 #include <daq_exception.hpp>
 #include <daqt_messages.hpp>
 #include <scu_function_generator.h>
#endif

namespace fgw
{

using POLYNOM_T = Scu::FG_PARAM_SET_T;

using POLYMOM_VECT_T = std::vector<POLYNOM_T>;

int parseInStream( POLYMOM_VECT_T& rVect, std::istream& rInput );

} // namespace fgw

#endif // ifndef _FGW_PARSER_HPP
//================================== EOF ======================================
