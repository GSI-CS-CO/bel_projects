/*!
 *  @file daqexample.cpp
 *  @brief Minimal example of using a MIL DAQ compare channel.
 *
 *  @date 02.09.2019
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
#include <mdaq_administration.hpp>

using namespace Scu;
using namespace std;

///////////////////////////////////////////////////////////////////////////////
/*
 * We need to specializing the class MiLdaq::DaqCompare obtaining
 * the callback function "MiLdaq::DaqCompare::onData"
 */
class MyCompare: public MiLdaq::DaqCompare
{
public:
   MyCompare( uint iterfaceAddress ):
      MiLdaq::DaqCompare( iterfaceAddress )
      {}

   /*
    * See comment below.
    */
   void onData( uint64_t wrTimeStamp, MiLdaq::MIL_DAQ_T actlValue,
                                      MiLdaq::MIL_DAQ_T setValue ) override;

   /*
    * Function becomes invoked ones its object is linked to the
    * MiLdaq::DaqAdministration object. (Nice to have but not
    * in every cases necessary.)
    */
   void onInit( void ) override
   {
      cout << "DaqCompare( " << getAddress() << " ) registered." << endl;
   }
};

/*-----------------------------------------------------------------------------
 * This is the central callback function which becomes invoked by the
 * loop- function "MiLdaq::DaqAdministration::distributeData()" when
 * the data of a registered function generator has been received.
 */
void MyCompare::onData( uint64_t wrTimeStamp, MiLdaq::MIL_DAQ_T actlValue,
                                              MiLdaq::MIL_DAQ_T setValue )
{
   cout << "fg-" << getParent()->getLocation() << '-'
                 << getAddress() << "\ttime: " << wrTimeStamp << " readable: "
                 << daq::wrToTimeDateString( wrTimeStamp )
                 << "\tset value: " << daq::rawToVoltage( setValue )
                 << " Volt\tactual value: " << daq::rawToVoltage( actlValue )
                 << " Volt" << endl;
}


///////////////////////////////////////////////////////////////////////////////
/*
 * It's not really necessary specializing the class MiLdaq::DaqAdministration,
 * but in this way we obtain the optional callback function "onUnregistered"
 * which becomes invoked for each receiving of data
 * where no "compare object" still exist.
 */
class MyMilDaqAdministration: public MiLdaq::DaqAdministration
{
public:
   MyMilDaqAdministration( DaqEb::EtherboneConnection* poEtherbone )
     :MiLdaq::DaqAdministration( poEtherbone )
     {}

   /*
    * Becomes invoked after each receiving of data which doesn't belongs
    * to a already registered data-compare object.
    */
   void onUnregistered( RingItem* pUnknownItem ) override;
};

/*-----------------------------------------------------------------------------
 * By this optional callback function it becomes possible making a
 * auto- registration of all received function generators if desired.
 */
void MyMilDaqAdministration::onUnregistered( RingItem* pUnknownItem )
{
  /*
   * You can activate the following code by setting #if 0 to #if 1
   * just for fun, but this will produced a lot of additional output of all
   * unregistered function generators.
   */
#if 0
   cout << pUnknownItem->getTimestamp() << ' '
        << static_cast<int>(pUnknownItem->getActValue()) << ' '
        << static_cast<int>(pUnknownItem->getSetValue())
        << "\tnot registered  fg-" << pUnknownItem->getMilDaqLocation()
        << '-' <<  pUnknownItem->getMilDaqAddress() << endl;
#endif
}

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////// main /////////////////////////////////////////
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
       * Building a object of etherbone-connection for the communication
       * whith a DAQ via wishbone/etherbone.
       */
      DaqEb::EtherboneConnection ebConnection( ppArgv[1] );

      /*
       * If the etherbone connection will not made outside of the class
       * Mildaq::DaqAdministration, so this class will accomplished this
       * in its constructor and the disconnect will made by the destructor.
       * In this example the connection will made outside, but that is not
       * really necessary.
       */
      ebConnection.connect();


      /*
       * We need a object of class MiLdaq::DaqAdministration
       * which handles the etherbone-connection respectively
       * the communication from and to the LM32 application.
       * And as container for at least one or more DAQ-devices.
       */
      MyMilDaqAdministration milDaqContainer( &ebConnection );

      /*
       * In this example the set and actual values of function generator
       * "fg-39-130" will received and evaluated.
       *
       * The order of the following both constructor invocations
       * doesn't matter.
       */

      /*
       * 1) We need a DAQ device which contains at least one or more
       *    MIL-DAQ compare objects.
       *
       *                       fg-39-130
       *                          ||      */
      MiLdaq::DaqDevice myDevice( 39 );


      /*
       * 2) We need a DAQ compare object containing the callback function
       *    MyCompare::onData
       *
       *             fg-39-130
       *                   |||           */
      MyCompare myCompare( 130 );

      /*
       * Making the DAQ device the compare channel known.
       */
      myDevice.registerDaqCompare( &myCompare );

      /*
       * Making the DAQ-administrator respectively the DAQ device container
       * the DAQ device known.
       */
      milDaqContainer.registerDevice( &myDevice );

      /*
       * Function "daq::getSysMicrosecs()" is defined in "daq_calculations.hpp"
       * The following loop will run for 10 seconds.
       * This is really not the best solution, but all other alternatives
       * will made this example too complex.
       */
      daq::USEC_T stopTime = daq::getSysMicrosecs() + daq::MICROSECS_PER_SEC * 10;
      do
      {  /*
          * This function will invoke the callback function MyCompare::onData
          * in the case the channel is known, otherwise the optional
          * callback function MyMilDaqAdministration::onUnregistered
          * becomes invoked.
          */
         milDaqContainer.distributeData();
      }
      while( daq::getSysMicrosecs() < stopTime );

      /*
       * In this example the connection was made outside of the
       * object milDaqContainer (see above), therefore the disconnect has
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
      cerr << "ERROR: Something was going wrong: \"" << e.what() << '"'
           << endl;
      return EXIT_FAILURE;
   }
   return EXIT_SUCCESS;
}

//================================== EOF ======================================

