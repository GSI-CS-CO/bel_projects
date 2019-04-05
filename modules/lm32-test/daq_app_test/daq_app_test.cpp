
#include <iostream>
#include <iomanip>
#include <eb_console_helper.h>
#include <daq_channel_container.hpp>
#include <unistd.h>
#include <assert.h>
#include <termios.h>

using namespace daq;
using namespace std;

class Terminal
{
   struct termios m_originTerminal;

public:
   Terminal( void )
   {
      struct termios newTerminal;
      ::tcgetattr( STDIN_FILENO, &m_originTerminal );
      newTerminal = m_originTerminal;
      newTerminal.c_lflag     &= ~(ICANON | ECHO);  /* Disable canonic mode and echo.*/
      newTerminal.c_cc[VMIN]  = 1;  /* Reading is complete after one byte only. */
      newTerminal.c_cc[VTIME] = 0; /* No timer. */
      ::tcsetattr( STDIN_FILENO, TCSANOW, &newTerminal );
   }

   ~Terminal( void )
   {
      ::tcsetattr( STDIN_FILENO, TCSANOW, &m_originTerminal );
   }

   static int readKey( void )
   {
      int inKey = 0;
      fd_set rfds;

      struct timeval sleepTime = {0, 10};
      FD_ZERO( &rfds );
      FD_SET( STDIN_FILENO, &rfds );

      if( ::select( STDIN_FILENO+1, &rfds, NULL, NULL, &sleepTime ) > 0 )
         ::read( STDIN_FILENO, &inKey, sizeof( inKey ) );
      else
         inKey = 0;
      return (inKey & 0xFF);
   }
};




class MyDaqChannel: public DaqChannel
{
   uint64_t      m_LastTimestamp;
   unsigned int  m_blockCount;
public:
   MyDaqChannel( void )
     :DaqChannel( 0 )
     ,m_LastTimestamp( 0 )
     ,m_blockCount( 0 )
   {
      cout << "Constructor" << endl;
   }

   virtual ~MyDaqChannel( void )
   {
      cout << "Destructor channel: " << getNumber() <<
              ",  slot: " << getSlot() << endl;
   }

   void clearCounter( void ) { m_blockCount = 0; }

   bool onDataBlock( DAQ_DATA_T* pData, std::size_t wordLen ) override;

};


//-----------------------------------------------------------------------------
bool MyDaqChannel::onDataBlock( DAQ_DATA_T* pData, std::size_t wordLen )
{
  cout << "-------------------------------------" << endl;
  cout << "Slot:    " << getSlot();
  cout << "\nChannel: " << getNumber() << endl;

  cout << setiosflags(ios::uppercase);
  cout << "trigger: 0x" << hex << setfill('0') << setw(8)
       << descriptorGetTreggerCondition( pData ) << dec << endl;

  cout << "delay: 0x" << hex << descriptorGetTriggerDelay( pData )
       << dec << endl;

  cout << "CRC: 0x" << hex << setfill('0') << setw(2)
       << static_cast<unsigned int>(descriptorGetCrc( pData ))
       << dec << endl;

  cout << "Block:     " << ++m_blockCount << endl;
  cout << "Timestamp: " << descriptorGetTimestamp( pData )
       << endl;
  if( m_LastTimestamp > 0 )
     cout << "Delta time: " << descriptorGetTimestamp( pData ) - m_LastTimestamp << endl;
  m_LastTimestamp = descriptorGetTimestamp( pData );


#if 0
  cout << "Seconds:     "
       << daqDescriptorGetTimeStampSec( reinterpret_cast<DAQ_DESCRIPTOR_T*>(pData) )
       << endl;
  cout << "Nanoseconds: " <<
     daqDescriptorGetTimeStampNanoSec( reinterpret_cast<DAQ_DESCRIPTOR_T*>(pData) )
       << endl;
#endif
  cout << "Data:" << endl;
  DAQ_DATA_T minimum = static_cast<DAQ_DATA_T>(~0);
  DAQ_DATA_T maximum = 0;
  uint64_t summe = 0;
  for( std::size_t i = c_discriptorWordSize; i < wordLen; i++ )
  {
   //  if( i % 100 == 0 )
   //     cout << "  " << pData[i] << endl;
     minimum = min( pData[i], minimum );
     maximum = max( pData[i], maximum );
     summe   += pData[i];
  }

  unsigned int numOfSamples = wordLen - c_discriptorWordSize;
  cout << "  Samples: " << numOfSamples << ", Modus: " ESC_FG_CYAN;
  if( descriptorWasPM( pData ) )
     cout << "Post Mortem";
  else if( descriptorWasHiRes( pData ) )
     cout << "High Resolution";
  else if( descriptorWasDaq( pData ))
     cout << "Continuous";
  cout << ESC_NORMAL << endl;

  assert( (wordLen - numOfSamples) > 0 );
  DAQ_DATA_T average = summe / numOfSamples;

  cout << "  Minimum: " << minimum << " -> " << rawToVoltage( minimum ) << " Volt" << endl;
  cout << "  Average: " << average << " -> " << rawToVoltage( average ) << " Volt" << endl;
  cout << "  Maximum: " << maximum << " -> " << rawToVoltage( maximum ) << " Volt" << endl;
  cout << ESC_NORMAL;
  return false;
}

//------------------------------------------------------------------------------
void doTest( const string wbName )
{
   DaqChannelContainer< MyDaqChannel > oDaqInterface( wbName );

   cout << "WB-interface:        " << oDaqInterface.getWbDevice() << endl;
   cout << "Found devices:       " << oDaqInterface.getMaxFoundDevices() << endl;
   cout << "Registered channels: " << oDaqInterface.getMaxChannels() << endl;

   if( oDaqInterface.getMaxFoundDevices() == 0 )
   {
      cerr << ESC_FG_RED "Error: No DAQ devices found!" ESC_NORMAL << endl;
      return;
   }

   if( oDaqInterface.getMaxChannels() == 0 )
   {
      cerr << ESC_FG_RED "Error: No DAQ channels found! How that!?!" ESC_NORMAL
      << endl;
      return;
   }

   for( auto& itDev: oDaqInterface )
   {
      cout << "Slot:  " << itDev->getSlot() << endl;
      for( auto& itChannel: *itDev )
      {
         cout << "   Channel: " << itChannel->getNumber() << endl;
      }
   }

   MyDaqChannel* pChannel_a = oDaqInterface.getChannelByAbsoluteNumber( 1 );
   assert( pChannel_a != nullptr );
   MyDaqChannel* pChannel_b = oDaqInterface.getChannelByAbsoluteNumber( 5 );
   assert( pChannel_b != nullptr );
//   pChannel_a->sendTriggerDelay( 0x55 );
//   pChannel_a->sendTriggerCondition( 0xCAFEAFFE );
//   pChannel_b->sendTriggerDelay( 0xAA );
//   pChannel_b->sendTriggerCondition( 0x08154711 );

   cout << "Returncode: " << oDaqInterface.getLastReturnCodeString() << endl;

   cout << "Delay a: 0x" << hex << pChannel_a->receiveTriggerDelay() << endl;
   cout << "Trigger condition a: 0x" << hex << pChannel_a->receiveTriggerCondition() << dec << endl;
   cout << "Delay b: 0x" << hex << pChannel_b->receiveTriggerDelay() << endl;
   cout << "Trigger condition b: 0x" << hex << pChannel_b->receiveTriggerCondition() << dec << endl;



//   pChannel_a->sendEnableContineous( DAQ_SAMPLE_100US, 10 );
   pChannel_b->sendEnableContineous( DAQ_SAMPLE_100US, 4 );

//   pChannel_a->sendEnablePostMortem();
   //pChannel_a->sendEnableHighResolution();
  // usleep( 100000 );
 //  pChannel_a->sendDisablePmHires( true );
  //
   usleep( 1000000 );

  //  pChannel_a->sendDisablePmHires( true );
  // cout << "Ram level: " << oDaqInterface.getCurrentRamSize(true ) << endl;


   int key;
   while( (key = Terminal::readKey()) != '\e' )
   {
     // unsigned int RamSize = oDaqInterface.getCurrentRamSize( true );
     // if( RamSize != 0 )
     //    cout << "Ram level: " << RamSize << endl;

      switch( key )
      {
         case 'd':
         {
            cout << "Endurance test begin" << endl;
            do
            {
                pChannel_a->sendDisablePmHires( false );
                pChannel_a->sendDisableContinue();
            }
            while( Terminal::readKey() == 0 );
            cout << "Endurance test end" << endl;
            break;
         }
         case 'p':
         {
            pChannel_a->sendDisablePmHires( true );
            break;
         }
         case 'P':
         {
            pChannel_a->sendEnablePostMortem(true);
            break;
         }
         case 'C':
         {
            pChannel_a->sendEnableContineous( DAQ_SAMPLE_100US, 4 );
            pChannel_b->sendEnableContineous( DAQ_SAMPLE_100US, 4 );
            usleep( 1000000 );
            break;
         }
         case 'c':
         {
            pChannel_a->sendDisableContinue();
            pChannel_b->sendDisableContinue();
            break;
         }
         case 'r':
         {
            oDaqInterface.sendReset();
            for( auto& itDev: oDaqInterface )
               for( auto& itChannel: *itDev )
                  static_cast<MyDaqChannel*>(itChannel)->clearCounter();
            break;
         }
      }
      oDaqInterface.distributeData();
   }

//   oDaqInterface.start();
// sleep( 4 );
}


///////////////////////////////////////////////////////////////////////////////
int main( int argc, const char** ppArgv )
{
   cout << ESC_FG_MAGNETA << ppArgv[0] << ESC_NORMAL << endl;

   try
   {
      Terminal terminal;
      doTest( ppArgv[1] );
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
