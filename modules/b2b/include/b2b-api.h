/******************************************************************************
 *  b2b-api.h
 *
 *  created : 2019
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 18-September-2020
 *
 * API for b2b
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
#ifndef _B2B_API_H_
#define _B2B_API_H_

#define B2B_X86_VERSION "01.00.00"                     // version
#define B2B_F_CLK       200000000                      // clock for DDS, here: BuTiS 200 MHz

#include <b2b.h>
#include <etherbone.h>

// init for communicaiton with shared mem
void api_initShared(eb_address_t lm32_base,            // base address of lm32
                    eb_address_t sharedOffset          // offset of shared area
                    );

// convert LSA frequency to DDS frequency
double api_flsa2fdds(double flsa                       // LSA frequency [Hz]
                     );
#endif
