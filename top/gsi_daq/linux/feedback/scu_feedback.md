Introduction in the library  scu_fg_feedback.a  {#mainpage}
==============================================

The library user interface consists of three classes declared in header-file scu_fg_feedback.hpp :

1. [FgFeedbackChannel](@ref Scu::FgFeedbackChannel)
2. [FgFeedbackDevice](@ref Scu::FgFeedbackDevice)
3. [FgFeedbackAdministration](@ref Scu::FgFeedbackAdministration)

Whereby the class [FgFeedbackChannel](@ref Scu::FgFeedbackChannel) is abstract and with its virtual callback function
[onData](@ref Scu::FgFeedbackChannel::onData) which has to be overwritten by your own function.\n
Each object of this Class represents a feedback channel of one function generator which receives the:
1. White rabbit timestamp.
2. The set value of the function generator
3. The actual value from a LEMO- connector.


A obcect of type [FgFeedbackDevice](@ref Scu::FgFeedbackDevice) is the container of at least one or more objects of type
[FgFeedbackChannel](@ref Scu::FgFeedbackChannel). It represents e.g. a SCU- bus slave. E.g. ADDAC, MIL or ACU device.\n
Each channel object of Type [FgFeedbackChannel](@ref Scu::FgFeedbackChannel) respectively its driven type has to be registered via
function [registerChannel](@ref Scu::FgFeedbackDevice::registerChannel).\n\n


A object of type [FgFeedbackAdministration](@ref Scu::FgFeedbackAdministration) is the container of at least one or ore objects of type
[FgFeedbackDevice](@ref Scu::FgFeedbackDevice)\n
It represents the hardware of the SCU.\n
Each device object of type [FgFeedbackDevice](@ref Scu::FgFeedbackDevice) has to be registered via
function [registerDevice](@ref Scu::FgFeedbackAdministration::registerDevice).

This class contains the polling function [distributeData](@ref Scu::FgFeedbackAdministration::distributeData)
This function has to be put in a polling-loop, which runs for example in a own thread.


See also in the following small well documented example program
@include feedback-example.cpp

In some cases not all DAQ-data residing in the DDR3-RAM will read and evaluated in one iteration step.\n
This could be the case when the data is jammed in the DDR3-RAM.\n
In this case the function [distributeData](@ref Scu::FgFeedbackAdministration::distributeData) returns by the number of DDR3-payload items (size 64 bit respectively 8 byte per item) which are in the
DDR3-RAM and not evaluated by the callback [onData](@ref Scu::FgFeedbackChannel::onData) function yet.

Proposal to read the entire data-buffer in any cases:
@code
void myThreadFunction_or_myTimerEvent( void )
{
   uint remainingData;
   do
   {
      remainingData = myScu.distributeData();

      // Maybe do something others here...
   }
   while( remainingData != 0 );
}
@endcode
