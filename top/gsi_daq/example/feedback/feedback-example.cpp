/*!
 *  @file feedback-example.cpp
 *  @brief Minimal example testing feedback channels of function generators.
 *
 *  @date 26.05.2020
 *  @copyright (C) 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
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
#include <scu_env.hpp>
#include <scu_fg_feedback.hpp>


using namespace Scu;
using namespace std;


///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 * Specialization of class "FgFeedbackChannel" so that the callback function
 * "onData" can be implemented.
 */
class MyFeedbackChannel: public FgFeedbackChannel
{
public:

   MyFeedbackChannel( const uint fgNumber ): FgFeedbackChannel( fgNumber ) {}

   void onInit( void ) override;

   void onData( uint64_t wrTimeStampTAI, DAQ_T actlValue, DAQ_T setValue ) override;
};

/*-----------------------------------------------------------------------------
 * This callback function is optional only.
 *
 * It becomes invoked once the container object of type FgFeedbackDevice of
 * this object is registered in the administration object of type
 * FgFeedbackAdministration.
 */
void MyFeedbackChannel::onInit( void )
{
   cout << (isMil()? "MIL":"ADDAC/ACU")
        << "-feedback object: \"fg-" << getSocket() << '-'
        << getFgNumber() << "\"  final registered!" << endl;
}

/*-----------------------------------------------------------------------------
 * This is the central callback function which becomes invoked by the
 * loop- function "FgFeedbackAdministration::distributeData()" when
 * the data of a registered function generator has been received.
 */
void MyFeedbackChannel::onData( uint64_t wrTimeStampTAI,
                                DAQ_T actlValue,
                                DAQ_T setValue )
{
   cout << "fg-" << getSocket() << '-'
        << getFgNumber()
        << "\tlast time-stamp: " << getLastTimestamp() << " readable: "
        << daq::wrToTimeDateString( getLastTimestamp() )
        << "\tcurrent time-stamp: " << wrTimeStampTAI << " readable: "
        << daq::wrToTimeDateString( wrTimeStampTAI )
        << "\tset value: " << daq::rawToVoltage( setValue )
        << " Volt\tactual value: " << daq::rawToVoltage( actlValue )
        << " Volt" << endl;
}

///////////////////////////////////////////////////////////////////////////////
int main( const int argc, const char** ppArgv )
{
   try
   {
      string ebDeviceName;
      if( isRunningOnScu() )
      {
         ebDeviceName = "dev/wbm0";
      }
      else
      {
         cout << "CAUTION: This application doesn't run on SCU, in this case"
                 " the port- forwarder \"socat\" has to be run on SCU at first!" << endl;
         if( argc < 2 )
         {
            cerr << "ERROR: No SCU-URL given!" << endl;
            return EXIT_FAILURE;
         }
         ebDeviceName = ppArgv[1];
      }

      cout << "Wishbone-device: " << ebDeviceName << endl;

      /*
       * Building a object of etherbone-connection for the communication
       * whith a DAQ via wishbone/etherbone.
       *
       * Note: The namespace-name "DaqEb" is defined in the header file
       *       "daq_eb_ram_buffer.hpp" as:
       *           "namespace DaqEb = FeSupport::Scu::Etherbone;"
       *       It's just a shortcut making the source code better readable.
       *
       * Assuming the program argument is a SCU-wishbone-address e.g.:
       * "tcp/scuxl4711" or in the case this example will run directly in
       * the IPC of a SCU: "dev/wbm0".
       */
      DaqEb::EtherboneConnection ebConnection( ebDeviceName );

      /*
       * If the etherbone connection will not made outside of the class
       * FgFeedbackAdministration, so this class will accomplished this
       * in its constructor and the disconnect will made by the destructor.
       * In this example the connection will made outside, but that is not
       * really necessary.
       */
      ebConnection.connect();

      /*
       * We need a object of class FgFeedbackAdministration
       * which handles the etherbone-connection respectively
       * the communication from and to the LM32 application and DDR3 memory.
       * And as container for at least one or more DAQ-devices.
       *
       * If it's impossible to establish a connection so the
       * constructor will thrown an exception.
       *
       * The constructors second parameter determines whether a
       * re-scan will made or not. If true a re-scan will made.
       */
      FgFeedbackAdministration myScu( &ebConnection, false );

      /*!
       * Uncomment this if you will not the history of the data-buffer.
       */
     // myScu.clearBuffer();

      /*
       * After the successful generating of the object "myScu" some information
       * about the concerning SCU are available:
       */
      cout << "LM32 firmware major version number: " << myScu.getLm32SoftwareVersion() << endl;
      cout << "Found function generators in: " << myScu.getScuDomainName() << endl;
      cout << myScu.getNumOfFoundFg() << " Function generators found." << endl;
      cout << myScu.getNumOfFoundMilFg() << " MIL function generators found." << endl;
      cout << myScu.getNumOfFoundNonMilFg() << " ADDAC/ACU Function generators found." << endl;
      for( const auto& fg: myScu.getFgList() )
      {
         cout << "Slot " << fg.getSlot()
              << ", Version: " << fg.getVersion()
              << ", Bits: " << fg.getOutputBits()
              << ", fg-" << fg.getSocket() << '-' << fg.getDevice()
              << "\tDAQ: " << (fg.isMIL()? "MIL" : "ADDAC/ACU") << endl;
      }

      /*
       * Checks whether the currently loaded LM32-firmware supports ADDAC/ACQ DAQs,
       * if not, than an exception becomes thrown when trying to register
       * an ADDAC/DAQ- feedback channel.
       */
      if( !myScu.isAddacDaqSupport() )
      {
         cerr << "CAUTION: The currently loaded firmware on " << myScu.getScuDomainName()
              << " doesn't support ADDAC/ACU- DAQs! " << endl;
      }

      /*
       * In this example we use a bit unconventional method obtaining the first found
       * function generator via the iterator pointing to the list begin.
       */
      const auto& pMyFirstFoundFg = myScu.getFgList().begin();
      cout << "Using first found FG: \"fg-" << pMyFirstFoundFg->getSocket() << '-'
           << pMyFirstFoundFg->getDevice() << "\" its a "
           << (pMyFirstFoundFg->isMIL()? "MIL" : "ADDAC/ACU") << "-device" << endl;


      /*
       * Creating a feedback channel for a specific function generator.
       * The value of the argument is function generator number of
       * the concerning device:
       * Example: fg-39-1
       *                |
       *                +- Here it's function generator number 1 of
       *                   device (socket) 39.
       *
       * But in this example the first found function-generator will used.
       *
       * NOTE: At this moment the feedback-channel-object doesn't know
       *       yet whether it's a MIL or ADDAC/ACU object.
       */
      MyFeedbackChannel myFeedBackChannel( pMyFirstFoundFg->getDevice() );
     //   MyFeedbackChannel myFeedBackChannel( 1 );
      /*
       * Creating a feedback device as container for the feedback channels.
       * Based on the constructors argument the device object will be a
       * MIL- or a ADDAC/ACU- device.
       * The constructors argument is the device-number respectively the
       * socket number.
       * Example: fg-39-1
       *             ||
       *             ++- Here it's socket number 39 and function generator 1.
       *
       * But in this example the first found device will used.
       *
       * NOTE: In the case of a invalid socket-number the constructor will
       *       thrown an exception!
       * NOTE: If the socket number between 1 and 12 then it will be
       *       a ADDAC/ACU-device. In this case the socket-number is equal
       *       to the SCU- bus slot number.
       *
       * In the case of a invalid socket number the constructor will thrown
       * an exception!
       */
      FgFeedbackDevice myFeedBackDevice( pMyFirstFoundFg->getSocket() );
      //FgFeedbackDevice myFeedBackDevice( 39 );

      /*
       * Registering the function-generator feedback object in the feedback
       * device.
       * At this moment the feedback channel object mutates to a MIL- or
       * to a ADDAC/ACU- feedback-channel depending on the feedback-device
       * object (see above).
       *
       * If the channel-number invalid or already registered then an
       * exception will thrown!
       */
      myFeedBackDevice.registerChannel( &myFeedBackChannel );

      /*
       * For info and debug purposes only:
       * Before the registration the device doesn't know yet what type it belongs to.
       */
      cout << "Device type: " << deviceType2String( myFeedBackDevice.getTyp() ) << endl;

      /*
       * Making the feedback-device known for the feedback administration of
       * this SCU.
       * Is the hardware belonging to this device not present or this device
       * was already registered, then en exception will thrown!
       */
      myScu.registerDevice( &myFeedBackDevice );

      /*
       * For info and debug purposes only:
       * After successful registration, the device knows what type it belongs to.
       */
      cout << "Device type: " << deviceType2String( myFeedBackDevice.getTyp() ) << endl;

      /*
       * Function "daq::getSysMicrosecs()" is defined in "daq_calculations.hpp"
       * The following loop will run for 10 seconds.
       * This isn't really the best solution, but all other alternatives
       * will made this example too complex.
       */
      daq::USEC_T stopTime = daq::getSysMicrosecs() + daq::MICROSECS_PER_SEC * 1; //10


      myScu.setMaxEbCycleDataLen(100);

      /*
       * Polling loop. This could be a own thread as well.
       */
      //int x = 0;
      do
      {  /*
          * If data form one or more registered function generators present,
          * then this function will invoke the callback functions "onData"
          * of the concerning feedback channel objects.
          * In this example it is the function "MyFeedbackChannel::onData"
          * only.
          */
         uint remainingData = myScu.distributeData();

         /*
          * To reduce the duration of the function distributeData() in some cases
          * there are still data sets in the DDR3-buffer.
          * The return value has the number of data-sets which are still in
          * the DDR3-buffer which shall be read by the next call of distributeData().
          */
         cout << "Remaining data sets in DDR3 RAM: " << remainingData << endl;
         //x++;
      }
      while( daq::getSysMicrosecs() < stopTime );
      //while( x < 4 );

      /*
       * In this example the connection was made outside of the
       * object myScu (see above), therefore the disconnect has
       * to be made outside as well.
       *
       * CAUTION: Only when the connect was made outside like in this example
       *          its allow to made the disconnect outside as well.
       *          Otherwise a "Segmentation fault" occurs!
       */
      ebConnection.disconnect();

      cout << "End..." << endl;

   } // end try()

   catch( exception& e )
   {
      cerr << "ERROR: Something went wrong: \"" << e.what() << '"'
           << endl;
      return EXIT_FAILURE;
   }

   return EXIT_SUCCESS;
}
