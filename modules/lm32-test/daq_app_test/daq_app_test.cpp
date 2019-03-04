
#include <iostream>
#include <eb_console_helper.h>
#include <daq_administration.hpp>

using namespace daq;
using namespace std;

int main( int argc, const char** ppArgv )
{
   cout << ESC_FG_MAGNETA << ppArgv[0] << ESC_NORMAL << endl;

   try
   {
      DaqAdmin oDaqInterface( ppArgv[1] );
      cout << "WB-interface: " << oDaqInterface.getWbDevice() << endl;

      DaqInterface::SLOT_FLAGS_T flags = oDaqInterface.readSlotStatus();
      for( unsigned int i = 1; i < DaqInterface::c_maxDevices; i++ )
      {
         if( DaqInterface::isDevicePresent( flags, i ) )
         {
            cout << "Slot: " << i << endl;
         }
      }
   }
   catch( DaqInterface::Exception& e )
   {
      cerr << ESC_FG_RED "DaqInterface::Exception occurred: " << e.what() << endl;
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
