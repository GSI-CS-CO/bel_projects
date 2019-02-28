
#include <iostream>
#include <eb_console_helper.h>
#include <daq_interface.hpp>

using namespace daq;
using namespace std;

int main( int argc, const char** ppArgv )
{
   cout << ESC_FG_MAGNETA << ppArgv[0] << ESC_NORMAL << endl;

   try
   {
      Daq oDaq( ppArgv[1] );
      cout << "WB-interface: " << oDaq.getWbDevice() << endl;
   }
   catch( Daq::Exception& e )
   {
      cerr << ESC_FG_RED "Daq::Exception occurred: " << e.what() << endl;
   }
   catch( exception& e )
   {
      cerr << ESC_FG_RED "std::xception occurred: " << e.what() << endl;
   }
   catch( ... )
   {
      cerr << ESC_FG_RED "Undefined exception occurred!" ESC_NORMAL << endl;
   }

   cout << ESC_FG_MAGNETA << "End" << ESC_NORMAL << endl;
   return 0;
}
