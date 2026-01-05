/******************************************************************************
 *  common-core.h
 *
 *  created : 2023
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 21-Sep-2023
 *
 * common routines x86 and epcu firmware
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
#ifndef _COMMON_CORE_H_
#define _COMMON_CORE_H_

#define COMMON_CORE_VERSION "00.05.01"

// non-optimed routine for converting single precision to half precision float
// IEEE 754 but no support for subnormal numbers
uint16_t comcore_float2half(float f             // single precision number
                            );

// non-optimed routine for converting half precision to single precision float
// IEEE 754 but no support for subnormal numbers
float comcore_half2float(uint16_t h             // half precision number
                         );

// unsigned 32 bit integer division with rounding
uint32_t comcore_intdiv(uint32_t n,             // integer number
                        uint32_t d              // divisor
                        );

#endif
