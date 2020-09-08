/*!
 *  @file daqexample.cpp
 *  @brief Minimal example of using a DAQ channel.
 *
 *  @date 31.05.2019
 *  @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *
 ******************************************************************************
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */
#include <iostream>
#include <daq_administration.hpp>
#include <unistd.h>

#include <boost/scope_exit.hpp>

using namespace Scu;
using namespace daq;
using namespace std;

///////////////////////////////////////////////////////////////////////////////
/*
 * We have to specialized the abstract base class "DaqChannel" to give a target
 * for the received data-blocks in the callback function "onDataBlock"
 */
class MyChannel: public DaqChannel
{
   /*
    * In this example we need something like a counter which counts the
    * incoming data-blocks to limit the maximum of received blocks.
    */
   uint  m_receivedBlockCount;

public:
   MyChannel( unsigned int channelNumber )
      :DaqChannel( channelNumber )
      ,m_receivedBlockCount( 0 )
   {}

   uint getBlockCount( void ) const
   {
      return m_receivedBlockCount;
   }

   /*
    * This callback function becomes invoked after each successful
    * received data-block.
    * The type DAQ_DATA_T is defined in the file "daq_descriptor.h"
    * and is a 16-bit type (uint16_t).
    * The parameter wordLen gives the number of received 16 bit payload data
    * words.
    */
   bool onDataBlock( ::DAQ_DATA_T* pData, std::size_t wordLen ) override;
};

void dump64( uint64_t v )
{
   cout << hex;
   for( int i = sizeof( uint64_t )-1; i >= 0; i-- )
     cout << "0x" << static_cast<uint>(reinterpret_cast<uint8_t*>(&v)[i]) << ' ';
   cout << dec;
}

/*
 * The example call back function for the received raw data calculates
 * the maximum, minimum and the average value over the entire received block.
 */
bool MyChannel::onDataBlock( ::DAQ_DATA_T* pData, std::size_t wordLen )
{
   m_receivedBlockCount++;
#if 1
   cout << "Slot:            " << getSlot() << endl;
   cout << "Channel:         " << getNumber() << endl;
   cout << "Block number:    " << m_receivedBlockCount << endl;
   cout << "Sequence number: " <<
                    static_cast<uint>(descriptorGetSequence()) << endl;
#endif
#if 1
   cout << "Timestamp:       " << wrToTimeDateString( descriptorGetTimeStamp() ) <<
        std::hex << ",    0x" << descriptorGetTimeStamp() << std::dec
                              << "  " << descriptorGetTimeStamp() << "  ";
        dump64(  descriptorGetTimeStamp() );
        cout << endl;
#endif
#if 1
   cout << "Sample time:     " << descriptorGetTimeBase() << " ns" << endl;
   cout << "Received values: " << wordLen << " in 16 bit words" << endl;

   ::DAQ_DATA_T minimum = static_cast<::DAQ_DATA_T>(~0);
   ::DAQ_DATA_T maximum = 0;
   uint64_t summe = 0;
   for( std::size_t i = 0; i < wordLen; i++ )
   {
      minimum = min( pData[i], minimum );
      maximum = max( pData[i], maximum );
      summe   += pData[i];
   }
   ::DAQ_DATA_T average = summe / wordLen;

   cout << "Minimum: " << minimum << " -> "
        << rawToVoltage( minimum ) << " Volt" << endl;
   cout << "Average: " << average << " -> "
        << rawToVoltage( average ) << " Volt" << endl;
   cout << "Maximum: " << maximum << " -> "
        << rawToVoltage( maximum ) << " Volt\n" << endl;
#endif
   return false;
}


///////////////////////////////////////////////////////////////////////////////
int main( int argc, const char** ppArgv )
{
   if( argc < 2 )
   {
      cerr << "ERROR: No SCU-URL given!" << endl;
      return EXIT_FAILURE;
   }
   cout << "SCU-URL: " << ppArgv[1] << endl;

   try
   {  /*
       * Building a object of etherbone-connection for tie communication
       * whith a DAQ via wishbone/etherbone.
       */
      DaqEb::EtherboneConnection ebConnection( ppArgv[1] );

      /*
       * If the etherbone connection will not made outside of the class
       * DaqAdministration, so this class will accomplished this
       * in its constructor and the disconnect will made by the destructor.
       * In this example the connection will made outside.
       */
      ebConnection.connect();

      /*
       * The class "DaqAdministration" represents a SCU including at least one
       * DAQ in a arbitrary SCU-BUS slot.
       */
      DaqAdministration scuWithDaq( &ebConnection );

      /*
       * If this SCU doesn't have any DAQ...
       */
      if( scuWithDaq.getMaxFoundDevices() == 0 )
      {
         cerr << "ERROR: No DAQ in this SCU: \""
              << scuWithDaq.getScuDomainName()
              << "\" found!" << endl;
         return EXIT_FAILURE;
      }

      /*
       * Printing the slot-number of all found DAQs and the number of
       * its channels.
       * NOTE: The counting of DAQs and its channels begins at 1 and not
       *       at zero!
       */
       for( uint i = 1; i <= scuWithDaq.getMaxFoundDevices(); i++ )
       {
          cout << "slot " << scuWithDaq.getSlotNumber( i )
               << ": channels: " << scuWithDaq.readMaxChannels( i ) << endl;
       }

       /*
        * In this simple example we'll choose the first found DAQ form the left
        * side.
        * A object of the class "DaqDevice" represents the hardware of one of
        * the DAQ locating in the SCU-Bus.
        * NOTE: The physical slot-number isn't the virtual device number!
        * The constructor needs the slot-number the function
        * DaqAdministration::getSlotNumber translates a device-number into
        * the slot-number.
        */
        DaqDevice myDaq( scuWithDaq.getSlotNumber( 1 ) );

        /*
         * Registering the DAQ device in the DAQ administrator.
         */
        scuWithDaq.registerDevice( &myDaq );

        /*
         * At least one channel object will need.
         * We chose the channel 1.
         */
        MyChannel myChannel( 1 );

        /*
         * The chosen channel object has to be registered in
         * the concerning DAQ device. We do this here.
         */
        myDaq.registerChannel( &myChannel );

        /*
         * It is possible to browse trough all registered DAQs
         * and all registered channels by the following nested loops.
         * But in our case it is a bit boring because only one device and
         * only one channel will used in this example.
         */
        cout << "Registered devices:" << endl;
        for( auto& itDaq: scuWithDaq ) // Device loop
        {
           cout << "\tSlot: " << itDaq->getSlot() << ", Registered channels:"
                << endl;
           for( auto& itChannel: *itDaq ) // Channel loop
           {
              cout << "\t\tChannel: " << itChannel->getNumber() << endl;
           }
        }
        cout << endl;

        /*
         * Sending the command "continuous on" with a sample rate of 1 ms to
         * the LM32.
         * The enum constant DAQ_SAMPLE_1MS is in file
         * daq_command_interface.h defined.
         */
        myChannel.sendEnableContineous( DAQ_SAMPLE_1MS );

        /*
         *                   *** Main loop ***
         *
         * In this example the main loop will left after the receiving of
         * 10 data blocks has been completed.
         */
        while( myChannel.getBlockCount() < 10 )
        { /*
           * The function DaqAdministration::distributeData() polls the
           * size of the DDR3 ring-buffer. When the level of the ring buffer
           * has at least reached the data volume of one block, so this data will
           * copied in the concerning channel object. That means
           * the corresponding call-back function DaqChannel::onDataBlock()
           * becomes invoked by the function DaqAdministration::distributeData().
           */
           try
           {
              scuWithDaq.distributeData();
           }
           catch( exception& e )
           {
              cerr << "ERROR: Block receiving: " << e.what() << endl;
              break;
           }
        }
#ifdef CONFIG_DAQ_TIME_MEASUREMENT
        cout << "Maximum WB/EB cycle time: " << scuWithDaq.getElapsedTime()
             << " us" << endl;
#endif
        /*
         * At least we send a reset command to the LM32 program:
         */
        scuWithDaq.sendReset();

        /*
         * In this example the etherbone-connection was made outside
         * of the class DaqAdministration, so we have also to disconnect
         * outside.
         */
        ebConnection.disconnect();
   } // End try()

   catch( exception& e )
   {
      cerr << "ERROR: Something was going wrong: \"" << e.what() << '"'
           << endl;
      return EXIT_FAILURE;
   }
   return EXIT_SUCCESS;
}

//============================== EOF ==========================================
