#!/bin/bash
#Script for Testing Timers (Delaytimer, Eventtimer and Waittimer)

echo "Test der SIO Timer (Delaytimer, Eventtimer and Waittimer)"
echo "Sie können das Script jederzeit mit CTRL C abbrechen"
sleep 2

clear


eingabefehler=j
while eingabefehler=j 
 do
  read -p "Geben Sie die SCU Nummer ein (z.B. 0022 für scuxl0022):" scunummer
  scuname="tcp/scuxl$scunummer.acc"
  echo ""
  echo "Script benötigt "$scuname "und SIO im Slave slot 7"
  echo ""
  read -p  "Ist der Test Set-Up ok ( j / n ):" eingabefehler 
   if [ "$eingabefehler" == "j" ] 
    then
     break
    else
     echo -e "\e[7mSie können den Namen nun aendern\e[0m"
     sleep 1
   fi
done

clear



echo "-------------------------------------------------------------------------------"
echo "WAIT Timer Tests"

echo "some readouts the wait timer"

eb-read  $scuname 0x4c0810/2
eb-read  $scuname 0x4c081c/2

echo "some readouts the wait timer"

eb-read  $scuname 0x4c0810/2
eb-read  $scuname 0x4c081c/2

echo "some readouts the wait timer"

eb-read  $scuname 0x4c0810/2
eb-read  $scuname 0x4c081c/2

echo "software reset of the wait timer"

eb-write $scuname 0x4c0810/2 0xffff
echo "some readouts the wait timer"

eb-read  $scuname 0x4c0810/2
eb-read  $scuname 0x4c081c/2

echo "some readouts the wait timer"

eb-read  $scuname 0x4c0810/2
eb-read  $scuname 0x4c081c/2

echo "some readouts the wait timer"

eb-read  $scuname 0x4c0810/2
eb-read  $scuname 0x4c081c/2

echo "Check that WAIT timer counts up and starts after reset"
echo "now do some readout with 2sec sleeps to see overrun behaviour"

sleep 5
echo "some readouts the wait timer"
eb-read  $scuname 0x4c0810/2
eb-read  $scuname 0x4c081c/2


sleep 2
echo "some readouts the wait timer 2"
eb-read  $scuname 0x4c0810/2
eb-read  $scuname 0x4c081c/2

sleep 2
echo "some readouts the wait timer 4"
eb-read  $scuname 0x4c0810/2
eb-read  $scuname 0x4c081c/2


sleep 2
echo "some readouts the wait timer 6"
eb-read  $scuname 0x4c0810/2
eb-read  $scuname 0x4c081c/2

sleep 2
echo "some readouts the wait timer 8"
eb-read  $scuname 0x4c0810/2
eb-read  $scuname 0x4c081c/2

sleep 2
echo "some readouts the wait timer 10"
eb-read  $scuname 0x4c0810/2
eb-read  $scuname 0x4c081c/2

sleep 2
echo "some readouts the wait timer 12"
eb-read  $scuname 0x4c0810/2
eb-read  $scuname 0x4c081c/2

echo "It can be seen that the WAIT timer didn't stop on overrun"
echo "-------------------------------------------------------------------------------"
read -p " Continue to EVENT timer test with Return or finish with CTRL C" weiter

clear
echo "Event Timer Test"

echo "some readouts the Event timer"
eb-read  $scuname 0x4c080c/2
eb-read  $scuname 0x4c0818/2

echo "some readouts the Event timer"
eb-read  $scuname 0x4c080c/2
eb-read  $scuname 0x4c0818/2


echo "some readouts the Event timer"
eb-read  $scuname 0x4c080c/2
eb-read  $scuname 0x4c0818/2


echo "software reset of the Event timer"
eb-write $scuname 0x4c080c/2 0xffff

echo "some readouts the Event timer"
eb-read  $scuname 0x4c080c/2
eb-read  $scuname 0x4c0818/2


echo "some readouts the Event timer"
eb-read  $scuname 0x4c080c/2
eb-read  $scuname 0x4c0818/2

echo "some readouts the Event timer"

eb-read  $scuname 0x4c080c/2
eb-read  $scuname 0x4c0818/2

echo "It can be seen that EVENT timer counts up and starts after reset"
echo "now do some readout with 2sec sleeps to see overrun behaviour"

sleep 5
echo "some readouts the Event timer"
eb-read  $scuname 0x4c080c/2
eb-read  $scuname 0x4c0818/2

sleep 2
echo "some readouts the Event timer 2"
eb-read  $scuname 0x4c080c/2
eb-read  $scuname 0x4c0818/2


sleep 2
echo "some readouts the Event timer 4"
eb-read  $scuname 0x4c080c/2
eb-read  $scuname 0x4c0818/2


sleep 2
echo "some readouts the Event timer 6"
eb-read  $scuname 0x4c080c/2
eb-read  $scuname 0x4c0818/2

sleep 2
echo "some readouts the Event timer 8"
eb-read  $scuname 0x4c080c/2
eb-read  $scuname 0x4c0818/2

sleep 2
echo "some readouts the Event timer 10"
eb-read  $scuname 0x4c080c/2
eb-read  $scuname 0x4c0818/2

sleep 2
echo "some readouts the Event timer 12"
eb-read  $scuname 0x4c080c/2
eb-read  $scuname 0x4c0818/2

sleep 2
echo "some readouts the Event timer 14"
eb-read  $scuname 0x4c080c/2
eb-read  $scuname 0x4c0818/2

sleep 2
echo "some readouts the Event timer 16"
eb-read  $scuname 0x4c080c/2
eb-read  $scuname 0x4c0818/2
sleep 2

echo "some readouts the Event timer 18"
eb-read  $scuname 0x4c080c/2
eb-read  $scuname 0x4c0818/2

sleep 2
echo "some readouts the Event timer 20"
eb-read  $scuname 0x4c080c/2
eb-read  $scuname 0x4c0818/2

echo "At this point we stop Event timer test (32bit count would last too long)"
echo "-------------------------------------------------------------------------------"
read -p " Continue to DELAY timer test with Return or finish with CTRL C" weiter

clear

echo "Delay Timer Test"

echo "preload lw latch 0xbabe "
eb-write  $scuname 0x4c0820/2 0xbabe 

echo "preload hw latch 0x00ff"
eb-write  $scuname 0x4c0822/2 0x00ff 


echo "load Delay timer with latched values by writing dummy data to 0x4c080e"
eb-write  $scuname 0x4c080e/2 0x0000 

echo " This start delay timer due to bit 8 of High Word is zero"

echo "---------------Timer runs now--------------------------------------"
echo "read upper and lower value of delay timer and sleep for 3 secs"
eb-read  $scuname 0x4c080e/2
eb-read  $scuname 0x4c0820/2
sleep 3
echo "read upper and lower value of delay timer and sleep for 3 secs"
eb-read  $scuname 0x4c080e/2
eb-read  $scuname 0x4c0820/2
sleep 3

echo "read upper and lower value of delay timer and sleep for 3 secs"
eb-read  $scuname 0x4c080e/2
eb-read  $scuname 0x4c0820/2
sleep 3

echo "read upper and lower value of delay timer and sleep for 3 secs"
eb-read  $scuname 0x4c080e/2
eb-read  $scuname 0x4c0820/2
sleep 3

echo "read upper and lower value of delay timer and sleep for 3 secs"
eb-read  $scuname 0x4c080e/2
eb-read  $scuname 0x4c0820/2
sleep 3

echo "read upper and lower value of delay timer and sleep for 3 secs"
eb-read  $scuname 0x4c080e/2
eb-read  $scuname 0x4c0820/2
sleep 3

echo "read upper and lower value of delay timer and sleep for 3 secs"
eb-read  $scuname 0x4c080e/2
eb-read  $scuname 0x4c0820/2
sleep 3

echo "read upper and lower value of delay timer and sleep for 3 secs"
eb-read  $scuname 0x4c080e/2
eb-read  $scuname 0x4c0820/2
sleep 3


echo "It can be seen that the timer performs overrun and stops at 0x01FFFFFF"

read -p " Continue for RESTART of DELAY timer with Return or finish with CTRL C" weiter

echo "preload hw latch 0x00ff"
eb-write  $scuname 0x4c0822/2 0x00ff 

echo "load Delay timer with latched values by writing dummy data to 0x4c080e"
eb-write  $scuname 0x4c080e/2 0x0000 

echo "read upper and lower value of delay timer and sleep for 3 secs"
eb-read  $scuname 0x4c080e/2
eb-read  $scuname 0x4c0820/2
sleep 3
echo "read upper and lower value of delay timer and sleep for 3 secs"
eb-read  $scuname 0x4c080e/2
eb-read  $scuname 0x4c0820/2
sleep 3

echo "read upper and lower value of delay timer and sleep for 3 secs"
eb-read  $scuname 0x4c080e/2
eb-read  $scuname 0x4c0820/2

echo "now we pause delay timer by setting bit 8 of upper word"
echo ""
eb-write $scuname 0x4c0822/2 0x01a2
eb-write $scuname 0x4c080e/2 0x0000
echo "read upper and lower value of delay timer and sleep for 3 secs"
eb-read  $scuname 0x4c080e/2
eb-read  $scuname 0x4c0820/2
echo "read upper and lower value of delay timer and sleep for 3 secs"
eb-read  $scuname 0x4c080e/2
eb-read  $scuname 0x4c0820/2

echo ""
read -p " Delaytimer is now stopped and can be started again as shown above"

echo "End of test sio3_test_timer.sh"






