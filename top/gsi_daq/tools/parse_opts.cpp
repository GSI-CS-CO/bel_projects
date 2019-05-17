/*****************************************************************************/
/*                                                                           */
/*!      @brief Redundancy free command-line option parser for C++>=11       */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*! @file    parse_opts.cpp                                                  */
/*! @see     parse_opts.hpp                                                  */
/*! @author  Ulrich Becker                                                   */
/*! @date    17.12.2016                                                      */
/*  Updates:                                                                 */
/*****************************************************************************/
/*
 * MIT License
 *
 * Copyright (c) 2016 Ulrich Becker
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <parse_opts.hpp>
#include <algorithm>

#if defined( CONFIG_CLOP_NO_NO_ARG )          \
    && defined( CONFIG_CLOP_NO_REQUIRED_ARG ) \
    && defined( CONFIG_CLOP_NO_OPTIONAL_ARG )
 #error Not allowed to define all three macros!
#endif

//! @see parse_opts.hpp
namespace CLOP
{
using namespace std;

//////////////////////////////////////////////////////////////////////////////
/*!----------------------------------------------------------------------------
@see parse_opts.hpp
*/
void OPTION::print( ostream& out )
{
   string sParam;

   switch( m_hasArg )
   {
   #ifndef CONFIG_CLOP_NO_NO_ARG
      case NO_ARG:       sParam = "";          break;
   #endif
   #ifndef CONFIG_CLOP_NO_REQUIRED_ARG
      case REQUIRED_ARG: sParam = " <PARAM>";  break;
   #endif
   #ifndef CONFIG_CLOP_NO_OPTIONAL_ARG
      case OPTIONAL_ARG: sParam = " [=PARAM]"; break;
   #endif
      default: assert( false ); break;
   }

   if( m_shortOpt != 0 )
   {
      out << "-" << m_shortOpt << sParam;
      if( m_longOpt.size() > 0 )
         out << ',';
   }
   else
      out << "   ";

   if( m_longOpt.size() > 0 )
      out << " --" << m_longOpt << sParam;
}

///////////////////////////////////////////////////////////////////////////////
/*!----------------------------------------------------------------------------
@see parse_opts.hpp
*/
int OPTION_V::adapter( PARSER* poParser )
{
   return static_cast<OPTION_V*>(poParser->getCurrentOption())->onGiven( poParser );
}

/*!----------------------------------------------------------------------------
@see parse_opts.hpp
*/
OPTION_V::OPTION_V( void )
{
   m_func = &adapter;
}

///////////////////////////////////////////////////////////////////////////////
/*!----------------------------------------------------------------------------
@see parse_opts.hpp
*/
PARSER::PARSER( int argc, char** ppArgv, bool allowNegativeDigits ):
   m_ppArgv( ppArgv ),
   m_argc( argc ),
   m_pCurrentOption( nullptr ),
   m_allowNegativeDigits( allowNegativeDigits )
{
}

/*!----------------------------------------------------------------------------
@see parse_opts.hpp
*/
int PARSER::parse( int offset )
{
   int ret = _parse( offset );
   m_pCurrentOption = nullptr;
   return ret;
}

/*!----------------------------------------------------------------------------
@see parse_opts.hpp
*/

#define _RETURN_HANDLING( f ) \
   ret = (f);                 \
   if( ret < 0 )              \
      return ret;             \
   if( ret > 0 )              \
      error = true;           \


int PARSER::_parse( int offset )
{
   assert( offset >= 1 );

   int ret;
   bool error = false;

   for( m_index = offset; m_index < m_argc; m_index++ )
   {
      if( m_ppArgv[m_index][0] != '-' )
      {
         ret = onArgument(); // Returns 0 by default
         if( ret == 0 )
            break;
         if( ret > 0 )
            continue;
         return ret;
      }

      const char* pCurrent = &m_ppArgv[m_index][1];
      if( *pCurrent == '\0' )
      {
         ret = onErrorMissingOption();
         if( ret != 0 )
            return ret;
      }

      // In the case the first non-option argument is a negative number.
      if( !m_allowNegativeDigits && (*pCurrent >= '0') && (*pCurrent <= '9') )
      { // Argument is not a option but a negative number.
         ret = onArgument(); // Returns 0 by default
         if( ret == 0 )
            break;
         if( ret > 0 )
            continue;
         return ret;
      }

      m_optArg.clear();

      if( *pCurrent == '-' ) // Long option?
      { // Yes
         pCurrent++;
         if( *pCurrent == '\0' )
         {
            ret = onErrorMissingLongOption();
            if( ret != 0 )
               return ret;
         }

         std::size_t tl = 0;
         while( (pCurrent[tl] != '\0') && (pCurrent[tl] != '=') )
            tl++;

         m_pCurrentOption = nullptr;
         for( auto& it : *this )
         {
            assert( it->m_func != nullptr );
            if( it->m_longOpt.empty() )
            {  // In this case at least OPTION::m_shortOpt must be defined!
               assert( it->m_shortOpt != '\0' );
               continue;
            }
            if( tl != it->m_longOpt.length() )
               continue;
            if( it->m_longOpt.compare( 0, tl, pCurrent, tl ) == 0 )
            {
               assert( m_pCurrentOption == nullptr );
               m_pCurrentOption = it;
               break;
            }
         }
         if( m_pCurrentOption == nullptr )
         {
            if( onErrorUnrecognizedLongOption( pCurrent ) < 0 )
               return -1;
            error = true;
            continue;
         }

         switch( m_pCurrentOption->m_hasArg )
         {
         #ifndef CONFIG_CLOP_NO_NO_ARG
            case OPTION::NO_ARG:
            {
               _RETURN_HANDLING( m_pCurrentOption->m_func( this ) )
               break;
            }
         #endif // ifndef CONFIG_CLOP_NO_NO_ARG
         #ifndef CONFIG_CLOP_NO_REQUIRED_ARG
            case OPTION::REQUIRED_ARG:
            {
               if( (m_index+1) == m_argc )
               {
                  _RETURN_HANDLING( onErrorLongMissingRequiredArg() )
                  break;
               }
               m_index++;
               m_optArg = m_ppArgv[m_index];
               _RETURN_HANDLING( m_pCurrentOption->m_func( this ) )
               break;
            } // End case OPTION::REQUIRED_ARG
         #endif // ifndef CONFIG_CLOP_NO_REQUIRED_ARG
         #ifndef CONFIG_CLOP_NO_OPTIONAL_ARG
            case OPTION::OPTIONAL_ARG:
            {
               if( pCurrent[tl] == '\0' )
               {
                  if( ((m_index+1) == m_argc) || (m_ppArgv[m_index+1][0] != '=') )
                  { // No argument
                     _RETURN_HANDLING( m_pCurrentOption->m_func( this ) )
                     break;
                  }
                  if( m_ppArgv[m_index+1][0] == '=' )
                  {
                     m_index++;
                     if( m_ppArgv[m_index][1] != '\0' )
                     { // "--OPTION =ARGUMENT"
                        m_optArg = &m_ppArgv[m_index][1];
                        _RETURN_HANDLING( m_pCurrentOption->m_func( this ) )
                        break;
                     }
                     if( (m_index+1) == m_argc )
                     {
                        _RETURN_HANDLING( onErrorlongOptionalArg() )
                        break;
                     }
                     // "--OPTION = ARGUMENT"
                     m_index++;
                     m_optArg = m_ppArgv[m_index];
                     _RETURN_HANDLING( m_pCurrentOption->m_func( this ) )
                  }
                  break;
               } // if( pCurrent[tl] == '\0' )
               if( pCurrent[tl+1] != '\0' )
               { // "--OPTION=ARGUMENT"
                  m_optArg = &pCurrent[tl+1];
                  _RETURN_HANDLING( m_pCurrentOption->m_func( this ) )
                  break;
               }
               if( (m_index+1) == m_argc )
               {
                  _RETURN_HANDLING( onErrorlongOptionalArg() )
                  break;
               }
               // "--OPTION= ARGUMENT"
               m_index++;
               m_optArg = m_ppArgv[m_index];
               _RETURN_HANDLING( m_pCurrentOption->m_func( this ) )
               break;
            } // End of case OPTIONAL_ARG:
         #endif // ifndef CONFIG_CLOP_NO_OPTIONAL_ARG
            default: assert( false ); break;
         } // End switch( m_pCurrentOption->m_hasArg )
         continue; // Of for( m_index = offset; m_index < m_argc; m_index++ )
      } // if( *pCurrent == '-' ) End of long-option parsing.

      while( *pCurrent != '\0' ) // short option
      {
         m_pCurrentOption = nullptr;
         for( auto& it : *this )
         {
            assert( it->m_func != nullptr );
            if( it->m_shortOpt == '\0' )
            {  // In this case at least OPTION::m_longOpt must be defined!
               assert( !it->m_longOpt.empty() );
               continue;
            }
            if( it->m_shortOpt == *pCurrent )
            {
               assert( m_pCurrentOption == nullptr );
               m_pCurrentOption = it;
               break;
            }
         }
         if( m_pCurrentOption == nullptr )
         {
            if( onErrorUnrecognizedShortOption( *pCurrent ) < 0 )
               return -1;
            error = true;
            pCurrent++;
            continue;
         }

         switch( m_pCurrentOption->m_hasArg )
         {
         #ifndef CONFIG_CLOP_NO_NO_ARG
            case OPTION::NO_ARG:
            {
               _RETURN_HANDLING( m_pCurrentOption->m_func( this ) )
               break;
            }
         #endif // ifndef CONFIG_CLOP_NO_NO_ARG
         #ifndef CONFIG_CLOP_NO_REQUIRED_ARG
            case OPTION::REQUIRED_ARG:
            {
               if( (pCurrent[1] == '\0') && ((m_index+1) == m_argc) )
               {
                  _RETURN_HANDLING( onErrorShortMissingRequiredArg() )
                  break;
               }
               if( pCurrent[1] != '\0' )
               {
                  m_optArg = &pCurrent[1];
                  _RETURN_HANDLING( m_pCurrentOption->m_func( this ) )
                  do
                     pCurrent++;
                  while( pCurrent[1] != '\0' );
               }
               else
               {
                  m_index++;
                  m_optArg = m_ppArgv[m_index];
                  _RETURN_HANDLING( m_pCurrentOption->m_func( this ) )
               }
               break;
            } // End case OPTION::REQUIRED_ARG
         #endif // ifndef CONFIG_CLOP_NO_REQUIRED_ARG
         #ifndef CONFIG_CLOP_NO_OPTIONAL_ARG
            case OPTION::OPTIONAL_ARG:
            {
               if( pCurrent[1] == '=' )
               {
                  pCurrent++;
                  if( pCurrent[1] != '\0' )
                     m_optArg = &pCurrent[1]; // "-O=ARGUMENT"
                  else if( (m_index+1) < m_argc )
                  { // "-O= ARGUMENT"
                     m_index++;
                     m_optArg = m_ppArgv[m_index];
                  }
                  else
                  {
                     _RETURN_HANDLING( onErrorShortOptionalArg() )
                     break;
                  }
               }
               else if( (pCurrent[1] == '\0') && ((m_index+1) < m_argc) )
               {
                  if( m_ppArgv[m_index+1][0] == '=' )
                  {
                     m_index++;
                     if( m_ppArgv[m_index][1] != '\0' )
                        m_optArg = &m_ppArgv[m_index][1]; // "-O =ARGUMENT"
                     else
                     {
                        if( (m_index+1) < m_argc )
                        { // "-O = ARGUMENT"
                           m_index++;
                           m_optArg = m_ppArgv[m_index];
                        }
                        else
                        {
                           _RETURN_HANDLING( onErrorShortOptionalArg() )
                           break;
                        }
                     }
                  }
               }

               _RETURN_HANDLING( m_pCurrentOption->m_func( this ) )   

               if( m_optArg.empty() )
                  break;

               while( pCurrent[1] != '\0' )
                  pCurrent++;
               break;
            } // End of case OPTIONAL_ARG:
         #endif // ifndef CONFIG_CLOP_NO_OPTIONAL_ARG
            default: assert( false ); break;
         } // End switch( m_pCurrentOption->m_hasArg )
         pCurrent++;
      } // while( *pCurrent != '\0' ) Ent of short option.
   } // End for( m_index = offset; m_index < m_argc; m_index++ )
   return error? -1 : m_index;
}

/*!----------------------------------------------------------------------------
@see parse_opts.hpp
*/
void PARSER::list( ostream& out )
{
   for( auto& lIt : *this )
   {
      out << '\n';
      lIt->print( out );
      out << "\n\t";
      for( auto& cIt : lIt->m_helpText )
      {
         out << cIt;
         if( cIt == '\n' )
            out << '\t'; // Inserting a tabulator after each linefeed.
      }
      out << '\n';
   }
}

/*!----------------------------------------------------------------------------
@see parse_opts.hpp
*/
void PARSER::sortShort( bool down )
{
   sort( begin(), end(),
         [down]( OPTION* a, OPTION* b ) -> bool
         {
            return down != (a->m_shortOpt < b->m_shortOpt);
         });
}

/*!----------------------------------------------------------------------------
@see parse_opts.hpp
*/
void PARSER::sortLong( bool down )
{
   sort( begin(), end(),
         [down]( OPTION* a, OPTION* b ) -> bool
         {
            return down != (a->m_longOpt.compare( b->m_longOpt ) < 0);
         });
}

/*!----------------------------------------------------------------------------
 * @ingroup OPT_ERROR
 * @see parse_opts.hpp
 */
int PARSER::onErrorMissingOption( void )
{
   cerr << getProgramName() << ": missing option -?" << endl;
   return -1;
}

/*!----------------------------------------------------------------------------
 * @ingroup OPT_ERROR
 * @see parse_opts.hpp
 */
int PARSER::onErrorMissingLongOption( void )
{
   cerr << getProgramName() << ": missing long option --???" << endl;
   return -1;
}

/*!----------------------------------------------------------------------------
 * @ingroup OPT_ERROR
 * @see parse_opts.hpp
 */
int PARSER::onErrorUnrecognizedShortOption( char unrecognized )
{
   cerr << getProgramName()
        << ": unrecognized option -"
        << unrecognized
        << endl;
   return 0;
}

/*!----------------------------------------------------------------------------
 * @ingroup OPT_ERROR
 * @see parse_opts.hpp
 */
int PARSER::onErrorUnrecognizedLongOption( const string& unrecognized )
{
   cerr << getProgramName()
        << ": unrecognized long option --"
        << unrecognized
        << endl;
   return 0;
}

/*!----------------------------------------------------------------------------
 * @ingroup OPT_ERROR
 * @see parse_opts.hpp
 */
int PARSER::onErrorlongOptionalArg( void )
{
   cerr << getProgramName()
        << ": missing argument after '=' of long option --"
        << getCurrentOption()->m_longOpt
        << endl;
   return -1;
}

/*!----------------------------------------------------------------------------
 * @ingroup OPT_ERROR
 * @see parse_opts.hpp
 */
int PARSER::onErrorShortOptionalArg( void )
{
   assert( dynamic_cast<OPTION*>(m_pCurrentOption) != nullptr );
   cerr << getProgramName()
        << ": missing argument after '=' of short option -"
        << getCurrentOption()->m_shortOpt
        << endl;
   return -1;
}

/*!----------------------------------------------------------------------------
 * @ingroup OPT_ERROR
 * @see parse_opts.hpp
 */
int PARSER::onErrorShortMissingRequiredArg( void )
{
   cerr << getProgramName()
        << ": missing argument for option \'"
        << getCurrentOption()->m_shortOpt
        << '\''<< endl;
   return -1;
}

/*!----------------------------------------------------------------------------
 * @ingroup OPT_ERROR
 * @see parse_opts.hpp
 */
int PARSER::onErrorLongMissingRequiredArg( void )
{
   cerr << getProgramName()
        << ": missing argument of long option --"
        << getCurrentOption()->m_longOpt
        << endl;
   return -1;
}

} // namespace CLOP

//================================== EOF ======================================
