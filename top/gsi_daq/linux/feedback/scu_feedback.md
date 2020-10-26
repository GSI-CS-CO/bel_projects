Introduction in the library  scu_fg_feedback.a  {#mainpage}
==============================================

The library user interface consists of three classes:

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


See also in the following link of a small well documented example program.\n
[C++ example](@ref feedback-example.cpp)
