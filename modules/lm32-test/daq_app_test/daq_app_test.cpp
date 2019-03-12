
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
      cout << "WB-interface:        " << oDaqInterface.getWbDevice() << endl;
      cout << "Found devices:       " << oDaqInterface.getMaxFoundDevices() << endl;
      for( unsigned int i = 1; i <= oDaqInterface.getMaxFoundDevices(); i++ )
      {
         DaqDevice* pDevice = new DaqDevice;
         oDaqInterface.registerDevice( pDevice );
         for( unsigned int j = 1; j <= pDevice->getMaxChannels(); j++ )
         {
            DaqChannel* pChannel = new DaqChannel;
            pDevice->registerChannel( pChannel );
         }
      }
      cout << "Registered channels: " << oDaqInterface.getMaxChannels() << endl;
      for( unsigned int i = 1; i <= oDaqInterface.getMaxChannels(); i++ )
      {
         DaqChannel* pChannel = oDaqInterface.getChannelByAbsoluteNumber( i );
         SCU_ASSERT( pChannel != nullptr );
         cout << "Slot:    " << pChannel->getSlot() << endl;
         cout << "Device:  " << pChannel->getDeviceNumber() << endl;
         cout << "Channel: " << pChannel->getNumber() << "\n" << endl;
      }

      DaqChannel* pChannel = oDaqInterface.getChannelByAbsoluteNumber( 1 );
      pChannel->setTriggerDelay( 0x55 );
      pChannel->setTriggerCondition( 0xCAFEAFFE );
      cout << "Returncode: " << oDaqInterface.getLastReturnCodeString() << endl;
      uint16_t delay;
      uint32_t triggerCondition;
      pChannel->getTriggerDelay( delay );
      cout << "Delay: 0x" << hex << delay << endl;
      pChannel->getTriggerCondition( triggerCondition );
      cout << "Trigger condition: 0x" << hex << triggerCondition << endl;
      oDaqInterface.distributeData();
   }
   catch( daq::Exception& e )
   {
      cerr << ESC_FG_RED "daq::Exception occurred: " << e.what() << ESC_NORMAL << endl;
   }
   catch( std::exception& e )
   {
      cerr << ESC_FG_RED "std::exception occurred: " << e.what() << ESC_NORMAL << endl;
   }
   catch( ... )
   {
      cerr << ESC_FG_RED "Undefined exception occurred!" ESC_NORMAL << endl;
   }

   cout << ESC_FG_MAGNETA << "End" << ESC_NORMAL << endl;
   return EXIT_SUCCESS;
}
