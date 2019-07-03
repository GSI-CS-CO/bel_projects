/******************************************************************************
 *  b2btest-api.h
 *
 *  created : 2019
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 15-April-2019
 *
 * API for b2btest
 *
 * -------------------------------------------------------------------------------------------
 * License Agreement for this software:
 *
 * Copyright (C) 2013  Dietrich Beck
 * GSI Helmholtzzentrum für Schwerionenforschung GmbH
 * Planckstraße 1
 * D-64291 Darmstadt
 * Germany
 *
 * Contact: d.beck@gsi.de
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For all questions and ideas contact: d.beck@gsi.de
 * Last update: 15-April-2019
 ********************************************************************************************/
#ifndef __B2B_TEST_API_H_
#define __B2B_TEST_API_H_

#define B2BTEST_X86_VERSION "0.0.7"

#include <b2b-test.h>

// small helper function
uint64_t getSysTime();     

// convert status code to status text
const char* common_state_text(uint32_t code              // status code
                              );

// convert state code to state text
const char* b2btest_status_text(uint32_t code            // state code
                                );

#endif
