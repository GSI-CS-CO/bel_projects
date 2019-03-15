
#include <iostream>
#include <eb_console_helper.h>
#include <daq_channel_container.hpp>
#include <unistd.h>
#include <assert.h>

using namespace daq;
using namespace std;


class MyDaqChannel: public DaqChannel
{
public:
   MyDaqChannel( void ):
      DaqChannel( 0 )
   {
      cout << "Constructor" << endl;
   }

   virtual ~MyDaqChannel( void )
   {
      cout << "Destructor channel: " << getNumber() <<
              ",  slot: " << getSlot() << endl;
   }

   bool onDataInput( DAQ_DATA_T data, bool isPayload ) override;
};

bool MyDaqChannel::onDataInput( DAQ_DATA_T data, bool isPayload )
{
}

///////////////////////////////////////////////////////////////////////////////
int main( int argc, const char** ppArgv )
{
   cout << ESC_FG_MAGNETA << ppArgv[0] << ESC_NORMAL << endl;

   try
   {
      //DaqAdministration oDaqInterface( ppArgv[1] );
      DaqChannelContainer< MyDaqChannel > oDaqInterface( ppArgv[1] );
      cout << "WB-interface:        " << oDaqInterface.getWbDevice() << endl;
      cout << "Found devices:       " << oDaqInterface.getMaxFoundDevices() << endl;

      cout << "Registered channels: " << oDaqInterface.getMaxChannels() << endl;
      for( auto& itDev: oDaqInterface )
      {
         cout << "Slot:  " << itDev->getSlot() << endl;
         for( auto& itChannel: *itDev )
         {
            cout << "   Channel: " << itChannel->getNumber() << endl;
         }
      }

      MyDaqChannel* pChannel = oDaqInterface.getChannelByAbsoluteNumber( 1 );
      assert( pChannel != nullptr );
      pChannel->sendTriggerDelay( 0x55 );
      pChannel->sendTriggerCondition( 0xCAFEAFFE );
      cout << "Returncode: " << oDaqInterface.getLastReturnCodeString() << endl;
      cout << "Delay: 0x" << hex << pChannel->receiveTriggerDelay() << endl;
      cout << "Trigger condition: 0x" << hex << pChannel->receiveTriggerCondition() << dec << endl;
      pChannel->sendEnableContineous( DAQ_SAMPLE_10US );
      usleep( 100000 );
      cout << "Ram level: " << oDaqInterface.getCurrentRamSize(true ) << endl;

      oDaqInterface.distributeData();
   }
   catch( daq::EbException& e )
   {
      cerr << ESC_FG_RED "daq::EbException occurred: " << e.what() << ESC_NORMAL << endl;
   }
   catch( daq::DaqException& e )
   {
      cerr << ESC_FG_RED "daq::DaqException occurred: " << e.what();
      cerr << "\nStatus: " <<  e.getStatusString() <<  ESC_NORMAL << endl;
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
