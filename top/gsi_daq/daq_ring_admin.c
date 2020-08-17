/*!
 * @file daq_ring_admin.c
 * @brief Administration of the indexes for a ring-buffer.
 *
 * @note This module is suitable for LM32 and Linux
 * @see scu_ramBuffer.h
 *
 * @see scu_ddr3.h
 * @see scu_ddr3.c
 * @date 19.06.2019
 * @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
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
#warning Module daq_ring_admin.c is deprecated, use circular_index.c instead!
#include <circular_index.c>
/*================================== EOF ====================================*/
