/*!
 *
 * @brief     Definition of some terminal escape sequences (ISO 6429)
 *
 *            Helpful for outputs via eb-console
 *
 * @file      eb_console_helper.h
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      12.11.2018
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
#ifndef _EB_CONSOLE_HELPER_H
#define _EB_CONSOLE_HELPER_H

#if defined(__lm32__)
  #include <mprintf.h>
#elif !defined( mprintf ) && !defined(__DOXYGEN__)
  #include <helper_macros.h>
  #include <stdio.h>
  #define mprintf printf
#endif

#define ESC_FG_BLACK   "\e[30m" /*!< @brief Foreground color black   */
#define ESC_FG_RED     "\e[31m" /*!< @brief Foreground color red     */
#define ESC_FG_GREEN   "\e[32m" /*!< @brief Foreground color green   */
#define ESC_FG_YELLOW  "\e[33m" /*!< @brief Foreground color yellow  */
#define ESC_FG_BLUE    "\e[34m" /*!< @brief Foreground color blue    */
#define ESC_FG_MAGENTA "\e[35m" /*!< @brief Foreground color magenta */
#define ESC_FG_CYAN    "\e[36m" /*!< @brief Foreground color cyan    */
#define ESC_FG_WHITE   "\e[37m" /*!< @brief Foreground color white   */

#define ESC_BG_BLACK   "\e[40m" /*!< @brief Background color black   */
#define ESC_BG_RED     "\e[41m" /*!< @brief Background color red     */
#define ESC_BG_GREEN   "\e[42m" /*!< @brief Background color green   */
#define ESC_BG_YELLOW  "\e[43m" /*!< @brief Background color yellow  */
#define ESC_BG_BLUE    "\e[44m" /*!< @brief Background color blue    */
#define ESC_BG_MAGENTA "\e[45m" /*!< @brief Background color magenta */
#define ESC_BG_CYAN    "\e[46m" /*!< @brief Background color cyan    */
#define ESC_BG_WHITE   "\e[47m" /*!< @brief Background color white   */

#define ESC_BOLD      "\e[1m"   /*!< @brief Bold on  */
#define ESC_BLINK     "\e[5m"   /*!< @brief Blink on */
#define ESC_NORMAL    "\e[0m"   /*!< @brief All attributes off */
#define ESC_HIDDEN    "\e[8m"   /*!< @brief Hidden on */

#define ESC_CLR_LINE  "\e[K"    /*!< @brief Clears the sctual line   */
#define ESC_CLR_SCR   "\e[2J"   /*!< @brief Clears the terminal screen */

/*!
 * @brief Set the cursor to the position.
 * @note This macro provides that the numbers of _X and _Y are strings,
 *       therefore they has to put in quotes: "".
 *
 * E.g.:
 * @code
 * printf( ESC_XY( "2", "4" ) "Text begins at column 2 and line 4" );
 * @endcode
 * @param _X Column position in quotes.
 * @param _y Line position in quotes.
 */
#define ESC_XY( _X, _Y ) "\e[" _Y ";" _X "H"

#define ESC_ERROR   ESC_BOLD ESC_FG_RED    /*!< @brief Format for error messages */
#define ESC_WARNING ESC_BOLD ESC_FG_YELLOW /*!< @brief Format for warning messages */
#define ESC_DEBUG   ESC_FG_YELLOW          /*!< @brief Format for debug messages */

#ifdef __cplusplus
extern "C" {
namespace gsi
{
#endif

/*!
 * @ingroup PRINTF
 * @brief Set cursor position.
 * @param x Column position (horizontal)
 * @param y Line position (vertical)
 */
STATIC inline void gotoxy( const unsigned int x, const unsigned int y )
{
   mprintf( ESC_XY( "%u", "%u" ), y, x );
}

/*!
 * @ingroup PRINTF
 * @brief Clears the entire console screen and moves the cursor to (0,0)
 */
STATIC inline void clrscr( void )
{
   mprintf( ESC_CLR_SCR );
}

/*!
 * @ingroup PRINTF
 * @brief Clears all characters from the cursor position to the end of the
 *        line (including the character at the cursor position).
 */
STATIC inline void clrline( void )
{
   mprintf( ESC_CLR_LINE );
}

#ifdef __cplusplus
} /* namespace gsi */
} /* extern "C" */
#endif


#endif /* ifndef _EB_CONSOLE_HELPER_H */
/*================================== EOF ====================================*/
