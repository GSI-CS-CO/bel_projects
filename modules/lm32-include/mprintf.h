/*!
 * @file mprintf.c
 * @brief Implementation of the mprintf- family.
 *
 * @note In contrast to the ANSI printf family the mprintf family
 *       doesn't support floating point formats!
 *
 * @date unknown (improved 28.05.2020)
 * @copyright (C) 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author unknown (improved by Ulrich Becker <u.becker@gsi.de>)
 */
#ifndef _MPRINTF_H
#define _MPRINTF_H

/*!
 * @defgroup PRINTF Code reduced variant of the ANSI printf-family especially
 *                  for small devices.
 * @note This variant doesn't support floating point numbers!
 */

#include <stdarg.h>
#include <stdlib.h>

#ifdef __lm32__
  #include "uart.h"
#endif

#include <helper_macros.h>

#ifndef DEFAULT_SPRINTF_LIMIT
 #define DEFAULT_SPRINTF_LIMIT 80
#endif

#define C_DIM 0x80

#ifdef __cplusplus
extern "C" {
#endif

/*! ---------------------------------------------------------------------------
 * @ingroup PRINTF
 * @brief Writes the C string pointed by format to the LM32-UART, replacing \n
 *        any format specifier in the same way as printf does, but using the \n
 *       elements in the variable argument list identified by ap instead \n
 *       of additional function arguments.
 *
 * Internally, the function retrieves arguments from the list identified by
 * ap as if va_arg was used on it, and thus the state of arg is likely altered
 * by the call.\n
 * In any case, ap should have been initialized by va_start at some point
 * before the call, and it is expected to be released by va_end at some point
 * after the call
 * @param format C string that contains a format string that follows the same
 *        specifications as format in printf.
 * @param ap A value identifying a variable arguments list initialized with
 *        va_start.
 * @return On success, the total number of characters written is returned.
 */
int vprintf( const char* format, va_list ap );


//int _p_vsprintf(char const *format,va_list ap, char* dst);

/*! ---------------------------------------------------------------------------
 * @ingroup PRINTF
 * @brief Composes a string with the same text that would be printed if \n
 *        format was used on printf, but using the elements in  \n
 *        variable argument list identified by arg instead of additional \n
 *        function arguments and storing the resulting content as a C \n
 *        in the buffer pointed by s (taking n as the maximum buffer capacity
 *        to fill).
 * @param s Pointer to a buffer where the resulting C-string is stored.
 * @param n Maximum number of bytes to be used in the buffer.
 *        The generated string has a length of at most n-1, leaving space for
 *        the additional terminating null character.
 * @param format C string that contains a format string that follows the same
 *        specifications as format in printf
 * @param arg A value identifying a variable arguments list initialized with
 *        va_start.
 * @return The number of characters that would have been written if n had been
 *         sufficiently large, not counting the terminating null character.
 */
int vsnprintf( char* s, size_t n, const char* format, va_list arg );

/*! ---------------------------------------------------------------------------
 * @ingroup PRINTF
 * @brief Writes the C string pointed by format to the LM32-UART.
 *
 * If format includes format specifiers (subsequences beginning with %),
 * the additional arguments following format are formatted and inserted in the
 * resulting string replacing their respective specifiers.
 *
 * @note In contrast to ANSI- printf floating-point numbers will not supported!
 *
 * @param format C string that contains the text to be written to LM32-UART.
 *        It can optionally contain embedded format specifiers that are
 *        replaced by the values specified in subsequent additional arguments
 *        and formatted as requested.
 * @param ... Additional arguments.
 *        Depending on the format string, the function may expect a
 *        sequence of additional arguments, each containing a value to be
 *        used to replace a format specifier in the format string.
 *        There should be at least as many of these arguments as the number
 *        of values specified in the format specifiers.
 *        Additional arguments are ignored by the function.
 * @return On success, the total number of characters written is returned.
 */
int mprintf( const char* format, ... );

/*! ---------------------------------------------------------------------------
 * @ingroup PRINTF
 * @brief Composes a string with the same text that would be printed if format \n
 *        was used on printf, but instead of being printed, the content is \n
 *        stored as a C string in the buffer pointed by dst.
 * @note  The assumed length of the buffer is in macro DEFAULT_SPRINTF_LIMIT
 *        defined. If possible prefer the function snprintf instead of this.
 * @param dst Pointer to a buffer where the resulting C-string is stored.
 * @param format C string that contains a format string that follows the same
 *        specifications as format in printf.
 * @param ... Additional arguments.
 *        Depending on the format string, the function may expect a
 *        sequence of additional arguments, each containing a value to be
 *        used to replace a format specifier in the format string.
 *        There should be at least as many of these arguments as the number
 *        of values specified in the format specifiers.
 *        Additional arguments are ignored by the function.
 * @return On success, the total number of characters written is returned.
 */
int sprintf( char* dst, char const* format, ... );

/*! ---------------------------------------------------------------------------
 * @ingroup PRINTF
 * @brief Composes a string with the same text that would be printed if format \n
 *        was used on printf, but instead of being printed, the content is \n
 *        stored as a C string in the buffer pointed by  \n
 *        (taking n as the maximum buffer capacity to fill).
 * @param s Pointer to a buffer where the resulting C-string is stored.
 *        The buffer should have a size of at least n characters.
 * @param n Maximum number of bytes to be used in the buffer.
 *        The generated string has a length of at most n-1,
 *        leaving space for the additional terminating null character.
 * @param format C string that contains a format string that follows the same
 *        specifications as format in printf
 * @param ... Additional arguments.
 *        Depending on the format string, the function may expect a sequence
 *        of additional arguments, each containing a value to be used to
 *        replace a format specifier in the format string.
 *        There should be at least as many of these arguments as the number
 *        of values specified in the format specifiers. Additional arguments
 *        are ignored by the function.
 * @return On success, the total number of characters written is returned.
 */
int snprintf( char* s, size_t n, const char* format, ... );

GSI_DEPRECATED void m_cprintf( int color, const char*fmt, ...);
GSI_DEPRECATED void m_pcprintf( int row, int col, int color, const char *fmt, ...);
GSI_DEPRECATED void m_term_clear( void );

#ifdef __cplusplus
}
#endif

#endif /* ifndef _MPRINTF_H */
/*================================== EOF ====================================*/
