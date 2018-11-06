/*!
 *
 * @brief     Example for using C++ in LM32 environment.
 *
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
 */
/*! !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * @note Implementing C++ source in LM32 target requires some limitations
 *       in contrast to C++ in a OS like Linux, MacOS or MS-Windows.
 *
 *       1) No exception handling by the C++ keywords "try", "throw" and "catch".
 *          Disable the exception handling by the command line option
 *          "-fno-exceptions" in your Makefile.
 *
 *       2) No Run Time Type Information (rtti).
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
 *
 * Conclusion:
 * The using of C++ in a LM32-target is similar like the code of
 * a so called "Arduino-Sketch" because these are written in C++ as well.
 *
 * What is possible in any cases can you see in this example below.
 * And I think that's a lot! :-) UB
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */

#include "mprintf.h"
#include "mini_sdb.h"

extern "C" void __cxa_pure_virtual() {} // Workaround not nice, I know... :-/

/////////////////////////////////////////////////////////////////////////////////
class SysInit
{
public:
   SysInit( void )
   {
      discoverPeriphery(); // mini-sdb: get info on important Wishbone infrastructure
      uart_init_hw();      // init UART, required for printf...
      mprintf("\nHello world in C++\n");
   }

   ~SysInit( void )
   {
      mprintf( "End...\n" );
   }
};

////////////////////////////////////////////////////////////////////////////////
class Base
{
public:
   Base( void );
   ~Base( void );
   virtual void onSomething( void ) = 0;
};

//-----------------------------------------------------------------------------
Base::Base( void )
{
   mprintf( "Coustructor of class \"Base\"\n" );
}

//-----------------------------------------------------------------------------
Base::~Base( void )
{
   mprintf( "Destructor of class \"Base\"\n" );
}

///////////////////////////////////////////////////////////////////////////////
class Foo: public Base
{
public:
   Foo( void );
   ~Foo( void );
   void onSomething( void ) override;
};

//-----------------------------------------------------------------------------
Foo::Foo( void )
{
   mprintf( "Coustructor of class \"Foo\"\n" );
}

//-----------------------------------------------------------------------------
Foo::~Foo( void )
{
   mprintf( "Destructor of class \"Foo\"\n" );
}

//-----------------------------------------------------------------------------
void Foo::onSomething( void )
{
   mprintf( "Function \"Foo::onSomething\"\n" );
}

///////////////////////////////////////////////////////////////////////////////
class Bar: public Base
{
public:
   Bar( void );
   ~Bar( void );
   void onSomething( void ) override;
};

//-----------------------------------------------------------------------------
Bar::Bar( void )
{
   mprintf( "Coustructor of class \"Bar\"\n" );
}

//-----------------------------------------------------------------------------
Bar::~Bar( void )
{
   mprintf( "Destructor of class \"Bar\"\n" );
}

//-----------------------------------------------------------------------------
void Bar::onSomething( void )
{
   mprintf( "Function \"Bar::onSomething\"\n" );
}

//=============================================================================
int main( void )
{
   SysInit sysInit;

   Foo foo;
   Bar bar;

   Base* pBase = &foo;
   pBase->onSomething();

   pBase = &bar;
   pBase->onSomething();

   // Checking whether the modern C++ keyword "nullptr" is usable.
   pBase = nullptr;

   // Checking whether a C++11 lambda-function is possible.
   auto myLambda = [](int a, int b) { return a + b; };

   mprintf( "Return of lambda: %d\n", myLambda( 30, 12 ) );

   return 0;
}

//================================== EOF ======================================
