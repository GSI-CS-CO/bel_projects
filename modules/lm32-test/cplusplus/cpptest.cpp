/*!
 * @brief     Example for using C++ in LM32 environment
 * @file      cpptest.cpp
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      06.11.2018
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
 *
 * This example shows what's at the moment possible coding in C++11 for a
 * LM32 soft core processor.
 *
 * To compile this module, the LM32 toolchain lm32-elf-gcc version must be at
 * least 7.3.0 or higher.
 * You can obtain a actual tool-chain by cloning the following git-repository:
 * @code
 * $ git clone https://github.com/UlrichBecker/gcc-toolchain-builder.git
 * @endcode
 * Change in the new directory
 * @code
 * $ cd gcc-toolchain-builder
 * @endcode
 * Initialize the shell variable "PREFIX" by the path where the tool chain
 * shall be installed. E.g.:
 * @code
 * export PREFIX=$HOME/.local/bin
 * @endcode
 * Invoke the shell-script "build-lm32-toolchain.sh"
 * @code
 * $ ./build-lm32-toolchain.sh
 * @endcode
 *
 * ... Have a break of at least 45 minutes... ;-)
 *
 */
/*! !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * @note Implementing C++ source in LM32 target requires some limitations
 *       in contrast to C++ in a OS like Linux, MacOS or MS-Windows.
 *
 *       1) No exception handling by the C++ keywords "try", "throw" and "catch".
 *          Disable the exception handling by the command line option
 *          "-fno-exceptions" in your Makefile.
 *
 *       2) No Run Time Type Information (rtti) that means
 *          don't use the cast operator "dynamic_cast<>()" as well. \n
 *          Disable RTTI by the command line option "-fno-rtti" in your Makefile.
 *
 *       3) Avoid the using of "new" respectively "delete" except you writes
 *          your own "new" and "delete" by operator-overloading.
 *
 *       4) Please avoid the using of the Standard Template Library (STL),
 *          not to mention the use of BOOST! :-O
 *
 *       5) Despite whether the module is running in a OS-environment or not,
 *          the C++ compiler expects a return value of the type "int" of
 *          the function "main()". \n
 *          E.g.:
 *          @code
 *          int main( void )
 *          {
 *             // ... more or less meaningful C++ code ...
 *             return 0; // Important for C++!
 *          }
 *          @endcode
 *
 *       6) The included C header-files must be embedded in a C++ wrapper. \n
 *          E.g.:
 *          @code
 *          #ifdef __cplusplus
 *          extern "C" {
 *          #endif
 *
 *          int foo( void );
 *          void bar( int );
 *
 *          #ifdef __cplusplus
 *          }
 *          #endif
 *          @endcode
 *
 *       7) Don't use the stream-operators for eb-console outputs E.g.:
 *          @code
 *          std::cout << "bla bla" << std::endl; // Wrong!
 *          @endcode
 *          Use the C-function "mprintf" instead. E.g.: \n
 *          @code
 *          mprintf("bla bla\n"); // Correct!
 *          @endcode
 *          Except you writes your own stream-operators like
 *          the example below in this file.
 *
 * Conclusion:
 * The using of C++ in a LM32-target is similar like the code of
 * a so called "Arduino-Sketch" because these are written in C++ as well.
 *
 * What is possible in any cases can you see in this example below.
 * And I think that's a lot! :-) UB
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */
#include "eb_console_helper.h"
#include "mini_sdb.h"
#include "helper_macros.h"


#if COMPILER_VERSION_NUMBER < 70300
  #error This module requieres the gcc-version 7.3.0 or higher.
#endif
#ifndef __lm32__
  #error This module is for the target LM32 only!
#endif

/////////////////////////////////////////////////////////////////////////////////
class SysInit
{
public:
   SysInit( void )
   {
      ::discoverPeriphery(); // mini-sdb: get info on important Wishbone infrastructure
      ::uart_init_hw();      // init UART, required for printf...
      gsi::clrscr();
      gsi::gotoxy( 0, 0 );
      ::mprintf( ESC_FG_MAGENTA
                 "Hello world in C++11, Compiler: " COMPILER_VERSION_STRING "\n"
                 ESC_NORMAL );
   }

   ~SysInit( void )
   {
      ::mprintf( ESC_FG_MAGENTA "End...\n" ESC_NORMAL );
   }
};

/*! @todo Global initialization respectively constructor-calls
 *        - in this case: "SysInit g_sysInit" - will not work at now yet. \n
 *        This issue could be in the start-up module "crt0.S" respectively in
 *        "crt0.o" written in assembler, which is responsible for the calling
 *        the function "main" and the interrupt-handler as well. \n
 *        The providing of the start-up module "crt0" is a part of
 *        the "newLib" rather than of "gcc" and becomes compiled in
 *        the second stage during the toolchain-building. \n
 *        "ftp://sources.redhat.com/pub/newlib/newlib-<version>.tar.gz"
 */
//void * __dso_handle;
// SysInit g_sysInit; // Geht ned!!! ;-(


///////////////////////////////////////////////////////////////////////////////
/*!
 * @brief We build our own mini "ostream" library for test purposes only.
 *
 * But it's very nice to have...
 */
class OverloadingTest
{
public:
   void operator () ( void )
   {
      *this << "I'm a so called functor: \"OverloadingTest::operator()\"\n";
   }

   OverloadingTest& operator << ( const char* str )
   {
      ::mprintf( str );
      return *this;
   }

   OverloadingTest& operator << ( char c )
   {
      ::mprintf( "%c", c );
      return *this;
   }

   OverloadingTest& operator << ( int i )
   {
      ::mprintf( "%d", i );
      return *this;
   }
};

////////////////////////////////////////////////////////////////////////////////
/*!
 * @brief Test of the C++ hidden virtual function table respectively abstract
 *        C++ classes and its virtual methods.
 */
class Base
{
protected:
   OverloadingTest& m_rOut;

public:
   Base( OverloadingTest& );
   ~Base( void );
   virtual void onSomething( void ) = 0;
};

//-----------------------------------------------------------------------------
Base::Base( OverloadingTest& rOut ):
   m_rOut( rOut )
{
   m_rOut << "Coustructor of class \"Base\"\n";
}

//-----------------------------------------------------------------------------
Base::~Base( void )
{
   m_rOut << "Destructor of class \"Base\"\n";
}

///////////////////////////////////////////////////////////////////////////////
class Foo: public Base
{
public:
   Foo( OverloadingTest& );
   ~Foo( void );
   void onSomething( void ) override;
};

//-----------------------------------------------------------------------------
Foo::Foo( OverloadingTest& rOut ):
   Base( rOut )
{
   m_rOut << "Coustructor of class \"Foo\"\n";
}

//-----------------------------------------------------------------------------
Foo::~Foo( void )
{
   m_rOut << "Destructor of class \"Foo\"\n";
}

//-----------------------------------------------------------------------------
void Foo::onSomething( void )
{
   m_rOut << "Function \"Foo::onSomething\"\n";
}

///////////////////////////////////////////////////////////////////////////////
class Bar: public Base
{
public:
   Bar( OverloadingTest& );
   ~Bar( void );
   void onSomething( void ) override;
};

//-----------------------------------------------------------------------------
Bar::Bar( OverloadingTest& rOut ):
   Base( rOut )
{
   m_rOut << "Coustructor of class \"Bar\"\n";
}

//-----------------------------------------------------------------------------
Bar::~Bar( void )
{
   m_rOut << "Destructor of class \"Bar\"\n";
}

//-----------------------------------------------------------------------------
void Bar::onSomething( void )
{
   m_rOut << "Function \"Bar::onSomething\"\n";
}

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 * @brief This class tests whether the macro CONTAINER_OF() works properly
 *        in C++ respectively the macro offsetof().
 * @note The structure respectively class has to be a so called plain type. \n
 *       That means the class members shouldn't to be reverences and/or
 *       virtual functions. Otherwise the following compiler warning will
 *       appear: \n
 *       <b>warning: offsetof within non-standard-layout type </b> \n
 *       In other words, the members has to be C- compatible.
 */
class Container
{
   class Content
   {
   public:
      Content( void );
      ~Content( void );
   };

   /*
    * For plain types the member m_pOut shall be a pointer rather than a
    * C++ reverence.
    */
   OverloadingTest* m_pOut;

   /*
    * The following member has to be implemented plain, otherwise
    * the macro CONTAINER_OF will crash!
    * That means, not as a pointer or a C++ reverence.
    */
   Content          m_oContent;

public:
   Container( OverloadingTest& rOut );
   ~Container( void );
};

//-----------------------------------------------------------------------------
Container::Container( OverloadingTest& rOut ):
   m_pOut( &rOut )
{
   *m_pOut << "Constructor of class \"Container\"\n";
}

//-----------------------------------------------------------------------------
Container::~Container( void )
{
   *m_pOut << "Destructor of class \"Container\"\n";
}

//-----------------------------------------------------------------------------
Container::Content::Content( void )
{
   *CONTAINER_OF( this, Container, m_oContent )->m_pOut << "Constructor of class \"Container::Content\"\n";
}

//-----------------------------------------------------------------------------
Container::Content::~Content( void )
{
   *CONTAINER_OF( this, Container, m_oContent )->m_pOut << "Destructor of class \"Container::Content\"\n";
}

///////////////////////////////////////////////////////////////////////////////
/*!
 * If its possible to write a template alternatively to a preprocessoer macro,
 * so prefer the template because of the better type-checking and the lower
 * likelihood to produce bugs.
 */
template <typename TYP> TYP min( TYP a, TYP b )
{
   return (a < b)? a : b;
}

//=============================================================================
int main( void )
{
   SysInit sysInit;
//#define sysInit g_sysInit
#if 1
   OverloadingTest ov;
   ov();

   Container container( ov );

   Foo foo( ov );
   Bar bar( ov );

   Base* pBase = &foo;
   pBase->onSomething();

   pBase = &bar;
   pBase->onSomething();

   // Checking whether a C++11 lambda-function is possible.
   auto myLambda = [](int a, int b) -> int { return a + b; };

   ov << "Return of lambda: " << myLambda( 30, 12 ) << '\n';

   ov << "Template-test: " << min( 4711, 42 ) << '\n';

   // Test of the template "convertByteEndian" defined in "helper-macros.h"
   uint32_t x = 0x11223344;
   mprintf( "Endian convert test: 0x%x --> 0x%x\n", x, gsi::convertByteEndian( x ) );
#endif
   return 0;
}

//================================== EOF ======================================
