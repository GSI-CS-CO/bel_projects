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

#include <stddef.h> // Necessary for the macro "offsetof()"

/*!
 * @brief Macro represents the full version number of the compiler as integer
 *        value.
 *
 * E.g.: If the compilers version - displayed by the command line option
 *       gcc --version - 7.3.0 then this macro will generate the number 70300.
 */
#if defined(__GNUC__) || defined(__DOXYGEN__)
 #define COMPILER_VERSION (__GNUC__ * 10000 \
                         + __GNUC_MINOR__ * 100 \
                         + __GNUC_PATCHLEVEL__)
#else
 #define COMPILER_VERSION 0
 #warning "Unknown compiler therefore its not possible to determine the macro COMPILER_VERSION !"
#endif

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

/*!
 * @brief Modifier- macro for variables which are not used.
 *
 * In this case the compiler will suppress a appropriate warning. \n
 * This macro is meaningful for some call-back functions respectively
 * pointers to them, with a unique policy of parameters. For example
 * interrupt service routines:
 * @code
 * void interruptHandler( int interruptNumber UNUSED, void* pContext UNUSED )
 * {
 *  // No warning when "interruptNumber" and/or "pContext" will not used.
 * }
 * @endcode
 * @note At the moment this macro has been tested for GCC- compiler only!
 */
#ifdef UNUSED
 #undef UNUSED
#endif
#define UNUSED __attribute__((unused))

/*!
 * @brief Generates a deprecated warning during compiling
 *
 * The deprecated modifier- macro enables the declaration of a deprecated
 * variable or function without any warnings or errors being issued by the
 * compiler. \n
 * However, any access to a deprecated variable or function creates a warning
 * but still compiles.
 */
#ifdef DEPRECATED
 #undef DEPRECATED
#endif
#define DEPRECATED __attribute__((deprecated))

/*!
 * @brief Declares a function as always inline.
 *
 * Generally, functions are not inlined unless optimization is specified. \n
 * For functions declared inline, this attribute inlines the function independent
 * of any restrictions that otherwise apply to inlining. \n
 * Failure to inline such a function is diagnosed as an error. \n
 * @Note that if such a function is called indirectly the compiler
 * may or may not inline it depending on optimization level and a failure
 * to inline an indirect call may or may not be diagnosed.
 */
#define ALWAYS_INLINE __attribute__((always_inline))

/*!
 * @brief Patch of a bug in initializing local static variables by zero if
 *        the code will compiled for the LM32 processor.
 * @note Use for the definition of local static variables this macro rather than "static".
 *
 * This is a temporary workaround till this issue will resolved.
 *
 * @todo Perhaps this issue could be find in the start-up code crt0.S.
 */
#if defined(__GNUC__) && defined(__lm32__)
  #define STATIC_LOCAL __attribute__((section(".BSS"))) static
#else
  #define STATIC_LOCAL static
#endif


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



/*!
 * @brief Will used from DECLARE_CONVERT_BYTE_ENDIAN
 *        and IMPLEMENT_CONVERT_BYTE_ENDIAN
 */
#define __FUNCTION_HEAD_CONVERT_BYTE_ENDIAN( TYP )      \
   TYP convertByteEndian_##TYP( const TYP value )

/*!
 * @brief Will used from macro IMPLEMENT_CONVERT_BYTE_ENDIAN
 *        and C++ template "convertByteEndian" in the
 *        case of C++ only.
 */
#define __FUNCTION_BODY_CONVERT_BYTE_ENDIAN( TYP )      \
   {                                                    \
      TYP result;                                       \
      int i;                                            \
      for( i = sizeof(TYP)-1; i >= 0; i-- )             \
        ((unsigned char*)&result)[i] =                  \
           ((unsigned char*)&value)[sizeof(TYP)-1-i];   \
      return result;                                    \
   }                                                    \

/*!
 * @brief Generates a prototypes of endian converting functions
 *        for header-files.
 * @see IMPLEMENT_CONVERT_BYTE_ENDIAN
 * @param TYP Integer type of value to convert.
 */
#define DECLARE_CONVERT_BYTE_ENDIAN( TYP ) \
   __FUNCTION_HEAD_CONVERT_BYTE_ENDIAN( TYP );

/*!
 * @brief Generates functions converting little to big endian
 *        and vice versa.
 *
 * @see DECLARE_CONVERT_BYTE_ENDIAN
 * Example:
 * @code
 * IMPLEMENT_CONVERT_BYTE_ENDIAN( uint64_t )
 * @endcode
 * implements a function with the following prototype:
 * @code
 * uint64_t convertByteEndian_uint64_t( const uint64_t )
 * @endcode
 * @see DECLARE_CONVERT_BYTE_ENDIAN
 * @param TYP Integer type of value to convert.
 *
 * ... Unfortunately in contrast to C++ C doesn't understand templates. :-/
 */
#define IMPLEMENT_CONVERT_BYTE_ENDIAN( TYP )            \
   __FUNCTION_HEAD_CONVERT_BYTE_ENDIAN( TYP )           \
   __FUNCTION_BODY_CONVERT_BYTE_ENDIAN( TYP )

#if defined(__cplusplus ) || defined(__DOXYGEN__)
/*!
 * @brief Template converts the given value from little to big endian
 *        and vice versa.
 * @note For C++ only!
 * @param value Integer value to convert.
 * @return Converted value.
 */
template <typename TYP> TYP convertByteEndian( const TYP value )
   __FUNCTION_BODY_CONVERT_BYTE_ENDIAN( TYP )
#endif /* __cplusplus */

/*!
 * @brief Cast a member of a structure out to the containing structure.
 *
 * This macro has been adopt from the Linux kernel-source.
 * Origin in <kernel-source>/include/linux/kernel.h as "container_of".
 *
 * @param ptr    The pointer to the member.
 * @param type   The type of the container struct this is embedded in.
 * @param member The name of the member within the struct.
 * @return The pointer of the container-object including this member.
 */
#define CONTAINER_OF(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})


#ifdef TO_STRING_LITERAL
   #undef TO_STRING_LITERAL
#endif
#ifdef TO_STRING
   #undef TO_STRING
#endif
#define TO_STRING_LITERAL( s ) # s

/*!
 * @brief Converts a constant expression to a zero terminated ASCII string.
 * @param s Constant expression
 * @return Zero terminated ASCII string.
 */
#define TO_STRING( s ) TO_STRING_LITERAL( s )


#endif // ifndef _HELPER_MACROS_H
//================================== EOF ======================================