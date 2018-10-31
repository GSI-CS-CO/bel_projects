/*!
 *
 * @brief     Some helpfull macro definitions
 *
 * @file      helper_macros.h
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      30.10.2018
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
 *******************************************************************************
 */
#ifndef _HELPER_MACROS_H
#define _HELPER_MACROS_H

/*!
 * @brief Macro will be substituted by the number of elements of the given array.
 * @param a Name of the c-array variable
 * @return Number of array-elements
 * 
 * Example:
 * @code
 * int myArray[42], i;
 * for( i = 0; i < ARRAY_SIZE(myArray); i++)
 *    myArray[i] = i;
 * @endcode
 */
#ifndef ARRAY_SIZE
 #define ARRAY_SIZE( a ) ( sizeof(a) / sizeof((a)[0]) )
#endif

#ifndef __GNUC__
  #warning "Compiler isn't a GNU- compiler! Therefore it's not guaranteed that the following macro-definition PACKED_SIZE will work."
#endif
#ifdef PACKED_SIZE
  #undef PACKED_SIZE
#endif
/*!
 * @brief Modifier- macro forces the compiler to arrange the elements of a \n
 *        structure in the smallest possible size of the structure.
 * @see STATIC_ASSERT
 * @note At the moment this macro has been tested for GCC- compiler only!
 */
#define PACKED_SIZE __attribute__((packed))


#ifndef STATIC_ASSERT
 #ifndef __DOXYGEN__
  #define __STATIC_ASSERT__( condition, line ) \
       extern char static_assertion_on_line_##line[2*((condition)!=0)-1];
 #endif
/*!
 * @brief Macro produces a compiletime-error if the given static condition
 *        isn't true.
 * @param condition static condition to test
 *
 * Example 1 should be ok.
 * @code
 * typedef struct
 * {
 *    char x;
 *    int  y;
 * } PACKED_SIZE FOO;
 * STATIC_ASSERT( sizeof(FOO) == (sizeof(char) + sizeof(int))); // OK
 * @endcode
 *
 * Example 2 should make a compiletime-error.
 * @code
 * typedef struct
 * {
 *    char x;
 *    int  y;
 * } BAR;
 * STATIC_ASSERT( sizeof(BAR) == (sizeof(char) + sizeof(int))); // Error
 * @endcode
 * @see PACKED_SIZE
 */
  #define STATIC_ASSERT( condition ) __STATIC_ASSERT__( condition, __LINE__)
#endif // ifndef STATIC_ASSERT

#endif // ifndef _HELPER_MACROS_H
//================================== EOF ======================================