/*!
 * @brief Weak function definitions for the case if they
 *        won't use in the project but the linker expect them. \n
 *        They could be overwritten if necessary.
 * @file      stups.c
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 *******************************************************************************
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
 ******************************************************************************
 */

void __attribute__((weak)) main(void) {
}

void __attribute__((weak)) _irq_entry(void) {
}

void __attribute__((weak)) _segfault(void) {
}

#ifdef CONFIG_CPLUSPLUS_MODULE_PRESENT
/*! ---------------------------------------------------------------------------
 * @brief Dummy function for the case a virtual function pointer is not filled.
 *
 * The linker expect this if abstract C++ classes will used. \n
 * Workaround not nice, I know... :-/
 * @author Ulrich Becker <u.becker@gsi.de>
 * @date 24.01.2019
 */
void __attribute__((weak)) __cxa_pure_virtual( void ) {}
#endif

/*================================== EOF ====================================*/
