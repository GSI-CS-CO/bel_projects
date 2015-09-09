#!/bin/bash
#Script for Testing Timers (Delaytimer, Eventtimer and Waittimer)

echo "Testing Timers (Delaytimer, Eventtimer and Waittimer)"
scuname="tcp/scuxl0079.acc"
echo ""
echo "This script must be used with "$scuname " and SIO2 or SIO3 at slave slot 7"
echo "In case of wrong scu change variable scuname in this script"

read -p " Continue with Return or finish with CTRL C" weiter 


echo "At first do the Wait Timer Tests"

echo "some readouts the wait timer"

eb-read  $scuname 0x4e0810/2
eb-read  $scuname 0x4e081c/2

echo "some readouts the wait timer"

eb-read  $scuname 0x4e0810/2
eb-read  $scuname 0x4e081c/2

echo "some readouts the wait timer"

eb-read  $scuname 0x4e0810/2
eb-read  $scuname 0x4e081c/2

echo "software reset of the wait timer"

eb-write $scuname 0x4e0810/2 0xffff
echo "some readouts the wait timer"

eb-read  $scuname 0x4e0810/2
eb-read  $scuname 0x4e081c/2

echo "some readouts the wait timer"

eb-read  $scuname 0x4e0810/2
eb-read  $scuname 0x4e081c/2

echo "some readouts the wait timer"

eb-read  $scuname 0x4e0810/2
eb-read  $scuname 0x4e081c/2

echo "It can be seen that wait timer counts up and starts after reset"
echo "now do some readout with 2sec sleeps to see overrun behaviour"

sleep 5
echo "some readouts the wait timer"
eb-read  $scuname 0x4e0810/2
eb-read  $scuname 0x4e081c/2


sleep 2
echo "some readouts the wait timer 2"
eb-read  $scuname 0x4e0810/2
eb-read  $scuname 0x4e081c/2

sleep 2
echo "some readouts the wait timer 4"
eb-read  $scuname 0x4e0810/2
eb-read  $scuname 0x4e081c/2


sleep 2
echo "some readouts the wait timer 6"
eb-read  $scuname 0x4e0810/2
eb-read  $scuname 0x4e081c/2

sleep 2
echo "some readouts the wait timer 8"
eb-read  $scuname 0x4e0810/2
eb-read  $scuname 0x4e081c/2

sleep 2
echo "some readouts the wait timer 10"
eb-read  $scuname 0x4e0810/2
eb-read  $scuname 0x4e081c/2

sleep 2
echo "some readouts the wait timer 12"
eb-read  $scuname 0x4e0810/2
eb-read  $scuname 0x4e081c/2

echo "It can be seen that the wait timer didn't stop on overrun"
read -p " Continue to next test with Return or finish with CTRL C" weiter
echo ""
echo ""
echo ""

echo "Now perform the Event Timer Tests"

echo ""

echo "some readouts the Event timer"
eb-read  $scuname 0x4e080c/2
eb-read  $scuname 0x4e0818/2

echo "some readouts the Event timer"
eb-read  $scuname 0x4e080c/2
eb-read  $scuname 0x4e0818/2


echo "some readouts the Event timer"
eb-read  $scuname 0x4e080c/2
eb-read  $scuname 0x4e0818/2


echo "software reset of the Event timer"
eb-write $scuname 0x4e080c/2 0xffff

echo "some readouts the Event timer"
eb-read  $scuname 0x4e080c/2
eb-read  $scuname 0x4e0818/2


echo "some readouts the Event timer"
eb-read  $scuname 0x4e080c/2
eb-read  $scuname 0x4e0818/2

echo "some readouts the Event timer"

eb-read  $scuname 0x4e080c/2
eb-read  $scuname 0x4e0818/2

echo "It can be seen that Event timer counts up and starts after reset"
echo "now do some readout with 2sec sleeps to see overrun behaviour"

sleep 5
echo "some readouts the Event timer"
eb-read  $scuname 0x4e080c/2
eb-read  $scuname 0x4e0818/2

sleep 2
echo "some readouts the Event timer 2"
eb-read  $scuname 0x4e080c/2
eb-read  $scuname 0x4e0818/2


sleep 2
echo "some readouts the Event timer 4"
eb-read  $scuname 0x4e080c/2
eb-read  $scuname 0x4e0818/2


sleep 2
echo "some readouts the Event timer 6"
eb-read  $scuname 0x4e080c/2
eb-read  $scuname 0x4e0818/2

sleep 2
echo "some readouts the Event timer 8"
eb-read  $scuname 0x4e080c/2
eb-read  $scuname 0x4e0818/2

sleep 2
echo "some readouts the Event timer 10"
eb-read  $scuname 0x4e080c/2
eb-read  $scuname 0x4e0818/2

sleep 2
echo "some readouts the Event timer 12"
eb-read  $scuname 0x4e080c/2
eb-read  $scuname 0x4e0818/2

sleep 2
echo "some readouts the Event timer 14"
eb-read  $scuname 0x4e080c/2
eb-read  $scuname 0x4e0818/2

sleep 2
echo "some readouts the Event timer 16"
eb-read  $scuname 0x4e080c/2
eb-read  $scuname 0x4e0818/2
sleep 2

echo "some readouts the Event timer 18"
eb-read  $scuname 0x4e080c/2
eb-read  $scuname 0x4e0818/2

sleep 2
echo "some readouts the Event timer 20"
eb-read  $scuname 0x4e080c/2
eb-read  $scuname 0x4e0818/2

echo "At this point we stop Event timer test (32bit count would last too long)"

read -p " Continue to next test with Return or finish with CTRL C" weiter


echo""
echo""


echo "Now do some tests with delay timer"

echo "preload lw latch 0xbabe "
eb-write  $scuname 0x4e0820/2 0xbabe 

echo "preload hw latch 0x00ff"
eb-write  $scuname 0x4e0822/2 0x00ff 


echo "load Delay timer with latched values by writing dummy data to 0x4e080e"
eb-write  $scuname 0x4e080e/2 0x0000 

echo " This start delay timer due to bit 8 of High Word is zero"

echo "---------------Timer runs now--------------------------------------"
echo "read upper and lower value of delay timer and sleep for 3 secs"
eb-read  $scuname 0x4e080e/2
eb-read  $scuname 0x4e0820/2
sleep 3
echo "read upper and lower value of delay timer and sleep for 3 secs"
eb-read  $scuname 0x4e080e/2
eb-read  $scuname 0x4e0820/2
sleep 3

echo "read upper and lower value of delay timer and sleep for 3 secs"
eb-read  $scuname 0x4e080e/2
eb-read  $scuname 0x4e0820/2
sleep 3

echo "read upper and lower value of delay timer and sleep for 3 secs"
eb-read  $scuname 0x4e080e/2
eb-read  $scuname 0x4e0820/2
sleep 3

echo "read upper and lower value of delay timer and sleep for 3 secs"
eb-read  $scuname 0x4e080e/2
eb-read  $scuname 0x4e0820/2
sleep 3

echo "read upper and lower value of delay timer and sleep for 3 secs"
eb-read  $scuname 0x4e080e/2
eb-read  $scuname 0x4e0820/2
sleep 3

echo "read upper and lower value of delay timer and sleep for 3 secs"
eb-read  $scuname 0x4e080e/2
eb-read  $scuname 0x4e0820/2
sleep 3

echo "read upper and lower value of delay timer and sleep for 3 secs"
eb-read  $scuname 0x4e080e/2
eb-read  $scuname 0x4e0820/2
sleep 3


echo "It can be seen that the timer performs overrun and stops at 0x01FFFFFF"

read -p " Continue to next test with Return or finish with CTRL C" weiter

echo "Now we restart delay timer"

echo "preload hw latch 0x00ff"
eb-write  $scuname 0x4e0822/2 0x00ff 

echo "load Delay timer with latched values by writing dummy data to 0x4e080e"
eb-write  $scuname 0x4e080e/2 0x0000 

echo "read upper and lower value of delay timer and sleep for 3 secs"
eb-read  $scuname 0x4e080e/2
eb-read  $scuname 0x4e0820/2
sleep 3
echo "read upper and lower value of delay timer and sleep for 3 secs"
eb-read  $scuname 0x4e080e/2
eb-read  $scuname 0x4e0820/2
sleep 3

echo "read upper and lower value of delay timer and sleep for 3 secs"
eb-read  $scuname 0x4e080e/2
eb-read  $scuname 0x4e0820/2


echo "now we pause delay timer by setting bit 8 of upper word"
echo ""
eb-write $scuname 0x4e0822/2 0x01a2
eb-write $scuname 0x4e080e/2 0x0000
echo "read upper and lower value of delay timer and sleep for 3 secs"
eb-read  $scuname 0x4e080e/2
eb-read  $scuname 0x4e0820/2
echo "read upper and lower value of delay timer and sleep for 3 secs"
eb-read  $scuname 0x4e080e/2
eb-read  $scuname 0x4e0820/2

echo ""
read -p " Delaytimer is now stopped and can be started again as shown above"

echo ""
echo ""
echo    "This test now continues with some wait timer tests"
read -p "Continue to next test with Return or finish with CTRL C" weiter

echo "some readouts the wait timer"

eb-read  $scuname 0x4e0810/2
eb-read  $scuname 0x4e082c/2

echo "some readouts the wait timer"

eb-read  $scuname 0x4e0810/2
eb-read  $scuname 0x4e082c/2

echo "some readouts the wait timer"

eb-read  $scuname 0x4e0810/2
eb-read  $scuname 0x4e082c/2

echo "software reset of the wait timer"

eb-write $scuname 0x4e0810/2 0xffff
echo "some readouts the wait timer"

eb-read  $scuname 0x4e0810/2
eb-read  $scuname 0x4e082c/2

echo "some readouts the wait timer"

eb-read  $scuname 0x4e0810/2
eb-read  $scuname 0x4e082c/2

echo "some readouts the wait timer"

eb-read  $scuname 0x4e0810/2
eb-read  $scuname 0x4e082c/2




