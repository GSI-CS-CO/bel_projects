/*!
 * @see
 * <a href="https://github.com/UlrichBecker/command_line_option_parser_cpp11">
 * GitHub Repository</a>
 */
/*****************************************************************************/
/*                                                                           */
/*!      @brief Redundancy free command-line option parser for C++>=11       */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*! @file    parse_opts.hpp                                                  */
/*! @see     parse_opts.cpp                                                  */
/*  Library: libParseOptsCpp11.so, resp: -lParseOptsCpp11                    */
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

//! @example cmd_opt_test.cpp
//! @example cmd_opt_lambda.cpp
//! @example cmd_opt_objects.cpp

#ifndef _PARSE_OPTS_HPP
#define _PARSE_OPTS_HPP

#include <assert.h>
#include <type_traits>
#include <string>
#include <iostream>
#include <vector>

//! @brief Name-space of the Command-Line-Option-Parser.
namespace CLOP
{

class PARSER;
///////////////////////////////////////////////////////////////////////////////
//! @brief Base-type for each single option.
struct OPTION
{
   friend class PARSER;

public:
   //! @brief Signature of the option callback functions.
   //! @param PARSER* Pointer to the object of the parser.
   //! @retval ==0 Function was successful.
   //! @retval <0  Function was not successful. \n
   //!             In this case the parser terminates immediately
   //!             its work by the return-value of this function.
   //! @retval >0  Function was not successful. \n
   //!             In this case the parser continues its work but
   //!             the return value of the parser will be -1.
   typedef int (*FUNC_T)( PARSER* );

   //! @brief Definition of the option-type.
   enum ARG_REQUIRE_T
   {
   #ifndef CONFIG_CLOP_NO_NO_ARG
      //! @brief Option has no argument.
      NO_ARG       = 0
    #if !defined( CONFIG_CLOP_NO_REQUIRED_ARG ) || !defined( CONFIG_CLOP_NO_OPTIONAL_ARG )
      ,
    #endif
   #endif
   #ifndef CONFIG_CLOP_NO_REQUIRED_ARG
      //! @brief Option needs a argument.
      REQUIRED_ARG = 1
    #if !defined( CONFIG_CLOP_NO_OPTIONAL_ARG )
      ,
    #endif
   #endif
   #ifndef CONFIG_CLOP_NO_OPTIONAL_ARG
      //! @brief Option can have a argument but must not.
      //! @note  If a argument for this option type is given, \n
      //!        so the usage of the '=' character in the command line
      //!        becomes necessary.
      OPTIONAL_ARG = 2
   #endif
   };

   //! @brief Mandatory pointer to the corresponding callback-function.\n
   //!        (Function pointer or lambda-function.)
   FUNC_T        m_func;

   //! @brief Determines whether the option needs a additional argument.
   //! @see CLOP::OPTION::ARG_REQUIRE_T
   ARG_REQUIRE_T m_hasArg;

   //! @brief Optional option identifier to distinguish options
   //!        with the same callback-function. \n 
   //!        (Nice to have.)
   int           m_id;

   //! @brief Character for the short-option "-o". \n
   //!        Mandatory when CLOP::OPTION::m_longOpt is omitted. \n
   //! @note  Initialize it by zero '\0' if it won’t used.
   char          m_shortOpt;

   //! @brief String for the long-option "--option". \n
   //!        Mandatory when CLOP::OPTION::m_shortOpt is omitted.
   //! @note  Initialize it by "" if it won’t used.
   std::string   m_longOpt;

   //! @brief Additional help-text for the corresponding option. \n
   //!        You can omit this in your initializer by initializing it by "",\n
   //!        but that's not a good idea. ;-)
   std::string   m_helpText;

   //! @brief Prints the short option (if present) and long option
   //!        (if present) of this object in the given output-stream.
   //! @param out Output stream e.g.: cout or cerr.
   void print( std::ostream& out );
};

//! @brief Macro to implementing anonymous functions directly in the initializer
//!        of the option list.
//!
//! This "nice to have" macro is useful when the call-back function is very small,
//! e.g.: for setting or resetting flags. \n
//! Example:
//! @code
//! class MY_PARSER: public CLOP::PARSER
//! {
//!    bool m_verbose;
//! public:
//!    void setVerbose( bool v ) { m_verbose = v; }
//! };
//!
//! vector<CLOP::OPTION> optList =
//! {
//!    {
//!       OPT_LAMBDA( poParser, { static_cast<MY_PARSER*>(poParser)->setVerbose( true ); return 0; }),
//!       .m_hasArg   = CLOP::OPTION::NO_ARG,
//!       .m_id       = 0,
//!       .m_shortOpt = 'v',
//!       .m_longOpt  = "verbose",
//!       .m_helpText = "Be verbose"
//!    },
//!    // Further options...
//! };
//! @endcode
//! @param argName Name of the argument variable which is
//!                the pointer to the object of the parsers base-class CLOP::PARSER*.
//! @param body    Function body in {}. @see CLOP::OPTION::FUNC_T
#define OPT_LAMBDA( argName, body ) \
   .m_func = [] (CLOP::PARSER* argName) -> int body


///////////////////////////////////////////////////////////////////////////////
//! @brief Specializing of the base-type OPTION for using virtual callback
//!        functions and single objects per option, instead of using
//!        initializer-lists and lambda- or function-pointers.
class OPTION_V: public OPTION
{
public:
   //! @brief Constructor
   OPTION_V( void );

   //! @brief Destructor
   virtual ~OPTION_V( void ) {}

   //! @brief Callback-function becomes immediately invoked from the parser
   //!        when the corresponding command-line-option (short or long)
   //!        was recognized.
   //! @see CLOP::OPTION::FUNC_T
   //! @param poParser Pointer to the object of the parser.
   //! @retval ==0 Function was successful.
   //! @retval <0  Function was not successful. \n
   //!             In this case the parser terminates immediately
   //!             its work by the return-value of this function.
   //! @retval >0  Function was not successful. \n
   //!             In this case the parser continues its work but
   //!             the return value of the parser will be -1.
   virtual int onGiven( PARSER* poParser ) = 0;

private:
   static int adapter( PARSER* poParser );
};

///////////////////////////////////////////////////////////////////////////////
//! @brief Base-class of the command-line-option-parser.
//!
//! Example
//! @code
//! #include <parse_opts.hpp>
//! using namespace CLOP;
//! using namespace std;
//!
//! int main( int argc, char** ppArgv )
//! {
//!    vector<OPTION> optList =
//!    {
//!       {
//!          OPT_LAMBDA( parser,
//!          {
//!             cout << "Usage: " << parser->getProgramName() << " [options,...]\n";
//!             parser->list( cout );
//!             cout << endl;
//!             exit( EXIT_SUCCESS );
//!             return 0;
//!          }),
//!          .m_hasArg   = OPTION::NO_ARG,
//!          .m_id       = 0,
//!          .m_shortOpt = 'h',
//!          .m_longOpt  = "help",
//!          .m_helpText = "Print this help and exit"
//!       },
//!       //further items...
//!   };
//!   PARSER parser( argc, ppArgv, optList );
//!   for( int i = 1; i < argc; i++ )
//!   {
//!      i = parser( i );
//!      if( i < 0 )
//!         return EXIT_FAILURE;
//!      if( i < argc )
//!         cout << "Non option argument on index " << i << ": " << ppArgv[i] << endl;
//!   }
//!   return EXIT_SUCCESS;
//! }
//! @endcode
class PARSER
{
   const char* const*   m_ppArgv;
   const int            m_argc;
   int                  m_index;
   OPTION*              m_pCurrentOption; //!<@note Only valid during the parsing-time!
   std::string          m_optArg;
   bool                 m_allowNegativeDigits;

protected:
   std::vector<OPTION*> m_optPtrList; //!<@brief Internal pointer-list of type CLOP::OPTION*

public:
   //! @defgroup OPT_ADD Functions to add single option-objects or
   //!                   initialized option-lists to the parser.
   //! @{

   //! @brief Adds a single object of the base-type OBJECT to the internal
   //!        object list.
   //! @param rOptionObj Option-object of base-type CLOP::OPTION
   //! @retval: Reverence to its own object.
   template< typename O_T >
   PARSER& add( O_T& rOptionObj )
   {
      static_assert( std::is_class<O_T>::value,
                     "Option type is not a class!" );
      static_assert( std::is_base_of<OPTION, O_T>::value,
                     "Option type has not the base CLOP::OPTION" );
      assert( dynamic_cast<OPTION*>(&rOptionObj) != nullptr );
      m_optPtrList.push_back( &rOptionObj );
      return *this;
   }

   //! @brief Adds a single object of the base-type OBJECT to the internal
   //!        object list.
   //! @param rOptionObj Option-object of base-type CLOP::OPTION
   //! @retval: Reverence to its own object.
   template< typename O_T >
   PARSER& operator()( O_T& rOptionObj )
   {
      return add( rOptionObj );
   }

   //! @brief Adds a initialized static list (container std::vector) to
   //!        the parsers internal object-list.
   //! @param rOptList Initialized list of option-objects containing in std::vector<>.
   //! @retval: Reverence to its own object.
   template< typename OL_T >
   PARSER& add( std::vector<OL_T>& rOptList )
   {
      static_assert( std::is_class<OL_T>::value,
                     "Option type is not a class!" );
      static_assert( std::is_base_of<OPTION, OL_T>::value,
                     "Option type has not the base CLOP::OPTION" );
      for( auto& it : rOptList )
         add( it );
      return *this; 
   }

   //! @brief Adds a initialized static list (container std::vector) to
   //!        the parsers internal object-list.
   //! @param rOptList Initialized list of option-objects containing in std::vector<>.
   //! @retval: Reverence to its own object.
   template< typename OL_T >
   PARSER& operator()( std::vector<OL_T>& rOptList )
   {
      return add( rOptList );
   }

   //! @} End of defgroup OPT_ADD

   //! @defgroup OPT_CONSTRUCTION
   //! @{

   //! @brief Constructor
   //! @param argc The first argument of your main() function
   //! @param ppArgv Argument vector the second argument of your main() function.
   //! @param allowNegativeDigits If this parameter == true, so the parser 
   //!        allows digits [-0..-9] as short option.\n
   //!        If == false (default) digits are traded as negative numbers.
   PARSER( int argc, char** ppArgv, bool allowNegativeDigits = false );

   //! @brief Constructor
   //! @param argc The first argument of your main() function
   //! @param ppArgv Argument vector the second argument of your main() function
   //! @param rOptList Initialized list of option-objects containing in std::vector<>.
   //! @param allowNegativeDigits If this parameter == true, so the parser
   //!        allows digits [-0..-9] as short option.\n
   //!        If == false (default), digits are traded as negative numbers.
   template< typename OL_T >
   PARSER( int argc,
           char** ppArgv,
           std::vector<OL_T>& rOptList,
           bool allowNegativeDigits = false ):
      m_ppArgv( ppArgv ),
      m_argc( argc ),
      m_pCurrentOption( nullptr ),
      m_allowNegativeDigits( allowNegativeDigits )
   {
      static_assert( std::is_class<OL_T>::value,
                     "Option type is not a class!" );
      static_assert( std::is_base_of<OPTION, OL_T>::value,
                     "Option type has not the base CLOP::OPTION" );
      add( rOptList );
   }

   //! @brief Destructor
   virtual ~PARSER( void ) {}

   //! @} End of defgroup OPT_CONSTRUCTION

   //! @defgroup OPT_ITERATOR
   //! @{

   //! @brief Get the iterator to the beginning of the option pointer list.
   //! @retval: Iterator to the beginning of the option pointer list of
   //!          type CLOP::OPTION*.
   const std::vector<OPTION*>::iterator begin( void )
   {
      return m_optPtrList.begin();
   }

   //! @brief Get the iterator to the end of the option pointer list.
   //! @retval: Iterator to the end of the option pointer list of
   //!          type CLOP::OPTION*.
   const std::vector<OPTION*>::iterator end( void )
   {
      return m_optPtrList.end();
   }

   //! @} End of defgroup OPT_ITERATOR

   
   //! @brief (Re-)Starts the parsing.
   //! @param offset Start-offset, initializer of the index for ppArgv[].
   //!        default is 1.
   //! @retval <0 Parsing was not successful.
   //! @retval >0 Index-value to the first non-option argument after
   //!            possible options if present.
   int parse( int offset = 1 );

   //! @brief (Re-)Starts the parsing.
   //! @param offset Start-offset, initializer of the index for ppArgv[].
   //!        default is 1.
   //! @retval <0 Parsing was not successful.
   //! @retval >0 Index-value to the first non-option argument after
   //!            possible options if present.
   int operator()( int offset = 1 ) { return parse( offset ); }

   //! @brief (Re-)Starts the parsing.
   //! @param offset Start-offset, initializer of the index for ppArgv[].
   //!        default is 1.
   //! @param optList Initialized list of option-objects containing in std::vector<>.
   //! @retval <0 Parsing was not successful.
   //! @retval >0 Index-value to the first non-option argument after
   //!            possible options if present.
   template< typename OL_T >
   int parse( std::vector<OL_T>& optList, int offset = 1 )
   {
      static_assert( std::is_class<OL_T>::value,
                     "Option type is not a class!" );
      static_assert( std::is_base_of<OPTION, OL_T>::value,
                     "Option type has not the base CLOP::OPTION" );
      addList( optList );
      return parse( offset );
   }

   //! @brief (Re-)Starts the parsing.
   //! @param offset Start-offset, initializer of the index for ppArgv[].
   //!        default is 1.
   //! @param optList Initialized list of option-objects containing in std::vector<>.
   //! @retval <0 Parsing was not successful.
   //! @retval >0 Index-value to the first non-option argument after
   //!            possible options if present.
   template< typename OL_T >
   int operator()( std::vector<OL_T>& optList, int offset = 1 )
   {
      static_assert( std::is_class<OL_T>::value,
                     "Option type is not a class!" );
      static_assert( std::is_base_of<OPTION, OL_T>::value,
                     "Option type has not the base CLOP::OPTION" );
      return parse( optList, offset );
   }

   //! @brief Gets the actual index-value of ppArgv[] after parsing,
   //!        to parse non-option arguments.
   //! @retval Index-value
   int getArgIndex( void ) const { return m_index; }

   //! @brief Gets the number of all arguments in the command-line.
   //!        Value of the second argument in function "main()".
   //! @retval: Total number of command-line arguments.
   int getArgCount( void ) const { return m_argc; }

   //! @brief Gets the start address of the argument-vector.
   //!        First argument of the function "main()".
   //! @retval Start-address of argument-vector.
   const char* const* getArgVect( void ) const { return m_ppArgv; }

   //! @defgroup CALL_BACK_HELPER Helper functions for the callback
   //!           functions.
   //! @{

   //! @brief Indicates whether the current option has a argument.
   //!
   //! This can be used within callback-functions of corresponding options
   //! which has a optional parameter to indicate whether a additional
   //! argument is given or not.
   //! @see CLOP::OPTION::OPTIONAL_ARG
   //! @retval ==true Current option has a argument.
   //! @retval ==false Current doesn't have a argument.
   //! @see CLOP::PARSER::getOptArg
   //! @note The use of this function is only within the
   //!       callback-functions of base CLOP::OPTION allowed!
   //!       Otherwise a assertion will occur!
   bool isOptArgPersent( void ) const
   {
      assert( m_pCurrentOption != nullptr );
      return !m_optArg.empty();
   }

   //! @brief Get the character-string of the argument (if present)
   //!        of the current option.
   //! @retval Character-string of the argument of the current option.
   //! @note The use of this function is only within the
   //!       callback-functions of base CLOP::OPTION allowed!
   //!       Otherwise a assertion will occur!
   std::string& getOptArg( void )
   {
      assert( m_pCurrentOption != nullptr );
      return m_optArg;
   }

   //! @brief Get the pointer of the current option-object.
   //!
   //! Because of lambda-functions or the function-pointer they are not
   //! real virtual member-functions of the base CLOP::OPTION, therefore
   //! this function is necessary.
   //! @retval: Pointer of the current option-object
   //! @note The use of this function is only within the
   //!       callback-functions of base CLOP::OPTION allowed!
   //!       Otherwise a assertion will occur!
   OPTION* getCurrentOption( void ) const 
   {
      assert( m_pCurrentOption != nullptr );
      return m_pCurrentOption;
   }

   //! @} End of defgroup CALL_BACK_HELPER

   //! @brief Returns the program-name (and path).
   //! @retval: Program-name (and path)
   const std::string getProgramName( void ) const { return m_ppArgv[0]; }

   //! @brief Prints the formatted content of each element of your option-block-list
   //!        in the given output-stream
   //! 
   //! Helper-function can be used to simplify your print-help-function. \n
   //! Example:
   //! @code
   //! std::vector<CLOP::OPTION> optList = 
   //! {
   //!    {
   //!       OPT_LAMBDA( parser,
   //!       {
   //!          assert( !parser->isOptArgPersent() );
   //!          std::cout << "bla bla bla...";
   //!          std::cout << "Usage: " << parser->getProgramName() << " [options,...]\n";
   //!          parser->list( std::cout );
   //!          std::cout << endl;
   //!          exit( EXIT_SUCCESS );
   //!          return 0;
   //!       }),
   //!       .m_hasArg   = OPTION::NO_ARG,
   //!       .m_id       = 0,
   //!       .m_shortOpt = 'h',
   //!       .m_longOpt  = "help",
   //!       .m_helpText = "Print this help and exit"
   //!    },
   //!    // Further options...
   //! };
   //! @endcode
   //! @param out Output stream e.g.: std::cout or std::cerr.
   void list( std::ostream& out );

   //! @defgroup OPT_SORT
   //! @{

   //! @brief Sorts the short options of the internal option-container
   //!        in a lexical order.
   //!
   //! This is the order which appears e.g. by the function CLOP::PARSER::list.
   //! @param down == false (default): Rising sequence.
   //! @param down == true: Falling sequence.
   void sortShort( bool down = false );

   //! @brief Sorts the long options of the internal option-container
   //!        in a lexical order.
   //!
   //! This is the order which appears e.g. by the function CLOP::PARSER::list.
   //! @param down == false (default): Rising sequence.
   //! @param down == true: Falling sequence.
   void sortLong( bool down = false );

   //! @} End of defgroup OPT_SORT

   //! @defgroup OPT_USING_DIGITS
   //! @{

   //! @brief Indicates whether using digits as short options [0..9]
   //!        are allowed or not.
   //! @retval true  Digits allowed
   //! @retval false Digits not allowed
   bool nagativeDigitsAllowed( void ) const { return m_allowNegativeDigits; }

   //! @brief Allows to use digits [0..9] as short options.
   //! @param allow == true (default): Digits as short options allowed.
   //! @param allow == false: Using digits as short options are not allowed.
   void allowNegativeDigits( bool allow = true ) { m_allowNegativeDigits = allow;  }

   //! @} End of defgroup OPT_USING_DIGITS

protected:
   //! @brief Function becomes invoked by each recognized non-option argument
   //!        of the command-line during parsing.
   //!
   //! In this manner it becomes possible to parse the complete command-line
   //! if the return value is greater zero.
   //! @example cmd_opt_objects.cpp
   //!
   //! @retval ==0 Option-parsing will terminated (default). \n
   //!             (For classical order: first options; second non-options.)
   //! @retval >0  Option-parsing will continued. \n
   //!             (For mixed order of options and non-options.)
   //! @retval <0  Option-parsing will immediately terminated and the
   //!             return value of the parser will be the negative
   //!             return-value of this function.
   virtual int onArgument( void ) { return 0; }

   //! @defgroup OPT_ERROR Over writable callback functions for error-handling.
   //! @brief This overwritable functions gives the possibility for example \n
   //!        to translate the error-messages into other languages and/or \n
   //!        to introduce the C++ exception-handling.
   //! @{

   //! @brief Becomes triggered if after the option-introducer '-' no option
   //!        follows.
   //! @retval: != 0 Parser terminates immediately by the return-value
   //!          of this value.
   //! @retval: == 0 Parser continues its work.
   //! @note If the option-introducer '-' shall be a single non-option-argument
   //!       so the function can be overwritten as follows:
   //! @code
   //! int MY_PARSER::onErrorMissingOption( void )
   //! {
   //!    onArgument(); // If this function is also overwritten.
   //!    return getArgIndex();
   //! }
   //! @endcode
   virtual int onErrorMissingOption( void );

   //! @brief Becomes triggered if after a long option-introducer '--' doesn't
   //!        follows the option-keyword.
   //! @retval: != 0 Parser terminates immediately by the return-value
   //!          of this value.
   //! @retval: == 0 Parser continues its work.
   //! @note If the long option-introducer '--' shall be a single non-option-argument
   //!       so the function can be overwritten as follows:
   //! @code
   //! int MY_PARSER::onErrorMissingLongOption( void )
   //! {
   //!    onArgument(); // If this function is also overwritten.
   //!    return getArgIndex();
   //! }
   //! @endcode
   virtual int onErrorMissingLongOption( void );

   //! @brief Becomes triggered if a short-potion in the command-line was not
   //!        found in the internal option-container.
   //! @param unrecognized Unrecognized character form the command-line.
   //! @retval <0 Parser returns immediately by -1.
   //! @retval >=0 Parser continues its work and returns by -1.
   virtual int onErrorUnrecognizedShortOption( char unrecognized );

   //! @brief Becomes triggered if a long-potion in the command-line was not
   //!        found in the internal option-container.
   //! @param unrecognized Unrecognized keyword-string form the command-line.
   //! @retval <0 Parser returns immediately by -1.
   //! @retval >=0 Parser continues its work and returns by -1.
   virtual int onErrorUnrecognizedLongOption( const std::string& unrecognized );

   //! @brief Becomes triggered if a error in a long optional argument after '='
   //!        occurs.
   //! @see CLOP::OPTION::OPTIONAL_ARG
   //! @retval <0 Parser returns immediately by this negative value. (default)
   //! @retval ==0 Parser continues its work.
   //! @retval >0 Parser continues its work but returns by -1.
   virtual int onErrorlongOptionalArg( void );

   //! @brief Becomes triggered if a error in a short optional argument after '='
   //!        occurs.
   //! @see CLOP::OPTION::OPTIONAL_ARG
   //! @retval <0 Parser returns immediately by this negative value. (default)
   //! @retval ==0 Parser continues its work.
   //! @retval >0 Parser continues its work but returns by -1.
   virtual int onErrorShortOptionalArg( void );

   //! @brief Becomes triggered if a required argument of a short option
   //!        is missing.
   //! @see CLOP::OPTION::REQUIRED_ARG
   //! @retval <0 Parser returns immediately by this negative value. (default)
   //! @retval ==0 Parser continues its work.
   //! @retval >0 Parser continues its work but returns by -1.
   virtual int onErrorShortMissingRequiredArg( void );

   //! @brief Becomes triggered if a required argument of a long option
   //!        is missing.
   //! @retval <0 Parser returns immediately by this negative value. (default)
   //! @retval ==0 Parser continues its work.
   //! @retval >0 Parser continues its work but returns by -1.
   virtual int onErrorLongMissingRequiredArg( void );

   //! @} End of defgroup OPT_ERROR

private:
   int _parse( int );
}; // End of class PARSER

} // End namespace CLOP
#endif // ifndef _PARSE_OPTS_HPP
//================================== EOF ======================================
